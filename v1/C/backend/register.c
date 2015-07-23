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
	person reg_person;
	reg_person.first_name=NULL;
	reg_person.name=NULL;
	reg_person.email=NULL;
	reg_person.acronym=NULL;
	char * teach=NULL;
	reg_person.password=NULL;
	reg_person.auth=false;
	reg_person.sid=0;
	char * acceptTOS=NULL;
	extractPOSTdata(&datCGI, "name_vor", &reg_person.first_name);
	extractPOSTdata(&datCGI, "name", &reg_person.name);
	extractPOSTdata(&datCGI, "email", &reg_person.email);
	extractPOSTdata(&datCGI, "pass", &reg_person.password);
	extractPOSTdata(&datCGI, "acronym", &reg_person.acronym);
	extractPOSTdata(&datCGI, "teach", &teach);
	extractPOSTdata(&datCGI, "acceptTOS", &acceptTOS);
	//TODO: fehlerhaften Aufruf abfangen
	if(strcmp(teach, "true") == 0){
		reg_person.isTeacher=true;
	}else{
		reg_person.isTeacher=false;
	}

	fprintf(stderr, "\nnow comes da htmlz\n");
	insertUser(&reg_person);


	httpHeader(TEXT);
	printf("%s\n", datCGI.POST_data);


	puts("Erhaltene Daten:\n");
	printf("CONTENT_LENGTH: %d -- REQUEST_METHOD: %s\n", datCGI.content_length, datCGI.request_method);
	printf("Name:           %s\nPassword:       %s\n", reg_person.name, reg_person.password);
	printf("Kuerzel:        %s\nTeach:          %s\n", reg_person.acronym, teach);
	printf("accepted TOS:   %s\n\n", acceptTOS);


	printf("Post Data:      %s\n", datCGI.POST_data);


	exit(0);
}
