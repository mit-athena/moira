/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/critical.c,v 1.8 1990-01-29 18:09:43 mar Exp $
 *
 * Log and send a zephyrgram about any critical errors.
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

/* At Athena, we use zephyr & not syslog.  Change the following line 
 * if necessary */
#define ZEPHYR


#include <mit-copyright.h>
#include <stdio.h>
#include <sys/file.h>
#ifdef ZEPHYR
#include <zephyr/zephyr.h>
#endif
#ifdef SYSLOG
#include <syslog.h>
#endif
#include <sms_app.h>


/* mode to create the file with */
#define LOGFILEMODE	0644

extern char *whoami;


/* This routine sends a class SMS zephyrgram of specified instance
 * and logs to a special logfile the message passed to it via msg
 * and args in printf format.  *** It expects the global variable
 * whoami to be defined and contain the name of the calling program.
 * It's a kludge that it takes a max of 8 arguments in a way that
 * isn't necessarily portable, but varargs doesn't work here and we
 * don't have vsprintf().
 */

void critical_alert(instance, msg, arg1, arg2, arg3, arg4,
		    arg5, arg6, arg7, arg8)
  char *instance;		/* Instance for zephyr gram */
  char *msg;			/* printf format message */
  char *arg1, *arg2, *arg3, *arg4, *arg5, *arg6, *arg7, *arg8;
{
    FILE *crit;			/* FILE for critical log file */
    char buf[BUFSIZ];		/* Holds the formatted message */

    sprintf(buf, msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);

    /* Send zephyr notice */
    send_zgram(instance, buf);

    /* Log message to critical file */
    if ((crit = fopen(CRITERRLOG, "a")) != (FILE *)NULL) 
    {
	long t;
	char  *time_s;

	time(&t);
	time_s = ctime(&t) + 4;
	time_s[strlen(time_s)-6] = '\0';

	fprintf(crit, "%s <%d> %s\n", time_s, getpid(), buf);
	fclose(crit);
    }

    com_err(whoami, 0, buf);
}



/* Sends a zephyrgram of class "SMS", instance as a parameter.  Ignores
 * errors while sending message.
 */

send_zgram(inst, msg)
char *inst;
char *msg;
{
#ifdef ZEPHYR
    ZNotice_t znotice;

    bzero (&znotice, sizeof (znotice));
    znotice.z_kind = UNSAFE;
    znotice.z_class = "SMS";
    znotice.z_class_inst = inst;
    znotice.z_default_format = "SMS $instance:\n $message\n";
    (void) ZInitialize ();
    znotice.z_message = msg;
    znotice.z_message_len = strlen(msg) + 1;
    znotice.z_opcode = "";
    znotice.z_recipient = "";
    ZSendNotice(&znotice, ZNOAUTH);
#endif
#ifdef SYSLOG
    {
	char buf[512];
	sprintf(buf, "SMS: %s %s", inst, msg);
	syslog(LOG_ERR, buf);
    }
#endif
}
