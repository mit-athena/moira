/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/mr_proto.h,v $
 *	$Author: danw $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/mr_proto.h,v 1.6 1997-01-29 23:21:23 danw Exp $
 *
 *	Copyright (C) 1987, 1990 by the Massachusetts Institute of Technology
 *
 */

#include <stdio.h>
#include <gdb.h>
#include <moira.h>

typedef struct mr_params {
	uint32 mr_size;
	uint32 mr_version_no;
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
