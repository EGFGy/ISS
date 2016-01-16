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


		bool all_or_spec=false; // false = all; true= spec
		char * offset=NULL; // Auf welcher Seite war er
		if(extract_QUERY_data(&datCGI, "offset", &offset) == 0){
			char * return_link=NULL;
			if(extract_QUERY_data(&datCGI, "all_messages", NULL) == 0){
				//Zurück zur Seite mit allen Meldungen
				all_or_spec=true;
				asprintf(&return_link, "/cgi-bin/all_messages.cgi?offset=%s", offset);
			}else if(extract_QUERY_data(&datCGI, "spec_messages", NULL) == 0){
				//Zu den Kursbezogene Meldungen
				char * course_s=NULL; // Bei welche mKurs war er (wenn überhaupt)
				if(extract_QUERY_data(&datCGI, "course", &course_s) == 0){
					asprintf(&return_link, "/cgi-bin/spec_messages.cgi?offset=%s&course=%s#button_%s", offset, course_s);
				}else{
					asprintf(&return_link, "/cgi-bin/spec_messages.cgi");
				}
				if(course_s)free(course_s);
				all_or_spec=false;
			}else{
				//lel
			}

			message mes;
			init_message(&mes);
			char * message_ID_s=NULL;
			if(extract_QUERY_data(&datCGI, "message_ID", &message_ID_s) == 0){
				int id=atoi(message_ID_s);
				if(get_message_by_id(id, &mes)){
					if(mes.creator_id == check_person.id){
						if(delete_message_by_id(&mes)){
							print_html_error("Meldung gelöscht", return_link);
						}else{
							print_html_error("Meldung nicht gelöscht", return_link);
						}

					}else{
						char * redirectString=NULL;

						asprintf(&redirectString, "https://%s%s", datCGI.http_host, return_link);

						httpRedirect(redirectString);
						free(redirectString);
					}
					free_message(&mes);
				}else{
					print_html_error("Meldung nicht gefunden", return_link);
				}
			}
			//free_message(&mes);
			if(message_ID_s)free(message_ID_s);
			if(return_link)free(return_link);
		}else{
			print_html_error("fehlerhafte Anfrage", "/cgi-bin/all_messages.cgi");
		}

		if(offset)free(offset);
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
