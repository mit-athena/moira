/* $Id: mr_data.c,v 1.10 1998-02-08 19:31:20 danw Exp $
 *
 * A few variables
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include "mr_private.h"

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_data.c,v 1.10 1998-02-08 19:31:20 danw Exp $");

CONNECTION _mr_conn;

OPERATION _mr_send_op, _mr_recv_op;

int sending_version_no = MR_VERSION_2;

