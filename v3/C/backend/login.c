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

	UserState user_state=verify_user(&login_person);

	//Zwei cookies setzen
	if(user_state == PW_CORRECT || user_state == PW_CORRECT_ALREADY_LOGGED_IN){
		httpSetCookie("EMAIL", login_person.email);
		char * sid_string;
		asprintf(&sid_string, "%d", login_person.sid);
		httpSetCookie("SID", sid_string);
		free(sid_string);

		httpCacheControl("no-store, no-cache, must-revalidate, max-age=0");

		char * redirectString=NULL;
		asprintf(&redirectString, "https://%s/cgi-bin/all_messages.cgi", datCGI.http_host);
		httpRedirect(redirectString);
		free(redirectString);
	}
	if(user_state == PW_INCORRECT){
		httpSetCookie("EMAIL", "NULL");
		httpSetCookie("SID", "0");
		httpCacheControl("no-store, no-cache, must-revalidate, max-age=0");
		char * redirectString=NULL;
		asprintf(&redirectString, "https://%s/incorrect_password.html", datCGI.http_host);
		httpRedirect(redirectString);

	}

	free_cgi(&datCGI);
	free_person(&login_person);

	exit(0);
}

