#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <my_global.h>

#include <mysql.h>

#include <mysql.h>
#include <ctype.h>
#include <time.h>

#include <sys/types.h>
#include <regex.h>

#include <crypt.h>

#include "SQL_functions.h"
#include "CGI_functions.h"

#define SELECT_Schueler "SELECT Benutzer.id, name, passwort, kurse FROM Benutzer, Schueler WHERE name=\"?\" AND Schueler.id=Benutzer.id;"

//id, vorname, name, email, passwort, kuerzel, kurse, sid
// > ^ <

#define DEBUG

/**
Identifikation per E-mail oder Kürzel (base5)
*/

//Ausprobier
/*
extern int asprintf (char **__restrict __ptr,
                     const char *__restrict __fmt, ...)
     __THROWNL __attribute__ ((__format__ (__printf__, 2, 3))) __wur;
*/

const char * german_weekdays[5]={"Mo", "Di", "Mi", "Do", "Fr"};
const char * long_german_weekdays[5]={"Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag"};

void init_person(person * p){
	p->acronym=NULL;
	p->auth=false;
	p->courses=NULL;
	p->email=NULL;
	p->first_name=NULL;
	p->id=0;
	p->isTeacher=false;
	p->name=NULL;
	p->password=NULL;
	p->sid=0;
	p->login_time=NULL;
}

void init_message(message * mes){
	mes->courses=NULL;
	mes->created=NULL;
	mes->creator_id=0;
	mes->id=0;
	mes->message=NULL;
	mes->title=NULL;
}

void init_course(course * c){
	c->id=0;
	c->name=NULL;
	c->time=NULL;
	c->room=NULL;

	c->teacher=NULL;
}

/** \brief Überprüfen, ob eine Person in der Datenbank ist und ob das Passwort stimmt
 *
 * \param pers person*  Person, die angemeldet werden soll
 * \return int          1: Benutzer ist schon angemeldet; 0: Benutzer war noch nicht angemeldet (PW richtig); -1: PW falsch
 *  Die Funktion testet, ob der Name oder das Kürzel (je nachdem was eingegeben wurde)
 *  (Erkennung an der Länge des Strings: =3 --> Kürzel, >3 --> Name)
 *  in der DB vorhanden ist. Wenn der Name existiert wird geprüft ob das Passwort richtig ist.
 *  Wenn das Passwort stimmt wird der bool-Wert "auth" in "person" auf true gesetzt.
 *  --> die Person ist authentifiziert.
 */
int verify_user(person * pers){
	bool isAcronym;
	UserState user_state=PW_INCORRECT;

	if(pers->email==NULL || pers->password==NULL){
		print_exit_failure("Programm falsch!");
	}

	isAcronym=detect_convert_acronym(pers);

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){

		print_exit_failure("MYSQL-connection error!");
	}else{
		//fprintf(stderr, "Connection extablished!\n");
	}

	MYSQL_RES * result=NULL;
	bool found=false;

	if(isAcronym){
		//Es ist sicher eine Lehrer (jemand hat das Kürzel eingegeben)
		//TODO: sql-injection verhindern

		char * sql_query=NULL;
		if(asprintf(&sql_query, "SELECT  * FROM Benutzer WHERE kuerzel='%s'", pers->acronym) == -1){
			print_exit_failure("Es konnte kein Speicher angefordert werden (verify_user)");
		}
		#ifdef DEBUG
		fprintf(stderr, "Lehrer.\nsql_query: %s\n", sql_query);
		#endif // DEBUG

		if(mysql_query(my, sql_query)){  //Liefert 0 bei Erfolg
			print_exit_failure("mysql_query failed (Lehrer)");
		}

		result = mysql_store_result(my);

		if(mysql_num_rows(result) == 1){
			found=true;
			isAcronym=true; //Da wir jetzt eine Person mit Kürzel gefunden haben
			pers->isTeacher=true;
		}
		free(sql_query);
    }else{
		//Es könnte ein Lehrer oder ein Schüler sein

		char * sql_query=NULL;
		if(asprintf(&sql_query, "SELECT * FROM Benutzer WHERE email='%s'", pers->email) == -1){
			print_exit_failure("Es konnte kein Speicher angefordert werden (verify_user)");
		}
		#ifdef DEBUG
		fprintf(stderr, "Schueler o. Lehrer.\nsql_query: %s\n", sql_query);
		#endif // DEBUG

		if(mysql_query(my, sql_query)){  // Liefert 0 bei Erfolg
			print_exit_failure("mysql_query failed (Schueler - Lehrer)");
		}

		result = mysql_store_result(my);

		if(mysql_num_rows(result) == 1){
			found=true;
			isAcronym=false; //Da wir jetzt eine Person mit Kürzel gefunden haben
		}else{
			//Person nicht vorhanden, oder Fehler
			//print_exit_failure("mysql: Person nicht vorhanden, oder Passwort falsch."); //Was auch immer
			found=false;
			isAcronym=false;
		}
		free(sql_query);
    }

	//Ab hier wurde die SQL-Query für Lehrer oder Schüler ausgeführt.
	if(found == true){
		MYSQL_ROW row;
		row=mysql_fetch_row(result);
		#ifdef DEBUG
		fprintf(stderr, "\nEin Ergebnis!\n Name: %s, Pass: %s, SID: '%s'\n", row[COL_NAME], row[COL_PASS], row[COL_SID]);
		#endif // DEBUG

		//Auslesen des Salt
		char * salt=calloc(SALT_LENGTH+1, sizeof(char));
		//strncat(salt, row[COL_PASS], 1);
		//strncat(salt, row[COL_PASS]+1, 1);
		for(int i=0; i<SALT_LENGTH; i++){
			strncat(salt, row[COL_PASS]+i, 1);
		}
		char * arg=NULL;
		asprintf(&arg, "$6$%s$", salt);
		char * encr=crypt(pers->password, arg);
		free(pers->password);
		char * load_pw=NULL;
		asprintf(&load_pw, "%s%s", salt, encr+strlen(arg));
		//pers->password=encr+strlen(arg);

		if(strcmp(load_pw, row[COL_PASS]) == 0){
			pers->auth=true;

			//Name holen
			//pers->name=calloc(strlen(row[COL_NAME])+1, sizeof(char));
			//strcpy(pers->name, row[COL_NAME]);
			asprintf(&pers->name, "%s", row[COL_NAME]);
			//pers->first_name=calloc(strlen(row[COL_VORNAME])+1, sizeof(char));
			//strcpy(pers->first_name, row[COL_VORNAME]);
			asprintf(&pers->first_name, "%s", row[COL_VORNAME]);

			if(isAcronym){
				//Person hat Kürzel angegeben --> es ist eine Leherer --> email holen holen
				pers->email=calloc(strlen(row[COL_EMAIL])+1, sizeof(char));
				strcpy(pers->email, row[COL_EMAIL]);
			}else{
				//Person hat ihre Email-Adresse statt dem Kürzel angegeben --> (Falls es ein Lehrer ist, dessen Kürzel holen)
				pers->acronym=NULL;
				if(row[COL_ACR] != NULL){
					//Die Person hat ein Küzel --> Lehrer
					pers->acronym=calloc(strlen(row[COL_ACR])+1, sizeof(char));
					strcpy(pers->acronym, row[COL_ACR]);
					pers->isTeacher=true;
				}else{
					pers->isTeacher=false;
				}
			}

			//Kurse (falls vorhanden)
			if(row[COL_COURSE] != NULL){
				pers->courses=calloc(strlen(row[COL_COURSE])+1, sizeof(char));
				strcpy(pers->courses, row[COL_COURSE]);
			}

			//ID holen
			if(row[COL_ID] != NULL){
				pers->id=atoi(row[COL_ID]);
			}

			if(row[COL_SID] != NULL){
				//Benutzer ist schon angemeldet
				user_state=PW_CORRECT_ALREADY_LOGGED_IN;
				pers->auth=true;
				pers->sid=atoi(row[COL_SID]);
			}else{
				user_state=PW_CORRECT;
				create_session(pers);
			}
		}else{
			user_state=PW_INCORRECT;
			pers->auth=false;
			pers->sid=0;
		}
	}

	mysql_free_result(result);
	mysql_close(my);

	return user_state;
}


