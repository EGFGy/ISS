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

#define NO_ERROR 0
#define ERROR_DOUBLE_COURSE 1
#define ERROR_DOUBLE_TEACHER 2

#define DEBUG


int main(int argc, char ** argv){
	cgi datCGI;
	init_CGI(&datCGI);
	person check_person;
	init_person(&check_person);

	get_CGI_data(&datCGI);
	if(datCGI.http_cookies == NULL){
		html_redirect_to_login();
		exit(0);
	}

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
							<option value='7'>7</option>\n\
							<option value='8'>8</option>\n\
							<option value='9'>9</option>\n\
							<option value='10'>10</option>\n\
							<option selected='selected' value='11'>11</option>\n\
							<option value='12'>12</option>\n\
						</select>\n\
						<select style='border: 2px solid grey; border-radius: .5em; background-color: white; display: none;' id='letter' onchange='toggleLetter();'>\n\
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
			<td>Kursname suchen:</td> <td><input id='search-string' onfocus='textBoxReset();' onkeydown='if (event.keyCode == 13)searchString(null);' type='text'>");
					puts("  <button id='stringFilterButton' style='border: 2px solid grey; background-color: white;' onclick='searchString(this);' >suchen</button></td>\n\
			</tr>\n\
			</table>");
			puts("<small>* erfordert Javascript</small>");
			puts("<div id='select-all' style='margin-left: auto; margin-right: auto; text-align: center;'><button onclick='SelectAllShown();' style='margin-left: auto; margin-right: auto; display: block; border: 2px solid #808080; background-color: #FFFFFF;'>Pflichtfächer AUSWÄHLEN</button>\n\
				<small>Sport, Konfession und Sprache müssen noch ausgewählt werden</small></div>\n");

			printf("<form action='https://%s/cgi-bin/settings.cgi?course_update=change' method='POST'>\n", datCGI.http_host);
			puts("<div id='courses' style='padding: 1em;'>");

			//Blöcke mit Kursbezeichnungen ausgeben
			for(size_t i=0; i<num_max_courses; i++){
			    bool selected=false;
                selected=course_regex_search(all_courses+i, check_person.courses);

				printf("<input class='courseselector' type='checkbox' %s id='%s' name='%s'><label for='%s'>%s</label>\n",
				//		strstr(check_person.courses, (all_courses+i)->name)!=NULL ? "checked='checked'" : "",
                        selected ? "checked='checked'" : "",
						(all_courses+i)->name, (all_courses+i)->name, (all_courses+i)->name, (all_courses+i)->name);
			}
			puts("</div>"); // zu 'courses'
			//Die Sicherheitsabfrage soll erst dann sichtbar sein, wenn der Nutzer bereits Kurse eingestellt hat
			if(strcmp(check_person.courses, "n/a") != 0 || strlen(check_person.courses)>1){
					puts("<input onclick='toggleId(this, \"btn_save_course\");' type='checkbox' id='really' name='really'><label for='really'>Wirklich bereits eingestellte Kurse Ver&auml;ndern?</label>");
			}
			printf("<br><input id='btn_save_course' class='submitButton' %s type='submit' value='Speichern'>",
					strcmp(check_person.courses, "n/a") != 0 ? "onload=\"this.style.display='none';\"" : ""); //style='display: block;'

			puts("</form>");
			puts("</div>"); //zu courseSelection

			// E-Mail-Einstellungen
			printf("<h2>%s E-Mail-Adresse</h2>", check_person.isTeacher ? "Ihre" : "Deine");
			puts("<div id='emailSettings'>");
			printf("<span>%s</span>", check_person.email);
			printf("<form action='https://%s/cgi-bin/settings.cgi?email_update=change' method='POST'>\n", datCGI.http_host);
			printf("<input type='email' required=""  onblur='checkDatEmail(this);' name='new_email' id='email' value='%s' placeholder='%s E-Mail-Adresse'>\n", check_person.email, check_person.isTeacher ? "Ihre" : "Deine");
			puts("<br><input id='btn_save_email' class='submitButton' type='submit' value='Speichern'>\n");
			puts("</form>\n");

			puts("</div>"); // zu emailSettings


			//Passwort-Einstellungen (von ATL4s789)
			printf("<h2>%s Passwort</h2>", check_person.isTeacher ? "Ihr" : "Dein");
			puts("<div id='passwordSettings'>\n\
	<br>\n\
	<form action='/cgi-bin/settings.cgi?password_update=change' method='POST'>\n\
		<span>altes Passwort</span>\n\
		<br>\n\
		<input required class='settings-input' name='pass_old' id='pass_old' placeholder='altes Passwort' type='password'>\n\
		<br>\n\
		<span>neues Passwort</span>\n\
		<br>\n\
		<input required onkeyup=\"pruefStaerke(this.value)\" onkeydown=\"pruefStaerke(this.value)\" onchange=\"pruefStaerke(this.value)\" class='settings-input' name='pass_new_1' id='pass_new_1' placeholder='neues Passwort' type='password'>\n\
					<br>\n\
		<progress id='resultat' class='settings-input' value=0 max=100 style='width: 226px; border: 2px solid;'></progress>\n\
		<br>\n\
		<span>neues Passwort bestätigen</span>\n\
		<br>\n\
		<input required class='settings-input' name='pass_new_2' id='pass_new_2' placeholder='neues Passwort bestätigen' type='password'>\n\
		<br>\n\
		<input id='btn_save' class='submitButton' value='Speichern' type='submit'>\n\
	</form>\n\
</div>");



			puts("<!-- WÖRKARAUND für js-onload -->\n\
					<img src='/img/Arrow-Download-4-icon.png' style='display: none; width: 0px; heigth: 0px;' onload=\"document.getElementById('btn_save_course').style.display='none';\">");
			puts("</div>");  //zu content
			puts("</div></div>"); //immer da
			puts("</body></html>");

		}else if(datCGI.request_method == BOTH){
			bool return_to_settings=true; // soll der Nutzer nach einer erfolgreichen Änderung wieder zur Seite mit den Einstellungen umgeleitet werden?
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

				//Nur weitermachen, wenn auch was drinsteht
				if(selected_courses != NULL){
					if(strcmp(check_person.courses, "n/a") == 0){
						//Die Person hatte noch keine Kurse --> erstes einstellen
						check_person.courses=selected_courses;
						update_user_courses(&check_person);
					}else if(extract_POST_data(&datCGI, "really",NULL) == 0){
						#ifdef DEBUG
						fprintf(stderr, "#################\nDer Nutzer will tatsächlich seine Kurse ändern\n#################\n");
						#endif // DEBUG

						// Nicht mehrere Kurs zur selben Zeit + nicht mehrere Lehrer in einem Kurs
						check_person.courses=selected_courses;
						char ** arr_selected_courses=NULL;
						int num_courses=comma_to_array(selected_courses, &arr_selected_courses);
						int is_ok=NO_ERROR; //ist die Auswahl in Ordnung? (keine doppelten Lehrer / Stunden)
						size_t oldsize=0;
						course * timetable_courses=NULL;

						char * error_message=NULL;

						for(int i=num_courses; is_ok == NO_ERROR && i--;){
							/**
							Die Liste aller Stunden in der Woche holen und überprüfen, ob Stunden doppelt belegt sind
							*/
							char * current_course=*(arr_selected_courses+i);
							course * current_course_set=NULL;
							size_t num_new_courses=get_course(current_course, &current_course_set);
							if(num_new_courses > 0){
								if(timetable_courses != NULL){
									//Wenn schon Stunden zum Vergleich verfügbar sind
									for(int j=num_new_courses; j--;){ // Über alle neue Stunden
										for(int e=oldsize; e--;){ // Über alle schon im Stundenplan
											if(strcmp(timetable_courses[e].time, current_course_set[j].time) == 0){
												/**
												Wenn die Zeit(en) der/des neue(n) Kurse(s) mit den schon vorhanden
												Übereinstimmen, ist mindestens eine Stunde doppelt belegt.
												Das darf nicht sein.
												*/
												#ifdef DEBUG
												fprintf(stderr, "Dopp! %s und %s bei %s\n", current_course_set[j].name, timetable_courses[e].name, current_course_set[j].time);
												#endif // DEBUG
												asprintf(&error_message, "%s %s %s und %s bei %s",
															error_message ? error_message : "",
															error_message ? "<br>" : "",
															current_course_set[j].name,
															timetable_courses[e].name,
															current_course_set[j].time);
												is_ok=ERROR_DOUBLE_COURSE;
											}
										}
									}
								}
								//Die neuen Kurse werden an den Stundenplan angehängt
								timetable_courses=(course *)realloc(timetable_courses, (num_new_courses+oldsize)*sizeof(course));
								memcpy((timetable_courses+oldsize), current_course_set, sizeof(course)*num_new_courses);
								free(current_course_set);
								oldsize+=num_new_courses;
							}

							/**
							Prüfen, ob ein Kurs schon von einem anderen Lehrer unterrichtet wird
							*/
							if(check_person.isTeacher && is_ok == NO_ERROR){
								person possible_teacher;
								init_person(&possible_teacher);
								if(get_teacher_by_course(&possible_teacher, arr_selected_courses[i])){
									//Der Kurs wird schon von einem Lehrer unterrichtet
									if(possible_teacher.id != check_person.id){
										//Der Kurs wird von einem anderen Lehrer unterrichtet.
										is_ok=ERROR_DOUBLE_TEACHER;
									}else{
										//Der aktuelle Lehrer unterrichtet diesen Kurs
										//und hatte ihn schon vorher ausgewählt
										is_ok=NO_ERROR;
									}
								}else{
									//Der Kurs wird noch nicht unterrichtet
								}
							}
						}

						/*if(check_person.isTeacher){
							person possible_teacher;
							init_person(&possible_teacher);
							for(int i=num_courses; is_ok==NO_ERROR && i--;){
								if(get_teacher_by_course(&possible_teacher, arr_selected_courses[i])){
									//Der Kurs wird schon von einem Lehrer unterrichtet
									if(possible_teacher.id != check_person.id){
										//Der Kurs wird von einem anderen Lehrer unterrichtet.
										is_ok=ERROR_DOUBLE_TEACHER;
									}else{
										//Der aktuelle Lehrer unterrichtet diesen Kurs
										//und hatte ihn schon vorher ausgewählt
										is_ok=NO_ERROR;
									}
								}else{
									//Der Kurs wird noch nicht unterrichtet
								}
							}
						}

						if(is_ok == NO_ERROR){
							course * timetable_courses=NULL;
							size_t oldsize=0;
							for(int i=num_courses; i--;){
								char * current_course=*(arr_selected_courses+i);
								course * current_course_set=NULL;
								size_t num_new_courses=get_course(current_course, &current_course_set);
								if(num_new_courses > 0){
									timetable_courses=(course *)realloc(timetable_courses, (num_new_courses+oldsize)*sizeof(course));
									memcpy((timetable_courses+oldsize), current_course_set, sizeof(course)*num_new_courses);
									free(current_course_set);
									oldsize+=num_new_courses;
								}
							}
							for(int h=1; is_ok==NO_ERROR && h<HOUR_MAX; h++){
								for(int d=0; is_ok==NO_ERROR && d<WEEKDAY_MAX; d++){
									int cnt=0; // Zähler für das Auftreten dieser bestimmten Stunde (z.B. Di4)
									           // Muss 1 sein
									char * time_string=NULL;
									asprintf(&time_string, "%s%d", german_weekdays[d], h+1);
									for(int i=oldsize; is_ok==NO_ERROR && i--;){
										if(strstr(timetable_courses[i].time, time_string)){
											cnt++;
										}
										if(cnt>1)is_ok=ERROR_DOUBLE_COURSE;
									}
									free(time_string);
								}
							}
						}*/


						if(is_ok == NO_ERROR){
							update_user_courses(&check_person);
						}else{
							char * Meldung=NULL;
							//asprintf(&Meldung, "Falg: %d", is_ok);
							switch(is_ok){
								case ERROR_DOUBLE_COURSE:
									asprintf(&Meldung, "%s mehrere Kurse ausgew&auml;hlt die zur selben Zeit stattfinden.</span><br><br><span class='error-text'>%s</span>",
													check_person.isTeacher ? "Sie haben" : "Du hast", error_message);
								break;

								case ERROR_DOUBLE_TEACHER:
									asprintf(&Meldung, "Der Kurs wird schon von einem anderen Lehrer unterrichtet");
								break;
								default:
									asprintf(&Meldung, "Fehlerhafte Auswahl!");
								break;
							}


							print_html_error(Meldung, "/cgi-bin/settings.cgi");
							exit(EXIT_FAILURE); //(kann doch weggelassen werden, oder?)
						}
					}
				}
			}else{
				#ifdef DEBUG
				fprintf(stderr, "Person will nicht die Kurse ändern\n");
				#endif // DEBUG
			}

			if(extract_QUERY_data(&datCGI, "email_update", NULL)==0){
				//Person will ihre E-Mail-Adresse verändern

				return_to_settings=false;
				char * new_email=NULL;

				extract_POST_data(&datCGI, "new_email", &new_email);
				if(strcmp(check_person.email, new_email) != 0){
					if(email_exists(new_email)){
						print_html_error("Email existiert in der Datenbank bereits", "/cgi-bin/settings.cgi");
						exit(0);
					}else{
						//TODO Bessere E-Mail-Adressen-Prüfung einbauen
                        if((strchr(new_email, '@') == strrchr(new_email, '@')) && strchr(new_email, '@')) {
							//Der Nutzer hat eine Gültige E-Mail-Adresse (mit genau einem '@') eingegeben
							//E-Mail-Adresse wir jetzt geändert
							#ifdef DEBUG
							fprintf(stderr, "Neue E-Mail-Adresse: %s, alte E-Mail-Adresse: %s\n", new_email, check_person.email);
							#endif // DEBUG
							update_user_email(&check_person, new_email);

							//Cookies neu setzen (da E-Mail jetzt anders)
							httpSetCookie("EMAIL", new_email);
							print_html_error("E-Mail-Adresse erfolgreich geändert!", "/cgi-bin/settings.cgi");
							//httpSetCookie("SID", );
                        }else{
							print_html_error("Geben sie eine Gültige E-Mail-Adresse ein!", "/cgi-bin/settings.cgi");
                        }
					}
				}
			}

			if(extract_QUERY_data(&datCGI, "password_update", NULL)==0){
				//Person will ihr Passwort verändern
				return_to_settings=false;

				char * pass_old=NULL;
				char * pass_new_1=NULL;
				char * pass_new_2=NULL;

				extract_POST_data(&datCGI, "pass_old", &pass_old);
				extract_POST_data(&datCGI, "pass_new_1", &pass_new_1);
				extract_POST_data(&datCGI, "pass_new_2", &pass_new_2);
				remove_newline(pass_new_1);
				remove_newline(pass_new_2);
				remove_newline(pass_old);

				check_person.password=pass_old;
				if(strcmp(pass_new_1, pass_new_2) == 0){
					//Die neuen Passwörter wurden richtig eingegeben
					if(strcmp(pass_new_1, pass_old) !=0){
						//Neues Passwort ist anders als das Alte

						if(verify_user_password(&check_person)){
							//Person hat ihr aktuelles Passwort richtig eingegeben
							free(check_person.password); check_person.password=NULL;
							check_person.password=pass_new_1;

							bool state=update_user_password(&check_person);


							if(state)print_html_error("Passwort erfolgreich geändert!", "/cgi-bin/settings.cgi");
						}else{
							print_html_error("Passwort falsch!", "/cgi-bin/settings.cgi");
						}
					}

				}else{
					print_html_error("Die beiden Passwörter stimmen nicht überein", "/cgi-bin/settings.cgi");
				}
			}

			//Den Nutzer wieder auf die Einstellungsseite umleiten
			if(return_to_settings){
				char * redirectString=NULL;
				asprintf(&redirectString, "https://%s/cgi-bin/settings.cgi", datCGI.http_host);
				httpRedirect(redirectString);
			}
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
