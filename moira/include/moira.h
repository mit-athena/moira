/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/moira.h,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/moira.h,v 1.9 1988-06-14 14:10:40 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 */

#ifndef _sms_h_
#define _sms_h_

/* return values from queries (and error codes) */

#include "sms_et.h"
#define SMS_SUCCESS 0		/* Query was successful */

#define SMS_VERSION_1 1		/* Version in use from 7/87 to 4/88 */
#define SMS_VERSION_2 2		/* After 4/88, new query lib */

/* return values for sms server calls, used by clients */

#define SMS_CONT 0		/* Everything okay, continue sending values. */
#define SMS_ABORT -1		/* Something went wrong don't send anymore
				   values. */

/* Protocol operations */
#define SMS_NOOP 0
#define SMS_AUTH 1
#define SMS_SHUTDOWN 2
#define SMS_QUERY 3
#define SMS_ACCESS 4
#define SMS_DO_UPDATE 5
#define SMS_MAX_PROC 5

/* values used in NFS physical flags */
#define SMS_FS_STUDENT	0x0001
#define	SMS_FS_FACULTY  0x0002
#define SMS_FS_STAFF	0x0004
#define SMS_FS_MISC	0x0008

/* magic values to pass for list and user queries */
#define UNIQUE_GID	"create unique GID"
#define UNIQUE_UID	"create unique UID"
#define UNIQUE_LOGIN	"create unique login ID"


#ifdef __STDC__
int sms_connect();
int sms_disconnect();
int sms_auth();
int sms_access(char *handle, int argc, char **argv);
int sms_access_internal(int argc, char **argv);
int sms_query(char *handle, int argc, char **argv,
	      int (*callback)(), char *callarg);
int sms_query_internal(int argc, char **argv,
		       int (*callback)(), char *callarg);
int sms_noop();
int sms_shutdown(char *reason);
#else !__STDC__
int sms_connect();
int sms_disconnect();
int sms_auth();
int sms_access();
int sms_query();
int sms_noop();
int sms_shutdown();
#endif __STDC__

#endif _sms_h_			/* Do Not Add Anything after this line. */
