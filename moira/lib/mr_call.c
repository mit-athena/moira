/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_call.c,v $
 *	$Author: danw $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_call.c,v 1.11 1998-01-07 17:13:28 danw Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 */

#ifndef lint
static char *rcsid_mr_call_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_call.c,v 1.11 1998-01-07 17:13:28 danw Exp $";
#endif

#include <mit-copyright.h>
#include "mr_private.h"

int mr_do_call(struct mr_params *params, struct mr_params **reply)
{
  CHECK_CONNECTED;

  if (!_mr_send_op)
    _mr_send_op = create_operation();

  if (!_mr_recv_op)
    _mr_recv_op = create_operation();

  initialize_operation(_mr_send_op, mr_start_send, (char *)params, NULL);
  queue_operation(_mr_conn, CON_OUTPUT, _mr_send_op);

  initialize_operation(_mr_recv_op, mr_start_recv, (char *)reply, NULL);
  queue_operation(_mr_conn, CON_INPUT, _mr_recv_op);

  /* Block until operation done. */
  mr_complete_operation(_mr_send_op);
  mr_complete_operation(_mr_recv_op);
  /* Look at results */
  if ((OP_STATUS(_mr_send_op) != OP_COMPLETE) ||
      (OP_STATUS(_mr_recv_op) != OP_COMPLETE))
    {
      mr_disconnect();
      return MR_ABORTED;
    }
  return 0;
}
