#define log_DEBUG 0
#define log_INFO  1
#define log_WARNING 2
#define log_ERROR 3

#define SERVICE_NAME "moira_update"
/* For unknown reasons, we're running des_pcbc_encrypt in DEcrypt mode,
   not ENcrypt, so we need to guarantee that the block size is a multiple
   of 8 to prevent the data from being zero-padded. */
#define UPDATE_BUFSIZ ((BUFSIZ + 7) & ~7)

int mr_send_file(int conn, char *pathname, char *target_path, int encrypt);
int mr_send_auth(int conn, char *hostname);
int mr_execute(int conn, char *path);
void mr_send_quit(int conn);

#include <krb.h>
int get_mr_update_ticket(char *host, KTEXT ticket);

extern char *whoami;
