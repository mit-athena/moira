/* $Id: blanche.c,v 1.42 1999-12-14 20:13:59 danw Exp $
 *
 * Command line oriented Moira List tool.
 *
 * by Mark Rosenstein, September 1988.
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
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

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/blanche/blanche.c,v 1.42 1999-12-14 20:13:59 danw Exp $");

struct member {
  int type;
  char *name;
};

/* It is important to membercmp that M_USER < M_LIST < M_STRING */
#define M_ANY		0
#define M_USER		1
#define M_LIST		2
#define M_STRING	3
#define M_KERBEROS	4

/* argument parsing macro */
#define argis(a, b) (!strcmp(*arg + 1, a) || !strcmp(*arg + 1, b))

/* flags from command line */
int infoflg, verbose, syncflg, memberflg, recursflg, noauth;
int showusers, showstrings, showkerberos, showlists;
int createflag, setinfo, active, public, hidden, maillist, grouplist;
struct member *owner;
char *desc, *newname;

/* various member lists */
struct save_queue *addlist, *dellist, *memberlist, *synclist;

char *listname, *whoami;

void usage(char **argv);
void show_list_member(struct member *memberstruct);
int show_list_info(int argc, char **argv, void *hint);
int save_list_info(int argc, char **argv, void *hint);
int show_list_count(int argc, char **argv, void *hint);
void recursive_display_list_members(void);
void unique_add_member(struct save_queue *q, struct member *m);
int get_list_members(int argc, char **argv, void *sq);
void get_members_from_file(char *filename, struct save_queue *queue);
int collect(int argc, char **argv, void *l);
struct member *parse_member(char *s);
int membercmp(const void *mem1, const void *mem2);
int sq_count_elts(struct save_queue *q);

