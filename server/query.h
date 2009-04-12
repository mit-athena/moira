/* $Id: query.h,v 2.15 2008-08-29 14:50:08 zacheiss Exp $
 *
 * Structures and constants used in the query dispatch table
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

/* Query Types */
enum query_type {MR_Q_RETRIEVE, MR_Q_UPDATE, MR_Q_APPEND, MR_Q_DELETE, MR_Q_SPECIAL};

/* Query Definition Structure */
struct query
{
  char *name;			/* query name */
  char *shortname;		/* abbreviated query name (must be 4 chars) */
  int  version;			/* query version */
  enum query_type type;		/* query type */
  char *rvar;			/* range variable */
  enum tables rtable;		/* range table */
  char *tlist;			/* target list */
  char **fields;		/* input and output variable names (for help) */
  int  vcnt;			/* variable count */
  char *qual;			/* format string for "where" clause */
  int  argc;			/* number of args for qualifier */
  char *sort;			/* fields to sort on */
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
  int (*acs_rtn)(struct query *q, char *Argv[], client *cl);
  /* pre-processing routine (var setup only) */
  int (*pre_rtn)(struct query *q, char *Argv[], client *cl);
  /* post-processing routine */
  int (*post_rtn)();
};

/* Validated Object Types */
enum vo_type {V_NAME, V_ID, V_TYPE, V_TYPEDATA, V_RENAME, V_CHAR,
	      V_LEN, V_NUM};

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

/* prototypes from increment.dc */
void incremental_before(enum tables table, char *qualx, char **argv);
void incremental_clear_before(void);
void incremental_after(enum tables table, char *qualx, char **argv);
void incremental_clear_after(void);

/* prototypes from qrtn.dc */
int check_query_access(struct query *q, char *argv[], client *cl);
int set_next_object_id(char *objectx, enum tables table, int limit);
int name_to_id(char *name, enum tables type, int *id);
int id_to_name(int id, enum tables type, char **name);

/* prototypes from qsubs.c */
void list_queries(client *cl, int (*action)(int, char *[], void *),
		  void *actarg);
void help_query(struct query *q, int (*action)(int, char *[], void *),
		void *actarg);
struct query *get_query_by_name(char *name, int version);
