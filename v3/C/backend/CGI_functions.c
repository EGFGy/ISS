#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <features.h>

#include "CGI_functions.h"
#include "urlcode.h"


int ishex(int x){
	return	(x >= '0' && x <= '9')	||
		(x >= 'a' && x <= 'f')	||
		(x >= 'A' && x <= 'F');
}

void init_CGI(cgi * c){
	c->content_length=0;
	c->http_cookies=NULL;
	c->http_host=NULL;
	c->POST_data=NULL;
	c->query_string=NULL;
	c->request_method=GET;
}

/** \brief Aus dem Environment und der Standardeingabe wichtige Daten einlesen
 *
 * \param gotCGI cgi*  CGI-Objekt, in dem die gefundenen Daten gespeichert werden
 * \return void
 * Es werden aus den Eivironment-Variablen und je nachdem Wert von REQUEST_METHOD ein maximal
 * content_max langer String aus der Standardeingabe gelesen.
 */
void get_CGI_data(cgi * gotCGI){

    int content_length = 0;
	char * contentLength = NULL; //Länge des übertragenen Strings
	char * request_method = getenv("REQUEST_METHOD"); //HTTP-Request-Method
	char * env_cook = getenv("HTTP_COOKIE"); //Cookies holen
	char * http_host=getenv("HTTP_HOST"); //Host-Adresse holen
	char * POST_data = NULL; //Pointer in dem stdin gespeichert wird

	//fprintf(stderr, "get_CGI_data: env done\n");

	if(request_method == NULL){
		//request_method ist absolut notwendig
		print_exit_failure("Holen der Environment-Varialbe \"REQUEST_METHOD\" fehlgeschlagen");
	}


    if(env_cook != NULL)fprintf(stderr, "strlen() der cookies: %d\n", (int)strlen(env_cook));

	if(strncmp(request_method, "POST", 4) != 0){
	//TODO: FIX THIS !!!
		if(strncmp(request_method, "GET", 3) == 0){
			//TODO: Test this!
			char * query_string=getenv("QUERY_STRING");
			if(query_string == NULL){
				//print_exit_failure("Holen der Environment-Varialbe \"QUERY_STRING\" fehlgeschlagen");
				gotCGI->query_string=NULL;
				gotCGI->request_method=GET;
				gotCGI->http_cookies = env_cook;
				gotCGI->http_host=http_host;
			}else{

				char * pch;
				int query_len=strnlen(query_string, content_max);
				//Alle '+' durch Leerzeichen ersetzen
				for(int i=0; i<query_len; i++){
					pch=memchr(query_string, '+', query_len);
					if(pch !=NULL) *pch=' ';
				}
				//gotCGI->query_string=query_string;
				gotCGI->query_string=calloc(strlen(query_string)+1, sizeof(char));
				decodeHEX(query_string, gotCGI->query_string);
				//gotCGI->request_method = request_method;
				gotCGI->request_method=GET;
				gotCGI->http_cookies = env_cook;
				gotCGI->http_host=http_host;
			}
		}else{
			print_exit_failure("Use GET or POST");
		}
	}else{
		//Es ist POST
		contentLength = getenv("CONTENT_LENGTH");
		if(contentLength == NULL){
			print_exit_failure("Holen der Environment-Varialbe \"CONTENT_LENGTH\" fehlgeschlagen");
		}
		content_length=atoi(contentLength)+1;
		if(content_length > content_max){
			print_exit_failure("Eingabe zu lang!");
		}else{
			//fprintf(stderr, "get_CGI_data: loading POST\n");
			POST_data=calloc(content_length+2, sizeof(char));
			if(POST_data == NULL){
				print_exit_failure("Es konnte kein Speicher angefordert werden");
			}

			fgets(POST_data, content_length, stdin); //Standardeingabe lesen (vom fcgiwrapper)
			//fprintf(stderr, "get_CGI_data: loading POST done\ndata: %s\n", POST_data);

			if(POST_data == NULL){
				print_exit_failure("Keine Eingabe");
			}

			char * pch;
			//Alle '+' durch Leerzeichen ersetzen
			for(int i=0; i<content_length; i++){
				pch=memchr(POST_data, '+', content_length);
				if(pch !=NULL) *pch=' ';
			}


			//Wenn zusätzlich auch noch ein QUERY_STRING da ist, diesen auch einlesen
			char * query_string=getenv("QUERY_STRING");
			if(query_string != NULL){
				if(strlen(query_string)>0){
					fprintf(stderr, "(BOTH) QUERY_STRING: '%s'\n", query_string);

					char * pch;
					int query_len=strnlen(query_string, content_max);
					//Alle '+' durch Leerzeichen ersetzen
					for(int i=0; i<query_len; i++){
						pch=memchr(query_string, '+', query_len);
						if(pch !=NULL) *pch=' ';
					}
					gotCGI->query_string=query_string;
					gotCGI->request_method=BOTH;
				}else{
					gotCGI->request_method=POST;
				}
			}else{
				gotCGI->request_method=POST;
			}
			gotCGI->content_length=content_length;
			gotCGI->http_cookies=env_cook;
			gotCGI->POST_data=calloc(strlen(POST_data)+1, sizeof(char));

			//fprintf(stderr, "\nPOST_DATA vor HEX: '%s'\n\n", POST_data);

			decodeHEX(POST_data, gotCGI->POST_data);

			//fprintf(stderr, "POST_DATA nach HEX: '%s'\n\n", gotCGI->POST_data);
			gotCGI->http_host=http_host;
			return;
		}
	}
}

