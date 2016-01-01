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
	char * name=NULL;

	extractPOSTdata(&thisCGI, "name", &name);

	httpHeader(TEXT);
	if(user_exists(name)){
		puts("exists\n");
	}else{
		puts("no\n");
	}

	return 1;

}
