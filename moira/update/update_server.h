/* prototypes for the update_server */

int auth_002(char *str);
int exec_002(char *str);
int xfer_002(char *str);
int xfer_003(char *str);

/* from checksum.c */
int checksum_file(char *path);

/* from config.c */
char *config_lookup(char *key);

/* from get_file.c */
int get_file(char *pathname, int file_size, int checksum,
	     int mode, int encrypt);

/* from log.c */
void mr_update_initialize(void);
void mr_log_error(char *msg);
void mr_log_warning(char *msg);
void mr_log_info(char *msg);
void mr_debug(char *msg);

/* from update_server.c */
void initialize(void);
int send_ok(void);
int quit(char *str);
void lose(char *msg);
void report_error(char *msg);
void reject_call(int c);
