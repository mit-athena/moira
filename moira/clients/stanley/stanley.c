/*
 * Command line oriented Moira users tool.
 *
 * zacheiss@mit.edu, September 2001
 *
 * Inspired by blanche
 *
 * Copyright (C) 2000, 2001 by the Massachusetts Institute of Technology.
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
#include <stdlib.h>
#include <string.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/stanley/stanley.c,v 1.5 2001-09-26 04:49:24 zacheiss Exp $");

struct string_list {
  char *string;
  struct string_list *next;
};

/* argument parsing macro */
#define argis(a, b) (!strcmp(*arg + 1, a) || !strcmp(*arg + 1, b))

/* flags from command line */
int info_flag, update_flag, create_flag, deact_flag, reg_flag;
int list_res_flag, update_res_flag, unformatted_flag, verbose, noauth;

struct string_list *reservation_add_queue, *reservation_remove_queue;

char *username, *whoami;

char *newlogin, *uid, *shell, *winshell, *last, *first, *middle, *u_status;
char *clearid, *class, *comment, *secure;

static char *states[] = {
  "Registerable (0)",
  "Active (1)",
  "Half Registered (2)",
  "Deleted (3)",
  "Not registerable (4)",
  "Enrolled/Registerable (5)",
  "Enrolled/Not Registerable (6)",
  "Half Enrolled (7)"
};

static char *UserState(int state)
{
  static char buf[BUFSIZ];

  if (state < 0 || state >= US_END)
    {
      sprintf(buf, "Unknown (%d)", state);
      return buf;
    }
  return states[state];
}

void usage(char **argv);
int save_query_info(int argc, char **argv, void *hint);
int show_reservations(int argc, char **argv, void *hint);
void show_user_info(char **argv);
void show_user_info_unformatted(char **argv);
struct string_list *add_to_string_list(struct string_list *old_list, char *s);
int wrap_mr_query(char *handle, int argc, char **argv,
		  int (*callback)(int, char **, void *), void *callarg);
void print_query(char *query_name, int argc, char **argv);