/** \brief Überprüfen ob das Passwort einer angemeldeten Person stimmt
 *
 * \param pers person*  Person, mit E-Mail-Adresse und SessionID
 * \return bool         true: Passwort ist richtig; false: Passwort ist falsch
 *
 */
bool verify_user_password(person * pers){
	char * query=NULL;
	bool password_state=false;
	MYSQL * my=NULL;

	if(pers->password == NULL || pers->sid==0){
		print_exit_failure("Programm falsch (verify_user_password)");
	}

	if(asprintf(&query, "SELECT * FROM Benutzer WHERE sid='%d' AND email='%s'", pers->sid, pers->email) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (verify_user_password)");
	}

	my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure (verify_user_password)");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		print_exit_failure("mysql_query failed (verify_user_password)");
		#ifdef DEBUG
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		#endif // DEBUG
	}else{
		MYSQL_RES * result=NULL;
		result = mysql_store_result(my);

		if(mysql_num_rows(result) == 1){
			MYSQL_ROW * row=NULL;
			row=mysql_fetch_row(result);
			#ifdef DEBUG
			fprintf(stderr, "Benutzer gefunden (verify_user_password)\n");
			#endif // DEBUG
			char * password_encrypted=NULL;
			char * password_db=NULL;
			char * salt=NULL;
			password_db=row[COL_PASS];

			salt=salt_extract(password_db);

			char * arg=NULL;
			asprintf(&arg, "$6$%s$", salt);
			char * encr=crypt(pers->password, arg);
			//free(pers->password);
			asprintf(&password_encrypted, "%s%s", salt, encr+strlen(arg));

			free(salt);
			free(arg);
			free(encr);

			if(strcmp(password_db, password_encrypted) == 0){
				#ifdef DEBUG
				fprintf(stderr, "Passwort war richtig! (Benutzer: %s)", pers->email);
				#endif // DEBUG
				password_state=true;
			}else{
				password_state=false;
			}


		}else{
			pers->auth=false;
		}
		mysql_free_result(result);
	}

    mysql_close(my);
    free(query);

    return password_state;

}

/** \brief Feststellen ob die E-mail möglicherweise ein Kürzel ist und ggf. Zuordnung ändern.
 *
 * \param pers person*  Personen-Struktur
 * \return bool         true: es gibt ein Kürzel, false: es ist kein Kürzel.
 *  Falls 'email' genau drei Buchstaben lang ist und ein '@' beinhaltet wird der Inhalt in das Kürzel verschoben und
 *  zu nur Großbuchstaben umgewandelt.
 */
bool detect_convert_acronym(person * pers){
	bool isAcronym;

	if(pers->email==NULL || pers->password==NULL){
		print_exit_failure("Programm falsch!");
	}

	if(strlen(pers->email) > 3 || strchr(pers->email, '@') != NULL){
		isAcronym=false;
	}else{
		if(strlen(pers->email) == 3){
			isAcronym=true;
			pers->acronym=pers->email;
			pers->email=NULL;

			uppercase_acr(pers);
		}
	}
    return isAcronym;
}

/** \brief Kürzel in nur Großbuchstaben umwandeln
 *
 * \param pers person*  Personen-Struktur
 * \return void
 *
 */
