/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_server.h,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_server.h,v 1.5 1987-06-30 20:05:14 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 * Revision 1.4  87/06/21  16:42:07  wesommer
 * Performance work, rearrangement of include files.
 * 
 * Revision 1.3  87/06/03  16:07:59  wesommer
 * Fixes for lint.
 * 
 * Revision 1.2  87/06/01  03:35:10  wesommer
 * added stuff for authentication.
 * 
 */

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "sms_proto.h"
#include <krb.h>

typedef struct returned_tuples {
	struct returned_tuples *next;
	OPERATION op;
	sms_params *retval;
} returned_tuples;

/*
 * This should be in the kerberos header file.
 */

struct krbname {
	char name[ANAME_SZ];
	char inst[INST_SZ];
	char realm[REALM_SZ];
};

/*
 * This structure holds all per-client information; one of these is
 * allocated for each active client.
 */
   
typedef struct _client {
	OPERATION pending_op;	/* Primary pending operation */
	CONNECTION con;		/* Connection to the client */
	int state;		/* XXX this is really superfluous and should */
				/* be removed */
	int action;		/* what action is pending? */
	sms_params *args, reply;
	int id;			/* Unique id of client */
	struct sockaddr_in haddr; /* IP address of client */
	char *clname;		/* Name client authenticated to */
	struct krbname kname; 	/* Parsed version of the above */
	returned_tuples *first, *last;
} client;

/*
 * States
 */

#define CL_DEAD 0
#define CL_STARTING 1

/*
 * Actions.
 */

#define CL_ACCEPT 0
#define CL_RECEIVE 1
#define CL_SEND 2

extern char *krb_realm;
