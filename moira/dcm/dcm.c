/*
 * The Data Control Manager for MOIRA.
 *
 * Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file
 * "mit-copyright.h".
 *
 * $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dcm/dcm.c,v $
 * $Author: danw $
 * $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dcm/dcm.c,v 1.18 1997-01-29 23:13:53 danw Exp $
 */

#ifndef lint
static char rcsid_dcm_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dcm/dcm.c,v 1.18 1997-01-29 23:13:53 danw Exp $";
#endif lint

#include <stdio.h>
#include <stdlib.h>
#include <update.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>
#include <moira.h>
#include <moira_site.h>
#include "dcm.h"
#include "mit-copyright.h"
#include <unistd.h>
#include <com_err.h>

extern int log_flags, errno;
int gqval();


/* declared global so that we can get the current time from different places. */
struct timeval tv;


main(argc, argv)
int argc;
char *argv[];
{
	int i;
	char **arg = argv;
	char *qargv[3];
	char *s;
	int status;

	whoami = argv[0];
	s = getenv("DEBUG");
	dbg = s ? atoi(s) : 0;
	umask(UMASK);
	log_flags = 0;

	setvbuf(stderr, NULL, _IOLBF, BUFSIZ);
	setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
	
	while(++arg - argv < argc) {
	    if (**arg == '-')
		switch((*arg)[1]) {
		case 'd':
		    dbg =  atoi((*arg)[2]? *arg+2: *++arg);
		    break;
		}
	}
	set_com_err_hook(dcm_com_err_hook);
	
	/* if /etc/nodcm exists, punt quietly. */
	if (!access(NODCMFILE, F_OK)) {
		exit(1);
	} 

	if (status = mr_connect("")) {
	    com_err(whoami, status, " on mr_connect");
	    leave("connect failed");
	}

	if (status = mr_auth("dcm")) {
	    com_err(whoami, status, " on \"authenticate\"");
	    leave("auth failed");
	}

	/* if DCM is not enabled, exit after logging */
	qargv[0] = "dcm_enable";
	if (status = mr_query("get_value", 1, qargv, gqval, &i)) {
	    com_err(whoami, status, " check dcm_enable");
	    leave("query failed");
	}
	if (i == 0) {
	    errno = 0;
	    leave("dcm_enable not set");
	} 

	/* do it! */
	do_services();
	errno = 0;
	leave("");
}


/* Used by the get_value query when checking for dcm_enable. */

gqval(argc, argv, hint)
int argc;
char **argv;
int *hint;
{
    *hint = atoi(argv[0]);
    return(UPCALL_STOP);
}


/* Used by qualified_get_server to make a list of servers to check */

qgetsv(argc, argv, sq)
int argc;
char **argv;
struct save_queue *sq;
{
    sq_save_data(sq, strsave(argv[0]));
    return(UPCALL_CONT);
}


/* Used by get_server_info to record all of the returned information */

getsvinfo(argc, argv, sserv)
int argc;
char **argv;
struct service *sserv;
{
    sserv->service = strsave(argv[0]);
    sserv->interval = atoi(argv[1]);
    sserv->target = strsave(argv[2]);
    sserv->script = strsave(argv[3]);
    sserv->dfgen = atoi(argv[4]);
    sserv->dfcheck = atoi(argv[5]);
    sserv->type = strsave(argv[6]);
    sserv->enable = atoi(argv[7]);
    sserv->inprogress = atoi(argv[8]);
    sserv->harderror = atoi(argv[9]);
    sserv->errmsg = strsave(argv[10]);
    return(UPCALL_STOP);
}


/* Scan the services and process any that need it. */