int main(int argc, char **argv)
{
  int status;
  char **arg = argv;
  char *server = NULL;

  /* clear all flags & lists */
  info_flag = update_flag = create_flag = deact_flag = reg_flag = 0;
  list_res_flag = update_res_flag = unformatted_flag = verbose = noauth = 0;
  newlogin = uid = shell = winshell = last = first = middle = NULL;
  u_status = clearid = class = comment = secure = NULL;
  reservation_add_queue = reservation_remove_queue = NULL;
  whoami = argv[0];

  /* parse args */
  while (++arg - argv < argc)
    {
      if (**arg == '-')
	{
	  if (argis("i", "info"))
	    info_flag++;
	  else if (argis("C", "create"))
	    create_flag++;
	  else if (argis("D", "deact"))
	    deact_flag++;
	  else if (argis("r", "register"))
	    reg_flag++;
	  else if (argis("R", "rename")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      update_flag++;
	      newlogin = *arg;
	    } else
	      usage(argv);
	  }
	  else if (argis("U", "uid")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      update_flag++;
	      uid = *arg;
	    } else
	      usage(argv);
	  }
	  else if (argis("s", "shell")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      update_flag++;
	      shell = *arg;
	    } else
	      usage(argv);
	  }
	  else if (argis("w", "winshell")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      update_flag++;
	      winshell = *arg;
	    } else
	      usage(argv);
	  }
	  else if (argis("L", "last")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      update_flag++;
	      last = *arg;
	    } else
	      usage(argv);
	  }
	  else if (argis("F", "first")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      update_flag++;
	      first = *arg;
	    } else
	      usage(argv);
	  }
	  else if (argis("M", "middle")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      update_flag++;
	      middle = *arg;
	    } else
	      usage(argv);
	  }
	  else if (argis("S", "status")) {
	    if (arg - argv < argc - 1) { 
	      int i;
	      int len;

	      arg++;
	      update_flag++;
	      u_status = *arg;
	      len = strlen(u_status);
	      for (i = 0; i < len; i++) {
		if (!isdigit(u_status[i])) {
		  printf("Error:  status code %s is not numeric.\n", u_status);
		  exit(1);
		}
	      }
	    } else
	      usage(argv);
	  }
	  else if (argis("I", "clearid")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      update_flag++;
	      clearid = *arg;
	    } else
	      usage(argv);
	  }
	  else if (argis("cl", "class")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      update_flag++;
	      class = *arg;
	    } else
	      usage(argv);
	  }
	  else if (argis("c", "comment")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      update_flag++;
	      comment = *arg;
	    } else
	      usage(argv);
	  }
	  else if (argis("6", "secure")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      update_flag++;
	      secure = *arg;
	    } else
	      usage(argv);
	  }
	  else if (argis("ar", "addreservation")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      reservation_add_queue = add_to_string_list(reservation_add_queue,
							 *arg);
	    } else
	      usage(argv);
	    update_res_flag++;
	  }
	  else if (argis("dr", "deletereservation")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      reservation_remove_queue = add_to_string_list(reservation_remove_queue, *arg);
	    } else
	      usage(argv);
	    update_res_flag++;
	  }
	  else if (argis("lr", "listreservation"))
	    list_res_flag++;
	  else if (argis("u", "unformatted"))
	    unformatted_flag++;
	  else if (argis("n", "noauth"))
	    noauth++;
	  else if (argis("v", "verbose"))
	    verbose++;
	  else if (argis("db", "database"))
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
      else if (username == NULL)
	username = *arg;
      else
	usage(argv);
    }
  if (username == NULL && !create_flag)
    usage(argv);

  /* default to info_flag if nothing else was specified */
  if(!(info_flag       || update_flag || create_flag   || \
       deact_flag      || reg_flag    || list_res_flag || \
       update_res_flag)) {
    info_flag++;
  }

  /* fire up Moira */
  status = mrcl_connect(server, "stanley", 8, !noauth);
  if (status == MRCL_AUTH_ERROR)
    {
      com_err(whoami, 0, "Try the -noauth flag if you don't "
	      "need authentication.");
    }
  if (status)
    exit(2);

  /* create if needed */
  if (create_flag)
    {
      char *argv[20];
      int cnt;

      for (cnt = 0; cnt < 14; cnt++) {
	argv[cnt] = "";
      }

      if (username)
	argv[U_NAME] = username;
      else
	argv[U_NAME] = UNIQUE_LOGIN;
      if (uid)
	argv[U_UID] = uid;
      else
	argv[U_UID] = UNIQUE_UID;
      if (shell)
	argv[U_SHELL] = shell;
      else
	argv[U_SHELL] = "/bin/athena/tcsh";
      if (winshell)
	argv[U_WINCONSOLESHELL] = winshell;
      else
	argv[U_WINCONSOLESHELL] = "cmd";
      if (last)
	argv[U_LAST] = last;
      if (first)
	argv[U_FIRST] = first;
      if (middle)
	argv[U_MIDDLE] = middle;
      if (u_status)
	argv[U_STATE] = u_status;
      else
	argv[U_STATE] = "0";
      if (clearid)
	argv[U_MITID] = clearid;
      if (class)
	argv[U_CLASS] = class;
      if (comment)
	argv[U_COMMENT] = comment;
      /* Signature field always is the empty string */
      argv[U_SIGNATURE] = "";
      if (secure)
	argv[U_SECURE] = secure;
      else
	argv[U_SECURE] = "0";

      status = wrap_mr_query("add_user_account", 13, argv, NULL, NULL);
      if (status)
	{
	  com_err(whoami, status, "while adding user account.");
	  exit(1);
	}
    }
  else if (update_flag)
    {
      char *old_argv[20];
      char *argv[20];
      char *args[5];

      args[0] = username;
      
      status = wrap_mr_query("get_user_account_by_login", 1, args, 
			     save_query_info, old_argv);
      if (status)
	{
	  com_err(whoami, status, "while getting user information.");
	  exit(1);
	}

      argv[1] = old_argv[0];
      argv[2] = old_argv[1];
      argv[3] = old_argv[2];
      argv[4] = old_argv[3];
      argv[5] = old_argv[4];
      argv[6] = old_argv[5];
      argv[7] = old_argv[6];
      argv[8] = old_argv[7];
      argv[9] = old_argv[8];
      argv[10] = old_argv[9];
      argv[11] = old_argv[10];
      argv[12] = old_argv[11];
      argv[13] = old_argv[12];
      
      argv[0] = username;
      if (newlogin)
	argv[1] = newlogin;
      if (uid)
	argv[2] = uid;
      if (shell)
	argv[3] = shell;
      if (winshell)
	argv[4] = winshell;
      if (last)
	argv[5] = last;
      if (first)
	argv[6] = first;
      if (middle)
	argv[7] = middle;
      if (u_status)
	argv[8] = u_status;
      if (clearid)
	argv[9] = clearid;
      if (class)
	argv[10] = class;
      if (comment)
	argv[11] = comment;
      if (secure)
	argv[13] = secure;

      status = wrap_mr_query("update_user_account", 14, argv, NULL, NULL);

      if (status)
	com_err(whoami, status, "while updating user.");
      else if (newlogin)
	username = newlogin;
    }

  /* Deactivate a user, and the matching list and filesystem if they exist */
  if (deact_flag)
    {
      char *args[2];
      char *argv[20];
      int i;

      args[0] = username;
      args[1] = "3";

      status = wrap_mr_query("update_user_status", 2, args, NULL, NULL);
      if (status)
	{
	  com_err(whoami, status, "while deactivating user.");
	  exit(1);
	}

      status = wrap_mr_query("get_list_info", 1, args, save_query_info, argv);
      if (status == MR_SUCCESS)
	{
	  for (i = 13; i > 0; i--)
	    argv[i + 1] = argv[i];
	  argv[1] = username;
	  argv[L_ACTIVE + 1] = "0";
	  
	  status = wrap_mr_query("update_list", 14, argv, NULL, NULL);
	  if (status)
	    {
	      com_err(whoami, status, "while updating list, "
		      "not deactivating list or filesystem.");
	      exit(1);
	    }
	}
      else if (status && status != MR_NO_MATCH)
	{
	  com_err(whoami, status, "while retrieving list information.");
	  exit(1);
	}

      status = wrap_mr_query("get_filesys_by_label", 1, args, save_query_info,
			     argv);
      if (status == MR_SUCCESS)
	{
	  for (i = 11; i > 0; i--)
	    argv[i + 1] = argv[i];
	  argv[1] = username;
	  argv[FS_TYPE + 1] = "ERR";
	  argv[FS_COMMENTS + 1] = "Locker disabled; call 3-1325 for help";
	  
	  status = wrap_mr_query("update_filesys", 12, argv, NULL, NULL);
	  if (status)
	    {
	      com_err(whoami, status, "while updating filesystem, "
		      "not deactivating filesystem.");
	      exit(1);
	    }
	}
      else if (status && status != MR_NO_MATCH)
	{
	  com_err(whoami, status, "while retrieving filesystem information.");
	  exit(1);
	}
    }

  /* Display user info */
  if (info_flag)
    {
      char *args[2];
      char *argv[20];

      args[0] = username;
      status = wrap_mr_query("get_user_account_by_login", 1, args,
			     save_query_info, argv);
      if (status)
	{
	  com_err(whoami, status, "while getting user information.");
	  exit(1);
	}
      if (unformatted_flag)
	show_user_info_unformatted(argv);
      else
	show_user_info(argv);
    }

  /* register a user */
  if (reg_flag)
    {
      char *args[3];
      char *argv[20];

      args[0] = username;
      status = wrap_mr_query("get_user_account_by_login", 1, args,
			     save_query_info, argv);
      if (status)
	{
	  com_err(whoami, status, "while looking up uid.");
	  exit(1);
	}

      args[0] = argv[U_UID];
      args[1] = username;
      args[2] = "IMAP";

      status = wrap_mr_query("register_user", 3, args, NULL, NULL);
      if (status)
	{
	  com_err(whoami, status, "while registering user.");
	  exit(1);
	}
    }

  /* list user reservations */
  if (list_res_flag)
    {
      char *args[1];

      args[0] = username;
      status = wrap_mr_query("get_user_reservations", 1, args, 
			     show_reservations, NULL);
      if (status)
	if (status != MR_NO_MATCH) {
	  com_err(whoami, status, "while getting user reservations.");
	  exit(1);
	}
    }

  /* add user reservations */
  if (reservation_add_queue)
    {
      struct string_list *q = reservation_add_queue;

      while (q) 
	{
	  char *reservation = q->string;
	  char *args[2];

	  args[0] = username;
	  args[1] = reservation;
	  status = wrap_mr_query("add_user_reservation", 2, args, NULL, NULL);
	  if (status)
	    {
	      com_err(whoami, status, "while adding user reservation.");
	      exit(1);
	    }

	  q = q->next;
	}
    }

  /* delete user reservations */
  if (reservation_remove_queue)
    {
      struct string_list *q = reservation_remove_queue;

      while (q)
	{
	  char *reservation = q->string;
	  char *args[2];

	  args[0] = username;
	  args[1] = reservation;
	  status = wrap_mr_query("delete_user_reservation", 2, args, NULL, 
				 NULL);
	  if (status)
	    {
	      com_err(whoami, status, "while deleting user reservation.");
	      exit(1);
	    }
	  
	  q = q->next;
	}
    }
  /* We're done! */
  mr_disconnect();
  exit(0);
}