void uppercase_acr(person * pers){
	if(pers->acronym != NULL){
		int p=0;
		while(pers->acronym[p]){
			pers->acronym[p]=toupper(pers->acronym[p]);
			p++;
		}
	}
}

/** \brief Nutzer mit Name, (ggf. Kürzel) und Passwort in die DB einfügen
 *
 * \param pers person*  Personen-Struktur
 * \return void
 * Eine Person in die DB einfügen, falls diese noch nicht existiert.
 * Das Passwort wird mithilfe von crypt() verschlüsselt
 */
void insert_user(person * pers){
	MYSQL *my=NULL;
	char * query=NULL;

	if(pers == NULL){
		print_exit_failure("Programm falsch.\n Wörk!");
	}

	if(email_exists(pers->email)){
		print_exit_failure("Benutzer Existiert schon!");
	}

	my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure\n Wörk!");
	}

	if(mysql_real_connect(my, "localhost", SQL_ALTERNATE_USER, SQL_ALTERNATE_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	char * salt=NULL;
	salt_generate(&salt);

    //Verhindern, dass ein bereits vorhandenes Salt zweimal verwendet wird (falls zwei Nutzer identische Passwörter wählen)
	while(salt_exists(&salt)){
		salt_generate(&salt);
	}
	char * arg=NULL;
	asprintf(&arg, "$6$%s$", salt);

	char * encr=crypt(pers->password, arg);
	char * store_pw=NULL;
	asprintf(&store_pw, "%s%s", salt, encr+strlen(arg));
	free(arg);
	free(pers->password);

	//pers->password=encr+strlen(arg);

	clean_string(pers->first_name);
	clean_string(pers->email);
	clean_string(pers->name);


	//Ist es eine Lehrer oder ein Schüler?
	if(!pers->isTeacher){
		if(asprintf(&query, "INSERT INTO Benutzer (vorname, name, email, passwort, kurse) \
					VALUES('%s', '%s', '%s', '%s', 'n/a')",
					pers->first_name, pers->name, pers->email, store_pw) == -1)
		{
			print_exit_failure("Es konnte kein Speicher angefordert werden (insert_user)");
		}
	}else{
		clean_string(pers->acronym);
		if(asprintf(&query, "INSERT INTO Benutzer (vorname, name, email, passwort, kurse, kuerzel) \
					VALUES('%s', '%s', '%s', '%s', 'n/a', '%s')",
					pers->first_name, pers->name, pers->email, store_pw, pers->acronym) == -1)
		{
			print_exit_failure("Es konnte kein Speicher angefordert werden (insert_user)");
		}
	}
	#ifdef DEBUG
	fprintf(stderr, "\nInsert dat:\n%s\n", query);
	#endif // DEBUG
	if(mysql_query(my, query)){
		#ifdef DEBUG
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		#endif // DEBUG
		print_exit_failure("mysql_query failed (insert)");
	}
	free(query);
	mysql_close(my);
}

/** \brief Salz generieren aus urandom (könnte man das nicht einfacher haben?)
 *
 * \param salt char**   Pointer auf einen Salt-pointer
 * \return void
 *
 */
void salt_generate(char ** salt){
	FILE * f_random=fopen("/dev/urandom", "r");
	if(!f_random)print_exit_failure("Problem beim generiern des Salt: urandom wurde nicht geoeffnet!");
	*salt=calloc(SALT_LENGTH+1, sizeof(unsigned char));

	char * letters="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

	//strcpy(salt, "00");
	for(int i=SALT_LENGTH; i>0; i--){
		sprintf(*salt,"%s%c",*salt,letters[fgetc(f_random) % 64]);
	}
}

/** \brief Prüfen ob das Salz noch nicht vorhanden ist
 *
 * \param salt char**   Pointer auf einen Salt-pointer
 * \return bool         true: Salz gefunden ; false: Salz nicht gefunden
 *
 */
bool salt_exists(char ** salt){

	char * seekSalt=calloc(SALT_LENGTH+1, sizeof(char));
	char * query=NULL;
	MYSQL * my=NULL;
	MYSQL_RES * result=NULL;

	strncpy(seekSalt, *salt, SALT_LENGTH);

	if(asprintf(&query, "SELECT passwort FROM Benutzer WHERE passwort REGEXP '^%s'", seekSalt) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (salt_exists)");
	}

	my=mysql_init(NULL);

	if(my == NULL){
		print_exit_failure("MYSQL init failure\n Wörk!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}


	if(mysql_query(my, query)){
		print_exit_failure("mysql_query failed (search salt)");
		#ifdef DEBUG
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		#endif // DEBUG
	}else{
		result = mysql_store_result(my);

		if(mysql_num_rows(result) > 0){
			#ifdef DEBUG
			fprintf(stderr, "Salz gefunden, wörk\n");
			#endif // DEBUG
			mysql_free_result(result);
			mysql_close(my);
			free(query);
			return true;
		}
	}

	mysql_free_result(result);
	mysql_close(my);

	free(query);
	return false;
}

/** \brief Prüfen ob eine bestimmte E-mail schon in der Datenbank existiert
 *
 * \param name char*   Name der gesucht werden soll
 * \return bool        true: Name gefunden; false: Name nicht gefunden
 *
 */
bool email_exists(char * email){
	char * query=NULL;
	MYSQL * my=NULL;
	MYSQL_RES * result=NULL;


	if(email == NULL){
		print_exit_failure("Programm falsch, (email_exists)");
	}

	if(asprintf(&query, "SELECT email FROM Benutzer WHERE email='%s'", email) == -1){
        print_exit_failure("Es konnte kein Speicher angefordert werden (email_exists)");
	}

	my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}


	if(mysql_query(my, query)){
		#ifdef DEBUG
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		#endif // DEBUG
		print_exit_failure("mysql_query failed (email_exists)");
	}else{
		result = mysql_store_result(my);
		if(mysql_num_rows(result) > 0){
			#ifdef DEBUG
			fprintf(stderr, "Benutzer gefunden, wörk\n");
			#endif // DEBUG
			mysql_free_result(result);
			mysql_close(my);
			free(query);
			return true;
		}else{
			mysql_free_result(result);
			mysql_close(my);
			free(query);
			return false;
		}
	}
	mysql_free_result(result); //Was wenn result NULL ist?
	mysql_close(my);
	free(query);
	return false;
}


/** \brief Prüfen ob ein eingegebenes Kürzel schon in der Datenbank existiert
 *
 * \param acronym char*    Kürzel
 * \return bool            true: existiert; false: existiert nicht (gut)
 *
 */
bool acronym_exists(char * acronym){
	char * query=NULL;
	MYSQL * my=NULL;
	MYSQL_RES * result=NULL;

	if(acronym == NULL){
		print_exit_failure("Programm falsch, Wörk!");
	}

	if(asprintf(&query, "SELECT kuerzel FROM Benutzer WHERE kuerzel='%s'", acronym) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (acronym_exists)");
	}

	my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}


	if(mysql_query(my, query)){
		#ifdef DEBUG
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		print_exit_failure("mysql_query failed (acronym_exists)");
		#endif // DEBUG
	}else{
		result = mysql_store_result(my);

		if(mysql_num_rows(result) > 0){
			#ifdef DEBUG
			fprintf(stderr, "Benutzer gefunden, wörk\n");
			#endif // DEBUG
			mysql_free_result(result);
			mysql_close(my);
			free(query);
			return true;
		}else{
			mysql_free_result(result);
			mysql_close(my);
			free(query);
			return false;
		}
	}
	mysql_free_result(result);
	mysql_close(my);
	free(query);
	return false;

}

/** \brief Eine neue SID für einen Benutzer erzeugen
 *
 * \param pers person*   Person für die die Sitzung angelegt werden soll (enthält die id)
 * \return int           ???
 *
 */
int create_session(person * pers){
    char * query=NULL;
    MYSQL * my=NULL;
	MYSQL_RES * result=NULL;

	srand(time(NULL));
	int generated_sid=rand();
	while((sid_exists(generated_sid)) ^ (generated_sid==0)){
        generated_sid=rand();
	}

	pers->sid=generated_sid;

	if(asprintf(&query, "UPDATE Benutzer SET sid='%d' WHERE id='%d'", pers->sid, pers->id) == -1){
        print_exit_failure("Es konnte kein Speicher angefordert werden (create_session)");
	}

	my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_ALTERNATE_USER, SQL_ALTERNATE_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		#ifdef DEBUG
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		#endif // DEBUG
		print_exit_failure("mysql_query failed (create_session)");
	}
    mysql_close(my);

	free(query);
	return 0;
}