int main(int argc, char **argv)
{
  int status, success;
  char **arg = argv;
  char *membervec[3];
  struct member *memberstruct;
  char *server = NULL, *p;

  /* clear all flags & lists */
  infoflg = verbose = syncflg = memberflg = recursflg = 0;
  noauth = showusers = showstrings = showkerberos = showlists = 0;
  createflag = setinfo = 0;
  active = public = hidden = maillist = grouplist = -1;
  listname = newname = desc = NULL;
  owner = NULL;
  addlist = sq_create();
  dellist = sq_create();
  memberlist = sq_create();
  synclist = sq_create();
  whoami = argv[0];

  success = 1;

  /* parse args, building addlist, dellist, & synclist */
  while (++arg - argv < argc)
    {
      if (**arg == '-')
	{
	  if (argis("m", "members"))
	    memberflg++;
	  else if (argis("u", "users"))
	    showusers++;
	  else if (argis("s", "strings"))
	    showstrings++;
	  else if (argis("l", "lists"))
	    showlists++;
	  else if (argis("k", "kerberos"))
	    showkerberos++;
	  else if (argis("i", "info"))
	    infoflg++;
	  else if (argis("n", "noauth"))
	    noauth++;
	  else if (argis("v", "verbose"))
	    verbose++;
	  else if (argis("r", "recursive"))
	    recursflg++;
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
	  else if (argis("a", "add"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
		  if ((memberstruct = parse_member(*arg)))
		    sq_save_data(addlist, memberstruct);
		}
	      else
		usage(argv);
	    }
	  else if (argis("al", "addlist"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
		  get_members_from_file(*arg, addlist);
		}
	      else
		usage(argv);
	    }
	  else if (argis("d", "delete"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
		  if ((memberstruct = parse_member(*arg)))
		    sq_save_data(dellist, memberstruct);
		}
	      else
		usage(argv);
	    }
	  else if (argis("dl", "deletelist"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
		  get_members_from_file(*arg, dellist);
		}
	      else
		usage(argv);
	    }
	  else if (argis("f", "file"))
	    {
	      if (arg - argv < argc - 1)
		{
		  syncflg++;
		  ++arg;
		  get_members_from_file(*arg, synclist);
		}
	      else
		usage(argv);
	    }
	  else if (argis("C", "create"))
	    createflag++;
	  else if (argis("P", "public"))
	    {
	      setinfo++;
	      public = 1;
	    }
	  else if (argis("NP", "private"))
	    {
	      setinfo++;
	      public = 0;
	    }
	  else if (argis("A", "active"))
	    {
	      setinfo++;
	      active = 1;
	    }
	  else if (argis("I", "inactive"))
	    {
	      setinfo++;
	      active = 0;
	    }
	  else if (argis("V", "visible"))
	    {
	      setinfo++;
	      hidden = 0;
	    }
	  else if (argis("H", "hidden"))
	    {
	      setinfo++;
	      hidden = 1;
	    }
	  else if (argis("M", "mail"))
	    {
	      setinfo++;
	      maillist = 1;
	    }
	  else if (argis("NM", "notmail"))
	    {
	      setinfo++;
	      maillist = 0;
	    }
	  else if (argis("G", "group"))
	    {
	      setinfo++;
	      grouplist = 1;
	    }
	  else if (argis("NG", "notgroup"))
	    {
	      setinfo++;
	      grouplist = 0;
	    }
	  else if (argis("D", "desc"))
	    {
	      if (arg - argv < argc - 1)
		{
		  setinfo++;
		  ++arg;
		  desc = *arg;
		}
	      else
		usage(argv);
	    }
	  else if (argis("O", "owner"))
	    {
	      if (arg - argv < argc - 1)
		{
		  setinfo++;
		  ++arg;
		  owner = parse_member(*arg);
		}
	      else
		usage(argv);
	    }
	  else if (argis("R", "rename"))
	    {
	      if (arg - argv < argc - 1)
		{
		  setinfo++;
		  ++arg;
		  newname = *arg;
		}
	      else
		usage(argv);
	    }
	  else
	    usage(argv);
	}
      else if (listname == NULL)
	listname = *arg;
      else
	usage(argv);
    }
  if (listname == NULL)
    usage(argv);

  /* if no other options specified, turn on list members flag */
  if (!(infoflg || syncflg || createflag || setinfo ||
	addlist->q_next != addlist || dellist->q_next != dellist))
    memberflg++;

  /* If none of {users,strings,lists,kerberos} specified, turn them all on */
  if (!(showusers || showstrings || showlists || showkerberos))
    showusers = showstrings = showlists = showkerberos = 1;

  /* fire up Moira */
  status = mrcl_connect(server, "blanche", !noauth);
  if (status == MRCL_AUTH_ERROR)
    {
      com_err(whoami, 0, "Try the -noauth flag if you don't "
	      "need authentication.");
    }
  if (status)
    exit(2);

  /* check for username/listname clash */
  if (createflag || (setinfo && newname && strcmp(newname, listname)))
    {
      status = mr_query("get_user_account_by_login", 1,
			createflag ? &listname : &newname,
			NULL, NULL);
      if (status != MR_NO_MATCH)
	fprintf(stderr, "WARNING: A user by that name already exists.\n");
    }

  /* create if needed */
  if (createflag)
    {
      char *argv[10];

      argv[0] = listname;
      argv[1] = (active == 0) ? "0" : "1";
      argv[2] = (public == 1) ? "1" : "0";
      argv[3] = (hidden == 1) ? "1" : "0";
      argv[4] = (maillist == 0) ? "0" : "1";
      argv[5] = (grouplist == 1) ? "1" : "0";
      argv[6] = UNIQUE_GID;
      argv[9] = desc ? desc : "none";

      if (owner)
	{
	  argv[8] = owner->name;
	  switch (owner->type)
	    {
	    case M_ANY:
	    case M_USER:
	      argv[7] = "USER";
	      status = mr_query("add_list", 10, argv, NULL, NULL);
	      if (owner->type != M_ANY || status != MR_USER)
		break;

	    case M_LIST:
	      argv[7] = "LIST";
	      status = mr_query("add_list", 10, argv, NULL, NULL);
	      break;

	    case M_KERBEROS:
	      argv[7] = "KERBEROS";
	      status = mr_query("add_list", 10, argv, NULL, NULL);
	      break;
	    }
	}
      else
	{
	  argv[7] = "USER";
	  argv[8] = getenv("USER");

	  status = mr_query("add_list", 10, argv, NULL, NULL);
	}

      if (status)
	{
	  com_err(whoami, status, "while creating list.");
	  exit(1);
	}
    }
  else if (setinfo)
    {
      char *argv[11];

      status = mr_query("get_list_info", 1, &listname,
			save_list_info, argv);
      if (status)
	{
	  com_err(whoami, status, "while getting list information");
	  exit(1);
	}

      argv[0] = listname;
      if (newname)
	argv[1] = newname;
      if (active != -1)
	argv[2] = active ? "1" : "0";
      if (public != -1)
	argv[3] = public ? "1" : "0";
      if (hidden != -1)
	argv[4] = hidden ? "1" : "0";
      if (maillist != -1)
	argv[5] = maillist ? "1" : "0";
      if (grouplist != -1)
	argv[6] = grouplist ? "1" : "0";
      if (desc)
	argv[10] = desc;

      if (owner)
	{
	  argv[9] = owner->name;
	  switch (owner->type)
	    {
	    case M_ANY:
	    case M_USER:
	      argv[8] = "USER";
	      status = mr_query("update_list", 11, argv, NULL, NULL);
	      if (owner->type != M_ANY || status != MR_USER)
		break;

	    case M_LIST:
	      argv[8] = "LIST";
	      status = mr_query("update_list", 11, argv, NULL, NULL);
	      break;

	    case M_KERBEROS:
	      argv[8] = "KERBEROS";
	      status = mr_query("update_list", 11, argv, NULL, NULL);
	      break;
	    }
	}
      else
	status = mr_query("update_list", 11, argv, NULL, NULL);

      if (status)
	{
	  com_err(whoami, status, "while updating list.");
	  success = 0;
	}
      else if (newname)
	listname = newname;
    }

  /* display list info if requested to */
  if (infoflg)
    {
      status = mr_query("get_list_info", 1, &listname, show_list_info, NULL);
      if (status)
	{
	  com_err(whoami, status, "while getting list information");
	  success = 0;
	}
      if (verbose && !memberflg)
	{
	  status = mr_query("count_members_of_list", 1, &listname,
			    show_list_count, NULL);
	  if (status)
	    {
	      com_err(whoami, status, "while getting list count");
	      success = 0;
	    }
	}
    }

  /* if we're synchronizing to a file, we need to:
   *  get the current members of the list
   *	for each member of the sync file
   *	   if they are on the list, remove them from the in-memory copy
   *	   if they're not on the list, add them to add-list
   *	if anyone is left on the in-memory copy, put them on the delete-list
   * lastly, reset memberlist so we can use it again later
   */
  if (syncflg)
    {
      status = mr_query("get_members_of_list", 1, &listname,
			get_list_members, memberlist);
      if (status)
	{
	  com_err(whoami, status, "getting members of list %s", listname);
	  exit(2);
	}
      while (sq_get_data(synclist, &memberstruct))
	{
	  struct save_queue *q;
	  int removed = 0;

	  for (q = memberlist->q_next; q != memberlist; q = q->q_next)
	    {
	      if (membercmp(q->q_data, memberstruct) == 0)
		{
		  q->q_prev->q_next = q->q_next;
		  q->q_next->q_prev = q->q_prev;
		  removed++;
		  break;
		}
	    }
	  if (!removed)
	    sq_save_data(addlist, memberstruct);
	}
      while (sq_get_data(memberlist, &memberstruct))
	sq_save_data(dellist, memberstruct);
      sq_destroy(memberlist);
      memberlist = sq_create();
    }

  /* Process the add list */
  while (sq_get_data(addlist, &memberstruct))
    {
      /* canonicalize string if necessary */
      if (memberstruct->type != M_KERBEROS &&
	  (p = strchr(memberstruct->name, '@')))
	{
	  char *host = canonicalize_hostname(strdup(++p));
	  static char **mailhubs = NULL;
	  char *argv[4];
	  int i;

	  if (!mailhubs)
	    {
	      argv[0] = "mailhub";
	      argv[1] = "TYPE";
	      argv[2] = "*";
	      mailhubs = malloc(sizeof(char *));
	      mailhubs[0] = NULL;
	      status = mr_query("get_alias", 3, argv, collect,
				&mailhubs);
	      if (status != MR_SUCCESS && status != MR_NO_MATCH)
		{
		  com_err(whoami, status,
			  " while reading list of MAILHUB servers");
		  mailhubs[0] = NULL;
		}
	    }
	  for (i = 0; (p = mailhubs[i]); i++)
	    {
	      if (!strcasecmp(p, host))
		{
		  host = strdup(memberstruct->name);
		  *(strchr(memberstruct->name, '@')) = 0;
		  if (memberstruct->type == M_STRING)
		      memberstruct->type = M_ANY;
		  fprintf(stderr, "Warning: \"%s\" converted to "
			  "\"%s\" because it is a local name.\n",
			  host, memberstruct->name);
		  break;
		}
	    }
	  free(host);
	}
      /* now continue adding member */
      membervec[0] = listname;
      membervec[2] = memberstruct->name;
      if (verbose)
	{
	  printf("Adding member ");
	  show_list_member(memberstruct);
	}
      switch (memberstruct->type)
	{
	case M_ANY:
	case M_USER:
	  membervec[1] = "USER";
	  status = mr_query("add_member_to_list", 3, membervec, NULL, NULL);
	  if (status == MR_SUCCESS)
	    break;
	  else if (status != MR_USER || memberstruct->type != M_ANY)
	    {
	      com_err(whoami, status, "while adding member %s to %s",
		      memberstruct->name, listname);
	      success = 0;
	      break;
	    }
	case M_LIST:
	  membervec[1] = "LIST";
	  status = mr_query("add_member_to_list", 3, membervec,
			    NULL, NULL);
	  if (status == MR_SUCCESS)
	    {
	      if (!strcmp(membervec[0], getenv("USER")))
		{
		  fprintf(stderr, "\nWARNING: \"LIST:%s\" was just added "
			  "to list \"%s\".\n", membervec[2], membervec[0]);
		  fprintf(stderr, "If you meant to add yourself to the list "
			  "\"%s\", type:\n", membervec[2]);
		  fprintf(stderr, "\tblanche %s -d %s\t(to undo this)\n",
			  membervec[0], membervec[2]);
		  fprintf(stderr, "\tblanche %s -a %s\t(to add yourself to "
			  "that list)\n", membervec[2], membervec[0]);
		}
	      break;
	    }
	  else if (status != MR_LIST || memberstruct->type != M_ANY)
	    {
	      com_err(whoami, status, "while adding member %s to %s",
		      memberstruct->name, listname);
	      success = 0;
	      break;
	    }
	case M_STRING:
	  if (memberstruct->type == M_ANY &&
	      !strchr(memberstruct->name, '@') &&
	      !strchr(memberstruct->name, '!') &&
	      !strchr(memberstruct->name, '%'))
	    {
	      /* if user is trying to add something which isn't a
		 remote string, or a list, or a user, and didn't
		 explicitly specify `STRING:', it's probably a typo */
	      com_err(whoami, MR_NO_MATCH, "while adding member %s to %s",
		      memberstruct->name, listname);
	      success = 0;
	      break;
	    }

	  membervec[1] = "STRING";
	  status = mr_query("add_member_to_list", 3, membervec,
			    NULL, NULL);
	  if (status != MR_SUCCESS)
	    {
	      com_err(whoami, status, "while adding member %s to %s",
		      memberstruct->name, listname);
	      success = 0;
	    }
	  break;
	case M_KERBEROS:
	  membervec[1] = "KERBEROS";
	  status = mr_query("add_member_to_list", 3, membervec,
			    NULL, NULL);
	  if (status != MR_SUCCESS)
	    {
	      com_err(whoami, status, "while adding member %s to %s",
		      memberstruct->name, listname);
	      success = 0;
	    }
	}
    }

  /* Process the delete list */
  while (sq_get_data(dellist, &memberstruct))
    {
      membervec[0] = listname;
      membervec[2] = memberstruct->name;
      if (verbose)
	{
	  printf("Deleting member ");
	  show_list_member(memberstruct);
	}
      switch (memberstruct->type)
	{
	case M_ANY:
	case M_USER:
	  membervec[1] = "USER";
	  status = mr_query("delete_member_from_list", 3, membervec,
			    NULL, NULL);
	  if (status == MR_SUCCESS)
	    break;
	  else if ((status != MR_USER && status != MR_NO_MATCH) ||
		   memberstruct->type != M_ANY)
	    {
	      com_err(whoami, status, "while deleting member %s from %s",
		      memberstruct->name, listname);
	      success = 0;
	      break;
	    }
	case M_LIST:
	  membervec[1] = "LIST";
	  status = mr_query("delete_member_from_list", 3, membervec,
			    NULL, NULL);
	  if (status == MR_SUCCESS)
	    break;
	  else if ((status != MR_LIST && status != MR_NO_MATCH) ||
		   memberstruct->type != M_ANY)
	    {
	      if (status == MR_PERM && memberstruct->type == M_ANY &&
		  !strcmp(membervec[2], getenv("USER")))
		{
		  /* M_ANY means we've fallen through from the user
		   * case. The user is trying to remove himself from
		   * a list, but we got MR_USER or MR_NO_MATCH above,
		   * meaning he's not really on it, and we got MR_PERM
		   * when trying to remove LIST:$USER because he's not
		   * on the acl. That error is useless, so return
		   * MR_NO_MATCH instead. However, this will generate the
		   * wrong error if the user was trying to remove the list
		   * with his username from a list he doesn't administrate
		   * without explicitly specifying "list:".
		   */
		  status = MR_NO_MATCH;
		}
	      com_err(whoami, status, "while deleting member %s from %s",
		      memberstruct->name, listname);
	      success = 0;
	      break;
	    }
	case M_STRING:
	  membervec[1] = "STRING";
	  status = mr_query("delete_member_from_list", 3, membervec,
			    NULL, NULL);
	  if (status == MR_STRING && memberstruct->type == M_ANY)
	    {
	      com_err(whoami, 0, " Unable to find member %s to delete from %s",
		      memberstruct->name, listname);
	      success = 0;
	      if (!strcmp(membervec[0], getenv("USER")))
		{
		  fprintf(stderr, "(If you were trying to remove yourself "
			  "from the list \"%s\",\n", membervec[2]);
		  fprintf(stderr, "the correct command is \"blanche %s -d "
			  "%s\".)\n", membervec[2], membervec[0]);
		}
	    }
	  else if (status != MR_SUCCESS)
	    {
	      com_err(whoami, status, "while deleting member %s from %s",
		      memberstruct->name, listname);
	      success = 0;
	    }
	  break;
	case M_KERBEROS:
	  membervec[1] = "KERBEROS";
	  status = mr_query("delete_member_from_list", 3, membervec,
			    NULL, NULL);
	  if (status != MR_SUCCESS)
	    {
	      com_err(whoami, status, "while deleting member %s from %s",
		      memberstruct->name, listname);
	      success = 0;
	    }
	}
    }

  /* Display the members of the list now, if requested */
  if (memberflg)
    {
      if (recursflg)
	recursive_display_list_members();
      else
	{
	  status = mr_query("get_members_of_list", 1, &listname,
			    get_list_members, memberlist);
	  if (status)
	    com_err(whoami, status, "while getting members of list %s",
		    listname);
	  while (sq_get_data(memberlist, &memberstruct))
	    show_list_member(memberstruct);
	}
    }

  /* We're done! */
  mr_disconnect();
  exit(success ? 0 : 1);
}

