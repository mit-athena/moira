/* $Id: chpobox.c,v 1.24 1999-05-13 18:55:17 danw Exp $
 *
 * Talk to the Moira database to change a person's home mail machine. This may
 * be an Athena machine, or a completely arbitrary address.
 *
 * chpobox with no modifiers reports the current mailbox.
 *
 * chpobox -s [address] means set the mailbox to this address.
 *
 * chpobox -p restores the pobox to a previous POP box, if there was one.
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

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/chpobox.c,v 1.24 1999-05-13 18:55:17 danw Exp $");

int get_pobox(int argc, char **argv, void *callarg);
void usage(void);

char *whoami;

static int match;

int main(int argc, char *argv[])
{
  struct passwd *pwd;
  char *mrarg[3];
  char *address, *uname;
  int c, setflag, prevpop, status;

  c = setflag = prevpop = 0;
  address = uname = NULL;

  if ((whoami = strrchr(argv[0], '/')) == NULL)
    whoami = argv[0];
  else
    whoami++;

  if (argc > 5)
    usage();

  while ((c = getopt(argc, argv, "s:pu:")) != -1)
    switch (c)
      {
      case 's':
	if (prevpop)
	  usage();
	else
	  {
	    setflag++;
	    address = optarg;
	  }
	break;
      case 'p':
	if (setflag)
	  usage();
	else
	  prevpop++;
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

  if (optind != argc)
    usage();

  if (!uname)
    {
      uname = mrcl_krb_user();
      if (!uname)
	exit(1);
    }
  mrarg[0] = uname;

  if (mrcl_connect(NULL, "chpobox", 1) != MRCL_SUCCESS)
    exit(1);

  if (setflag)
    {
      char *addr;
      if (mrcl_validate_pobox_smtp(uname, address, &addr) != MRCL_SUCCESS)
	{
	  printf("\n");
	  goto show;
	}
      mrarg[1] = "SMTP";
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
  else if (prevpop)
    {
      status = mr_query("set_pobox_pop", 1, mrarg, NULL, NULL);
      if (status == MR_MACHINE)
	{
	  fprintf(stderr,
		  "Moira has no record of a previous POP box for %s\n", uname);
	}
      else if (status != 0)
	com_err(whoami, status, " while setting pobox");
    }

  /*
   * get current box
   */
show:
  status = mr_query("get_pobox", 1, mrarg, get_pobox, NULL);
  if (status == MR_NO_MATCH)
    printf("User %s has no pobox.\n", uname);
  else if (status != 0)
    com_err(whoami, status, " while retrieving current mailbox");
  mr_disconnect();
  exit(0);
}


/*
 * get_pobox gets all your poboxes and displays them.
 */

int get_pobox(int argc, char **argv, void *callarg)
{
  if (!strcmp(argv[1], "POP"))
    printf("User %s, Type %s, Box: %s@%s\n",
	   argv[0], argv[1], argv[0], argv[2]);
  else
    printf("User %s, Type %s, Box: %s\n",
	   argv[0], argv[1], argv[2]);
  printf("  Modified by %s on %s with %s\n", argv[4], argv[3], argv[5]);
  return 0;
}

void usage(void)
{
  fprintf(stderr, "Usage: %s [-s address] [-p] [-u user]\n", whoami);
  exit(1);
}
