/*
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/reg_svr.h,v $
 *      $Author: qjb $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/reg_svr.h,v 1.6 1990-01-12 11:22:48 qjb Exp $
 *
 *      Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *      Server for user registration with SMS and Kerberos.
 *
 *      This file contains all the information needed by all source
 *      files for the user registration server.
 */

#include <sys/types.h>
#include <ctype.h>
#include "ureg_err.h"
#include "ureg_proto.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define FAIL_INST "reg_svr"	/* Instance for failure zephyrgrams */

#define CUR_UREG_VERSION 1	/* Version for the register protocol */
#define SUCCESS 0		/* General purpose success code */
#define FAILURE 1		/* To use when any non-zero number will work */
#define min(a,b) ((a)>(b)?(b):(a))
#define MIN_UNAME 3		/* Username must be between three and */
#define MAX_UNAME 8		/*    eight characters long. */
#define CRYPT_LEN 14		/* crypt() returns a 13 char string */
#define LOGIN_LEN MAX_UNAME + 1	/* Leave room for a null */
#define UID_LEN 7		/* Allow room for a 16 bit number */

extern char *strdup();
extern char *malloc();

extern char *whoami;		/* Name of program - used by libraries */
extern int errno;		/* Unix error number */

/* This structure holds information from the SMS database that will be
   worth holding on to.  An instance of it appears in the formatted 
   packet structure. */
struct db_data
{
    char mit_id[CRYPT_LEN];	/* Encrypted MIT ID */
    int reg_status;		/* Registration status */
    char uid[UID_LEN];		/* Reserved uid */
    char login[LOGIN_LEN];	/* Login (username) */
};

/* This structure stores information sent over in the packet in a 
   more convenient format and also stores some information obtained 
   from the database that will be needed for each transaction.  It
   initialized from format_pkt() and find_user(). */
struct msg
{    
    U_32BIT version;		/* User registration protocol version */
    U_32BIT request;		/* Request */
    char *first;		/* First name */
    char *last;			/* Last name */
    char *encrypted;		/* Encrypted information in packet */
    int encrypted_len;		/* Length of encrypted information in packet */
    char *leftover;		/* Leftover information sent in the packet */
    int leftover_len;		/* Length of leftover information */
    struct db_data db;		/* Information from the SMS database */
};

void failure_alert();		/* Log an unexplainable failure */
int parse_pkt();		/* Parse a packet from the client */
int format_pkt();		/* Prepare a packet to send to client*/
int verify_user();		/* Make sure user is allowed to register */
int reserve_user();		/* Reserve a login for this user */
int set_password();		/* Set this user's password */


/* For logging successful database transactions */
extern FILE *journal;

