/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_call.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_call.c,v 1.7 1989-09-06 17:19:55 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 */

#ifndef lint
static char *rcsid_sms_call_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_call.c,v 1.7 1989-09-06 17:19:55 mar Exp $";
#endif lint

#include <mit-copyright.h>
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
    sms_complete_operation(_sms_send_op);
    sms_complete_operation(_sms_recv_op);
    /* Look at results */
    if ((OP_STATUS(_sms_send_op) != OP_COMPLETE) ||
	(OP_STATUS(_sms_recv_op) != OP_COMPLETE)) {
	sms_disconnect();
	return SMS_ABORTED;
    }
    return 0;
}
