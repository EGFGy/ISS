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

void free_person(person * p){
	if(p){
		if(p->acronym){
			free(p->acronym);
			p->acronym=NULL;
		}
		if(p->courses){
			free_course_set(p->courses);
			p->courses=NULL;
		}
		if(p->email){
			free(p->email);
			p->email=NULL;
		}
		if(p->first_name){
			free(p->first_name);
			p->first_name=NULL;
		}
		if(p->name){
			free(p->name);
			p->name=NULL;
		}
		if(p->login_time){
			free(p->login_time);
			p->login_time=NULL;
		}
		if(p->password){
			free(p->password);
			p->password=NULL;
		}
		p->auth=false;
		p->id=0;
		p->isTeacher=false;
		p->sid=0;
	}
}

/** \brief Eine Nachricht initialisieren
 *
 * \param mes message* zu initialisierende Nachricht
 * \return void
 *
 */
void init_message(message * mes){
	mes->course_id=0;
	mes->created=NULL;
	mes->creator_id=0;
	mes->id=0;
	mes->message=NULL;
	mes->title=NULL;
	mes->s_created=NULL;
}

/** \brief Eine Nachricht löschen (alle Stirngs)
 *
 * \param mes message* zu löschende Nachricht
 * \return void
 *
 */
void free_message(message * mes){
	mes->course_id=0;
	if(mes->message){
		free(mes->message);
		mes->message=NULL;
	}
	if(mes->s_created){
		free(mes->s_created);
		mes->s_created=NULL;
	}
	if(mes->title){
		free(mes->title);
		mes->title=NULL;
	}
	mes->creator_id=0;
	mes->id=0;
}

/** \brief Ein message_set initialisieren
 *
 * \param m message_set* zu initialisierendes message_set
 * \return void
 *
 */
void init_message_set(message_set * m){
	m->all_messages=NULL;
	m->cnt=0;
}

/** \brief Ein message_set löschen (alle Nachrichten und deren Strings)
 *
 * \param m message_set* zu löschendes message_set
 * \return void
 *
 */
void free_message_set(message_set * m){
	if(m){
		if(m->cnt > 0 && m->all_messages){
			for(size_t i = 0; i < m->cnt; i++){
				free_message((m->all_messages+i));
			}
			free(m->all_messages);
			m->all_messages=NULL;
		}
	}
}

/** \brief Einen Kurs initialisieren
 *
 * \param c course*  zu initialisierender Kurs
 * \return void
 *
 */
void init_course(course * c){
	c->id=0;
	c->name=NULL;
	c->time_day=0;
	c->time_hour=0;
	c->room=NULL;

	c->teacher=NULL;

	c->alter_time=NULL;
	c->alter_room=NULL;
	c->alter_teacher_acronym=NULL;
	c->status=UNCHANGED;
}

/** \brief Einen Kurs löschen (ALLES: alle Strings und die Person)
 *
 * \param c course*  zu löschender Kurs
 * \return void
 *
 */
void free_course(course * c){
	if(c->alter_room){
		free(c->alter_room);
		c->alter_room=NULL;
	}
	if(c->alter_teacher_acronym){
		free(c->alter_teacher_acronym);
		c->alter_teacher_acronym=NULL;
	}
	if(c->alter_time){
		free(c->alter_time);
		c->alter_time=NULL;
	}
	if(c->name){
		free(c->name);
		c->name=NULL;
	}
	if(c->original_time){
		free(c->original_time);
		c->original_time=NULL;
	}
	if(c->room){
		free(c->room);
		c->room=NULL;
	}
	if(c->teacher){
		free_person(c->teacher); // wie ist das? Es könnte ja sein, dass man den Lehrer noch braucht
		free(c->teacher);
		c->teacher=NULL;
	}
	c->time_day=0;
	c->time_hour=0;
	c->id=0;
	c->status=0;
}

/** \brief Ein course_set initialisieren
 *
 * \param c course_set*  zu initialisierendes course_set
 * \return void
 *
 */
void init_course_set(course_set * c){
	c->c_set=NULL;
	c->number=0;
}


