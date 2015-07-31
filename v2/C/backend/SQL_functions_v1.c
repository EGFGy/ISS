#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <my_global.h>
#include <mysql.h>
#include <ctype.h>
#include <time.h>

#include <crypt.h>

#include "SQL_functions.h"
#include "CGI_functions.h"

#define SELECT_Schueler "SELECT Benutzer.id, name, passwort, kurse FROM Benutzer, Schueler WHERE name=\"?\" AND Schueler.id=Benutzer.id;"

//id, vorname, name, email, passwort, kuerzel, kurse, sid

/**
Identifikation per E-mail oder Kürzel (base5)
*/


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
}

void init_message(message * mes){
	mes->courses=NULL;
	mes->created=NULL;
	mes->creator_id=0;
	mes->id=0;
	mes->message=NULL;
	mes->title=NULL;
}

/** \brief Überprüfen, ob eine Person in der Datenbank ist und ob das Passwort stimmt
 *
 * \param pers person*  Person, die angemeldet werden soll
 * \return void
 *  Die Funktion testet, ob der Name oder das Kürzel (je nachdem was eingegeben wurde)
 *  (Erkennung an der Länge des Strings: =3 --> Kürzel, >3 --> Name)
 *  in der DB vorhanden ist. Wenn der Name existiert wird geprüft ob das Passwort richtig ist.
 *  Wenn das Passwort stimmt wird der bool-Wert "auth" in "person" auf true gesetzt.
 *  --> die Person ist authentifiziert.
 */
int verify_user(person * pers){
	bool isAcronym;

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

		fprintf(stderr, "Lehrer.\nsql_query: %s\n", sql_query);

		if(mysql_query(my, sql_query)){  //Liefert 0 bei Erfolg
			print_exit_failure("mysql_query failed (Lehrer)");
		}

		result = mysql_store_result(my);

		if(mysql_num_rows(result) == 1){
			found=true;
			isAcronym=true; //Da wir jetzt eine Person mit Kürzel gefunden haben
			pers->isTeacher=true;
		}
    }else{
		//Es könnte ein Lehrer oder ein Schüler sein

		char * sql_query=NULL;
		if(asprintf(&sql_query, "SELECT * FROM Benutzer WHERE email='%s'", pers->email) == -1){
			print_exit_failure("Es konnte kein Speicher angefordert werden (verify_user)");
		}

		fprintf(stderr, "Schueler o. Lehrer.\nsql_query: %s\n", sql_query);

		if(mysql_query(my, sql_query)){  // Liefert 0 bei Erfolg
			print_exit_failure("mysql_query failed (Schueler - Lehrer)");
		}

		result = mysql_store_result(my);

		if(mysql_num_rows(result) == 1){
			found=true;
			isAcronym=false; //Da wir jetzt eine Person mit Kürzel gefunden haben
		}else{
			//Person nicht vorhanden, oder Fehler
			print_exit_failure("mysql: Person nicht vorhanden, oder Passwort falsch."); //Was auch immer
		}
    }

	//Ab hier wurde die SQL-Query für Lehrer oder Schüler ausgeführt.
	if(found == true){
		MYSQL_ROW row;
		row=mysql_fetch_row(result);
		fprintf(stderr, "\nEin Ergebnis!\n Name: %s, Pass: %s, SID: '%s'\n", row[COL_NAME], row[COL_PASS], row[COL_SID]);

		if(row[COL_SID] != NULL){
            pers->auth=true;
            pers->sid=atoi(row[COL_SID]);
            pers->email=calloc(strlen(row[COL_EMAIL]), sizeof(char));
			strcpy(pers->email, row[COL_EMAIL]);
            return 1;
		}

		//Auslesen des Salt
		char * salt=calloc(SALT_LENGTH+1, sizeof(char));
		strncat(salt, row[COL_PASS], 1);
		strncat(salt, row[COL_PASS]+1, 1);
		pers->password=crypt(pers->password, salt);

		if(strcmp(pers->password, row[COL_PASS]) == 0){
			pers->auth=true;

			//Name holen
            pers->name=calloc(strlen(row[COL_NAME])+1, sizeof(char));
            strcpy(pers->name, row[COL_NAME]);
            pers->first_name=calloc(strlen(row[COL_VORNAME])+1, sizeof(char));
            strcpy(pers->first_name, row[COL_VORNAME]);

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

			create_session(pers);
		}else{
			pers->auth=false;
			pers->sid=0;
		}
	}

    mysql_free_result(result);
    mysql_close(my);


    return 0;
}

