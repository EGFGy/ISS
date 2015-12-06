#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>

#include "CGI_functions.h"
#include "SQL_functions.h"

typedef struct{
	message * all_messages;
	int count;
}message_set;

void free_message_set(message_set * m){
	if(m){
		if(m->count > 0 && m->all_messages){
			for(int i=0; i<m->count; i++){
				free_message((m->all_messages+i));
			}
			free(m->all_messages);
		}
	}
}

int main(int argc, char ** argv){
    cgi datCGI;
    init_CGI(&datCGI);
	person check_person;
	init_person(&check_person);
	message_set all_messages; //TODO vielleicht eine message_set Struktur machen die die Meldungen und die Anzahl enthält um diese danach zu löschen
	int number=0; //Zahl um die tatsächliche Anzahl an Meldungen zu speichern
	int offset=0; //Vom Nutzer gewünschter Offset

    get_CGI_data(&datCGI);
	if(datCGI.http_cookies == NULL){
		//Nicht angemeldet, oder Cookies deaktiviert
		//print_exit_failure("Cookies müssen aktiv und gesetzt sein!");
		html_redirect_to_login();
		/**
		* Ab jetzt sind die cookies möglicherweise auf 0 bzw. NULL gesetzt
		* --> Wenn der Nutzer nochmal unangemeldet hier her kommt wird er gleich umgeleitet
		*     (Siehe 'UMLEITUNG')
		*/
		free_cgi(&datCGI);
		exit(0);
	}

	if(datCGI.request_method != GET)print_exit_failure("Use GET!");

	//Anhand der SID und der E-Mail-Adresse wird geprüft ob der aktuelle Benutzer angemeldet ist.
	char * s_sid=NULL;
	extract_COOKIE_data(&datCGI, "SID", &s_sid);
	extract_COOKIE_data(&datCGI, "EMAIL", &check_person.email);
	check_person.sid=atoi(s_sid);
	free(s_sid);


    if(verify_sid(&check_person)){
		/** Restliche Informationen über die aktuelle Person in Erfahrung bringen */
		get_person_by_sid(&check_person);



		char * s_offest=NULL;
		if(extract_QUERY_data(&datCGI, "offset", &s_offest) != -1){
			offset=atoi(s_offest);
		}
		if(s_offest)free(s_offest);s_offest=NULL;

		//all_messages=get_messages(&number, offset);
		all_messages.count=get_messages(&all_messages.all_messages, offset, NULL);
		bool no_older=false;
		if(all_messages.count==0 && offset!=0){
			//all_messages=get_messages(&number, offset-1);
			all_messages.count=get_messages(&all_messages.all_messages, offset-1, NULL);
			no_older=true;
		}

		/** HTTP + HTML ab hier */
		httpCacheControl("no-store, no-cache, must-revalidate, max-age=0");
		httpHeader(HTML);

		print_html_pure_head_menu("Anzeige von allgemeinen Meldungen", "Allgemeines", MAIN);
		puts("<a id='scroll-top' href='#'></a>"); //"Nach-oben"-Knopf // http://icons8.com/ https://icons8.com/license/
		//Nachrichten ab hier
		puts("<div class='content'>");

		/** Wenn ein Lehrer die Seite aufruft, das Eingabefeld anzeigen*/
        if(check_person.isTeacher)puts("<div id='message-form'><form style='border-radius: 1em; padding: 1em;' action='/cgi-bin/post_message.cgi' method='POST' enctype='application/x-www-form-urlencoded'>\n\
			  <label style='font-weight: bold;' for='ti'>Titel</label><input style='display: block;' name='titel' id='ti' type='text' onchange='main.countLettersInThis(this); main.validateAllInput();' onkeyup='main.countLettersInThis(this); main.validateAllInput();'>\n\
			  <label style='font-weight: bold;' for='tex'>Text</label><textarea style='display: block;' name='meldung' id='tex' onchange='main.countLettersInThis(this); main.validateAllInput();' onkeyup='main.countLettersInThis(this); main.validateAllInput();'></textarea>\n\
			  <input id='submit' style='display: block; margin-left: auto; margin-right: auto;' class='submitButton' type='submit' value='Absenden'>\n\
			  </form></div>\
			");

		printf("<span>Seite %d</span>\n", no_older ? offset : offset+1);

		/** Die einzelnen Meldungen der Reihe nach auflisten*/
		for(int i=0; i<number; i++){
			person pers;
			init_person(&pers);
			pers.id=(all_messages.all_messages+i)->creator_id; // TODO Check


			puts("<div class='messageBox'>");
			printf("	<h2 class='content-subhead'>%s</h2>\n	<p>%s</p>\n", (all_messages.all_messages+i)->title, (all_messages.all_messages+i)->message);

			/** Ersteller und Uhrzeit zu der die Meldung verfasst wurde anzeigen*/
			if(get_person_by_id(&pers)){
				printf("	<h3 style='font-size: 12px; font-style: italic;' class='message-info'>Um %s von %s %s erstellt</h3>", (all_messages.all_messages+i)->s_created, pers.first_name, pers.name );
			}else{
				printf("	<h3 style='font-size: 12px; font-style: italic;' class='message-info'>Um %s von <span style='color: red;'>gel&ouml;schtem Benutzer</span> erstellt</h3>", (all_messages.all_messages+i)->s_created);
			}

			//TODO: Button soll zur ganzen Meldung führen
			puts("<button style='border: 2px solid; border-radius: 2em; background-color: lightblue;'>MEHR</button>");
			puts("</div>");
			free_person(&pers);
		}

		if(no_older){
			puts("<div class='messageBox warningBox'><em>Keine &auml;lteren Meldungen!</em></div>");
			offset--; //Damit man mit den Knöpfen nicht weiter schalten kann
		}

		puts("</div>"); //zu Content
		puts("<div style='text-align: center;'>");


		/** Navigationstasten */
		if(offset>0){
			printf("<a class='pure-menu-link' style='display: inline; color: black;' href='/cgi-bin/all_messages.cgi?offset=%d'>&#x2770; Neuere</a>", offset-1);
			puts("<a class='pure-menu-link' style='display: inline; color: black;' href='/cgi-bin/all_messages.cgi'>Neueste &#x21ef;</a>");

		}
		if(!no_older)printf("<a class='pure-menu-link' style='display: inline; color: black;' href='/cgi-bin/all_messages.cgi?offset=%d'>&Auml;ltere &#x2771;</a>", offset+1);
		puts("</div><br>");
		puts("<br><a style='width: 8em; color: black;' class='pure-menu-link' href='https://icons8.com/'>Quelle der Icons</a>");


		puts("<!-- WÖRKARAUND für js-onload -->\n\
				<img src='/img/Arrow-Download-4-icon.png' style='display: none; width: 0px; heigth: 0px;' onload=\"document.getElementById('submit').disabled=true;\">");


		puts("</div></div>");
		puts("</body></html>");
    }else{
		// UMLEITUNG
		char * redirectString=NULL;
		asprintf(&redirectString, "https://%s/index.html", datCGI.http_host);
		httpCacheControl("no-store, no-cache, must-revalidate, max-age=0");
		httpRedirect(redirectString);
		free(redirectString);
    }

    free_cgi(&datCGI);
    free_person(&check_person);
    free_message_set(&all_messages);
	exit(0);
}
