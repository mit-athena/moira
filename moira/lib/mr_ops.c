/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_ops.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_ops.c,v 1.6 1989-09-06 17:43:52 mar Exp $
 *
 *	Copyright (C) 1987, 1989 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *	
 * 	This routine is part of the client library.  It handles
 *	the protocol operations: invoking an update and getting the
 *	SMS message of the day.
 */

#ifndef lint
static char *rcsid_sms_do_update_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_ops.c,v 1.6 1989-09-06 17:43:52 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include "sms_private.h"


/* Invoke a DCM update. */

int sms_do_update()
{
    int status;
    sms_params param_st;
    struct sms_params *params = NULL;
    struct sms_params *reply = NULL;

    CHECK_CONNECTED;
    params = &param_st;
    params->sms_version_no = sending_version_no;
    params->sms_procno = SMS_DO_UPDATE;
    params->sms_argc = 0;
    params->sms_argl = NULL;
    params->sms_argv = NULL;
	
    if ((status = sms_do_call(params, &reply)) == 0)
	status = reply->sms_status;
	
    sms_destroy_reply(reply);

    return status;
}


/* Get the SMS motd.  This returns an SMS status, and motd will either
 * point to NULL or the motd in a static buffer.
 */

int sms_motd(motd)
char **motd;
{
    int status;
    sms_params param_st;
    struct sms_params *params = NULL;
    struct sms_params *reply = NULL;
    static char buffer[1024];

    *motd = NULL;
    CHECK_CONNECTED;
    params = &param_st;
    params->sms_version_no = sending_version_no;
    params->sms_procno = SMS_MOTD;
    params->sms_argc = 0;
    params->sms_argl = NULL;
    params->sms_argv = NULL;
	
    if ((status = sms_do_call(params, &reply)))
      goto punt;

    while ((status = reply->sms_status) == SMS_MORE_DATA) {
	if (reply->sms_argc > 0) {
	    strncpy(buffer, reply->sms_argv[0], sizeof(buffer));
	    *motd = buffer;
	}
	sms_destroy_reply(reply);
	reply = NULL;

	initialize_operation(_sms_recv_op, sms_start_recv, &reply,
			     (int (*)())NULL);
	queue_operation(_sms_conn, CON_INPUT, _sms_recv_op);

	sms_complete_operation(_sms_recv_op);
	if (OP_STATUS(_sms_recv_op) != OP_COMPLETE) {
	    sms_disconnect();
	    status = SMS_ABORTED;
	    return(status);
	}
    }	
 punt:
    sms_destroy_reply(reply);
    /* for backwards compatability */
    if (status == SMS_UNKNOWN_PROC)
      return(0);
    else
      return(status);
}


/* Tell the library to take care of another input source while it is
 * processing a query.  For instance, an X toolkit application would
 * do something like
 *    sms_set_alternate_input(ConnectionNumber(XtDisplay(widget)), doxinput);
 * where doxinput is defined as:
 *    doxinput() {
 *	extern Widget toplevel;
 *	XEvent event;
 *	while (XPending(XtDisplay(toplevel))) {
 *	    XNextEvent(XtDisplay(toplevel), &event);
 *	    XtDispatchEvent(&event);
 *      }
 *      XFlush(XtDisplay(toplevel));
 *    }
 */

static int sms_alternate_input = 0;
static int (*sms_alternate_handler)();

int sms_set_alternate_input(fd, proc)
int fd;
int (*proc)();
{
    if (sms_alternate_input != 0)
      return(SMS_ALREADY_CONNECTED);
    sms_alternate_input = fd;
    sms_alternate_handler = proc;
    return(SMS_SUCCESS);
}


/* This is used by the parts of the library that must wait for GDB.  It
 * handles alternate input streams (such as X) as well.
 */

sms_complete_operation(op)
OPERATION op;
{
    long infd, outfd, exfd;
    int rc;
 
    gdb_progress();		/* try for an immediate completion */

    if (sms_alternate_input == 0)
      return(complete_operation(op));

    infd = (1<<sms_alternate_input);
    outfd = exfd = 0;

    while(op->status != OP_COMPLETE && op->status != OP_CANCELLED) {
	rc = con_select(sms_alternate_input, (fd_set *)&infd, (fd_set *)&outfd,
			  (fd_set *)&exfd, (struct timeval *)NULL);
	if (rc > 0 && sms_alternate_handler) {
	    (*sms_alternate_handler)();
	}
    }
    return(op->status);
}

