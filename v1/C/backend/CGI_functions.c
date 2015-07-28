#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <features.h>

#include "CGI_functions.h"


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
	c->request_method=NULL;
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

	if(request_method == NULL){
        print_exit_failure("Holen der Environment-Varialbe \"REQUEST_METHOD\" fehlgeschlagen");
    }

    if(env_cook != NULL)fprintf(stderr, "strlen() der cookies: %d\n", (int)strlen(env_cook));

	if(strncmp(request_method, "POST", 5) != 0){

	//TODO: FIX THIS !!!
		//if(strncmp(request_method, "GET", 3) != 0){
			//TODO: Test this!
			char * query_string=getenv("QUERY_STRING");
			if(query_string == NULL){
				print_exit_failure("Holen der Environment-Varialbe \"QUERY_STRING\" fehlgeschlagen");
			}

			char * pch;
			int query_len=strnlen(query_string, content_max);
			//Alle '+' durch Leerzeichen ersetzen
			for(int i=0; i<query_len; i++){
				pch=memchr(query_string, '+', query_len);
				if(pch !=NULL) *pch=' ';
			}
			gotCGI->query_string=query_string;
			gotCGI->request_method = request_method;
			gotCGI->http_cookies = env_cook;
			gotCGI->http_host=http_host;
		/*}else{
			print_exit_failure("Use GET or POST");
		}*/
	}else{
		//Es ist POST
		contentLength = getenv("CONTENT_LENGTH");
		if(contentLength == NULL){
			print_exit_failure("Holen der Environment-Varialbe \"CONTENT_LENGTH\" fehlgeschlagen");
		}
		content_length=atoi(contentLength);
		if(content_length > content_max){
			print_exit_failure("Eingabe zu lang!");
		}else{
			POST_data=calloc(content_length+1, sizeof(char));
			if(POST_data == NULL){
				print_exit_failure("Es konnte kein Speicher angefordert werden");
			}

			fgets(POST_data, content_length+1, stdin); //Standardeingabe lesen (vom fcgiwrapper)

			if(POST_data == NULL){
				print_exit_failure("Keine Eingabe");
			}

			char * pch;

			//Alle '+' durch Leerzeichen ersetzen
			for(int i=0; i<content_length; i++){

				pch=memchr(POST_data, '+', content_length);
				if(pch !=NULL) *pch=' ';
			}
			gotCGI->content_length=content_length;
			gotCGI->http_cookies=env_cook;
			gotCGI->POST_data=calloc(strlen(POST_data), sizeof(char));
			decodeHEX(POST_data, gotCGI->POST_data);
			//gotCGI->POST_data=POST_data;
			gotCGI->query_string=NULL;
			gotCGI->request_method=request_method;
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
void setCookie(char name[], char content[]){
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
	klaus=strtok(start, delim)+strlen(prop); //alles bis zum '&' ausschneiden und
	if(klaus == NULL){
		print_exit_failure("Token nicht gefunden");
	}

	//Neue Zeile am Ende durch 0-Terminator ersetzen.
	char * newline=strchr(klaus, '\n');
	if(newline != NULL){
		*newline='\0'; //vorher '\0'
	}

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
	return _extractCGIdata(cgi->POST_data, property, "&", out);
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