/** \brief Prüfen ob die sid noch nicht vorhanden ist
 *
 * \param sid int       zu überprüfende sid
 * \return bool         true: sid gefunden ; false: sid nicht gefunden (gut)
 *
 */
bool sid_exists(int sid){

	char * query=NULL;
	MYSQL * my=NULL;
	MYSQL_RES * result=NULL;

	if(asprintf(&query, "SELECT sid FROM Benutzer WHERE sid='%d'", sid) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (sid_exists)");
	}

	my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure\n Wörk!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		print_exit_failure("mysql_query failed (search sid)");
		#ifdef DEBUG
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		#endif // DEBUG
	}else{
		result = mysql_store_result(my);

		if(mysql_num_rows(result) > 0){
			#ifdef DEBUG
			fprintf(stderr, "sid gefunden, wörk\n");
			#endif // DEBUG
			mysql_free_result(result);
			mysql_close(my);
			free(query);
			return true;
		}
	}

	mysql_free_result(result);
	mysql_close(my);

	free(query);
	return false;
}

/** \brief SID des Benutzers löschen (--> Abmelden)
 *
 * \param pers person * Person, die abzumelden ist
 * \return bool         true: sid gefunden ; false: sid nicht gefunden
 *
 */
bool sid_set_null(person * pers){
	char * query=NULL;
	MYSQL * my=NULL;
	bool success=false;

	if(asprintf(&query, "UPDATE Benutzer SET sid=NULL WHERE email='%s' AND sid='%d'", pers->email, pers->sid) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (sid_set_null)");
	}

	my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_ALTERNATE_USER, SQL_ALTERNATE_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		#ifdef DEBUG
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		#endif // DEBUG
		success=false;
	}else{
		success=true;
	}
	mysql_close(my);
	free(query);
	return success;
}

/** \brief Überprüfen, ob ein bestimmter Nutzer ein SID hat
 *
 * \param pers person*  Person mit E-mail und SID
 * \return bool         true:  wenn E-mail und SID vorhanden sind --> Benutzer darf dinge tun
 *                      false: wenn SID und email nicht bei dem Benutzer sind --> Benutzer darf nichts
 *
 */
bool verify_sid(person * pers){
	char * query=NULL;
	MYSQL * my=NULL;

	if(asprintf(&query, "SELECT * FROM Benutzer WHERE email='%s' AND sid='%d'", pers->email, pers->sid) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (verify_sid)");
	}

	my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		print_exit_failure("mysql_query failed (verify_sid)");
		#ifdef DEBUG
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		#endif // DEBUG
	}else{
		MYSQL_RES * result=NULL;
		result = mysql_store_result(my);

		if(mysql_num_rows(result) > 0){  //TODO Wohl doch eher == 1 (es muss ja genau 1 sein)
			#ifdef DEBUG
			fprintf(stderr, "sid gefunden\n");
			#endif // DEBUG
			pers->auth=true;
		}else{
			pers->auth=false;
		}
		mysql_free_result(result);
	}

    mysql_close(my);
    free(query);
	return pers->auth;
}

