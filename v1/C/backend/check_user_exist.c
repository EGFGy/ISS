#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>

#include "CGI_functions.h"
#include "SQL_functions.h"


int main(int argc, char ** argv){
	cgi thisCGI;

	getCGIdata(&thisCGI);
	char * email=NULL;

	extractPOSTdata(&thisCGI, "email", &email);
	char * convertedMail=calloc(strlen(email)+1, sizeof(char));
	//convertHEXtoAscii(email, &convertedMail);
	decodeHEX(email, convertedMail);

	printf("Email: %s\nconv. Email '%s'\n\n", email, convertedMail);

	if(email_exists(convertedMail)){
		puts("exists\n");
	}else{
		puts("no\n");
	}

	printf("Email war: '%s'\n", convertedMail);

	return 1;

}
