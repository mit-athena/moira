/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qrtn.qc,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qrtn.qc,v 1.36 1989-12-28 17:17:26 mar Exp $
 *
 *	Copyright (C) 1987, 1988 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 * 
 */

#ifndef lint
static char *rcsid_qrtn_qc = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qrtn.qc,v 1.36 1989-12-28 17:17:26 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include "query.h"
#include "sms_server.h"

char *Argv[16];

int ingres_errno = 0;
int sms_errcode = 0;
## int query_timeout = 30;
extern char *whoami;
extern FILE *journal;

#define INGRES_BAD_INT 4111
#define INGRES_BAD_DATE 4302
#define INGRES_DEADLOCK 4700
#define INGRES_TIMEOUT 4702

/*
 * ingerr: (supposedly) called when Ingres indicates an error.
 * I have not yet been able to get this to work to intercept a
 * database open error.
 */

static int ingerr(num)
    int *num;
{
    ingres_errno = *num;

    switch (*num) {
    case INGRES_BAD_INT:
	sms_errcode = SMS_INTEGER;
	break;
    case INGRES_BAD_DATE:
	sms_errcode = SMS_DATE;
	break;
    case INGRES_DEADLOCK:
	sms_errcode = SMS_DEADLOCK;
	com_err(whoami, 0, "INGRES deadlock detected");
	break;
    case INGRES_TIMEOUT:
	sms_errcode = SMS_BUSY;
	com_err(whoami, 0, "timed out getting lock");
	break;
    default:
	sms_errcode = SMS_INGRES_ERR;
	com_err(whoami, SMS_INGRES_ERR, " code %d\n", *num);
	critical_alert("SMS", "SMS server encountered INGRES ERROR %d", *num);
	return (*num);
    }
    return (0);
}

int sms_open_database()
{
    register int i;
    char *malloc();
    static first_open = 1;

    if (first_open) {
	first_open = 0;

	/* initialize local argv */
	for (i = 0; i < 16; i++)
	  Argv[i] = malloc(ARGLEN);

	IIseterr(ingerr);
	incremental_init();
	flush_cache();
    }
	
    ingres_errno = 0;

    /* open the database */
##  ingres sms
##  set lockmode session where level = table, timeout = query_timeout
##  set lockmode on capacls where readlock = shared
##  set lockmode on alias where readlock = shared
    return ingres_errno;
}

int sms_close_database()
{
    flush_cache();
##  exit
}

sms_check_access(cl, name, argc, argv_ro)
    client *cl;
    char *name;
    int argc;
    char *argv_ro[];
{
    struct query *q;
    struct query *get_query_by_name();

    ingres_errno = 0;

    q = get_query_by_name(name, cl->args->sms_version_no);
    if (q == (struct query *)0)
	return(SMS_NO_HANDLE);

    return(sms_verify_query(cl, q, argc, argv_ro));
}