/** \brief 5 Nachrichten holen
 *
 * \param mes message ** message*   Pointer auf ein Array aus max. GET_MESSAGE_COUNT Nachrichten
 * \param offset int                Alle 5 Nachrichten ab der offset*GET_MESSAGE_COUNT-ten Nachricht holen
 * \return int                      Tatsächliche Anzahl an Meldungen
 *
 */

int get_messages(message ** mes, int offset, char * select_course){

	char * query=NULL;
	int num=0;
	MYSQL * my=NULL;

	if(select_course){
		if(asprintf(&query, "SELECT * FROM Meldungen WHERE kurse REGEXP '(^|, )%s($|, )'  ORDER BY erstellt DESC LIMIT %d OFFSET %d", select_course, GET_MESSAGE_COUNT,(offset*GET_MESSAGE_COUNT)) == -1){
			print_exit_failure("Es konnte kein Speicher angefordert werden (get_all_messages)");
		}
	}else{
		if(asprintf(&query, "SELECT * FROM Meldungen WHERE kurse='all' ORDER BY erstellt DESC LIMIT %d OFFSET %d", GET_MESSAGE_COUNT,(offset*GET_MESSAGE_COUNT)) == -1){
			print_exit_failure("Es konnte kein Speicher angefordert werden (get_all_messages)");
		}
	}

	my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		print_exit_failure("mysql_query failed (get_messages)");
		#ifdef DEBUG
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		#endif // DEBUG
	}else{
		MYSQL_RES * result=NULL;
		result = mysql_store_result(my);
		num=mysql_num_rows(result);
		if(mysql_num_rows(result) > 0){
			*mes = calloc(mysql_num_rows(result), sizeof(message));
			MYSQL_ROW message_row;
			for(my_ulonglong i=0; i<mysql_num_rows(result) && (message_row=mysql_fetch_row(result)); i++){
				(*mes+i)->id=atoi(message_row[COL_MESSAGE_ID]);

				//(*mes+i)->title=calloc(strlen(message_row[COL_MESSAGE_TITEL])+1, sizeof(char));
				//strcpy((*mes+i)->title, message_row[COL_MESSAGE_TITEL]);
				asprintf(&(*mes+i)->title, "%s", message_row[COL_MESSAGE_TITEL]);

				//TODO: nur den ersten Satz / die ersten n Zeichen ausgeben oder bis zum ersten <br> ???
				//(*mes+i)->message=calloc(strlen(message_row[COL_MESSAGE_MES])+1, sizeof(char));
				//strcpy((*mes+i)->message, message_row[COL_MESSAGE_MES]);
				asprintf(&(*mes+i)->message, "%s", message_row[COL_MESSAGE_MES]);

				//(*mes+i)->courses=calloc(strlen(message_row[COL_MESSAGE_COURSES])+1, sizeof(char));
				//strcpy((*mes+i)->courses, message_row[COL_MESSAGE_COURSES]);
				asprintf(&(*mes+i)->courses, "%s", message_row[COL_MESSAGE_COURSES]);

				(*mes+i)->creator_id=atoi(message_row[COL_MESSAGE_CREATORID] ? message_row[COL_MESSAGE_CREATORID] : "-1");

				//(*mes+i)->s_created=calloc(strlen(message_row[COL_MESSAGE_TIME_CREATED])+1, sizeof(char));
				//strcpy((*mes+i)->s_created, message_row[COL_MESSAGE_TIME_CREATED]);
				asprintf(&(*mes+i)->s_created, "%s", message_row[COL_MESSAGE_TIME_CREATED]);

            }
		}
		mysql_free_result(result);
	}
	mysql_close(my);

	free(query);
	return num;
}

/** \brief Anhand der ID der Person die restliche Information über sie herausfinden
 *
 * \param person*  Personenobjekt das die id enthält und in das die Daten geschrieben werden
 * \return bool    true: Person gefunden; false: Person nucht gefunden
 */
bool get_person_by_id(person * pers){

	if(pers->id < 1)return NULL;
	char * query=NULL;
	MYSQL * my=NULL;
	bool found=false;

	if(asprintf(&query, "SELECT * FROM Benutzer WHERE id='%d'", pers->id) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (get_person_by_id)");
	}

	my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		print_exit_failure("mysql_query failed (get_person_by_id)");
		#ifdef DEBUG
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		#endif // DEBUG
	}else{
		MYSQL_RES * result=NULL;
		result = mysql_store_result(my);

		if(mysql_num_rows(result) > 0){
			#ifdef DEBUG
			fprintf(stderr, "uid gefunden\n");
			#endif // DEBUG

			MYSQL_ROW row;
			row=mysql_fetch_row(result);

			//Name holen
			pers->name=calloc(strlen(row[COL_NAME])+1, sizeof(char));
			strcpy(pers->name, row[COL_NAME]);
			pers->first_name=calloc(strlen(row[COL_VORNAME])+1, sizeof(char));
			strcpy(pers->first_name, row[COL_VORNAME]);

			//Kürzel (falls vorhanden) holen
			if(row[COL_ACR] != NULL){
				//Die Person hat ein Küzel --> Lehrer
				pers->acronym=calloc(strlen(row[COL_ACR])+1, sizeof(char));
				strcpy(pers->acronym, row[COL_ACR]);
				pers->isTeacher=true;
			}else{
				pers->isTeacher=false;
			}

			//Kurse (falls vorhanden)
			if(row[COL_COURSE] != NULL){
				pers->courses=calloc(strlen(row[COL_COURSE])+1, sizeof(char));
				strcpy(pers->courses, row[COL_COURSE]);
			}

			//SID (falls vorhanden)
			if(row[COL_SID] != NULL){
				pers->sid=atoi(row[COL_SID]);
			}

			found=true;
		}
		mysql_free_result(result);
	}

    mysql_close(my);
    free(query);
    return found;
}

