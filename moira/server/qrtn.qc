/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qrtn.qc,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qrtn.qc,v 1.14 1988-01-14 13:37:57 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 * Revision 1.14  88/01/14  13:14:37  mar
 * 1. separate access checking from setup routines
 * 2. check max argument length
 * 
 * Revision 1.13  88/01/04  12:02:13  mar
 * moved transaction start before pre-routines (wesommer)
 * 
 * Revision 1.13  87/11/12  18:13:12  wesommer
 * Move transaction boundary to include the pre routine.
 * 
 * Revision 1.12  87/09/12  20:42:11  wesommer
 * Clean up after Gretzinger: cl->kname is not valid unless cl->clname is
 * non-NULL.
 * 
 * Revision 1.12  87/09/12  20:06:46  wesommer
 * Fix security hole/null dereference bug: if clname is NULL, return
 * permission denied in get_client.
 * 
 * Revision 1.11  87/09/01  16:10:01  wesommer
 * This change was made by Mike, who didn't feel like checking it in.
 * Temp hack: ignore instances.
 * 
 * Revision 1.10  87/08/28  14:57:51  mike
 * Modified sms_query to not enclose RETRIEVE queries in begin/end transaction.
 * This was necessary to allow get_all_poboxes and get_groups_of_all_users
 * to temporarily change the Ingres lockmode.
 * 
 * Revision 1.9  87/08/22  17:47:38  wesommer
 * Cleanup (these changes were by Mike).
 * 
 * Revision 1.8  87/08/10  16:22:26  mike
 * wesommer modified error reporting.
 * 
 * Revision 1.7  87/08/04  01:49:20  wesommer
 * Rearranged messages.
 * 
 * Revision 1.6  87/08/04  01:30:54  wesommer
 * Mike's changes; checked in prior to working over messages.
 * 
 * Revision 1.5  87/06/21  16:37:58  wesommer
 * Changed include files, reindented things.
 * 
 * 
 * Revision 1.4  87/06/08  05:03:27  wesommer
 * Reindented; added header and trailer.
 * 
 */

#ifndef lint
static char *rcsid_qrtn_qc = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qrtn.qc,v 1.14 1988-01-14 13:37:57 mar Exp $";
#endif lint

#include "query.h"
#include "sms_server.h"

#define SMS_SUCCESS 0

char *Argv[16];

static int ingres_errno = 0;
extern char *whoami;

/*
 * ingerr: (supposedly) called when Ingres indicates an error.
 * I have not yet been able to get this to work to intercept a
 * database open error.
 */

static int ingerr(num)
    int *num;
{
    ingres_errno = SMS_INGRES_ERR;
    com_err(whoami, SMS_INGRES_ERR, " code %d\n", ingres_errno);
    return *num;
}

int sms_open_database()
{
    register int i;

    /* initialize local argv */
    for (i = 0; i < 16; i++)
	Argv[i] = (char *)malloc(ARGLEN);

    IIseterr(ingerr);
	
    ingres_errno = 0;
	
    /* open the database */
##  ingres sms
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

    q = get_query_by_name(name);
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

    /* list queries command */
    if (!strcmp(name, "_list_queries")) {
	list_queries(action, actarg);
	return(SMS_SUCCESS);
    }

    /* help query command */
    if (!strcmp(name, "_help")) {
	q = get_query_by_name(argv_ro[0]);
	if (q == (struct query *)0) return(SMS_NO_HANDLE);
	help_query(q, action, actarg);
	return(SMS_SUCCESS);
    }

    /* get query structure, return error if named query does not exist */
    q = get_query_by_name(name);
    if (q == (struct query *)0) return(SMS_NO_HANDLE);
    v = q->validate;

    if (q->type != RETRIEVE)
##      begin transaction

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
		status = (*v->post_rtn)(q, sq, v, action, actarg);
	    } else {
		/* normal retrieve */
		status = do_retrieve(q, pqual, psort, action, actarg);
	    }
	    if (status != SMS_SUCCESS) break;
	    table = q->rtable;
