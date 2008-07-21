/* $Id: chpobox.c,v 1.6 2008-05-16 16:49:38 zacheiss Exp $
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

#define argis(a, b) (!strcmp(*arg + 1, a) || !strcmp(*arg + 1, b))

RCSID("$Header: /afs/athena.mit.edu/astaff/project/moiradev/repository/moira/clients/chpobox/chpobox.c,v 1.6 2008-05-16 16:49:38 zacheiss Exp $");

int get_pobox(int argc, char **argv, void *callarg);
void usage(void);

char *whoami;

static int match;

int main(int argc, char *argv[])
{
  char *mrarg[3];
  char *address, *uname;
  char **arg = argv;
  char *server = NULL;
  int c, setflag, splitflag, prevflag, status;

  setflag = splitflag = prevflag = 0;
  address = uname = NULL;

  if ((whoami = strrchr(argv[0], '/')) == NULL)
    whoami = argv[0];
  else
    whoami++;

  if (argc > 5)
    usage();

  while (++arg - argv < argc)
    {
      if (**arg == '-')
	{
	  if (argis("s", "set")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      setflag++;
	      address = *arg;
	    } else
	      usage();
	  }
	  else if (argis("S", "split")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      splitflag++;
	      address = *arg;
	    } else
	      usage();
	  }
	  else if (argis("p", "previous"))
	    prevflag++;
	  else if (argis("u", "username")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      uname = *arg;
	    } else
	      usage();
	  }
	  else if (argis("db", "database")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      server = *arg;
	    } else
	      usage();
	  }
	}
      else if (uname == NULL)
	uname = *arg;
      else
	usage();
    }

  if (prevflag + splitflag + setflag > 1)
    usage();

  if (!uname)
    {
      uname = mrcl_krb_user();
      if (!uname)
	exit(1);
    }
  mrarg[0] = uname;

  if (mrcl_connect(server, "chpobox", 2, 1) != MRCL_SUCCESS)
    exit(1);

  if (setflag || splitflag)
    {
      char *addr;
      status = mrcl_validate_pobox_smtp(uname, address, &addr);
      if (mrcl_get_message())
	mrcl_com_err(whoami);
      if (status != MRCL_SUCCESS)
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
    mrcl_com_err(whoami);
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
  fprintf(stderr, "Usage: %s [-s|-S address] [-p] [-u user] [-db database]\n", whoami);
  exit(1);
}
