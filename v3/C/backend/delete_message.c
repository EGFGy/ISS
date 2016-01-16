#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <time.h>

#include "CGI_functions.h"
#include "SQL_functions.h"

//#define DEBUG

int main(int argc, char ** argv){
	cgi datCGI;
	init_CGI(&datCGI);
	person check_person;
	init_person(&check_person);

	get_CGI_data(&datCGI);
	if(datCGI.http_cookies == NULL){
		free_cgi(&datCGI);
		html_redirect_to_login();
		exit(0);
	}

	//Anhand der SID und der Email wird geprüft ob der aktuelle Benutzer angemeldet ist.
	char * s_sid=NULL;
	extract_COOKIE_data(&datCGI, "SID", &s_sid);
	extract_COOKIE_data(&datCGI, "EMAIL", &check_person.email);
	check_person.sid=atoi(s_sid);
	free(s_sid);


	if(verify_sid(&check_person)){
		//httpHeader(TEXT);
		get_person_by_sid(&check_person);
		char * message_ID_s=NULL;
		message mes;

		if(extract_QUERY_data(&datCGI, "message_ID", &message_ID_s) == 0){
			int id=atoi(message_ID_s);
			if(get_message_by_id(id, &mes)){
				if(mes.creator_id == check_person.id){
					if(delete_message_by_id(&mes)){
						print_html_error("Meldung gelöscht", "/cgi-bin/all_messages.cgi");
					}else{
						print_html_error("Meldung nicht gelöscht", "/cgi-bin/all_messages.cgi");
					}

				}else{
					char * redirectString=NULL;

					//Abhängig von der art der Nachricht wieder auf die entsprechende Seite umleiten (spec_/all_ messages)
					//asprintf(&redirectString, "https://%s/cgi-bin/%s", datCGI.http_host, strncmp(mes.courses, "all", 3) == 0 ? "all_messages.cgi" : "spec_messages.cgi",  );
					if(strncmp(mes.courses, "all", 3) == 0){
						asprintf(&redirectString, "https://%s/cgi-bin/all_messages.cgi", datCGI.http_host);
					}else{
						asprintf(&redirectString, "https://%s/cgi-bin/spec_messages.cgi?course=%s#button_%s", datCGI.http_host, mes.courses, mes.courses);
					}

					httpRedirect(redirectString);
					free(redirectString);
				}
			}else{
				print_html_error("Nachricht nicht gefunden", "/cgi-bin/all_messages.cgi"); //TODO: anpassen
			}
		}
		free(message_ID_s);
	/*
		char * redirectString=NULL;

		//Abhängig von der art der Nachricht wieder auf die entsprechende Seite umleiten (spec_/all_ messages)
		//asprintf(&redirectString, "https://%s/cgi-bin/%s", datCGI.http_host, strncmp(mes.courses, "all", 3) == 0 ? "all_messages.cgi" : "spec_messages.cgi",  );
		if(strncmp(mes.courses, "all", 3) == 0){
			asprintf(&redirectString, "https://%s/cgi-bin/all_messages.cgi", datCGI.http_host);
		}else{
			asprintf(&redirectString, "https://%s/cgi-bin/spec_messages.cgi?course=%s#button_%s", datCGI.http_host, mes.courses, mes.courses);
		}

		httpRedirect(redirectString);
		free(redirectString);
*/
		free_message(&mes);

	}else{
		char * redirectString=NULL;
		asprintf(&redirectString, "https://%s/index.html", datCGI.http_host);
		httpRedirect(redirectString);
		free(redirectString);
    }

    free_cgi(&datCGI);
    free_person(&check_person);

    exit(0);
}