sms_process_query(cl, name, argc, argv_ro, action, actarg)
    client *cl;
    char *name;
    int argc;
    char *argv_ro[];
    int (*action)();
    char *actarg;
{
    register struct query *q;
    register int status;
    register struct validate *v;
    char qual[256];
    char sort[32];
    char *pqual;
    char *psort;
##  char *table, *rvar;
    struct save_queue *sq;
    struct query *get_query_by_name();
    int sq_save_args();
    struct save_queue *sq_create();
    char *build_sort();

    ingres_errno = 0;

    /* list queries command */
    if (!strcmp(name, "_list_queries")) {
	list_queries(cl->args->sms_version_no, action, actarg);
	return(SMS_SUCCESS);
    }

    /* help query command */
    if (!strcmp(name, "_help")) {
	if (argc < 1)
	    return(SMS_ARGS);
	q = get_query_by_name(argv_ro[0], cl->args->sms_version_no);
	if (q == (struct query *)0) return(SMS_NO_HANDLE);
	help_query(q, action, actarg);
	return(SMS_SUCCESS);
    }

    /* get query structure, return error if named query does not exist */
    q = get_query_by_name(name, cl->args->sms_version_no);
    if (q == (struct query *)0) return(SMS_NO_HANDLE);
    v = q->validate;

    if (q->type != RETRIEVE) {
##      begin transaction
    }

    /* setup argument vector, verify access and arguments */
    if ((status = sms_verify_query(cl, q, argc, argv_ro)) != SMS_SUCCESS)
	goto out;

    /* perform any special query pre-processing */
    if (v && v->pre_rtn) {
	status = (*v->pre_rtn)(q, Argv, cl, 0);
	if (status != SMS_SUCCESS)
	    goto out;
    }

    switch (q->type) {
    case RETRIEVE:
	/* for queries that do not permit wildcarding, check if row
	   uniquely exists */
	if (v && v->field) {
	    status = validate_row(q, Argv, v);
	    if (status != SMS_EXISTS) break;
	}

	/* build "where" clause if needed */
	if (q->qual) {
	    build_qual(q->qual, q->argc, Argv, qual);
	    pqual = qual;
	} else {
	    pqual = 0;
	}

	/* build "sort" clause if needed */
	if (v && v->valobj) {
	    psort = build_sort(v, sort);
	} else {
	    psort = 0;
	}

	/* if there is a followup routine, then we must save the results */
	/* of the first query for use by the followup routine */
	/* if q->rvar = NULL, perform post_rtn only */
	if (q->rvar) {
	    if (v && v->post_rtn) {
		sq = sq_create();
		status = do_retrieve(q, pqual, psort, sq_save_args, sq);
		if (status != SMS_SUCCESS) {
		    sq_destroy(sq);
		    break;
		}
		status = (*v->post_rtn)(q, sq, v, action, actarg, cl);
	    } else {
		/* normal retrieve */
		status = do_retrieve(q, pqual, psort, action, actarg);
	    }
	    if (status != SMS_SUCCESS) break;
	} else {
	    status = (*v->post_rtn)(q, Argv, cl, action, actarg);
	}

	break;

    case UPDATE:
	/* see if row already exists */
	if (v->field) {
	    status = validate_row(q, Argv, v);
	    if (status != SMS_EXISTS) break;
	}

	/* build "where" clause and perform update */
	/* if q->rvar = NULL, perform post_rtn only */
	if (q->rvar) {
	    build_qual(q->qual, q->argc, Argv, qual);
	    incremental_before(q->rtable, qual, argv_ro);
	    status = do_update(q, &Argv[q->argc], qual, action, actarg);
	    incremental_after(q->rtable, qual, argv_ro);
	    if (status != SMS_SUCCESS) break;
	    flush_name(argv_ro[0], q->rtable);
	    table = q->rtable;
	    if (strcmp(q->shortname, "sshi") && strcmp(q->shortname, "ssif")) {
##		repeat replace tblstats (updates = tblstats.updates + 1,
##					 modtime = "now")
##		    where tblstats.#table = @table
	    }
	}

	/* execute followup routine (if any) */
	if (v->post_rtn) status = (*v->post_rtn)(q, Argv, cl);

	break;

    case APPEND:
	/* see if row already exists */
	if (v->field) {
	    status = validate_row(q, Argv, v);
	    if (status != SMS_NO_MATCH) break;
	}

	/* increment id number if necessary */
	if (v->object_id) {
	    status = set_next_object_id(v->object_id, q->rtable);
	    if (status != SMS_SUCCESS) break;
	}

	/* build "where" clause if needed */
	if (q->qual) {
	    build_qual(q->qual, q->argc, Argv, qual);
	    pqual = qual;
	} else {
	    pqual = 0;
	}

	/* perform the append */
	/* if q->rvar = NULL, perform post_rtn only */
	if (q->rvar) {
	    incremental_clear_before();
	    status = do_append(q, &Argv[q->argc], pqual, action, actarg);
	    if (status != SMS_SUCCESS) break;
	    if (v && v->object_id) {
		sprintf(qual, "%s.%s = values.value and values.name = \"%s\"",
			q->rvar, v->object_id, v->object_id);
		incremental_after(q->rtable, qual, argv_ro);
	    } else
	      incremental_after(q->rtable, pqual, argv_ro);

	    table = q->rtable;
##	    repeat replace tblstats (appends = tblstats.appends + 1,
##				     modtime = "now")
##		where tblstats.#table = @table
	}
	
	/* execute followup routine */
	if (v->post_rtn) status = (*v->post_rtn)(q, Argv, cl);
	break;

    case DELETE:
	/* see if row already exists */
	if (v->field) {
	    status = validate_row(q, Argv, v);
	    if (status != SMS_EXISTS) break;
	}

	/* build "where" clause and perform delete */
	/* if q->rvar = NULL, perform post_rtn only */
	if (q->rvar) {
	    build_qual(q->qual, q->argc, Argv, qual);
	    table = q->rtable;
	    rvar = q->rvar;
##	    range of rvar is table
	    incremental_before(q->rtable, qual, argv_ro);
	    status = do_delete(q, qual, action, actarg);
	    incremental_clear_after();
	    if (status != SMS_SUCCESS) break;
	    flush_name(argv_ro[0], q->rtable);
##	    repeat replace tblstats (deletes = tblstats.deletes + 1,
##				     modtime = "now")
##		where tblstats.#table = @table
	}

	/* execute followup routine */
	if (v->post_rtn) status = (*v->post_rtn)(q, Argv, cl);
	break;

    }

out:
    if (status == SMS_SUCCESS && ingres_errno != 0) {
	com_err(whoami, SMS_INTERNAL, "Server didn't notice INGRES ERROR %d",
		       ingres_errno);
	status = sms_errcode;
    }

    if (q->type != RETRIEVE) {
        if (status == SMS_SUCCESS) {
##          end transaction	/* commit to this */
	    if (journal) {
		char buf[1024], *bp;
		int i;
		extern time_t now;

		fprintf(journal, "%% %s %s %s",
			cl->clname, cl->entity, ctime(&now));
		fprintf(journal, "%s[%d] ", q->name, cl->args->sms_version_no);
		for (i = 0; i < argc; i++) {
		    if (i != 0) {
			putc(' ', journal);
		    }
		    requote(buf, argv_ro[i], sizeof(buf));
		    fputs(buf, journal);
		}
		putc('\n', journal);
		fflush(journal);
	    }
	    incremental_update();
	} else {
	    if (ingres_errno != INGRES_DEADLOCK) {
##          	abort		/* it never happened */
	    }
	    incremental_flush();
	}
##      set lockmode session where readlock = system
    }

    if (status != SMS_SUCCESS && log_flags & LOG_RES)
	com_err(whoami, status, " (Query failed)");
    return(status);
}

