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

#define DEBUG


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

			puts("<div id='courseSettings'>\n");
			puts("<span style='font-weight: bold;'>Filter*</span>\n");
			puts("<table class='pure-table pure-table-bordered'>\n\
				<tr><td>Klasse:<td>\n\
						<select style='border: 2px solid grey; border-radius: .5em; background-color: white;' id='grade' onchange='toggleLetter();'>\n\
							<option value='9'>9</option>\n\
							<option value='10'>10</option>\n\
							<option selected='selected' value='11'>11</option>\n\
							<option value='12'>12</option>\n\
						</select>\n\
						<select style='border: 2px solid grey; border-radius: .5em; background-color: white; display: none;' id='letter'>\n\
							<option selected='selected' value='a'>a</option>\n\
							<option value='b'>b</option>\n\
							<option value='c'>c</option>\n\
							<option value='d'>d</option>\n\
							<option value='e'>e</option>\n\
							<option value='f'>f</option>\n\
						</select>\n\
						<button id='gradeFilterButton' style='border: 2px solid grey; background-color: white;' onclick='selectOnly(this);'>Filter anwenden</button>\n\
						</td>\n\
						</tr>\n\
			<tr>\n\
			<td>Kursname suchen:</td> <td><input id='search-string' onkeydown='if (event.keyCode == 13)searchString(null); ' type='text'>");
					puts("  <button id='stringFilterButton' style='border: 2px solid grey; background-color: white;' onclick='searchString(this);' >suchen</button></td>\n\
			</tr>\n\
			</table>");
			puts("<small>* erfordert Javascript</small>");

			printf("<form action='https://%s/cgi-bin/settings.cgi?course_update=change' method='POST'>\n", datCGI.http_host);
			puts("<div id='courses' style='padding: 1em;'>");

			//Blöcke mit Kursbezeichnungen ausgeben
			for(size_t i=0; i<num_max_courses; i++){
				printf("<input class='courseselector' type='checkbox' %s id='%s' name='%s'><label for='%s'>%s</label>\n",
						strstr(check_person.courses, (all_courses+i)->name)!=NULL ? "checked='checked'" : "",
						(all_courses+i)->name, (all_courses+i)->name, (all_courses+i)->name, (all_courses+i)->name);
			}
			puts("</div>"); // zu 'courses'
			//Die Sicherheitsabfrage soll erst dann sichtbar sein, wenn der Nutzer bereits Kurse eingestellt hat
			if(strcmp(check_person.courses, "n/a") != 0 || strlen(check_person.courses)>1){
					puts("<input onclick='toggleId(this, \"btn_save\");' type='checkbox' id='really' name='really'><label for='really'>Wirklich bereits eingestellte Kurse Ver&auml;ndern?</label>");
			}
			printf("<br><input id='btn_save' class='submitButton' %s type='submit' value='Speichern'>", strcmp(check_person.courses, "n/a") != 0 ? "style='display: none;'" : "style='display: block;'");

			puts("</form>");
			puts("</div>"); //zu courseSelection

			// E-Mail-Einstellungen
			printf("<h2>%s E-Mail-Adresse</h2>", check_person.isTeacher ? "Ihre" : "Deine");
			puts("<div id='emailSettings'");
			printf("<span>%s</span>", check_person.email);
			printf("<form action='https://%s/cgi-bin/settings.cgi?email_update=change' method='POST'>\n", datCGI.http_host);
			printf("<input type='email'required=""  onblur='checkDatEmail(this);' name='new_email' id='email' value='%s' placeholder='Ihre E-Mail-Adresse'>\n", check_person.email);
			puts("<input id='btn_save' class='submitButton' type='submit' value='Speichern'>\n");
			puts("</form>\n");

			puts("</div>"); // zu emailSettings

			puts("</div>");  //zu content
			puts("</div></div>"); //immer da
			puts("</body></html>");

		}else if(datCGI.request_method == BOTH){
			//Die Einstellungen sollen verändern werden

			if(extract_QUERY_data(&datCGI, "course_update", NULL) == 0){
				//Person will ihre Kurse verändern
				char * selected_courses=NULL;
				//Die Liste aller Kurse durchgehen und die ausgewählten in die neue Kursliste eintrage
				for(size_t i=0; i<num_max_courses; i++){
					char * result=NULL;
					if(extract_POST_data(&datCGI, (all_courses+i)->name, &result) == 0){
						//Kursliste erstellen
						asprintf(&selected_courses, "%s%s%s", selected_courses ? selected_courses : "", selected_courses ? ", " : "" , (all_courses+i)->name);
					}
					#ifdef DEBUG
					fprintf(stderr, "kurs: %s  result %s\n", (all_courses+i)->name, result);
					#endif // DEBUG
					free(result);
				}
				#ifdef DEBUG
				fprintf(stderr, "--------------\nKURSLISTE: %s\n--------------\n\n", selected_courses);
				#endif // DEBUG
				if(selected_courses != NULL){
					if(strcmp(check_person.courses, "n/a") == 0){
						//Die Person hatte noch keine Kurse --> erstes einstellen
						check_person.courses=selected_courses;
						update_user_courses(&check_person);
					}else if(extract_POST_data(&datCGI, "really",NULL) == 0){
						#ifdef DEBUG
						fprintf(stderr, "#################\nDer Nutzer will tatsächlich seine Kurse ändern\n#################");
						#endif // DEBUG

						// Nicht mehrere Kurs zur selben Zeit + nicht mehrere Lehrer in einem Kurs
						check_person.courses=selected_courses;
						char ** arr_selected_courses=NULL;
						int num_courses=comma_to_array(selected_courses, &arr_selected_courses);
						bool is_ok=true; //ist die Auswahl in Ordnung? (keine doppelten Lehrer / Stunden)

						if(check_person.isTeacher){
							person possible_teacher;
							init_person(&possible_teacher);
							for(int i=num_courses; i--;){
								if(get_teacher_by_course(&possible_teacher, arr_selected_courses[i])){
									//Der Kurs wird schon von einem Lehrer unterrichtet
									if(possible_teacher.id != check_person.id){
										//Der Kurs wird von einem anderen Lehrer unterrichtet.
										is_ok=false;
									}else{
										//Der aktuelle Lehrer unterrichtet diesen Kurs
										//und hatte ihn schon vorher ausgewählt
										is_ok=true;
									}
								}else{
									//Der Kurs wird noch nicht unterrichtet
								}
							}
						}

						//TODO: Doppelte Stundenbelegung-Testen

						if(is_ok){
							update_user_courses(&check_person);
						}else{
							char * url=NULL;
							asprintf(&url, "https://%s/cgi-bin/settings.cgi", datCGI.http_host);
							print_html_error("Fehlerhafte Auswahl!", url);
							exit(EXIT_FAILURE);
						}
					}
				}
			}else{
				#ifdef DEBUG
				fprintf(stderr, "Person will nicht die Kurse ändern\n");
				#endif // DEBUG
			}

			if(extract_QUERY_data(&datCGI, "email_update", NULL)==0){
				//Person will ihre Email verändern
				char * new_email=NULL;

				extract_POST_data(&datCGI, "new_email", &new_email);
				if(strcmp(check_person.email, new_email) != 0){
					if(email_exists(new_email)){
						char * url=NULL;
						asprintf(&url, "https://%s/cgi-bin/settings.cgi", datCGI.http_host);
						print_html_error("Email existiert in der Datenbank bereits", url);
						exit(0);
					}else{
                        if((strchr(new_email, '@') == strrchr(new_email, '@')) && strchr(new_email, '@')) {
							//Der Nutzer hat eine Gültige E-Mail-Adresse (mit genau einem '@') eingegeben
							//E-Mail-Adresse wir jetzt geändert
							#ifdef DEBUG
							fprintf(stderr, "Neue E-Mail-Adresse: %s, alte E-Mail-Adresse: %s\n", new_email, check_person.email);
							#endif // DEBUG
							update_user_email(&check_person, new_email);

							//Cookies neu setzen (da E-Mail jetzt anders)
							httpSetCookie("EMAIL", new_email);
							//httpSetCookie("SID", );
                        }else{
							char * url=NULL;
							asprintf(&url, "https://%s/cgi-bin/settings.cgi", datCGI.http_host);
							print_html_error("Geben sie eine Gültige E-Mail-Adresse ein!", url);
							exit(0);
                        }
					}
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
