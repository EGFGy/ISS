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

	httpHeader(TEXT);


	if(email_exists(email)){
		puts("exists\n");
	}else{
		puts("no\n");
	}

	printf("Email war: '%s'\n", email);

	return 1;

}
