/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_data.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_data.c,v 1.5 1988-09-13 15:52:14 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 */

#ifndef lint
static char *rcsid_sms_data_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_data.c,v 1.5 1988-09-13 15:52:14 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include "sms_private.h"

CONNECTION _sms_conn;

OPERATION _sms_send_op, _sms_recv_op;

int sending_version_no = SMS_VERSION_2;

