/* $Id$
 *
 * Talk to the Moira database to change a person's login shell.  The chosen
 * shell must exist.  A warning will be issued if the shell is not in
 * /etc/shells.
 *
 * chsh with no modifiers changes the shell of the user who is running
 * the program.
 *
 * If a commandline argument is given, it is taken to be the username
 * of the user whose shell is to be changed.
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include <moira_site.h>
#include <mrclient.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <errno.h>

#define argis(a, b) (!strcmp(*arg + 1, a) || !strcmp(*arg + 1, b))

RCSID("$HeadURL$ $Id$");

void usage(void);
int get_shell(int argc, char **argv, void *username);
int get_winshell(int argc, char **argv, void *username);
int get_fmodtime(int argc, char **argv, void *username);
void check_shell(char *shell);
char *mr_getusershell(void);

char *whoami;
char *username;

int main(int argc, char *argv[])
{
  int status;                   /* general purpose exit status */
  int q_argc;                   /* argc for mr_query */
  char *q_argv[U_END];          /* argv for mr_query */
  char *motd;                   /* determine Moira server status */
  int got_one = 0;              /* Have we got a shell yet? */
  int windowsflg = 0, unixflg = 0;  
  char **arg = argv;
  char *server = NULL, *shell = NULL;

  if ((whoami = strrchr(argv[0], '/')) == NULL)
    whoami = argv[0];
  else
    whoami++;

  /* parse our command line options */
  while (++arg - argv < argc)
    {
      if (**arg == '-')
	{
	  if (argis("w", "winshell"))
	    windowsflg++;
	  else if (argis("u", "unixshell"))
	    unixflg++;
	  else if (argis("s", "shell"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
		  shell = *arg;
		}
	      else
		usage();
	    }
	  else if (argis("db", "database"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
		  server = *arg;
		}
	      else
		usage();
	    }
	}
      else if (username == NULL)
	username = *arg;
      else
	usage();
    }

  if (!username)
    {
      username = mrcl_krb_user();
      if (!username)
	exit(1);
    }

  if (!unixflg && !windowsflg)
    unixflg++;

  if (unixflg && windowsflg)
    usage();
  
  if (mrcl_connect(server, "chsh", -1, 1) != MRCL_SUCCESS)
    exit(1);

  /* First, do an access check */

  q_argv[USH_NAME] = username;
  q_argv[USH_SHELL] = "junk";
  q_argc = USH_END;

  if ((status = mr_access("update_user_shell", q_argc, q_argv)))
    {
      com_err(whoami, status, "; shell not\nchanged.");
      exit(2);
    }

  printf("Changing login shell for %s.\n", username);

  /* Display current information */
  
  q_argv[NAME] = username;
  q_argc = NAME + 1;
  
  if ((status = mr_query("get_finger_by_login", q_argc, q_argv,
			 get_fmodtime, username)))
    {
      com_err(whoami, status, "while getting user information.");
      exit(2);
    }

  if (unixflg)
    {
      if ((status = mr_query("get_user_account_by_login", q_argc, q_argv,
			     get_shell, username)))
	{
	  com_err(whoami, status, "while getting user information.");
	  exit(2);
	}


      if (!shell)
	{
	  /* Ask for new shell */
	  while (!got_one)
	    {
	      shell = malloc(MAXPATHLEN);
	      if (!shell)
		exit(1);
	      printf("New shell: ");
	      if (!fgets(shell, MAXPATHLEN, stdin))
		exit(0);
	      got_one = (strlen(shell) > 1);
	    }

	  shell[strlen(shell) - 1] = 0; /* trim newline */
	}

      /* Make sure we have a valid shell.  This routine could exit */
      check_shell(shell);
      
      /* Change shell */
      
      printf("Changing shell to %s...\n", shell);
      
      q_argv[USH_NAME] = username;
      q_argv[USH_SHELL] = shell;
      q_argc = USH_END;
      if ((status = mr_query("update_user_shell", q_argc, q_argv, NULL, NULL)))
	{
	  com_err(whoami, status, "while changing shell.");
	  exit(2);
	}
      
      printf("Shell successfully changed.\n");
    }
  else if (windowsflg)
    {
      if ((status = mr_query("get_user_account_by_login", q_argc, q_argv,
			     get_winshell, username)))
	{
	  com_err(whoami, status, "while getting user information.");
	  exit(2);
	}

      if (!shell)
	{
	  /* Ask for new Windows shell */
	  while(!got_one)
	    {
	      shell = malloc(MAXPATHLEN);
	      if (!shell)
		exit(1);
	      printf("New Windows shell: ");
	      if (!fgets(shell, MAXPATHLEN, stdin))
		exit(0);
	      got_one = (strlen(shell) > 1);
	    }
      
	  shell[strlen(shell) - 1] = 0; /* strip off newline */
	}

      /* Change shell */
      
      printf("Changing Windows shell to %s...\n", shell);
      
      q_argv[USH_NAME] = username;
      q_argv[USH_SHELL] = shell;
      q_argc = USH_END;
      if ((status = mr_query("update_user_windows_shell", q_argc, q_argv,
			     NULL, NULL)))
	{
	  com_err(whoami, status, "while changing Windows shell.");
	  exit(2);
	}
      
      printf("Windows shell successfully changed.\n");
    }
     
  mr_disconnect();
      
  return 0;
}

