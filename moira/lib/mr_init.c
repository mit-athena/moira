/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_init.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_init.c,v 1.3 1989-06-01 21:44:22 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 */

#ifndef lint
static char *rcsid_sms_init_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_init.c,v 1.3 1989-06-01 21:44:22 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include "sms_private.h"

int sms_inited = 0;

/* the reference to link_against_the_moira_version_of_gdb is to make
 * sure that this is built with the proper libraries.
 */
sms_init()
{
	extern int link_against_the_moira_version_of_gdb;
	if (sms_inited) return;
	
	gdb_init();
	init_sms_err_tbl();
	init_krb_err_tbl();
	link_against_the_moira_version_of_gdb = 0;
	sms_inited=1;
}
