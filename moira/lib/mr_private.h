/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_private.h,v $
 *	$Author: danw $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_private.h,v 1.8 1998-01-05 19:53:13 danw Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	Private declarations of the Moira library.
 */

#include "mr_proto.h"

extern CONNECTION _mr_conn;
extern OPERATION _mr_send_op, _mr_recv_op;

extern int mr_inited;
extern int sending_version_no;

/*
 * You are in a maze of twisty little FSM's, all different.
 */

#define S_RECV_START 1
#define S_RECV_DATA 2
#define S_DECODE_DATA 3

#define EVER (;;)

#define CHECK_CONNECTED {if (!_mr_conn) return MR_NOT_CONNECTED;}
