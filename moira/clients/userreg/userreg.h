/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/userreg.h,v $
 *	$Author: ostlund $
 *	$Locker:  $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/userreg.h,v 1.1 1986-08-21 18:06:46 ostlund Exp $
 */

#include <stdio.h>
#include <ctype.h>
#include "files.h"

#include "../userinfod/db.h"
#include "../userinfod/protocol.h"

/* Input timeouts.  The most important timeouts are those for the username
   and the new password which should not be any longer than necessary.  The
   firstname timeout causes userreg to restart itself periodically since
   userreg is waiting for a firstname when it is not being used.  All the
   other timeouts are just there so that userreg will not stay in a half-used
   state -- possibly confusing an unwary registree.
 */
#define FIRSTNAME_TIMEOUT            600 /* 30 minutes */
#define MI_TIMEOUT                    90
#define LASTNAME_TIMEOUT              90
#define MITID_TIMEOUT                 90
#define USERNAME_TIMEOUT              90 /* This should not be too long */
#define OLD_PASSWORD_TIMEOUT          90
#define NEW_PASSWORD_TIMEOUT          90 /* Neither should this */
#define REENTER_PASSWORD_TIMEOUT      90
#define YN_TIMEOUT                    90
#define TIMER_TIMEOUT                 90 /* default timeout for timer_on() */

#define NO    0
#define YES   1

/* Externs from display.c */
extern  setup_display ();
extern  reset_display ();
extern  restore_display ();
extern  redisp ();
extern  input ();
extern  input_no_echo ();
extern  display_text_line ();
extern  display_text ();

/* Global variables */
extern struct user  user,
                    db_user;
char typed_mit_id[100];