/** \brief Dem Aufrufer (Person mit Browser) sagen, was nicht funktioniert hat. Programm beenden.
 *
 * \param message const char*  Nachricht
 * \return void
 *
 */
void print_exit_failure(const char * message){
	printf("Content-type: text/plain\n\n");
	if(message != NULL){
		printf("ERROR: %s\n", message);
		fprintf(stderr, "ERROR: %s\n", message);
	}else{
		printf("ERROR: Keine Nachricht\n");
		fprintf(stderr, "ERROR: Keine Nachricht\n");
	}
	exit(EXIT_FAILURE);
}

/** \brief HTTP-Header-Text für das Setzten eins Cookies drucken
 *
 * \param name[] char    Attributname (Cookie-Name)
 * \param content[] char Attributwert (Cookie-Inhalt)
 * \return void
 *
 */
void httpSetCookie(char name[], char content[]){
	if(name == NULL || content == NULL){
		return;
	}else{
		printf("Set-Cookie: %s=%s\n", name, content);
	}
}

/** \brief HTTP-Header drucken
 *
 * \param type httpHeaderType  Typ des Headers (nur Text, oder html usw.)
 * \return void
 *
 */
void httpHeader(httpHeaderType type){
	switch(type){
		case HTML:
			puts("Content-type: text/html\n");
		break;
		case TEXT:
			puts("Content-type: text/plain\n\n");
		break;
	}
}

/** \brief HTTP-Header-Zeile zum Umleiten drucken
 *
 * \param url const char*  Zieladresse
 * \return void
 *
 */
void httpRedirect(const char * url){
	if(url != NULL){
		puts("Status: 301");
		printf("Location: %s\n\n", url);
	}
}

void httpCacheControl(const char * directive){
	if(directive != NULL){
		printf("Cache-Control: %s\n", directive);
	}
}

/** \brief Extrahiert aus einem Eingabe-String alle Zeichen zwischen "<property>=" und <delim>. Speicherung in out
 *
 * \param data char*            Eingabe-String
 * \param property const char*  Suchmuster
 * \param delim char*           Ende
 * \param out char**            Rückgabewert
 * \return int                  0: Erfolg , -1: Wenn der Wert nicht gefunden wird
 *
 */
