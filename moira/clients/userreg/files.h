/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/files.h,v $
 *	$Author: mar $
 *	$Locker:  $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/files.h,v 1.5 1990-03-13 12:59:11 mar Exp $
 */

#ifndef __SABER__
#define TEXT "/mit/register

#define WELCOME TEXT/welcome"	/* welcome message */
#define NETWORK_DOWN TEXT/network_down"
#define USERNAME_BLURB TEXT/username_blurb"
#define PASSWORD_BLURB TEXT/password_blurb"
#define DELETED_ACCT TEXT/deleted_acct"
#define NOT_ALLOWED TEXT/not_allowed"
#define IMPROPER_LOGIN TEXT/improper_reg"
#define OFFER_ENROLL TEXT/offer_enroll"
#else
#define TEXT "/mit/register"

#define WELCOME TEXT "/welcome"	/* welcome message */
#define NETWORK_DOWN TEXT "/network_down"
#define USERNAME_BLURB TEXT "/username_blurb"
#define PASSWORD_BLURB TEXT "/password_blurb"
#define DELETED_ACCT TEXT "/deleted_acct"
#define NOT_ALLOWED TEXT "/not_allowed"
#define IMPROPER_LOGIN TEXT "/improper_reg"
#define OFFER_ENROLL TEXT "/offer_enroll"
#endif __SABER__
