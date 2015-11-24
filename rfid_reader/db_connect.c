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
 * @file db_connect.c
 * @author Katie Awty-Carroll (kah36@aber.ac.uk)
 * 
 * Connects to PostgreSQL database and checks that the plant name is valid, 
 * and that it is entered in the database with the correct car ID. 
 * This ensures that plants which have no record in the database, or 
 * which have mismatched car ID's and plant ID's, do not get imaged.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nfc/nfc.h>
#include "mifare.h"
#include <postgresql/libpq-fe.h>

char *server = "server";
char *user = "username";
char *password = "password";

FILE *output_file;

PGconn *open_connection(char *database) {
    
    PGconn *conn;
    
    conn = PQsetdbLogin(server, NULL, NULL, NULL, database, user, password);
    
    if (PQstatus(conn) == CONNECTION_BAD) {
      fprintf(output_file,"Could not connect to database\n");
      fclose(output_file);
      exit(1);
    }
    
    return conn;
}

PGresult *fetch_results(PGconn *conn, char *query) {
    
    PGresult *res;
    
    res = PQexec(conn,query);
    if(PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(output_file,"Query did not execute correctly\n");
        fclose(output_file);
        exit(1);
    }  
    return res;
}

int get_plant_details(char *plant_name, char *plant_carid, FILE *output){
    
    PGconn *conn;
    PGresult *res;
    char query_1[100];
    char query_2[100];
    char *db_1 = "db_1";
    char *db_2 = "db_2";
    char *ret_carid;
    output_file = output;
    
    conn = open_connection(db_1);  
    snprintf(query_1, 100, "SELECT EXISTS (SELECT 1 FROM import_data WHERE id_tag = '%s')", plant_name);
    res = fetch_results(conn, query_1);

    if(strcmp(PQgetvalue(res,0,0), "t") !=0) { // Returns t or f
        fprintf(output_file,"Plant is not in database\n");
        fclose(output_file);
        exit(1);
    }
    PQfinish(conn);
    PQclear(res);
    
    conn = open_connection(db_2);
    snprintf(query_2, 100, "SELECT carid from plants WHERE identcode = '%s'", plant_name);
    res = fetch_results(conn, query_2);
    
    ret_carid = PQgetvalue(res, 0, 0);
    if(strcmp(plant_carid, ret_carid) != 0) {
        fprintf(output_file,"Plant ID and car ID do not match\n");
        fprintf(output_file,"Car ID on tag is %s, car ID in database is %s\n",plant_carid, ret_carid);
        fclose(output_file);
        exit(1);
    }
    
    printf("%s",plant_name);
    fflush(stdout);
    PQfinish(conn);
    PQclear(res);
    
    return 0;
}
