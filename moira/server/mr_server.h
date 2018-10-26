/* $Id$
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

#include <stdio.h>
#include <stdarg.h>

#include <mr_krb.h>
#include <krb5.h>

enum clstate { CL_ACCEPTING, CL_ACTIVE, CL_CLOSING };

/*
 * This structure holds all per-client information; one of these is
 * allocated for each active client.
 */

typedef struct _client {
  int con;			/* Connection to the client */
  int id;			/* Unique id of client */
  struct sockaddr_in haddr; 	/* IP address of client */
  enum clstate state;		/* State of the connection */
  char clname[MAX_K_NAME_SZ];	/* Name client authenticated to */
  char entity[USERS_MODWITH_SIZE]; /* client program being used */
  int users_id;			/* Moira-internal ID of authenticated user */
  int client_id;		/* Moira-internal ID of client */
  int proxy_id;			/* client_id of orig user, if proxied */
  int version;			/* Max query version known by client */
  time_t last_time_used;	/* Last time connection used */
  mr_params req;		/* Current request */
  mr_params *tuples;		/* Tuples waiting to send back to client */
  int ntuples;			/* Number of tuples waiting */
  int tuplessize;		/* Current size of tuple array */
  int nexttuple;		/* Next tuple to return */
  char *hsbuf;			/* Buffer for initial connection handshaking */
  int hslen;			/* Length of data in hsbuf */
} client;

struct mxentry
{
  char *name;
  int pref;
};

extern char *krb_realm;

/* max length of query argument allowed */
#define ARGLEN	MAX_FIELD_WIDTH
/* Used to setup static argv, maximum argc */
#define QMAXARGS	30

/* statistics on number of queries by version number */
extern int newqueries;

/* Maximum and minimum values that will be used for uids and gids */
#define MAX_ID_VALUE	262144
#define MIN_ID_VALUE	100

/* VLAN constants */
#define MIN_VLAN_VALUE 1
#define MAX_VLAN_VALUE 4095

/* DNS constants */
#define MAX_TTL_VALUE 2147483647

/* IPv6 constants */
#define MIN_IPV6_ADDR "::"
#define MAX_IPV6_ADDR "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"
#define MIN_IPV6_PREFIX_LEN 64

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
char *build_qual(char *fmt, int argc, char *argv[]);


/* prototypes from qsupport.dc */
int set_pop_usage(int id, int cnt);

/* prototypes from qvalidate.dc */
void sanity_check_database(void);
int add_string(char *name);
int convert_wildcards(char *arg);

/* prototypes from mr_main.c */
void clist_delete(client *cp);

/* prototypes from mr_sauth.c */
void do_auth(client *cl);
void do_proxy(client *cl);
void do_krb5_auth(client *cl);

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
char *masksize_to_mask(char *addr_type, unsigned int prefix);
int mask_to_masksize(char *addr_type, char *mask);

