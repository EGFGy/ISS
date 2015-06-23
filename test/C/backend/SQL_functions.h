#define COL_ID 0
#define COL_NAME 1 //zweite Spalte ist der Name
#define COL_PASS 2
#define COL_COURSE 3
#define COL_ACR 4

#define SALT_SIZE 2


typedef struct {
	char * name;
	char * passwort;
	char * courses;
	char * acronym;
	bool auth;
	int sid;
	bool isTeacher;
}person;


void verifyUser(person * pers);
bool detectConvertAcronym(person * pers);
void insertUser(person * pers);
void salt_generate(char ** salt);
bool salt_exists(char ** salt);
bool user_exists(char * name);
