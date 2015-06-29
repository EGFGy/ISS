#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>

#include "CGI_functions.h"
#include "SQL_functions.h"

//name=fhfhiu&teach=true&acronym=FFF&pass=weew&acceptTOS=on (58)

int main(int argc, char ** argv){
	cgi datCGI;
	getCGIdata(&datCGI);
	if(strncmp(datCGI.request_method, "POST", 4) != 0){
		printExitFailure("Use POST!");
	}

	fprintf(stderr, "Hello, welcome\n");

	//Für die Namen: siehe HTML-Dokument mit entsprechenden <input>-Elementen
	char * name=NULL;
	char * acronym=NULL;
	char * teach=NULL;
	char * pass=NULL;
	char * acceptTOS=NULL;
	extractPOSTdata(&datCGI, "name", &name);
	extractPOSTdata(&datCGI, "pass", &pass);
	extractPOSTdata(&datCGI, "acronym", &acronym);
	extractPOSTdata(&datCGI, "teach", &teach);
	extractPOSTdata(&datCGI, "acceptTOS", &acceptTOS);
	person reg_person;
	reg_person.name=name;
	reg_person.passwort=pass;
	reg_person.acronym=acronym;
	if(strcmp(teach, "true") == 0){
		reg_person.isTeacher=true;
	}else{
		reg_person.isTeacher=false;
	}

	fprintf(stderr, "\nnow comes da htmlz\n");
	insertUser(&reg_person);


	httpHeader(TEXT);
	//puts("Content-type: text\/plain\n\n\TEST");
	//printf("FAAAAAAACK!!!\n\n\n\n");
	//puts("Content-type: text/plain\n\n");
	printf("%s\n", datCGI.POST_data);


	puts("Erhaltene Daten:\n");
	printf("CONTENT_LENGTH: %d -- REQUEST_METHOD: %s\n", datCGI.content_length, datCGI.request_method);
	printf("Name:           %s\nPassword:       %s\n", name, pass);
	printf("Kuerzel:        %s\nTeach:          %s\n", acronym, teach);
	printf("accepted TOS:   %s\n\n", acceptTOS);


	printf("Post Data:      %s\n", datCGI.POST_data);


	exit(0);
}