void usage(char **argv)
{
  fprintf(stderr, "Usage: %s listname [options]\n", argv[0]);
  fprintf(stderr, "Options are\n");
  fprintf(stderr, "  %-39s%-39s\n", "-v  | -verbose",
	  "-C  | -create");
  fprintf(stderr, "  %-39s%-39s\n", "-m  | -members",
	  "-R  | -rename newname");
  fprintf(stderr, "  %-39s%-39s\n", "-u  | -users",
	  "-P  | -public");
  fprintf(stderr, "  %-39s%-39s\n", "-l  | -lists",
	  "-NP | -private");
  fprintf(stderr, "  %-39s%-39s\n", "-s  | -strings",
	  "-A  | -active");
  fprintf(stderr, "  %-39s%-39s\n", "-k  | -kerberos",
	  "-I  | -inactive");
  fprintf(stderr, "  %-39s%-39s\n", "-i  | -info",
	  "-V  | -visible");
  fprintf(stderr, "  %-39s%-39s\n", "-r  | -recursive",
	  "-H  | -hidden");
  fprintf(stderr, "  %-39s%-39s\n", "-a  | -add member",
	  "-M  | -mail");
  fprintf(stderr, "  %-39s%-39s\n", "-d  | -delete member",
	  "-NM | -notmail");
  fprintf(stderr, "  %-39s%-39s\n", "-al | -addlist filename",
	  "-G  | -group");
  fprintf(stderr, "  %-39s%-39s\n", "-dl | -deletelist filename",
	  "-NG | -notgroup");
  fprintf(stderr, "  %-39s%-39s\n", "-f  | -file filename",
	  "-D  | -desc description");
  fprintf(stderr, "  %-39s%-39s\n", "-n  | -noauth",
	  "-O  | -owner owner");
  fprintf(stderr, "  %-39s%-39s\n", "-db | -database host[:port]",
	  "");
  exit(1);
}