/** \brief Personendaten anhand der SID und E-mail abrufen
 *
 * \param pers person*  Person mit E-mail und SID in der die restlichen Daten gespeichert werden
 * \return bool         true: Person gefunden; false: Person nicht gefunden
 *
 */
bool get_person_by_sid(person * pers){
	char * query=NULL;
	MYSQL * my=NULL;
	bool found=false;

	if(asprintf(&query, "SELECT * FROM Benutzer WHERE sid='%d' AND email='%s'", pers->sid, pers->email) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (get_person_by_sid)");
	}

	my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		print_exit_failure("mysql_query failed (get_person_by_sid)");
		#ifdef DEBUG
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		#endif // DEBUG
	}else{
		MYSQL_RES * result=NULL;
		result = mysql_store_result(my);

		if(mysql_num_rows(result) > 0){
			#ifdef DEBUG
			fprintf(stderr, "uid gefunden\n");
			#endif // DEBUG

			MYSQL_ROW row;
			row=mysql_fetch_row(result);

			//Name holen
			pers->name=calloc(strlen(row[COL_NAME])+1, sizeof(char));
			strcpy(pers->name, row[COL_NAME]);
			pers->first_name=calloc(strlen(row[COL_VORNAME])+1, sizeof(char));
			strcpy(pers->first_name, row[COL_VORNAME]);

			//Kürzel (falls vorhanden) holen
			if(row[COL_ACR] != NULL){
				//Die Person hat ein Küzel --> Lehrer
				pers->acronym=calloc(strlen(row[COL_ACR])+1, sizeof(char));
				strcpy(pers->acronym, row[COL_ACR]);
				pers->isTeacher=true;
			}else{
				pers->isTeacher=false;
			}

			//Kurse (falls vorhanden)
			if(row[COL_COURSE] != NULL){
				pers->courses=calloc(strlen(row[COL_COURSE])+1, sizeof(char));
				strcpy(pers->courses, row[COL_COURSE]);
			}

			//ID holen
			if(row[COL_ID] != NULL){
				pers->id=atoi(row[COL_ID]);
			}

			found=true;
		}
		mysql_free_result(result);
	}

    mysql_close(my);
    free(query);
    return found;
}

/** \brief Eine Nachricht in die Datenbank einfügen
 *
 * \param mes message*  Nachricht
 * \return bool         true: es hat funktioniert; false: es hat nicht funktoiniert
 *
 */
bool insert_message(message * mes){
	char * query=NULL;
	char * time_created=NULL;
	MYSQL * my=NULL;
	bool success=false;

	if(strlen(mes->title)<2 || strlen(mes->message)<2)print_exit_failure("Geben Sie eine Meldung ein!!");

	clean_string(mes->message);
	clean_string(mes->title);
	mes->message=nlcr_to_htmlbr(mes->message);
	#ifdef DEBUG
	fprintf(stderr, "\n\n Meldung: '%s'", mes->message);
	#endif // DEBUG
	remove_newline(mes->message);

	time_created=calloc(21, sizeof(char)); // 20
	strftime(time_created, 20, "%Y-%m-%d %H:%M:%S", mes->created);
	//                           4  2  2  2  2  2 ---> ? 19 +1 (\0) --> 20 = ????

	if(asprintf(&query, "INSERT INTO Meldungen \
				(titel, meldung, kurse, erstellerID, erstellt)\
				VALUES('%s', '%s', '%s', '%d', '%s')",
				mes->title, mes->message, mes->courses, mes->creator_id, time_created  ) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (insert_message)");
	}
	#ifdef DEBUG
	fprintf(stderr, "\n\nQuery (insert_message) : '%s'\n\n", query);
	#endif // DEBUG

	my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_ALTERNATE_USER, SQL_ALTERNATE_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		#ifdef DEBUG
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		#endif // DEBUG

		success=false;
	}else{
		success=true;
	}
	mysql_close(my);
	free(query);
	return success;
}

/** \brief	Alle verfügbaren Kurse in alphabetischer Reihenfolge in dass Array "c" speichern
 *
 * \param c course**   Array, in dem die Kurse abgelegt werden (muss vorher NULL sein)
 * \return size_t      Anzahl der Kurse (Größe des Arrays)
 *
 */
size_t get_distinct_courses(course ** c){
	char * query=NULL;
	MYSQL *my=NULL;
	size_t number=0;

	if(asprintf(&query, "SELECT DISTINCT name FROM Kurse ORDER BY name") == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (get_all_courses)");
	}

	my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		#ifdef DEBUG
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		#endif // DEBUG
		number=0;
	}else{

		MYSQL_RES * result=NULL;
		result = mysql_store_result(my);

		if(mysql_num_rows(result) > 0){
			number=mysql_num_rows(result);
			*c=calloc(number, sizeof(course));
			MYSQL_ROW row;
			size_t i=0;
			while((row=mysql_fetch_row(result)) && i<number){
				init_course((*c+i));
				asprintf(&(*c+i)->name, "%s", row[0]);  //0, da nur eine Spalte erwartet wird (^ Siehe query)
				i++;
			}
		}
		mysql_free_result(result);
	}
	mysql_close(my);
	free(query);
	return number;
}

/** \brief  Die Kursliste des Benutzers in die Datenbank schreiben
 *
 * \param pers person*  Person mit Kursliste
 * \return bool         true: Erfolg; false: kein Erfolg
 *
 */