int save_query_info(int argc, char **argv, void *hint)
{
  int i;
  char **nargv = hint;

  for(i = 0; i < argc; i++)
    nargv[i] = strdup(argv[i]);
  
  return MR_CONT;
}

int show_reservations(int argc, char **argv, void *hint)
{
  printf("Reservation: %s\n", argv[0]);

  return MR_CONT;
}

struct string_list *add_to_string_list(struct string_list *old_list, char *s) {
  struct string_list *new_list;

  new_list = (struct string_list *)malloc(sizeof(struct string_list *));
  new_list->next = old_list;
  new_list->string = s;

  return new_list;
}

int wrap_mr_query(char *handle, int argc, char **argv,
                  int (*callback)(int, char **, void *), void *callarg) {
  if (verbose)
    print_query(handle, argc, argv);

  return mr_query(handle, argc, argv, callback, callarg);
}

void print_query(char *query_name, int argc, char **argv) {
  int cnt;

  printf("qy %s", query_name);
  for(cnt = 0; cnt < argc; cnt++)
    printf(" <%s>", argv[cnt]);
  printf("\n");
}

void show_user_info(char **argv)
{
  char tbuf[BUFSIZ];
  int status;

  sprintf(tbuf, "%s, %s %s", argv[U_LAST], argv[U_FIRST], argv[U_MIDDLE]);
  printf("Login name: %-20s Full name: %s\n", argv[U_NAME], tbuf);
  printf("User id: %-23s Login shell: %-10s\n", argv[U_UID], argv[U_SHELL]);
  printf("Class: %-25s Windows Console Shell: %-10s\n", argv[U_CLASS],
	 argv[U_WINCONSOLESHELL]);
  printf("Account is: %-20s MIT ID number: %s\n",
	 UserState(atoi(argv[U_STATE])), argv[U_MITID]);
  status = atoi(argv[U_STATE]);
  if (status == 0 || status == 2)
    {
      printf("User %s secure Account Coupon to register\n",
	      atoi(argv[U_SECURE]) ? "needs" : "does not need");
    }
  printf("Comments: %s\n", argv[U_COMMENT]);
  printf("Last mod by %s at %s with %s.\n", argv[U_MODBY], argv[U_MODTIME],
	 argv[U_MODWITH]);
}

