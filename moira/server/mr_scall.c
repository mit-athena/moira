/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_scall.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_scall.c,v 1.5 1987-06-26 10:55:53 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
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
static char *rcsid_sms_scall_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_scall.c,v 1.5 1987-06-26 10:55:53 wesommer Exp $";
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

#ifdef notdef
char *procnames[] = {
	 "noop",
	 "auth",
	 "shutdown",
	 "query",
	 };
#endif notdef

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
#ifdef SMS_DBG
	fprintf(stderr, "[#%d] %s(", cl->id, procnames[pn]);
	for (i=0; i < cl->args->sms_argc; i++) {
		if (i) fputc(',', stderr);
		frequote(stderr,cl->args->sms_argv[i]);
	}
	fprintf(stderr, ")\n");
#endif SMS_DBG

	switch(pn) {
	case SMS_NOOP:
		cl->reply.sms_status = 0;
		com_err(whoami, 0, "noop");
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

retr_callback(argc, argv, p_cp)
	int argc;
	char **argv;
	char *p_cp;
{
	register client *cp = (client *)p_cp;
	/* XXX MEM when are these freed?? */
	/*
	 * This takes too much advantage of the fact that
	 * serialization of the data happens during the queue operation.
	 */
	sms_params *arg_tmp = (sms_params *)db_alloc(sizeof(sms_params));
	OPERATION op_tmp = create_operation();

#ifdef notdef			/* We really don't want this logged */
	com_err(whoami, 0, "Returning next data:");
	log_args(argc, argv);
#endif notdef
	
	arg_tmp->sms_status = SMS_MORE_DATA;
	arg_tmp->sms_argc = argc;
	arg_tmp->sms_argv = argv;
	arg_tmp->sms_flattened = (char *)NULL;
	arg_tmp->sms_argl = (int *)NULL;
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
#ifdef notdef
	if (!cl->clname) {
		com_err(whoami, 0, "Unauthenticated query rejected");
		cl->reply.sms_status = EACCES;
		return;
	}
#endif notdef
	com_err(whoami, 0, "Processing query: ");
	log_args(cl->args->sms_argc, cl->args->sms_argv);
	
	cl->reply.sms_status = 
	  sms_process_query(cl,
			    cl->args->sms_argv[0],
			    cl->args->sms_argc-1,
			    cl->args->sms_argv+1,
			    retr_callback,
			    (char *)cl);

	com_err(whoami, 0, "Query complete.");
}

do_access(cl)
	client *cl;
{
	cl->reply.sms_argc = 0;
	cl->reply.sms_status = 0;
#ifdef notdef
	if (!cl->clname) {
		com_err(whoami, 0, "Unauthenticated query rejected");
		cl->reply.sms_status = EACCES;
		return;
	}
#endif notdef
	com_err(whoami, 0, "Checking access: ");
	log_args(cl->args->sms_argc, cl->args->sms_argv);
	
	cl->reply.sms_status = 
	  sms_check_access(cl,
			   cl->args->sms_argv[0],
			   cl->args->sms_argc-1,
			   cl->args->sms_argv+1);
	
	com_err(whoami, 0, "Access check complete.");
}
