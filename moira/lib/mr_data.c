/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_data.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_data.c,v 1.3 1988-04-19 12:42:10 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 * Revision 1.2  87/06/01  03:31:36  wesommer
 * Added RCS headers.
 * 
 */

#ifndef lint
static char *rcsid_sms_data_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_data.c,v 1.3 1988-04-19 12:42:10 mar Exp $";
#endif lint

#include "sms_private.h"

CONNECTION _sms_conn;

OPERATION _sms_send_op, _sms_recv_op;

int sending_version_no = SMS_VERSION_1;

