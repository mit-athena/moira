/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_call.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_call.c,v 1.5 1987-06-16 17:47:50 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 */

#ifndef lint
static char *rcsid_sms_call_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_call.c,v 1.5 1987-06-16 17:47:50 wesommer Exp $";
#endif lint

#include "sms_private.h"

sms_do_call(params, reply)
    struct sms_params *params;
    struct sms_params **reply;
{
    CHECK_CONNECTED;
    
    if (!_sms_send_op)
	_sms_send_op = create_operation();

    if (!_sms_recv_op)
	_sms_recv_op = create_operation();

    initialize_operation(_sms_send_op, sms_start_send, (char *)params,
			 (int (*)())NULL);
    queue_operation(_sms_conn, CON_OUTPUT, _sms_send_op);

    initialize_operation(_sms_recv_op, sms_start_recv, (char *)reply,
			 (int (*)())NULL);
    queue_operation(_sms_conn, CON_INPUT, _sms_recv_op);

    /* Block until operation done. */
    complete_operation(_sms_send_op);
    complete_operation(_sms_recv_op);
    /* Look at results */
    if ((OP_STATUS(_sms_send_op) != OP_COMPLETE) ||
	(OP_STATUS(_sms_recv_op) != OP_COMPLETE)) {
	sms_disconnect();
	return SMS_ABORTED;
    }
    return 0;
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
