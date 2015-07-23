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
	person check_person;
	message * all_messages;
	int number=0;

	printf("Hello wörld\n");

	char * s_sid=NULL;
    getCGIdata(&datCGI);
	if(datCGI.http_cookies == NULL)printExitFailure("Cookies müssen aktiv und gesetzt sein!");
    extractCOOKIEdata(&datCGI, "SID", &s_sid);
    extractCOOKIEdata(&datCGI, "EMAIL", &check_person.email);
    check_person.sid=atoi(s_sid);

	httpHeader(HTML);

    if(verify_sid(&check_person)){
		all_messages=get_all_messages(&number);

		FILE * html_source;
		html_source=fopen("/usr/share/nginx/html/cgi-bin/resources/main.text", "r");
		if(html_source){
			char c=NULL;
			while((c = getc(html_source)) != EOF){
				putchar(c);
			}
			fclose(html_source);
		}
		//Nachrichten ab hier
		puts("<div class=\"content\">");
		puts("<div class='messageBox'><h2 class=\"content-subhead\">Hallo B&uuml;l</h2>\n<p>InfoText</p>\n");
		puts("<button style='border: 2px solid; border-radius: 2em; background-color: lightblue;'>MEHR</button>");
		puts("</div>");
		//TODO: Umlaute!!!
		for(int i=0; i<number; i++){
			printf("<div class='messageBox'><h2 class=\"content-subhead\">%s</h2>\n<p>%s</p>\n", (all_messages+i)->title, (all_messages+i)->message);
			puts("<button style='border: 2px solid; border-radius: 2em; background-color: lightblue;'>MEHR</button>");
			puts("</div>");
		}

		puts("</div></div></div>");
		puts("</body></html>");
		//printf("Daten dürfen angesehen werden!!\n");
		//printf("SID: %d\n", check_person.sid);
    }else{
		printf("Erst anmelden!!");
    }



    //Prüfen ob Nutzer angemeldet ist
	return 0;
}
