#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <my_global.h>
#include <mysql.h>
#include <ctype.h>

#include "SQL_functions.h"

#define SELECT_Schueler "SELECT Benutzer.id, name, passwort, kurse FROM Benutzer, Schueler WHERE name=\"?\" AND Schueler.id=Benutzer.id;"

/** \brief Überprüfen, ob eine Person in der Datenbank ist und ob das Passwor stimmt
 *
 * \param pers person*  Person, die angemeldet werden soll
 * \return void
 *  Die Funktion testet, ob der Name oder das Kürzel (je nachdem was eingegeben wurde)
 *  (Erkennung an der Länge des Strings: =3 --> Kürzel, >3 --> Name)
 *  in der DB vorhanden ist. Wenn der Name existiert wird geprüft ob das Passwort richtig ist.
 *  Wenn das Passwort stimmt wird der bool-Wert "auth" in "person" auf true gesetzt.
 *  --> die Person ist authentifiziert.
 */
void verifyUser(person * pers){
    bool isAcronym;

    if(pers->name==NULL || pers->passwort==NULL){
		printExitFailure("Programm falsch!");
	}

    isAcronym=detectConvertAcronym(pers);
	/*if(strlen(pers->name) > 3){
        isAcronym=false;
    }else{
        if(strlen(pers->name) == 3){
            isAcronym=true;
            pers->acronym=pers->name;
            pers->name=NULL;
            //char * c=pers->acronym_id;
            int p=0;
            while(pers->acronym[p]){
                pers->acronym[p]=toupper(pers->acronym[p]);
                p++;
            }
        }
    }*/

    MYSQL *my=mysql_init(NULL);
    if(my == NULL){
        printExitFailure("MYSQL init failure");
    }

    if(mysql_real_connect(my, "localhost", "web_user", "web_pass", "base3", 0, NULL, 0) == NULL){
        /*fprintf (stderr, "Fehler mysql_real_connect(): %u (%s)\n",
        mysql_errno (my), mysql_error (my));
        exit(EXIT_FAILURE);*/
        printExitFailure("MYSQL-connection error!");
    }else{
        //fprintf(stderr, "Connection extablished!\n");
    }

    MYSQL_RES * result=NULL;
    bool found=false;

    if(!isAcronym){

        //TODO: sql-injection verhindern
        size_t len_start=strlen("SELECT Benutzer.id, name, passwort, kurse FROM Benutzer, Schueler WHERE name=\"");
        size_t len_mid=strlen(pers->name);
        size_t len_end=strlen("\" AND Schueler.id=Benutzer.id;");

        char * sql_query=calloc(len_start+len_end+len_mid+1, sizeof(char));
        if(sql_query == NULL){
            printExitFailure("Es konnte kein Speicher für \"sql_query\" angefrdert werden");
        }
        strcpy(sql_query, "SELECT Benutzer.id, name, passwort, kurse FROM Benutzer, Schueler WHERE name=\"");
        strncat(sql_query, pers->name, len_mid);
        strcat(sql_query, "\" AND Schueler.id=Benutzer.id;");

        fprintf(stderr, "Schueler\nsql_query: %s\n", sql_query);

        if(mysql_query(my, sql_query)){  //SELECT * FROM Benutzer WHERE name="<name>"; Liefert 0 bei Erfolg
            printExitFailure("mysql_query failed");
        }/*
        MYSQL_STMT * stmt = mysql_stmt_init(my);
        if(stmt == NULL){
            printExitFailure("stmt: es konnte kein Speicher angefpordert werden.");
        }

        mysql_stmt_prepare(stmt, SELECT_Schueler, strlen(SELECT_Schueler));
        MYSQL_BIND bind[1];
        memset(bind, 0,sizeof(bind));
        bind[0].buffer_type=MYSQL_TYPE_STRING;
        bind[0].buffer=(char *)pers->name;
        bind[0].buffer_length=50;
        bind[0].is_null=0;
        bind[0].length=(long *)strlen(pers->name);

        mysql_stmt_bind_param(stmt, bind);

        mysql_stmt_execute(stmt);
        mysql_stmt_store_result(stmt);

        int row_count= mysql_stmt_affected_rows(stmt);*/


        result = mysql_store_result(my);

        if(mysql_num_rows(result) == 1){
            //Es ist eine Schüler
            found=true;
        }else{
            //Entweder es ist eine Lehrer, oder die Person existiert nicht in der Datenabank (oder ist doppelt vorhanden).
            char * sql_query_lehrer=calloc(strlen("SELECT Benutzer.id, name, passwort, kurse, Lehrer.kuerzel FROM Lehrer, Benutzer WHERE Benutzer.name=\"\" AND Lehrer.id=Benutzer.id;")+strlen(pers->name)+1, sizeof(char));
            if(sql_query_lehrer == NULL){
                printExitFailure("Es konnte kein Speicher für \"sql_query_lehrer\" angefrdert werden");
            }
            strcpy(sql_query_lehrer, "SELECT Benutzer.id, name, passwort, kurse, Lehrer.kuerzel FROM Lehrer, Benutzer WHERE Benutzer.name=\"");
            strcat(sql_query_lehrer, pers->name);
            strcat(sql_query_lehrer, "\" AND Lehrer.id=Benutzer.id;");

            fprintf(stderr, "Lehrer\nsql_query: %s\n", sql_query_lehrer);

            if(mysql_query(my, sql_query_lehrer)){
                printExitFailure("mysql_query failed");
            }
            result = mysql_store_result(my);

            if(mysql_num_rows(result) == 1){
                found=true;
                isAcronym=true; //Da wir jetzt eine Person mit Kürzel gefunden haben
                pers->isTeacher=true;
            }else{
                //Nichts gefunden (oder Name doppelt)
                found=false;
                isAcronym=false;
                pers->auth=false;
                pers->sid=0;
            }
        }

    }else{
        //Es ist eine Person mit Kürzel! (Das sind bei uns nur Lehrer)
        char * sql_query=calloc(strlen("SELECT Benutzer.id, name, passwort, kurse, Lehrer.kuerzel FROM Lehrer, Benutzer WHERE Lehrer.kuerzel=\"\" AND Lehrer.id=Benutzer.id;")+strlen(pers->acronym)+1, sizeof(char));
        if(sql_query == NULL){
            printExitFailure("Es konnte kein Speicher für \"sql_query\" angefrdert werden");
        }
        strcpy(sql_query, "SELECT Benutzer.id, name, passwort, kurse, Lehrer.kuerzel FROM Lehrer, Benutzer WHERE Lehrer.kuerzel=\"");
        strcat(sql_query, pers->acronym);
        strcat(sql_query, "\" AND Lehrer.id=Benutzer.id;");

        fprintf(stderr, "Kuerzel\nsql_query: %s\n", sql_query);

        if(mysql_query(my, sql_query)){ //
            printExitFailure("mysql_query failed");
        }

        result=mysql_store_result(my);

        if(mysql_num_rows(result) == 1){
            found=true;
            isAcronym=true;
            pers->isTeacher=true;
        }
    }

    //Ab hier wurde die SQL-Query für Leher oder Schuler ausgeführt.
    if(found == true){

        MYSQL_ROW row;
        row=mysql_fetch_row(result);
        fprintf(stderr, "\nEin Ergebnis!\n Name: %s, Pass: %s\n", row[COL_NAME], row[COL_PASS]);

        //TODO: Passwort RICHTIG machen (mit hash + salt)
        if(strcmp(pers->passwort, row[COL_PASS]) == 0){
            //fprintf(stderr, "Passwort \"%s\" korrekt!\n", pers->passwort);
            pers->auth=true;
            if(isAcronym){
                //es ist eine Leherer (Person mit Kürzel)
                pers->name=calloc(strlen(row[COL_NAME])+1, sizeof(char));
                strcpy(pers->name, row[COL_NAME]);
            }else{
                //Person ohne Kürzel
                pers->acronym=NULL;
            }

            pers->courses=calloc(strlen(row[COL_COURSE])+1, sizeof(char));
            strcpy(pers->courses, row[COL_COURSE]);

            srand((unsigned int)time(NULL));
            pers->sid=rand();
        }else{
            pers->auth=false;
            pers->sid=0;
        }
    }

    mysql_free_result(result);
    mysql_close(my);
}

/** \brief  Funktion zum Testen, ob der Name vielleicht ein Kürzel ist
 *
 * \param pers person*  Personen-Objekt, das den Name enthält
 * \return bool
 *
 */
bool detectConvertAcronym(person * pers){
    bool isAcronym;

    if(pers->name==NULL || pers->passwort==NULL){
		printExitFailure("Programm falsch!");
	}

    //Ist anstelle ds Namen ein Kürzel?
	if(strlen(pers->name) > 3){
        isAcronym=false;
    }else{
        if(strlen(pers->name) == 3){
            isAcronym=true;
            pers->acronym=pers->name;
            pers->name=NULL;
        }
    }

    //Wenn ein Kürzel in acronym gespeichert ist, dann dieses zu Großbuchstaben umwandeln
    if(pers->acronym != NULL && strlen(pers->acronym)==3){
        int p=0;
        while(pers->acronym[p]){
            pers->acronym[p]=toupper(pers->acronym[p]);
            p++;
        }
    }

    return isAcronym;
}

void insertUser(person * pers){

}

