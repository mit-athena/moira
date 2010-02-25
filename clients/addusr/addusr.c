/* $Id: addusr.c 3979 2010-02-16 20:26:57Z zacheiss $
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
#include <mrclient.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

RCSID("$HeadURL: svn+ssh://svn.mit.edu/moira/trunk/moira/clients/addusr/addusr.c $ $Id: addusr.c 3979 2010-02-16 20:26:57Z zacheiss $");

struct owner_type {
  int type;
  char *name;
};

#define M_ANY		0
#define M_USER		1
#define M_LIST		2
#define M_KERBEROS	3
#define M_NONE		4

#ifdef ATHENA
#define DEFAULT_SHELL "/bin/athena/bash"
#else
#define DEFAULT_SHELL "/bin/bash"
#endif

#define DEFAULT_WINCONSOLESHELL "cmd"
#define DEFAULT_WINHOMEDIR "[DFS]"
#define DEFAULT_WINPROFILEDIR "[DFS]"

/* flags from command line */
char *class, *comment, *status_str, *shell, *winconsoleshell, *filename;
char *expiration;
int reg_only, reg, verbose, nodupcheck, securereg, nocaps;
struct owner_type *sponsor;

/* argument parsing macro */
#define argis(a, b) (!strcmp(*arg + 1, a) || !strcmp(*arg + 1, b))

char *whoami;
int duplicate, errors;

void usage(char **argv);
int usercheck(int argc, char **argv, void *qargv);
int get_uid(int argc, char **argv, void *qargv);
struct owner_type *parse_member(char *s);

