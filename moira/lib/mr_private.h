/* $Id $
 *
 * Private declarations of the Moira library.
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
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

/* prototypes from mr_call.h */
int mr_do_call(struct mr_params *params, struct mr_params **reply);

/* prototypes from mr_init.c */
void mr_init(void);

/* prototypes from mr_ops.c */
int mr_complete_operation(OPERATION op);

/* prototypes from mr_params.c */
int mr_cont_send(OPERATION op, HALF_CONNECTION hcon, struct mr_params *arg);
int mr_cont_recv(OPERATION op, HALF_CONNECTION hcon, mr_params **argp);
