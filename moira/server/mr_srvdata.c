/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_srvdata.c,v $
 *	$Author: danw $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_srvdata.c,v 1.11 1998-01-07 17:13:40 danw Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 * 	Global variables inside the Moira server.
 */

#ifndef lint
static char *rcsid_mr_srvdata_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_srvdata.c,v 1.11 1998-01-07 17:13:40 danw Exp $";
#endif lint

#include <mit-copyright.h>
#include "mr_server.h"

/*
 * Connections & clients:
 *
 * The template and most recently connected connection:
 */
CONNECTION listencon, newconn;
/*
 * The template listen operation.
 */
OPERATION listenop;
/*
 * The list of operations corresponding to the I/O being done by the
 * currently connected clients.
 */
LIST_OF_OPERATIONS op_list;
/*
 * The current number of connected clients, an array of them, and the
 * "current" client, if any.
 */
int nclients = 0;
client **clients, *cur_client;
/*
 * Socket address of the most recently connected client.
 */
struct sockaddr_in client_addr;
int client_addrlen;
/*
 * Additional data sent at connect time by the client
 * (provided by GDB; ignored by Moira)
 */
TUPLE client_tuple;

/*
 * Name server was invoked as.
 */
char *whoami;
/*
 * Buffer for use in error messages.
 */
char buf1[BUFSIZ];

/*
 * If non-null, reason for shutdown.  (Moira will be going down shortly
 * if this is non-null)
 */
char *takedown = NULL;

/* States for putting the server to sleep & waking it up again. */
int dormant = AWAKE;

/*
 * The name of the local Kerberos realm
 */

char *krb_realm = NULL;
/*
 * Logging levels.
 */

int log_flags = LOG_CONNECT|LOG_REQUESTS|LOG_ARGS|LOG_RES;

/*
 * Time of last time through main loop.
 */
time_t now;


/*
 * Statistics on number of queries of each version that have been attempted
 */

int newqueries = 0;
int oldqueries = 0;


/* Journalling file */
FILE *journal = NULL;

