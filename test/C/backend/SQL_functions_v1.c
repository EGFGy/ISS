#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <my_global.h>
#include <mysql.h>
#include <ctype.h>
/*
#define _GNU_SOURCE
#include <crypt.h>
*/
#include "SQL_functions.h"
#include "CGI_functions.h"

#define SELECT_Schueler "SELECT Benutzer.id, name, passwort, kurse FROM Benutzer, Schueler WHERE name=\"?\" AND Schueler.id=Benutzer.id;"

/**
Neue Version mit nur einer Tabelle. (base4)
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
void verifyUser(person * pers){
	bool isAcronym;

	if(pers->name==NULL || pers->passwort==NULL){
		printExitFailure("Programm falsch!");
	}

	isAcronym=detectConvertAcronym(pers);

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		printExitFailure("MYSQL init failure");
	}

	if(mysql_real_connect(my, "localhost", "web_user", "web_pass", "base4", 0, NULL, 0) == NULL){
		/*fprintf (stderr, "Fehler mysql_real_connect(): %u (%s)\n",
		mysql_errno (my), mysql_error (my));
		exit(EXIT_FAILURE);*/
		printExitFailure("MYSQL-connection error!");
	}else{
		//fprintf(stderr, "Connection extablished!\n");
	}

	MYSQL_RES * result=NULL;
	bool found=false;


	if(isAcronym){
		//Es ist sicher eine Lehrer

		//TODO: Verhindern, dass ein bereits vorhandener Name erneut eingefügt wird.
		//TODO: sql-injection verhindern
		size_t len_start=strlen("SELECT id, name, passwort, kurse, kuerzel FROM Benutzer WHERE kuerzel=\"");
		size_t len_mid=strlen(pers->acronym);
		size_t len_end=strlen("\";");

		char * sql_query=calloc(len_start+len_end+len_mid+1, sizeof(char));
		if(sql_query == NULL){
			printExitFailure("Es konnte kein Speicher für \"sql_query\" angefrdert werden");
		}
		strcpy(sql_query, "SELECT id, name, passwort, kurse, kuerzel FROM Benutzer WHERE kuerzel=\"");
		strncat(sql_query, pers->acronym, len_mid);
		strcat(sql_query, "\";");

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

		size_t len_start=strlen("SELECT id, name, passwort, kurse, kuerzel FROM Benutzer WHERE name=\"");
		size_t len_mid=strlen(pers->name);
		size_t len_end=strlen("\";");

		char * sql_query=calloc(len_start+len_end+len_mid+1, sizeof(char));
		if(sql_query == NULL){
			printExitFailure("Es konnte kein Speicher für \"sql_query\" angefrdert werden");
		}
		strcpy(sql_query, "SELECT id, name, passwort, kurse, kuerzel FROM Benutzer WHERE name=\"");
		strncat(sql_query, pers->name, len_mid);
		strcat(sql_query, "\";");

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
		fprintf(stderr, "\nEin Ergebnis!\n Name: %s, Pass: %s\n", row[COL_NAME], row[COL_PASS]);

		//TODO: Passwort RICHTIG machen (mit hash + salt)
		char * salt=calloc(SALT_SIZE+1, sizeof(char));
		strncat(salt, row[COL_PASS], 1);
		strncat(salt, row[COL_PASS]+1, 1);
		pers->passwort=crypt(pers->passwort, salt);

		if(strcmp(pers->passwort, row[COL_PASS]) == 0){
			pers->auth=true;
			if(isAcronym){
				//Person hat Kürzel als Name angegeben --> es ist eine Leherer --> Name holen
				pers->name=calloc(strlen(row[COL_NAME])+1, sizeof(char));
				strcpy(pers->name, row[COL_NAME]);
			}else{
				//Person hat ihren Namen statt dem Kürzel angegeben --> (Falls es ein Lehrerist, dessen Kürzel holen)
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
			if(row[COL_COURSE] != NULL){
				pers->courses=calloc(strlen(row[COL_COURSE])+1, sizeof(char));
				strcpy(pers->courses, row[COL_COURSE]);
			}

			srand((unsigned int)time(NULL));
			pers->sid=rand();
		}else{
			pers->auth=false;
			pers->sid=0;
		}
    }

    mysql_free_result(result);
    mysql_close(my);
}

/** \brief Feststellen ob der name möglicherweise ein Kürzel ist und ggf. Zuordnung ändern.
 *
 * \param pers person*  Personen-Struktur
 * \return bool         true: es gibt ein Kürzel, false: es ist kein Kürzel.
 *  Falls der Name genau drei buchstabe lang ist wird der Inhalt in das Kürzel verschoben und
 *  zu nur Großbuchstaben umgewandelt.
 */
