/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_scall.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_scall.c,v 1.7 1987-07-14 00:39:01 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 * Revision 1.6  87/06/30  20:04:43  wesommer
 * Free returned tuples when possible.
 * 
 * Revision 1.5  87/06/26  10:55:53  wesommer
 * Added sms_access, now paiys attention to return code from 
 * sms_process_query, sms_check_access.
 * 
 * Revision 1.4  87/06/21  16:42:00  wesommer
 * Performance work, rearrangement of include files.
 * 
 * Revision 1.3  87/06/04  01:35:01  wesommer
 * Added a working query request handler.
 * 
 * Revision 1.2  87/06/03  16:07:50  wesommer
 * Fixes for lint.
 * 
 * Revision 1.1  87/06/02  20:07:10  wesommer
 * Initial revision
 * 
 */

#ifndef lint
static char *rcsid_sms_scall_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_scall.c,v 1.7 1987-07-14 00:39:01 wesommer Exp $";
#endif lint

#include <krb.h>
#include <errno.h>
#include "sms_server.h"
extern char buf1[];
extern int nclients;
extern char *whoami;

extern void clist_delete(), do_auth(), do_shutdown();
void do_call();

/*
 * Welcome to the (finite state) machine (highest level).
 */
void
do_client(cp)
	client *cp;
{
	free_rtn_tuples(cp);
	if (OP_STATUS(cp->pending_op) == OP_CANCELLED) {
		(void) sprintf(buf1, "Closed connection (now %d client%s)",
			       nclients-1,
			       nclients!=2?"s":"");
		com_err(whoami, 0, buf1);
		clist_delete(cp);
		return;
	}
	switch (cp->action) {
	case CL_ACCEPT:
	case CL_SEND:
		/* Start recieving next request */
		initialize_operation(cp->pending_op, sms_start_recv,
				     (char *)&cp->args, (int (*)())NULL);
		queue_operation(cp->con, CON_INPUT, cp->pending_op);
		cp->action = CL_RECEIVE;
		break;
	case CL_RECEIVE:
		/* Data is here. Process it & start it heading back */
		do_call(cp); /* This may block for a while. */
		initialize_operation(cp->pending_op, sms_start_send,
				     (char *)&cp->reply, (int (*)())NULL);
		queue_operation(cp->con, CON_OUTPUT, cp->pending_op);
		cp->action = CL_SEND;
		break;
	}
}		

char *procnames[] = {
	 "noop",
	 "auth",
	 "shutdown",
	 "query",
	 "access",
};


void
do_call(cl)
	client *cl;
{
	int pn;
	cl->reply.sms_argc = 0;
	cl->reply.sms_status = 0;
	if (((pn = cl->args->sms_procno) < 0) ||
	    (pn > SMS_MAX_PROC)) {
		com_err(whoami, 0, "procno out of range");
		cl->reply.sms_status = SMS_UNKNOWN_PROC;
		return;
	}
	if (log_flags & LOG_ARGS)
		log_args(procnames[pn], cl->args->sms_argc,
			 cl->args->sms_argv);
	else if (log_flags & LOG_REQUESTS)
		com_err(whoami, 0, procnames[pn]);

	switch(pn) {
	case SMS_NOOP:
		cl->reply.sms_status = 0;
		return;

	case SMS_AUTH:
		do_auth(cl);
		return;

	case SMS_QUERY:
		do_retr(cl);
		return;

	case SMS_ACCESS:
		do_access(cl);
		return;
		
	case SMS_SHUTDOWN:
		do_shutdown(cl);
		return;
	}
}

free_rtn_tuples(cp)
	client *cp;
{
	register returned_tuples *temp;
	for (temp=cp->first; temp && OP_DONE(temp->op); ) {
		register returned_tuples *t1=temp;
		temp = t1->next;
		if (t1 == cp->last) cp->last = NULL;
#ifdef notdef
		sms_destroy_reply(t1->retval);
#endif notdef
		if (t1->retval) {
			register sms_params *p = t1->retval;
			if (p->sms_flattened)
				free(p->sms_flattened);
			if (p->sms_argl)
				free(p->sms_argl);
			free(p);
		}
		delete_operation(t1->op);
		free(t1);
	}
	cp->first = temp;
}	

retr_callback(argc, argv, p_cp)
	int argc;
	char **argv;
	char *p_cp;
{
	register client *cp = (client *)p_cp;
	/*
	 * This takes too much advantage of the fact that
	 * serialization of the data happens during the queue operation.
	 */
	sms_params *arg_tmp = (sms_params *)db_alloc(sizeof(sms_params));
	returned_tuples *tp = (returned_tuples *)
		db_alloc(sizeof(returned_tuples));
	
	OPERATION op_tmp = create_operation();

	if (log_flags & LOG_RESP)
		log_args("return: ", argc, argv);

	tp->op = op_tmp;
	tp->retval = arg_tmp;
	tp->next = NULL;
	
	arg_tmp->sms_status = SMS_MORE_DATA;
	arg_tmp->sms_argc = argc;
	arg_tmp->sms_argv = argv;
	arg_tmp->sms_flattened = (char *)NULL;
	arg_tmp->sms_argl = (int *)NULL;

	if (cp->last) {
		cp->last->next = tp;
		cp->last = tp;
	} else {
		cp->last = cp->first = tp;
	}
	
	reset_operation(op_tmp);
	initialize_operation(op_tmp, sms_start_send, (char *)arg_tmp,
			     (int (*)())NULL);
	queue_operation(cp->con, CON_OUTPUT, op_tmp);
}


do_retr(cl)
	client *cl;
{
	cl->reply.sms_argc = 0;
	cl->reply.sms_status = 0;

	cl->reply.sms_status = 
	  sms_process_query(cl,
			    cl->args->sms_argv[0],
			    cl->args->sms_argc-1,
			    cl->args->sms_argv+1,
			    retr_callback,
			    (char *)cl);
	if (log_flags & LOG_RES)
		com_err(whoami, 0, "Query complete.");
}

do_access(cl)
	client *cl;
{
	cl->reply.sms_argc = 0;

	cl->reply.sms_status = 
	  sms_check_access(cl,
			   cl->args->sms_argv[0],
			   cl->args->sms_argc-1,
			   cl->args->sms_argv+1);
	
	com_err(whoami, 0, "Access check complete.");
}
