/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/log.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/log.c,v 1.13 1998-01-06 20:40:22 danw Exp $
 */
/*  (c) Copyright 1988 by the Massachusetts Institute of Technology. */
/*  For copying and distribution information, please see the file */
/*  <mit-copyright.h>. */

#ifndef lint
static char *rcsid_log_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/log.c,v 1.13 1998-01-06 20:40:22 danw Exp $";
#endif

/*
 * handle logging for dcm and update server
 *
 * this should eventually use zephyr
 */

/*
 * define syslog for using syslog,
 * default to tty
 */

#include <mit-copyright.h>
#include <stdio.h>
#include "com_err.h"
#include <stdarg.h>
#include "update.h"
#include <krb.h>

#ifdef use_syslog
#include <syslog.h>
#else
#define use_tty
#endif

#ifdef use_syslog
int syslog_prio[] = {
  LOG_DEBUG,
  LOG_INFO,
  LOG_WARNING,
  LOG_ERR
};
#endif
extern int log_priority;
extern char *whoami;

void mr_update_com_err_hook(const char *whoami, long code,
			    const char *fmt, va_list args)
{
  char buf[BUFSIZ], *cp;
  FILE _strbuf;

#ifndef use_syslog
  strcpy(buf, whoami);
  for (cp = buf; *cp; cp++)
    ;
  *cp++ = ':';
  *cp++ = ' ';
#else
  cp = buf;
  *cp = '\0';
#endif
  if (code)
    {
      strcpy(cp, error_message(code));
      while (*cp)
	cp++;
    }
  vsprintf(cp, fmt, args);
#ifdef use_syslog
  syslog(syslog_prio[log_priority], "%s", buf);
#endif
#ifdef use_tty
  puts(buf);
#endif
}

mr_update_initialize(void)
{
  static int initialized = 0;
  if (initialized)
    return;
#ifdef use_syslog
  openlog(whoami, LOG_PID, LOG_DAEMON);
#endif
  set_com_err_hook(mr_update_com_err_hook);
  log_priority = log_INFO;
  initialized = 1;
}


static char fmt[] = "[%s] %s";

#define	def(name, level, prio) \
name(char *msg)\
{\
   int old_prio; \
   old_prio = log_priority; \
   mr_update_initialize(); \
   com_err(whoami, 0, fmt, level, msg); \
   log_priority = old_prio; \
}

def(mr_log_error, "error", log_ERROR)
def(mr_log_warning, "warning", log_WARNING)
def(mr_log_info, "info", log_INFO)
def(mr_debug, "debug", log_DEBUG)
