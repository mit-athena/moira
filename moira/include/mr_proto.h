/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/mr_proto.h,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/mr_proto.h,v 1.4 1990-03-17 15:57:31 mar Exp $
 *
 *	Copyright (C) 1987, 1990 by the Massachusetts Institute of Technology
 *
 */

#include <stdio.h>
#include <gdb.h>
#include <sms.h>

typedef struct mr_params {
	u_long mr_size;
	u_long mr_version_no;
	union {
		u_long procno;	/* for call */
		u_long status;	/* for reply */
	} u;
#define mr_procno u.procno
#define mr_status u.status
	int mr_argc;
	char **mr_argv;
	int *mr_argl;
	char *mr_flattened;
	int mr_state;
} mr_params;

extern int mr_start_recv(), mr_start_send();
