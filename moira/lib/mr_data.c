/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_data.c,v $
 *	$Author: danw $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_data.c,v 1.8 1998-01-07 17:13:30 danw Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 */

#ifndef lint
static char *rcsid_mr_data_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_data.c,v 1.8 1998-01-07 17:13:30 danw Exp $";
#endif

#include <mit-copyright.h>
#include "mr_private.h"

CONNECTION _mr_conn;

OPERATION _mr_send_op, _mr_recv_op;

int sending_version_no = MR_VERSION_2;

