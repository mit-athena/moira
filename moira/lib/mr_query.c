/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_query.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_query.c,v 1.3 1987-08-02 21:49:53 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 * Revision 1.2  87/06/16  17:48:58  wesommer
 * Clean up memory allocation, indenting.
 * 
 * Revision 1.1  87/06/04  01:29:32  wesommer
 * Initial revision
 * 
 */

#ifndef lint
static char *rcsid_sms_query_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_query.c,v 1.3 1987-08-02 21:49:53 wesommer Exp $";
#endif lint

#include "sms_private.h"

/*
 * This routine is the primary external interface to the sms library.
 *
 * It builds a new argument vector with the query handle prepended,
 * and calls sms_query_internal.
 */
int level = 0;

int sms_query(name, argc, argv, callproc, callarg)
    char *name;		/* Query name */
    int argc;		/* Arg count */
    char **argv;		/* Args */
    int (*callproc)();	/* Callback procedure */
    char *callarg;		/* Callback argument */
{
    register char **nargv = (char **)malloc(sizeof(char *) * (argc+1));
    register int status = 0;
    nargv[0] = name;
    bcopy((char *)argv, (char *)(nargv+1), sizeof(char *) * argc);
    status = sms_query_internal(argc+1, nargv, callproc, callarg);
    free(nargv);
    return status;
}
/*
 * This routine makes an SMS query.
 *
 * argv[0] is the query name.
 * argv[1..argc-1] are the query arguments.
 *
 * callproc is called once for each returned value, with arguments
 * argc, argv, and callarg.
 * If it returns a non-zero value, further calls to it are not done, and
 * all future data from the server is ignored (there should be some
 * way to send it a quench..)
 */

int sms_query_internal(argc, argv, callproc, callarg)
    int argc;		/* Arg count */
    char **argv;		/* Args */
    int (*callproc)();	/* Callback procedure */
    char *callarg;		/* Callback argument */
{
    int status;
    sms_params params_st;
    register sms_params *params = NULL;
    sms_params *reply = NULL;
    int stopcallbacks = 0;

    if (level) return SMS_QUERY_NOT_REENTRANT;
    level++;
    
    CHECK_CONNECTED;

    params = &params_st;
    params->sms_procno = SMS_QUERY;
    params->sms_argc = argc;
    params->sms_argl = NULL;
    params->sms_argv = argv;
	
    if ((status = sms_do_call(params, &reply)))
	goto punt;

    while ((status = reply->sms_status) == SMS_MORE_DATA) {
	if (!stopcallbacks) 
	    stopcallbacks =
		(*callproc)(reply->sms_argc, reply->sms_argv, callarg);
	sms_destroy_reply(reply);
	reply = NULL;

	initialize_operation(_sms_recv_op, sms_start_recv, &reply,
			     (int (*)())NULL);
	queue_operation(_sms_conn, CON_INPUT, _sms_recv_op);

	complete_operation(_sms_recv_op);
	if (OP_STATUS(_sms_recv_op) != OP_COMPLETE) {
	    sms_disconnect();
	    status = SMS_ABORTED;
	    goto punt_1;
	}
    }	
punt:
    sms_destroy_reply(reply);
punt_1:
    level--;
    return status;
}
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
