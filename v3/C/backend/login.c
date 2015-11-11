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

/**
Benutzer anmelden (Passwort Überprüfen)

*/
int main(int argc, char ** argv)
{

	cgi datCGI;
	init_CGI(&datCGI);
	person login_person;
	init_person(&login_person);

	//fprintf(stderr, "Hallo vor Post\n");
	get_CGI_data(&datCGI);

	if(datCGI.request_method != POST){
		print_exit_failure("Use POST!");
	}

	//fprintf(stderr, "POST_DATA: %s", datCGI.POST_data);

	//Aus POST_data den String zwischen <AttributName>= und '&' ausschneiden
	extract_POST_data(&datCGI, "email", &login_person.email);

	remove_newline(login_person.email);
	extract_POST_data(&datCGI, "pass", &login_person.password);
	remove_newline(login_person.password);


	if(login_person.email == NULL){
		httpSetCookie("EMAIL", "NULL");
		httpSetCookie("SID", "0");
		httpCacheControl("no-cache");
		char * redirectString=NULL;
		asprintf(&redirectString, "https://%s/incorrect_password.html", datCGI.http_host);
		httpRedirect(redirectString);
	}

	//fprintf(stderr, "POST_DATA: %s", datCGI.POST_data);
	//TODO: Verhindern, dass sich ein anderer Nutzer vom selben Rechner aus einloggt wenn der erste noch nicht abgemeldet ist
	//(zweimaliges Anmelden verhindern)

	//Das ist sehr unwahrscheinlich
	/*
	if(datCGI.http_cookies != NULL){
		person already_logged_in_person;
		init_person(&already_logged_in_person);

		char * cook_sid=NULL;
		if(extract_COOKIE_data(&datCGI, "EMAIL", &already_logged_in_person.email) == 0 && extract_COOKIE_data(&datCGI, "SID", &cook_sid) == 0){
			//print_exit_failure("Hier ist schon jemand eingeloggt");
			already_logged_in_person.sid=atoi(cook_sid);

			if(get_person_by_sid(&already_logged_in_person)){
				print_exit_failure("Hier ist schon jemand eingeloggt");
			}
		}
	}*/
	UserState user_state=verify_user(&login_person);

	//Zwei cookies setzen
	if(user_state == PW_CORRECT || user_state == PW_CORRECT_ALREADY_LOGGED_IN){
		httpSetCookie("EMAIL", login_person.email);
		char * sid_string;
		asprintf(&sid_string, "%d", login_person.sid);
		httpSetCookie("SID", sid_string);

		httpCacheControl("no-store, no-cache, must-revalidate, max-age=0");

		char * redirectString=NULL;
		asprintf(&redirectString, "https://%s/cgi-bin/all_messages.cgi", datCGI.http_host);
		httpRedirect(redirectString);
	}
	if(user_state == PW_INCORRECT){
		httpSetCookie("EMAIL", "NULL");
		httpSetCookie("SID", "0");
		httpCacheControl("no-store, no-cache, must-revalidate, max-age=0");
		char * redirectString=NULL;
		asprintf(&redirectString, "https://%s/incorrect_password.html", datCGI.http_host);
		httpRedirect(redirectString);

	}
/*
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
	</html>");*/

	exit(0);
}

