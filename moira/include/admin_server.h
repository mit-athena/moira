/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/admin_server.h,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/admin_server.h,v 1.3 1987-08-22 17:14:58 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 * 	Defines for talking to the kerberos admin_server, for changing
 * 	passwords, etc.
 *
 * 	Originally by Jeffery I. Schiller, January 1987.
 * 
 *	$Log: not supported by cvs2svn $
 * Revision 1.2  87/08/09  00:22:52  wesommer
 * Added RCS header.
 * 
 */


#define PW_SRV_VERSION	2		/* version number */
#ifndef KERB_HOST
#define	KERB_HOST	"kerberos"
#endif
#define INSTALL_NEW_PW	(1<<0)		/* ver, cmd, name, password,
					   old_pass, crypt_pass, uid */
#define ADMIN_NEW_PW	(2<<1)		/* ver, cmd, name, passwd,
					   old_pass (grot), crypt_pass (grot),
					   */
#define ADMIN_SET_KDC_PASSWORD (3<<1)   /* ditto */
#define ADMIN_ADD_NEW_KEY (4<<1)        /* ditto */
#define ADMIN_ADD_NEW_KEY_ATTR (5<<1)   /* ver, cmd, name, passwd,
					   inst, attr (grot),
					   */
#define INSTALL_REPLY	(1<<1)		/* ver, cmd, name, password */
#define	RETRY_LIMIT	1
#define	TIME_OUT	30
#define USER_TIMEOUT	90

#define MAX_KPW_LEN	40		/* hey, seems like a good number */

