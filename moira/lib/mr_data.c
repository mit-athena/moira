/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_data.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_data.c,v 1.2 1987-06-01 03:31:36 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 */

#ifndef lint
static char *rcsid_sms_data_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_data.c,v 1.2 1987-06-01 03:31:36 wesommer Exp $";
#endif lint

#include "sms_private.h"

CONNECTION _sms_conn;

OPERATION _sms_send_op, _sms_recv_op;

