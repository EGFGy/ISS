#define COL_ID 0
#define COL_VORNAME 1 //zweite Spalte ist der Name
#define COL_NAME 2
#define COL_EMAIL 3
#define COL_PASS 4
#define COL_ACR 5
#define COL_COURSE 6

//id | vorname | name | email | passwort | kuerzel | kurse


#define SALT_SIZE 2


typedef struct {
	char * vorname;
	char * name;
	char * passwort;
	char * courses;
	char * acronym;
	char * email;
	bool auth;
	int sid;
	bool isTeacher;
}person;


void verifyUser(person * pers);
bool detectConvertAcronym(person * pers);
void uppercase_acr(person * pers);
void insertUser(person * pers);
void salt_generate(char ** salt);
bool salt_exists(char ** salt);
bool user_exists(char * name);
