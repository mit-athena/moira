/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_server.h,v $
 *	$Author: danw $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_server.h,v 1.31 1998-01-07 17:13:39 danw Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 */

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "mr_proto.h"
#include <moira_site.h>
#include <krb.h>
#include <com_err.h>
#include <stdlib.h>
#include <stdarg.h>

typedef struct returned_tuples {
  struct returned_tuples *next;
  OPERATION op;
  mr_params *retval;
} returned_tuples;

/*
 * This should be in the kerberos header file.
 */

struct krbname {
  char name[ANAME_SZ];
  char inst[INST_SZ];
  char realm[REALM_SZ];
};

/*
 * This structure holds all per-client information; one of these is
 * allocated for each active client.
 */

typedef struct _client {
  OPERATION pending_op;		/* Primary pending operation */
  CONNECTION con;		/* Connection to the client */
  int action;			/* what action is pending? */
  mr_params *args, reply;
  int id;			/* Unique id of client */
  struct sockaddr_in haddr; 	/* IP address of client */
  char clname[MAX_K_NAME_SZ];	/* Name client authenticated to */
  struct krbname kname; 	/* Parsed version of the above */
  int users_id;			/* Moira-internal ID of authenticated user */
  int client_id;		/* Moira-internal ID of client for modby field */
  returned_tuples *first, *last;
  time_t last_time_used;	/* Last time connection used */
  char entity[9];		/* entity on other end of the connection */
} client;

/*
 * States
 */

#define CL_DEAD 0
#define CL_STARTING 1

/*
 * Actions.
 */

#define CL_ACCEPT 0
#define CL_RECEIVE 1
#define CL_SEND 2

extern char *krb_realm;

/*
 * Debugging options.
 */

extern int log_flags;

#define LOG_CONNECT		0x0001
#define LOG_REQUESTS		0x0002
#define LOG_ARGS		0x0004
#define LOG_RESP		0x0008
#define LOG_RES			0x0010
#define LOG_VALID		0x0020
#define LOG_SQL			0x0040
#define LOG_GDSS		0x0080


/* max length of query argument allowed */
#define ARGLEN	257
/* Used to setup static argv, maximum argc */
#define QMAXARGS	22

/* statistics on number of queries by version number */
extern int newqueries, oldqueries;

/* Maximum and minimum values that will ever be chosen for IDs */
#define MAX_ID_VALUE	31999
#define MIN_ID_VALUE	100

/* Sleepy states for the server! */
#define AWAKE 0
#define SLEEPY 1
#define ASLEEP 2
#define GROGGY 3
extern int dormant;

/* state for the incremental update system */
extern int inc_running, inc_pid;
extern time_t inc_started, now;
#define INC_TIMEOUT (3 * 60)	/* 3 minutes */


#define SQL_NO_MATCH 1403 /* oracle, not ingres (= 100) */

/* prototypes from gdb */
int gdb_init(void);
int gdb_debug(int flag);
void start_accepting_client(CONNECTION, OPERATION, CONNECTION *,
			     char *, int *, TUPLE *);
int initialize_operation(OPERATION, int (*init_function)(), char *,
			 int (*cancel_function)());
int reset_operation(OPERATION);
int delete_operation(OPERATION);
int start_replying_to_client(OPERATION, CONNECTION, int, char *, char *);
int op_select(LIST_OF_OPERATIONS, int, fd_set *, fd_set *, fd_set *,
	      struct timeval *);

/* prototypes from libmoira */
struct save_queue *sq_create(void);
int sq_save_data(struct save_queue *sq, void *data);
int sq_save_unique_data(struct save_queue *sq, void *data);
int sq_save_args(int argc, void *argv[], struct save_queue *sq);
int sq_get_data(struct save_queue *sq, void *data);
int sq_remove_data(struct save_queue *sq, void *data);
int sq_empty(struct save_queue *sq);
void sq_destroy(struct save_queue *sq);

void send_zgram(char *instance, char *buf);
void critical_alert(char *, char *, ...);
void mr_destroy_reply(mr_params *reply);
int gdss2et(int);

/* prototypes from increment.dc */
void incremental_init(void);
void next_incremental(void);
void incremental_update(void);
void incremental_flush(void);

/* prototypes from qrtn.dc */
void dbmserr(void);
void dosql(char *buffers[]);
int mr_open_database(void);
void mr_close_database(void);
int mr_process_query(client *cl, char *name, int argc, char *argv_ro[],
		     int (*action)(), char *actarg);
int mr_check_access(client *cl, char *name, int argc, char *argv_ro[]);
void sanity_check_queries(void);
int set_krb_mapping(char *name, char *login, int ok, int *kid, int *uid);
int find_member(char *list_type, int list_id, client *cl);
int do_for_all_rows(char *query, int count, int (*action)(), int actarg);
int build_qual(char *fmt, int argc, char *argv[], char *qual);


/* prototyoes from qsupport.dc */
int set_pop_usage(int id, int cnt);

/* prototypes from qvalidate.dc */
void sanity_check_database(void);
int add_string(char *name);
int convert_wildcards(char *arg);

/* prototypes from mr_main.c */
void clist_delete(client *cp);

/* prototypes from mr_sauth.c */
void do_auth(client *cl);

/* prototypes from mr_scall.c */
void do_client(client *cp);
int trigger_dcm(int dummy0, int dummy1, client *cl);

/* prototypes from mr_shutdown.c */
void sigshut(int);
void do_shutdown(client *cl);

/* prototypes from mr_util.c */
char *requote(char *buf, char *cp, int len);
void log_args(char *tag, int version, int argc, char **argv);
void mr_com_err(const char *whoami, long code, const char *fmt, va_list pvar);
int mr_trim_args(int argc, char **argv);
char **mr_copy_args(char **argv, int argc);
