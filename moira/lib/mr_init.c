/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_init.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_init.c,v 1.1 1987-05-31 22:06:40 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 */

#ifndef lint
static char *rcsid_sms_init_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_init.c,v 1.1 1987-05-31 22:06:40 wesommer Exp $";
#endif lint

#include "sms_private.h"

int sms_inited = 0;

sms_init()
{
	if (sms_inited) return;
	
	gdb_init();
	init_sms_err_tbl();
	init_krb_err_tbl();
	sms_inited=1;
}
