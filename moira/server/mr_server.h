/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_server.h,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_server.h,v 1.19 1989-12-28 17:03:37 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 */

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "sms_proto.h"
#include <sms_app.h>
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
	int action;		/* what action is pending? */
	sms_params *args, reply;
	int id;			/* Unique id of client */
	struct sockaddr_in haddr; /* IP address of client */
	char clname[MAX_K_NAME_SZ];/* Name client authenticated to */
	struct krbname kname; 	/* Parsed version of the above */
	int users_id;		/* SMS internal ID of authenticated user */
	int client_id;		/* SMS internal ID of client for modby field */
	returned_tuples *first, *last;
	time_t last_time_used;  /* Last time connection used */
	char entity[9];		/* entity on other end of the connection */
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

/*
 * Debugging options.
 */

extern int log_flags;

#define LOG_CONNECT		0x0001
#define LOG_REQUESTS		0x0002
#define LOG_ARGS		0x0004
#define LOG_RESP		0x0008
#define LOG_RES			0x0010
#define LOG_VALID		0x0020


/* max length of query argument allowed */
#define ARGLEN	257

/* statistics on number of queries by version number */
extern int newqueries, oldqueries;

/* Maximum and minimum values that will ever be chosen for IDs */
#define MAX_ID_VALUE	32765
#define MIN_ID_VALUE	100

/* Sleepy states for the server! */
#define AWAKE 0
#define SLEEPY 1
#define ASLEEP 2
#define GROGGY 3
extern int dormant;

/* state for the incremental update system */
extern int inc_running, inc_pid;
extern time_t inc_started, now;
#define INC_TIMEOUT (3 * 60)	/* 3 minutes */