/** \brief Das course_set löschen (ALLES: alle Strings in den Kursen, die Personen der Kurse)
 *         ACHTUNG: in mehreren Kursen darf es keine ointer geben die auf nur eine Person zeigen
 * \param c course_set*  course_set dessen Inhalt gelöscht werden soll
 * \return void
 *
 */
void free_course_set(course_set * c){
	if(c){
		if(c->number > 0){
			for(size_t i = 0; i < c->number; i++){
				free_course((c->c_set+i));
				//free(c->c_set+i);
			}
			free(c->c_set);
			c->c_set=NULL;
		}else{
			#ifdef DEBUG
			fprintf(stderr, "Ein 'course_set' mit 0 Einträgen sollte gelöscht werden\n");
			#endif // DEBUG
		}
	}
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
		print_exit_failure("Programm falsch! (verify_user)");
	}

	isAcronym=detect_convert_acronym(pers);

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure (verify_user)");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){

		print_exit_failure("MYSQL-connection error! (verify_user)");
	}else{
		//fprintf(stderr, "Connection extablished!\n");
	}

	MYSQL_RES * result=NULL;
	bool found=false;

	if(isAcronym){
		//Es ist sicher eine Lehrer (jemand hat das Kürzel eingegeben)
		//TODO: sql-injection verhindern

		char * sql_query=NULL;
		if(asprintf(&sql_query, "SELECT * FROM Benutzer WHERE kuerzel='%s'", pers->acronym) == -1){
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
		/*for(int i=0; i<SALT_LENGTH; i++){
			strncat(salt, row[COL_PASS]+i, 1);
		}*/
		strncat(salt, row[COL_PASS], SALT_LENGTH);

		char * arg=NULL;
		asprintf(&arg, "$6$%s$", salt);
		char * encr=crypt(pers->password, arg);
		free(pers->password); pers->password=NULL;
		char * load_pw=NULL;
		asprintf(&load_pw, "%s%s", salt, encr+strlen(arg));

		free(arg); arg=NULL;
		free(salt); salt=NULL;

		//pers->password=encr+strlen(arg);

		if(strcmp(load_pw, row[COL_PASS]) == 0){
			pers->auth=true;

			//Name holen
			asprintf(&pers->name, "%s", row[COL_NAME]);
			asprintf(&pers->first_name, "%s", row[COL_VORNAME]);

			if(isAcronym){
				//Person hat Kürzel angegeben --> es ist eine Leherer --> email holen holen
				asprintf(&pers->email, "%s",row[COL_EMAIL]);
			}else{
				//Person hat ihre Email-Adresse statt dem Kürzel angegeben --> (Falls es ein Lehrer ist, dessen Kürzel holen)
				pers->acronym=NULL;
				if(row[COL_ACR] != NULL){
					//Die Person hat ein Küzel --> Lehrer
					asprintf(&pers->acronym, "%s", row[COL_ACR]);
					pers->isTeacher=true;
				}else{
					pers->isTeacher=false;
				}
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
		free(load_pw);
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
			MYSQL_ROW row=NULL;
			row=mysql_fetch_row(result); //warning: assignment from incompatible pointer type (WTF?)
			#ifdef DEBUG
			fprintf(stderr, "Benutzer gefunden (verify_user_password)\n");
			#endif // DEBUG
			char * password_encrypted=NULL;
			char * password_db=NULL;
			char * salt=NULL;
			password_db=strdup(row[COL_PASS]);

			salt=salt_extract(password_db);

			char * arg=NULL;
			asprintf(&arg, "$6$%s$", salt);
			char * encr=crypt(pers->password, arg);
			//free(pers->password);
			asprintf(&password_encrypted, "%s%s", salt, encr+strlen(arg));

			free(salt);
			free(arg);
			//free(encr);

			if(strcmp(password_db, password_encrypted) == 0){
				#ifdef DEBUG
				fprintf(stderr, "Passwort war richtig! (Benutzer: %s)", pers->email);
				#endif // DEBUG
				password_state=true;
			}else{
				password_state=false;
			}
			free(password_encrypted);
			free(password_db);
			password_encrypted=NULL;
			password_db=NULL;
			//password_db=NULL;

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
	bool isAcronym=false;

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

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
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
	fclose(f_random);
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
	free(seekSalt);

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

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
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
	bool success=false;

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
		success=false;
	}else{
		result = mysql_store_result(my);

		if(mysql_num_rows(result) > 0){
			#ifdef DEBUG
			fprintf(stderr, "sid gefunden, wörk\n");
			#endif // DEBUG

			success=true;
		}else{
			success=false;
		}
	}

	mysql_free_result(result);
	mysql_close(my);
	free(query);

	return success;
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

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
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

/** \brief
 *
 * \param mes message**            Pointer auf ein Array aus max. GET_MESSAGE_COUNT Nachrichten (wird in der Funktion alloziert)
 * \param offset int               Alle 5 Nachrichten ab der offset*GET_MESSAGE_COUNT-ten Nachricht holen
 * \param select_course char*
 * \return int                     Tatsächliche Anzahl an Meldungen
 *
 */
bool get_messages(message_set * mes, int offset, char * select_course){
	bool success=false;

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
		num = mysql_num_rows(result);
		if(mysql_num_rows(result) > 0){
			mes->all_messages = calloc(mysql_num_rows(result), sizeof(message));
			MYSQL_ROW message_row;
			for(my_ulonglong i=0; i<mysql_num_rows(result) && (message_row=mysql_fetch_row(result)); i++){
				(mes->all_messages+i)->id=atoi(message_row[COL_MESSAGE_ID]);

				asprintf(&(mes->all_messages+i)->title, "%s", message_row[COL_MESSAGE_TITEL]);
				asprintf(&(mes->all_messages+i)->message, "%s", message_row[COL_MESSAGE_MES]);
				asprintf(&(mes->all_messages+i)->courses, "%s", message_row[COL_MESSAGE_COURSES]);

				(mes->all_messages+i)->creator_id=atoi(message_row[COL_MESSAGE_CREATORID] ? message_row[COL_MESSAGE_CREATORID] : "-1");

				asprintf(&(mes->all_messages+i)->s_created, "%s", message_row[COL_MESSAGE_TIME_CREATED]);

            }
            mes->cnt=num;
            success=true;
		}else{
			success=false;
		}
		mysql_free_result(result);
	}
	mysql_close(my);

	free(query);
	return success;
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
			asprintf(&pers->name, "%s", row[COL_NAME]);
			asprintf(&pers->first_name, "%s", row[COL_VORNAME]);

			//Kürzel (falls vorhanden) holen
			if(row[COL_ACR] != NULL){
				//Die Person hat ein Küzel --> Lehrer
				asprintf(&pers->acronym, "%s", row[COL_ACR]);
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
			//pers->name=calloc(strlen(row[COL_NAME])+1, sizeof(char));
			//strcpy(pers->name, row[COL_NAME]);
			if(asprintf(&pers->name, "%s", row[COL_NAME]) == -1){
				print_exit_failure("Es konnte kein Speicher angefordert werden (get_person_by_sid)");
			}
			//pers->first_name=calloc(strlen(row[COL_VORNAME])+1, sizeof(char));
			//strcpy(pers->first_name, row[COL_VORNAME]);
			if(asprintf(&pers->first_name, "%s", row[COL_VORNAME]) == -1){
				print_exit_failure("Es konnte kein Speicher angefordert werden (get_person_by_sid)");
			}

			//Kürzel (falls vorhanden) holen
			if(row[COL_ACR] != NULL){
				//Die Person hat ein Küzel --> Lehrer
				//pers->acronym=calloc(strlen(row[COL_ACR])+1, sizeof(char));
				//strcpy(pers->acronym, row[COL_ACR]);
				if(asprintf(&pers->acronym, "%s", row[COL_ACR]) == -1){
					print_exit_failure("Es konnte kein Speicher angefordert werden (get_person_by_sid)");
				}
				pers->isTeacher=true;
			}else{
				pers->isTeacher=false;
			}

			//Kurse (falls vorhanden)
			if(row[COL_COURSE] != NULL){
				//pers->courses=calloc(strlen(row[COL_COURSE])+1, sizeof(char));
				//strcpy(pers->courses, row[COL_COURSE]);
				if(asprintf(&pers->courses, "%s", row[COL_COURSE]) == -1){
					print_exit_failure("Es konnte kein Speicher angefordert werden (get_person_by_sid)");
				}
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


bool get_person_by_acronym(person * pers, char * acronym){
	char * query=NULL;
	MYSQL * my=NULL;
	bool found=false;

	if(acronym == NULL){
		print_exit_failure("Programm falsch (get_person_by_acronym)");
	}

	if(asprintf(&query, "SELECT * FROM Benutzer WHERE kuerzel='%s'", acronym) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (get_person_by_acronym)");
	}

	my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		print_exit_failure("mysql_query failed (get_person_by_acronym)");
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
			asprintf(&pers->name, "%s", row[COL_NAME]);
			asprintf(&pers->first_name, "%s", row[COL_VORNAME]);

			//Kürzel (falls vorhanden) holen
			if(row[COL_ACR] != NULL){
				//Die Person hat ein Küzel --> Lehrer
				asprintf(&pers->acronym, "%s", row[COL_ACR]);
				pers->isTeacher=true;
			}else{
				pers->isTeacher=false;
			}

			//Kurse (falls vorhanden)
			if(row[COL_COURSE] != NULL){
				asprintf(&pers->courses, "%s", row[COL_COURSE]);
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

	if(strlen(mes->title)<2 || strlen(mes->message)<2)print_html_error("Geben Sie eine längere Meldung ein!!", "#"); // TODO: Der Zurück-Link soll gehen

	clean_string(mes->message);
	clean_string(mes->title);
	char * old_mes=mes->message;
	mes->message=nlcr_to_htmlbr(mes->message);
	free(old_mes); old_mes=NULL;
	#ifdef DEBUG
	fprintf(stderr, "\n\n Meldung: '%s'", mes->message);
	#endif // DEBUG
	remove_newline(mes->message);
	remove_newline(mes->courses);

	time_created=calloc(21, sizeof(char)); // 20
	strftime(time_created, 20, "%Y-%m-%d %H:%M:%S", mes->created);
	//                           4  2  2  2  2  2 ---> ? 19 +1 (\0) --> 20 = ????

	if(asprintf(&query, "INSERT INTO Meldungen \
				(titel, meldung, kurse, erstellerID, erstellt)\
				VALUES('%s', '%s', '%s', '%d', '%s')",
				mes->title, mes->message, mes->courses, mes->creator_id, time_created  ) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (insert_message)");
	}

	free(time_created);
	#ifdef DEBUG
	fprintf(stderr, "\n\nQuery (insert_message) : '%s'\n\n", query);
	#endif // DEBUG

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

		success=false;
	}else{
		success=true;
	}
	mysql_close(my);
	free(query);
	return success;
}

/** \brief Alle verfügbaren Kurse in alphabetischer Reihenfolge in einem course_set speichern
 *         (jeweils nur ein Kurs, ohne daten, nur der Name)
 *
 * \param c course_set*  leeres course_set
 * \return size_t        Anzahl der geholten Kurse
 *
 */
size_t get_distinct_courses(course_set * c){
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
			c->c_set=calloc(number, sizeof(course));
			MYSQL_ROW row;
			size_t i=0;
			while((row=mysql_fetch_row(result)) && i<number){
				init_course((c->c_set+i));
				asprintf(&(c->c_set+i)->name, "%s", row[0]);  //0, da nur eine Spalte erwartet wird (^ Siehe query)
				i++;
			}
			c->number=number;
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

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
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

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
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
		free(salt);
		salt_generate(&salt);
	}
	asprintf(&arg, "$6$%s$", salt);

	char * encr=NULL;
	encr=crypt(pers->password, arg);
	asprintf(&store_pw, "%s%s", salt, encr+strlen(arg));
	free(arg); arg=NULL;
	free(salt); salt=NULL;
	free(pers->password); pers->password=NULL;



	if(asprintf(&query, "UPDATE Benutzer SET passwort='%s' WHERE id='%d' AND email='%s'", store_pw, pers->id, pers->email) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (update_user_password)");
	}

	free(store_pw);

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

		success=false;
	}else{
		success=true;
	}

	mysql_close(my);
	free(query);
	return success;
}

/** \brief Alle Stunden eines Kurses holen
 *
 * \param this_course char*  String mit Name des Kurses
 * \param c_arr course_set*  leeres course_set in dem die Stunden gespeichert werden
 * \return bool              true: Erfolg; false. Fehler
 *
 */
bool get_course(char * this_course, course_set * c_arr){
	bool success=false;
	size_t number=0;
	MYSQL *my=NULL;
	char * query=NULL;

	if(this_course==NULL || c_arr==NULL){
		print_exit_failure("Programm falsch (get_course)");
	}

	if(asprintf(&query, "SELECT * FROM Kurse WHERE name='%s'", this_course) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (get_course)");
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
			c_arr->c_set=calloc(number, sizeof(course));
			MYSQL_ROW row;
			size_t i=0;
			while((row=mysql_fetch_row(result)) && i<number){
				init_course((c_arr->c_set+i));
				asprintf(&(c_arr->c_set+i)->name, "%s", row[COL_COURSE_NAME]);
				//asprintf(&(c_arr->c_set+i)->id, "%s", row[COL_COURSE_ID]); OMFG
				(c_arr->c_set+i)->id=atoi(row[COL_COURSE_ID]);
				asprintf(&(c_arr->c_set+i)->time, "%s", row[COL_COURSE_TIME]);
				asprintf(&(c_arr->c_set+i)->room, "%s", row[COL_COURSE_ROOM]);
				i++;
			}
			c_arr->number=number;
			success=true;
		}
		mysql_free_result(result);
	}
	mysql_close(my);
	free(query);
	return success;
}

/** \brief Die Vertretungsstunden zu einem bestimmten Kurs aus der Datenbank abrufen
 *
 * \param this_course char*   String mit Name des Kurses, dessen Vertretungsstunden geholt werde sollen
 * \param c_arr course_set*   leeres course_set in dem die Stunden gespeichert werden
 * \return bool               true: Erfolg; false. Fehler
 *
 */
bool get_alter_course(char * this_course, course_set * c_arr){
	bool success=false;
	size_t number=0;
	MYSQL *my=NULL;
	char * query=NULL;

	if(this_course==NULL || c_arr==NULL){
		print_exit_failure("Programm falsch (get_alter_course)");
	}

	if(asprintf(&query, "SELECT * FROM Vertretungen WHERE name='%s'", this_course) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (get_alter_course)");
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
			c_arr->c_set=calloc(number, sizeof(course));
			MYSQL_ROW row;
			size_t i=0;
			while((row=mysql_fetch_row(result)) && i<number){
				init_course((c_arr->c_set+i));
				asprintf(&(c_arr->c_set+i)->name, "%s", row[COL_ALTER_COURSE_NAME]);
				//asprintf(&(c_arr->c_set+i)->id, "%s", row[COL_ALTER_COURSE_ID]); OMFG
				(c_arr->c_set+i)->id=atoi(row[COL_ALTER_COURSE_ID]);
				asprintf(&(c_arr->c_set+i)->time, "%s", row[COL_ALTER_COURSE_ORIGINAL_TIME]);
				(c_arr->c_set+i)->status=UNCHANGED;

				if(row[COL_ALTER_COURSE_ROOM] == NULL && row[COL_ALTER_COURSE_TEACHER_ACR] == NULL && row[COL_ALTER_COURSE_TIME] == NULL){
					(c_arr->c_set+i)->status=OMITTED;
				}else{
					if(row[COL_ALTER_COURSE_TEACHER_ACR] != NULL){
						//anderer Lehrer
						(c_arr->c_set+i)->status |= (1<<0); // LSB
						asprintf(&(c_arr->c_set+i)->alter_teacher_acronym, "%s", row[COL_ALTER_COURSE_TEACHER_ACR]);

					}

					if(row[COL_ALTER_COURSE_ROOM]){
						(c_arr->c_set+i)->status |= (1 << 1);
						asprintf(&(c_arr->c_set+i)->alter_room, "%s", row[COL_ALTER_COURSE_ROOM]);
					}
				}

				i++;
			}
			c_arr->number=number;
			success=true; // vorher false, warum, keine Ahnung.
		}else{
			success=false;
		}
		mysql_free_result(result);
	}
	mysql_close(my);
	free(query);
	return success;
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

			//mysql_free_result(result);
			success=true;

		}
		mysql_free_result(result);
	}
	mysql_close(my);
	free(query);

	return success;
}

