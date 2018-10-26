/* $Id$
 *
 * Log and send a zephyrgram about any critical errors.
 *
 * (c) Copyright 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */


#ifdef HAVE_ZEPHYR
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
#include <stdlib.h>
#include <string.h>
#ifndef HAVE_ZEPHYR
#include <syslog.h>
#endif
#include <time.h>
#include <unistd.h>

RCSID("$HeadURL$ $Id$");

/* mode to create the file with */
#define LOGFILEMODE	0644
#define SLACK_SCRIPT    "/moira/bin/slack-send"

/* This routine sends a class MOIRA zephyrgram of specified instance
 * and logs to a special logfile the message passed to it via msg
 * and args in printf format.  *** It expects the global variable
 * whoami to be defined and contain the name of the calling program.
 */

void critical_alert(char *whoami, char *instance, char *msg, ...)
{
  FILE *crit;
  char *buf, sysbuf[BUFSIZ];
  va_list ap;
  long start;

  /* Log message to critical file */
  if ((crit = fopen(CRITERRLOG, "a")))
    {
      time_t t;
      char *time_s;

      time(&t);
      time_s = ctime(&t) + 4;
      time_s[strlen(time_s) - 6] = '\0';

      fprintf(crit, "%s <%ld>", time_s, (long)getpid());
      start = ftell(crit);
      va_start(ap, msg);
      vfprintf(crit, msg, ap);
      va_end(ap);
      fprintf(crit, "\n");

      buf = malloc(ftell(crit) - start);
      fclose(crit);

      if (buf)
	{
	  va_start(ap, msg);
	  vsprintf(buf, msg, ap);
	  va_end(ap);

	  send_zgram(instance, buf);
	  com_err(whoami, 0, "%s", buf);
	  send_slack(buf);
	  free(buf);
	}
    }

  if (!crit || !buf)
    {
      send_zgram(instance, "Couldn't format critical syslog!");
      com_err(whoami, 0, "Couldn't format critical syslog!");
      send_slack("Couldn't format critical syslog!");
    }
}


/* Sends a zephyrgram of class "MOIRA", instance as a parameter.  Ignores
 * errors while sending message.
 */

void send_zgram(char *inst, char *msg)
{
#ifdef HAVE_ZEPHYR
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
#else
  char *buf;

  buf = malloc(9 + strlen(inst) + strlen(msg));
  if (buf)
    {
      sprintf(buf, "MOIRA: %s %s", inst, msg);
      syslog(LOG_ERR, "%s", buf);
      free(buf);
    }
#endif
}

/* Sends a message via Slack; channel / username is expected to be handled
 * via the sending script.
 */

void send_slack(char *msg)
{
  char *buf;

  if (access(SLACK_SCRIPT, F_OK|X_OK) == 0)
    {
      buf = malloc(4 + strlen(msg) + strlen(SLACK_SCRIPT));
      if (buf)
	{
	  sprintf(buf, "%s \"%s\"", SLACK_SCRIPT, msg);
	  system(buf);
	  free(buf);
	}
    }
}
