/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/query.h,v 2.4 1998-01-05 19:53:47 danw Exp $
 *
 * Structures and constants used in the query dispatch table
 */

/* Query Types */
enum query_type {RETRIEVE, UPDATE, APPEND, DELETE, SPECIAL};

/* Tables */
enum tables {NO_TABLE, USERS_TABLE, KRBMAP_TABLE, MACHINE_TABLE,
	     HOSTALIAS_TABLE, SUBNET_TABLE, CLUSTER_TABLE, MCMAP_TABLE,
	     SVC_TABLE, LIST_TABLE, IMEMBERS_TABLE, SERVERS_TABLE,
	     SERVERHOSTS_TABLE, FILESYS_TABLE, FSGROUP_TABLE, NFSPHYS_TABLE,
	     QUOTA_TABLE, ZEPHYR_TABLE, HOSTACCESS_TABLE, STRINGS_TABLE,
	     SERVICES_TABLE, PRINTCAP_TABLE, PALLADIUM_TABLE, CAPACLS_TABLE,
	     ALIAS_TABLE, NUMVALUES_TABLE, TBLSTATS_TABLE, INCREMENTAL_TABLE};

/* Query Definition Structure */
struct query
{
  char *name;			/* query name */
  char *shortname;		/* abbreviated query name (must be 4 chars) */
  enum query_type type;		/* query type */
  char *rvar;			/* range variable */
  enum tables rtable;		/* range table */
  char *tlist;			/* target list */
  char **fields;		/* input and output variable names (for help) */
  int  vcnt;			/* variable count */
  char *qual;			/* format string for "where" clause */
  int  argc;			/* number of args for qualifier */
  struct validate *validate;	/* validation support */
  int  acl;			/* hint as to query ACL for this query */
  int  everybody;		/* is the default user on this ACL? */
};

/* Query Validation Structure */
struct validate
{
  /* object validation (e.g., user, filesys, type) */
  struct valobj *valobj;	/* array of object validation descriptors */
  int objcnt;			/* size of array */
  /* row validation - retrieve (exists = any(rvar.field where qual)) */
  char *field;			/* field to check for */
  char *qual;			/* format string for "where" clause */
  int  argc;			/* number of args used in qual */
  /* values field containing current max object id */
  char *object_id;
  /* routine to verify access permission on objects */
  int (*acs_rtn)();
  /* pre-processing routine (var setup only) */
  int (*pre_rtn)();
  /* post-processing routine */
  int (*post_rtn)();
};

/* Validated Object Types */
enum vo_type {V_NAME, V_ID, V_TYPE, V_TYPEDATA, V_DATE,
	      V_SORT, V_RENAME, V_CHAR, V_LOCK, V_WILD, V_UPWILD,
              V_RLOCK, V_LEN};

/* Validated Object Definition */
struct valobj
{
  enum vo_type type;
  int index;			/* index of object to validate */
  enum tables table;     	/* table containing object */
  char *namefield;		/* table's fieldname for object */
  char *idfield;		/* table's corresponding id field (if any) */
  int error;
};


/* prototypes from cache.dc */
void flush_cache(void);
int name_to_id(char *name, enum tables type, int *id);
int id_to_name(int id, enum tables type, char **name);
int cache_entry(char *name, enum tables type, int id);
void flush_name(char *name, enum tables type);
void cache_commit(void);
void cache_abort(void);

/* prototypes from increment.dc */
void incremental_before(enum tables table, char *qualx, char **argv);
void incremental_clear_before(void);
void incremental_after(enum tables table, char *qualx, char **argv);
void incremental_clear_after(void);

/* prototypes from qfollow.dc */
int set_pobox_modtime(struct query *q, char *argv[], client *cl);

/* prototypes from qrtn.dc */
int check_query_access(struct query *q, char *argv[], client *cl);
int set_next_object_id(char *objectx, enum tables table, int limit);

/* prototypes from qsubs.c */
void list_queries(int version, int (*action)(), char *actarg);
void help_query(struct query *q, int (*action)(), char *actarg);
struct query *get_query_by_name(char *name, int version);