/* Display the members stored in the queue */

void show_list_member(struct member *memberstruct)
{
  char *s = "";

  switch (memberstruct->type)
    {
    case M_USER:
      if (!showusers)
	return;
      s = "USER";
      break;
    case M_LIST:
      if (!showlists)
	return;
      s = "LIST";
      break;
    case M_STRING:
      if (!showstrings)
	return;
      s = "STRING";
      break;
    case M_KERBEROS:
      if (!showkerberos)
	return;
      s = "KERBEROS";
      break;
    case M_ANY:
      printf("%s\n", memberstruct->name);
      return;
    }

  if (verbose)
    printf("%s:%s\n", s, memberstruct->name);
  else
    {
      if (memberstruct->type == M_LIST)
	printf("LIST:%s\n", memberstruct->name);
      else if (memberstruct->type == M_KERBEROS)
	printf("KERBEROS:%s\n", memberstruct->name);
      else if (memberstruct->type == M_STRING &&
	       !strchr(memberstruct->name, '@'))
	printf("STRING:%s\n", memberstruct->name);
      else
	printf("%s\n", memberstruct->name);
    }
}


/* Show the retrieved information about a list */

int show_list_info(int argc, char **argv, void *hint)
{
  printf("List: %s\n", argv[0]);
  printf("Description: %s\n", argv[9]);
  printf("Flags: %s, %s, and %s\n",
	 atoi(argv[1]) ? "active" : "inactive",
	 atoi(argv[2]) ? "public" : "private",
	 atoi(argv[3]) ? "hidden" : "visible");
  printf("%s is %sa maillist and is %sa group", argv[0],
	 atoi(argv[4]) ? "" : "not ",
	 atoi(argv[5]) ? "" : "not ");
  if (atoi(argv[5]))
    printf(" with GID %d\n", atoi(argv[6]));
  else
    printf("\n");
  printf("Owner: %s %s\n", argv[7], argv[8]);
  printf("Last modified by %s with %s on %s\n", argv[11], argv[12], argv[10]);
  return MR_CONT;
}


