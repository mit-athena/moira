/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_query.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_query.c,v 1.1 1987-06-04 01:29:32 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 */

#ifndef lint
static char *rcsid_sms_query_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_query.c,v 1.1 1987-06-04 01:29:32 wesommer Exp $";
#endif lint

#include "sms_private.h"

int sms_query(name, argc, argv, callproc, callarg)
	char *name;		/* Query name */
	int argc;		/* Arg count */
	char **argv;		/* Args */
	int (*callproc)();	/* Callback procedure */
	char *callarg;		/* Callback argument */
{
	int status;
	sms_params *params = NULL, *reply = NULL;
	
	if (!_sms_conn) {
		return SMS_NOT_CONNECTED;
	}
	
	params = (struct sms_params *) malloc(sizeof(*params));
	params->sms_procno = SMS_QUERY;
	params->sms_argc = 1 + argc;
	params->sms_argl = NULL;

	params->sms_argv = (char **)malloc(sizeof(char *) * params->sms_argc);

	params->sms_argv[0] = name;
	bcopy((char *)argv, (char *)(params->sms_argv + 1),
	      sizeof(char *) * argc);
	
	if ((status = sms_do_call(params, &reply)))
		goto punt;

	while ((status = reply->sms_status) == SMS_MORE_DATA) {
		(*callproc)(reply->sms_argc, reply->sms_argv, callarg);
		sms_destroy_reply(reply);
		reply = NULL;
		/*XXX error handling here sucks */
		initialize_operation(_sms_recv_op, sms_start_recv, &reply,
				     (int (*)())NULL);
		queue_operation(_sms_conn, CON_INPUT, _sms_recv_op);
		complete_operation(_sms_recv_op);
	}
	
punt:
	sms_destroy_reply(reply);
	if(params) {
		if(params->sms_argv)
			free(params->sms_argv);
		if(params->sms_argl)
			free(params->sms_argl);
		free(params);
	}
	return status;
}
