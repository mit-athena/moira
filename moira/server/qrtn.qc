/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qrtn.qc,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qrtn.qc,v 1.30 1989-07-19 14:58:47 mar Exp $
 *
 *	Copyright (C) 1987, 1988 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 * 
 */

#ifndef lint
static char *rcsid_qrtn_qc = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qrtn.qc,v 1.30 1989-07-19 14:58:47 mar Exp $";
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

    /* initialize local argv */
    for (i = 0; i < 16; i++)
	Argv[i] = malloc(ARGLEN);

    IIseterr(ingerr);
	
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
##  char *table;
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
##	set lockmode session where readlock = exclusive
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
	    status = do_update(q, &Argv[q->argc], qual, action, actarg);
	    if (status != SMS_SUCCESS) break;
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
	    status = do_append(q, &Argv[q->argc], pqual, action, actarg);
	    if (status != SMS_SUCCESS) break;
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
	    status = do_delete(q, qual, action, actarg);
	    if (status != SMS_SUCCESS) break;
	    table = q->rtable;
##	    repeat replace tblstats (deletes = tblstats.deletes + 1,
##				     modtime = "now")
##		where tblstats.#table = @table
	}

	/* execute followup routine */
	if (v->post_rtn) status = (*v->post_rtn)(q, Argv, cl);
	break;

    }

out:
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
	} else {
	    if (ingres_errno != INGRES_DEADLOCK) {
##          	abort		/* it never happened */
	    }
	}
##      set lockmode session where readlock = system
    }

    if (status == SMS_SUCCESS && ingres_errno != 0) {
	critical_alert("SMS", "Server didn't notice INGRES ERROR %d",
		       ingres_errno);
	status = SMS_INTERNAL;
    }

    if (status != SMS_SUCCESS && log_flags & LOG_RES)
	com_err(whoami, status, " (Query failed)");
    /* until all the clients know about SMS_BUSY, convert this error
     * to one they will recognize.
     */
    if (status == SMS_BUSY)
      status = SMS_DEADLOCK;
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

    /* get query access control list */
    name = q->shortname;
##  repeat retrieve (acl_id = capacls.list_id) where capacls.tag = @name
##  inquire_equel (rowcount = "rowcount", errorno = "errorno")
    if (errorno != 0) return(SMS_INGRES_ERR);
    if (rowcount == 0) return(SMS_PERM);

    /* initialize default uid */
    if (def_uid == 0) {
##	retrieve (def_uid = users.users_id) where users.login = "default"
    }

    /* check for default access */
##  range of m is members
##  repeat retrieve (exists = any(m.#member_id where m.list_id = @acl_id and
##                   m.member_type = "USER" and m.#member_id = def_uid))
    if (exists) return(SMS_SUCCESS);

    /* parse client name */
    status = get_client(cl, &client_type, &client_id);
    if (status != SMS_SUCCESS) return(status);

    /* see if client is in the list (or any of its sub-lists) */
    exists = find_member("LIST", acl_id, client_type, client_id, 0);
    return ((exists) ? SMS_SUCCESS : SMS_PERM);
##}

get_client(cl, client_type, client_id)
    client *cl;
    char **client_type;
    int *client_id;
##{
    struct krbname *krb;
##  int member_id;
##  char *name;
##  int rowcount;

    if (cl->clname == NULL)
	return SMS_PERM;
    
    /* for now ignore instances */
    krb = &cl->kname;

    /* if client is from local realm, get users_id */
    if (!strcmp(krb->realm, krb_realm)) {
	*client_id = cl->users_id;
	*client_type = "USER";
	return(SMS_SUCCESS);
    }

    /* otherwise use string_id */
    name = cl->clname;
##  repeat retrieve (member_id = strings.string_id) 
##      where strings.string = @name
	
    /* make sure we found a users or string id */
##  inquire_equel (rowcount = "rowcount")
    if (rowcount == 0) return(SMS_PERM);

    *client_type = "STRING";
    *client_id = member_id;
    return(SMS_SUCCESS);
##}

##find_member(list_type, list_id, member_type, member_id, sq)
    char *list_type;
##  int list_id;
##  char *member_type;
##  int member_id;
    struct save_queue *sq;
##{
##  int exists;
##  int sublist;
    int child;
    struct save_queue *sq_create();

    if (!strcmp(strtrim(list_type), strtrim(member_type)) &&
	list_id == member_id)
	return(1);

    /* see if client is a direct member of list */
##  repeat retrieve (exists = any(m.#member_id where 
##				  m.#list_id = @list_id and
##				  m.#member_type = @member_type and 
##				  m.#member_id = @member_id))
    if (exists) return(1);

    /* are there any sub-lists? */
##  repeat retrieve (exists = any(m.#member_id where m.#list_id = @list_id and
##                   m.#member_type = "LIST"))
    if (!exists) return(0);

    /* yes; now recurse through sublists */

    /* create a save queue */
    if (sq == (struct save_queue *)0) {
	sq = sq_create();
	child = 0;
    } else {
	child = 1;
    }

    /* save all sublist ids */
##  range of m is members
##  retrieve (sublist = m.#member_id) 
##      where m.#list_id = list_id and m.#member_type = "LIST"
##  {
	 sq_save_unique_data(sq, sublist);
##  }

    if (child) return(0);

    /* at top-level, check sub-lists for client (breadth-first search) */
    while (sq_get_data(sq, &sublist)) {
	exists = find_member(list_type, sublist, member_type, member_id, sq);
	if (exists) {
	    sq_destroy(sq);
	    return(1);
	}
    }
    sq_destroy(sq);
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
##  range of v is values
##  repeat retrieve (value = v.#value) where v.#name = @name
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
##  repeat replace v (#value = @value) where v.#name = @name
    return(SMS_SUCCESS);
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
