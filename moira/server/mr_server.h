/* $Id: mr_server.h,v 1.34 1998-02-23 19:24:32 danw Exp $
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <moira.h>
#include <mr_private.h>
#include <moira_site.h>
#include <moira_schema.h>

#include <netinet/in.h>

#include <stdarg.h>

#include <krb.h>

/* This should be in the kerberos header file. */

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
  int con;			/* Connection to the client */
  int id;			/* Unique id of client */
  struct sockaddr_in haddr; 	/* IP address of client */
  char clname[MAX_K_NAME_SZ];	/* Name client authenticated to */
  struct krbname kname; 	/* Parsed version of the above */
  char entity[9];		/* client program being used */
  int users_id;			/* Moira-internal ID of authenticated user */
  int client_id;		/* Moira-internal ID of client */
  time_t last_time_used;	/* Last time connection used */
  mr_params *tuples;		/* Tuples waiting to send back to client */
  int ntuples;			/* Number of tuples waiting */
  int tuplessize;		/* Current size of tuple array */
  int nexttuple;		/* Next tuple to return */
  int done;			/* Close up next time through loop */
} client;

extern char krb_realm[REALM_SZ];

/* max length of query argument allowed */
#define ARGLEN	257
/* Used to setup static argv, maximum argc */
#define QMAXARGS	22

/* statistics on number of queries by version number */
extern int newqueries;

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

/* types needed for prototypes */
struct query;
struct validate;
struct valobj;

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
		     int (*action)(int, char *[], void *), void *actarg);
int mr_check_access(client *cl, char *name, int argc, char *argv_ro[]);
void sanity_check_queries(void);
int set_krb_mapping(char *name, char *login, int ok, int *kid, int *uid);
int find_member(char *list_type, int list_id, client *cl);
int do_for_all_rows(char *query, int count,
		    int (*action)(int, char *[], void *), void *actarg);
void build_qual(char *fmt, int argc, char *argv[], char *qual);


/* prototyoes from qsupport.dc */
int set_pop_usage(int id, int cnt);

/* prototypes from qvalidate.dc */
void sanity_check_database(void);
int add_string(char *name);
int convert_wildcards(char *arg);

/* prototypes from mr_main.c */
void clist_delete(client *cp);

/* prototypes from mr_sauth.c */
void do_auth(client *cl, mr_params req);

/* prototypes from mr_scall.c */
void do_client(client *cl);
void client_reply(client *cl, long status);
void client_return_tuple(client *cl, int argc, char **argv);
void client_read(client *cl);
void client_write(client *cl);

/* prototypes from mr_shutdown.c */
void sigshut(int);
void do_shutdown(client *cl);

/* prototypes from mr_util.c */
char *requote(char *buf);
void log_args(char *tag, int version, int argc, char **argv);
void mr_com_err(const char *whoami, long code, const char *fmt, va_list pvar);
int mr_trim_args(int argc, char **argv);
char **mr_copy_args(char **argv, int argc);
void *xmalloc(size_t);
void *xrealloc(void *, size_t);
char *xstrdup(char *);

/* prototypes from qaccess.pc */
int access_user(struct query *q, char *argv[], client *cl);
int access_login(struct query *q, char *argv[], client *cl);
int access_list(struct query *q, char *argv[], client *cl);
int access_visible_list(struct query *q, char *argv[], client *cl);
int access_vis_list_by_name(struct query *q, char *argv[], client *cl);
int access_member(struct query *q, char *argv[], client *cl);
int access_qgli(struct query *q, char *argv[], client *cl);
int access_service(struct query *q, char *argv[], client *cl);
int access_filesys(struct query *q, char *argv[], client *cl);
int access_host(struct query *q, char *argv[], client *cl);
int access_ahal(struct query *q, char *argv[], client *cl);
int access_snt(struct query *q, char *argv[], client *cl);

/* prototypes from qfollow.pc */
int followup_fix_modby(struct query *q, struct save_queue *sq,
		       struct validate *v, int (*action)(int, char **, void *),
		       void *actarg, client *cl);
int followup_gpob(struct query *q, struct save_queue *sq, struct validate *v,
		  int (*action)(int, char **, void *), void *actarg,
		  client *cl);
int followup_glin(struct query *q, struct save_queue *sq, struct validate *v,
		  int (*action)(int, char **, void *), void *actarg,
		  client *cl);
int followup_gzcl(struct query *q, struct save_queue *sq, struct validate *v,
		  int (*action)(int, char **, void *), void *actarg,
		  client *cl);
int followup_gsha(struct query *q, struct save_queue *sq, struct validate *v,
		  int (*action)(int, char **, void *), void *actarg,
		  client *cl);
int followup_gqot(struct query *q, struct save_queue *sq, struct validate *v,
		  int (*action)(int, char **, void *), void *actarg,
		  client *cl);
int followup_gpce(struct query *q, struct save_queue *sq, struct validate *v,
		  int (*action)(int, char **, void *), void *actarg,
		  client *cl);
