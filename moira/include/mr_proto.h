/* $Id: mr_proto.h,v 1.7 1998-02-05 22:51:17 danw Exp $
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
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

int mr_start_send(OPERATION op, HALF_CONNECTION hcon, struct mr_params *arg);
int mr_start_recv(OPERATION op, HALF_CONNECTION hcon, struct mr_params **argp);
void mr_destroy_reply(mr_params *reply);
