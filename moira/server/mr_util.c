/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_util.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_util.c,v 1.15 1989-08-16 20:47:49 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 */

#ifndef lint
static char *rcsid_sms_util_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_util.c,v 1.15 1989-08-16 20:47:49 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include "sms_server.h"

#include <ctype.h>
#include <strings.h>
#include <sys/types.h>

extern char *whoami;

char *
requote(buf, cp, len)
	char *buf;
	register char *cp;
{
	register int count = 0;
	register unsigned char c;
	if (len <= 2) return buf;
	*buf++ = '"'; count++; len--;
	for(; (count < 40) && (len > 1) && (c = *cp);
	    cp++, --len, ++count) {
		if (c == '\\' || c == '"') *buf++ = '\\';
		if (isprint(c)) *buf++ = c;
		else {
			sprintf(buf, "\\%03o", c);
			buf = index(buf, '\0');
		}
	}
	if (len > 1) { *buf++ = '"'; count++; len--; }
	if (len > 3 && count >= 40) {
		*buf++ = '.'; count++; len--;
		*buf++ = '.'; count++; len--;
		*buf++ = '.'; count++; len--;
	}
	if (len > 1) *buf = '\0';
	return buf;
}

log_args(tag, version, argc, argv)
	char *tag;
	int version;
	int argc;
	char **argv;
{
	char buf[BUFSIZ];
	register int i;
	register char *bp;
	
	i = strlen(tag);
	sprintf(buf, "%s[%d]: ", tag, version);
	for (bp = buf; *bp; bp++);
       
	for (i = 0; i < argc && ((buf - bp) + 1024) > 2; i++) {
		if (i != 0) {
			*bp++ = ',';
			*bp++ = ' '; 
		}
		bp = requote(bp, argv[i], (buf - bp) + 1024);
	}
	*bp = '\0';
	com_err(whoami, 0, "%s", buf);
}
	
void sms_com_err(whoami, code, fmt, pvar)
	char *whoami;
	int code;
	char *fmt;
	caddr_t pvar;
{
	extern char *error_message();
	extern client *cur_client;
	
	if (whoami) {
		fputs(whoami, stderr);
		if (cur_client) fprintf(stderr, "[#%d]", cur_client->id);
		fputs(": ", stderr);
	}
	if (code) {
		fputs(error_message(code), stderr);
	}
	if (fmt) {
		_doprnt(fmt, pvar, stderr);
	}
	putc('\n', stderr);
}


/* sms_trim_args: passed an argument vector, it will trim any trailing
 * spaces on the args by writing a null into the string.
 */

int sms_trim_args(argc, argv)
int argc;
char **argv;
{
    register char **arg;
    register char *p, *lastch;

    for (arg = argv; argc--; arg++) {
	for (lastch = p = *arg; *p; p++)
	  if (!isspace(*p))
	    lastch = p;
	if (p != lastch) {
	    if (isspace(*lastch))
	      *lastch = 0;
	    else
	      *(++lastch) = 0;
	}
    }
    return(0);
}
