#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>

#include "CGI_functions.h"
#include "SQL_functions.h"

//#define COURSE_BOXES
#define SCROLL_MENU

int main(int argc, char ** argv){
	cgi datCGI;
	init_CGI(&datCGI);
	person check_person;
	init_person(&check_person);

	get_CGI_data(&datCGI);
	if(datCGI.http_cookies == NULL){
		//Nicht angemeldet, oder Cookies deaktiviert
		//print_exit_failure("Cookies müssen aktiv und gesetzt sein!");
		html_redirect_to_login();
		free_cgi(&datCGI);
		exit(0);
	}
	if(datCGI.request_method != GET)print_exit_failure("Use GET!");

	//Anhand der SID und der Email wird geprüft ob der aktuelle Benutzer angemeldet ist.
	char * s_sid=NULL;
	extract_COOKIE_data(&datCGI, "SID", &s_sid);
	extract_COOKIE_data(&datCGI, "EMAIL", &check_person.email);
	check_person.sid=atoi(s_sid);
	free(s_sid);

	if(verify_sid(&check_person)){
		get_person_by_sid(&check_person);
		char ** a_course=NULL;
		int number_of_courses=comma_to_array(check_person.courses, &a_course);
		char * selected_course=NULL;
		extract_QUERY_data(&datCGI, "course", &selected_course);
		char * s_offset=NULL;
		int offset=0;
		if(extract_QUERY_data(&datCGI, "offset", &s_offset) == 0){
			offset=atoi(s_offset);
			free(s_offset);
		}

		httpCacheControl("no-store, no-cache, must-revalidate, max-age=0");
		httpHeader(HTML);

		print_html_pure_head_menu("Anzeige von kursbezogenen Meldungen", "Kursbezogenes", COURSE);

		puts("<a id='scroll-top' href='#'></a>"); //"Nach-oben"-Knopf // http://icons8.com/ https://icons8.com/license/

		puts("<div class='content'>");

		if(check_person.isTeacher){
			puts("<div id='message-form'><form style='border-radius: 1em; padding: 1em;' action='/cgi-bin/post_message.cgi' method='POST' enctype='application/x-www-form-urlencoded'>\n\
			  <label style='font-weight: bold;' for='ti'>Titel</label><input style='display: block;' name='titel' id='ti' type='text' onchange='main.countLettersInThis(this); main.validateAllInput();' onkeyup='main.countLettersInThis(this); main.validateAllInput();'>\n\
			  <label style='font-weight: bold;' for='tex'>Text</label><textarea style='display: block;' name='meldung' id='tex' onchange='main.countLettersInThis(this); main.validateAllInput();' onkeyup='main.countLettersInThis(this); main.validateAllInput();'></textarea>");

			//Kursauswahl
			puts("<label style='font-weight: bold;' for='grade'>Kurs</label><br>\n\
				<select name ='kurs'>");
			for(int i=0; i<number_of_courses; i++){
				bool this_course=false;
				if(selected_course){
					if(strcmp(selected_course, *(a_course+i)) == 0)this_course=true;
				}
				printf("<option value='%s' %s>%s</option>", *(a_course+i), this_course ? "selected" : "", *(a_course+i));
			}
			puts("</select>");

			//Knopf
			puts("  <input id='submit' style='display: block; margin-left: auto; margin-right: auto;' class='submitButton' type='submit' value='Absenden'>\n\
			  </form></div>\
			");
		}

		//char * a_course=NULL;

		#ifdef SCROLL_MENU

		// Menü
		puts("<div class='pure-menu pure-menu-horizontal pure-menu-scrollable'><ul class='pure-menu-list'>");
		for(int j=0; j<number_of_courses; j++){
			bool this_course=false;
			if(selected_course){
				if(strcmp(selected_course, *(a_course+j)) == 0)this_course=true;
			}
			printf("<li id='button_%s' class='pure-menu-item course-tab %s'><a href='/cgi-bin/spec_messages.cgi?course=%s#button_%s' class='pure-menu-link'>%s</a></li>",
					*(a_course+j), this_course ? "pure-menu-selected" : "", *(a_course+j), *(a_course+j), *(a_course+j));
		}
		puts("</ul></div>");


		if(selected_course){
			// Nur wenn ein Kurs ausgewählt wurde etwas anzeigen
			char * current_course=selected_course;
			message_set a_messages;
			init_message_set(&a_messages);

			get_messages(&a_messages, offset, current_course);

			bool no_older=false;
			if(a_messages.cnt==0 && offset!=0){
				get_messages(&a_messages, offset-1, current_course);
				no_older=true;
			}

			// Kurs-Box drucken (HTML-Anzeige)
			puts("<div class='courseBox'>");
			printf("<span id='%s' style='font-weight: bold; color: white;'>%s</span>\n", current_course, current_course);
			if(a_messages.cnt>0){
				printf("<br><span style='color: white;'>Seite %d</span>\n", no_older ? offset : offset+1);

			}
			for(size_t i=0; i < a_messages.cnt; i++){
				person pers;
				init_person(&pers);
				pers.id=(a_messages.all_messages+i)->creator_id;

				puts("<div class='messageBox'>");
				printf("	<h2 class='content-subhead'>%s</h2>\n	<p>%s</p>\n", (a_messages.all_messages+i)->title, (a_messages.all_messages+i)->message);
				if(get_person_by_id(&pers)){
					printf("	<h3 style='font-size: 12px; font-style: italic;' class='message-info'>Um %s von %s %s erstellt</h3>", (a_messages.all_messages+i)->s_created, pers.first_name, pers.name );
				}else{
					printf("	<h3 style='font-size: 12px; font-style: italic;' class='message-info'>Um %s von <span style='color: red;'>gel&ouml;schtem Benutzer</span> erstellt</h3>", (a_messages.all_messages+i)->s_created);
				}

				//TODO: Button soll zur ganzen Meldung führen
				puts("<button style='border: 2px solid; border-radius: 2em; background-color: lightblue;'>MEHR</button>");
				puts("</div>"); //zu messageBox
				free_person(&pers);
			}

			if(no_older){
				puts("<div class='messageBox warningBox'><em>Keine &auml;lteren Meldungen!</em></div>");
				//offset--; //Damit man mit den Knöpfen nicht weiter schalten kann
			}

			puts("<div style='text-align: center;'>");
			if(a_messages.cnt>0){
				if(offset>0){
					printf("<a class='pure-menu-link' style='display: inline; color: white;' href='/cgi-bin/spec_messages.cgi?offset=%d&course=%s#button_%s'>&#x2770; Neuere</a>", offset-1, current_course, current_course);
					printf("<a class='pure-menu-link' style='display: inline; color: white;' href='/cgi-bin/spec_messages.cgi?course=%s#button_%s'>Neueste &#x21ef;</a>", current_course, current_course);
				}
				if(!no_older)printf("<a class='pure-menu-link' style='display: inline; color: white;' href='/cgi-bin/spec_messages.cgi?offset=%d&course=%s#button_%s'>&Auml;ltere &#x2771;</a>", offset+1, current_course, current_course);
			}
			puts("</div><br>"); //zu vor-zurück
			puts("</div>"); //zu courseBox

			free_message_set(&a_messages);
		}

		#endif // SCROLL_MENU






		#ifdef COURSE_BOXES
		for(int j=0; j<number_of_courses; j++){
			//Parameter aus QueryString extrahieren

			//Nachrichten für den Jeweiligen Kurs holen
			char * current_course=*(a_course+j);
			if(strncmp(current_course, "n/a", 3) == 0 || strlen(current_course)<2)break;

			bool is_selected_course=false;
			if(selected_course){
				if(strcmp(current_course, selected_course) == 0){
					is_selected_course=true;
				}else{
					is_selected_course=false;
				}

			}
			message_set a_messages;
			init_message_set(&a_messages);
			get_messages(&a_messages, is_selected_course ? offset :0, current_course);

			bool no_older=false;
			if(is_selected_course){
				if(a_messages.cnt==0 && offset!=0){
					get_messages(&a_messages, offset-1, current_course);
					no_older=true;
				}
			}

			// Kurs-Box drucken (HTML-Anzeige)
			puts("<div class='courseBox'>");
			printf("<span id='%s' style='font-weight: bold; color: white;'>%s</span>\n", current_course, current_course);
			if(a_messages.cnt>0){
				if(is_selected_course){
					printf("<br><span style='color: white;'>Seite %d</span>\n", no_older ? offset : offset+1);
				}else{
					puts("<br><span style='color: white;'>Seite 1</span>\n");
				}
			}
			for(size_t i=0; i < a_messages.cnt; i++){
				person pers;
				init_person(&pers);
				pers.id=(a_messages.all_messages+i)->creator_id;

				puts("<div class='messageBox'>");
				printf("	<h2 class='content-subhead'>%s</h2>\n	<p>%s</p>\n", (a_messages.all_messages+i)->title, (a_messages.all_messages+i)->message);
				if(get_person_by_id(&pers)){
					printf("	<h3 style='font-size: 12px; font-style: italic;' class='message-info'>Um %s von %s %s erstellt</h3>", (a_messages.all_messages+i)->s_created, pers.first_name, pers.name );
				}else{
					printf("	<h3 style='font-size: 12px; font-style: italic;' class='message-info'>Um %s von <span style='color: red;'>gel&ouml;schtem Benutzer</span> erstellt</h3>", (a_messages.all_messages+i)->s_created);
				}

				//TODO: Button soll zur ganzen Meldung führen
				puts("<button style='border: 2px solid; border-radius: 2em; background-color: lightblue;'>MEHR</button>");
				puts("</div>"); //zu messageBox
				free_person(&pers);
			}

			if(no_older){
				puts("<div class='messageBox warningBox'><em>Keine &auml;lteren Meldungen!</em></div>");
				//offset--; //Damit man mit den Knöpfen nicht weiter schalten kann
			}

			puts("<div style='text-align: center;'>");
			if(a_messages.cnt>0){
				if(is_selected_course){
					if(offset>0){
						printf("<a class='pure-menu-link' style='display: inline; color: white;' href='/cgi-bin/spec_messages.cgi?offset=%d&course=%s#%s'>&#x2770; Neuere</a>", offset-1, current_course, current_course);
						printf("<a class='pure-menu-link' style='display: inline; color: white;' href='/cgi-bin/spec_messages.cgi?course=%s#%s'>Neueste &#x21ef;</a>", current_course, current_course);
					}
					if(!no_older)printf("<a class='pure-menu-link' style='display: inline; color: white;' href='/cgi-bin/spec_messages.cgi?offset=%d&course=%s#%s'>&Auml;ltere &#x2771;</a>", offset+1, current_course, current_course);
				}else{
					//Es ist nicht der ausgewählte Kurs
					if(a_messages.cnt < GET_MESSAGE_COUNT){
						//keine Bedienelemente anzeigen
					}else{
						//Bedienelemente anzeigen
						printf("<a class='pure-menu-link' style='display: inline; color: white;' href='/cgi-bin/spec_messages.cgi?offset=%d&course=%s#%s'>&Auml;ltere &#x2771;</a>", 1, current_course, current_course);
					}
				}
			}
			puts("</div><br>"); //zu vor-zurück
			puts("</div>"); //zu courseBox

			free_message_set(&a_messages);
		}
		#endif  //COURSE_BOXES

		puts("</div>"); //zu Content
		//puts("<br><a style='width: 8em; color: black;' class='pure-menu-link' href='https://icons8.com/'>Quelle der Icons</a>");

		puts("<script>document.getElementById('submit').disabled=true;</script>");

		puts("</div></div>");
		puts("</body></html>");

		// ENDE
		for(int i=0;i<number_of_courses;i++){
			free(a_course[i]);
			a_course[i]=NULL;
		}
		free(selected_course);
	}else{
		//printf("Erst anmelden!!");
		char * redirectString=NULL;
		asprintf(&redirectString, "https://%s/index.html", datCGI.http_host);
		httpCacheControl("no-store, no-cache, must-revalidate, max-age=0");
		httpRedirect(redirectString);
		free(redirectString);
	}
	free_cgi(&datCGI);
	free_person(&check_person);
	exit(0);
}
