#include "query.h"
#include "sms_private.h"
#include "sms_server.h"

char *Argv[16];

static int ingres_errno = 0;

static ingerr(num)
	int *num;
{
	ingres_errno = SMS_INGRES_ERR; /* "Process lacks permission to */
				       /* alter device status.." */
	return *num;
}

sms_open_database()
{
	register int i;

	/* initialize local argv */
	for (i = 0; i < 16; i++)
		Argv[i] = (char *)malloc(128);

	IIseterr(ingerr);
	
	ingres_errno = 0;
	
	/* open the database */
##	ingres sms
	return ingres_errno;
}

sms_close_database()
{
##	exit
}

sms_process_query(name, argc, argv_ro, action, actarg)
	char *name;
	int argc;
	char *argv_ro[];
	int (*action)();
	char *actarg;
{
	register struct query *q;
	register int i;
	struct query *get_query_by_name();
	char qual[128];

	/* copy the arguments into a local argv that we can modify */
	for (i = 0; i < argc; i++)
		strcpy(Argv[i], argv_ro[i]);

	q = get_query_by_name(name);

	switch (q->type) {
	case RETRIEVE:
		if (q->qual) {
			build_qual(q->qual, q->argc, Argv, qual);
			do_retrieve_with_qual(q, qual, action);
		} else
			do_retrieve(q, action);
		break;

	case UPDATE:
		if (q->support_rtn) {
			if ((*q->support_rtn)(Argv, action) == -1) break;
		}
		build_qual(q->qual, q->argc, &Argv[q->sargc], qual);
		do_update(q, &Argv[q->sargc + q->argc], qual, action);
		break;

	case APPEND:
		if (q->support_rtn) {
			if ((*q->support_rtn)(Argv, action) == -1) break;
		}
		do_append(q, &Argv[q->sargc + q->argc], action);
		break;

	case DELETE:
		if (q->support_rtn) {
			if ((*q->support_rtn)(Argv, action) == -1) break;
		}
		build_qual(q->qual, q->argc, &Argv[q->sargc], qual);
		do_delete(q, qual, action);
		break;
	}
}

build_qual(fmt, argc, argv, qual)
	char *fmt;
	int argc;
	char *argv[];
	char *qual;
{
	switch (argc) {
	case 0:
		strcpy(qual, fmt);
		break;

	case 1:
		sprintf(qual, fmt, argv[0]);
		break;

	case 2:
		sprintf(qual, fmt, argv[0], argv[1]);
		break;

	case 3:
		sprintf(qual, fmt, argv[0], argv[1], argv[2]);
		break;

	case 4:
		sprintf(qual, fmt, argv[0], argv[1], argv[2], argv[3]);
		break;
	}
}

do_retrieve(q, action)
	register struct query *q;
	int (*action)();
## {
##	char *rvar;
##	char *rtable;
##	int rowcount;
	char status[32];
	char *sp = status;

	if (q->rvar) {
		rvar = q->rvar;
		rtable = q->rtable;
##		range of rvar is rtable
	}

##	retrieve (param (q->tlist, q->vaddr))
##    	{
	       if (q->support_rtn)
		       /* save result */
		       (*q->support_rtn)(q->vcnt, q->vaddr, 0, 0); 
	       else
		       (*action)(q->vcnt, q->vaddr, &q->vnames[q->argc]);
## 	}

## 	inquire_equel (rowcount = "rowcount")

	if (q->support_rtn) {
		/* process and send saved results */
		(*q->support_rtn)(0, 0, &q->vnames[q->argc], action);
	}

	sprintf(status, "(%d row%s)", rowcount, (rowcount == 1)? "" : "s");
	(*action)(1, &sp, 0);
## }

do_retrieve_with_qual(q, qual, action)
	register struct query *q;
	char *qual;
	int (*action)();
## {
##	char *rvar;
##	char *rtable;
##	char *cqual;
##	int rowcount;
	char status[32];
	char *sp = status;
      
	if (q->rvar) {
		rvar = q->rvar;
		rtable = q->rtable;
##		range of rvar is rtable
	}

	cqual = qual;
