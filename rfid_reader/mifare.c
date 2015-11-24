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
 * @file mifare.c
 * @author Katie Awty-Carroll (kah36@aber.ac.uk)
 * 
 * Based on the example mifare.c which is distributed with the libnfc library.
 * Edited so that output goes to the same file as the rest of the program.
*/

/*-
 * Free/Libre Near Field Communication (NFC) library
 *
 * Libnfc historical contributors:
 * Copyright (C) 2009      Roel Verdult
 * Copyright (C) 2009-2013 Romuald Conty
 * Copyright (C) 2010-2012 Romain Tartière
 * Copyright (C) 2010-2013 Philippe Teuwen
 * Copyright (C) 2012-2013 Ludovic Rousseau
 * See AUTHORS file for a more comprehensive list of contributors.
 * Additional contributors of this file:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  1) Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  2 )Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Note that this license only applies on the examples, NFC library itself is under LGPL
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nfc/nfc.h>
#include "mifare.h"

FILE *output_file;

bool nfc_initiator_mifare_cmd(nfc_device *pnd, const mifare_cmd mc, const uint8_t ui8Block, mifare_param *pmp, FILE *output)
{
    uint8_t  abtRx[265];
    size_t  szParamLen;
    uint8_t  abtCmd[265];
    //bool    bEasyFraming;
    
    output_file = output;
    
    abtCmd[0] = mc;               // The MIFARE Classic command
    abtCmd[1] = ui8Block;         // The block address (1K=0x00..0x39, 4K=0x00..0xff)

    switch (mc) {
    // Read and store command have no parameter
    case MC_READ:
    case MC_STORE:
        szParamLen = 0;
        break;

      // Authenticate command
    case MC_AUTH_A:
    case MC_AUTH_B:
        szParamLen = sizeof(struct mifare_param_auth);
        break;

      // Data command
    case MC_WRITE:
        szParamLen = sizeof(struct mifare_param_data);
        break;

      // Value command
    case MC_DECREMENT:
    case MC_INCREMENT:
    case MC_TRANSFER:
        szParamLen = sizeof(struct mifare_param_value);
        break;

      // Please fix your code, you never should reach this statement
    default:
        return false;
    }

  // When available, copy the parameter bytes
    if (szParamLen)
        memcpy(abtCmd + 2, (uint8_t *) pmp, szParamLen);

    // FIXME: Save and restore bEasyFraming
    // bEasyFraming = nfc_device_get_property_bool (pnd, NP_EASY_FRAMING, &bEasyFraming);
    if (nfc_device_set_property_bool(pnd, NP_EASY_FRAMING, true) < 0) {
        fprintf(output_file, "nfc_device_set_property_bool");  //This should never happen
        return false;
    }
    // Fire the mifare command
    int res;
    if ((res = nfc_initiator_transceive_bytes(pnd, abtCmd, 2 + szParamLen, abtRx, sizeof(abtRx), -1))  < 0) {
        if (res == NFC_ERFTRANS) {
        // "Invalid received frame",  usual means we are
        // authenticated on a sector but the requested MIFARE cmd (read, write)
        // is not permitted by current access bytes;
        // So there is nothing to do here.
        } else {
            fprintf(output_file, "Mifare authentication failed: nfc_initiator_transceive_bytes");
            }
    // XXX nfc_device_set_property_bool (pnd, NP_EASY_FRAMING, bEasyFraming);
    return false;
    }
    
  // When we have executed a read command, copy the received bytes into the param
    if (mc == MC_READ) {
        if (res == 16) {
            memcpy(pmp->mpd.abtData, abtRx, 16);
        } else {
            return false;
        }
    }
    return true;
}