bool update_user_courses(person * pers){
	char * query=NULL;
	MYSQL * my=NULL;
	bool success=false;

	if(pers->courses == NULL){
		print_exit_failure("Programm falsch (update_user_courses)");
	}

	if(asprintf(&query, "UPDATE Benutzer SET kurse='%s' WHERE id='%d' AND email='%s'", pers->courses, pers->id, pers->email) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (update_user_courses)");
	}

	my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_ALTERNATE_USER, SQL_ALTERNATE_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		#ifdef DEBUG
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		#endif // DEBUG
		success=false;
	}else{
		success=true;
	}
	mysql_close(my);
	free(query);
	return success;
}

/** \brief  Die Kursliste des Benutzers in die Datenbank schreiben
 *
 * \param pers person*     Person mit Login-Daten
 * \param new_email char*  Neue E-Mail-Adresse
 * \return bool            true: Erfolg; false: kein Erfolg
 *
 */
bool update_user_email(person * pers, char * new_email){
	char * query=NULL;
	MYSQL *my=NULL;
	bool success=false;

	if(pers->email == NULL){
		print_exit_failure("Programm falsch (update_user_email)");
	}

	if(asprintf(&query, "UPDATE Benutzer SET email='%s' WHERE id='%d' AND email='%s'", new_email, pers->id, pers->email) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (update_user_email)");
	}

	my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_ALTERNATE_USER, SQL_ALTERNATE_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		#ifdef DEBUG
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		#endif // DEBUG
		success=false;
	}else{
		success=true;
	}
	mysql_close(my);
	free(query);
	return success;
}

/** \brief Das Passwort eines Nutzers ändern
 *
 * \param pers person*  Personen-Objekt, mit Passwort
 * \return bool         true: Änderung erfolgreich; false: Änderung fehlgeschlagen
 *
 */
bool update_user_password(person * pers){
	char * query=NULL;
	char * salt=NULL;
	char * arg=NULL; // für crypt
	char * store_pw=NULL;
	MYSQL * my=NULL;
	bool success=false;

	if(pers->password == NULL){
		print_exit_failure("Programm falsch (update_user_password)");
	}


	salt_generate(&salt);

    //Verhindern, dass ein bereits vorhandenes Salt zweimal verwendet wird (falls zwei Nutzer identische Passwörter wählen)
	while(salt_exists(&salt)){
		salt_generate(&salt);
	}
	asprintf(&arg, "$6$%s$", salt);

	char * encr=crypt(pers->password, arg);
	asprintf(&store_pw, "%s%s", salt, encr+strlen(arg));
	free(arg);
	free(salt);
	free(pers->password);


	if(asprintf(&query, "UPDATE Benutzer SET passwort='%s' WHERE id='%d' AND email='%s'", store_pw, pers->id, pers->email) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (update_user_password)");
	}

	my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_ALTERNATE_USER, SQL_ALTERNATE_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		#ifdef DEBUG
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		#endif // DEBUG

		success=false;
	}else{
		success=true;
	}

	mysql_close(my);
	free(query);
	return success;
}

/** \brief Alle Vorkommen + Daten (Zeiten, Räume IDs) eines Kurses holen
 *
 * \param this_course char*  String mit Name des Kurses (Kursbezeichnung)
 * \param c_arr course**     Array in dem die Kurse mit allen Daten gespeichert werden
 * \return size_t            Anzahl der Kurse in c_arr, 0: wenn der Kurs nicht gefunden wurde
 *
 */
size_t get_course(char * this_course, course ** c_arr){

	size_t number=0;
	MYSQL *my=NULL;
	char * query=NULL;

	if(this_course==NULL || c_arr==NULL){
		print_exit_failure("Programm falsch (get_course)");
	}

	if(asprintf(&query, "SELECT * FROM Kurse WHERE name='%s'", this_course) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (update_user_courses)");
	}

	my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		#ifdef DEBUG
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		#endif // DEBUG
		number=0;
	}else{

		MYSQL_RES * result=NULL;
		result = mysql_store_result(my);

		if(mysql_num_rows(result) > 0){
			number=mysql_num_rows(result);
			*c_arr=calloc(number, sizeof(course));
			MYSQL_ROW row;
			size_t i=0;
			while((row=mysql_fetch_row(result)) && i<number){
				init_course((*c_arr+i));
				asprintf(&(*c_arr+i)->name, "%s", row[COL_COURSE_NAME]);
				asprintf(&(*c_arr+i)->id, "%s", row[COL_COURSE_ID]);
				asprintf(&(*c_arr+i)->time, "%s", row[COL_COURSE_TIME]);
				asprintf(&(*c_arr+i)->room, "%s", row[COL_COURSE_ROOM]);
				i++;
			}
		}
		mysql_free_result(result);
	}
	mysql_close(my);
	free(query);
	return number;
}

/** \brief Anhand einer Kursbezeichnung herausfinden, welcher Lehrer diesen unterrichtet
 *
 * \param pers person*   Personen-Objekt, in dem die gefunden Person gespeichert wird
 * \param c char*        Kursbezeichnung
 * \return bool          true:  es gibt genau eine Person, die diesen Kurs unterrichtet
                         false: Fehler, oder mehre Personen unterrichtet den Kurs
 *
 */
