#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <time.h>

#define DEBUG

#include "CGI_functions.h"
#include "SQL_functions.h"

person * duplicate_person(person * p){
	person * return_p=calloc(1, sizeof(person));
	if(return_p == NULL){
		print_exit_failure("Es konnte kein Speicher angefordert werden (duplicate_person)");
	}
	init_person(return_p);
	if(p->acronym)return_p->acronym=strdup(p->acronym);
	if(p->courses)return_p->courses=strdup(p->courses);
	if(p->email)return_p->email=strdup(p->email);
	if(p->first_name)return_p->first_name=strdup(p->first_name);
	if(p->name)return_p->name=strdup(p->name);
	if(p->password)return_p->password=strdup(p->password);
	return_p->auth=p->auth;
	return_p->id=p->id;
	return_p->isTeacher=p->isTeacher;
	return_p->sid=p->sid;

	return return_p;
}


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

		//Komma-getrennte Kursliste derDatenbank in ausgefranstes Array aus Strings umwandeln
		char ** a_course=NULL;
		int number_of_courses=comma_to_array(check_person.courses, &a_course);

		course_set timetable;
		init_course_set(&timetable);
		course_set alternate_courses;
		init_course_set(&alternate_courses);

		//course * timetable_courses=NULL; //Array in das alle Stunden der Woche gespeichert werden
		//size_t timetable.number=0; //Anzahl der Stunden

		/**
		Alle Kurse der Woche aus der Datenbank holen.
		--> Array mit allen Stunden die in einer Woche stattfinden wird erzeugt
		--> Array mit allen Vertretungsstunden wird erzeugt
		*/
		for(int i=number_of_courses; i--;){
			char * current_course=*(a_course+i);

			course_set new_courses;
			init_course_set(&new_courses);
			course_set new_alternate_courses;
			init_course_set(&new_alternate_courses);

			get_alter_course(current_course, &new_alternate_courses);
			get_course(current_course, &new_courses);
			//course * current_course_set=NULL; //Die neuen n Schulstunden (z.B. alle Mathe-Stunden der Woche)
			//size_t num_new_course=get_course(current_course, &current_course_set);
			if(new_courses.number > 0){
				timetable.c_set=(course *)realloc(timetable.c_set, (new_courses.number+timetable.number)*sizeof(course));
				person * teach=NULL;
				teach=calloc(1, sizeof(person));
				init_person(teach);
				bool success=get_teacher_by_course(teach, new_courses.c_set[0].name);

				//HINWEIS: die 1 wird von new_courses.number schon am Anfang abgezogen
				// (da ja die Bedingung geprüft wird (j!=0) (eig. schlechter Stil ???)
				for(int j=new_courses.number; j--;){
					if(success){
						//new_courses.c_set[j].teacher=teach;
						//person * p=new_courses.c_set[j].teacher;

						new_courses.c_set[j].teacher=duplicate_person(teach);

						//memcpy()
						//memcpy(new_courses.c_set[j].teacher, teach, sizeof(person));
					}else{
						new_courses.c_set[j].teacher=NULL;
					}
				}
				//Die neuen Kurse werden an den Stundenplan angehängt
				memcpy((timetable.c_set+timetable.number), new_courses.c_set, sizeof(course)*new_courses.number);
				//free(new_courses.c_set);
				if(teach){
					free_person(teach);
					free(teach);
				}
				timetable.number+=new_courses.number;
			}

			if(new_alternate_courses.number>0){
				alternate_courses.c_set=(course *)realloc(alternate_courses.c_set, (new_alternate_courses.number+alternate_courses.number)*sizeof(course));

				person * teach=NULL;
				teach=calloc(1, sizeof(person));
				init_person(teach);
				bool success=get_teacher_by_course(teach, new_alternate_courses.c_set[0].name);

				for(int j=new_alternate_courses.number; j--;){
					if(success){
						/*new_alternate_courses.c_set[j].teacher=calloc(1, sizeof(person));
						memcpy(new_alternate_courses.c_set[j].teacher, teach, sizeof(person));*/
						new_alternate_courses.c_set[j].teacher=duplicate_person(teach);
					}else{
						new_alternate_courses.c_set[j].teacher=NULL;
					}
				}
				//Die neuen Kurse werden an die Liste der Vertretungsstunden angehängt
				memcpy((alternate_courses.c_set+alternate_courses.number), new_alternate_courses.c_set, sizeof(course)*new_alternate_courses.number);
				//free(new_alternate_courses.c_set);
				if(teach){
					free_person(teach);
					free(teach);
				}
				alternate_courses.number+=new_alternate_courses.number;
			}

			/**
			Problem:
			Informationen des Lehrers werden nur einmal gespeichert
			in der Schleife von 'free_course_set' wird der Inhalt des Lehrers einmal gelöscht.
			Die Restlichen Pointer (in anderen Kursen) zeigen aber immer noch auf den jetzt gelöschten Lehrer.

			Drei Leute Zeigen auf ein Haus. Das Haus wird gesprengt. (free_person)
			Der erste sagt: "Das Haus ist weg" (NULL wird zugewiesen)
			Die andern beiden sagen: "Da ist ein Haus" (die anderen Pointer, in den anderen Kursen bleiben unverändert)

			Lösung #1: <-- diese wird verwendet
			Für jeden Kurs einzeln einen Lehrer anlegen
			(--> braucht mehr Speicher)

			Lösung #2:
			Die Freigabe-Funktion muss prinzipiell anders aufgebaut werden und sich merken,
			welche Pointer schon freigegeben wurden und welche nicht
			(--> aufwändig, brauch mehr Zeit zum Implementieren)
			*/

			//free_course_set(&new_alternate_courses);

			free(new_alternate_courses.c_set); // Pointer löschen (Inhalt wurde vorher Kopiert und bleibt erhalten)
			free(new_courses.c_set);
		}

		/**
		Vertretungsplan einbauen (--> Stundenplan wird abhängig von den alternativen Kursen (=Änderungen))
		*/
		for(size_t i=0; i<alternate_courses.number; i++){

			#ifdef DEBUG
			//struct timeval stop, start;
			//gettimeofday(&start, NULL);
			fprintf(stderr, "alternate_course %zu '%s'\n", i, alternate_courses.c_set[i].name);
			#endif // DEBUG

            for(size_t j=0; j<timetable.number; j++){
				int name_match=strcmp(alternate_courses.c_set[i].name, timetable.c_set[j].name);
				int time_match=strcmp(alternate_courses.c_set[i].time, timetable.c_set[j].time);
				if(name_match == 0 && time_match == 0){
					// Hier ist was geändert
					#ifdef DEBUG
					fprintf(stderr, "Änderung von %s\n", timetable.c_set[j].name);
					#endif // DEBUG
					timetable.c_set[j].status=alternate_courses.c_set[i].status;

					if(alternate_courses.c_set[i].status & TEACHER){
						//TODO: null teacher abfangen

						#ifdef DEBUG
						fprintf(stderr, "	Alter Lehrer: %s\n", timetable.c_set[j].teacher->acronym);
						#endif // DEBUG
						free_person(timetable.c_set[j].teacher);
						free(timetable.c_set[j].teacher);

						timetable.c_set[j].teacher=NULL;
						timetable.c_set[j].teacher=calloc(1, sizeof(person));
						init_person(timetable.c_set[j].teacher);

						get_person_by_acronym(timetable.c_set[j].teacher, alternate_courses.c_set[i].alter_teacher_acronym);
						#ifdef DEBUG
						fprintf(stderr, "	Neuer Lehrer: %s\n", timetable.c_set[j].teacher->acronym);
						#endif // DEBUG
					}
					if(alternate_courses.c_set[i].status & ROOM ){
						#ifdef DEBUG
						fprintf(stderr, "	ROOM: %s --> %s, Kurs %s\n", timetable.c_set[j].room, alternate_courses.c_set[i].alter_room, alternate_courses.c_set[i].name);
						#endif // DEBUG
						if(timetable.c_set[j].room)free(timetable.c_set[j].room);
						timetable.c_set[j].room=strdup(alternate_courses.c_set[i].alter_room);


						#ifdef DEBUG
						fprintf(stderr, "	ROOM: %s bei %s Jetzt\n", timetable.c_set[j].room, timetable.c_set[j].name);
						#endif // DEBUG
					}
				}
            }

            #ifdef DEBUG
            //gettimeofday(&stop, NULL);
			//fprintf(stderr, "Zeit: %d\n", stop.tv_usec-start.tv_usec);
			#endif // DEBUG
		}

		/*
		puts("___________");
		puts("C   T   R");
		for(size_t i=0; i<timetable.number; i++){
			printf("%s  %s  %s\n", timetable.c_set[i].name, timetable.c_set[i].time, timetable.c_set[i].room);
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
				for(size_t el=timetable.number; !c && el--;){
					if(strcmp(timetable.c_set[el].time, time_string) == 0){
						c=(timetable.c_set+el);
					}
				}
				if(c){
					//printf("Stunde: %s Kurs: %s Raum: %s\n", c->time, c->name, c->room);

					char * css_class="";
					if(c->status & OMITTED)css_class="cancelled";
					if(c->status & TEACHER)css_class="teacher";
					if(c->status & ROOM)css_class="moved";

					#ifdef DEBUG
					person * t=c->teacher;
					fprintf(stderr, "PLAN: T='%s' N='%s' R='%s' A='%s'\n", time_string, c->name, c->room, t ? t->acronym : "NOPE");
					#endif // DEBUG

					printf("<a href='/cgi-bin/spec_messages.cgi?course=%s#button_%s'>", c->name, c->name);
					printf(" %s<table class='sub-table'>\n <tr class='%s'> \n", c->status==OMITTED ? "<s style='color: black;'>":"" , css_class);
					printf("<td class='sub-field'>%s</td> <td class='sub-field'>%s</td> <td class='sub-field'>%s</td>",
								c->name, c->room , c->teacher ? c->teacher->acronym : "---");
					printf("</tr></table>%s", c->status==OMITTED ? "</s>":"");
					puts("</a>\n");
				}
				free(time_string);
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

		free_course_set(&timetable);
		free_course_set(&alternate_courses);
    }else{
		//printf("Erst anmelden!!");
		char * redirectString=NULL;
		asprintf(&redirectString, "https://%s/index.html", datCGI.http_host);
		httpCacheControl("no-store, no-cache, must-revalidate, max-age=0");
		httpRedirect(redirectString);
	}

	free_cgi(&datCGI);
	free_person(&check_person);
	exit(0);
}
