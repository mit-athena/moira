/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_util.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_util.c,v 1.3 1987-06-04 01:35:28 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 * Revision 1.2  87/06/03  16:08:07  wesommer
 * Fixes for lint.
 * 
 * Revision 1.1  87/06/02  20:07:32  wesommer
 * Initial revision
 * 
 */

#ifndef lint
static char *rcsid_sms_util_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_util.c,v 1.3 1987-06-04 01:35:28 wesommer Exp $";
#endif lint

#include "sms_private.h"
#include "sms_server.h"

#include <ctype.h>
#include <strings.h>

extern char *whoami;

/*
 * XXX WARNING! THIS DOES NO RANGE CHECKING!!!
 * This is a temporary hack...
 */
char *
requote(buf, cp)
	char *buf;
	register char *cp;
{
	register char c;
	*buf++ = '"';
	for( ; c= *cp; cp++){
		if (c == '\\' || c == '"') *buf++ = '\\';
		if (isprint(c)) *buf++ = c;
		else {
			sprintf(buf, "\\%03o", c);
			buf = index(buf, '\0');
		}
	}
	*buf++ = '"';
	*buf = '\0';
	return buf;
}
/*
 * XXX WARNING! THIS DOES NO RANGE CHECKING!!!
 * This is a temporary hack...
 */
log_args(argc, argv)
	int argc;
	char **argv;
{
	char buf[BUFSIZ];
	register int i;
	register char *bp = buf;
	
	for (i = 0; i < argc; i++) {
		if (i != 0) {
			*bp++ = ',';
			*bp++ = ' '; 
		}
		bp = requote(bp, argv[i]);
	}
	*bp = '\0';
	com_err(whoami, 0, buf);
}
	
void sms_com_err(whoami, code, message)
	char *whoami;
	int code;
	char *message;
{
	extern char *error_message();
	extern client *cur_client;
	
	struct iovec strings[7];
	char buf[32];
	if (cur_client)
		(void) sprintf(buf, "[#%d]", cur_client->id);
	else buf[0]='\0';
	
	strings[1].iov_base = buf;
	strings[1].iov_len = strlen(buf);
	
	strings[0].iov_base = whoami;
	if (whoami) {
		strings[0].iov_len = strlen(whoami);
		strings[2].iov_base = ": ";
		strings[2].iov_len = 2;
	} else {
		strings[0].iov_len = 0;
		strings[2].iov_base = " ";
		strings[2].iov_len = 1;
	}
	if (code) {
		register char *errmsg = error_message(code);
		strings[3].iov_base = errmsg;
		strings[3].iov_len = strlen(errmsg);
		strings[4].iov_base = " ";
		strings[4].iov_len = 1;
	} else {
		strings[3].iov_len = 0;
		strings[4].iov_len = 0;
	}
	strings[5].iov_base = message;
	strings[5].iov_len = strlen(message);
	strings[6].iov_base = "\n";
	strings[6].iov_len = 1;
	(void) writev(2, strings, 7);
}