int _extractCGIdata(char * data, const char * property, char * delim, char ** out){
	if(data == NULL || property == NULL || delim == NULL){
		print_exit_failure("Eingabeparameter von extractCGIdata sind falsch");
	}
	char * prop=NULL;
	prop=(char *)calloc(strlen(property)+1+1, sizeof(char)); // Für den Namen des Attributs Speicher anfordern
	char * tempData=NULL;
	tempData=(char *)calloc(strlen(data)+1, sizeof(char));

	if(prop == NULL || tempData == NULL){
		print_exit_failure("Es konnte kein Speicher angefordert werden");
	}
	strcpy(prop, property); //Den Namen, des Attributs kopieren und
	strcat(prop, "="); // ein '=' anfügen
	strcpy(tempData, data);

	char * start=NULL;
	start=strstr(tempData, prop); //Anfangspunkt der Suche festlegen
	if(start == NULL){
		//print_exit_failure("Fehler beim Suchen des Attributnamens");
		return -1;
	}
	char * klaus=NULL;
	klaus=strtok(start, delim)+strlen(prop); //alles bis zum '&' ausschneiden
	if(klaus == NULL){
		print_exit_failure("Token nicht gefunden");
	}

	//Neue Zeile am Ende durch 0-Terminator ersetzen.
	//das hier muss geändert werden!
	//char * newline=strchr(klaus, '\n');
	//if(newline != NULL){
	//	*newline='\0'; //vorher '\0'
	//}

	*out=calloc(strlen(klaus)+1, sizeof(char)); //Für den Rückgabepointer (out) Speicher anfordern
	if(*out == NULL){
		print_exit_failure("Es konnte kein Speicher angefordert werden");
	}
	strcpy(*out, klaus); //klaus in den Rückgabepointer kopieren


	//fprintf(stderr, "\n\ninhalt:\nprop:%s\ntempdata: %s\nout: %s", prop, tempData, *out);
	//Speicher freigeben
	free(prop);
	free(tempData);
	return 0;
	//return *out;

}


/** \brief Aus dem mit POST übergeben String Attributwerte extrahieren
 *
 * \param cgi cgi*              CGI-Objekt
 * \param property const char*  Attributname
 * \param out char**            Rückgabe des gefundenen Attributwertes
 * \return int                  0: Erfolg , -1: Wenn der Wert nicht gefunden wird
 *
 */
int extract_POST_data(cgi * cgi, const char * property, char ** out){
	if(out==NULL){
		char * placeholder=NULL;
		int i= _extractCGIdata(cgi->POST_data, property, "&",&placeholder);
		if(placeholder)free(placeholder);
		return i;
	}else{
		return _extractCGIdata(cgi->POST_data, property, "&", out);
	}
}

/** \brief Cookie-Daten extrahieren, anhand deren Attributnamen
 *
 * \param data char*            String mit allen Cookies
 * \param property const char*  Name des zu suchenden Attributs
 * \param out char**            Rückgabe des gefundenen Attributwertes
 * \return int                  0: Erfolg , -1: Wenn der Wert nicht gefunden wird
 *
 */
int extract_COOKIE_data(cgi * cgi, const char * property, char ** out){
	return _extractCGIdata(cgi->http_cookies, property, ";", out);
}

int extract_QUERY_data(cgi * cgi, const char * property, char ** out){
	//return _extractCGIdata(cgi->query_string, property, "&", out);
	if(out==NULL){
		char * placeholder=NULL;
		int i= _extractCGIdata(cgi->query_string, property, "&",&placeholder);
		if(placeholder)free(placeholder);
		return i;
	}else{
		return _extractCGIdata(cgi->query_string, property, "&", out);
	}
}


/** \brief Wandelt Hexadezimal-Sequenzen in entsprechende Zeichen um
 *
 * \param s const char*  Eingabestring
 * \param dec char*      Ausgabe
 * \return int
 *  von Rosetta-Code http://rosettacode.org/wiki/URL_decoding#C
 */
