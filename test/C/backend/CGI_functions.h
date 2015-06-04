#ifndef content_max
#define content_max 700
#endif // content_max

typedef struct{
    int content_length;
    char * request_method;
    char * POST_data;
    char * query_string;
    char * http_cookies;
}cgi;
void getCGIdata(cgi * gotCGI);
void printExitFailure(const char * message);
void setCookie(char name[], char content[]);
void httpRedirect(const char * url);
int extractCGIdata(char * data, const char * property, char * delim, char ** out);
int extractPOSTdata(char * data, const char * property, char ** out);
int extractCOOKIEdata(char * data, const char * property, char ** out);
