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

int main(int argc, char ** argv){
    cgi datCGI;
    getCGIdata(&datCGI);
    if(strncmp(datCGI.request_method, "POST", 4) != 0){
        printExitFailure("Use POST!");
    }

    //Für die Namen: siehe HTML-Dokument mit entsprechenden <input>-Elementen
    char * name=NULL;
    char * acronym=NULL;
    char * teach=NULL;
    char * pass=NULL;
    char * acceptTOS=NULL;
    extractPOSTdata(datCGI.POST_data, "name", &name);
    extractPOSTdata(datCGI.POST_data, "pass", &pass);
    extractPOSTdata(datCGI.POST_data, "acronym", &acronym);
    extractPOSTdata(datCGI.POST_data, "teach", &teach);
    extractPOSTdata(datCGI.POST_data, "acceptTOS", &acceptTOS);


    puts("Content-type: text/plain\n\n");
	printf("%s\n", datCGI.POST_data);


    puts("Erhaltene Daten:\n");
	printf("CONTENT_LENGTH: %d -- REQUEST_METHOD: %s\n", datCGI.content_length, datCGI.request_method);
    printf("Name:           %s\nPassword:       %s\n", name, pass);
    printf("Kuerzel:        %s\nTeach:          %s\n", acronym, teach);
    printf("accepted TOS:   %s\n\n", acceptTOS);


	printf("Post Data:      %s\n", datCGI.POST_data);


	exit(0);
}
