/* $Id: chpobox.c,v 1.21 1998-03-10 21:22:42 danw Exp $
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

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/chpobox.c,v 1.21 1998-03-10 21:22:42 danw Exp $");

int get_pobox(int argc, char **argv, void *callarg);
char *potype(char *machine);
int check_match(int argc, char **argv, void *callback);
int check_match3(int argc, char **argv, void *callback);
int usage(void);

char *whoami;

static int match;

int main(int argc, char *argv[])
{
  struct passwd *pwd;
  char *mrarg[3], buf[BUFSIZ];
  char *address, *uname, *machine, *motd;
  uid_t u;
  int c, setflag, prevpop, usageflag, status;

  extern int optind;
  extern char *optarg;

  c = setflag = prevpop = usageflag = 0;
  address = uname = NULL;
  u = getuid();

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
	  usageflag++;
	else
	  {
	    setflag++;
	    strcpy(buf, optarg);
	    address = buf;
	  }
	break;
      case 'p':
	if (setflag)
	  usageflag++;
	else
	  prevpop++;
	break;
      case 'u':
	uname = strdup(optarg);
	break;
      default:
	usageflag++;
	break;
      }
  if (argc == 2 && optind == 1 && !uname)
    uname = argv[optind++];

  if (usageflag || optind != argc)
    usage();

  if (!uname)
    {
      if (!(uname = getlogin()))
	usage();

      if (uname[0] == '\0')
	{
	  pwd = getpwuid(u);
	  strcpy(uname, pwd->pw_name);
	}
    }
  mrarg[0] = uname;

  status = mr_connect(NULL);
  if (status)
    {
      com_err(whoami, status, " while connecting to Moira");
      exit(1);
    }

  status = mr_motd(&motd);
  if (status)
    {
      mr_disconnect();
      com_err(whoami, status, " unable to check server status");
      exit(1);
    }
  if (motd)
    {
      fprintf(stderr, "The Moira server is currently unavailable:\n%s\n",
	      motd);
      mr_disconnect();
      exit(1);
    }

  status = mr_auth("chpobox");
  if (status)
    {
      com_err(whoami, status, " while authenticating -- "
	      "run \"kinit\" and try again.");
      mr_disconnect();
      exit(1);
    }

  if (setflag)
    {
      /* Address is of form user@host.  Split it up. */
      if (!address)
	{
	  fprintf(stderr, "%s: no address was specified.\n", whoami);
	  goto show;
	}
      machine = strchr(address, '@');
      if (machine)
	{
	  *machine++ = '\0';		/* get rid of the @ sign */
	  machine = strtrim(machine);	/* get rid of whitespace */
	}
      else
	{
	  fprintf(stderr, "%s: no at sign (@) in address \"%s\"\n",
		  whoami, address);
	  goto show;
	}
      mrarg[2] = canonicalize_hostname(strdup(machine));
      mrarg[1] = potype(mrarg[2]);
      if (!strcmp(mrarg[1], "POP"))
	{
	  if (strcmp(address, uname))
	    {
	      fprintf(stderr,
		      "%s: the name on the POP box must match the username\n",
		      whoami);
	      goto show;
	    }
	}
      else if (!strcmp(mrarg[1], "LOCAL"))
	{
	  strcat(address, "@");
	  strcat(address, mrarg[2]);
	  mrarg[2] = address;
	  if ((address = strchr(address, '@')) &&
	      (address = strchr(address, '.')))
	    *address = 0;
	  strcat(mrarg[2], ".LOCAL");
	  mrarg[1] = "SMTP";
	}
      else if (!strcmp(mrarg[1], "MAILHUB"))
	{
	  if (!strcmp(address, uname))
	    {
	      fprintf(stderr,
		      "Error: this will set a mail forwarding loop.\n");
	      fprintf(stderr,
		      "Use \"%s -p\" to set a local post office server.\n",
		      whoami);
	      exit(1);
	    }
	  fprintf(stderr, "Error: \"%s@%s\" is a local mail address.\n",
		  address, machine);
	  fprintf(stderr, "Your mail drop must be on a post office server "
		  "or an external mail address.\n");
	  exit(1);
	}
      else if (mrarg[1])
	{
	  if (*machine != '"' && strcasecmp(mrarg[2], machine))
	    {
	      fprintf(stderr, "Warning: hostname %s canonicalized to %s\n",
		      machine, mrarg[2]);
	    }
	  strcat(address, "@");
	  strcat(address, mrarg[2]);
	  mrarg[2] = address;
	} else
	  goto show;
      status = mr_query("set_pobox", 3, mrarg, NULL, NULL);
      if (status)
	com_err(whoami, status,
		" while setting pobox for %s to type %s, box %s",
		mrarg[0], mrarg[1], mrarg[2]);
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

/*
 * given a canonicalized machine name, ask the Moira server if it is of type
 * pop, or of type local -- if neither, we assume that it's of type foreign.
 */
char *potype(char *machine)
{
  char *service[1], *argv[3];
  int status;

  match = 0;
  service[0] = "POP";
  status = mr_query("get_server_locations", 1, service, check_match, machine);
  if (status && (status != MR_NO_MATCH))
    {
      com_err(whoami, status, " while reading list of POP servers");
      return NULL;
    }
  if (match)
    return "POP";

  service[0] = "LOCAL";
  status = mr_query("get_server_locations", 1, service, check_match, machine);
  if (status && (status != MR_NO_MATCH))
    {
      com_err(whoami, status, " while reading list of LOCAL servers");
      return NULL;
    }
  if (match)
    return "LOCAL";

  argv[0] = "mailhub";
  argv[1] = "TYPE";
  argv[2] = "*";
  status = mr_query("get_alias", 3, argv, check_match3, machine);
  if (status && (status != MR_NO_MATCH))
    {
      com_err(whoami, status, " while reading list of MAILHUB servers");
      return NULL;
    }
  if (match)
    return "MAILHUB";
  else
    return "SMTP";
}

int check_match(int argc, char **argv, void *callback)
{
  if (match)
    return 0;

  if (!strcasecmp(argv[1], callback))
    match = 1;

  return 0;
}

int check_match3(int argc, char **argv, void *callback)
{
  if (match)
    return 0;

  if (!strcasecmp(argv[2], callback))
    match = 1;

  return 0;
}

int usage(void)
{
  fprintf(stderr, "Usage: %s [-s address] [-p] [-u user]\n", whoami);
  exit(1);
}
