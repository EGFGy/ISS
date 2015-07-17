#ifndef content_max
#define content_max 700  //Wir sagen: "Es gib einfach nicht mehr als contenr_max bytes in den Übergebenen Daten!!"
#endif // content_max

typedef struct{
	int content_length;
	char * request_method;
	char * POST_data;
	char * query_string;
	char * http_cookies;
}cgi;
typedef enum {TEXT, HTML}httpHeaderType;


void getCGIdata(cgi * gotCGI);
void printExitFailure(const char * message);
void setCookie(char name[], char content[]);
void httpRedirect(const char * url);
void httpHeader(httpHeaderType type);
int extractCGIdata(char * data, const char * property, char * delim, char ** out);
int extractPOSTdata(cgi * cgi, const char * property, char ** out);
int extractCOOKIEdata(cgi * cgi, const char * property, char ** out);
int decodeHEX(char *s, char *dec);
