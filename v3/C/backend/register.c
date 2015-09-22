#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>

#include "CGI_functions.h"
#include "SQL_functions.h"

#define DEBUG

//name=fhfhiu&teach=true&acronym=FFF&pass=weew&acceptTOS=on (58)

int main(int argc, char ** argv){
	cgi datCGI;
	char * teach=NULL;
	char * acceptTOS=NULL;
	person reg_person;
	bool pw_short=false;

	init_person(&reg_person);
	init_CGI(&datCGI);

	get_CGI_data(&datCGI);
	if(datCGI.request_method == GET){
		print_exit_failure("Use POST!");
	}


	//Für die Namen: siehe HTML-Dokument mit entsprechenden <input>-Elementen
	extract_POST_data(&datCGI, "name_vor", &reg_person.first_name);
	remove_newline(reg_person.first_name);
	clean_string(reg_person.first_name);
	extract_POST_data(&datCGI, "name", &reg_person.name);
	remove_newline(reg_person.name);
	clean_string(reg_person.name);
	extract_POST_data(&datCGI, "email", &reg_person.email);
	remove_newline(reg_person.email);
	clean_string(reg_person.email);
	extract_POST_data(&datCGI, "pass", &reg_person.password);
	remove_newline(reg_person.password);
	extract_POST_data(&datCGI, "acronym", &reg_person.acronym);
	remove_newline(reg_person.acronym);
	clean_string(reg_person.acronym);
	extract_POST_data(&datCGI, "teach", &teach);
	remove_newline(teach);
	extract_POST_data(&datCGI, "acceptTOS", &acceptTOS);
	remove_newline(acceptTOS);
	//TODO: fehlerhaften Aufruf abfangen
	if(strcmp(teach, "true") == 0){
		reg_person.isTeacher=true;
		if(strlen(reg_person.acronym) != 3){
            print_html_error("Das K&uuml;rzel muss genau 3 Zeichen lang sein", "/registrierung.html");
            exit(EXIT_FAILURE);
		}
	}else{
		reg_person.isTeacher=false;
	}
	//Die E-Mail-Adresse muss genau EIN '@' beinhalten
	if((strchr(reg_person.email, '@') == strrchr(reg_person.email, '@')) && strchr(reg_person.email, '@')) {
		#ifdef DEBUG
		fprintf(stderr, "es scheint alles zu passen (EMAIL)\n");
		#endif // DEBUG

		if(strlen(reg_person.password)<8){
			pw_short=true;
		}

		insert_user(&reg_person);
	}


	//fprintf(stderr, "\nnow comes da htmlz\n");


	httpCacheControl("no-store, no-cache, must-revalidate, max-age=0");
	httpHeader(HTML);
	//printf("%s\n", datCGI.POST_data);

	print_html_head("Passwort erneut eingeben", "Verifikation");

	puts("<body>\n\
		<div id='login-form'>\n");
		printf("<p><span>Herzlich willkommen <span style='font-weight: bold;'>%s %s.</span><br>Bitte %s zum Anmelden %s Passwort ein</p>\n",
				reg_person.first_name, reg_person.name, reg_person.isTeacher ? "geben Sie" : "gib", reg_person.isTeacher ? "Ihr" : "dein" );
	printf("<form method='post' action='/cgi-bin/login.cgi' style='border-radius: 1em; padding: 1em;' autocomplete='off'>\n\
		<input type='hidden' name='email' value='%s' />\n\
		<input class='textIn' placeholder='Passwort' type='password' id='pass' name='pass' required>\n\
		<button class='submitButton' type='submit'>Anmelden*</button>\n\
		</form>\n",reg_person.email);
	puts("<small>* Cookies müssen aktiviert sein!</small>\n");
	if(pw_short){
		puts("<br><small style='color: yellow; background-color: red;'>Sie sollten wirklich ein l&auml;ngeres Passwort verwenden!!</small>\n");
	}
	puts("</div>\n</body>\n</html>\n");

	/*puts("Erhaltene Daten:\n");
	printf("CONTENT_LENGTH: %d\n", datCGI.content_length);
	printf("Name:           %s\nPassword:       %s\n", reg_person.name, reg_person.password);
	printf("Kuerzel:        %s\nTeach:          %s\n", reg_person.acronym, teach);
	printf("accepted TOS:   %s\n\n", acceptTOS);

	printf("Post Data:      %s\n", datCGI.POST_data);*/

	exit(0);
}