/* Copy retrieved information about a list into a new argv */

int save_list_info(int argc, char **argv, void *hint)
{
  char **nargv = hint;

  for (argc = 0; argc < 10; argc++)
    nargv[argc + 1] = strdup(argv[argc]);
  return MR_CONT;
}

/* Show the retrieve list member count */

int show_list_count(int argc, char **argv, void *hint)
{
  printf("Members: %s\n", argv[0]);
  return MR_CONT;
}


/* Recursively find all of the members of listname, and then display them */

void recursive_display_list_members(void)
{
  int status, count, savecount;
  struct save_queue *lists, *members;
  struct member *m, *m1, *data;

  lists = sq_create();
  members = sq_create();
  m = malloc(sizeof(struct member));
  m->type = M_LIST;
  m->name = listname;
  sq_save_data(lists, m);

  while (sq_get_data(lists, &m))
    {
      sq_destroy(memberlist);
      memberlist = sq_create();
      status = mr_query("get_members_of_list", 1, &(m->name),
			get_list_members, memberlist);
      if (status)
	com_err(whoami, status, "while getting members of list %s", m->name);
      while (sq_get_data(memberlist, &m1))
	{
	  if (m1->type == M_LIST)
	    unique_add_member(lists, m1);
	  else
	    unique_add_member(members, m1);
	}
    }
  savecount = count = sq_count_elts(members);
  data = malloc(count * sizeof(struct member));
  count = 0;
  while (sq_get_data(members, &m))
    memcpy(&data[count++], m, sizeof(struct member));
  qsort(data, count, sizeof(struct member), membercmp);
  for (count = 0; count < savecount; count++)
    show_list_member(&data[count]);
}