/** \brief Feststellen ob die E-mail möglicherweise ein Kürzel ist und ggf. Zuordnung ändern.
 *
 * \param pers person*  Personen-Struktur
 * \return bool         true: es gibt ein Kürzel, false: es ist kein Kürzel.
 *  Falls 'email' genau drei Buchstabe lang ist wird der Inhalt in das Kürzel verschoben und
 *  zu nur Großbuchstaben umgewandelt.
 */
bool detect_convert_acronym(person * pers){
    bool isAcronym;

    if(pers->email==NULL || pers->password==NULL){
		print_exit_failure("Programm falsch!");
	}

	if(strlen(pers->email) > 3){
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
	if(pers == NULL){
		print_exit_failure("Programm falsch.\n Wörk!");
	}

	if(email_exists(pers->email)){
		print_exit_failure("Benutzer Existiert schon!");
	}

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure\n Wörk!");
	}

	if(mysql_real_connect(my, "localhost", "root", "WUW", "base5", 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	char * salt=NULL;
	salt_generate(&salt);

    //Verhindern, dass ein bereits vorhandenes Salt zweimal verwendet wird (falls zwei Nutzer identische Passwörter wählen)
	while(salt_exists(&salt)){
		salt_generate(&salt);
	}

	pers->password=crypt(pers->password, salt);

	char * query=NULL;
	//Ist es eine Lehrer oder ein Schüler?
	if(!pers->isTeacher){
		if(asprintf(&query, "INSERT INTO Benutzer (vorname, name, email, passwort, kurse) \
					VALUES('%s', '%s', '%s', '%s', 'n/a')",
					pers->first_name, pers->name, pers->email, pers->password) == -1)
		{
			print_exit_failure("Es konnte kein Speicher angefordert werden (insert_user)");
		}
	}else{
		if(asprintf(&query, "INSERT INTO Benutzer (vorname, name, email, passwort, kurse, kuerzel) \
					VALUES('%s', '%s', '%s', '%s', 'n/a', '%s')",
					pers->first_name, pers->name, pers->email, pers->password, pers->acronym) == -1)
		{
			print_exit_failure("Es konnte kein Speicher angefordert werden (insert_user)");
		}
	}
	fprintf(stderr, "\nInsert dat:\n%s\n", query);
	if(mysql_query(my, query)){
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		print_exit_failure("mysql_query failed (insert)");
	}
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

	//strcpy(salt, "00");
	for(int i=SALT_LENGTH; i>0; i--){
		//fread(salt, 1, 2, f_random);
		//strcat(salt, fgetc(f_random));
		sprintf(*salt,"%s%x",*salt,fgetc(f_random));
		//sprintf(salt,"%c",fgetc(f_random));
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
	strncpy(seekSalt, *salt, SALT_LENGTH);

	char * query=NULL;

	if(asprintf(&query, "SELECT passwort FROM Benutzer WHERE passwort REGEXP '^%s'", seekSalt) == -1){
        print_exit_failure("Es konnte kein Speicher angefordert werden (salt_exists)");
	}

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure\n Wörk!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	MYSQL_RES * result=NULL;

	if(mysql_query(my, query)){
		print_exit_failure("mysql_query failed (search salt)");
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
	}else{
		result = mysql_store_result(my);

		if(mysql_num_rows(result) > 0){
			fprintf(stderr, "Salz gefunden, wörk\n");
			mysql_free_result(result);
			mysql_close(my);
			return true;
		}
	}

	mysql_free_result(result);
	mysql_close(my);

	return false;
}

/** \brief Prüfen ob eine bestimmte E-mail schon in der Datenbank existiert
 *
 * \param name char*   Name der gesucht werden soll
 * \return bool        true: Name gefunden; false: Name nicht gefunden
 *
 */
bool email_exists(char * email){
	if(email == NULL){
		print_exit_failure("Programm falsch, (email_exists)");
	}

	char * query=NULL;
	if(asprintf(&query, "SELECT email FROM Benutzer WHERE email='%s'", email) == -1){
        print_exit_failure("Es konnte kein Speicher angefordert werden (email_exists)");
	}

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	MYSQL_RES * result=NULL;

	if(mysql_query(my, query)){
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		print_exit_failure("mysql_query failed (email_exists)");
	}else{
		result = mysql_store_result(my);

		if(mysql_num_rows(result) > 0){
			fprintf(stderr, "Benutzer gefunden, wörk\n");
			mysql_free_result(result);
			mysql_close(my);
			return true;
		}else{
			mysql_free_result(result);
			mysql_close(my);
			return false;
		}
	}
	mysql_free_result(result);
	mysql_close(my);
	return false;
}


/** \brief Prüfen ob ein eingegebenes Kürzel schon in der Datenbank existiert
 *
 * \param acronym char*    Kürzel
 * \return bool            true: existiert; false: existiert nicht (gut)
 *
 */
bool acronym_exists(char * acronym){
	if(acronym == NULL){
		print_exit_failure("Programm falsch, Wörk!");
	}
	char * query=NULL;


	if(asprintf(&query, "SELECT kuerzel FROM Benutzer WHERE kuerzel='%s'", acronym) == -1){
        print_exit_failure("Es konnte kein Speicher angefordert werden (acronym_exists)");
	}

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	MYSQL_RES * result=NULL;

	if(mysql_query(my, query)){
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		print_exit_failure("mysql_query failed (acronym_exists)");
	}else{
		result = mysql_store_result(my);

		if(mysql_num_rows(result) > 0){
			fprintf(stderr, "Benutzer gefunden, wörk\n");
			mysql_free_result(result);
			mysql_close(my);
			return true;
		}else{
			mysql_free_result(result);
			mysql_close(my);
			return false;
		}
	}
	mysql_free_result(result);
	mysql_close(my);
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

	srand(time(NULL));
	int generated_sid=rand();
	while((sid_exists(generated_sid)) ^ (generated_sid==0)){
        generated_sid=rand();
	}

	pers->sid=generated_sid;

	if(asprintf(&query, "UPDATE Benutzer SET sid='%d' WHERE id='%d'", pers->sid, pers->id) == -1){
        print_exit_failure("Es konnte kein Speicher angefordert werden (create_session)");
	}

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_ALTERNATE_USER, SQL_ALTERNATE_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		print_exit_failure("mysql_query failed (create_session)");
	}
    mysql_close(my);

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
	if(asprintf(&query, "SELECT sid FROM Benutzer WHERE sid='%d'", sid) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (sid_exists)");
	}


	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure\n Wörk!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	MYSQL_RES * result=NULL;

	if(mysql_query(my, query)){
		print_exit_failure("mysql_query failed (search sid)");
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
	}else{
		result = mysql_store_result(my);

		if(mysql_num_rows(result) > 0){
			fprintf(stderr, "sid gefunden, wörk\n");
			mysql_free_result(result);
			mysql_close(my);
			return true;
		}
	}

	mysql_free_result(result);
	mysql_close(my);

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
	if(asprintf(&query, "UPDATE Benutzer SET sid=NULL WHERE email='%s' AND sid='%d'", pers->email, pers->sid) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (sid_set_null)");
	}

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_ALTERNATE_USER, SQL_ALTERNATE_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
        mysql_close(my);
		return false;
	}else{
        mysql_close(my);
		return true;
	}
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
	if(asprintf(&query, "SELECT * FROM Benutzer WHERE email='%s' AND sid='%d'", pers->email, pers->sid) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (verify_sid)");
	}

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		print_exit_failure("mysql_query failed (verify_sid)");
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
	}else{
		MYSQL_RES * result=NULL;
		result = mysql_store_result(my);

		if(mysql_num_rows(result) > 0){
			fprintf(stderr, "sid gefunden\n");
			mysql_free_result(result);
			mysql_close(my);

			pers->auth=true;
			return true;
		}
		mysql_free_result(result);
	}

    mysql_close(my);
	return false;
}

