/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/critical.c,v 1.1 1988-08-03 16:58:23 qjb Exp $
 *
 * Log and send a zephyrgram about any critical errors.
 */

#include <stdio.h>
#include <sys/file.h>
#include <zephyr/zephyr.h>


/* log file for critical events that require human intervention */
#define CRITERRLOG	"/u1/sms/critical.log"

/* mode to create the file with */
#define LOGFILEMODE	0644


void critical_alert(instance, msg, args)
  char *instance;		/* Instance for zephyr gram */
  char *msg;			/* printf format message */
				/* args = arguements, printf style */
  /* This routine sends a class SMS zephyrgram of specified instance
     and logs to a special logfile the message passed to it via msg
     and args in printf format. */
  /* Note: The part of this code that process the variable arguements
     was stolen from sprintf(). */
{
    FILE _bufstr;		/* For _doprnt() */
    int crit;			/* File descriptor for critical log file */
    char buf[BUFSIZ];		/* Holds the formatted message */

    /* Put the fully formatted message into buf */
    _bufstr._flag = _IOWRT + _IOSTRG;
    _bufstr._ptr = buf;
    _bufstr._cnt = BUFSIZ;
    _doprnt(msg, &args, &_bufstr);
    putc('\0', &_bufstr);

    /* Send zephyr notice */
    send_zgram(instance, buf);

    /* Log message to critical file */
    if ((crit = open(CRITERRLOG, 
		     O_WRONLY | O_APPEND | O_CREAT, LOGFILEMODE)) >= 0)
    {
	write(crit,buf,strlen(buf));
	close(crit);
    }
}



/* Sends a zephyrgram of class "SMS", instance as a parameter.  Ignores
 * errors while sending message.
 */

send_zgram(inst, msg)
char *inst;
char *msg;
{
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
}