int decodeHEX(char *s, char *dec)
{
	char *o;
	char *end = s + strlen(s);
	int c;

	for (o = dec; s <= end; o++) {
		c = *s++;
		if (c == '+'){
			c = ' ';
		}else if (c == '%' && (	!ishex(*s++) ||	!ishex(*s++) ||	!sscanf(s - 2, "%2x", &c))){
			return -1;
		}
		if (dec) *o = c;
	}
	return o - dec;
}

void remove_newline(char * str){
	char * newline=strchr(str, '\n');
	if(newline != NULL){
		*newline='\0'; //vorher '\0'
	}
}
void print_html_head(char * descr, char * title){
	printf("\
	<!doctype html>\n\
	<html lang='de'>\n\
	<head>\n\
	<meta charset='utf-8'>\n\
	<meta name='viewport' content='width=device-width, initial-scale=1.0'>\n\
	<meta name='description' content='%s'>\n\
	<meta http-equiv='Cache-Control' content='no-cache, no-store, must-revalidate' />\n\
	<meta http-equiv='Pragma' content='no-cache' />\n\
	<meta http-equiv='Expires' content='0' />\n\
	<link rel='stylesheet' href='/css/forms.css'>\n\
	<link rel='shortcut icon' href='/favicon.png' />\n\
	<title>InfoWall -- %s</title>\n\
	</head>\n\
",descr, title );
}

void print_html_pure_head_menu(char * descr, char * title, menuSelection menu){
	if(descr == NULL || title == NULL){
		print_exit_failure("Programm falsch");
	}

printf("<!doctype html>\n\
<html lang='de'>\n\
<head>\n\
<meta charset='utf-8'>\n\
<meta name='viewport' content='width=device-width, initial-scale=1.0'>\n\
<meta name='description' content='%s'>\n\
<meta http-equiv='Cache-Control' content='no-cache, no-store, must-revalidate' />\n\
<meta http-equiv='Pragma' content='no-cache' />\n\
<meta http-equiv='expires' content='0' />\n\
<link rel='shortcut icon' href='/favicon.png' />\n\
\n\
<title>InfoWall -- %s</title>\n\
\n\
\n\
<link rel='stylesheet' href='/css/layouts/pure-min.css' type='text/css'>\n\
<link rel='stylesheet' href='/css/box.css' type='text/css'>\n\
<link rel='stylesheet' href='/css/forms.css' type='text/css'>\n\
<link rel='stylesheet' href='/css/radio_block.css' type='text/css'>\n\
\n\
	<!--[if lte IE 8]>\n\
        <link rel='stylesheet' href='/css/layouts/side-menu-old-ie.css' type='text/css'>\n\
    <![endif]-->\n\
    <!--[if gt IE 8]><!-->\n\
        <link rel='stylesheet' href='/css/layouts/side-menu.css' type='text/css'>\n\
    <!--<![endif]-->\n\
<!-- Modified http://purecss.io/js/ui.js -->\n\
<script src='/js/toggle.js'></script>\n\
<script src='/js/filter.js'></script>\n\
<script src='/js/good_passwd.js'></script>\n\
<script src='/js/jquery.js'></script>\n\
\n\
</head>\n\
<body>\n\
		<div id='layout'>\n\
                <!-- Menu toggle -->\n\
                <a href='#menu' onclick='menuShowHide(this);' id='menuLink' class='menu-link'>\n\
                        <!-- Hamburger icon -->\n\
                        <span></span>\n\
                </a>\n\
                <div id='menu'>\n\
                        <div class='pure-menu'>\n\
                                <a class='pure-menu-heading' href='#'>EGF</a>\n", descr, title);
	puts("<ul class='pure-menu-list'>\n");
	if(menu == TIMETABLE){
		puts("							<li class='pure-menu-item'>\n\
											<a href='/cgi-bin/timetable.cgi' class='pure-menu-link pure-menu-selected'>Stundenplan</a>\n\
										</li>\n\
			");
	}else{
        puts("							<li class='pure-menu-item'>\n\
											<a href='/cgi-bin/timetable.cgi' class='pure-menu-link'>Stundenplan</a>\n\
                                        </li>\n\
			");
	}

    if(menu == MAIN){
		puts("							<li class='pure-menu-item menu-item-divided pure-menu-selected'>\n\
											<a href='/cgi-bin/all_messages.cgi' class='pure-menu-link'>Allgemeines</a>\n\
										</li>\n\
			");
    }else{
		puts("							<li class='pure-menu-item'>\n\
											<a href='/cgi-bin/all_messages.cgi' class='pure-menu-link'>Allgemeines</a>\n\
										</li>\n\
			");
    }
    if(menu == COURSE){
		puts("							<li class='pure-menu-item menu-item-divided pure-menu-selected'>\n\
											<a href='/cgi-bin/spec_messages.cgi' class='pure-menu-link'>Kursbezogenes</a>\n\
										</li>\n\
			");
    }else{
		puts("							<li class='pure-menu-item'>\n\
											<a href='/cgi-bin/spec_messages.cgi' class='pure-menu-link'>Kursbezogenes</a>\n\
										</li>\n\
			");
    }
    if(menu == SETTINGS){
		puts("							<li class='pure-menu-item menu-item-divided pure-menu-selected'>\n\
											<a href='/cgi-bin/settings.cgi' class='pure-menu-link'>Einstellungen &#9881;</a>\n\
										</li>\n\
			");
    }else{
		puts("							<li class='pure-menu-item'>\n\
											<a href='/cgi-bin/settings.cgi' class='pure-menu-link'>Einstellungen &#9881;</a>\n\
										</li>\n\
			");
    }
    puts("								<li class='pure-menu-item'>\n\
											<a href='/hilfe.html' class='pure-menu-link'>Hilfe</a>\n\
										</li>\n\
										<li class='pure-menu-item'>\n\
											<a href='/cgi-bin/logout.cgi' class='pure-menu-link'>LOGOUT</a>\n\
										</li>\n\
		");
	printf("\
								</ul>\n\
							</div>\n\
				</div>\n\
				<div id='main'>\n\
					<div class='header'>\n\
				<h1>%s</h1>\n\
						<h2>%s</h2>\n\
				</div>\n\
				\n\
				<!-- dynamischer Inhalt ab hier -->\n", title, descr);
}