bool get_teacher_by_course(person * pers, char * c){
	char * query=NULL;
	MYSQL * my=NULL;
	bool success=false;
	if(pers == NULL || c==NULL){
		print_exit_failure("Programm falsch (get_teacher_by_course)");
	}

	if(asprintf(&query, "SELECT * FROM Benutzer WHERE kurse REGEXP '(^|, )%s($|, )' AND kuerzel IS NOT NULL", c) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (update_user_courses)");
	}

	my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		#ifdef DEBUG
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		#endif // DEBUG
		mysql_close(my);
		free(query);
		success=false;
	}else{
		MYSQL_RES * result=NULL;
		result = mysql_store_result(my);

		if(mysql_num_rows(result) == 1){
			MYSQL_ROW row;
			row=mysql_fetch_row(result);

			//Name holen
			asprintf(&pers->name, "%s", row[COL_NAME]);
			asprintf(&pers->first_name, "%s", row[COL_VORNAME]);

			asprintf(&pers->acronym, "%s", row[COL_ACR]);
			pers->isTeacher=true;

			//Kurse
			asprintf(&pers->courses, "%s", row[COL_COURSE]);

			//ID holen
			pers->id=atoi(row[COL_ID]);

			mysql_free_result(result);
			success=true;

		}
	}
	mysql_close(my);
	free(query);

	return success;
}


/**
 * ----------------------------------------------------------------------------
 * Ab hier: Funktionen, die nicht direkt mit mysql zu tun haben,
 * aber dennoch häufig gemeinsam mit den obigen Funktionen aufgerufen werden
 * ----------------------------------------------------------------------------
*/
//Sollte man nicht alle html-bezogenen Funktionen in eine eigen Datei auslagern
//TODO FEHELER!!!!!!! ( Eckige Klammer-auf geht durch ????
/** \brief Einen String von "geföhrlichen" Zeichen befreien (nur bestimmtes Durchlassen)
 *
 * \param str char*  String der Verändert werden soll
 * \return void
 *
 */
void clean_string(char * str){
	regex_t reg;
	regcomp(&reg, "[^A-Za-z0-9 #@\n\rÄÖÜäöüßéèêáàâíìîóòôúùûÉÈÊÁÀÂÍÌÎÓÒÔÚÙÛ!?(!?(),.-]]*", REG_EXTENDED); //Nur diese Zeichen sind erlaubt
	size_t str_len=strlen(str)+1;
	regmatch_t * pmatch=calloc(str_len, sizeof(regmatch_t));
	#ifdef DEBUG
	fprintf(stderr, "String : '%s'\n", str);
	#endif // DEBUG

	while(regexec(&reg, str, str_len, pmatch, REG_NOTBOL) == 0){
		int i=(int)pmatch->rm_so;
		str[i]=' ';
	}
}

bool course_regex_search(course * c, char * all_courses){
	regex_t reg;
	char * reg_string=NULL;
	asprintf(&reg_string, "(,| |^)%s(,|$)", c->name);
	bool match=false;

	int test=regcomp(&reg, reg_string, REG_EXTENDED);

	size_t str_len=strlen(all_courses)+1;
	regmatch_t * pmatch=calloc(str_len, sizeof(regmatch_t));
	#ifdef DEBUG
	fprintf(stderr, "(course_regex_search) String : '%s'\n", all_courses);
	#endif // DEBUG
	int res=regexec(&reg, all_courses, str_len, pmatch, 0); // 0 !?
	if(res == 0){
		match=true;
	}else{
		match=false;
	}

	return match;
}

/** \brief Alle \r\n (0x0D 0x0A durch <br> ersetzen (Funktion ist rekursiv)
 *
 * \param str char*  String der \r\n enthält
 * \return char*     String der <br> enthält
 *
 */
char * nlcr_to_htmlbr(char * str){

	if(str == NULL){
		print_exit_failure("Programm falsch (nlcr_to_htmlbr)");
	}

	char * found=strstr(str, "\r\n");  //CR LF finden (%0D%0A) (machen alle Browser das so?) (IE, Safari, opera?)
	char * out=NULL;
	if(found){
		for(char * i=str;i[0]!= found[0]; i++){
			#ifdef DEBUG
			fprintf(stderr, "for_loop: str: '%s', found '%s', i '%c'\n", str, found, i[0]);
			#endif // DEBUG
			asprintf(&out, "%s%c", out ? out : "", i[0]);
		}
	}else{
		asprintf(&out, "%s%s", out ? out : "", str);
	}

	asprintf(&out, "%s<br>%s", out ? out : "", (found ? nlcr_to_htmlbr(found+2) : "") );
	return out;
}

/** \brief 	Einen String der durch Kommata getrennt ist ("1D1, 1M1, 1E5" usw.) in die einzelnen Bestandteil zerlegen
 *			und seine in einem Array speichern
 *
 * \param comma_char char*	Durch Kommata getrennter String
 * \param str_array char***	Array mit einzelnen STrings
 * \return int
 *
 */
int comma_to_array(char * comma_char, char *** str_array){
	if(comma_char == NULL){
		print_exit_failure("Programm falsch (comma_to_array)");
	}
	char * local_comma_char=NULL;
	asprintf(&local_comma_char, "%s", comma_char);
	int j;
	for (j=0; ; j++, local_comma_char = NULL) {
		char * token = strtok(local_comma_char, ",");
		if (token == NULL)break;
		//printf("%d: %s\n", j, token);

	   //asprintf(str_array+j, "%s", token);
	}
	asprintf(&local_comma_char, "%s", comma_char);
	/*char ** local_str_array*/*str_array=calloc(j,sizeof(char *));
	for (j=0; ; j++, local_comma_char = NULL) {
		char * token = strtok(local_comma_char, ", ");
		if (token == NULL)break;
		//printf("%d: %s\n", j, token);

		asprintf((&(*(/*local_*/*str_array+j))), "%s", token);
	}
	//str_array=local_str_array;
	return j;
}


char * salt_extract(char * db_passwd){
	char * salt=NULL;
	if(db_passwd != NULL){
		salt=calloc(SALT_LENGTH+1, sizeof(char));
		for(int i=0; i<SALT_LENGTH; i++){
			strncat(salt, db_passwd+i, 1);
		}
	}
	return salt;
}