bool detectConvertAcronym(person * pers){
    bool isAcronym;

    if(pers->name==NULL || pers->passwort==NULL){
		printExitFailure("Programm falsch!");
	}

	if(strlen(pers->name) > 3){
		isAcronym=false;
	}else{
		if(strlen(pers->name) == 3){
			isAcronym=true;
			pers->acronym=pers->name;
			pers->name=NULL;

			uppercase_acr(pers);
			//char * c=pers->acronym_id;
			/*int p=0;
			while(pers->acronym[p]){
				pers->acronym[p]=toupper(pers->acronym[p]);
				p++;
			}*/
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

	if(user_exists(pers->name)){
		printExitFailure("Benutzer Existiert schon!");
	}

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		printExitFailure("MYSQL init failure\n Wörk!");
	}

	if(mysql_real_connect(my, "localhost", "root", "WUW", "base4", 0, NULL, 0) == NULL){
		/*fprintf (stderr, "Fehler mysql_real_connect(): %u (%s)\n",
		mysql_errno (my), mysql_error (my));
		exit(EXIT_FAILURE);*/
		printExitFailure("MYSQL-connection error!");
	}else{
		//fprintf(stderr, "Connection extablished!\n");
	}

	//TODO Salt erzeugen und "Salt-reuse verhindern"
	char * salt=NULL;
	salt_generate(&salt);

	while(salt_exists(&salt)){
		salt_generate(&salt);
	}

	pers->passwort=crypt(pers->passwort, salt);


	char * query;
	//Ist es eine Lehrer oder ein Schüler?
	if(!pers->isTeacher){
		query = calloc(strlen("INSERT INTO Benutzer (name, passwort, kurse) VALUES('")+strlen(pers->name)+strlen("', '")+strlen(pers->passwort)+strlen("', 'n/a');")+1, sizeof(char));
		strcat(query, "INSERT INTO Benutzer (name, passwort, kurse) VALUES('");
		strcat(query, pers->name);
		strcat(query, "', '");
		strcat(query, pers->passwort);
		strcat(query, "', 'n/a');");
	}else{
		query = calloc(strlen("INSERT INTO Benutzer (name, passwort, kurse, kuerzel) VALUES('")+strlen(pers->name)+strlen("', '")+strlen(pers->passwort)+strlen("', 'n/a', '")+strlen(pers->acronym)+strlen("');")+1, sizeof(char));
		strcat(query, "INSERT INTO Benutzer (name, passwort, kurse, kuerzel) VALUES('");
		strcat(query, pers->name);
		strcat(query, "', '");
		strcat(query, pers->passwort);
		strcat(query, "', 'n/a', '");
		strcat(query, pers->acronym);
		strcat(query, "');");
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
	*salt=calloc(SALT_SIZE+1, sizeof(unsigned char));

	//strcpy(salt, "00");
	for(int i=SALT_SIZE; i>0; i--){
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

	char * seekSalt=calloc(SALT_SIZE+1, sizeof(char));
	strncpy(seekSalt, *salt, SALT_SIZE);

	char * query=calloc(strlen("SELECT passwort FROM Benutzer WHERE passwort REGEXP \"^")+strlen(seekSalt)+strlen("\";")+1, sizeof(char));
	strcat(query, "SELECT passwort FROM Benutzer WHERE passwort REGEXP '^");
	strcat(query, seekSalt);
	strcat(query, "';");

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		printExitFailure("MYSQL init failure\n Wörk!");
	}

	if(mysql_real_connect(my, "localhost", "web_user", "web_pass", "base4", 0, NULL, 0) == NULL){
		/*fprintf (stderr, "Fehler mysql_real_connect(): %u (%s)\n",
		mysql_errno (my), mysql_error (my));
		exit(EXIT_FAILURE);*/
		printExitFailure("MYSQL-connection error!");
	}else{
		//fprintf(stderr, "Connection extablished!\n");
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
bool user_exists(char * name){
	if(name == NULL){
		printExitFailure("Programm falsch, Wörk!");
	}

	char * query=NULL;
	query=calloc(strlen("SELECT name FROM Benutzer WHERE name='")+strlen(name)+strlen("';")+1, sizeof(char));
	strcat(query, "SELECT name FROM Benutzer WHERE name='");
	strcat(query, name);
	strcat(query, "';");

	MYSQL *my=mysql_init(NULL);
	if(my == NULL){
		printExitFailure("MYSQL init failure!");
	}

	if(mysql_real_connect(my, "localhost", "web_user", "web_pass", "base4", 0, NULL, 0) == NULL){
		printExitFailure("MYSQL-connection error!");
	}else{
		//fprintf(stderr, "Connection extablished!\n");
	}

	MYSQL_RES * result=NULL;

	if(mysql_query(my, query)){
		fprintf(stderr, "sql_query:\n%s\nfailed\n", query);
		printExitFailure("mysql_query failed (search user)");
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

