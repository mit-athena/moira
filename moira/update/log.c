/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/log.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/log.c,v 1.1 1987-08-22 17:54:37 wesommer Exp $
 */

#ifndef lint
static char *rcsid_log_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/log.c,v 1.1 1987-08-22 17:54:37 wesommer Exp $";
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
#define use_syslog

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
