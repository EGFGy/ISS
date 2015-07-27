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

/** \brief Überprüfen, ob eine Person in der Datenbank ist und ob das Passwor stimmt
 *
 * \param pers person*  Person, die angemeldet werden soll
 * \return void
 *  Die Funktion testet, ob der Name oder das Kürzel (je nachdem was eingegeben wurde)
 *  (Erkennung an der Länge des Strings: =3 --> Kürzel, >3 --> Name)
 *  in der DB vorhanden ist. Wenn der Name existiert wird geprüft ob das Passwort richtig ist.
 *  Wenn das Passwort stimmt wird der bool-Wert "auth" in "person" auf true gesetzt.
 *  --> die Person ist authentifiziert.
 */
int verifyUser(person * pers){
	bool isAcronym;

	if(pers->name==NULL || pers->password==NULL){
		printExitFailure("Programm falsch!");
	}

	isAcronym=detect_convert_acronym(pers);

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		printExitFailure("MYSQL init failure");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){

		printExitFailure("MYSQL-connection error!");
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
			printExitFailure("Es konnte kein Speicher angefordert werden (verifyUser)");
		}

		fprintf(stderr, "Lehrer.\nsql_query: %s\n", sql_query);

		if(mysql_query(my, sql_query)){  //Liefert 0 bei Erfolg
			printExitFailure("mysql_query failed (Lehrer)");
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
			printExitFailure("Es konnte kein Speicher angefordert werden (verifyUser)");
		}

		fprintf(stderr, "Schueler o. Lehrer.\nsql_query: %s\n", sql_query);

		if(mysql_query(my, sql_query)){  // Liefert 0 bei Erfolg
			printExitFailure("mysql_query failed (Schueler - Lehrer)");
		}

		result = mysql_store_result(my);

		if(mysql_num_rows(result) == 1){
			found=true;
			isAcronym=false; //Da wir jetzt eine Person mit Kürzel gefunden haben
		}else{
			//Person nicht vorhanden, oder Fehler
			printExitFailure("mysql: Person nicht vorhanden, oder Passwort falsch."); //Was auch immer
		}
    }

	//Ab hier wurde die SQL-Query für Leher oder Schuler ausgeführt.
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
				//Person hat ihre Email-Addresse statt dem Kürzel angegeben --> (Falls es ein Lehrer ist, dessen Kürzel holen)
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

/** \brief Feststellen ob der name möglicherweise ein Kürzel ist und ggf. Zuordnung ändern.
 *
 * \param pers person*  Personen-Struktur
 * \return bool         true: es gibt ein Kürzel, false: es ist kein Kürzel.
 *  Falls der Name genau drei buchstabe lang ist wird der Inhalt in das Kürzel verschoben und
 *  zu nur Großbuchstaben umgewandelt.
 */
