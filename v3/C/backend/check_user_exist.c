#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>

#include "CGI_functions.h"
#include "SQL_functions.h"

/**
Prüfen ob ein E-Mail in der Datenbank existiert.
- für Registrierungsformular
- Javascript führt Anfrage aus --> Auswertung der Antwort im Browser
  --> dem Benutzer wir eine Warnung angezeigt, wenn die E-Mail schon in der DB existiert.
*/
int main(int argc, char ** argv){
	cgi thisCGI;
	init_CGI(&thisCGI);

	get_CGI_data(&thisCGI);
	char * email=NULL;

	extract_POST_data(&thisCGI, "email", &email);
	remove_newline(email);

	httpHeader(TEXT);
	if(email_exists(email)){
		puts("exists\n");
	}else{
		puts("no\n");
	}

	printf("Email war: '%s'\n", email);

	return 1;

}
