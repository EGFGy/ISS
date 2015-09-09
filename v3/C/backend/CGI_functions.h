#ifndef content_max
#define content_max 700  //Wir sagen: "Es gibt einfach nicht mehr als contenr_max Bytes in den Übergebenen Daten!!"
#endif

typedef enum {TEXT, HTML}httpHeaderType;
typedef enum {POST, GET, BOTH}httpRequestMethod;
typedef enum {TIMETABLE, MAIN, SETTINGS, COURSE}menuSelection;

typedef struct{
	int content_length;                // Länge der Daten (bytes) in POST-Data
	httpRequestMethod request_method;  // Methode der Anfrage
	char * POST_data;                  // per POST übertragene Daten (falls vorhanden)
	char * query_string;               // QUERY_STRING (falls vorhanden)
	char * http_cookies;               // Cookies
	char * http_host;                  // eigene Adresse
}cgi;

void init_CGI(cgi * c);
void get_CGI_data(cgi * gotCGI);
void print_exit_failure(const char * message);
void httpSetCookie(char name[], char content[]);
void httpRedirect(const char * url);
void httpCacheControl(const char * directive);
void httpHeader(httpHeaderType type);
int _extractCGIdata(char * data, const char * property, char * delim, char ** out);
int extract_POST_data(cgi * cgi, const char * property, char ** out);
int extract_COOKIE_data(cgi * cgi, const char * property, char ** out);
int extract_QUERY_data(cgi * cgi, const char * property, char ** out);
int decodeHEX(char *s, char *dec);
void remove_newline(char * str);
void print_html_head(char * descr, char * title);
void print_html_pure_head_menu(char * descr, char * title, menuSelection menu);
void print_html_error(char * ErrorText, char * back_url);
