/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_private.h,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_private.h,v 1.3 1987-06-16 17:48:39 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	Private declarations of the SMS library.
 * 
 *	$Log: not supported by cvs2svn $
 * Revision 1.2  87/05/31  22:07:06  wesommer
 * Private data to the sms server and library.
 * 
 * Revision 1.1  87/05/20  03:12:00  wesommer
 * Initial revision
 * 
 */

#include "sms_proto.h"

extern CONNECTION _sms_conn;
extern OPERATION _sms_send_op, _sms_recv_op;

extern int sms_inited;

/*
 * You are in a maze of twisty little FSM's, all different.
 */

#define S_RECV_START 1
#define S_RECV_DATA 2
#define S_DECODE_DATA 3

#define EVER (;;)
	
#define CHECK_CONNECTED {if (!_sms_conn) return SMS_NOT_CONNECTED;}

/*
 * There should be an include file for these..
 */
	
extern char *malloc();
#ifndef htonl
extern u_long htonl(), ntohl();
extern u_short htons(), ntohs();
#endif htonl