int main(int argc, char **argv)
{
  int status, lineno;
  char **arg = argv, *qargv[U_END];
  char *p, *first, *middle, *last, *id, *login, *server;
  char buf[BUFSIZ];
  FILE *input;

  /* clear all flags & lists */
  reg_only = reg = verbose = lineno = nodupcheck = errors = securereg = nocaps = 0;
  sponsor = NULL;
  server = NULL;
  filename = "-";
  shell = DEFAULT_SHELL;
  winconsoleshell = DEFAULT_WINCONSOLESHELL;
  class = "TEMP";
  comment = "";
  expiration = "";
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
	  else if (argis("w", "winshell"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
		  winconsoleshell = *arg;
		}
	      else 
		usage(argv);
	    }
	  else if (argis("sp", "sponsor"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
		  sponsor = parse_member(*arg);
		}
	      else
		usage(argv);
	    }
	  else if (argis("e", "expiration"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
		  expiration = *arg;
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
	  else if (argis("n", "nocaps"))
	    nocaps++;
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

  /* fire up Moira */
  if (mrcl_connect(server, "addusr", 14, 1) != MRCL_SUCCESS)
    exit(2);

  qargv[U_NAME] = UNIQUE_LOGIN;
  qargv[U_UID] = UNIQUE_UID;
  qargv[U_SHELL] = shell;
  qargv[U_WINCONSOLESHELL] = winconsoleshell;
  qargv[U_WINHOMEDIR] = DEFAULT_WINHOMEDIR;
  qargv[U_WINPROFILEDIR] = DEFAULT_WINPROFILEDIR;
  qargv[U_EXPIRATION] = expiration;
  qargv[U_ALT_EMAIL] = "";
  qargv[U_ALT_PHONE] = "";
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
      first = strtrim(p);
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
      if (!nocaps)
	{
	  FixCase(qargv[U_FIRST]);
	  FixCase(qargv[U_MIDDLE]);
	  FixCase(qargv[U_LAST]);
	}
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
			  qargv[U_FIRST], qargv[U_LAST]);
		  com_err(whoami, 0, "NOT ADDING USER");
		  errors++;
		  continue;
		}
	      if (duplicate > 0)
		{
		  com_err(whoami, MR_EXISTS, "user %s %s already exists",
			  qargv[U_FIRST], qargv[U_LAST]);
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

	  if (sponsor)
	    {
	      qargv[U_SPONSOR_NAME] = sponsor->name;
	      switch (sponsor->type)
		{
		case M_ANY:
		case M_USER:
		  qargv[U_SPONSOR_TYPE] = "USER";
		  status = mr_query("add_user_account", 18, qargv, NULL, NULL);
		  if (sponsor->type != M_ANY || status != MR_USER)
		    break;
		  
		case M_LIST:
		  qargv[U_SPONSOR_TYPE] = "LIST";
		  status = mr_query("add_user_account", 18, qargv, NULL, NULL);
		  break;
		  
		case M_KERBEROS:
		  qargv[U_SPONSOR_TYPE] = "KERBEROS";
		  status = mrcl_validate_kerberos_member(qargv[U_SPONSOR_NAME],
							 &qargv[U_SPONSOR_NAME]);
		  if (mrcl_get_message())
		    mrcl_com_err(whoami);
		  if (status == MRCL_REJECT)
		exit(1);
		  status = mr_query("add_user_account", 18, qargv, NULL, NULL);
		  break;
		  
		case M_NONE:
		  qargv[U_SPONSOR_TYPE] = "NONE";
		  status = mr_query("add_user_account", 18, qargv, NULL, NULL);
		  break;
		}
	    }
	  else
	    {
	      qargv[U_SPONSOR_TYPE] = "NONE";
	      qargv[U_SPONSOR_NAME] = "NONE";
	  
	      status = mr_query("add_user_account", 18, qargv, NULL, NULL);
	    }
	  
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
	  char *gargv[2], *rargv[3], uid[10];

	  uid[0] = '\0';
	  gargv[0] = qargv[U_FIRST];
	  gargv[1] = qargv[U_LAST];
	  status = mr_query("get_user_account_by_name", 2, gargv,
			    get_uid, &uid);
	  if (status)
	    {
	      com_err(whoami, status, "while looking up uid for %s %s",
		      qargv[U_FIRST], qargv[U_LAST]);
	      errors++;
	      continue;
	    }

	  rargv[0] = uid;
	  rargv[1] = login;
	  rargv[2] = "EXCHANGE";

	  status = mr_query("register_user", 3, rargv, NULL, NULL);
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
  fprintf(stderr, "   -w | -winshell windows console shell (default %s)\n",
	  DEFAULT_WINCONSOLESHELL);
  fprintf(stderr, "   -sp | -sponsor sponsor (default NONE)\n");
  fprintf(stderr, "   -e | -expiration \"expiration date\" (default \"\")\n");
  fprintf(stderr, "   -r | -reg_only\n");
  fprintf(stderr, "   -R | -register (and add to database)\n");
  fprintf(stderr, "   -v | -verbose\n");
  fprintf(stderr, "   -d | -nodupcheck (don't check for duplicates)\n");
  fprintf(stderr, "   -n | -nocaps (don't fix capitalization of names)\n");
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

/* query callback to get uid of a just-added account */
int get_uid(int argc, char **argv, void *uidv)
{
  char *uid = uidv;

  if (uid[0] == '\0')
    strcpy(uid, argv[U_UID]);
  else
    {
      if (!strcmp(argv[U_MODWITH], "addusr"))
	strcpy(uid, argv[U_UID]);
    }

  return MR_CONT;
}

/* Parse a line of input, fetching a member.  NULL is returned if a member
 * is not found.  ';' is a comment character.
 */
struct owner_type *parse_member(char *s)
{
  struct owner_type *m;
  char *p, *lastchar;

  while (*s && isspace(*s))
    s++;
  lastchar = p = s;
  while (*p && *p != '\n' && *p != ';')
    {
      if (isprint(*p) && !isspace(*p))
	lastchar = p++;
      else
	p++;
    }
  lastchar++;
  *lastchar = '\0';
  if (p == s || strlen(s) == 0)
    return NULL;

  if (!(m = malloc(sizeof(struct owner_type))))
    return NULL;

  if ((p = strchr(s, ':')))
    {
      *p = '\0';
      m->name = ++p;
      if (!strcasecmp("user", s))
	m->type = M_USER;
      else if (!strcasecmp("list", s))
	m->type = M_LIST;
      else if (!strcasecmp("kerberos", s))
	m->type = M_KERBEROS;
      else if (!strcasecmp("none", s))
	m->type = M_NONE;
      else
	{
	  m->type = M_ANY;
	  *(--p) = ':';
	  m->name = s;
	}
      m->name = strdup(m->name);
    }
  else
    {
      m->name = strdup(s);
      m->type = strcasecmp(s, "none") ? M_ANY : M_NONE;
    }
  return m;
}
