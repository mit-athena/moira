/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/userreg.h,v $
 *	$Author: danw $
 *	$Locker:  $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/userreg.h,v 1.9 1998-01-05 19:52:27 danw Exp $
 */

#include <stdio.h>
#include <ctype.h>
#include "files.h"

/*
 * This is a kludge for compatibility with the old Athenareg stuff
 */

struct user {
  char u_first[100];
  char u_mid_init[100];
  char u_last[100];
  char u_login[100];
  char u_password[100];
  char u_mit_id[100];
  char u_home_dir[100];
  int u_status;
};

struct alias {
  int foo;
};

#define SUCCESS 0
#define FAILURE 1
#define NOT_FOUND 2
#define FIRST_NAME_SIZE 17
#define LAST_NAME_SIZE 17
#define MID_INIT_SIZE 17
#define LOGIN_SIZE 9
#define PASSWORD_SIZE 64

/* Input timeouts.  The most important timeouts are those for the username
   and the new password which should not be any longer than necessary.  The
   firstname timeout causes userreg to restart itself periodically since
   userreg is waiting for a firstname when it is not being used.  All the
   other timeouts are just there so that userreg will not stay in a half-used
   state -- possibly confusing an unwary registree.
 */
#define FIRSTNAME_TIMEOUT            180 /* 3 minutes */
#define MI_TIMEOUT                    90 /* 1.5 minutes */
#define LASTNAME_TIMEOUT              90 /* 1.5 minutes */
#define MITID_TIMEOUT                 90 /* 1.5 minutes */
#define USERNAME_TIMEOUT             180 /* This should not be too long */
#define OLD_PASSWORD_TIMEOUT          90 /* 1.5 minutes */
#define NEW_PASSWORD_TIMEOUT         180 /* Neither should this */
#define REENTER_PASSWORD_TIMEOUT      90 /* 1.5 minutes */
#define YN_TIMEOUT                    90 /* 1.5 minutes */
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
