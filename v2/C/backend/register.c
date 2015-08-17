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
	init_CGI(&datCGI);
	get_CGI_data(&datCGI);
	if(datCGI.request_method == GET){
		print_exit_failure("Use POST!");
	}

	person reg_person;
	init_person(&reg_person);
	char * teach=NULL;
	char * acceptTOS=NULL;
	//Für die Namen: siehe HTML-Dokument mit entsprechenden <input>-Elementen
	extract_POST_data(&datCGI, "name_vor", &reg_person.first_name);
	remove_newline(reg_person.first_name);
	extract_POST_data(&datCGI, "name", &reg_person.name);
	remove_newline(reg_person.name);
	extract_POST_data(&datCGI, "email", &reg_person.email);
	remove_newline(reg_person.email);
	extract_POST_data(&datCGI, "pass", &reg_person.password);
	remove_newline(reg_person.password);
	extract_POST_data(&datCGI, "acronym", &reg_person.acronym);
	remove_newline(reg_person.acronym);
	extract_POST_data(&datCGI, "teach", &teach);
	remove_newline(teach);
	extract_POST_data(&datCGI, "acceptTOS", &acceptTOS);
	remove_newline(acceptTOS);
	//TODO: fehlerhaften Aufruf abfangen
	if(strcmp(teach, "true") == 0){
		reg_person.isTeacher=true;
	}else{
		reg_person.isTeacher=false;
	}

	fprintf(stderr, "\nnow comes da htmlz\n");
	insert_user(&reg_person);

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
