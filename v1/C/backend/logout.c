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
	person logout_person;

	char * s_sid=NULL;
    getCGIdata(&datCGI);
    if(datCGI.http_cookies == NULL)printExitFailure("Cookies müssen aktiv und gesetzt sein! Anmeldung wie? SID wie?");

    extractCOOKIEdata(&datCGI, "SID", &s_sid);
    extractCOOKIEdata(&datCGI, "EMAIL", &logout_person.email);
    logout_person.sid=atoi(s_sid);

    if(!(logout_person.sid == 0 && strcmp(logout_person.email, "NULL")==0)){

		if(sid_exists(logout_person.sid)){
			setCookie("EMAIL", "NULL");
			setCookie("SID", "0");
			httpHeader(TEXT);
			if(sid_set_null(&logout_person)){
				printf("Logout erfolgreich\n");
			}else{
				printf("Fehler in sid_set_null\n");
			}

		}else{
			printExitFailure("Bereits abgemeldet");
		}
    }else{
		printExitFailure("Bereits abgemeldet (cookies waren schon 0 oder \"NULL\")");
	}
}
