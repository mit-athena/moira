/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/files.h,v $
 *	$Author: wesommer $
 *	$Locker:  $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/files.h,v 1.2 1987-08-22 18:37:39 wesommer Exp $
 */

#ifndef __SABER__
#define TEXT "/usr/athena/lib/userreg

#define WELCOME TEXT/welcome"	/* welcome message */
#define NETWORK_DOWN TEXT/network_down"
#define USERNAME_BLURB TEXT/username_blurb"
#define PASSWORD_BLURB TEXT/password_blurb"
#else
#define TEXT "/usr/athena/lib/userreg"

#define WELCOME TEXT "/welcome"	/* welcome message */
#define NETWORK_DOWN TEXT "/network_down"
#define USERNAME_BLURB TEXT "/username_blurb"
#define PASSWORD_BLURB TEXT "/password_blurb"
#endif __SABER__