/** \brief 5 Nachrichten holen
 *
 * \param num int*    Pointer in dem die tatsächliche Anzahl an Nachrichten gespeichert wird
 * \param offset int  Alle 5 Nachrichten ab der offset*5-ten Nachricht holen
 * \return message*   Pointer auf ein Array aus max. 5 Nachrichten
 *
 */
message * get_messages(int * num, int offset){
	message * mes=NULL;

	char * query=NULL;
	if(asprintf(&query, "SELECT * FROM Meldungen WHERE kurse='all' ORDER BY erstellt DESC LIMIT 5 OFFSET %d", (offset*5)) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (get_all_messages)");
	}

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		print_exit_failure("mysql_query failed (get_messages)");
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
	}else{
		MYSQL_RES * result=NULL;
		result = mysql_store_result(my);
		*num=mysql_num_rows(result);
		if(mysql_num_rows(result) > 0){
			mes = calloc(mysql_num_rows(result), sizeof(message));
            MYSQL_ROW message_row;
            for(my_ulonglong i=0; i<mysql_num_rows(result) && (message_row=mysql_fetch_row(result)); i++){
				(mes+i)->id=atoi(message_row[COL_MESSAGE_ID]);

				(mes+i)->title=calloc(strlen(message_row[COL_MESSAGE_TITEL])+1, sizeof(char));
				strcpy((mes+i)->title, message_row[COL_MESSAGE_TITEL]);

				(mes+i)->message=calloc(strlen(message_row[COL_MESSAGE_MES])+1, sizeof(char));
				strcpy((mes+i)->message, message_row[COL_MESSAGE_MES]);

				(mes+i)->courses=calloc(strlen(message_row[COL_MESSAGE_COURSES])+1, sizeof(char));
				strcpy((mes+i)->courses, message_row[COL_MESSAGE_COURSES]);

				(mes+i)->creator_id=atoi(message_row[COL_MESSAGE_CREATORID] ? message_row[COL_MESSAGE_CREATORID] : "-1");

				//TODO: uhrzeit rchtig machen ????
				(mes+i)->s_created=calloc(strlen(message_row[COL_MESSAGE_TIME_CREATED])+1, sizeof(char));
				strcpy((mes+i)->s_created, message_row[COL_MESSAGE_TIME_CREATED]);

            }
		}
		mysql_free_result(result);
	}
	mysql_close(my);

	return mes;
}

