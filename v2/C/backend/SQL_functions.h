#define COL_ID      0
#define COL_VORNAME 1 //zweite Spalte ist der Name
#define COL_NAME    2
#define COL_EMAIL   3
#define COL_PASS    4
#define COL_ACR     5
#define COL_COURSE  6
#define COL_SID     7

#define COL_MESSAGE_ID            0
#define COL_MESSAGE_TITEL         1
#define COL_MESSAGE_MES           2
#define COL_MESSAGE_COURSES       3
#define COL_MESSAGE_CREATORID     4
#define COL_MESSAGE_TIME_CREATED  5

#define COL_COURSE_ID    0
#define COL_COURSE_NAME  1
#define COL_COURSE_TIME  2

#define GET_MESSAGE_COUNT 5

#define SQL_USER            "web_user"
#define SQL_PASS            "web_pass"
#define SQL_BASE            "base5"
#define SQL_ALTERNATE_USER  "root"
#define SQL_ALTERNATE_PASS  "WUW"

#define SALT_LENGTH 12


typedef struct {
	int id;                 // ID der Person
	char * first_name;      // Vorname
	char * name;            // Nachname
	char * password;        // eingegebenes Passwort
	char * courses;         // Liste der Kurse an denen die Person teilnimmt
	char * acronym;         // Kürzel der Person (falls vorhanden)
	char * email;           // E-Mail-Adresse der Person
	struct tm * login_time; // Zeit zu der sich der Nutzer angemeldet hat
	bool auth;              // ist die Person authentifiziert
	int sid;                // Session-ID
	bool isTeacher;         // Lehrer ?
}person;

typedef struct{
	int id;               // ID der Nachricht
	char * title;         // Titel
	char * message;       // Nachricht
	char * courses;       // String mit Kursliste aus der Datenbank / vom Browser (bei Erstellung)
	int creator_id;       // ID des / der Erstellers / Erstellerin
	struct tm * created;  // Speicherung der Zeit bei Erstellung
	char * s_created;     // String in dem die Zeit de rErstellung aus der Datenbank gespeichert wird
}message;

typedef struct{
	int id;
	char * name;
	char * time;
}course;

typedef enum {PW_CORRECT, PW_CORRECT_ALREADY_LOGGED_IN, PW_INCORRECT}UserState;

void init_person(person * p);
void init_message(message * mes);
void init_course(course * c);
int verify_user(person * pers);
bool detect_convert_acronym(person * pers);
void uppercase_acr(person * pers);
void insert_user(person * pers);
void salt_generate(char ** salt);
bool salt_exists(char ** salt);
bool email_exists(char * name);
bool acronym_exists(char * acronym);
int create_session(person * pers);
bool sid_exists(int sid);
bool sid_set_null(person * pers);
bool verify_sid(person * pers);
int get_messages(message ** mes, int offset);
bool get_person_by_id(person * pers);
bool get_person_by_sid(person * pers);
bool insert_message(message * mes);
size_t get_distinct_courses(course ** c);
bool update_user_courses(person * pers);

void clean_string(char * str);
char * nlcr_to_htmlbr(char * str);
