#include <sys/time.h>
#include <des.h>

typedef struct reg_client {
  int fd;				/* socket */
  int clientid;				/* client id */
  unsigned char *buf;			/* buffer for data read */
  int nread, nmax;			/* # of bytes cur/max in buffer */
  time_t lastmod;			/* last time we sent/received */
  int encrypted;			/* set if sched is valid */
  des_key_schedule sched;		/* DES key schedule */
  long uid;				/* uid of user. set by RIFO */
  char *id;				/* stored MIT ID if "six words" are
					   needed. NULL otherwise. Set by
					   RIFO, cleared by SWRD */
  char *suggestions;			/* suggested usernames */
  char *username;			/* desired username. set by LOGN */
  int reserved_username;		/* if username wasn't picked by user */
  unsigned short *random;		/* random state */
} reg_client;

void RIFO(reg_client *rc, int argc, char **argv);
void SWRD(reg_client *rc, int argc, char **argv);
void LOGN(reg_client *rc, int argc, char **argv);
void PSWD(reg_client *rc, int argc, char **argv);
void QUIT(reg_client *rc, int argc, char **argv);

#define REG_SVR_PRINCIPAL "sms"
#define REG_SVR_INSTANCE ""

#define REG_SVR_RSA_KEY "/moira/reg_svr/rsa_key"
#define REG_SVR_HMAC_KEY "/moira/reg_svr/hmac_key"

#define REG_SVR_ERROR_MESSAGES "/moira/reg_svr/errors"
enum { NO_MESSAGE, INTERNAL_ERROR, PROTOCOL_ERROR, DATABASE_CLOSED,
       ENCRYPT_KEY, NOT_FOUND_IN_DATABASE, ALREADY_REGISTERED,
       ACCOUNT_DELETED, NOT_ELIGIBLE, FOUND, FORCED_USERNAME,
       BAD_SIX_WORDS, BAD_USERNAME, USERNAME_UNAVAILABLE, 
       RESERVED_USERNAME_UNAVAILABLE, USERNAME_OK, PASSWORD_SHORT,
       PASSWORD_SIMPLE, PASSWORD_SAMPLE, KADM_ERROR, DONE,
       NUM_REG_ERRORS };

#define TIMEOUT 300 /* 5 minutes */

#define REG_RSA_ENCRYPTED_KEY 0x43
#define REG_ENCRYPTED 0x45
#define REG_UNENCRYPTED 0x50

/* Prototypes from kerberos.c */
long init_kerberos(void);
long check_kerberos(char *username);
long register_kerberos(char *username, char *password);

/* Prototypes from procotol.c */
int read_rsa_key(void);
int read_errors(void);
void reply(reg_client *rc, int msg, char *state, char *clean, char *data,
	   ...);
void parse_packet(reg_client *rc, int type, long len, char *buf, int sleeping);

/* prototypes from reg_svr.pc */
void *xmalloc(size_t);
void *xrealloc(void *, size_t);
char *xstrdup(char *);

/* Prototypes from words.c */
int read_hmac_key(void);
unsigned short *init_rand(void *);
void getwordlist(char *input, char *words[]);