int get_shell(int argc, char **argv, void *username)
{
  /* We'll just take the first information we get since login names
     cannot be duplicated in the database. */

  if (argc < U_END || strcmp(argv[U_NAME], username))
    {
      fprintf(stderr, "Some internal error has occurred.  Try again.\n");
      exit(3);
    }

  printf("Current shell for %s is %s.\n", (char *)username, argv[U_SHELL]);

  return MR_ABORT;		/* Don't pay attention to other matches. */
}

int get_winshell(int argc, char **argv, void *username)
{
  /* We'll just take the first information we get since login names
     cannot be duplicated in the database. */

  if (argc < U_END || strcmp(argv[U_NAME], username))
    {
      fprintf(stderr, "Some internal error has occurred.  Try again.\n");
      exit(3);
    }

  printf("Current Windows shell for %s is %s.\n", (char *)username, 
	 argv[U_WINCONSOLESHELL]);

  return MR_ABORT;		/* Don't pay attention to other matches. */
}

int get_fmodtime(int argc, char **argv, void *username)
{
  /* We'll just take the first information we get since login names
     cannot be duplicated in the database. */

  if (argc < F_END || strcmp(argv[F_NAME], username))
    {
      fprintf(stderr, "Some internal error has occurred.  Try again.\n");
      exit(3);
    }

  printf("Finger information last changed on %s\n", argv[F_MODTIME]);
  printf("by user %s with %s.\n", argv[F_MODBY], argv[F_MODWITH]);

  return MR_ABORT;		/* Don't pay attention to other matches. */
}

void check_shell(char *shell)
{
  char *valid_shell;
  int ok = 0;

  while ((valid_shell = (char *)mr_getusershell()))
    {
      if (!strcmp(shell, valid_shell))
	{
	  ok = 1;
	  break;
	}
      else if (!strcmp(shell, 1 + strrchr(valid_shell, '/')))
	{
	  ok = 1;
	  strcpy(shell, valid_shell);
	  break;
	}
    }

  if (!ok)
    {
      if (shell[0] != '/')
	{
	  fprintf(stderr, "%s is not a standard shell.  ", shell);
	  fprintf(stderr, "You may choose to use a nonstandard\n");
	  fprintf(stderr, "shell, but you must specify its complete ");
	  fprintf(stderr, "path name.\n");
	  exit(2);
	}
      else if (access(shell, X_OK))
	{
	  fprintf(stderr, "%s is not available.\n", shell);
	  exit(2);
	}
      else
	{
	  printf("%s exists but is an unusual choice.\n", shell);
	  printf("Try again if it is not what you wanted.\n");
	}
    }
}

void usage(void)
{
  fprintf(stderr, "Usage: %s [-w|-u] [-s shell] [user]\n", whoami);
  exit(1);
}

char *mr_getusershell(void)
{
  static FILE *shells = NULL;

  /* In a sane universe, no shell will have a length longer than
   * MAXPATHLEN. If any line in /etc/shells does, we'll lose, but
   * not much. shrug.
   */
  static char buf[MAXPATHLEN];
  char *p;

  if (!shells)
    {
      shells = fopen(SHELLS_LIST, "r");
      if (!shells && errno == ENOENT)
	shells = fopen("/etc/shells", "r");
      if (!shells)
	{
	  fprintf(stderr, "%s: Can't open /etc/shells. Unable to determine if "
		  "this is a normal shell.\n\n", whoami);
	  return NULL;
	}
    }

  while (1)
    {
      if (!fgets(buf, sizeof(buf), shells))
	{
	  fclose(shells);
	  shells = NULL;
	  return NULL;
	}

      if (buf[0] != '/')
	continue;

      p = strchr(buf, '\n');
      if (p)
	*p = '\0';

      return buf;
    }
}
