/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_call.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_call.c,v 1.8 1990-03-17 16:36:51 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 */

#ifndef lint
static char *rcsid_sms_call_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_call.c,v 1.8 1990-03-17 16:36:51 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include "mr_private.h"

mr_do_call(params, reply)
    struct mr_params *params;
    struct mr_params **reply;
{
    CHECK_CONNECTED;
    
    if (!_mr_send_op)
	_mr_send_op = create_operation();

    if (!_mr_recv_op)
	_mr_recv_op = create_operation();

    initialize_operation(_mr_send_op, mr_start_send, (char *)params,
			 (int (*)())NULL);
    queue_operation(_mr_conn, CON_OUTPUT, _mr_send_op);

    initialize_operation(_mr_recv_op, mr_start_recv, (char *)reply,
			 (int (*)())NULL);
    queue_operation(_mr_conn, CON_INPUT, _mr_recv_op);

    /* Block until operation done. */
    mr_complete_operation(_mr_send_op);
    mr_complete_operation(_mr_recv_op);
    /* Look at results */
    if ((OP_STATUS(_mr_send_op) != OP_COMPLETE) ||
	(OP_STATUS(_mr_recv_op) != OP_COMPLETE)) {
	mr_disconnect();
	return MR_ABORTED;
    }
    return 0;
}
