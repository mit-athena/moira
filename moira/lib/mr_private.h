/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_private.h,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_private.h,v 1.2 1987-05-31 22:07:06 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	Private declarations of the SMS library.
 * 
 *	$Log: not supported by cvs2svn $
 * Revision 1.1  87/05/20  03:12:00  wesommer
 * Initial revision
 * 
 */

#include <stdio.h>
#include <gdb.h>
#include <sms.h>

typedef struct sms_params {
	int sms_size;
	int sms_version_no;
	union {
		int procno;	/* for call */
		int status;	/* for reply */
	} u;
#define sms_procno u.procno
#define sms_status u.status
	int sms_argc;
	char **sms_argv;
	int *sms_argl;
	char *sms_flattened;
	int sms_state;
} sms_params;

extern CONNECTION _sms_conn;

extern OPERATION _sms_send_op, _sms_recv_op;
extern int sms_start_recv(), sms_start_send();
extern int sms_inited;

#define SMS_GDB_SERV "sms:#8973"

/*
 * You are in a maze of twisty little FSM's, all different.
 */

#define S_RECV_START 1
#define S_RECV_DATA 2
#define S_DECODE_DATA 3

#define EVER (;;)
	
