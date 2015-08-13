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
	if(datCGI.http_cookies == NULL){
		//Nicht angemeldet, oder Cookies deaktiviert
		//print_exit_failure("Cookies müssen aktiv und gesetzt sein!");
		httpSetCookie("EMAIL", "NULL");
		httpSetCookie("SID", "0");
		httpHeader(HTML);
        print_html_head("Benutzung von Cookeis", "Cookies");
        puts("<body>Cookies müssen aktiv und gesetzt sein!<br>");
		printf("<a href=https://%s/index.html>ZUR&Uuml; zur Anmeldung</a>", datCGI.http_host);
		exit(0);
	}
	if(strncmp(datCGI.request_method, "GET", 3) != 0)print_exit_failure("Use GET!");

	//Anhand der SID und der Email wird geprüft ob der aktuelle Benutzer angemeldet ist.
	char * s_sid=NULL;
    extract_COOKIE_data(&datCGI, "SID", &s_sid);
    extract_COOKIE_data(&datCGI, "EMAIL", &check_person.email);
    check_person.sid=atoi(s_sid);


    if(verify_sid(&check_person)){
		get_person_by_sid(&check_person);
		httpCacheControl("no-store, no-cache, must-revalidate, max-age=0");
		httpHeader(HTML);

		char * s_offest=NULL;
		if(extract_QUERY_data(&datCGI, "offset", &s_offest) != -1){
			offset=atoi(s_offest);
		}
		//all_messages=get_messages(&number, offset);
		number=get_messages(&all_messages, offset);
		bool no_older=false;
		if(number==0 && offset!=0){
			//all_messages=get_messages(&number, offset-1);
			number=get_messages(&all_messages, offset-1);
			no_older=true;
		}

		/*FILE * html_source;
		html_source=fopen("/usr/share/nginx/html/cgi-bin/resources/main.text", "r");
		if(html_source){
			char c=NULL;
			while((c = getc(html_source)) != EOF){
				putchar(c);
			}
			fclose(html_source);
		}*/

		print_html_pure_head_menu("Anzeige von allgemeinen Meldungen", "Allgemeines", MAIN);

		//Nachrichten ab hier
		puts("<div class='content'>");

        if(check_person.isTeacher)puts("<div id='login-form'><form style='border-radius: 1em; padding: 1em;' action='/cgi-bin/post_message.cgi' method='POST'>\n\
			  <label for='ti'>Titel</label><input class='textIn' style='display: block;' name='titel' id='ti' type='text'>\n\
			  <label for='tex'>Text</label><textarea class='textIn' style='display: block; height: 88px; width: 427px;' name='meldung' id='tex'></textarea>\n\
			  <input class='submitButton' style='display: block;' type='submit'>\n\
			  </form></div>\
			");

		printf("<span>Seite %d</span>", no_older ? offset : offset+1);
		//TODO: Umlaute!!!
		for(int i=0; i<number; i++){
			person pers;
			init_person(&pers);
			pers.id=(all_messages+i)->creator_id;


			puts("<div class='messageBox'>");
			printf("<h2 class='content-subhead'>%s</h2>\n<p>%s</p>\n", (all_messages+i)->title, (all_messages+i)->message);
			if(get_person_by_id(&pers)){
				printf("<h3 style='font-size: 12px; font-style: italic;' class='message-info'>Um %s von %s %s erstellt</h3>", (all_messages+i)->s_created, pers.first_name, pers.name );
			}else{
				printf("<h3 style='font-size: 12px; font-style: italic;' class='message-info'>Um %s von <span style='color: red;'>gel&ouml;schtem Benutzer</span> erstellt</h3>", (all_messages+i)->s_created);
			}

			//TODO: Button soll zur ganzen Meldung führen
			puts("<button style='border: 2px solid; border-radius: 2em; background-color: lightblue;'>MEHR</button>");
			puts("</div>");
		}

		if(no_older){
			puts("<div class='messageBox warningBox'><em>Keine &auml;lteren Meldungen!</em></div>");
			offset--; //Damit man mit den Knöpfen nicht weiterschalten kann
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
    }else{
		//printf("Erst anmelden!!");
		char * redirectString=NULL;
		asprintf(&redirectString, "https://%s/index.html", datCGI.http_host);
		httpCacheControl("no-store, no-cache, must-revalidate, max-age=0");
		httpRedirect(redirectString);

    }
	exit(0);
}