do_services()
{
    char *qargv[6];
    struct save_queue *sq, *sq_create();
    char *service, dfgen_prog[64], dfgen_cmd[128];
    struct service svc;
    int status, lock_fd, ex, (*cstat)();
    struct timezone tz;
    register char *p;
    int waits;
    struct sigaction action, prevaction;

    if (dbg & DBG_VERBOSE)
	com_err(whoami, 0, "starting pass over services");

    qargv[0] = "true";
    qargv[1] = "dontcare";
    qargv[2] = "false";
    sq = sq_create();
    if (status = mr_query_with_retry("qualified_get_server", 3, qargv,
				      qgetsv, sq)) {
	com_err(whoami, status, " getting services");
	leave("query failed");
    }
    while (sq_get_data(sq, &service)) {
	for (p = service; *p; p++)
	  if (isupper(*p))
	    *p = tolower(*p);
	com_err(whoami, 0, "checking %s...", service);
	qargv[0] = service;
	sprintf(dfgen_prog, "%s/%s.gen", BIN_DIR, service);
	if (!file_exists(dfgen_prog)) {
	    com_err(whoami, 0, "prog %s doesn't exist\n", dfgen_prog);
	    free(service);
	    continue;
	}
	sprintf(dfgen_cmd, "exec %s %s/%s.out",
		dfgen_prog, DCM_DIR, service);
	gettimeofday(&tv, &tz);
	if (status = mr_query_with_retry("get_server_info", 1, qargv,
					  getsvinfo, &svc)) {
	    com_err(whoami, status, " getting service %s info, skipping to next service", service);
	    continue;
	}
	svc.service = strsave(service);
	qargv[0] = strsave(service);
	qargv[1] = itoa(svc.dfgen);
	qargv[2] = itoa(svc.dfcheck);
	qargv[3] = strsave("0");
	qargv[4] = itoa(svc.harderror);
	qargv[5] = strsave(svc.errmsg);
	if (svc.interval != 0) {
	    if (svc.interval * 60 + svc.dfcheck < tv.tv_sec) {
		lock_fd = maybe_lock_update("@db@", service, 1);
		if (lock_fd < 0)
		  goto free_service;
		free(qargv[3]);
		free(qargv[4]);
		free(qargv[5]);
		qargv[3] = strsave("1");
		qargv[4] = strsave("0");
		qargv[5] = strsave("");
		status = mr_query_with_retry("set_server_internal_flags", 6,
					      qargv, scream, NULL);
		if (status != MR_SUCCESS) {
		    com_err(whoami, status, " setting server state");
		    goto free_service;
		}
	    
		com_err(whoami, status, " running %s", dfgen_prog);

		action.sa_flags = 0;
		sigemptyset(&action.sa_mask);
		action.sa_handler = SIG_DFL;
		sigaction(SIGCHLD, &action, &prevaction);
		waits = system(dfgen_cmd);
		sigaction(SIGCHLD, &prevaction, NULL);
		if (WIFSIGNALED(waits)) {
		    status = MR_COREDUMP;
		    com_err(whoami, status, " %s exited on signal %d",
			    dfgen_prog, WTERMSIG(waits));
		} else if (WEXITSTATUS(waits)) {
		    /* extract the process's exit value */
		    status = WEXITSTATUS(waits) + ERROR_TABLE_BASE_sms;
		    com_err(whoami, status, " %s exited", dfgen_prog);
		}

		if (SOFT_FAIL(status)) {
		    free(qargv[5]);
		    qargv[5] = strsave(error_message(status));
		} else if (status == MR_NO_CHANGE) {
		    free(qargv[2]);
		    qargv[2] = itoa(tv.tv_sec);
		    svc.dfcheck = tv.tv_sec;
		} else if (status == MR_SUCCESS) {
		    free(qargv[1]);
		    free(qargv[2]);
		    qargv[1] = itoa(tv.tv_sec);
		    qargv[2] = strsave(qargv[1]);
		    svc.dfcheck = svc.dfgen = tv.tv_sec;
		} else { /* HARD_FAIL(status) */
		    free(qargv[2]);
		    free(qargv[4]);
		    free(qargv[5]);
		    qargv[2] = itoa(tv.tv_sec);	
		    svc.dfcheck = tv.tv_sec;
		    qargv[4] = itoa(status);
		    qargv[5] = strsave(error_message(status));
		    critical_alert("DCM","DCM building config files for %s: %s",
				  service, qargv[5]);
		}
	    free_service:
		free(qargv[3]);
		qargv[3] = strsave("0");
		status = mr_query_with_retry("set_server_internal_flags", 6,
					      qargv, scream, NULL);
		if (status)
		  com_err(whoami, status, " setting service state");
		close(lock_fd);
		free(qargv[0]);
		free(qargv[1]);
		free(qargv[2]);
		free(qargv[3]);
		free(qargv[4]);
		free(qargv[5]);
	    }
	    if (!strcmp(svc.type, "REPLICAT"))
	      ex = 1;
	    else
	      ex = 0;
	    lock_fd = maybe_lock_update("@db@", service, ex);
	    if (lock_fd >= 0) {
		do_hosts(&svc);
		close(lock_fd);
	    }
	}
	free(svc.service);
	free(svc.target);
	free(svc.script);
	free(svc.type);
	free(svc.errmsg);
	free(service);
    }
    sq_destroy(sq);
}


/* Used by qualified_get_server_host to make a list of hosts to check */

qgethost(argc, argv, sq)
int argc;
char **argv;
struct save_queue *sq;
{
    sq_save_data(sq, strsave(argv[1]));
    return(UPCALL_CONT);
}


/* Used by get_server_host_info to store all of the info about a host */

