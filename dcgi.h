/* dcgi.h */


struct cgi_pair
{
  char *key;
  char *value;
};

struct cgi_info
{
  int number_pairs;
  struct cgi_pair *pairs;
};


int cgi_parse_input(struct cgi_info *info);
char *cgi_get_value( struct cgi_info *info, char *key);
const char *cgi_get_error_string( int error);
void  cgi_clean_up(struct cgi_info *info);
