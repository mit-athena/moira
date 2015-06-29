/*
 * Copyright 1987, 1988 by MIT Student Information Processing Board.
 *
 * For copyright info, see mit-sipb-copyright.h.
 */

#include "mit-sipb-copyright.h"

#include <stdio.h>
#include <stdarg.h>
#include "error_table.h"
#include "com_err.h"

static const char rcsid[] = "$Id: com_err.c 3956 2010-01-05 20:56:56Z zacheiss $";

static void default_com_err_proc(const char *progname, long code,
				 const char *fmt, va_list args);

com_err_handler_t com_err_hook = default_com_err_proc;

static void default_com_err_proc(const char *progname, long code,
				 const char *fmt, va_list args)
{
    if (progname) {
	fputs(progname, stderr);
	fputs(": ", stderr);
    }

    if (code) {
	fputs(error_message(code), stderr);
	fputs(" ", stderr);
    }
    if (fmt)
        vfprintf(stderr, fmt, args);

    /* possibly should do the \r only on a tty in raw mode */
    putc('\r', stderr);
    putc('\n', stderr);
    fflush(stderr);
}

void com_err_va(const char *progname, long code, const char *fmt, va_list args)
{
    (*com_err_hook)(progname, code, fmt, args);
}

void com_err(const char *progname, long code, const char *fmt, ...)
{
    va_list pvar;

    va_start(pvar, fmt);
    com_err_va(progname, code, fmt, pvar);
    va_end(pvar);
}

com_err_handler_t set_com_err_hook(com_err_handler_t new_proc)
{
    com_err_handler_t x = com_err_hook;

    com_err_hook = (new_proc) ? new_proc : default_com_err_proc;
    return x;
}

com_err_handler_t reset_com_err_hook()
{
    return set_com_err_hook(NULL);
}
