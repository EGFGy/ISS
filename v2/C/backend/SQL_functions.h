#define COL_ID 0
#define COL_VORNAME 1 //zweite Spalte ist der Name
#define COL_NAME 2
#define COL_EMAIL 3
#define COL_PASS 4
#define COL_ACR 5
#define COL_COURSE 6
#define COL_SID 7

#define COL_MESSAGE_ID 0
#define COL_MESSAGE_TITEL 1
#define COL_MESSAGE_MES 2
#define COL_MESSAGE_COURSES 3
#define COL_MESSAGE_CREATORID 4
#define COL_MESSAGE_TIME_CREATED 5

#define SQL_USER "web_user"
#define SQL_PASS "web_pass"
#define SQL_BASE "base5"
#define SQL_ALTERNATE_USER "root"
#define SQL_ALTERNATE_PASS "WUW"

#define SALT_LENGTH 2


typedef struct {
    int id;
	char * first_name;
	char * name;
	char * password;
	char * courses;
	char * acronym;
	char * email;
	bool auth;
	int sid;
	bool isTeacher;
}person;

typedef struct{
	int id;
	char * title;
	char * message;
	char * courses;
	int creator_id;
	struct tm * created;
}message;

void init_person(person * p);
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
message * get_messages(int * number, int offset);
person * get_person_by_id(int id);
