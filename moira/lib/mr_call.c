/* $Id $
 *
 * Pass an mr_params off to the Moira server and get a reply
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include "mr_private.h"

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_call.c,v 1.12 1998-02-05 22:51:26 danw Exp $");

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
  complete_operation(_mr_send_op);
  complete_operation(_mr_recv_op);
  /* Look at results */
  if ((OP_STATUS(_mr_send_op) != OP_COMPLETE) ||
      (OP_STATUS(_mr_recv_op) != OP_COMPLETE))
    {
      mr_disconnect();
      return MR_ABORTED;
    }
  return 0;
}