build_qual(fmt, argc, argv, qual)
	char *fmt;
	int argc;
	char *argv[];
	char *qual;
{
    register char *c;
    register int i;
    char *args[4];
    char *index();

    c = fmt;
    for (i = 0; i < argc; i++) {
	c = index(c, '%');
	if (c++ == (char *)0) return(SMS_ARGS);
	if (*c == 's')
	    args[i] = argv[i];
	else if (*c == 'd')
	    *(int *)&args[i] = *(int *)argv[i];	/* sigh */
	else
	    return(SMS_INGRES_ERR);
    }

    switch (argc) {
    case 0:
	strcpy(qual, fmt);
	break;

    case 1:
	sprintf(qual, fmt, args[0]);
	break;

    case 2:
	sprintf(qual, fmt, args[0], args[1]);
	break;

    case 3:
	sprintf(qual, fmt, args[0], args[1], args[2]);
	break;

    case 4:
	sprintf(qual, fmt, args[0], args[1], args[2], args[3]);
	break;
    }
    return(SMS_SUCCESS);
}

char *
build_sort(v, sort)
    register struct validate *v;
    char *sort;
{
    register struct valobj *vo;
    register int n;
    char elem[16];

    n = v->objcnt;
    vo = v->valobj;
    *sort = 0;

    while (--n >= 0) {
	if (vo->type == V_SORT) {
	    sprintf(elem, "RET_VAR%d", vo->index + 1);
	    if (*sort) strcat(sort, ", ");
	    strcat(sort, elem);
	}
	vo++;
    }

    return ((*sort) ? sort : 0);
}


