/*
 * Verify that all SMS updates are successful
 *
 * Copyright 1988 by the Massachusetts Institute of Technology. For copying
 * and distribution information, see the file "mit-copyright.h". 
 *
 * $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrcheck/mrcheck.c,v $
 * $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrcheck/mrcheck.c,v 1.3 1989-06-28 13:20:16 mar Exp $
 * $Author: mar $
 */

#ifndef lint
static char *rcsid_chsh_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrcheck/mrcheck.c,v 1.3 1989-06-28 13:20:16 mar Exp $";
#endif	lint

#include <stdio.h>
#include <sms.h>
#include <sms_app.h>
#include "mit-copyright.h"

char *malloc(), *rindex(), *strsave();

static int status;
static char *whoami;


char *atot(itime)
char *itime;
{
    int time;
    char *ct, *ctime();

    time = atoi(itime);
    ct = ctime(&time);
    ct[24] = 0;
    return(&ct[4]);
}

gserv(argc, argv, sq)
int argc;
char **argv;
struct save_queue *sq;
{
    char *tmp;

    tmp = strsave(atot(argv[4]));
    printf("Service %s, error %s: %s\n\tlast success %s, last try %s\n",
	   argv[0], argv[9], argv[10], tmp, atot(argv[5]));
    free(tmp);
    return(SMS_CONT);
}

ghost(argc, argv, sq)
int argc;
char **argv;
struct save_queue *sq;
{
    char *tmp;

    tmp = strsave(atot(argv[9]));
    printf("Host %s:%s, error %s: %s\n\tlast success %s, last try %s\n",
	   argv[0], argv[1], argv[6], argv[7], tmp, atot(argv[8]));
    free(tmp);
    return(SMS_CONT);
}

save_args(argc, argv, sq)
int argc;
char **argv;
struct save_queue *sq;
{
    sq_save_args(argc, argv, sq);
    return(SMS_CONT);
}

main(argc, argv)
    char *argv[];

{
    char *args[6], buf[BUFSIZ], **service, **host, *motd;
    struct save_queue *services, *hosts;
    int count = 0, scream();

    if ((whoami = rindex(argv[0], '/')) == NULL)
	whoami = argv[0];
    else
	whoami++;

    if (argc > 1) {
	usage();
    }

    status = sms_connect(SMS_SERVER);
    if (status) {
	(void) sprintf(buf, "\nConnection to the SMS server failed.");
	goto punt;
    }

    status = sms_motd(&motd);
    if (status) {
        com_err(whoami, status, " unable to check server status");
	exit(2);
    }
    if (motd) {
	fprintf(stderr, "The SMS server is currently unavailable:\n%s\n", motd);
	sms_disconnect();
	exit(2);
    }
    status = sms_auth("smscheck");
    if (status) {
	(void) sprintf(buf, "\nAuthorization failure -- run \"kinit\" \
and try again");
	goto punt;
    }

    services = sq_create();
    args[0] = args[2] = "TRUE";
    args[1] = "DONTCARE";
    if ((status = sms_query("qualified_get_server", 3, args, save_args,
			    (char *)services)) &&
	status != SMS_NO_MATCH)
      com_err(whoami, status, " while getting servers");

    hosts = sq_create();
    args[0] = "*";
    args[1] = args[5] = "TRUE";
    args[2] = args[3] = args[4] = "DONTCARE";
    if ((status = sms_query("qualified_get_server_host", 6, args, save_args,
			    (char *)hosts)) &&
	status != SMS_NO_MATCH)
      com_err(whoami, status, " while getting server/hosts");

    while (sq_get_data(services, &service)) {
	count++;
	if (status = sms_query("get_server_info", 1, service, gserv, NULL))
	  com_err(whoami, status, " while getting info about service %s",
		  service[0]);
    }

    while (sq_get_data(hosts, &host)) {
	count++; 
	if (status = sms_query("get_server_host_info", 2, host, ghost, NULL))
	  com_err(whoami, status, " while getting info about host %s:%s",
		  host[0], host[1]);
   }

    if (!count)
      strcpy(buf, "Nothing has failed at this time");
    else
      sprintf(buf, "%d thing%s ha%s failed at this time", count,
	      count == 1 ? "" : "s", count == 1 ? "s" : "ve");
    puts(buf);

    sms_disconnect();
    exit(0);

punt:
    com_err(whoami, status, buf);
    sms_disconnect();
    exit(1);
}


scream()
{
    com_err(whoami, status, "Update to SMS returned a value -- \
programmer botch.\n");
    sms_disconnect();
    exit(1);
}

usage()
{
    fprintf(stderr, "Usage: %s\n", whoami);
    exit(1);
}
