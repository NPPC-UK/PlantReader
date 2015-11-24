/*
 * Copyright (C) 2015		Aberystwyth University	
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* 
 * @file main.c
 * @author Katie Awty-Carroll (kah36@aber.ac.uk)
 * 
 * Script to retrieve name and car ID of a plant from its RFID tag.
 * Based on examples from the 'libnfc' library http://nfc-tools.org/index.php?title=Libnfc.
 * Errors go to "rfid_out.txt". 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nfc/nfc.h>
#include <ctype.h>
#include "mifare.h"
#include <time.h>

nfc_context *context;
nfc_device *pnd;
nfc_target nt;
mifare_param mp;
mifare_classic_tag mtKeys;   
char name_buffer[17];                      
char carid_buffer[17];
char plant_name[17];                       
char plant_carid[5];
char file_name[41] = "rfid_out.txt"; // Output going to file as Python script relies on stdout
FILE *output_file;

/* Key is just the standard key */
uint8_t key[] = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

const nfc_modulation nmMifare = {
  .nmt = NMT_ISO14443A,             // Modulation type
  .nbr = NBR_106,                   // Baud rate
};

/* Writes the bytes into the buffer, then extracts bytes which are not null to find the name */
void write_name(const uint8_t *pbtData, const size_t szBytes) {
    
    size_t  szPos;
    int i = 0;
    int a = 0;
    
    for (szPos = 0; szPos < szBytes; szPos++) {
        name_buffer[i] = pbtData[szPos];
        i++;
    }

    for(i=0;i<16;i++) {
        if(name_buffer[i] != 0x0) {
            plant_name[a] = name_buffer[i];
            a++;
            }
    }
}

/* Writes the bytes into the buffer, then extracts bytes which are not null and are numeric to find the number */
void write_num(const uint8_t *pbtData, const size_t szBytes) {
    
    size_t  szPos;
    int i = 0;
    int a = 0;
    
    for (szPos = 0; szPos < szBytes; szPos++) {
        carid_buffer[i] = pbtData[szPos];
        i++;
    }

    for(i=0;i<16;i++) {
        if(carid_buffer[i] != 0x0) {
            if(isdigit(carid_buffer[i])) {
                plant_carid[a] = carid_buffer[i];
                a++;
            }
        }
    }
}

/* Blocks need authenticating with the key before they can be read */
bool authenticate_block(uint32_t uiBlock) {
    
    memcpy(mp.mpa.abtAuthUid, nt.nti.nai.abtUid + nt.nti.nai.szUidLen - 4, 4);
    memcpy(mp.mpa.abtKey, key, 6);
    
    if (nfc_initiator_mifare_cmd(pnd, MC_AUTH_A, uiBlock, &mp, output_file)) {         
        memcpy(mtKeys.amb[uiBlock].mbt.abtKeyA, &mp.mpa.abtKey, 6);
        return true;
    }
    if (nfc_initiator_select_passive_target(pnd, nmMifare, nt.nti.nai.abtUid, nt.nti.nai.szUidLen, NULL) <= 0) {
        return false;
    }
    return false;
}

/* Reads the block passed to it */
bool read_block(uint32_t block) {
    
    if(!authenticate_block(block)) {
        return false;
    }
    if (nfc_initiator_mifare_cmd(pnd, MC_READ, block, &mp, output_file)) {
        if(block == 1) {                    //Block 1 is where the number is stored                
            write_num(mp.mpd.abtData, 16);
            return true;
        }
        if(block == 5) {                    //Block 5 is where the name is stored
            write_name(mp.mpd.abtData, 16);
            return true;
        }
    } else {
        fprintf(output_file, "Failed to read block %d\n", block);
        return false;
    }
}

int main() {
  
    nfc_init(&context);
    int timeout;
    
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    
    output_file = fopen(file_name, "a"); // Append to file
    
    if(output_file == NULL) {
        exit(1);
    }
    
    if (context == NULL) {
        exit(1);
    }

    /* Try to open NFC reader - gets three attempts*/
    for(timeout=0;timeout<3;timeout++) {
        pnd = nfc_open(context, NULL);
        if(pnd !=NULL) {
            break;
        }
        nfc_exit(context);
        fprintf(output_file,"No NFC reader found\n");
        fclose(output_file);
        exit(1);
    }
    
    /* Currently polls infinitely */
    nfc_device_set_property_bool(pnd, NP_INFINITE_SELECT, true);
            
    /* Initialise tag - this should really only fail if the wrong type of tag is presented */
    if(!nfc_initiator_select_passive_target(pnd, nmMifare, NULL, 0, &nt)) {
        fprintf(output_file, "No valid tag found\n");
        fclose(output_file);
        nfc_exit(context);
        exit(1);
    }
    
    /* Read from the tag - gets five attempts */
    for(timeout = 1;timeout<=5;timeout++) {  
        if (read_block(1) && read_block(5)) { //Block 1 contains the car ID, block 5 contains the plant name
            
            /* Print the time to the output file */
            fprintf(output_file,"****** %.2d-%.2d-%.2d %.2d:%.2d:%.2d ******\n",  tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900,tm.tm_hour, tm.tm_min, tm.tm_sec);
    
            /* Clean up */
            nfc_exit(context);
            
            fprintf(output_file,"Read from plant: %s on car: %s on attempt %d\n", plant_name, plant_carid, timeout);
    
            /* Now fetch the plant details */
            get_plant_details(plant_name, plant_carid, output_file);
            exit(0);
        }
        else {
            fprintf(output_file,"Failed to read from tag on attempt %d\n", timeout);
        }
    }
    
    nfc_exit(context); 
    fprintf(output_file,"Reading from tag failed\n");
    fclose(output_file);
    exit(1);
}
