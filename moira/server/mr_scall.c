/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_scall.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_scall.c,v 1.2 1987-06-03 16:07:50 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 * Revision 1.1  87/06/02  20:07:10  wesommer
 * Initial revision
 * 
 */

#ifndef lint
static char *rcsid_sms_scall_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_scall.c,v 1.2 1987-06-03 16:07:50 wesommer Exp $";
#endif lint

#include <krb.h>
#include "sms_private.h"
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
	 "retrieve",
	 "append",
	 "delete",
	 "edit",
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
#ifdef notdef
	case SMS_RETRIEVE:
		do_retr(cl);
		return;
#endif notdef

	case SMS_SHUTDOWN:
		do_shutdown(cl);
		return;
	}
}

#ifdef notdef
do_retr(cl)
	client *cl;
{
	
}
#endif notdef
