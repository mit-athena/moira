/*
 * Copyright 1988 by the Massachusetts Institute of Technology. For copying
 * and distribution information, see the file "mit-copyright.h". 
 *
 * $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/chfn.c,v $
 * $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/chfn.c,v 1.9 1990-03-26 05:42:13 marc Exp $
 * $Author: marc $
 *
 */

#ifndef lint
static char *rcsid_chfn_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/chfn.c,v 1.9 1990-03-26 05:42:13 marc Exp $";
#endif not lint

/*
 * Talk to the MOIRA database to change a person's GECOS information.
 * 
 * chfn with no modifiers changes the information of the user who is 
 * running the program.
 * 
 * If a commandline argument is given, it is taken to be the username
 * of the user whose information is to be changed.
 *
 */

#define FALSE 0
#define TRUE 1

#include <sys/types.h>
#include <stdio.h>
#include <strings.h>
#include <sys/file.h>
#include <krb.h>
#include <ctype.h>
#include <errno.h>

/* MOIRA includes */
#include <moira.h>
#include <moira_site.h>
#include "mit-copyright.h"

char *whoami;

extern char *krb_err_txt[];

struct finger_info {
    char *fullname;
    char *nickname;
    char *home_address;
    char *home_phone;
    char *office_address;
    char *office_phone;
    char *mit_department;
    char *mit_year;
};

main(argc, argv)
    int argc;
    char *argv[];
{
    char pname[ANAME_SZ];
    char *uname = pname;
    int k_errno;
    char *whoami;

    if ((whoami = rindex(argv[0], '/')) == NULL)
	whoami = argv[0];
    else
	whoami++;

    if (argc > 2) {
	usage();
    }
    
    if (argc == 2)
	uname = argv[1];
    else
    {
	/* Do it right; get name from kerberos ticket file rather than 
	   from passord file. */
	
	if (k_errno = tf_init(TKT_FILE, R_TKT_FIL)) {
	    fprintf(stderr, "%s: %s\n", whoami, krb_err_txt[k_errno]);
	    exit(1);
	}
	
	if (k_errno = tf_get_pname(pname)) {
	    fprintf(stderr, "%s: %s\n", whoami, krb_err_txt[k_errno]);
	    exit(1);
	}

	tf_close();
    }
    
    exit(chfn(uname));
}

leave(status)
  int status;
  /* This should be called rather than exit once connection to moira server
     has been established. */
{
    mr_disconnect();
    exit(status);
}

scream()
{
    com_err(whoami, 0, "Unexpected return value from Moira -- programmer botch");
    leave(1);
}

chfn(uname)
  char *uname;
{
    int status;			/* general purpose exit status */
    int q_argc;			/* argc for mr_query */
    char *q_argv[F_END];	/* argv for mr_query */
    char *motd;			/* for MR server status */
    int i;

    int get_user_info();
    void get_new_info();

    struct finger_info old_info;
    struct finger_info new_info;

    /* Try each query.  If we ever fail, print error message and exit. */

    status = mr_connect(NULL);
    if (status) {
	com_err(whoami, status, "while connecting to Moira");
	exit(1);
    }

    status = mr_motd(&motd);
    if (status) {
        com_err(whoami, status, "unable to check server status");
	leave(1);
    }
    if (motd) {
	fprintf(stderr, "The Moira server is currently unavailable:\n%s\n", motd);
	leave(1);
    }

    status = mr_auth("chfn");	/* Don't use argv[0] - too easy to fake */
    if (status) {
	com_err(whoami, status, 
		"while authenticating -- run \"kinit\" and try again.");
	leave(1);
    }

    /* First, do an access check. */

    q_argv[F_NAME] = uname;
    for (i = F_NAME + 1; i < F_MODTIME; i++)
	q_argv[i] = "junk";
    q_argc = F_MODTIME;		/* one more than the last updatable field */
    
    if (status = mr_access("update_finger_by_login", q_argc, q_argv)) {
	com_err(whoami, status, "; finger\ninformation not changed.");
	leave(2);
    }

    printf("Changing finger information for %s.\n", uname);

    /* Get information */

    q_argv[NAME] = uname;
    q_argc = NAME + 1;
    if (status = mr_query("get_finger_by_login", q_argc, q_argv, 
		       get_user_info, (char *) &old_info))
    {
	com_err(whoami, status, "while getting user information.");
	leave(2);
    }

    /* Get the new information from the user */

    get_new_info(&old_info, &new_info);

    /* Do the update */

    printf("Changing finger information...\n");

    q_argv[F_NAME] = uname;
    q_argv[F_FULLNAME] = new_info.fullname;
    q_argv[F_NICKNAME] = new_info.nickname;
    q_argv[F_HOME_ADDR] = new_info.home_address;
    q_argv[F_HOME_PHONE] = new_info.home_phone;
    q_argv[F_OFFICE_ADDR] = new_info.office_address;
    q_argv[F_OFFICE_PHONE] = new_info.office_phone;
    q_argv[F_MIT_DEPT] = new_info.mit_department;
    q_argv[F_MIT_AFFIL] = new_info.mit_year;
    q_argc = F_MODTIME;		/* First non-update query argument */

    if (status = mr_query("update_finger_by_login", q_argc, q_argv,
			   scream, (char *)NULL))
    {
	com_err(whoami, status, "while updating finger information.");
	leave(1);
    }

    printf("Finger information updated succesfully.\n");

    return(0);
}

