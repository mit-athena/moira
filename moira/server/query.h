/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/query.h,v 2.0 1992-08-04 12:10:42 genoa Exp $
 *
 * Structures and constants used in the query dispatch table
 */

/* Query Types */
enum query_type {RETRIEVE, UPDATE, APPEND, DELETE, SPECIAL};

/* Query Definition Structure */
struct query
{
    char *name;			/* query name */
    char *shortname;		/* abbreviated query name (must be 4 chars) */
    enum query_type type;	/* query type */
    char *rvar;			/* range variable */
    char *rtable;		/* range table */
    char *tlist;		/* target list */
    char **fields;		/* input and output variable names (for help) */
    int  vcnt;			/* variable count */
    char *qual;			/* format string for "where" clause */
    int  argc;			/* number of args for qualifier */
    struct validate *validate;	/* validation support */
    int	 acl;			/* hint as to query ACL for this query */
    int  everybody;		/* is the default user on this ACL? */
};

/* Query Validation Structure */
struct validate
{
    /* object validation (e.g., user, filesys, type) */
    struct valobj *valobj;	/* array of object validation descriptors */
    int objcnt;			/* size of array */
    /* row validation - retrieve (exists = any(rvar.field where qual)) */
    char *field;		/* field to check for */
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
	      V_SORT, V_RENAME, V_CHAR, V_LOCK, V_WILD, V_UPWILD};

/* Validated Object Definition */
struct valobj
{
    enum vo_type type;
    int index;			/* index of object to validate */
    char *table;		/* table containing object */
    char *namefield;		/* table's fieldname for object */
    char *idfield;		/* table's corresponding id field (if any) */
    int error;
};

/* Used to setup static argv, maximum argc and argument lengths */
#define QMAXARGS	20
#define QMAXARGSIZE	513