void show_user_info_unformatted(char **argv)
{
  int status;

  printf("Login name:            %s\n", argv[U_NAME]);
  printf("Full name:             %s, %s %s\n", argv[U_LAST], argv[U_FIRST], 
	 argv[U_MIDDLE]);
  printf("User id:               %s\n", argv[U_UID]);
  printf("Class:                 %s\n", argv[U_CLASS]);
  printf("Login shell:           %s\n", argv[U_SHELL]);
  printf("Windows Console Shell: %s\n", argv[U_WINCONSOLESHELL]);
  printf("Account is:            %s\n", UserState(atoi(argv[U_STATE])));
  printf("MIT ID number:         %s\n", argv[U_MITID]);
  status = atoi(argv[U_STATE]);
  if (status == 0 || status == 2)
    printf("Secure:                %s secure Account Coupon to register\n",
	   atoi(argv[U_SECURE]) ? "Needs" : "Does not need");
  printf("Comments:              %s\n", argv[U_COMMENT]);
  printf("Last mod by:           %s\n", argv[U_MODBY]);
  printf("Last mod on:           %s\n", argv[U_MODTIME]);
  printf("Last mod with:         %s\n", argv[U_MODWITH]);
}

void usage(char **argv)
{
#define USAGE_OPTIONS_FORMAT "  %-39s%s\n"
  fprintf(stderr, "Usage: %s username [options]\n", argv[0]);
  fprintf(stderr, "Options are\n");
  fprintf(stderr, "  %-39s\n", "-i   | -info");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-C   | -create",
          "-D   | -deact");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-r   | -register",
	  "-R   | -rename newname");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-U   | -uid uid",
	  "-s   | -shell shell");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-S   | -status status",
	  "-w   | -winshell winshell");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-F   | -first firstname",
	  "-L   | -last lastname");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-M   | -middle middlename",
	  "-I   | -clearid mitid");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-cl  | -class class",
	  "-c   | -comment comment");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-6   | -secure 0|1",
	  "-lr  | -listreservation");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-ar  | -addreservation reservation",
	  "-dr  | -deletereservation reservation");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-u   | -unformatted",
	  "-n   | -noauth");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-v   | -verbose",
	  "-db  | -database host[:port]");

  exit(1);
}
