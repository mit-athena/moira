/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/log.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/log.c,v 1.3 1988-09-14 12:16:18 mar Exp $
 */
/*  (c) Copyright 1988 by the Massachusetts Institute of Technology. */
/*  For copying and distribution information, please see the file */
/*  <mit-copyright.h>. */

#ifndef lint
static char *rcsid_log_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/log.c,v 1.3 1988-09-14 12:16:18 mar Exp $";
#endif	lint

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
#include <varargs.h>
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
int log_priority;
extern char *whoami;

sms_update_com_err_hook(whoami, code, fmt, args)
    char *whoami;
    int code;
    char *fmt;
    va_list args;
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
    if (code) {
	strcpy(cp, error_message(code));
	while (*cp)
	    cp++;
    }
    _strbuf._flag = _IOWRT+_IOSTRG;
    _strbuf._ptr = cp;
    _strbuf._cnt = BUFSIZ-(cp-buf);
    _doprnt(fmt, args, &_strbuf);
    putc('\0', &_strbuf);
#ifdef use_syslog
    syslog(syslog_prio[log_priority], "%s", buf);
#endif
#ifdef use_tty
    puts(buf);
#endif
}

sms_update_initialize()
{
    static int initialized = 0;
    if (initialized)
	return;
#ifdef use_syslog
    openlog(whoami, LOG_PID, LOG_DAEMON);
#endif
    (void) set_com_err_hook(sms_update_com_err_hook);
    log_priority = log_INFO;
    initialized = 1;
}


static char fmt[] = "[%s] %s";

#define	def(name,level,prio) \
    name(msg) \
    char *msg; \
{\
     register int old_prio; \
     old_prio = log_priority; \
     sms_update_initialize(); \
     com_err(whoami, 0, fmt, level, msg); \
     log_priority = old_prio; \
}

def(sms_log_error, "error", log_ERROR)
def(sms_log_warning, "warning", log_WARNING)
def(sms_log_info, "info", log_INFO)
def(sms_debug, "debug", log_DEBUG)