gethostinfo(argc, argv, shost)
int argc;
char **argv;
struct svrhost *shost;
{
    shost->service = strsave(argv[0]);
    shost->machine = strsave(argv[1]);
    shost->enable = atoi(argv[2]);
    shost->override = atoi(argv[3]);
    shost->success = atoi(argv[4]);
    shost->inprogress = atoi(argv[5]);
    shost->hosterror = atoi(argv[6]);
    shost->errmsg = strsave(argv[7]);
    shost->lasttry = atoi(argv[8]);
    shost->lastsuccess = atoi(argv[9]);
    shost->value1 = atoi(argv[10]);
    shost->value2 = atoi(argv[11]);
    shost->value3 = strsave(argv[12]);
    return(UPCALL_STOP);
}


/* Scans all of the hosts for a particular service, and processes them. */

do_hosts(svc)
struct service *svc;
{
    char *argv[9], *machine;
    int status, lock_fd;
    struct save_queue *sq;
    struct svrhost shost;

    sq = sq_create();
    argv[0] = svc->service;
    argv[1] = "TRUE";
    argv[2] = argv[3] = argv[4] = "DONTCARE";
    argv[5] = "FALSE";
    status = mr_query_with_retry("qualified_get_server_host", 6, argv,
				  qgethost, sq);
    if (status == MR_NO_MATCH) {
	return;
    } else if (status) {
	com_err(whoami, status, " getting server_hosts for  %s", svc->service);
	return;
    }
    while (sq_get_data(sq, &machine)) {
	if (dbg & DBG_TRACE)
	  com_err(whoami, 0, "checking %s...", machine);
	argv[1] = machine;
	status = mr_query_with_retry("get_server_host_info", 2, argv,
				      gethostinfo, &shost);
	if (status) {
	    com_err(whoami,status, " getting server_host_info for %s", machine);
	    goto free_mach;
	}
	if (!shost.enable || shost.hosterror ||
	    (shost.success && !shost.override &&
	     shost.lastsuccess >= svc->dfgen)) {
	    if (dbg & DBG_TRACE)
	      com_err(whoami, 0, "not updating %s:%s", svc->service, machine);
	    goto free_mach;
	}

	lock_fd = maybe_lock_update(machine, svc->service, 1);
	if (lock_fd < 0)
	  goto free_mach;
	argv[0] = svc->service;
	argv[1] = machine;
	argv[2] = argv[3] = argv[5] = "0";
	argv[4] = "1";
	argv[6] = strsave("");
	argv[7] = itoa(tv.tv_sec);
	argv[8] = itoa(shost.lastsuccess);
	status = mr_query_with_retry("set_server_host_internal", 9, argv,
				      scream, NULL);
	if (status != MR_SUCCESS) {
	    com_err(whoami,status," while setting internal state for %s:%s",
		    svc->service, machine);
	    goto free_mach;
	}
	status = mr_update_server(svc->service, machine, svc->target,
				   svc->script);
	if (status == MR_SUCCESS) {
	    argv[2] = argv[4] = "0";
	    argv[3] = "1";
	    free(argv[8]);
	    argv[8] = itoa(tv.tv_sec);
	} else if (SOFT_FAIL(status)) {
	    argv[4] = "0";
	    free(argv[6]);
	    argv[6] = strsave(error_message(status));
	} else { /* HARD_FAIL */
	    argv[2] = itoa(shost.override);
	    argv[4] = "0";
	    argv[5] = itoa(status);
	    free(argv[6]);
	    argv[6] = strsave(error_message(status));
	    critical_alert("DCM", "DCM updating %s:%s: %s",
			   machine, svc->service, argv[6]);
	    if (!strcmp(svc->type, "REPLICAT")) {
		char *qargv[6];

		svc->harderror = status;
		svc->errmsg = strsave(argv[6]);
		qargv[0] = strsave(svc->service);
		qargv[1] = itoa(svc->dfgen);
		qargv[2] = itoa(svc->dfcheck);
		qargv[3] = strsave("0");
		qargv[4] = itoa(svc->harderror);
		qargv[5] = strsave(svc->errmsg);
		status = mr_query_with_retry("set_server_internal_flags",
					      6, qargv, scream, NULL);
		if (status)
		  com_err(whoami, status, " setting service state again");
		free(qargv[0]);
		free(qargv[1]);
		free(qargv[2]);
		free(qargv[3]);
		free(qargv[4]);
		free(qargv[5]);
		close(lock_fd);
		status = mr_query_with_retry("set_server_host_internal",
					      9, argv,scream,NULL);
		free(argv[2]);
		free(argv[5]);
		if (status)
		  com_err(whoami, status, " setting host state again");
		return(-1);
	    }
	}
	close(lock_fd);
	status = mr_query_with_retry("set_server_host_internal", 9, argv,
				      scream, NULL);
	if (status)
	  com_err(whoami, status, " setting host state again");
/*	free(argv[2]);
	free(argv[5]); */
    free_mach:
	free(machine);
	close(lock_fd);
    }
    return(0);
}
