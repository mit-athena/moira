/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_glue.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_glue.c,v 1.8 1989-06-14 15:08:08 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 *	Glue routines to allow the database stuff to be linked in to
 * 	a program expecting a library level interface.
 */

#ifndef lint
static char *rcsid_sms_glue_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_glue.c,v 1.8 1989-06-14 15:08:08 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include "sms_server.h"
#include <krb.h>		/* XXX for error codes */
#include <pwd.h>
#include "query.h"

static int already_connected = 0;

#define CHECK_CONNECTED { if (!already_connected) return SMS_NOT_CONNECTED; }

static client pseudo_client;
extern int krb_err_base, errno;
extern char *malloc(), *whoami;
extern time_t now;

sms_connect(server)
char *server;
{
    register int status;
    extern int query_timeout;

    if (already_connected) return SMS_ALREADY_CONNECTED;

    init_sms_err_tbl();
    init_krb_err_tbl();
    bzero((char *)&pseudo_client, sizeof(pseudo_client)); /* XXX */

    query_timeout = 0;
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
sms_auth(prog)
char *prog;
{
    struct passwd *pw;
    extern char *krb_realm;
    char buf[1024], *strsave();
    
    CHECK_CONNECTED;
    pw = getpwuid(getuid());
    if (!pw) return (KDC_PR_UNKNOWN + krb_err_base); /* XXX hack (we 
						    * need an extended 
						    * error code table)
						    */
    strcpy(pseudo_client.kname.name, pw->pw_name);
    get_krbrlm(pseudo_client.kname.realm, 1);
    krb_realm = pseudo_client.kname.realm;

    strcpy(buf, pw->pw_name);
    strcat(buf, "@");
    strcat(buf, pseudo_client.kname.realm);
    pseudo_client.clname = malloc(strlen(buf)+1);
    strcpy(pseudo_client.clname, buf);
    pseudo_client.users_id = get_users_id(pseudo_client.kname.name);
    pseudo_client.entity = strsave(prog);
    pseudo_client.args = (sms_params *) malloc(sizeof(sms_params));
    pseudo_client.args->sms_version_no = SMS_VERSION_2;
    return 0;
}

struct hint {
    int (*proc)();
    char *hint;
};

callback(argc, argv, arg)
int argc;
char **argv;
struct hint *arg;
{
    if (sms_trim_args(argc, argv) == SMS_NO_MEM) {
	com_err(whoami, SMS_NO_MEM, "while trimmming args");
    }
    (*arg->proc)(argc, argv, arg->hint);
}


int sms_query(name, argc, argv, callproc, callarg)
    char *name;		/* Query name */
    int argc;		/* Arg count */
    char **argv;		/* Args */
    int (*callproc)();	/* Callback procedure */
    char *callarg;		/* Callback argument */
{
    struct hint hints;

    time(&now);
    hints.proc = callproc;
    hints.hint = callarg;
    return sms_process_query(&pseudo_client, name, argc, argv,
			     callback, &hints);
}

int sms_access(name, argc, argv)
    char *name;			/* Query name */
    int argc;			/* Arg count */
    char **argv;		/* Args */
{
    time(&now);
    return sms_check_access(&pseudo_client, name, argc, argv);
}

int sms_query_internal(argc, argv, callproc, callarg)
    int argc;
    char **argv;
    int (*callproc)();
    char *callarg;
{
    struct hint hints;

    time(&now);
    hints.proc = callproc;
    hints.hint = callarg;
    return sms_process_query(&pseudo_client, argv[0], argc-1, argv+1,
			     callback, &hints);
}

int sms_access_internal(argc, argv)
    int argc;
    char **argv;
{
    time(&now);
    return sms_check_access(&pseudo_client, argv[0], argc-1, argv+1);
}

sms_shutdown(why)
    char *why;
{
    fprintf(stderr, "Sorry, not implemented\n");
}


/* trigger_dcm is also used as a followup routine to the 
 * set_server_host_override query, hence the two dummy arguments.
 */

struct query pseudo_query = {
	"trigger_dcm",
	"tdcm",
};

trigger_dcm(dummy0, dummy1, cl)
	int dummy0, dummy1;
	client *cl;
{
	register int pid, status;
	
	if (status = check_query_access(&pseudo_query, 0, cl))
	    return(status);

	pid = vfork();
	switch (pid) {
	case 0:
		execl("/u1/sms/bin/startdcm", "startdcm", 0);
		exit(1);
		
	case -1:
		return(errno);

	default:
		return(SMS_SUCCESS);
	}
}
