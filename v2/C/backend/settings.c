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
		//die PErson existiert und ist angemeldet

		course * all_courses=NULL;
		size_t num_max_courses=0;
        num_max_courses=get_distinct_courses(&all_courses);

		get_person_by_sid(&check_person);
		if(datCGI.request_method == GET){
			//die Einstellungen anzeigen
			httpCacheControl("no-store, no-cache, must-revalidate, max-age=0");
			httpHeader(HTML);
			print_html_pure_head_menu("Benutzerkonteneinstellungen", "Einstellung", SETTINGS);

			puts("<div class='content'>");
			printf("<h2>%s Kurse</h2>\n<span>%s</span>", check_person.isTeacher ? "Ihre" : "Deine", check_person.courses);

			puts("<div id='courseSelection'>\n");
			puts("<form style='border-radius: 1em; padding: 1em;' action='https://info-wall/cgi-bin/debug.cgi' method='POST'>\n");

			for(size_t i=0; i<num_max_courses; i++){
				printf("<input type='checkbox' id='%s' name='%s'/><label for='%s'>%s</label>\n",
						(all_courses+i)->name, (all_courses+i)->name, (all_courses+i)->name, (all_courses+i)->name);
			}
			puts("<input class='submitButton' style='display: block;' type='submit'></form>");
			puts("</div>");

			puts("</div>");

			puts("</div></div>");
			puts("</body></html>");

		}else if(datCGI.request_method == POST){
			//Die Einstellungen verändern
			char * selected_courses=NULL;
			for(size_t i=0; i<num_max_courses; i++){
				char * result=NULL;
				if(extract_POST_data(&datCGI, (all_courses+i)->name, &result) == 0){
					asprintf(&selected_courses, "%s, %s", selected_courses ? selected_courses : "", (all_courses+i)->name);
				}
				fprintf(stderr, "kurs: %s  result %s\n", (all_courses+i)->name, result);
				free(result);
			}

			fprintf(stderr, "--------------\nKURSLISTE: %s\n----------", selected_courses);

			char * redirectString=NULL;
			asprintf(&redirectString, "https://%s/cgi-bin/settings.cgi", datCGI.http_host);
			httpRedirect(redirectString);
		}
    }else{
    	char * redirectString=NULL;
		asprintf(&redirectString, "https://%s/index.html", datCGI.http_host);
		httpRedirect(redirectString);
    }

    exit(0);
}
