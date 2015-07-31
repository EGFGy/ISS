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
    init_CGI(&datCGI);
	person check_person;
	init_person(&check_person);
	message * all_messages;
	int number=0; //Zahl um die tatsächliche Anzahl an Meldungen zu speuchern
	int offset=0; //Vom nutzer gewünschter offset

    get_CGI_data(&datCGI);
	if(datCGI.http_cookies == NULL)print_exit_failure("Cookies müssen aktiv und gesetzt sein!");

	//Anhand der SID und der Email wird geprüft ob der aktuelle Benutzer angemeldet ist.
	char * s_sid=NULL;
    extract_COOKIE_data(&datCGI, "SID", &s_sid);
    extract_COOKIE_data(&datCGI, "EMAIL", &check_person.email);
    check_person.sid=atoi(s_sid);



    if(verify_sid(&check_person)){
		httpHeader(HTML);

		char * s_offest=NULL;
		if(extract_QUERY_data(&datCGI, "offset", &s_offest) != -1){
			offset=atoi(s_offest);
		}
		all_messages=get_messages(&number, offset);
		bool no_older=false;
		if(number==0 && offset!=0){
			all_messages=get_messages(&number, offset-1);
			no_older=true;
		}

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
		puts("<div class='content'>");

		puts("<form action='/cgi-bin/post_message.cgi' method='POST'> \
                <input style='display: block;' type='text' name='titel'>\
                <textarea style='display: block;' cols='25' rows='3' name='meldung'></textarea>\
                <input style='display: block;' type='submit'> \
        </form>");

		printf("<span>Seite %d</span>", no_older ? offset : offset+1);
		//TODO: Umlaute!!!
		for(int i=0; i<number; i++){
			person * pers=get_person_by_id((all_messages+i)->creator_id);

			puts("<div class='messageBox'>");
			printf("<h2 class='content-subhead'>%s</h2>\n<p>%s</p>\n", (all_messages+i)->title, (all_messages+i)->message);
			if(pers != NULL)printf("<h3 style='font-size: 12px; font-style: italic;' class='message-info'>Um %s von %s %s erstellt</h3>", (all_messages+i)->s_created, pers->first_name, pers->name );

			puts("<button style='border: 2px solid; border-radius: 2em; background-color: lightblue;'>MEHR</button>");
			puts("</div>");
		}

		if(no_older){
			puts("<div class='messageBox warningBox'><em>Keine &auml;lteren Meldungen!</em></div>");
			offset--;
		}

		puts("</div>");
		puts("<div style='text-align: center;'>");

		if(offset>0){
			puts("<a class='pure-menu-link' style='display: inline;' href='/cgi-bin/all_messages.cgi'>Neueste</a>");
			printf("<a class='pure-menu-link' style='display: inline;' href='/cgi-bin/all_messages.cgi?offset=%d'>Neuere</a>", offset-1);
		}
		if(!no_older)printf("<a class='pure-menu-link' style='display: inline;' href='/cgi-bin/all_messages.cgi?offset=%d'>&Auml;ltere</a>", offset+1);
		puts("</div>");

		puts("</div></div>");
		puts("</body></html>");
		//printf("Daten dürfen angesehen werden!!\n");
		//printf("SID: %d\n", check_person.sid);
    }else{
		//printf("Erst anmelden!!");
		char * redirectString=NULL;
		asprintf(&redirectString, "https://%s/index.html", datCGI.http_host);
		httpRedirect(redirectString);

    }
    //Prüfen ob Nutzer angemeldet ist
	return 0;
}