/** \brief Eine Nachricht aus der Datenbank anhand ihrer ID holen
 *
 * \param id int        ID der Nachricht
 * \param mes message*  Pointer auf eine Nachrichten-Struktur in der die Nachricht gespeichert wird
 * \return bool         (true: Nachricht gefunden; false: Nachricht nicht gefunden)
 *
 */
bool get_message_by_id(int id, message * mes){
	char * query=NULL;
	MYSQL * my=NULL;
	bool success=false;
	if(id <= 0 || mes == NULL){
		print_exit_failure("Programm falsch (get_message_by_id)");
	}

	if(asprintf(&query, "SELECT * FROM Meldungen WHERE id='%d'", id) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (get_message_by_id)");
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

			//Kurse holen
			asprintf(&(mes->courses), "%s", row[COL_MESSAGE_COURSES]);

			asprintf(&(mes->message), "%s", row[COL_MESSAGE_MES]);
			asprintf(&(mes->title),"%s", row[COL_MESSAGE_TITEL]);

			char * time;
			asprintf(&time, "%s", row[COL_MESSAGE_TIME_CREATED]);
			//TODO convert time

			free(time);

			mes->creator_id=atoi(row[COL_MESSAGE_CREATORID]);
			mes->id=atoi(row[COL_MESSAGE_ID]);



			//mysql_free_result(result);
			success=true;

		}
		mysql_free_result(result);
	}
	mysql_close(my);
	free(query);

	return success;
}

