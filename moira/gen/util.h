#include <moira_schema.h>

/* for MAXPATHLEN */
#include <sys/param.h>

void fix_file(char *targetfile);
char *dequote(char *s);
void db_error(int code);

int ModDiff(int *flag, char *tbl, time_t ModTime);
