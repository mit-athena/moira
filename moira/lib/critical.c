/* $Id $
 *
 * Log and send a zephyrgram about any critical errors.
 *
 * (c) Copyright 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */


#ifdef ZEPHYR
/* need to include before moira.h, which includes krb_et.h, because
   zephyr.h is broken */
#include <zephyr/zephyr.h>
/* zephyr.h doesn't prototype this */
extern Code_t ZSendNotice(ZNotice_t *notice, Z_AuthProc cert_routine);
#endif

#include <mit-copyright.h>
#include <moira.h>
#include <moira_site.h>

#include <stdio.h>
#include <string.h>
#ifdef SYSLOG
#include <syslog.h>
#endif

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/critical.c,v 1.17 1998-02-07 17:50:30 danw Exp $");

/* mode to create the file with */
#define LOGFILEMODE	0644

extern char *whoami;

/* This routine sends a class MOIRA zephyrgram of specified instance
 * and logs to a special logfile the message passed to it via msg
 * and args in printf format.  *** It expects the global variable
 * whoami to be defined and contain the name of the calling program.
 */

void critical_alert(char *instance, char *msg, ...)
{
  FILE *crit;			/* FILE for critical log file */
  char buf[BUFSIZ];		/* Holds the formatted message */
  va_list ap;

  va_start(ap, msg);
  vsprintf(buf, msg, ap);
  va_end(ap);

  /* Send zephyr notice */
  send_zgram(instance, buf);

  /* Log message to critical file */
  if ((crit = fopen(CRITERRLOG, "a")))
    {
      time_t t;
      char *time_s;

      time(&t);
      time_s = ctime(&t) + 4;
      time_s[strlen(time_s) - 6] = '\0';

      fprintf(crit, "%s <%ld> %s\n", time_s, (long)getpid(), buf);
      fclose(crit);
    }

  com_err(whoami, 0, buf);
}



/* Sends a zephyrgram of class "MOIRA", instance as a parameter.  Ignores
 * errors while sending message.
 */

void send_zgram(char *inst, char *msg)
{
#ifdef ZEPHYR
  ZNotice_t znotice;

  memset(&znotice, 0, sizeof(znotice));
  znotice.z_kind = UNSAFE;
  znotice.z_class = "MOIRA";
  znotice.z_class_inst = inst;
  znotice.z_default_format = "MOIRA $instance on $fromhost:\n $message\n";
  ZInitialize();
  znotice.z_message = msg;
  znotice.z_message_len = strlen(msg) + 1;
  znotice.z_opcode = "";
  znotice.z_recipient = "";
  ZSendNotice(&znotice, ZNOAUTH);
#endif
#ifdef SYSLOG
  {
    char buf[512];
    sprintf(buf, "MOIRA: %s %s", inst, msg);
    syslog(LOG_ERR, buf);
  }
#endif
}