##	retrieve (param (q->tlist, q->vaddr))
##      where cqual
##    	{
		if (q->support_rtn)
			(*q->support_rtn)(q->vcnt, q->vaddr, 0, 0);
		else
			(*action)(q->vcnt, q->vaddr, &q->vnames[q->argc]);
##    	}

##    	inquire_equel (rowcount = "rowcount")

	if (q->support_rtn) {
		(*q->support_rtn)(0, 0, &q->vnames[q->argc], action);
	}

	sprintf(status, "(%d row%s)", rowcount, (rowcount == 1)? "" : "s");
	(*action)(1, &sp, 0);
## }

do_update(q, argv, qual, action)
	register struct query *q;
	char *argv[];
	char *qual;
	int (*action)();
## {
##    char *rvar;
##    char *rtable;
##    char *cqual;
##    int rowcount;
	char status[32];
	char *sp = status;
      
	rvar = q->rvar;
	rtable = q->rtable;
##	range of rvar is rtable

	cqual = qual;
##	replace rvar (param (q->tlist, argv))
##      where cqual

##    	inquire_equel (rowcount = "rowcount")
	sprintf(status, "(%d row%s)", rowcount, (rowcount == 1)? "" : "s");
	(*action)(1, &sp, 0);
## }

do_append(q, argv, action)
	register struct query *q;
	char *argv[];
	int (*action)();
## {
##    	char *rvar;
##	char *rtable;
##    	int rowcount;
	char status[32];
	char *sp = status;

	rvar = q->rvar;
	rtable = q->rtable;
##      range of rvar is rtable

##      append to rtable (param (q->tlist, argv))

##      inquire_equel (rowcount = "rowcount")
	sprintf(status, "(%d row%s)", rowcount, (rowcount == 1)? "" : "s");
	(*action)(1, &sp, 0);
## }

do_delete(q, qual, action)
	register struct query *q;
	char *qual;
	int (*action)();
## {
##      char *rvar;
##      char *rtable;
##      char *cqual;
##      int rowcount;
	char status[32];
	char *sp = status;

	rvar = q->rvar;
	rtable = q->rtable;
##      range of rvar is rtable

	cqual = qual;
##      delete rvar where cqual

##      inquire_equel (rowcount = "rowcount")
	sprintf(status, "(%d row%s)", rowcount, (rowcount == 1)? "" : "s");
	(*action)(1, &sp, 0);
## }

/* Support Queries */

support_alis(argv, action)
	char *argv[];
	int (*action)();
## {
##	static int list_id;

##	range of tbi is tbinfo

##	repeat retrieve (list_id = tbi.value1) where tbi.table = "list"
	list_id++;
##	repeat replace tbi (value1 = @list_id) where tbi.table = "list"

	argv[0] = (char *)&list_id;
## }

/**
 ** support_member():
 **
 ** support for ADD_MEMBER_TO_LIST
 ** support for DELETE_MEMBER_FROM_LIST
 ** support for UPDATE_MEMBER_STATUS
 **
 **           Input                     Output
 ** argv[0]   List Type       argv[4]   List ID
 ** argv[1]   List Name       argv[5]   Member ID
 ** argv[2]   Member Type     argv[6]   Member Type
 ** argv[3]   Member Name
 **/

support_member(argv, action)
	char *argv[];
	int (*action)();
## {
##    char *list_name;
##    char *list_type;
##    char *member_name;
##    char *member_type;
##    int list_id;
##    int value;
##    int rowcount;
      char errmsg[64];
      char *p_errmsg = errmsg;

      list_type = argv[0];
      list_name = argv[1];
      member_type = argv[2];
      member_name = argv[3];

