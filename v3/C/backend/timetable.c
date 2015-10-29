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
	if(datCGI.http_cookies == NULL){
		//Nicht angemeldet, oder Cookies deaktiviert
		//print_exit_failure("Cookies müssen aktiv und gesetzt sein!");
		html_redirect_to_login();
		exit(0);
	}
	if(datCGI.request_method != GET)print_exit_failure("Use GET!");

	//Anhand der SID und der Email wird geprüft ob der aktuelle Benutzer angemeldet ist.
	char * s_sid=NULL;
	extract_COOKIE_data(&datCGI, "SID", &s_sid);
	extract_COOKIE_data(&datCGI, "EMAIL", &check_person.email);
	check_person.sid=atoi(s_sid);


    if(verify_sid(&check_person)){
		get_person_by_sid(&check_person);

		//Komma-getrennte Kursliste derDatenbank in ausgefranstes Array aus Strings umwandeln
		char ** a_course=NULL;
		int number_of_courses=comma_to_array(check_person.courses, &a_course);
		course * timetable_courses=NULL; //Array in der alle Stunden der Woche gespeichert werden
		size_t num_all_course=0; //Anzahl der Stunden

		/**
		Alle Kurse der Woche aus der Datenbank holen.
		--> Array mit allen Stunden die in einer Woche stattfinden wird erzeugt
		*/
		for(int i=number_of_courses; i--;){
			char * current_course=*(a_course+i);
			course * current_course_set=NULL; //Die neuen n Schulstunden (z.B. alle Mathe-Stunden der Woche)
			size_t num_new_courses=get_course(current_course, &current_course_set);
			if(num_new_courses > 0){
				timetable_courses=(course *)realloc(timetable_courses, (num_new_courses+num_all_course)*sizeof(course));
				person * teach;
				teach=calloc(1, sizeof(person));
				init_person(teach);
				bool success=get_teacher_by_course(teach, current_course_set[0].name);

				//HINWEIS: die 1 wird von num_new_courses schon am Anfang abgezogen
				// (da ja die Bedingung geprüft wird (j!=0) (eig. schlechter Stil ???)
				for(int j=num_new_courses; j--;){
					if(success){
						current_course_set[j].teacher=teach;
					}else{
						current_course_set[j].teacher=NULL;
					}
				}
				//Die neuen Kurse werden an den Stundenplan angehängt
				memcpy((timetable_courses+num_all_course), current_course_set, sizeof(course)*num_new_courses);
				free(current_course_set);
				if(!teach->acronym && teach)free(teach);
				num_all_course+=num_new_courses;
			}
		}



		/*
		puts("___________");
		puts("C   T   R");
		for(size_t i=0; i<num_all_course; i++){
			printf("%s  %s  %s\n", timetable_courses[i].name, timetable_courses[i].time, timetable_courses[i].room);
		}
		puts("___________");*/

		//Wochentag finden
		time_t now=time(NULL);
		struct tm * time_now=localtime(&now);
		int w_day=-1;
		if(time_now->tm_wday>0 && time_now->tm_wday<6)w_day=time_now->tm_wday-1;

		httpCacheControl("no-store, no-cache, must-revalidate, max-age=0");
		httpHeader(HTML);
		print_html_pure_head_menu("Hier ist der Stundenplan", "Stundenplan", TIMETABLE);

		puts("<div class='content' style='max-width: 900px;'>");

		printf("<span>Datum: %02d.%02d.%d %s</span>",
         time_now->tm_mday, time_now->tm_mon+1, time_now->tm_year+1900, time_now->tm_isdst ? "MESZ" : "MEZ" );

		puts("<table class='time-table' id='outer'>");
		puts("<tr><th>/</th>");

		//Wochentage als Spaltenüberschriften (aktuellen Tag hervorheben)
		for(int d=0; d<WEEKDAY_MAX; d++){
			printf("<th %s>%s</th>\n", d==w_day ? "class='day-active'" : "", long_german_weekdays[d]);
		}
		puts("</tr>");

		//Schulstunden und Tage durchlaufen
		for(int h=0; h<HOUR_MAX; h++){
			puts("<tr>");
			printf("<th> %d. Std.</th>", h+1);
			for(int d=0; d<WEEKDAY_MAX; d++){
				puts("<td>");
				course * c=NULL;
				char * time_string=NULL;
				asprintf(&time_string, "%s%d", german_weekdays[d], h+1);
				//printf("TagStunde: %s\n", time_string);

				//Herausfinden welche Stunde jetzt (h, d) ist
				for(size_t el=num_all_course; !c && el--;){
					if(strcmp(timetable_courses[el].time, time_string) == 0){
						c=(timetable_courses+el);
					}
				}
				if(c){
					//printf("Stunde: %s Kurs: %s Raum: %s\n", c->time, c->name, c->room);
					printf("<a href='/cgi-bin/spec_messages.cgi#%s'>", c->name);
					puts("<table class='sub-table'>\n<tr>\n");
					printf("<td class='sub-field'>%s</td> <td class='sub-field'>%s</td> <td class='sub-field'>%s</td>",
								c->name, c->room , c->teacher ? c->teacher->acronym : "---");
					puts("</tr></table>");
					puts("</a>\n");
				}

				puts("</td>");
			}
			puts("</tr>");

			//Jeweils nach der 2. 4. und 6. Stunde eine Pause darstellen
			//Nach der 9. Stunde auch eine Pause darstellen (aber diese ist die 3. Pause ?) oder was???
			if(h==1 || h==3 || h==5 || h==8){
				puts("<tr class='tiny'>\n");
				if(h!=5 && h!=8){
					printf("<th>%d. Pause</th><td>\n", (h+1)/2);
				}else{
					printf("<th>Mittagspause</th><td>");
				}
				puts("</td><td></td><td></td><td></td><td></td></tr>\n");
			}
		}
		puts("</table>"); //zu outer

		puts("</div>"); //content
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
