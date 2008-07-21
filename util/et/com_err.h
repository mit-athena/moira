/*
 * Header file for common error description library.
 *
 * Copyright 1988, Student Information Processing Board of the
 * Massachusetts Institute of Technology.
 *
 * For copyright and distribution info, see the documentation supplied
 * with this package.
 */

#ifndef COM_ERR__H
#define COM_ERR__H

#include <stdarg.h>

typedef void (*com_err_handler_t)(const char *, long, const char *, va_list);

void com_err(const char *progname, long code, const char *fmt, ...);
void com_err_va(const char *progname, long code, const char *fmt,
		va_list args);
char const *error_message(long code);
com_err_handler_t set_com_err_hook(com_err_handler_t handler);
com_err_handler_t reset_com_err_hook(void);

#endif
