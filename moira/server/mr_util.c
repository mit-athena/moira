/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_util.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_util.c,v 1.9 1987-08-04 01:50:13 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 * Revision 1.8  87/07/29  16:02:48  wesommer
 * Use unsigned char rather than char to prevent sign extension
 * problem.
 * 
 * Revision 1.7  87/07/14  00:39:47  wesommer
 * Changed interface to log_args.
 * 
 * Revision 1.6  87/07/06  16:09:07  wesommer
 * Only print ... if the string is too long..
 * 
 * Revision 1.5  87/06/30  20:05:52  wesommer
 * Added range checking.
 * 
 * Revision 1.4  87/06/21  16:42:19  wesommer
 * Performance work, rearrangement of include files.
 * 
 * Revision 1.3  87/06/04  01:35:28  wesommer
 * Added better logging routines.
 * 
 * Revision 1.2  87/06/03  16:08:07  wesommer
 * Fixes for lint.
 * 
 * Revision 1.1  87/06/02  20:07:32  wesommer
 * Initial revision
 * 
 */

#ifndef lint
static char *rcsid_sms_util_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_util.c,v 1.9 1987-08-04 01:50:13 wesommer Exp $";
#endif lint

#include "sms_server.h"

#include <ctype.h>
#include <strings.h>

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

log_args(tag, argc, argv)
	char *tag;
	int argc;
	char **argv;
{
	char buf[BUFSIZ];
	register int i;
	register char *bp = buf;
	
	i = strlen(tag);
	bcopy(tag, bp, i+1);
	bp += i;
	*bp++ =':';
	*bp++ =' ';
       
	for (i = 0; i < argc && ((buf - bp) + 1024) > 2; i++) {
		if (i != 0) {
			*bp++ = ',';
			*bp++ = ' '; 
		}
		bp = requote(bp, argv[i], (buf - bp) + 1024);
	}
	*bp = '\0';
	com_err(whoami, 0, buf);
}
	
void sms_com_err(whoami, code, fmt, pvar)
	char *whoami;
	int code;
	char *fmt;
	caddr_t pvar;
{
	extern char *error_message();
	extern client *cur_client;
	
	struct iovec strings[7];

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