/* Build arguement vector, verify query and arguments */

sms_verify_query(cl, q, argc, argv_ro)
    client *cl;
    struct query *q;
    int argc;
    char *argv_ro[];
{
    register int argreq;
    register int status;
    register struct validate *v = q->validate;
    register int i;
    register int privileged = 0;
    int len;

    /* copy the arguments into a local argv that we can modify */
    if (argc >= QMAXARGS)
      return(SMS_ARGS);
    for (i = 0; i < argc; i++) {
	if ((len = strlen(argv_ro[i])) < ARGLEN)
	    strcpy(Argv[i], argv_ro[i]);
	else
	    return(SMS_ARG_TOO_LONG);
	if (Argv[i][len-1] == '\\')
	  return(SMS_BAD_CHAR);
    }

    /* check initial query access */
    status = check_query_access(q, Argv, cl);
    if (status != SMS_SUCCESS && status != SMS_PERM)
	return(status);
    if (status == SMS_SUCCESS)
	privileged++;

    /* check argument count */
    argreq = q->argc;
    if (q->type == UPDATE || q->type == APPEND) argreq += q->vcnt;
    if (argc != argreq) return(SMS_ARGS);

    /* validate arguments */
    if (v && v->valobj) {
	status = validate_fields(q, Argv, v->valobj, v->objcnt);
	if (status != SMS_SUCCESS) return(status);
    }

    /* perform special query access check */
    if (!privileged && v && v->acs_rtn) {
	status = (*v->acs_rtn)(q, Argv, cl);
	if (status != SMS_SUCCESS && status != SMS_PERM)
	    return(status);
	if (status == SMS_SUCCESS)
	    privileged++;
    }

    return(privileged ? SMS_SUCCESS : SMS_PERM);
}


/* This routine caches info from the database.  Each query acl is stored
 * in the query structure, and whether that acl contains everybody.
 */

check_query_access(q, argv, cl)
    struct query *q;
    char *argv[];
    client *cl;
##{
##  char *name;
##  int acl_id;
##  int exists;
##  int rowcount;
##  int errorno;
##  static int def_uid;
    int status;
    int client_id;
    char *client_type;

    /* initialize default uid */
    if (def_uid == 0) {
##	retrieve (def_uid = users.users_id) where users.login = "default"
    }

    /* get query access control list */
    if (q->acl != 0)
      acl_id = q->acl;
    else {
	name = q->shortname;
##	retrieve (acl_id = capacls.list_id) where capacls.tag = name
##	inquire_equel (rowcount = "rowcount", errorno = "errorno")
	if (errorno != 0) return(SMS_INGRES_ERR);
	if (rowcount == 0) return(SMS_PERM);
	q->acl = acl_id;

	/* check for default access */
##	retrieve (exists = any(imembers.#member_id where
##			       imembers.list_id = acl_id and
##			       imembers.member_type = "USER" and
##			       imembers.#member_id = def_uid))
	q->everybody = exists;
    }

    if (q->everybody)
      return(SMS_SUCCESS);

    if (get_client(cl, &client_type, &client_id) != SMS_SUCCESS)
      return(SMS_PERM);
    if (find_member("LIST", acl_id, client_type, client_id, 0))
      return(SMS_SUCCESS);
    else
      return(SMS_PERM);
##}


get_client(cl, client_type, client_id)
    client *cl;
    char **client_type;
    int *client_id;
{
    if (cl->users_id > 0) {
	*client_id = cl->users_id;
	*client_type = "USER";
	return(SMS_SUCCESS);
    }

    if (cl->client_id < 0) {
	*client_id = -cl->users_id;
	*client_type = "KERBEROS";
	return(SMS_SUCCESS);
    }

    return(SMS_PERM);
}

##find_member(list_type, list_id, member_type, member_id)
    char *list_type;
