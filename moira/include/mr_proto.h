/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/mr_proto.h,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/mr_proto.h,v 1.3 1989-10-06 19:09:59 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 * Revision 1.2  87/06/28  05:28:18  spook
 * Added domain name to 'sms'...
 * 
 * Revision 1.1  87/06/16  17:48:47  wesommer
 * Initial revision
 * 
 */

#include <stdio.h>
#include <gdb.h>
#include <sms.h>

typedef struct sms_params {
	u_long sms_size;
	u_long sms_version_no;
	union {
		u_long procno;	/* for call */
		u_long status;	/* for reply */
	} u;
#define sms_procno u.procno
#define sms_status u.status
	int sms_argc;
	char **sms_argv;
	int *sms_argl;
	char *sms_flattened;
	int sms_state;
} sms_params;

extern int sms_start_recv(), sms_start_send();
