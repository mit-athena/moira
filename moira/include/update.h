#define log_DEBUG 0
#define log_INFO  1
#define log_WARNING 2
#define log_ERROR 3

#define SERVICE_NAME "moira_update"
#define	UPDATE_BUFSIZ	BUFSIZ

int send_file(char *pathname, char *target_path, int encrypt);
int send_auth(char *hostname);
int execute(char *path);
void send_quit(void);

#include <krb.h>
int get_mr_update_ticket(char *host, KTEXT ticket);