/* prototypes from qaccess.pc */
int access_user(struct query *q, char *argv[], client *cl);
int access_login(struct query *q, char *argv[], client *cl);
int access_spob(struct query *q, char *argv[], client *cl);
int access_list(struct query *q, char *argv[], client *cl);
int access_visible_list(struct query *q, char *argv[], client *cl);
int access_vis_list_by_name(struct query *q, char *argv[], client *cl);
int access_member(struct query *q, char *argv[], client *cl);
int access_qgli(struct query *q, char *argv[], client *cl);
int access_service(struct query *q, char *argv[], client *cl);
int access_filesys(struct query *q, char *argv[], client *cl);
int access_shot(struct query *q, char *argv[], client *cl);
int access_host(struct query *q, char *argv[], client *cl);
int access_adhr(struct query *q, char *argv[], client *cl);
int access_ahal(struct query *q, char *argv[], client *cl);
int access_ahad(struct query *q, char *argv[], client *cl);
int access_machidentifier(struct query *q, char *argv[], client *cl);
int access_uhp4(struct query *q, char *argv[], client *cl);
int access_snt(struct query *q, char *argv[], client *cl);
int access_printer(struct query *q, char *argv[], client *cl);
int access_zephyr(struct query *q, char *argv[], client *cl);
int access_container(struct query *q, char *argv[], client *cl);
int access_update_user(struct query *q, char *argv[], client *cl);
int check_mail_string(char *mailstring);
struct mxentry *getmxrecords(const char *);
int check_roles_authorization(char *login, char *function_name,
			      char *qualifier_code);

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
int followup_gsin(struct query *q, struct save_queue *sq, struct validate *v,
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
int followup_guax(struct query *q, struct save_queue *sq, struct validate *v,
		  int (*action)(int, char **, void *), void *actarg,
		  client *cl);
int followup_gsnt(struct query *q, struct save_queue *sq, struct validate *v,
		  int (*action)(int, char **, void *), void *actarg,
		  client *cl);
int followup_ghst(struct query *q, struct save_queue *sq, struct validate *v,
		  int (*action)(int, char **, void *), void *actarg,
		  client *cl);
int followup_gpsv(struct query *q, struct save_queue *sq, struct validate *v,
		  int (*action)(int, char **, void *), void *actarg,
		  client *cl);
int followup_gcon(struct query *q, struct save_queue *sq, struct validate *v,
		  int (*action)(int, char **, void *), void *actarg,
		  client *cl);
int followup_get_user(struct query *q, struct save_queue *sq, 
		      struct validate *v, int (*action)(int, char **, void *), 
		      void *actarg, client *cl);

int followup_ausr(struct query *q, char *argv[], client *cl);
int followup_aqot(struct query *q, char *argv[], client *cl);
int followup_dqot(struct query *q, char *argv[], client *cl);
int followup_ahad(struct query *q, char *argv[], client *cl);
int followup_dhad(struct query *q, char *argv[], client *cl);
int followup_srrt(struct query *q, char *argv[], client *cl);
int followup_sttl(struct query *q, char *argv[], client *cl);

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
int set_service_modtime(struct query *q, char *argv[], client *cl);
int _sdl_followup(struct query *q, char *argv[], client *cl);
int trigger_dcm(struct query *q, char *argv[], client *cl);

/* prototypes from qsetup.pc */
int prefetch_value(struct query *q, char *argv[], client *cl);
int prefetch_filesys(struct query *q, char *argv[], client *cl);
int setup_ausr(struct query *q, char *argv[], client *cl);
int setup_dusr(struct query *q, char *argv[], client *cl);
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
int setup_asnt(struct query *q, char *argv[], client *cl);
int setup_dsnt(struct query *q, char *argv[], client *cl);
int setup_ghst(struct query *q, char *argv[], client *cl);
int setup_ahst(struct query *q, char *argv[], client *cl);
int setup_ahal(struct query *q, char *argv[], client *cl);
int setup_ahad(struct query *q, char *argv[], client *cl);
int setup_dhad(struct query *q, char *argv[], client *cl);
int setup_uhp4(struct query *q, char *argv[], client *cl);
int setup_srrt(struct query *q, char *argv[], client *cl);
int setup_shap(struct query *q, char *argv[], client *cl);
int setup_ahid(struct query *q, char *argv[], client *cl);
int setup_aprn(struct query *q, char *argv[], client *cl);
int setup_dpsv(struct query *q, char *argv[], client *cl);
int setup_dcon(struct query *q, char *argv[], client *cl);
int setup_acon(struct query *q, char *argv[], client *cl);
int setup_scli(struct query *q, char *argv[], client *cl);
int setup_aali(struct query *q, char *argv[], client *cl);
int setup_sttl(struct query *q, char *argv[], client *cl);
int setup_ahrr(struct query *q, char *argv[], client *cl);
int setup_uust(struct query *q, char *argv[], client *cl);

/* prototypes from qsupport.pc */
int set_pobox(struct query *q, char *argv[], client *cl);
int set_pobox_pop(struct query *q, char *argv[], client *cl);
int add_member_to_list(struct query *q, char *argv[], client *cl);
int delete_member_from_list(struct query *q, char *argv[], client *cl);
int tag_member_of_list(struct query *q, char *argv[], client *cl);
int register_user(struct query *q, char *argv[], client *cl);
int do_user_reservation(struct query *q, char *argv[], client *cl);
int update_container(struct query *q, char *argv[], client *cl);
int set_container_list(struct query *q, char *argv[], client *cl);
int update_user_password_expiration(struct query *q, char *argv[], client *cl);
int upgrade_host_private_ipv4_addr(struct query *q, char *argv[], client *cl);
int get_ace_use(struct query *q, char **argv, client *cl,
		int (*action)(int, char *[], void *), void *actarg);
int get_host_by_owner(struct query *q, char **argv, client *cl,
		int (*action)(int, char *[], void *), void *actarg);
int get_host_usage(struct query *q, char **argv, client *cl,
		   int (*action)(int, char *[], void *), void *actarg);
int get_sponsored_user_accounts(struct query *q, char **argv, client *cl,
				int (*action)(int, char *[], void *),
				void *actarg);
int qualified_get_lists(struct query *q, char **argv, client *cl,
			int (*action)(int, char *[], void *), void *actarg);
int get_members_of_list(struct query *q, char **argv, client *cl,
			int (*action)(int, char *[], void *), void *actarg);
int qualified_get_server(struct query *q, char **argv, client *cl,
			 int (*action)(int, char *[], void *), void *actarg);
int qualified_get_serverhost(struct query *q, char **argv, client *cl,
			     int (*action)(int, char *[], void *),
			     void *actarg);
int count_members_of_list(struct query *q, char **argv, client *cl,
			  int (*action)(int, char *[], void *), void *actarg);
int get_lists_of_member(struct query *q, char **argv, client *cl,
			int (*action)(int, char *[], void *), void *actarg);
int get_user_reservations(struct query *q, char **argv, client *cl,
			  int (*action)(int, char *[], void *), void *actarg);
int get_user_by_reservation(struct query *q, char **argv, client *cl,
			    int (*action)(int, char *[], void *),
			    void *actarg);
int get_machines_of_container(struct query *q, char **argv, client *cl,
			    int (*action)(int, char *[], void *),
			    void *actarg);
int get_subcontainers_of_container(struct query *q, char **argv, client *cl,
			    int (*action)(int, char *[], void *),
			    void *actarg);
int get_incremental_queue_length(struct query *q, char **argv, client *cl,
				 int (*action)(int, char *[], void *),
				 void *actarg);
int add_dynamic_host_record(struct query *q, char **argv, client *cl,
                                 int (*action)(int, char *[], void *),
                                 void *actarg);

/* prototypes from qvalidate.pc */
int validate_fields(struct query *q, char *argv[], struct valobj *vo, int n);
int validate_row(struct query *q, char *argv[], struct validate *v);
