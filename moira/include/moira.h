/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/moira.h,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/moira.h,v 1.5 1987-08-02 21:50:25 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
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

#include "sms_et.h"

#define SMS_VERSION_1 1

#define SMS_NOOP 0
#define SMS_AUTH 1
#define SMS_SHUTDOWN 2
#define SMS_QUERY 3
#define SMS_ACCESS 4
#define SMS_MAX_PROC 4

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