/* add a struct member to a queue if that member isn't already there. */

void unique_add_member(struct save_queue *q, struct member *m)
{
  struct save_queue *qp;

  for (qp = q->q_next; qp != q; qp = qp->q_next)
    {
      if (!membercmp(qp->q_data, m))
	return;
    }
  sq_save_data(q, m);
}


/* Collect the retrieved members of the list */

int get_list_members(int argc, char **argv, void *sq)
{
  struct save_queue *q = sq;
  struct member *m;

  m = malloc(sizeof(struct member));
  switch (argv[0][0])
    {
    case 'U':
      m->type = M_USER;
      break;
    case 'L':
      m->type = M_LIST;
      break;
    case 'S':
      m->type = M_STRING;
      break;
    case 'K':
      m->type = M_KERBEROS;
      break;
    }
  m->name = strdup(argv[1]);
  sq_save_data(q, m);
  return MR_CONT;
}


/* Open file, parse members from file, and put them on the specified queue */
void get_members_from_file(char *filename, struct save_queue *queue)
{
  FILE *in;
  char buf[BUFSIZ];
  struct member *memberstruct;

  if (!strcmp(filename, "-"))
    in = stdin;
  else
    {
      in = fopen(filename, "r");
      if (!in)
	{
	  com_err(whoami, errno, "while opening %s for input", filename);
	  exit(2);
	}
    }

  while (fgets(buf, BUFSIZ, in))
    {
      if ((memberstruct = parse_member(buf)))
	sq_save_data(queue, memberstruct);
    }
  if (!feof(in))
    {
      com_err(whoami, errno, "while reading from %s", filename);
      exit(2);
    }
}


