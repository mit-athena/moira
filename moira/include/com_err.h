#include "mit-sipb-copyright.h"

#ifdef	__SABER__
void com_err(char *, int, char *, ...);
char *error_message(int);
void perror(char *);
/* too painful to do right.  someday... */
int (*com_err_hook)();
int (*set_com_err_hook())();
int (*reset_com_err_hook())();
#else
void com_err();
char *error_message();
void perror();
int (*com_err_hook)();
int (*set_com_err_hook())();
int (*reset_com_err_hook())();
#endif	/* __SABER__ */
