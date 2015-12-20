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
#define COL_COURSE_ROOM  3

#define COL_ALTER_COURSE_ID 0
#define COL_ALTER_COURSE_NAME 1
#define COL_ALTER_COURSE_TIME 2
#define COL_ALTER_COURSE_ORIGINAL_TIME 3
#define COL_ALTER_COURSE_ROOM 4
#define COL_ALTER_COURSE_TEACHER_ACR 5

#define GET_MESSAGE_COUNT 5

#define SQL_USER            "web_user"
#define SQL_PASS            "web_pass"
#define SQL_BASE            "base5"
#define SQL_ALTERNATE_USER  "web_user"
#define SQL_ALTERNATE_PASS  "web_pass"

#define SALT_LENGTH 12

#define WEEKDAY_MAX 5
#define HOUR_MAX 11

extern const char * german_weekdays[5];
extern const char * long_german_weekdays[5];

typedef enum {PW_CORRECT, PW_CORRECT_ALREADY_LOGGED_IN, PW_INCORRECT}UserState;
//typedef enum {UNCHANGED, TEACHER, ROOM, TEACHER_ROOM, TIME, TIME_TEACHER, TIME_ROOM, TIME_TEACHER_ROOM, OMITTED}CourseStatus;

#define UNCHANGED 0b0000
#define TEACHER 0b0001
#define ROOM 0b0010
#define TIME 0b0100
#define OMITTED 0b1000

/**
0 0000 UNCHANGED
1 0001 TEACHER
2 0010 ROOM
4 0100 TIME
8 1000 OMITTED
*/

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
	message * all_messages; // Array in dem mehrere Nachrichten gespeichert werden
	size_t cnt;             // Anzahl der Nachrichten
}message_set;

typedef struct{
	int id;                        // ID des Kurses
	char * name;                   // Kursname (z.B. 2PHO)
	char * time;                   // Zeit zu der der Kurs stattfindet (z.B. Mo3)
	char * room;                   // Raum in dem der Kurs stattfindet
	person * teacher;              // (optional) Person die den Kurs unterrichtet

	char * alter_time;             // Alternative Zeit zu der der Kurs stattfindet (aus V_Plan)
	char * original_time;          // Zeit des ursprünglichen Kurses (Verschiebung von Stunden --> die andere fällt aus)
	char * alter_room;             // Alternativer Raum in dem der Kurs stattfindet (aus V_Plan)
	char * alter_teacher_acronym;  // Alternativer Lehrer der den Kurs unterrichtet
	int status;           // Zustand (Änderungen, oder nichts)
}course;

typedef struct{
	course * c_set; // Array in dem mehrere Stunden gespeichert werden
	size_t number;  // Anzahl der Stunden
}course_set;

void init_person(person * p);
void free_person(person * p);
void init_message(message * mes);
void free_message(message * mes);
void init_message_set(message_set * m);
void free_message_set(message_set * m);
void init_course(course * c);
void init_course_set(course_set * c);

int verify_user(person * pers);
bool verify_user_password(person * pers);
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
bool get_messages(message_set * mes, int offset, char * course);
bool get_person_by_id(person * pers);
bool get_person_by_sid(person * pers);
bool get_person_by_acronym(person * pers, char * acronym);
bool insert_message(message * mes);
size_t get_distinct_courses(course ** c);
bool update_user_courses(person * pers);
bool update_user_email(person * pers, char * new_email);
bool update_user_password(person * pers);
bool get_course(char * this_course, course_set * c_arr);
bool get_alter_course(char * this_course, course_set * c_arr);
bool get_teacher_by_course(person * pers, char * c);

void clean_string(char * str);
bool course_regex_search(course * c, char * all_courses);
char * nlcr_to_htmlbr(char * str);
int comma_to_array(char * comma_char, char *** str_array);
char * salt_extract(char * db_passwd);
