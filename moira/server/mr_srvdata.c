/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_srvdata.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_srvdata.c,v 1.3 1987-06-30 20:03:15 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 * 	Global variables inside the SMS server.
 * 
 *	$Log: not supported by cvs2svn $
 * Revision 1.2  87/06/21  16:42:16  wesommer
 * Performance work, rearrangement of include files.
 * 
 * Revision 1.1  87/06/02  20:07:25  wesommer
 * Initial revision
 * 
 */

#ifndef lint
static char *rcsid_sms_srvdata_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_srvdata.c,v 1.3 1987-06-30 20:03:15 wesommer Exp $";
#endif lint

#include "sms_server.h"

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
int nclients=0;
client **clients, *cur_client;
/*
 * Socket address of the most recently connected client.
 */
struct sockaddr_in client_addr;
int client_addrlen;
/*
 * Additional data sent at connect time by the client
 * (provided by GDB; ignored by SMS)
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
 * If non-null, reason for shutdown.  (SMS will be going down shortly
 * if this is non-null)
 */
char *takedown=NULL;

/*
 * The name of the local Kerberos realm
 */

char *krb_realm = NULL;
