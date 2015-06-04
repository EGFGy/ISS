/*
 * File:   main_strtok_base3.c
 * Author: Quwert (Mr)
 *
 * Created on 25. Mai 2015, 13:50
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <my_global.h>
#include <mysql.h>
#include <ctype.h>

#include "CGI_functions.h"
#include "SQL_functions.h"

#define content_max 700
//3*(bytes von name (wegen %XX <- Sonderzeichen) + 3*(bytes von pass) + reserve


/* Struktur der Datenbank
 * mysql> DESCRIBE  Benutzer;
 * +----------+--------------+------+-----+---------+----------------+
 * | Field    | Type         | Null | Key | Default | Extra          |
 * +----------+--------------+------+-----+---------+----------------+
 * | id       | int(11)      | NO   | PRI | NULL    | auto_increment |
 * | name     | varchar(50)  | YES  |     | NULL    |                |
 * | passwort | varchar(200) | YES  |     | NULL    |                |
 * | kurse    | varchar(200) | YES  |     | NULL    |                |
 * +----------+--------------+------+-----+---------+----------------+
 *
 */

/* mysql> SELECT Benutzer.id, name, passwort, kurse, Lehrer.kuerzel FROM Lehrer, Benutzer WHERE Lehrer.kuerzel="FFU" AND Lehrer.id=Benutzer.id;
 * +----+---------------+----------+-------------+---------+
 * | id | name          | passwort | kurse       | kuerzel |
 * +----+---------------+----------+-------------+---------+
 * |  5 | Fritz Fuchs   | testp    | 1INF,1P_INF | FFU     |
 * +----+---------------+----------+-------------+---------+
 * mysql> SELECT Benutzer.id, name, passwort, kurse FROM Benutzer, Schueler WHERE name="Max Mustermann" AND Schueler.id=Benutzer.id;
 * +----+----------------+--------------+--------------+
 * | id | name           | passwort     | kurse        |
 * +----+----------------+--------------+--------------+
 * |  1 | Max Mustermann | fallersleben | 1L,1ETH,1PH2 |
 * +----+----------------+--------------+--------------+
 */

int main(int argc, char **argv)
{
	char * name = NULL; //Pointer für den eingegebenen Namen
	char * password = NULL; //Pointer für das eingegebene Passwort
	cgi datCGI;
	bool cookies_found=false;
	person login_person;

	login_person.name=NULL;
	login_person.acronym=NULL;
	login_person.passwort=NULL;
	login_person.auth=false;
	login_person.courses=NULL;
	login_person.sid=0;
	login_person.isTeacher=false;

    char * Cook_name=NULL;
    char * Cook_sid=NULL;


    getCGIdata(&datCGI);
    if(strncmp(datCGI.request_method, "POST", 4) != 0){
        printExitFailure("Use POST!");
    }
    //Aus POST_data den String zwischen <AttributName>= und '&' ausschneiden
    extractPOSTdata(datCGI.POST_data, "name", &name);
    extractPOSTdata(datCGI.POST_data, "pass", &password);
    char * isT=NULL;
    if(extractPOSTdata(datCGI.POST_data, "teach", &isT) ==0){
        if(strcmp(isT, "true") == 0){
            login_person.isTeacher=true;
        }
    }

    if(datCGI.http_cookies != NULL){
        if(extractCOOKIEdata(datCGI.http_cookies, "NAME", &Cook_name) == -1 || extractCOOKIEdata(datCGI.http_cookies, "SID", &Cook_sid) == -1){
            cookies_found=false;
        }else{
            cookies_found=true;
        }
    }

    if(name == NULL){
        printExitFailure("Name leer");
    }

    login_person.name=name;
    login_person.passwort=password;

    verifyUser(&login_person);

    //Zwei cookies setzen
    //login_person.sid=12345;
    setCookie("NAME", login_person.name);
    char sid_string[10];
    sprintf(sid_string, "%d", login_person.sid);
    setCookie("SID", sid_string);



    //Ab hier beginnt der Bereich, der an den Aufrufer übertragen wird
    //puts("Status: 301");
    //puts("Location: http://keller-pc/joke.html\n");


	puts("Content-type: text/plain\n\n");
	printf("%s\n", datCGI.POST_data);


    puts("Erhaltene Daten:\n");
	printf("CONTENT_LENGTH: %d -- REQUEST_METHOD: %s\n", datCGI.content_length, datCGI.request_method);
    printf("Name:           %s\nPassword:       %s\n", name, password);


	printf("Post Data:           %s\n", datCGI.POST_data);
	//if(datCGI.http_cookies != NULL){
	if(cookies_found){
        printf("Cook_sid: %s\nCook_name: %s", Cook_sid, Cook_name);
	}

	puts("\n\n\n\n");

	if(login_person.auth){
        printf("Personendaten:\n");
        printf("Name:     %s\n", login_person.name);
		printf("Passwort: %s (richtig)\n", login_person.passwort);
		printf("Faecher:  %s\n", login_person.courses);
		if(login_person.isTeacher)printf("Kuerzel:  %s\n", login_person.acronym);
		printf("SID:      %d\n", login_person.sid);
	}else{
		puts("YOU FAIL!!\n");
	}
	exit(0);
}

