/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_ops.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_ops.c,v 1.8 1991-03-08 10:32:17 mar Exp $
 *
 *	Copyright (C) 1987, 1989, 1990 by the Massachusetts Institute of
 *	Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *	
 * 	This routine is part of the client library.  It handles
 *	the protocol operations: invoking an update and getting the
 *	MR message of the day.
 */

#ifndef lint
static char *rcsid_sms_do_update_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_ops.c,v 1.8 1991-03-08 10:32:17 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include <strings.h>
#include "mr_private.h"


/* Invoke a DCM update. */

int mr_do_update()
{
    int status;
    mr_params param_st;
    struct mr_params *params = NULL;
    struct mr_params *reply = NULL;

    CHECK_CONNECTED;
    params = &param_st;
    params->mr_version_no = sending_version_no;
    params->mr_procno = MR_DO_UPDATE;
    params->mr_argc = 0;
    params->mr_argl = NULL;
    params->mr_argv = NULL;
	
    if ((status = mr_do_call(params, &reply)) == 0)
	status = reply->mr_status;
	
    mr_destroy_reply(reply);

    return status;
}


/* Get the MR motd.  This returns an MR status, and motd will either
 * point to NULL or the motd in a static buffer.
 */

int mr_motd(motd)
char **motd;
{
    int status;
    mr_params param_st;
    struct mr_params *params = NULL;
    struct mr_params *reply = NULL;
    static char buffer[1024];

    *motd = NULL;
    CHECK_CONNECTED;
    params = &param_st;
    params->mr_version_no = sending_version_no;
    params->mr_procno = MR_MOTD;
    params->mr_argc = 0;
    params->mr_argl = NULL;
    params->mr_argv = NULL;
	
    if ((status = mr_do_call(params, &reply)))
      goto punt;

    while ((status = reply->mr_status) == MR_MORE_DATA) {
	if (reply->mr_argc > 0) {
	    strncpy(buffer, reply->mr_argv[0], sizeof(buffer));
	    *motd = buffer;
	}
	mr_destroy_reply(reply);
	reply = NULL;

	initialize_operation(_mr_recv_op, mr_start_recv, &reply,
			     (int (*)())NULL);
	queue_operation(_mr_conn, CON_INPUT, _mr_recv_op);

	mr_complete_operation(_mr_recv_op);
	if (OP_STATUS(_mr_recv_op) != OP_COMPLETE) {
	    mr_disconnect();
	    status = MR_ABORTED;
	    return(status);
	}
    }	
 punt:
    mr_destroy_reply(reply);
    /* for backwards compatability */
    if (status == MR_UNKNOWN_PROC)
      return(0);
    else
      return(status);
}


/* Tell the library to take care of another input source while it is
 * processing a query.  For instance, an X toolkit application would
 * do something like
 *    mr_set_alternate_input(ConnectionNumber(XtDisplay(widget)), doxinput);
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

static int mr_alternate_input = 0;
static int (*mr_alternate_handler)();

int mr_set_alternate_input(fd, proc)
int fd;
int (*proc)();
{
    if (mr_alternate_input != 0)
      return(MR_ALREADY_CONNECTED);
    mr_alternate_input = fd;
    mr_alternate_handler = proc;
    return(MR_SUCCESS);
}


/* This is used by the parts of the library that must wait for GDB.  It
 * handles alternate input streams (such as X) as well.
 */

mr_complete_operation(op)
OPERATION op;
{
    long infd, outfd, exfd;
    int rc;
 
    gdb_progress();		/* try for an immediate completion */

    if (mr_alternate_input == 0)
      return(complete_operation(op));

    infd = (1<<mr_alternate_input);
    outfd = exfd = 0;

    while(op->status != OP_COMPLETE && op->status != OP_CANCELLED) {
	rc = con_select(mr_alternate_input, (fd_set *)&infd, (fd_set *)&outfd,
			  (fd_set *)&exfd, (struct timeval *)NULL);
	if (rc > 0 && mr_alternate_handler) {
	    (*mr_alternate_handler)();
	}
    }
    return(op->status);
}

