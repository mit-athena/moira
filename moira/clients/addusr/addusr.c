/* $Id: addusr.c,v 1.12 1998-08-07 14:22:15 danw Exp $
 *
 * Program to add users en masse to the moira database
 *
 * by Mark Rosenstein, July 1992.
 *
 * Copyright (C) 1992-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include <moira_site.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/addusr/addusr.c,v 1.12 1998-08-07 14:22:15 danw Exp $");

#ifdef ATHENA
#define DEFAULT_SHELL "/bin/athena/tcsh"
#else
#define DEFAULT_SHELL "/bin/csh"
#endif

/* flags from command line */
char *class, *comment, *status_str, *shell, *filename;
int reg_only, reg, verbose, nodupcheck, securereg;

/* argument parsing macro */
#define argis(a, b) (!strcmp(*arg + 1, a) || !strcmp(*arg + 1, b))

char *whoami;
int duplicate, errors;

void usage(char **argv);
int usercheck(int argc, char **argv, void *qargv);

int main(int argc, char **argv)
{
  int status, lineno;
  char **arg = argv, *qargv[U_END];
  char *motd, *p, *first, *middle, *last, *id, *login, *server;
  char buf[BUFSIZ], idbuf[32];
  FILE *input;

  /* clear all flags & lists */
  reg_only = reg = verbose = lineno = nodupcheck = errors = securereg = 0;
  server = NULL;
  filename = "-";
  shell = DEFAULT_SHELL;
  class = "TEMP";
  comment = "";
  status_str = "0";

  whoami = argv[0];

  /* parse args */
  while (++arg - argv < argc)
    {
      if  (**arg == '-')
	{
	  if (argis("c", "class"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
		  class = *arg;
		}
	      else
		usage(argv);
	    }
	  else if (argis("C", "comment"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
		  comment = *arg;
		}
	      else
		usage(argv);
	    }
	  else if (argis("s", "status"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
		  status_str = *arg;
		}
	      else
		usage(argv);
	    }
	  else if (argis("h", "shell"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
		  shell = *arg;
		}
	      else
		usage(argv);
	    }
	  else if (argis("6", "secure"))
	    securereg++;
	  else if (argis("r", "reg_only"))
	    reg_only++;
	  else if (argis("R", "register"))
	    reg++;
	  else if (argis("f", "file"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
		  filename = *arg;
		}
	      else
		usage(argv);
	    }
	  else if (argis("v", "verbose"))
	    verbose++;
	  else if (argis("d", "nodupcheck"))
	    nodupcheck++;
	  else if (argis("S", "server") || argis("db", "database"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
		  server = *arg;
		}
	      else
		usage(argv);
	    }
	  else
	    usage(argv);
	}
      else
	usage(argv);
    }

  if (!strcmp(filename, "-"))
    input = stdin;
  else
    {
      input = fopen(filename, "r");
      if (!input)
	{
	  com_err(whoami, errno, "opening input file %s", filename);
	  exit(2);
	}
    }

  if (!reg_only)
    {
      /* fire up Moira */
      if ((status = mr_connect(server)))
	{
	  com_err(whoami, status, "unable to connect to the Moira server");
	  exit(2);
	}
      if ((status = mr_motd(&motd)))
	{
	  com_err(whoami, status, "unable to check server status");
	  exit(2);
	}
      if (motd)
	{
	  fprintf(stderr, "The Moira server is currently unavailable:\n%s\n",
		  motd);
	  mr_disconnect();
	  exit(2);
	}

      if ((status = mr_auth("addusr")))
	{
	  if (status == MR_USER_AUTH)
	    com_err(whoami, status, "");
	  else
	    {
	      com_err(whoami, status, "unable to authenticate to Moira");
	      exit(2);
	    }
	}
    }

  qargv[U_NAME] = UNIQUE_LOGIN;
  qargv[U_UID] = UNIQUE_UID;
  qargv[U_SHELL] = shell;
  qargv[U_STATE] = status_str;
  qargv[U_CLASS] = class;
  qargv[U_COMMENT] = comment;
  qargv[U_SIGNATURE] = "";
  qargv[U_SECURE] = securereg ? "1" : "0";
  while (fgets(buf, BUFSIZ, input))
    {
      /* throw away terminating newline */
      p = &buf[strlen(buf) - 1];
      if (*p == '\n')
	*p = '\0';
      lineno++;
      if (strlen(buf) == 0)
	continue;
      /* Last name is first thing on line */
      last = buf;
      /* First name follows a comma */
      p = strchr(last, ',');
      if (!p)
	{
	  com_err(whoami, MR_BAD_CHAR, "Missing comma on line %d", lineno);
	  errors++;
	  continue;
	}
      *p++ = '\0';
      first = p;
      while (*p)		/* find end-of-line */
	p++;
      if (reg_only || reg)
	{
	  while (!isspace(*p))
	    p--;
	  if (p <= first)
	    {
	      com_err(whoami, 0, "Missing login on line %d", lineno);
	      errors++;
	      continue;
	    }
	  login = strtrim(&p[1]);
	  *p-- = '\0';
	}
      else
	{
	  /* ID is last thing on line */
	  p--;
	}
      while (isspace(*p))
	p--;
      while (!isspace(*p))
	p--;
      if (p <= first)
	{
	  com_err(whoami, 0, "Missing ID on line %d", lineno);
	  errors++;
	  continue;
	}
      id = &p[1];
      *p-- = '\0';
      /* If something between first name & ID, it's middle name */
      while (isspace(*p))
	p--;
      while (!isspace(*p))
	p--;
      if (p <= first)
	middle = "";
      else
	{
	  middle = &p[1];
	  *p = '\0';
	}
      qargv[U_FIRST] = strtrim(first);
      qargv[U_MIDDLE] = strtrim(middle);
      qargv[U_LAST] = strtrim(last);
      qargv[U_MITID] = strtrim(id);
      FixCase(qargv[U_FIRST]);
      FixCase(qargv[U_MIDDLE]);
      FixCase(qargv[U_LAST]);
      RemoveHyphens(qargv[U_MITID]);
      if (!reg_only)
	{
	  if (!nodupcheck)
	    {
	      char *dargv[2];

	      dargv[0] = qargv[U_FIRST];
	      dargv[1] = qargv[U_LAST];
	      duplicate = 0;
	      status = mr_query("get_user_account_by_name", 2, dargv,
				usercheck, qargv);
	      if (status && status != MR_NO_MATCH)
		{
		  com_err(whoami, status,
			  "checking to see if user %s %s already exists",
			  qargv[0], qargv[1]);
		  com_err(whoami, 0, "NOT ADDING USER");
		  errors++;
		  continue;
		}
	      if (duplicate > 0)
		{
		  com_err(whoami, MR_EXISTS, "user %s %s already exists",
			  qargv[0], qargv[1]);
		  com_err(whoami, 0, "NOT ADDING USER");
		  errors++;
		  continue;
		}
	      else if (duplicate < 0)
		{
		  com_err(whoami, MR_EXISTS,
			  "user %s %s already exists with different ID number",
			  qargv[U_FIRST], qargv[U_LAST]);
		  com_err(whoami, 0, "ADDING user anyway");
		}
	    }
	  status = mr_query("add_user_account", U_SECURE + 1, qargv,
			    NULL, NULL);
	  if (status)
	    {
	      com_err(whoami, status, "adding user %s %s", first, last);
	      errors++;
	    }
	  else if (verbose)
	    {
	      printf("Added user %s %s %s (%s)\n", qargv[U_FIRST],
		     qargv[U_MIDDLE], qargv[U_LAST], qargv[U_MITID]);
	    }
	}
      if (reg || reg_only)
	{
	  char *rargv[3];

	  rargv[0] = qargv[U_MITID];
	  rargv[1] = login;
	  rargv[2] = "0";

	  status = mr_query("register_user", 2, rargv, NULL, NULL);
	  if (status)
	    {
	      com_err(whoami, status, "while registering (login) %s %s",
		      qargv[U_FIRST], qargv[U_LAST]);
	      errors++;
	      continue;
	    }
	  else if (verbose)
	    {
	      printf("Registered user %s %s as %s\n", qargv[U_FIRST],
		     qargv[U_LAST], login);
	    }
	}
    }

  exit(errors);
}


void usage(char **argv)
{
  fprintf(stderr, "Usage: %s [options]\n", argv[0]);
  fprintf(stderr, "Options are\n");
  fprintf(stderr, "   -f | -file filename (default STDIN)\n");
  fprintf(stderr, "   -c | -class class (default TEMP)\n");
  fprintf(stderr, "   -C | -comment \"comment\" (default \"\")\n");
  fprintf(stderr, "   -s | -status status (default 0)\n");
  fprintf(stderr, "   -h | -shell shell (default %s)\n", DEFAULT_SHELL);
  fprintf(stderr, "   -r | -reg_only\n");
  fprintf(stderr, "   -R | -register (and add to database)\n");
  fprintf(stderr, "   -v | -verbose\n");
  fprintf(stderr, "   -d | -nodupcheck (don't check for duplicates)\n");
  fprintf(stderr, "   -db | -database host:port\n");
  exit(1);
}


/* query callback routine to check for duplicate users */

int usercheck(int argc, char **argv, void *qargv)
{
  if (!strcmp(argv[U_MITID], ((char **)qargv)[U_MITID]))
    duplicate++;
  else
    duplicate--;

  return MR_CONT;
}