void print_html_error(char * ErrorText, char * back_url){
httpHeader(HTML);
				puts("<!DOCTYPE html>\n\
<head>\n\
		<title>InfoWall -- ERROR</title>\n\
		<meta http-equiv='content-type' content='text/html;charset=utf-8' />\n\
		<meta name='viewport' content='width=device-width'>\n\
		<link rel='stylesheet' href='/css/forms.css'>\n\
		<link rel='stylesheet' href='/css/box.css'>\n\
		<link rel='shortcut icon' href='/favicon.png' />\n\
</head>\n\
<body>\n\
		<div id='login-form'>\n");
		printf("<p><span class='error-text'>%s</span></p>\n", ErrorText);
				printf("<br><a href='%s' class='gradient-button'>Zur&uuml;ck</a>\n", back_url);
				puts("</div>\n</body>\n</html>\n");
}

/** \brief  Cookies auf 0 setzten und einen Link zur Anmeldeseite anzeigen
 *
 * \return void
 *
 * Dem Nutzer anzeigen, dass Cookies verwendet werden, und dass er sich einloggen soll.
 */
void html_redirect_to_login(){
	httpSetCookie("EMAIL", "NULL");
	httpSetCookie("SID", "0");
	httpHeader(HTML);
	print_html_head("Benutzung von Cookies", "Cookies");
	puts("<body><h1>Sie sind nicht angemeldet</h1><br>Damit Sie sich anmelden können müssen Cookies aktiv sein!<br>");
	printf("<a href='/eingang.html'>ZUR&Uuml;CK zur Anmeldung</a>");
	exit(0);
}
