/* prototypes for the update_server */

void auth_002(int conn, char *str);
void exec_002(int conn, char *str);
void xfer_002(int conn, char *str);
void xfer_003(int conn, char *str);

/* from checksum.c */
long checksum_file(char *path);

/* from config.c */
char *config_lookup(char *key);

/* from get_file.c */
int get_file(int conn, char *pathname, int file_size, int checksum,
	     int mode, int encrypt);

/* from log.c */
void mr_update_initialize(void);
void mr_log_error(char *msg);
void mr_log_warning(char *msg);
void mr_log_info(char *msg);
void mr_debug(char *msg);

/* from update_server.c */
void initialize(void);
int send_ok(int conn);
void quit(int conn, char *str);
void lose(char *msg);
void report_error(char *msg);
void reject_call(int c);

extern char *whoami, *hostname;
extern int have_authorization, uid;
