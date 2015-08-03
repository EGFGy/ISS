#ifndef content_max
#define content_max 700  //Wir sagen: "Es gib einfach nicht mehr als contenr_max bytes in den Übergebenen Daten!!"
#endif // content_max

typedef struct{
	int content_length;
	char * request_method;
	char * POST_data;
	char * query_string;
	char * http_cookies;
	char * http_host;
}cgi;
typedef enum {TEXT, HTML}httpHeaderType;

void init_CGI(cgi * c);
void get_CGI_data(cgi * gotCGI);
void print_exit_failure(const char * message);
void setCookie(char name[], char content[]);
void httpRedirect(const char * url);
void httpHeader(httpHeaderType type);
int _extractCGIdata(char * data, const char * property, char * delim, char ** out);
int extract_POST_data(cgi * cgi, const char * property, char ** out);
int extract_COOKIE_data(cgi * cgi, const char * property, char ** out);
int extract_QUERY_data(cgi * cgi, const char * property, char ** out);
int decodeHEX(char *s, char *dec);
void remove_newline(char * str);
