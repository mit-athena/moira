/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/inst_001.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/inst_001.c,v 1.5 1992-08-25 14:44:25 mar Exp $
 */
/*  (c) Copyright 1988 by the Massachusetts Institute of Technology. */
/*  For copying and distribution information, please see the file */
/*  <mit-copyright.h>. */

#ifndef lint
static char *rcsid_inst_001_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/inst_001.c,v 1.5 1992-08-25 14:44:25 mar Exp $";
#endif	lint

#include <mit-copyright.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <gdb.h>
#include <moira.h>

STRING instructions = { 0, 0 };
extern CONNECTION conn;
extern int have_instructions, have_authorization;
extern int code;
extern char *whoami;


/*
 * instruction sequence transmission:
 * syntax:
 *	>>> (STRING) "inst_001"
 *	<<< (int)0
 *	>>> (STRING) instructions
 *	<<< (int)0
 */

inst_001(str)
     char *str;
{
    int file_size;
    int checksum;
    char *pathname;
    
    if (config_lookup("noinstructions")) {
	code = EPERM;
	code = send_object(conn, (char *)&code, INTEGER_T);
	com_err(whoami, EPERM, "Not allowed to receive instructions");
	return;
    }

    str += 8;
    while (*str == ' ')
	str++;
    if (!*str) {
    failure:
	reject_call(MR_ARGS);
	return;
    }
    file_size = atoi(str);
    while (isdigit(*str))
	str++;
    while (*str == ' ')
	str++;
    checksum = atoi(str);
    while (isdigit(*str))
	str++;
    while (*str == ' ')
	str++;
    if (*str != '/')
	goto failure;
    pathname = str;
    if (!have_authorization) {
	reject_call(MR_PERM);
	return;
    }
    code = send_ok();
    if (code)
	lose("sending ok for file xfer (2)");
    code = get_file(pathname, file_size, checksum, 0700);
    if (!code) {
	char buf[BUFSIZ];
	have_instructions = 1;
	strcpy(buf, "transferred file ");
	strcat(buf, pathname);
	mr_log_info(buf);
    }
#ifdef DEBUG
    printf("got instructions: %s\n", STRING_DATA(instructions));
#endif
}
