/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_glue.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_glue.c,v 1.1 1987-07-14 00:41:18 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	Glue routines to allow the database stuff to be linked in to
 * 	a program expecting a library level interface.
 * 
 * 	$Log: not supported by cvs2svn $
 */

#ifndef lint
static char *rcsid_sms_glue_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_glue.c,v 1.1 1987-07-14 00:41:18 wesommer Exp $";
#endif lint

#include "sms_server.h"
#include <krb.h>		/* XXX for error codes */
#include <pwd.h>
static int already_connected = 0;

#define CHECK_CONNECTED { if (!already_connected) return SMS_NOT_CONNECTED; }

static client pseudo_client;
extern int krb_err_base;

sms_connect()
{
    register int status;

    if (already_connected) return SMS_ALREADY_CONNECTED;

    init_sms_err_tbl();
    init_krb_err_tbl();
    bzero((char *)&pseudo_client, sizeof(pseudo_client)); /* XXX */

    status =  sms_open_database();
    if (!status) already_connected = 1;
    return status;
}

sms_disconnect()
{
    CHECK_CONNECTED;
    sms_close_database();
    already_connected = 0;
    return 0;
}

sms_noop()
{
    CHECK_CONNECTED;
    return 0;
}
/*
 * This routine is rather bogus, as it only fills in who you claim to be.
 */
sms_auth()
{
    struct passwd *pw;
    
    CHECK_CONNECTED;
    pw = getpwuid(getuid());
    if (!pw) return (KDC_PR_UNKNOWN + krb_err_base); /* XXX hack (we 
						    * need an extended 
						    * error code table)
						    */
    strcpy(pseudo_client.kname.name, pw->pw_name);
    get_krbrlm(pseudo_client.kname.realm, 1);
}

int sms_query(name, argc, argv, callproc, callarg)
    char *name;		/* Query name */
    int argc;		/* Arg count */
    char **argv;		/* Args */
    int (*callproc)();	/* Callback procedure */
    char *callarg;		/* Callback argument */
{
    return sms_process_query(&pseudo_client, name, argc, argv,
			     callproc, callarg);
}

int sms_access(name, argc, argv)
    char *name;			/* Query name */
    int argc;			/* Arg count */
    char **argv;		/* Args */
{
    return sms_check_access(&pseudo_client, name, argc, argv);
}

int sms_query_internal(argc, argv, callproc, callarg)
    int argc;
    char **argv;
    int (*callproc)();
    char *callarg;
{
    return sms_process_query(&pseudo_client, argv[0], argc-1, argv+1,
			     callproc, callarg);
}

int sms_access_internal(argc, argv)
    int argc;
    char **argv;
{
    return sms_check_access(&pseudo_client, argv[0], argc-1, argv+1);
}

sms_shutdown(why)
    char *why;
{
    fprintf(stderr, "Sorry, not implemented\n");
}

/*
 * Local Variables:
 * mode: c
 * c-indent-level: 4
 * c-continued-statement-offset: 4
 * c-brace-offset: -4
 * c-argdecl-indent: 4
 * c-label-offset: -4
 * End:
 */