/** \brief Eine Nachricht löschen
 *
 * \param mes message*  Nachrichten-Objekt (muss mindestens die id beinhalten)
 * \return bool         (true: löschen erfolgreich; false: Fehler ist aufgetreten)
 *
 */
bool delete_message_by_id(message * mes){
	char * query=NULL;
	MYSQL * my=NULL;
	bool success=false;
	if(mes == NULL){
		print_exit_failure("Programm falsch (delete_message_by_id)");
	}

	if(asprintf(&query, "DELETE FROM Meldungen WHERE id='%d'", mes->id) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (delete_message_by_id)");
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
		success=true;
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
	regcomp(&reg, "[^A-Za-z0-9 #@\n\rÄÖÜäöüßéèêáàâíìîóòôúùûÉÈÊÁÀÂÍÌÎÓÒÔÚÙÛ!?(!?(),.-€]]*", REG_EXTENDED); //Nur diese Zeichen sind erlaubt
	// Achtung: keine ' (Hochkomma), oder " (Anführungszeichen) --> SQL-injection
	size_t str_len=strlen(str)+1;
	regmatch_t * pmatch=calloc(str_len, sizeof(regmatch_t));
	#ifdef DEBUG
	fprintf(stderr, "String : '%s'\n", str);
	#endif // DEBUG

	while(regexec(&reg, str, str_len, pmatch, REG_NOTBOL) == 0){
		int i=(int)pmatch->rm_so;
		if(pmatch->rm_so != pmatch->rm_eo && pmatch->rm_so < strlen(str))str[i]=' ';
	}
	free(pmatch);
	regfree(&reg);
}

bool course_regex_search(course * c, char * all_courses){
	regex_t reg;
	char * reg_string=NULL;
	asprintf(&reg_string, "(,| |^)%s(,|$)", c->name);
	bool match=false;

	regcomp(&reg, reg_string, REG_EXTENDED);

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
	free(pmatch);
	free(reg_string);
	regfree(&reg);

	return match;
}

/** \brief Alle \r\n (0x0D 0x0A durch <br> ersetzen (Funktion ist rekursiv)
 *
 * \param str char*  String der \r\n enthält
 * \return char*     String der <br> enthält
 *
 */
char * nlcr_to_htmlbr(char * str){

	//TODO !!! Kommentare, sonst blickt das später keiner mehr !!! Da sind auch irgendwelche Speicherprobleme !!!!!!!
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

	asprintf(&out, "%s<br>%s", out ? out : "", (found ? nlcr_to_htmlbr(found+2) : "") ); // Rekursion
	return out;
}


/** \brief Aus einem String mit Salt + Hash-Wert des Passwortes aus der Datenbank
           das Salz extrahieren
 *
 * \param db_passwd char*  String aus der DB
 * \return char*           Allozierter Speicher mit dem Salt
 *
 */
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
