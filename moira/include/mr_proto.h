/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/mr_proto.h,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/mr_proto.h,v 1.1 1987-06-16 17:48:47 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
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

#define SMS_GDB_SERV "sms:sms_db"

/*
 * Local Variables:
 * mode: c
 * c-indent-level: 4
 * c-continued-statement-offset: 4
 * c-brace-offset: -4
 * c-argdecl-indent: 4
 * c-label-offset: -4
 * End:
 */