##    range of l is list
##    repeat retrieve (list_id = l.id) 
##        where l.name = @list_name and l.type = @list_type
      sprintf(argv[4], "%d", list_id);

      if (!strcmp(member_type, "acl") || !strcmp(member_type, "group") ||
	  !strcmp(member_type, "mail")) {
##	  repeat retrieve (value = l.id)
##            where l.name = @member_name and l.type = @member_type
##	  inquire_equel (rowcount = "rowcount")
	  if (rowcount == 0) {
	      sprintf(errmsg, "(No such list: %s)", member_name);
	      (*action)(1, p_errmsg, 0);
	      return(-1);
	    }
	} else if (!strcmp(member_type, "user")) {
##        range of u is users
##        repeat retrieve (value = u.id) where u.login = @member_name
##	  inquire_equel (rowcount = "rowcount")
	  if (rowcount == 0) {
	      sprintf(errmsg, "(No such user: %s)", member_name);
	      (*action)(1, p_errmsg, 0);
	      return(-1);
	    }
	} else if (!strcmp(member_type, "string")) {
##	  range of s is strings
##	  repeat retrieve (value = s.id) where s.string = @member_name
##        inquire_equel (rowcount = "rowcount")
	  if (rowcount == 0) {
##            range of tbi is tbinfo
##            retrieve (value = tbi.value1) where tbi.table = "strings"
	      value++;
##            replace tbi (value1 = value) where tbi.table = "strings"
##	      append to strings (id = value, string = member_name)
	    }
	} else {
	  sprintf(errmsg, "(Unknown member type: %s)", member_type);
	  (*action)(1, p_errmsg, 0);
	  return(-1);
	}

      sprintf(argv[5], "%d", value);
      strcpy(argv[6], member_type);
## }

/**
 ** support for GET_LIST_MEMBERS
 **
 **           Input           Output
 ** argv[0]   Member Type     Member Type
 ** argv[1]   Member Id       Member Name (ACL, Group, Maillist, User, String)
 ** argv[2]   Member Status   Member Status
 **
 ** This routine performs two functions:
 **   When called with argc > 0, it copies and saves argv in a queue
 **   When called with argc = 0, it does post-processing on the saved
 **     data, and sends the data to the client using the supplied action
 **     routine.
 **/

support_gmol(argc, argv, vnames, action)
	int argc;
	char *argv[];
	char *vnames[];
	int (*action)();
## {
##      char *member_type;
##      int member_id;
##      char member_name[33];
	char **sargv;
	char *nargv[3];
	register int n;
	static struct save_queue *sq = (struct save_queue *)0;
	struct save_queue *sq_create();

	if (argc > 0) {
		if (sq == (struct save_queue *)0) {
			sq = sq_create();
		}
		sargv = (char **)malloc(3 * sizeof (char *));
		/* copy member_type */
		n = strlen(argv[0]) + 1;
		sargv[0] = (char *)malloc(n);
		bcopy(argv[0], sargv[0], n);
		/* copy member_id */
		sargv[1] = (char *)malloc(sizeof (int));
		*(int *)sargv[1] = *(int *)argv[1];
		/* copy member_status */
		n = strlen(argv[2]) + 1;
		sargv[2] = (char *)malloc(n);
		bcopy(argv[2], sargv[2], n);
		/* store data */
		sq_save_data(sq, sargv);
		return;
	}

	while (sq_get_data(sq, &sargv)) {
		member_type = sargv[0];
		member_id = *(int *)sargv[1];

		nargv[0] = member_type;
		nargv[1] = member_name;
		nargv[2] = sargv[2];

		if (!strcmp(member_type, "acl") ||
		    !strcmp(member_type, "group") ||
		    !strcmp(member_type, "mail")) {
##              	range of l is list
##            		repeat retrieve (member_name = l.name)
##			where l.id = @member_id
	        } else if (!strcmp(member_type, "user")) {
##	      		range of u is users
##	      		repeat retrieve (member_name = u.login) where u.id = @member_id
		} else if (!strcmp(member_type, "string")) {
##	      		range of s is strings
##	      		repeat retrieve (member_name = s.string)
##			where s.id = @member_id
		} else {
			sprintf(member_name, "%d", member_id);
		}

		(*action)(3, nargv, vnames);
		free(sargv[0]);
		free(sargv[1]);
		free(sargv[2]);
	}

	sq_destroy(sq);
	sq = (struct save_queue *)0;
## }
