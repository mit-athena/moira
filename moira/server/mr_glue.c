/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_glue.c,v $
 *	$Author: tytso $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_glue.c,v 1.18 1993-12-10 14:01:30 tytso Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 *	Glue routines to allow the database stuff to be linked in to
 * 	a program expecting a library level interface.
 */

#ifndef lint
static char *rcsid_mr_glue_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_glue.c,v 1.18 1993-12-10 14:01:30 tytso Exp $";
#endif lint

#include <mit-copyright.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <krb_et.h>
#include <pwd.h>
#include "query.h"
#include "mr_server.h"

static int already_connected = 0;

#define CHECK_CONNECTED { if (!already_connected) return MR_NOT_CONNECTED; }

static client pseudo_client;
extern int errno;
extern char *whoami;
extern time_t now;
void reapchild();

mr_connect(server)
char *server;
{
    register int status;
    extern int query_timeout;

    if (already_connected) return MR_ALREADY_CONNECTED;

    initialize_sms_error_table();
    initialize_krb_error_table();
    bzero((char *)&pseudo_client, sizeof(pseudo_client)); /* XXX */

    query_timeout = 0;
    status =  mr_open_database();
    if (!status) already_connected = 1;

    signal(SIGCHLD, reapchild);
    return status;
}

mr_disconnect()
{
    CHECK_CONNECTED;
    mr_close_database();
    already_connected = 0;
    return 0;
}

mr_noop()
{
    CHECK_CONNECTED;
    return 0;
}
/*
 * This routine is rather bogus, as it only fills in who you claim to be.
 */
mr_auth(prog)
char *prog;
{
    struct passwd *pw;
    extern char *krb_realm;
    char buf[1024], *strsave();
    
    CHECK_CONNECTED;
    pw = getpwuid(getuid());
    if (!pw) return (KDC_PR_UNKNOWN + ERROR_TABLE_BASE_krb); /* XXX hack (we 
						    * need an extended 
						    * error code table)
						    */
    strcpy(pseudo_client.kname.name, pw->pw_name);
    krb_get_lrealm(pseudo_client.kname.realm, 1);
    krb_realm = pseudo_client.kname.realm;

    strcpy(buf, pw->pw_name);
    strcat(buf, "@");
    strcat(buf, pseudo_client.kname.realm);
    strcpy(pseudo_client.clname, buf);
    pseudo_client.users_id = 0;
    name_to_id(pseudo_client.kname.name, "USER", &pseudo_client.users_id);
    pseudo_client.client_id = pseudo_client.users_id;
    strcpy(pseudo_client.entity, prog);
    pseudo_client.args = (mr_params *) malloc(sizeof(mr_params));
    pseudo_client.args->mr_version_no = MR_VERSION_2;
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
    if (mr_trim_args(argc, argv) == MR_NO_MEM) {
	com_err(whoami, MR_NO_MEM, "while trimmming args");
    }
    (*arg->proc)(argc, argv, arg->hint);
}


int mr_query(name, argc, argv, callproc, callarg)
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
    next_incremental();
    return mr_process_query(&pseudo_client, name, argc,
			    mr_copy_args(argv, argc), callback, &hints);

}

int mr_access(name, argc, argv)
    char *name;			/* Query name */
    int argc;			/* Arg count */
    char **argv;		/* Args */
{
    time(&now);
    return mr_check_access(&pseudo_client, name, argc,
			   mr_copy_args(argv, argc));
}

int mr_query_internal(argc, argv, callproc, callarg)
    int argc;
    char **argv;
    int (*callproc)();
    char *callarg;
{
    struct hint hints;

    time(&now);
    hints.proc = callproc;
    hints.hint = callarg;
    next_incremental();
    return mr_process_query(&pseudo_client, argv[0], argc-1,
			    mr_copy_args(argv+1, argc-1), callback, &hints);
}

int mr_access_internal(argc, argv)
    int argc;
    char **argv;
{
    time(&now);
    return mr_check_access(&pseudo_client, argv[0], argc-1,
			   mr_copy_args(argv+1, argc-1));
}

mr_shutdown(why)
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
	char prog[128];
	
	if (status = check_query_access(&pseudo_query, 0, cl))
	    return(status);

	sprintf(prog, "%s/startdcm", BIN_DIR);
	pid = vfork();
	switch (pid) {
	case 0:
		execl(prog, "startdcm", 0);
		exit(1);
		
	case -1:
		return(errno);

	default:
		return(MR_SUCCESS);
	}
}


void reapchild()
{
    union wait status;
    int pid;

    while ((pid = wait3(&status, WNOHANG, (struct rusage *)0)) > 0) {
	if (pid == inc_pid) {
	    inc_running = 0;
	    next_incremental();
	}
	if  (status.w_termsig != 0 || status.w_retcode != 0)
	  com_err(whoami, 0, "%d: child exits with signal %d status %d",
		  pid, status.w_termsig, status.w_retcode);
    }
}