##  int list_id;
##  char *member_type;
##  int member_id;
##{
##  int exists, errorno;

    if (!strcmp(strtrim(list_type), strtrim(member_type)) &&
	list_id == member_id)
	return(1);

    /* see if client is a direct member of list */
##  repeat retrieve (exists = any(imembers.#member_id where 
##				  imembers.#list_id = @list_id and
##				  imembers.#member_type = @member_type and 
##				  imembers.#member_id = @member_id))
##  inquire_equel(errorno = "errorno")
    if (errorno == 0)
      return(exists);
    else
      return(0);
##}


do_retrieve(q, pqual, psort, action, actarg)
    register struct query *q;
    char *pqual;
    char *psort;
    int (*action)();
    char *actarg;
##{
##  char *rvar;
##  char *rtable;
##  char *cqual;
##  char *csort;
##  int rowcount;
##  int errorno;
    static char **vaddrs = (char **)NULL;

    if (!vaddrs) {
	register int i;

	if ((vaddrs = (char **)malloc(sizeof(char *) * QMAXARGS)) == NULL) {
	    com_err(whoami, SMS_NO_MEM, "setting up static argv");
	    exit(1);
	}
	for (i = 0; i < QMAXARGS; i++) {
	    if ((vaddrs[i] = malloc(QMAXARGSIZE)) == NULL) {
		com_err(whoami, SMS_NO_MEM, "setting up static argv");
		exit(1);
	    }
	}
    }

    if (q->rvar) {
	rvar = q->rvar;
	rtable = q->rtable;
##     	range of rvar is rtable
    }

    if (psort) {
	csort = psort;
	if (pqual) {
	    cqual = pqual;
##          retrieve unique (param (q->tlist, vaddrs)) where cqual
##                   sort by csort
##          {
		 (*action)(q->vcnt, vaddrs, actarg);
##          }
	} else {
##          retrieve unique (param (q->tlist, vaddrs))
##                   sort by csort
##          {
		 (*action)(q->vcnt, vaddrs, actarg);
##          }
	}

    } else {
	if (pqual) {
	    cqual = pqual;
##          retrieve unique (param (q->tlist, vaddrs)) where cqual
##          {
		 (*action)(q->vcnt, vaddrs, actarg);
##          }
	} else {
##          retrieve unique (param (q->tlist, vaddrs))
##          {
		 (*action)(q->vcnt, vaddrs, actarg);
##          }
	}
    }

##  inquire_equel (rowcount = "rowcount", errorno = "errorno")
    if (errorno != 0) return(SMS_INGRES_ERR);
    return ((rowcount == 0) ? SMS_NO_MATCH : SMS_SUCCESS);
##}

do_update(q, argv, qual, action, actarg)
    register struct query *q;
    char *argv[];
    char *qual;
    int (*action)();
    char *actarg;
##{
##  char *rvar;
##  char *rtable;
##  char *cqual;
##  int errorno;
      
    rvar = q->rvar;
    rtable = q->rtable;
##  range of rvar is rtable

    cqual = qual;
##  replace rvar (param (q->tlist, argv))
##  where cqual

##  inquire_equel (errorno = "errorno")
    if (errorno == INGRES_BAD_INT)
	return(SMS_INTEGER);
    else if (errorno != 0)
	return(SMS_INGRES_ERR);
    return(SMS_SUCCESS);
##}

do_append(q, argv, pqual, action, actarg)
    register struct query *q;
    char *argv[];
    char *pqual;
    int (*action)();
    char *actarg;
##{
##  char *rvar;
##  char *rtable;
##  char *cqual;
##  int errorno;

    rvar = q->rvar;
    rtable = q->rtable;
##  range of rvar is rtable

    if (pqual) {
	cqual = pqual;
##      append to rtable (param (q->tlist, argv)) where cqual
    } else {
##      append to rtable (param (q->tlist, argv))
    }

##  inquire_equel (errorno = "errorno")
    if (errorno == INGRES_BAD_INT)
	return(SMS_INTEGER);
    else if (errorno != 0)
	return(SMS_INGRES_ERR);
    return(SMS_SUCCESS);
