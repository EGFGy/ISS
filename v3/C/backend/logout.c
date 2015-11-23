#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>

#include "CGI_functions.h"
#include "SQL_functions.h"

int main(int argc, char ** argv){
	cgi datCGI;
	init_CGI(&datCGI);

	person logout_person;
	init_person(&logout_person);

	char * s_sid=NULL;
	get_CGI_data(&datCGI);
	if(datCGI.http_cookies == NULL){
		print_html_error("Anscheinend waren sie noch nie angemeldet oder es ist eine Fehler aufgetreten", "/index.html");
		exit(0);
	}

	extract_COOKIE_data(&datCGI, "SID", &s_sid);
	extract_COOKIE_data(&datCGI, "EMAIL", &logout_person.email);
	logout_person.sid=atoi(s_sid);
	free(s_sid);

	httpCacheControl("no-store, no-cache, must-revalidate, max-age=0");

	if(!(logout_person.sid == 0 && strcmp(logout_person.email, "NULL")==0)){

		if(sid_exists(logout_person.sid)){
			httpSetCookie("EMAIL", "NULL");
			httpSetCookie("SID", "0");
			//httpHeader(TEXT);
			if(sid_set_null(&logout_person)){
				//printf("Logout erfolgreich\n");
				char * url=NULL;
				asprintf(&url, "https://%s/index.html", datCGI.http_host);
				httpRedirect(url);
				free(url);
			}else{
				httpHeader(TEXT);
				printf("Fehler in sid_set_null\n");
			}
		}else{
			print_exit_failure("Bereits abgemeldet");
		}
	}else{
		print_exit_failure("Bereits abgemeldet (cookies waren schon 0 oder \"NULL\")");
	}

	free_cgi(&datCGI);
	free_person(&logout_person);
	exit(0);
}
