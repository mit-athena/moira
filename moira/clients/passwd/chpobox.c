/* $Id: chpobox.c,v 1.29 2000-03-15 22:44:12 rbasch Exp $
 *
 * Talk to the Moira database to change a person's home mail machine. This may
 * be an Athena machine, or a completely arbitrary address.
 *
 * chpobox with no modifiers reports the current mailbox.
 *
 * chpobox -s address means set the mailbox to this address.
 *
 * chpobox -p restores the pobox to a previous POP/IMAP box, if there was one.
 *
 * chpobox -S address means split mail between POP/IMAP and SMTP
 *
 * chpobox -u [user] is needed if you are logged in as one user, but
 * are trying to change the email address of another.  You must have
 * Kerberos tickets as the person whose address you are trying to
 * change, or the attempt will fail.
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include <moira_site.h>
#include <mrclient.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/chpobox.c,v 1.29 2000-03-15 22:44:12 rbasch Exp $");

int get_pobox(int argc, char **argv, void *callarg);
void usage(void);

char *whoami;

static int match;

int main(int argc, char *argv[])
{
  char *mrarg[3];
  char *address, *uname;
  int c, setflag, splitflag, prevflag, status;

  setflag = splitflag = prevflag = 0;
  address = uname = NULL;

  if ((whoami = strrchr(argv[0], '/')) == NULL)
    whoami = argv[0];
  else
    whoami++;

  if (argc > 5)
    usage();

  while ((c = getopt(argc, argv, "s:S:pu:")) != -1)
    switch (c)
      {
      case 's':
	setflag++;
	address = optarg;
	break;

      case 'S':
	splitflag++;
	address = optarg;
	break;

      case 'p':
	prevflag++;
	break;

      case 'u':
	uname = optarg;
	break;

      default:
	usage();
	break;
      }
  if (argc == 2 && optind == 1 && !uname)
    uname = argv[optind++];

  if (optind != argc || (prevflag + splitflag + setflag > 1))
    usage();

  if (!uname)
    {
      uname = mrcl_krb_user();
      if (!uname)
	exit(1);
    }
  mrarg[0] = uname;

  if (mrcl_connect(NULL, "chpobox", 2, 1) != MRCL_SUCCESS)
    exit(1);

  if (setflag || splitflag)
    {
      char *addr;
      if (mrcl_validate_pobox_smtp(uname, address, &addr) != MRCL_SUCCESS)
	{
	  printf("\n");
	  goto show;
	}
      mrarg[1] = setflag ? "SMTP" : "SPLIT";
      mrarg[2] = addr;
      status = mr_query("set_pobox", 3, mrarg, NULL, NULL);
      free(addr);
      if (status)
	{
	  com_err(whoami, status,
		  "while setting pobox for %s to type %s, box %s",
		  mrarg[0], mrarg[1], mrarg[2]);
	}
    }
  else if (prevflag)
    {
      status = mr_query("set_pobox_pop", 1, mrarg, NULL, NULL);
      if (status == MR_MACHINE)
	{
	  fprintf(stderr,
		  "Moira has no record of a previous POP box for %s\n", uname);
	}
      else if (status != 0)
	com_err(whoami, status, "while setting pobox");
    }

  /*
   * get current box
   */
show:
  status = mr_query("get_pobox", 1, mrarg, get_pobox, NULL);
  if (status == MR_NO_MATCH)
    printf("User %s has no pobox.\n", uname);
  else if (status != 0)
    com_err(whoami, status, "while retrieving current mailbox");
  mr_disconnect();
  exit(0);
}


/*
 * get_pobox gets all your poboxes and displays them.
 */

int get_pobox(int argc, char **argv, void *callarg)
{
  if (!strcmp(argv[1], "SMTP"))
    {
      printf("User %s, Type %s, Box: %s\n",
	     argv[0], argv[1], argv[2]);
    }
  else if (argc == 7)
    {
      printf("User %s, Type %s, Box: %s (%s)\n",
	   argv[0], argv[1], argv[2], argv[3]);
    }
  else
    {
      printf("User %s, Type %s, Box: %s@%s\n",
	     argv[0], argv[1], argv[0], argv[2]);
    }
  printf("  Modified by %s on %s with %s\n",
	 argv[argc - 2], argv[argc - 3], argv[argc - 1]);
  return 0;
}

void usage(void)
{
  fprintf(stderr, "Usage: %s [-s address] [-p] [-u user]\n", whoami);
  exit(1);
}
