/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/moira.h,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/moira.h,v 1.4 1987-06-23 16:13:36 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
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
