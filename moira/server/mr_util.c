/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_util.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_util.c,v 1.1 1987-06-02 20:07:32 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 */

#ifndef lint
static char *rcsid_sms_util_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_util.c,v 1.1 1987-06-02 20:07:32 wesommer Exp $";
#endif lint

#include "sms_private.h"
#include "sms_server.h"
#include <sys/uio.h>

#include <ctype.h>
#ifdef notdef
frequote(f, cp)
	FILE *f;
	register char *cp;
{
	register char c;
	putc('"', f);
	for( ; c= *cp; *cp++){
		if (c == '\\' || c == '"') putc('\\', f);
		if (isprint(c)) putc(c, f);
		else fprintf(f, "\\%03o", c);
	}
	putc('"', f);
}
#endif notdef

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
