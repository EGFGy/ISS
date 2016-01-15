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
		message mes;
		init_message(&mes);

		mes.creator_id=check_person.id;

		extract_POST_data(&datCGI, "meldung", &mes.message);
		extract_POST_data(&datCGI, "titel", &mes.title);
		if(extract_POST_data(&datCGI, "kurs", &mes.courses) == -1){ // TODO: Was tun, wenn falls ein Kurs ausgewählt ist, den es nicht gibt (manuelle Eingabe) ???
			asprintf(&mes.courses, "all");
		}
		#ifdef DEBUG
		fprintf(stderr, "\n\nPOST_DATA: '%s'\n\n", datCGI.POST_data);
		#endif // DEBUG

		time_t now=time(NULL);
		mes.created=localtime(&now);

		#ifdef DEBUG
		fprintf(stderr, "\n\nTitel: '%s',\nNachricht: '%s'\n\n\nend", mes.title, mes.message);
		#endif // DEBUG

		insert_message(&mes);

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
