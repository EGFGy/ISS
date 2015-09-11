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

#define WEEKDAY_MAX 5
const char * german_weekdays[5]={"Mo", "Di", "Mi", "Do", "Fr"};
const char * long_german_weekdays[5]={"Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag"};
#define HOUR_MAX 11

int main(int argc, char ** argv){
	cgi datCGI;
	init_CGI(&datCGI);
	person check_person;
	init_person(&check_person);

    get_CGI_data(&datCGI);
	if(datCGI.http_cookies == NULL){
		//Nicht angemeldet, oder Cookies deaktiviert
		//print_exit_failure("Cookies müssen aktiv und gesetzt sein!");
		httpSetCookie("EMAIL", "NULL");
		httpSetCookie("SID", "0");
		httpHeader(HTML);
		print_html_head("Benutzung von Cookies", "Cookies");
		puts("<body>Cookies müssen aktiv und gesetzt sein!<br>");
		printf("<a href=https://%s/index.html>ZUR&Uuml;CK zur Anmeldung</a>", datCGI.http_host);
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

		char ** a_course=NULL;
		int number_of_courses=comma_to_array(check_person.courses, &a_course);
		course * timetable_courses=NULL;
		size_t oldsize=0;
		for(int i=number_of_courses; i--;){
			char * current_course=*(a_course+i);
			course * current_course_set=NULL;
			size_t num_new_courses=get_course(current_course, &current_course_set);
			if(num_new_courses > 0){
				timetable_courses=(course *)realloc(timetable_courses, (num_new_courses+oldsize)*sizeof(course)); //CAUSES TROUBLE WHAARG
				/*for(int j=num_new_courses; j--;){
					timetable_courses[oldsize+j].id=current_course_set[j].id;
					timetable_courses[oldsize+j].name=current_course_set[j].name;
					timetable_courses[oldsize+j].time=current_course_set[j].time;
					timetable_courses[oldsize+j].room=current_course_set[j].room;
				}*/
				person * teach;
				teach=calloc(1, sizeof(person));
				init_person(teach);
				bool success=get_teacher_by_course(teach, current_course_set);

				for(int j=num_new_courses; j--;){
					if(success){
						current_course_set[j].teacher=teach;
					}else{
						current_course_set[j].teacher=NULL;
					}
				}

				memcpy((timetable_courses+oldsize), current_course_set, sizeof(course)*num_new_courses);
				free(current_course_set);
				if(!teach->acronym && teach)free(teach);
				oldsize+=num_new_courses;
			}
		}
		/*
		puts("___________");
		puts("C   T   R");
		for(size_t i=0; i<oldsize; i++){
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

		puts("<table class='time-table' id='outer'>");
		/*puts("<tr>\n\
	<th>/</th> <th>Montag </th> <th>Dienstag </th> <th>Mittwoch </th> <th>Donnerstag</th> <th>Freitag</th>\n\
</tr>\n");*/
		puts("<tr><th>/</th>");
		for(int d=0; d<WEEKDAY_MAX; d++){
			printf("<th %s>%s</th>\n", d==w_day ? "class='day-active'" : "", long_german_weekdays[d]);
		}
		puts("</tr>");

		for(int h=0; h<HOUR_MAX; h++){
			puts("<tr>");
			printf("<th> %d. Std.</th>", h+1);
			for(int d=0; d<WEEKDAY_MAX; d++){
				puts("<td>");
				course * c=NULL;
				char * time_string=NULL;
				asprintf(&time_string, "%s%d", german_weekdays[d], h+1);
				//printf("TagStunde: %s\n", time_string);
				for(size_t el=oldsize; el--;){
					if(strcmp(timetable_courses[el].time, time_string) == 0){
						c=(timetable_courses+el);
						break;
					}
				}
				if(c){
					//printf("Stunde: %s Kurs: %s Raum: %s\n", c->time, c->name, c->room);
					printf("<a href='/cgi-bin/spec_messages.cgi#%s'>", c->name);
					puts("<table class='sub-table'>\n<tr>\n");
					printf("<td class='sub-field'>%s</td> <td class='sub-field'>%s</td> <td class='sub-field'>%s</td>", c->name, "100", c->teacher ? c->teacher->acronym : "---");
					puts("</tr></table>");
					puts("</a>\n");
				}

				puts("</td>");
			}
			puts("</tr>");
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