##          repeat replace tblstats (retrieves = tblstats.retrieves + 1)
##                 where tblstats.#table = @table
	} else {
	    status = (*v->post_rtn)(q, Argv, action, actarg);
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
##	    repeat replace tblstats (updates = tblstats.updates + 1,
##				     modtime = "now")
##		where tblstats.#table = @table
	}

	/* execute followup routine (if any) */
	if (v->post_rtn) status = (*v->post_rtn)(q, Argv);

	break;

    case APPEND:
	/* see if row already exists */
	if (v->field) {
	    status = validate_row(q, Argv, v);
	    if (status != SMS_NO_MATCH) break;
	}

	/* increment id number if necessary */
	if (v->object_id) {
	    status = set_next_object_id(v->object_id);
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
	if (v->post_rtn) status = (*v->post_rtn)(q, Argv);
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
	if (v->post_rtn) status = (*v->post_rtn)(q, Argv);
	break;

    }

out:
    if (q->type != RETRIEVE) {
        if (status == SMS_SUCCESS) {
##          end transaction	/* commit to this */
	} else {
##          abort		/* it never happened */
	}
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

    c = fmt;
    for (i = 0; i < argc; i++) {
	c = (char *)index(c, '%');
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

    /* copy the arguments into a local argv that we can modify */
    for (i = 0; i < argc; i++) {
	if (strlen(argv_ro[i]) < ARGLEN)
	    strcpy(Argv[i], argv_ro[i]);
	else
	    return(SMS_ARG_TOO_LONG);
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
    exists = find_member(acl_id, client_type, client_id, 0);
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
	name = krb->name;
##	repeat retrieve (member_id = users.users_id) where users.login = @name
	*client_type = "USER";
    } else {
	/* otherwise use string_id */
	name = cl->clname;
##	repeat retrieve (member_id = strings.string_id) 
##          where strings.string = @name
	*client_type = "STRING";
    }
	
    /* make sure we found a users or string id */
##  inquire_equel (rowcount = "rowcount")
    if (rowcount == 0) return(SMS_PERM);

    *client_id = member_id;
    return(SMS_SUCCESS);
##}

##find_member(list_id, member_type, member_id, sq)
##  int list_id;
##  char *member_type;
##  int member_id;
    struct save_queue *sq;
##{
##  int exists;
##  int sublist;
    int child;
    struct save_queue *sq_create();

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

    if (child) return;

    /* at top-level, check sub-lists for client (breadth-first search) */
    while (sq_get_data(sq, &sublist)) {
	exists = find_member(sublist, member_type, member_id, sq);
	if (exists) {
	    sq_destroy(sq);
	    return(1);
	}
    }
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

    if (q->rvar) {
	rvar = q->rvar;
	rtable = q->rtable;
##     	range of rvar is rtable
    }

    if (psort) {
	csort = psort;
	if (pqual) {
	    cqual = pqual;
##          retrieve unique (param (q->tlist, q->vaddr)) where cqual
##                   sort by csort
##          {
		 (*action)(q->vcnt, q->vaddr, actarg);
##          }
	} else {
##          retrieve unique (param (q->tlist, q->vaddr))
##                   sort by csort
##          {
		 (*action)(q->vcnt, q->vaddr, actarg);
##          }
	}

    } else {
	if (pqual) {
	    cqual = pqual;
##          retrieve unique (param (q->tlist, q->vaddr)) where cqual
##          {
		 (*action)(q->vcnt, q->vaddr, actarg);
##          }
	} else {
##          retrieve unique (param (q->tlist, q->vaddr))
##          {
		 (*action)(q->vcnt, q->vaddr, actarg);
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
##  int rowcount;
##  int errorno;
      
    rvar = q->rvar;
    rtable = q->rtable;
##  range of rvar is rtable

    cqual = qual;
##  replace rvar (param (q->tlist, argv))
##  where cqual

##  inquire_equel (errorno = "errorno")
    if (errorno != 0) return(SMS_INGRES_ERR);
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
    if (errorno != 0) return(SMS_INGRES_ERR);
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


/*
 * Local Variables:
 * mode: c
 * c-indent-level: 4
 * c-continued-statement-offset: 4
 * c-brace-offset: -4
 * c-argdecl-indent: 4
 * c-label-offset: -4
 * End:
 */