##}

do_delete(q, qual, action, actarg)
    register struct query *q;
    char *qual;
    int (*action)();
    char *actarg;
##{
##  char *rvar;
##  char *rtable;
##  char *cqual;
##  int errorno;

    rvar = q->rvar;
    rtable = q->rtable;
##  range of rvar is rtable

    cqual = qual;
##  delete rvar where cqual

##  inquire_equel (errorno = "errorno")
    if (errorno != 0) return(SMS_INGRES_ERR);
    return(SMS_SUCCESS);
##}


/**
 ** set_next_object_id - set next object id in values table
 **
 ** Inputs: object - object name in values table and in objects
 **	    table - name of table objects are found in
 **
 ** - called before an APPEND operation to set the next object id to
 **   be used for the new record to the next free value
 **
 **/

set_next_object_id(object, table)
    char *object;
    char *table;
##{
##  char *name, *tbl;
##  int rowcount, exists, value;

    name = object;
    tbl = table;
##  repeat retrieve (value = values.#value) where values.#name = @name
##  inquire_equel(rowcount = "rowcount")
    if (rowcount != 1)
	return(SMS_NO_ID);

##  retrieve (exists = any(tbl.name where tbl.name = value))
##  inquire_equel(rowcount = "rowcount")
    if (rowcount != 1)
	return(SMS_NO_ID);
    while (exists) {
	value++;
	if (value > MAX_ID_VALUE)
	    value = MIN_ID_VALUE;
##	retrieve (exists = any(tbl.name where tbl.name = value))
    }

    if (LOG_RES)
        com_err(whoami, 0, "setting ID %s to %d", name, value);
##  repeat replace values (#value = @value) where values.#name = @name
    return(SMS_SUCCESS);
##}


/* Turn a kerberos name into the user's ID of the account that principal
 * owns.  Sets the kerberos ID and user ID.
 */

set_krb_mapping(name, login, ok, kid, uid)
char *name;
char *login;
int ok;
int *kid;
int *uid;
##{
##  int u_id, k_id, rowcount;
##  char *krbname;

    krbname = name;
    *kid = 0;
    *uid = 0;

##  repeat retrieve (u_id = krbmap.#users_id, k_id = krbmap.#string_id)
##	where krbmap.string_id = strings.string_id and strings.string = @krbname
##  inquire_equel (rowcount = "rowcount")
    
    if (rowcount == 1) {
	*kid = -k_id;
	*uid = u_id;
	return;
    }

    if (name_to_id(name, "STRING", &k_id) == SMS_SUCCESS)
      *kid = -k_id;

    if (!ok) {
	*uid = *kid;
	return;
    }

    if (name_to_id(login, "USER", uid) != SMS_SUCCESS)
      *uid = 0;

    if (*kid == 0)
      *kid = *uid;
##}


/* For now this just checks the argc's.  It should also see that there
 * are no duplicate names.
 */

sanity_check_queries()
{
    register int i;
    int maxv = 0, maxa = 0;
#ifdef MULTIPROTOCOLS
    extern int QueryCount1, QueryCount2;
    extern struct query Queries1[], Queries2[];
#else
    extern int QueryCount2;
    extern struct query Queries2[];
#endif MULTIPROTOCOLS

#define MAX(x,y) ((x) > (y) ? (x) : (y))

#ifdef MULTIPROTOCOLS
    for (i = 0; i < QueryCount1; i++) {
	maxv = MAX(maxv, Queries1[i].vcnt);
	maxa = MAX(maxa, Queries1[i].argc);
    }
#endif MULTIPROTOCOLS
    for (i = 0; i < QueryCount2; i++) {
	maxv = MAX(maxv, Queries2[i].vcnt);
	maxa = MAX(maxa, Queries2[i].argc);
    }
    if (MAX(maxv, maxa) > QMAXARGS) {
	com_err(whoami, 0, "A query has more args than QMAXARGS");
	exit(1);
    }
}