int followup_guax(struct query *q, struct save_queue *sq, struct validate *v,
		  int (*action)(int, char **, void *), void *actarg,
		  client *cl);
int followup_gsnt(struct query *q, struct save_queue *sq, struct validate *v,
		  int (*action)(int, char **, void *), void *actarg,
		  client *cl);
int followup_ghst(struct query *q, struct save_queue *sq, struct validate *v,
		  int (*action)(int, char **, void *), void *actarg,
		  client *cl);

int followup_ausr(struct query *q, char *argv[], client *cl);
int followup_aqot(struct query *q, char *argv[], client *cl);
int followup_dqot(struct query *q, char *argv[], client *cl);
int followup_uuac(struct query *q, char *argv[], client *cl);

int set_modtime(struct query *q, char *argv[], client *cl);
int set_modtime_by_id(struct query *q, char *argv[], client *cl);
int set_finger_modtime(struct query *q, char *argv[], client *cl);
int set_pobox_modtime(struct query *q, char *argv[], client *cl);
int set_uppercase_modtime(struct query *q, char *argv[], client *cl);
int set_mach_modtime_by_id(struct query *q, char *argv[], client *cl);
int set_cluster_modtime_by_id(struct query *q, char *argv[], client *cl);
int set_serverhost_modtime(struct query *q, char *argv[], client *cl);
int set_nfsphys_modtime(struct query *q, char *argv[], client *cl);
int set_filesys_modtime(struct query *q, char *argv[], client *cl);
int set_zephyr_modtime(struct query *q, char *argv[], client *cl);
int _sdl_followup(struct query *q, char *argv[], client *cl);
int trigger_dcm(struct query *q, char *argv[], client *cl);

/* prototypes from qsetup.pc */
int prefetch_value(struct query *q, char *argv[], client *cl);
int prefetch_filesys(struct query *q, char *argv[], client *cl);
int setup_ausr(struct query *q, char *argv[], client *cl);
int setup_dusr(struct query *q, char *argv[], client *cl);
int setup_spop(struct query *q, char *argv[], client *cl);
int setup_dpob(struct query *q, char *argv[], client *cl);
int setup_dmac(struct query *q, char *argv[], client *cl);
int setup_dclu(struct query *q, char *argv[], client *cl);
int setup_alis(struct query *q, char *argv[], client *cl);
int setup_dlis(struct query *q, char *argv[], client *cl);
int setup_dsin(struct query *q, char *argv[], client *cl);
int setup_dshi(struct query *q, char *argv[], client *cl);
int setup_afil(struct query *q, char *argv[], client *cl);
int setup_ufil(struct query *q, char *argv[], client *cl);
int setup_dfil(struct query *q, char *argv[], client *cl);
int setup_aftg(struct query *q, char *argv[], client *cl);
int setup_dnfp(struct query *q, char *argv[], client *cl);
int setup_dqot(struct query *q, char *argv[], client *cl);
int setup_akum(struct query *q, char *argv[], client *cl);
int setup_dsnt(struct query *q, char *argv[], client *cl);
int setup_ahst(struct query *q, char *argv[], client *cl);
int setup_ahal(struct query *q, char *argv[], client *cl);

/* prototypes from qsupport.pc */
int set_pobox(struct query *q, char *argv[], client *cl);
int add_member_to_list(struct query *q, char *argv[], client *cl);
int delete_member_from_list(struct query *q, char *argv[], client *cl);
int register_user(struct query *q, char *argv[], client *cl);

int get_list_info(struct query *q, char **argv, client *cl,
		  int (*action)(int, char *[], void *), void *actarg);
int get_ace_use(struct query *q, char **argv, client *cl,
		int (*action)(int, char *[], void *), void *actarg);
int qualified_get_lists(struct query *q, char **argv, client *cl,
			int (*action)(int, char *[], void *), void *actarg);
int get_members_of_list(struct query *q, char **argv, client *cl,
			int (*action)(int, char *[], void *), void *actarg);
int get_end_members_of_list(struct query *q, char **argv, client *cl,
			    int (*action)(int, char *[], void *),
			    void *actarg);
int qualified_get_server(struct query *q, char **argv, client *cl,
			 int (*action)(int, char *[], void *), void *actarg);
int qualified_get_serverhost(struct query *q, char **argv, client *cl,
			     int (*action)(int, char *[], void *),
			     void *actarg);
int count_members_of_list(struct query *q, char **argv, client *cl,
			  int (*action)(int, char *[], void *), void *actarg);
int get_lists_of_member(struct query *q, char **argv, client *cl,
			int (*action)(int, char *[], void *), void *actarg);

/* prototypes from qvalidate.pc */
/* from qvalidate.dc */
int validate_fields(struct query *q, char *argv[], struct valobj *vo, int n);
int validate_row(struct query *q, char *argv[], struct validate *v);