/** \brief Anhand der ID der Person die restliche Information über sie herausfinden
 *
 * \param id int    ID der Person
 * \return person*  Personenobjekt in das die Daten geschrieben werden
 *
 */
person * get_person_by_id(int id){

	if(id < 1)return NULL;
	char * query=NULL;
	if(asprintf(&query, "SELECT * FROM Benutzer WHERE id='%d'", id) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (get_person_by_id)");
	}

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		print_exit_failure("mysql_query failed (get_person_by_id)");
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
	}else{
		MYSQL_RES * result=NULL;
		result = mysql_store_result(my);

		if(mysql_num_rows(result) > 0){
			fprintf(stderr, "uid gefunden\n");

			MYSQL_ROW row;
			row=mysql_fetch_row(result);

			person * pers=calloc(1, sizeof(person));

			//Name holen
            pers->name=calloc(strlen(row[COL_NAME])+1, sizeof(char));
            strcpy(pers->name, row[COL_NAME]);
            pers->first_name=calloc(strlen(row[COL_VORNAME])+1, sizeof(char));
            strcpy(pers->first_name, row[COL_VORNAME]);

			mysql_free_result(result);
			mysql_close(my);
			return pers;
		}
		mysql_free_result(result);
	}

    mysql_close(my);
    return NULL;
}

/** \brief Personendaten anhand der SID und E-mail abrufen
 *
 * \param pers person*  Person mit E-mail und SID in der die restlichen Daten gespeichert werden
 * \return void
 *
 */
void get_person_by_sid(person * pers){
	char * query=NULL;
	if(asprintf(&query, "SELECT * FROM Benutzer WHERE sid='%d' AND email='%s'", pers->sid, pers->email) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (get_person_by_sid)");
	}

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		print_exit_failure("mysql_query failed (get_person_by_sid)");
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
	}else{
		MYSQL_RES * result=NULL;
		result = mysql_store_result(my);

		if(mysql_num_rows(result) > 0){
			fprintf(stderr, "uid gefunden\n");

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

			mysql_free_result(result);
			mysql_close(my);
			return;
		}
		mysql_free_result(result);
	}

    mysql_close(my);
    return;
}

/** \brief Eine Nachricht in die Datenbank einfügen
 *
 * \param mes message*  Nachricht
 * \return bool         true: es hat funktioniert; false: es hat nicht funktoiniert
 *
 */
bool insert_message(message * mes){
	char * query=NULL;
	char * time_created=calloc(20, sizeof(char));
	strftime(time_created, 19, "%Y-%m-%d %H:%M:00", mes->created);

	//TODO: Zeit umwandeln
	if(asprintf(&query, "INSERT INTO Meldungen \
				(titel, meldung, kurse, erstellerID, erstellt)\
				VALUES('%s', '%s', '%s', '%d', '%s')",
				mes->title, mes->message, mes->courses, mes->creator_id, time_created  ) == -1){
		print_exit_failure("Es konnte kein Speicher angefordert werden (insert_message)");
	}

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		print_exit_failure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_ALTERNATE_USER, SQL_ALTERNATE_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		print_exit_failure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
        mysql_close(my);
		return false;
	}else{
        mysql_close(my);
		return true;
	}
}
