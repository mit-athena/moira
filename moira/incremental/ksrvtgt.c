/*
 * $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/incremental/ksrvtgt.c,v $
 * $Author: danw $
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
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <krb.h>
/*#include <conf.h>*/

char rcsid[] =
    "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/incremental/ksrvtgt.c,v 1.7 1998-02-05 22:51:20 danw Exp $";

void usage(char **argv);

int main(int argc, char **argv)
{
  char realm[REALM_SZ + 1];
  int code;
  int i, lifetime = 1;
  char srvtab[MAXPATHLEN + 1];

  memset(realm, 0, sizeof(realm));
  memset(srvtab, 0, sizeof(srvtab));

  if (argc < 3)
    usage(argv);

  for (i = 3; i < argc; i++)
    {
      if (argv[i][0] != '-')
	usage(argv);
      switch (argv[i][1])
	{
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
	  usage(argv);
	}
    }

  if (!*srvtab)
    strcpy(srvtab, KEYFILE);

  if (!*realm)
    {
      if (krb_get_lrealm(realm, 1) != KSUCCESS)
	strcpy(realm, KRB_REALM);
    }

  code = krb_get_svc_in_tkt(argv[1], argv[2], realm,
			    "krbtgt", realm, lifetime, srvtab);
  if (code)
    fprintf(stderr, "%s\n", krb_err_txt[code]);
  exit(code);
}

void usage(char **argv)
{
  fprintf(stderr,
	  "Usage: %s name instance [-r realm] [-s srvtab] [-l lifetime]\n",
	  argv[0]);
  exit(1);
}
