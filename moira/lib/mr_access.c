/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_access.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_access.c,v 1.4 1988-09-13 15:51:51 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 */

#ifndef lint
static char *rcsid_sms_access_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_access.c,v 1.4 1988-09-13 15:51:51 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include "sms_private.h"

/*
 * Check access to a named query.
 */
int sms_access(name, argc, argv)
    char *name;			/* Query name */
    int argc;			/* Arg count */
    char **argv;		/* Args */
{
    register char **nargv = (char **)malloc(sizeof(char *) * (argc+1));
    register int status = 0;
    nargv[0] = name;
    bcopy((char *)argv, (char *)(nargv+1), sizeof(char *) * argc);
    status = sms_access_internal(argc+1, nargv);
    free(nargv);
    return status;
}
/*
 * Check access to a named query, where the query name is argv[0]
 * and the arguments are the rest of argv[].
 */
int sms_access_internal(argc, argv)
    int argc;			/* Arg count */
    char **argv;		/* Args */
{
    int status;
    sms_params params_st;
    register sms_params *params = NULL;
    sms_params *reply = NULL;
    
    CHECK_CONNECTED;

    params = &params_st;
    params->sms_version_no = sending_version_no;
    params->sms_procno = SMS_ACCESS;
    params->sms_argc = argc;
    params->sms_argl = NULL;
    params->sms_argv = argv;
	
    if ((status = sms_do_call(params, &reply)) == 0)
	status = reply->sms_status;

    sms_destroy_reply(reply);

    return status;
}

/*
 * Local Variables:
 * mode: c
 * c-indent-level: 4
 * c-continued-statement-offset: 4
 * c-brace-offset: -4
 * c-argdecl-indent: 4
 * c-label-offset: -4
 * End:
 */
