/*
 * $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/incremental/ksrvtgt.c,v $
 * $Author: mar $
 *
 * Copyright 1988 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 * Get a ticket-granting-ticket given a service key file (srvtab)
 * Modifed from the regular kerberos distribution in that it accepts
 * the lifetime of the ticket as an command-line argument.
 *
 */

#include <stdio.h>
#include <sys/param.h>
#include <krb.h>
/*#include <conf.h>*/

char rcsid[] =
    "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/incremental/ksrvtgt.c,v 1.3 1990-02-15 13:16:35 mar Exp $";


void usage(argv)
char **argv;
{
    fprintf(stderr,
	    "Usage: %s name instance [-r realm] [-s srvtab] [-l lifetime]\n",
	    argv[0]);
    exit(1);
}


main(argc,argv)
    int argc;
    char **argv;
{
    char realm[REALM_SZ + 1];
    register int code;
    int i, lifetime = 1;
    char srvtab[MAXPATHLEN + 1];

    bzero(realm, sizeof(realm));
    bzero(srvtab, sizeof(srvtab));

    if (argc < 3)
      usage(argv);

    for (i = 3; i < argc; i++) {
	if (argv[i][0] != '-')
	  usage(argv);
	switch (argv[i][1]) {
	case 'r':
	    if (i + 1 >= argc)
	      usage(argv);
	    strncpy(realm, argv[i++ + 1], sizeof(realm) - 1);
	    break;
	case 's':
	    if (i + 1 >= argc)
	      usage(argv);
	    strncpy(srvtab, argv[i++ + 1], sizeof(srvtab) - 1);
	    break;
	case 'l':
	    if (i + 1 >= argc)
	      usage(argv);
	    lifetime = atoi(argv[i++ + 1]);
	    if (lifetime < 5)
	      lifetime = 1;
	    else
	      lifetime /= 5;
	    if (lifetime > 255)
	      lifetime = 255;
	    break;
	default:
	    usage();
	}
    }

    if (srvtab[0] == 0)
	(void) strcpy(srvtab, KEYFILE);

    if (realm[0] == 0)
	if (krb_get_lrealm(realm, 1) != KSUCCESS)
	    (void) strcpy(realm, KRB_REALM);

    code = krb_get_svc_in_tkt(argv[1], argv[2], realm,
			      "krbtgt", realm, lifetime, srvtab);
    if (code)
	fprintf(stderr, "%s\n", krb_err_txt[code]);
    exit(code);
}