/* Collect the possible expansions of the alias MAILHUB */

int collect(int argc, char **argv, void *l)
{
  char ***list = l;
  int i;

  for (i = 0; (*list)[i]; i++)
    ;
  *list = realloc(*list, (i + 2) * sizeof(char *));
  (*list)[i] = strdup(argv[2]);
  (*list)[i + 1] = NULL;
  return MR_CONT;
}


/* Parse a line of input, fetching a member.  NULL is returned if a member
 * is not found.  ';' is a comment character.
 */

struct member *parse_member(char *s)
{
  struct member *m;
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

  if (!(m = malloc(sizeof(struct member))))
    return NULL;

  if ((p = strchr(s, ':')))
    {
      *p = '\0';
      m->name = ++p;
      if (!strcasecmp("user", s))
	m->type = M_USER;
      else if (!strcasecmp("list", s))
	m->type = M_LIST;
      else if (!strcasecmp("string", s))
	m->type = M_STRING;
      else if (!strcasecmp("kerberos", s))
	m->type = M_KERBEROS;
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
      m->type = M_ANY;
    }
  return m;
}


/*
 * This routine two compares members by the following rules:
 * 1.  A USER is less than a LIST
 * 2.  A LIST is less than a STRING
 * 3.  If two members are of the same type, the one alphabetically first
 *     is less than the other
 * It returs < 0 if the first member is less, 0 if they are identical, and
 * > 0 if the second member is less (the first member is greater).
 */

int membercmp(const void *mem1, const void *mem2)
{
  const struct member *m1 = mem1, *m2 = mem2;

  if (m1->type == M_ANY || m2->type == M_ANY || (m1->type == m2->type))
    return strcmp(m1->name, m2->name);
  else
    return m1->type - m2->type;
}


int sq_count_elts(struct save_queue *q)
{
  char *foo;
  int count;

  count = 0;
  while (sq_get_data(q, &foo))
    count++;
  return count;
}
