/*
 * File:   main_strtok_base3.c
 * Author: Quwert (Mr)
 *
 * Created on 25. Mai 2015, 13:50
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
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


/**
Benutzer anmelden (Passwort Überprüfen)

*/
int main(int argc, char ** argv)
{
	cgi datCGI;
	person login_person;

	login_person.email=NULL;
	login_person.acronym=NULL;
	login_person.password=NULL;
	login_person.auth=false;
	login_person.courses=NULL;
	login_person.sid=0;
	login_person.isTeacher=false;


	getCGIdata(&datCGI);
	if(strncmp(datCGI.request_method, "POST", 4) != 0){
		printExitFailure("Use POST!");
	}
	//Aus POST_data den String zwischen <AttributName>= und '&' ausschneiden
	extractPOSTdata(&datCGI, "email", &login_person.email);
	extractPOSTdata(&datCGI, "pass", &login_person.password);


	if(login_person.email == NULL){
		printExitFailure("Name leer");
	}

	int user_state=verifyUser(&login_person);

	//Zwei cookies setzen

	setCookie("EMAIL", login_person.email);
	char * sid_string;
	asprintf(&sid_string, "%d", login_person.sid);
	setCookie("SID", sid_string);



	//Ab hier beginnt der Bereich, der an den Aufrufer übertragen wird

	httpHeader(HTML);
	printf("<!DOCTYPE html><head>\
		<title>InfoWall -- Anmeldung</title>\
		<meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\" />\
		<meta name=\"viewport\" content=\"width=device-width\">\
	</head>\
	<body>");

	printf("%s\n", datCGI.POST_data);


	puts("<h1>Erhaltene Daten:</h1>\n");
	printf("<br>CONTENT_LENGTH: %d -- REQUEST_METHOD: %s\n", datCGI.content_length, datCGI.request_method);
	printf("<br>Name:           %s\nPassword:       %s\n", login_person.email, login_person.password);


	printf("<br>Post Data:           %s\n", datCGI.POST_data);

	puts("<br>\n\n\n\n");

	if(login_person.auth && user_state==0){
		puts("<h2>Personendaten:</h2>\n");
		printf("<br>User ID:   %d\n", login_person.id);
		printf("<br>Vorname:   %s\n", login_person.first_name);
		printf("<br>Nachname:  %s\n", login_person.name);
		printf("<br>Email:     %s\n", login_person.email);
		printf("<br>Passwort:  %s (richtig)\n", login_person.password);
		printf("<br>Faecher:   %s\n", login_person.courses);
		if(login_person.isTeacher)printf("<br>Kuerzel:   %s\n", login_person.acronym);
		printf("<br>SID:       %d\n", login_person.sid);

		puts("<a href=\"/cgi-bin/logout.cgi\" style=\"color: green;\">LOGOUT</a>\
         <br><a href=\"/cgi-bin/all_messages.cgi\">Alle Nachrichten</a>");
         puts("<iframe src=\"/cgi-bin/all_messages.cgi\" style=\"width: 100%; height: 500px;\"");
	}else{
		puts("<br>YOU FAIL!!\n");
		if(user_state == 1){
            puts("Bereits angemeldet!");
            printf("<a href=\"/cgi-bin/logout.cgi\">LOGOUT</a>\
         <br><a href=\"/cgi-bin/all_messages.cgi\">Alle Nachrichten</a>");
		}
	}

	printf("</body>\
	</html>");

	exit(0);
}

