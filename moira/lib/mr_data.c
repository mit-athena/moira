/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_data.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_data.c,v 1.6 1990-03-17 16:36:56 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 */

#ifndef lint
static char *rcsid_sms_data_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_data.c,v 1.6 1990-03-17 16:36:56 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include "mr_private.h"

CONNECTION _mr_conn;

OPERATION _mr_send_op, _mr_recv_op;

int sending_version_no = MR_VERSION_2;

