/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_connect.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_connect.c,v 1.13 1990-02-05 12:48:18 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *	
 * 	This routine is part of the client library.  It handles
 *	creating a connection to the sms server.
 */

#ifndef lint
static char *rcsid_sms_connect_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_connect.c,v 1.13 1990-02-05 12:48:18 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include "sms_private.h"
#include <sms_app.h>
#include <strings.h>

static char *sms_server_host = 0;

/*
 * Open a connection to the sms server.  Looks for the server name
 * 1) passed as an argument, 2) in environment variable, 3) by hesiod
 * 4) compiled in default (from sms_app.h).
 */

int sms_connect(server)
char *server;
{
    extern int errno;
    char *p, **pp, sbuf[256];
    extern char *getenv(), **hes_resolve();
	
    if (!sms_inited) sms_init();
    if (_sms_conn) return SMS_ALREADY_CONNECTED;
		
    if (!server || (strlen(server) == 0)) {
	server = getenv("MOIRASERVER");
    }

#ifdef HESIOD
    if (!server || (strlen(server) == 0)) {
	pp = hes_resolve("moira", "sloc");
	if (pp) server = *pp;
    }
#endif HESIOD

    if (!server || (strlen(server) == 0)) {
	server = SMS_SERVER;
    }

    if (!index(server, ':')) {
	p = index(SMS_SERVER, ':');
	p++;
	sprintf(sbuf, "%s:%s", server, p);
	server = sbuf;
    }

    errno = 0;
    _sms_conn = start_server_connection(server, ""); 
    if (_sms_conn == NULL)
	return errno;
    if (connection_status(_sms_conn) == CON_STOPPED) {
	register status = connection_errno(_sms_conn);
	if (!status) status = SMS_CANT_CONNECT;
	sms_disconnect();
	return status;
    }

    /*
     * stash hostname for later use
     */

    sms_server_host = strsave(server);
    if (p = index(sms_server_host, ':'))
	*p = 0;
    sms_server_host = canonicalize_hostname(sms_server_host);
    return 0;
}
	
int sms_disconnect()
{
    CHECK_CONNECTED;
    _sms_conn = sever_connection(_sms_conn);
    free(sms_server_host);
    sms_server_host = 0;
    return 0;
}

int sms_host(host, size)
  char *host;
  int size;
{
    CHECK_CONNECTED;

    /* If we are connected, sms_server_host points to a valid string. */
    strncpy(host, sms_server_host, size);
    return(0);
}

int sms_noop()
{
    int status;
    sms_params param_st;
    struct sms_params *params = NULL;
    struct sms_params *reply = NULL;

    CHECK_CONNECTED;
    params = &param_st;
    params->sms_version_no = sending_version_no;
    params->sms_procno = SMS_NOOP;
    params->sms_argc = 0;
    params->sms_argl = NULL;
    params->sms_argv = NULL;
	
    if ((status = sms_do_call(params, &reply)) == 0)
	status = reply->sms_status;
	
    sms_destroy_reply(reply);

    return status;
}
