/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/moira.h,v $
 *	$Author: kit $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/moira.h,v 1.8 1988-04-19 14:40:17 kit Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 * Revision 1.7  88/04/19  14:30:47  kit
 * Added new sms_version_2 for new version.
 * 
 * Revision 1.6  87/08/22  17:11:59  wesommer
 * Added flags for user filesystems.
 * 
 * Revision 1.5  87/08/02  21:50:25  wesommer
 * Added function prototypes.
 * 
 * Revision 1.4  87/06/23  16:13:36  wesommer
 * Added new request.
 * 
 * Revision 1.3  87/06/04  01:31:16  wesommer
 * Renamed retrieve to query.
 * 
 * Revision 1.2  87/06/01  03:33:32  wesommer
 * Added new procedure numbers.
 * 
 */

#ifndef _sms_h_
#define _sms_h_

#include "sms_et.h"

#define SMS_VERSION_1 1		/* Version in use from 7/87 to 4/88 */
#define SMS_VERSION_2 2		/* After 4/88, new query lib */

/* return values for sms server calls, used by clients */

#define SMS_CONT 0		/* Everything okay, continue sending values. */
#define SMS_ABORT -1		/* Something went wrong don't send anymore
				   values. */


#define SMS_NOOP 0
#define SMS_AUTH 1
#define SMS_SHUTDOWN 2
#define SMS_QUERY 3
#define SMS_ACCESS 4
#define SMS_DO_UPDATE 5
#define SMS_MAX_PROC 5

#define SMS_FS_STUDENT	0x0001
#define	SMS_FS_FACULTY  0x0002
#define SMS_FS_STAFF	0x0004
#define SMS_FS_MISC	0x0008

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