bool detect_convert_acronym(person * pers){
    bool isAcronym;

    if(pers->email==NULL || pers->password==NULL){
		printExitFailure("Programm falsch!");
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
void insertUser(person * pers){
	if(pers == NULL){
		printExitFailure("Programm falsch.\n Wörk!");
	}

	if(email_exists(pers->email)){
		printExitFailure("Benutzer Existiert schon!");
	}

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		printExitFailure("MYSQL init failure\n Wörk!");
	}

	if(mysql_real_connect(my, "localhost", "root", "WUW", "base5", 0, NULL, 0) == NULL){
		printExitFailure("MYSQL-connection error!");
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
			printExitFailure("Es konnte kein Speicher angefordert werden (insertUser)");
		}
	}else{
		if(asprintf(&query, "INSERT INTO Benutzer (vorname, name, email, passwort, kurse, kuerzel) \
					VALUES('%s', '%s', '%s', '%s', 'n/a', '%s')",
					pers->first_name, pers->name, pers->email, pers->password, pers->acronym) == -1)
		{
			printExitFailure("Es konnte kein Speicher angefordert werden (insertUser)");
		}
	}
	fprintf(stderr, "\nInsert dat:\n%s\n", query);
	if(mysql_query(my, query)){
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		printExitFailure("mysql_query failed (insert)");
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
	if(!f_random)printExitFailure("Problem beim generiern des Salt: urandom wurde nicht geoeffnet!");
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
        printExitFailure("Es konnte kein Speicher angefordert werden (salt_exists)");
	}

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		printExitFailure("MYSQL init failure\n Wörk!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		printExitFailure("MYSQL-connection error!");
	}

	MYSQL_RES * result=NULL;

	if(mysql_query(my, query)){
		printExitFailure("mysql_query failed (search salt)");
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

/** \brief Prüfen ob ein bestimmter Name schon in der Datenbank existiert
 *
 * \param name char*   Name der gesucht werden soll
 * \return bool        true: Name gefunden; false: Name nicht gefunden
 *
 */
bool email_exists(char * email){
	if(email == NULL){
		printExitFailure("Programm falsch, (email_exists)");
	}

	char * query=NULL;
	if(asprintf(&query, "SELECT email FROM Benutzer WHERE email='%s'", email) == -1){
        printExitFailure("Es konnte kein Speicher angefordert werden (email_exists)");
	}

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		printExitFailure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		printExitFailure("MYSQL-connection error!");
	}

	MYSQL_RES * result=NULL;

	if(mysql_query(my, query)){
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		printExitFailure("mysql_query failed (email_exists)");
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
		printExitFailure("Programm falsch, Wörk!");
	}
	char * query=NULL;


	if(asprintf(&query, "SELECT kuerzel FROM Benutzer WHERE kuerzel='%s'", acronym) == -1){
        printExitFailure("Es konnte kein Speicher angefordert werden (acronym_exists)");
	}

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		printExitFailure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		printExitFailure("MYSQL-connection error!");
	}

	MYSQL_RES * result=NULL;

	if(mysql_query(my, query)){
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		printExitFailure("mysql_query failed (acronym_exists)");
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
	while(sid_exists(generated_sid)){
        generated_sid=rand();
	}

	pers->sid=generated_sid;

	if(asprintf(&query, "UPDATE Benutzer SET sid='%d' WHERE id='%d'", pers->sid, pers->id) == -1){
        printExitFailure("Es konnte kein Speicher angefordert werden (create_session)");
	}

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		printExitFailure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_ALTERNATE_USER, SQL_ALTERNATE_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		printExitFailure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		printExitFailure("mysql_query failed (create_session)");
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
		printExitFailure("Es konnte kein Speicher angefordert werden (sid_exists)");
	}


	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		printExitFailure("MYSQL init failure\n Wörk!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		printExitFailure("MYSQL-connection error!");
	}

	MYSQL_RES * result=NULL;

	if(mysql_query(my, query)){
		printExitFailure("mysql_query failed (search sid)");
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
		printExitFailure("Es konnte kein Speicher angefordert werden (sid_set_null)");
	}

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		printExitFailure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_ALTERNATE_USER, SQL_ALTERNATE_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		printExitFailure("MYSQL-connection error!");
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

bool verify_sid(person * pers){
	char * query=NULL;
	if(asprintf(&query, "SELECT * FROM Benutzer WHERE email='%s' AND sid='%d'", pers->email, pers->sid) == -1){
		printExitFailure("Es konnte kein Speicher angefordert werden (verify_sid)");
	}

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		printExitFailure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		printExitFailure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		printExitFailure("mysql_query failed (verify_sid)");
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
	}else{
		MYSQL_RES * result=NULL;
		result = mysql_store_result(my);

		if(mysql_num_rows(result) > 0){
			fprintf(stderr, "sid gefunden\n");
			mysql_free_result(result);
			mysql_close(my);
			return true;
		}
		mysql_free_result(result);
	}

    mysql_close(my);
	return false;
}

message * get_all_messages(int * num){
	message * mes=NULL;

	char * query=NULL;
	if(asprintf(&query, "SELECT * FROM Meldungen WHERE kurse='all'") == -1){
		printExitFailure("Es konnte kein Speicher angefordert werden (get_all_messages)");
	}

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		printExitFailure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", SQL_USER, SQL_PASS, SQL_BASE, 0, NULL, 0) == NULL){
		printExitFailure("MYSQL-connection error!");
	}

	if(mysql_query(my, query)){
		printExitFailure("mysql_query failed (verify_sid)");
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
				(mes+i)->created=NULL;

            }
		}
		mysql_free_result(result);
	}
	mysql_close(my);

	return mes;
}
