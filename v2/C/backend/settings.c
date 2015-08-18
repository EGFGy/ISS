#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <time.h>

#include "CGI_functions.h"
#include "SQL_functions.h"


int main(int argc, char ** argv){
    cgi datCGI;
    init_CGI(&datCGI);
	person check_person;
	init_person(&check_person);

	get_CGI_data(&datCGI);
	if(datCGI.http_cookies == NULL)print_exit_failure("Cookies müssen aktiv und gesetzt sein!");

	//Anhand der SID und der Email wird geprüft ob der aktuelle Benutzer angemeldet ist.
	char * s_sid=NULL;
    extract_COOKIE_data(&datCGI, "SID", &s_sid);
    extract_COOKIE_data(&datCGI, "EMAIL", &check_person.email);
    check_person.sid=atoi(s_sid);



    if(verify_sid(&check_person)){
		//die Person existiert und ist angemeldet

		course * all_courses=NULL;
		size_t num_max_courses=0;
        num_max_courses=get_distinct_courses(&all_courses); //Alle vorhandenen Kurse in alphabetischer Reihenfolge holen

		get_person_by_sid(&check_person);
		if(datCGI.request_method == GET){
			//die Einstellungen anzeigen
			//char * course_array=NULL;
			//comma_to_array(check_person.courses, &course_array);

			httpCacheControl("no-store, no-cache, must-revalidate, max-age=0");
			httpHeader(HTML);
			print_html_pure_head_menu("Benutzerkonteneinstellungen", "Einstellung", SETTINGS);

			puts("<div class='content'>");
			printf("<h2>%s aktuellen Kurse</h2>\n<span>%s</span>", check_person.isTeacher ? "Ihre" : "Deine", check_person.courses);
			puts("<br><p>Um Kurse auszuw&auml;hlen einfach auf die bunten Flächen klicken. Nachdem alle betreffenden Kurse\
					ausgew&auml;hlt wurden auf <em>Speichern</em> klicken.</p>");

			puts("<div id='courseSelection'>\n");
			puts("<table class='pure-table pure-table-bordered'>\n\
				<tr><td>Klasse:<td>\n\
						<select id='grade' onchange='toggleLetter();'>\n\
							<option value='9'>9</option>\n\
							<option value='10'>10</option>\n\
							<option selected='selected' value='11'>11</option>\n\
							<option value='12'>12</option>\n\
						</select>\n\
						<select style='display: none;' id='letter'>\n\
							<option selected='selected' value='a'>a</option>\n\
							<option value='b'>b</option>\n\
							<option value='c'>c</option>\n\
							<option value='d'>d</option>\n\
							<option value='e'>e</option>\n\
							<option value='f'>f</option>\n\
						</select>\n\
						<button onclick='selectOnly();'>Filter</button>\n\
						</td>\n\
						</tr>\n\
			<tr>\n\
			<td>Suche nach Text:</td> <td><input id='search-string' onkeydown='if (event.keyCode == 13)searchString(); ' type='text'><button onclick='searchString();'>Suchen</button></td>\n\
			</tr>\n\
			</table>");

			puts("<div id='courses'>");
			printf("<form style='border-radius: 1em; padding: 1em;' action='https://%s/cgi-bin/settings.cgi?1234' method='POST'>\n", datCGI.http_host);

			//Blöcke mit Kursbezeichnungen ausgeben
			for(size_t i=0; i<num_max_courses; i++){
				//TODO: Kurse in denen die Person schon ist einfärben
				//(checked="checked") jeweils hinzufügen
				printf("<input class='courseselector' type='checkbox' %s id='%s' name='%s'></input><label for='%s'>%s</label>\n",
						strstr(check_person.courses, (all_courses+i)->name)!=NULL ? "checked='checked'" : "",
						(all_courses+i)->name, (all_courses+i)->name, (all_courses+i)->name, (all_courses+i)->name);
			}
			puts("</div>"); // zu 'courses'

			//Die Sicherheitsabfrage soll erst dann sichtbar sein, wenn der Nutzer bereits Kurse eingestellt hat
			if(strcmp(check_person.courses, "n/a") != 0){
					puts("<br><input onclick='toggleId(this, \"btn_save\");' type='checkbox' id='really' name='really'><label for='really'>Wirklich bereits eingestellte Kurse Ver&auml;ndern?</label>");
			}
			printf("<br><input id='btn_save' class='submitButton' %s type='submit' value='Speichern'>", strcmp(check_person.courses, "n/a") != 0 ? "style='display: none;'" : "style='display: block;'");

			puts("</form>");
			puts("</div>");

			puts("</div>");

			puts("</div></div>");
			puts("</body></html>");

		}else if(datCGI.request_method == BOTH){
			//Die Einstellungen sollen verändern werden
			char * selected_courses=NULL;
			//Die Liste aller Kurse durchgehen und die ausgewählten in die neue Kursliste eintrage
			for(size_t i=0; i<num_max_courses; i++){
				char * result=NULL;
				if(extract_POST_data(&datCGI, (all_courses+i)->name, &result) == 0){
					//Kursliste erstellen
					asprintf(&selected_courses, "%s%s%s", selected_courses ? selected_courses : "", selected_courses ? ", " : "" , (all_courses+i)->name);
				}
				fprintf(stderr, "kurs: %s  result %s\n", (all_courses+i)->name, result);
				free(result);
			}

			fprintf(stderr, "--------------\nKURSLISTE: %s\n--------------\n\n", selected_courses);
			if(selected_courses != NULL){
				if(strcmp(check_person.courses, "n/a") == 0){
					//Die Person hatte noch keine Kurse --> erstes einstellen
					check_person.courses=selected_courses;
					update_user_courses(&check_person);
				}else if(extract_POST_data(&datCGI, "really",NULL) == 0){
					fprintf(stderr, "#################\nDer Nutzer will tatsächlich seine Kurse ändern\n#################");
					check_person.courses=selected_courses;
					update_user_courses(&check_person);
				}
			}

			//Den Nutzer wieder auf die Einstellungsseite umleiten
			char * redirectString=NULL;
			asprintf(&redirectString, "https://%s/cgi-bin/settings.cgi", datCGI.http_host);
			httpRedirect(redirectString);
		}
    }else{
    	fprintf(stderr, "Person nicht angemeldet: email: %s, sid: %s", s_sid, check_person.email);
    	httpCacheControl("no-store, no-cache, must-revalidate, max-age=0");
    	char * redirectString=NULL;
		asprintf(&redirectString, "https://%s/index.html", datCGI.http_host);
		httpRedirect(redirectString);
    }

    exit(0);
}