get_user_info(argc, argv, message)
  int argc;
  char *argv[];
  char *message;
{
    struct finger_info *old_info = (struct finger_info *) message;
    
    if (argc != F_END) {
	fprintf(stderr, "Some internal error occurred; try again.\n");
	leave(3);
    }
    
    printf("Info last changed on %s by user %s with %s.\n",
	   argv[F_MODTIME], argv[F_MODBY], argv[F_MODWITH]);
    
    old_info->fullname = strsave(argv[F_FULLNAME]);
    old_info->nickname = strsave(argv[F_NICKNAME]);
    old_info->home_address = strsave(argv[F_HOME_ADDR]);
    old_info->home_phone = strsave(argv[F_HOME_PHONE]);
    old_info->office_address = strsave(argv[F_OFFICE_ADDR]);
    old_info->office_phone = strsave(argv[F_OFFICE_PHONE]);
    old_info->mit_department = strsave(argv[F_MIT_DEPT]);
    old_info->mit_year = strsave(argv[F_MIT_AFFIL]);
    
    /* Only pay attention to the first match since login names are
       unique in the database. */
    return(MR_ABORT);
}

char *ask(question, def_val, phone_num)
  char *question;
  char *def_val;
  int phone_num;		/* True if this must contain only digits */
{
    static char buf[BUFSIZ];
    int ok = FALSE;
    char *result;
    int i;
    int dashes = FALSE;
    
#define BLANK "none"
    
    while (!ok)
    {
	ok = TRUE;
	printf("%s [%s]: ", question, def_val);
	if (fgets(buf, sizeof(buf), stdin) == NULL)
	  leave(0);
	buf[strlen(buf) - 1] = NULL;
	if (strlen(buf) == 0)
	    result = def_val;
	else if (strcasecmp(buf, BLANK) == NULL)
	    result = "";
	else 
	    result = buf;
	
	for (i = 0; i < strlen(buf); i++)
	{
	    switch (buf[i])
	    {
	      case '"':
		printf("'\"' is not allowed.\n");
		ok = FALSE;
		break;
	      case ',':
		printf("',' is not allowed.\n");
		ok = FALSE;
		break;
	      case ':':
		printf("':' is not allowed.\n");
		ok = FALSE;
		break;
	      default:
		if (iscntrl(buf[i])) {
		    printf("Control characters are not allowed.\n");
		    ok = FALSE;
		    break;
		}
	    }
	    if (!ok)
		break;
	}
	
	if (phone_num && ok) {
	    for (i = 0; i < strlen(result); i++) {
		if (!isdigit(result[i]) && (result[i] != '-')) {
		    printf("Phone numbers can contain only digits.\n");
		    ok = FALSE;
		    break;
		}
		if (result[i] == '-')
		    dashes = TRUE;
            }
        }
    }
    
    /* Remove dashes if necessary */
    if (dashes && result == buf) {
	char *tmp1, *tmp2;
	tmp1 = tmp2 = (char *)buf;
	do {
	    if (*(tmp1) != '-')
		*(tmp2++) = *(tmp1);
	}
	while (*(tmp1++));
    }
    
    return(result);
}
	
void get_new_info(old_info, new_info)
  struct finger_info *old_info;
  struct finger_info *new_info;
{
    printf("Default values are printed inside of '[]'.\n");
    printf("To accept the default, type <return>.\n");
    printf("To have a blank entry, type the word '%s'.\n\n", BLANK);

#define GETINFO(m,v,n) \
    new_info->v = strsave(ask(m, old_info->v, n))
    
    GETINFO("Full name", fullname, FALSE);
    GETINFO("Nickname", nickname, FALSE);
    GETINFO("Home address (Ex: Atkinson 304)", home_address, FALSE);
    GETINFO("Home phone number (Ex: 3141592)", home_phone, TRUE);
    GETINFO("Office address (Exs: E40-342 or 2-108)", 
	    office_address, FALSE);
    GETINFO("Office phone (Ex: 3-1300)", office_phone, TRUE);
    GETINFO("MIT department (Exs: 9, Biology, Information Services)", 
	    mit_department, FALSE);
    GETINFO("MIT year (Exs: 1989, '91, Faculty, Grad)", mit_year, FALSE);
}

usage()
{
    fprintf(stderr, "Usage: %s [user]\n", whoami);
    exit(1);
}
