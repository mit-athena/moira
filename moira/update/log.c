/* $Id: log.c,v 1.14 1998-02-05 22:52:01 danw Exp $
 *
 * handle logging for dcm and update server
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include "update_server.h"

#include <stdio.h>
#include <stdarg.h>

#include "update.h"

#ifdef use_syslog
#include <syslog.h>
#else
#define use_tty
#endif

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/log.c,v 1.14 1998-02-05 22:52:01 danw Exp $");

void mr_update_com_err_hook(const char *whoami, long code,
			    const char *fmt, va_list args);

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

void mr_update_initialize(void)
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
void name(char *msg)\
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
