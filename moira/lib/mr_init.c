/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_init.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_init.c,v 1.2 1988-09-13 15:52:25 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 */

#ifndef lint
static char *rcsid_sms_init_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_init.c,v 1.2 1988-09-13 15:52:25 mar Exp $";
#endif lint

#include <mit-copyright.h>
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
