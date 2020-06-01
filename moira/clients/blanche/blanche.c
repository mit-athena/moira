/* $Id$
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

RCSID("$HeadURL$ $Id$");

/* argument parsing macro */
#define argis(a, b) (!strcmp(*arg + 1, a) || !strcmp(*arg + 1, b))

/* flags from command line */
int infoflg, verbose, syncflg, memberflg, recursflg, noauth;
int showusers, showstrings, showkerberos, showlists, showtags, showmachines, showids;
int createflag, setinfo, active, public, hidden, maillist, grouplist;
int nfsgroup, mailman, pacslist;
struct mrcl_ace_type *owner, *memacl;
char *desc, *newname, *mailman_server, *gid;

/* various member lists */
struct save_queue *addlist, *dellist, *memberlist, *synclist, *taglist;

char *listname, *whoami;

void usage(char **argv);
void show_list_member(struct mrcl_ace_type *memberstruct);
int show_list_info(int argc, char **argv, void *hint);
int save_list_info(int argc, char **argv, void *hint);
int show_list_count(int argc, char **argv, void *hint);
void recursive_display_list_members(void);
void unique_add_member(struct save_queue *q, struct mrcl_ace_type *m);
int get_list_members(int argc, char **argv, void *sq);
void get_members_from_file(char *filename, struct save_queue *queue);
int collect(int argc, char **argv, void *l);
int membercmp(const void *mem1, const void *mem2);
int sq_count_elts(struct save_queue *q);
char *get_username(void);

int main(int argc, char **argv)
{
  int status, success;
  char **arg = argv;
  char *membervec[4];
  struct mrcl_ace_type *memberstruct;
  char *server = NULL, *p;

  /* clear all flags & lists */
  infoflg = verbose = syncflg = memberflg = recursflg = 0;
  noauth = showusers = showstrings = showkerberos = showlists = 0;
  showtags = showmachines = showids = createflag = setinfo = 0;
  active = public = hidden = maillist = grouplist = nfsgroup = mailman = -1;
  pacslist = -1;
  listname = newname = desc = gid = NULL;
  owner = NULL;
  memacl = NULL;
  addlist = sq_create();
  dellist = sq_create();
  memberlist = sq_create();
  synclist = sq_create();
  taglist = sq_create();
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
	  else if (argis("mach", "machines"))
	    showmachines++;
	  else if (argis("id", "ids"))
	    showids++;
	  else if (argis("t", "tags"))
	    showtags++;
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
		  memberstruct = mrcl_parse_member(*arg);
		  if (memberstruct)
		    sq_save_data(addlist, memberstruct);
		  else 
		    com_err(whoami, 0, "Unable to add list member: invalid format.");
		}
	      else
		usage(argv);
	    }
	  else if (argis("at", "addtagged"))
	    {
	      if (arg - argv < argc - 2)
		{
		  ++arg;
		  memberstruct = mrcl_parse_member(*arg);
		  if (memberstruct)
		    {
		      sq_save_data(addlist, memberstruct);
		      memberstruct->tag = *++arg;
		    }
		  else
		    com_err(whoami, 0, "Unable to add list member: invalid format.");
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
		  memberstruct = mrcl_parse_member(*arg);
		  if (memberstruct)
		    sq_save_data(dellist, memberstruct);
		  else
		    com_err(whoami, 0, "Unable to delete list member: invalid format.");
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
	  else if (argis("ct", "changetag"))
	    {
	      if (arg - argv < argc - 2)
		{
		  ++arg;
		  memberstruct = mrcl_parse_member(*arg);
		  if (memberstruct)
		    {
		      sq_save_data(taglist, memberstruct);
		      memberstruct->tag = *++arg;
		    }
		  else
		    com_err(whoami, 0, "Unable to modify list member: invalid format.");
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
	  else if (argis("N", "nfs"))
	    {
	      setinfo++;
	      nfsgroup = 1;
	    }
	  else if (argis("NN", "notnfs"))
	    {
	      setinfo++;
	      nfsgroup = 0;
	    }
	  else if (argis("mm", "mailman"))
	    {
	      setinfo++;
	      mailman = 1;
	    }
	  else if (argis("nmm", "notmailman"))
	    {
	      setinfo++;
	      mailman = 0;
	    }
	  else if (argis("ms", "mailman_server"))
	    {
	      if (arg - argv < argc - 1)
		{
		  setinfo++;
		  ++arg;
		  mailman_server = canonicalize_hostname(strdup(*arg));
		}
	      else
		usage(argv);
	    }
	  else if (argis("p", "pacs"))
	    {
	      setinfo++;
	      pacslist = 1;
	    }
	  else if (argis("np", "notpacs"))
	    {
	      setinfo++;
	      pacslist = 0;
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
		  owner = mrcl_parse_member(*arg);
		  if (!owner || owner->type == MRCL_M_STRING || owner->type == MRCL_M_MACHINE || owner->type == MRCL_M_ID)
		    {
		      com_err(whoami, 0, "Invalid owner format.  Must be one of USER, LIST, KERBEROS, or NONE.");
		      exit(1);
		    }
		}
	      else
		usage(argv);
	    }
	  else if (argis("MA", "memacl"))
	    {
	      if (arg - argv < argc -1)
		{
		  setinfo++;
		  ++arg;
		  memacl = mrcl_parse_member(*arg);
		  if (!memacl || memacl->type == MRCL_M_STRING || memacl->type == MRCL_M_MACHINE || memacl->type == MRCL_M_ID)
		    {
		      com_err(whoami, 0, "Invalid memacl format.  Must be one of USER, LIST, KERBEROS, or NONE.");
		      exit(1);
		    }
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
	  else if (argis("g", "gid"))
	    {
	      if (arg - argv < argc - 1)
		{
		  setinfo++;
		  ++arg;
		  gid = *arg;
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
	addlist->q_next != addlist || dellist->q_next != dellist ||
	taglist->q_next != taglist))
    memberflg++;

  /* If none of {users,strings,lists,kerberos,machines} specified, 
     turn them all on */
  if (!(showusers || showstrings || showlists || showkerberos || 
	showmachines || showids))
    showusers = showstrings = showlists = showkerberos = showmachines = showids = 1;

  /* fire up Moira */
  status = mrcl_connect(server, "blanche", 18, !noauth);
  if (status == MRCL_AUTH_ERROR)
    {
      com_err(whoami, 0, "Authentication error while working on list %s",
	      listname);
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
      char *argv[15];

      argv[L_NAME] = listname;
      argv[L_ACTIVE] = (active == 0) ? "0" : "1";
      argv[L_PUBLIC] = (public == 1) ? "1" : "0";
      argv[L_HIDDEN] = (hidden == 1) ? "1" : "0";
      argv[L_MAILLIST] = (maillist == 0) ? "0" : "1";
      argv[L_GROUP] = (grouplist == 1) ? "1" : "0";

      if (gid)
	argv[L_GID] = gid;
      else
	argv[L_GID] = UNIQUE_GID;

      argv[L_NFSGROUP] = (nfsgroup == 1) ? "1" : "0";
      argv[L_MAILMAN] = (mailman == 1) ? "1" : "0";
      argv[L_PACSLIST] = (pacslist == 1) ? "1" : "0";
      argv[L_DESC] = desc ? desc : "none";

      if (mailman)
	argv[L_MAILMAN_SERVER] = mailman_server ? mailman_server : "[ANY]";
      else
	argv[L_MAILMAN_SERVER] = "[NONE]";

      if (memacl)
	{
	  argv[L_MEMACE_NAME] = memacl->name;
	  switch (memacl->type)
	    {
	    case MRCL_M_ANY:
	      status = mr_query("get_user_account_by_login", 1,
				&memacl->name, NULL, NULL);
	      if (status == MR_NO_MATCH)
		argv[L_MEMACE_TYPE] = "LIST";
	      else
		argv[L_MEMACE_TYPE] = "USER";
	      break;

	    case MRCL_M_USER:
	      argv[L_MEMACE_TYPE] = "USER";
	      break;

	    case MRCL_M_LIST:
	      argv[L_MEMACE_TYPE] = "LIST";
	      break;

	    case MRCL_M_KERBEROS:
	      status = mrcl_validate_kerberos_member(argv[L_MEMACE_NAME],
						     &argv[L_MEMACE_NAME]);
	      if (mrcl_get_message())
		mrcl_com_err(whoami);
	      if (status == MRCL_REJECT)
		exit(1);
	      argv[L_MEMACE_TYPE] = "KERBEROS";
	      break;

	    case MRCL_M_NONE:
	      argv[L_MEMACE_TYPE] = argv[L_MEMACE_NAME] = "NONE";
	      break;
	    }
	}
      else 
	argv[L_MEMACE_TYPE] = argv[L_MEMACE_NAME] = "NONE";

      if (owner)
	{
	  argv[L_ACE_NAME] = owner->name;
	  switch (owner->type)
	    {
	    case MRCL_M_ANY:
	    case MRCL_M_USER:
	      argv[L_ACE_TYPE] = "USER";
	      status = mr_query("add_list", 16, argv, NULL, NULL);
	      if (owner->type != MRCL_M_ANY || status != MR_USER)
		break;

	    case MRCL_M_LIST:
	      argv[L_ACE_TYPE] = "LIST";
	      status = mr_query("add_list", 16, argv, NULL, NULL);
	      break;

	    case MRCL_M_KERBEROS:
	      argv[L_ACE_TYPE] = "KERBEROS";
	      status = mrcl_validate_kerberos_member(argv[L_ACE_NAME], 
						     &argv[L_ACE_NAME]);
	      if (mrcl_get_message())
		mrcl_com_err(whoami);
	      if (status == MRCL_REJECT)
		exit(1);
	      status = mr_query("add_list", 16, argv, NULL, NULL);
	      break;
	    case MRCL_M_NONE:
	      argv[L_ACE_TYPE] = argv[L_ACE_NAME] = "NONE";
	      status = mr_query("add_list", 16, argv, NULL, NULL);
	      break;
	    }
	}
      else
	{
	  argv[L_ACE_TYPE] = "USER";
	  argv[L_ACE_NAME] = get_username();

	  status = mr_query("add_list", 16, argv, NULL, NULL);
	}

      if (status)
	{
	  com_err(whoami, status, "while creating list.");
	  exit(1);
	}
    }
  else if (setinfo)
    {
      char *argv[16];

      status = mr_query("get_list_info", 1, &listname,
			save_list_info, argv);
      if (status)
	{
	  com_err(whoami, status, "while getting list information");
	  exit(1);
	}

      argv[0] = listname;
      if (newname)
	argv[L_NAME + 1] = newname;
      if (active != -1)
	argv[L_ACTIVE + 1] = active ? "1" : "0";
      if (public != -1)
	argv[L_PUBLIC + 1] = public ? "1" : "0";
      if (hidden != -1)
	argv[L_HIDDEN + 1] = hidden ? "1" : "0";
      if (maillist != -1)
	argv[L_MAILLIST + 1] = maillist ? "1" : "0";
      if (grouplist != -1)
	argv[L_GROUP + 1] = grouplist ? "1" : "0";
      if (gid)
	argv[L_GID + 1] = gid;
      if (nfsgroup != -1)
	argv[L_NFSGROUP + 1] = nfsgroup ? "1" : "0";
      if (mailman != -1)
	argv[L_MAILMAN + 1] = mailman ? "1" : "0";
      if (pacslist != -1)
	argv[L_PACSLIST + 1] = pacslist ? "1" : "0";

      /* If someone toggled the mailman bit, but didn't specify a server,
       * default to [ANY].
       */
      if (mailman_server)
	argv[L_MAILMAN_SERVER + 1] = mailman_server;
      else if ((mailman == 1) && !strcmp(argv[L_MAILMAN_SERVER + 1], "[NONE]"))
	argv[L_MAILMAN_SERVER + 1] = "[ANY]";

      if (desc)
	argv[L_DESC + 1] = desc;

      if (memacl)
	{
	  argv[L_MEMACE_NAME + 1] = memacl->name;
	  switch (memacl->type)
	    {
	    case MRCL_M_ANY:
	      status = mr_query("get_user_account_by_login", 1,
				&memacl->name, NULL, NULL);
	      if (status == MR_NO_MATCH)
		argv[L_MEMACE_TYPE + 1] = "LIST";
	      else
		argv[L_MEMACE_TYPE + 1] = "USER";
	      break;

	    case MRCL_M_USER:
	      argv[L_MEMACE_TYPE + 1] = "USER";
	      break;

	    case MRCL_M_LIST:
	      argv[L_MEMACE_TYPE + 1] = "LIST";
	      break;

	    case MRCL_M_KERBEROS:
	      status = mrcl_validate_kerberos_member(argv[L_MEMACE_NAME + 1],
						     &argv[L_MEMACE_NAME + 1]);
	      if (mrcl_get_message())
		mrcl_com_err(whoami);
	      if (status == MRCL_REJECT)
		exit(1);
	      argv[L_MEMACE_TYPE + 1] = "KERBEROS";
	      break;

	    case MRCL_M_NONE:
	      argv[L_MEMACE_TYPE + 1] = argv[L_MEMACE_NAME + 1] = "NONE";
	    }
	}

      if (owner)
	{
	  argv[L_ACE_NAME + 1] = owner->name;
	  switch (owner->type)
	    {
	    case MRCL_M_ANY:
	    case MRCL_M_USER:
	      argv[L_ACE_TYPE + 1] = "USER";
	      status = mr_query("update_list", 17, argv, NULL, NULL);
	      if (owner->type != MRCL_M_ANY || status != MR_USER)
		break;

	    case MRCL_M_LIST:
	      argv[L_ACE_TYPE + 1] = "LIST";
	      status = mr_query("update_list", 17, argv, NULL, NULL);
	      break;

	    case MRCL_M_KERBEROS:
	      argv[L_ACE_TYPE + 1] = "KERBEROS";
	      status = mrcl_validate_kerberos_member(argv[L_ACE_NAME + 1],
						     &argv[L_ACE_NAME + 1]);
	      if (mrcl_get_message())
		mrcl_com_err(whoami);
	      if (status == MRCL_REJECT)
		exit(1);
	      status = mr_query("update_list", 17, argv, NULL, NULL);
	      break;
	    case MRCL_M_NONE:
	      argv[L_ACE_TYPE + 1] = argv[L_ACE_NAME + 1] = "NONE";
	      status = mr_query("update_list", 17, argv, NULL, NULL);
	      break;
	    }
	}
      else
	status = mr_query("update_list", 17, argv, NULL, NULL);

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
        case MRCL_M_ANY:
        case MRCL_M_USER:
          membervec[1] = "USER";
          status = mr_query("delete_member_from_list", 3, membervec,
                            NULL, NULL);
          if (status == MR_SUCCESS)
            break;
          else if ((status != MR_USER && status != MR_NO_MATCH) ||
                   memberstruct->type != MRCL_M_ANY)
            {
              com_err(whoami, status, "while deleting member %s from %s",
                      memberstruct->name, listname);
              success = 0;
              break;
            }
        case MRCL_M_LIST:
          membervec[1] = "LIST";
          status = mr_query("delete_member_from_list", 3, membervec,
                            NULL, NULL);
          if (status == MR_SUCCESS)
            break;
          else if ((status != MR_LIST && status != MR_NO_MATCH) ||
                   memberstruct->type != MRCL_M_ANY)
            {
              if (status == MR_PERM && memberstruct->type == MRCL_M_ANY &&
                  !strcmp(membervec[2], get_username()))
                {
		  /* MRCL_M_ANY means we've fallen through from the user
                   * case. The user is trying to remove himself from a
                   * list, but we got MR_USER or MR_NO_MATCH above,
                   * meaning he's not really on it, and we got MR_PERM
                   * when trying to remove LIST:$USER because he's not
                   * on the acl. That error is useless, so return
                   * MR_NO_MATCH instead. However, this will generate
                   * the wrong error if the user was trying to remove
                   * the list with his username from a list he doesn't
                   * administrate without explicitly specifying
                   * "list:".
                   */
                  status = MR_NO_MATCH;
                }
              com_err(whoami, status, "while deleting member %s from %s",
                      memberstruct->name, listname);
              success = 0;
              break;
            }
        case MRCL_M_STRING:
          membervec[1] = "STRING";
          status = mr_query("delete_member_from_list", 3, membervec,
                            NULL, NULL);
          if (status == MR_STRING && memberstruct->type == MRCL_M_ANY)
            {
              com_err(whoami, 0, " Unable to find member %s to delete from %s",
                      memberstruct->name, listname);
              success = 0;
              if (!strcmp(membervec[0], get_username()))
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
        case MRCL_M_KERBEROS:
          membervec[1] = "KERBEROS";
          status = mr_query("delete_member_from_list", 3, membervec,
                            NULL, NULL);
          if (status == MR_STRING || status == MR_NO_MATCH)
            {
              /* Try canonicalizing the Kerberos principal and trying
               * again.  If we succeed, print the message from mrcl.
               * Otherwise, just pretend we never did this and print
               * the original error message.
               */
              mrcl_validate_kerberos_member(membervec[2], &membervec[2]);
              if (mrcl_get_message())
                {
                  if (mr_query("delete_member_from_list", 3, membervec,
                               NULL, NULL) == MR_SUCCESS)
                    mrcl_com_err(whoami);
                  status = MR_SUCCESS;
                }
            }
          if (status != MR_SUCCESS)
            {
              com_err(whoami, status, "while deleting member %s from %s",
                      memberstruct->name, listname);
              success = 0;
            }
          break;
        case MRCL_M_MACHINE:
          membervec[1] = "MACHINE";
          membervec[2] = canonicalize_hostname(memberstruct->name);
          status = mr_query("delete_member_from_list", 3, membervec,
                            NULL, NULL);
          if (status != MR_SUCCESS)
            {
              com_err(whoami, status, "while deleting member %s from %s",
                      memberstruct->name, listname);
              success = 0;
            }
          free(membervec[2]);
	  break;
	case MRCL_M_ID:
	  membervec[1] = "ID";
	  status = mr_query("delete_member_from_list", 3, membervec,
			    NULL, NULL);
	  if (status != MR_SUCCESS)
	    {
	      com_err(whoami, status, "while deleting member %s from %s",
		      memberstruct->name, listname);
	      success = 0;
	    }
	  break;
	}
    }

  /* Process the add list */
  while (sq_get_data(addlist, &memberstruct))
    {
      /* canonicalize string if necessary */
      if (memberstruct->type != MRCL_M_KERBEROS &&
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
		  if (memberstruct->type == MRCL_M_STRING)
		      memberstruct->type = MRCL_M_ANY;
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
      membervec[3] = memberstruct->tag;
      if (verbose)
	{
	  printf("Adding member ");
	  show_list_member(memberstruct);
	}
      switch (memberstruct->type)
	{
	case MRCL_M_ANY:
	case MRCL_M_USER:
	  membervec[1] = "USER";
	  status = mr_query("add_tagged_member_to_list", 4, membervec,
			    NULL, NULL);
	  if (status == MR_SUCCESS)
	    break;
	  else if (status != MR_USER || memberstruct->type != MRCL_M_ANY)
	    {
	      com_err(whoami, status, "while adding member %s to %s",
		      memberstruct->name, listname);
	      success = 0;
	      break;
	    }
	case MRCL_M_LIST:
	  membervec[1] = "LIST";
	  status = mr_query("add_tagged_member_to_list", 4, membervec,
			    NULL, NULL);
	  if (status == MR_SUCCESS)
	    {
	      if (!strcmp(membervec[0], get_username()))
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
	  else if (status != MR_LIST || memberstruct->type != MRCL_M_ANY)
	    {
	      com_err(whoami, status, "while adding member %s to %s",
		      memberstruct->name, listname);
	      success = 0;
	      break;
	    }
	case MRCL_M_STRING:
	  status = mrcl_validate_string_member(memberstruct->name);
	  if (memberstruct->type == MRCL_M_ANY && status == MRCL_WARN)
	    {
	      /* if user is trying to add something which isn't a
		 remote string, or a list, or a user, and didn't
		 explicitly specify `STRING:', it's probably a typo */
	      com_err(whoami, MR_NO_MATCH, "while adding member %s to %s",
		      memberstruct->name, listname);
	      success = 0;
	      break;
	    }
	  else
	    mrcl_com_err(whoami);

	  if (status == MRCL_REJECT)
	    {
	      success = 0;
	      break;
	    }

	  membervec[1] = "STRING";
	  status = mr_query("add_tagged_member_to_list", 4, membervec,
			    NULL, NULL);
	  if (status != MR_SUCCESS)
	    {
	      com_err(whoami, status, "while adding member %s to %s",
		      memberstruct->name, listname);
	      success = 0;
	    }
	  break;
	case MRCL_M_KERBEROS:
	  membervec[1] = "KERBEROS";
	  status = mrcl_validate_kerberos_member(membervec[2], &membervec[2]);
	  if (mrcl_get_message())
	    mrcl_com_err(whoami);
	  if (status == MRCL_REJECT)
	    {
	      success = 0;
	      break;
	    }
	  status = mr_query("add_tagged_member_to_list", 4, membervec,
			    NULL, NULL);
	  if (status != MR_SUCCESS)
	    {
	      com_err(whoami, status, "while adding member %s to %s",
		      memberstruct->name, listname);
	      success = 0;
	    }
	  free(membervec[2]);
	  break;
	case MRCL_M_MACHINE:
	  membervec[1] = "MACHINE";
	  membervec[2] = canonicalize_hostname(strdup(memberstruct->name));
	  status = mr_query("add_tagged_member_to_list", 4, membervec,
			    NULL, NULL);
	  if (status != MR_SUCCESS)
	    {
	      com_err(whoami, status, "while adding member %s to %s",
		      memberstruct->name, listname);
	      success = 0;
	    }
	  free(membervec[2]);
	  break;
	case MRCL_M_ID:
	  membervec[1] = "ID";
	  status = mrcl_validate_id_member(membervec[1], &membervec[1], membervec[2], &membervec[2]);
	  if (mrcl_get_message())
	    mrcl_com_err(whoami);
	  if (status == MRCL_REJECT)
	    {
	      success = 0;
	      break;
	    }

	  status = mr_query("add_tagged_member_to_list", 4, membervec,
			    NULL, NULL);
	  if (status != MR_SUCCESS)
	    {
	      com_err(whoami, status, "while adding member %s to %s",
		      memberstruct->name, listname);
	      success = 0;
	    }
	  free(membervec[1]);
	  free(membervec[2]);
	  break;
	}
    }

  /* Process the tag list */
  while (sq_get_data(taglist, &memberstruct))
    {
      membervec[0] = listname;
      membervec[2] = memberstruct->name;
      membervec[3] = memberstruct->tag;
      if (verbose)
	{
	  printf("Tagging member ");
	  show_list_member(memberstruct);
	}
      switch (memberstruct->type)
	{
	case MRCL_M_ANY:
	case MRCL_M_USER:
	  membervec[1] = "USER";
	  status = mr_query("tag_member_of_list", 4, membervec,
			    NULL, NULL);
	  if (status == MR_SUCCESS)
	    break;
	  else if ((status != MR_USER && status != MR_NO_MATCH) ||
		   memberstruct->type != MRCL_M_ANY)
	    {
	      com_err(whoami, status, "while changing tag on member %s of %s",
		      memberstruct->name, listname);
	      success = 0;
	      break;
	    }
	case MRCL_M_LIST:
	  membervec[1] = "LIST";
	  status = mr_query("tag_member_of_list", 4, membervec,
			    NULL, NULL);
	  if (status == MR_SUCCESS)
	    break;
	  else if ((status != MR_LIST && status != MR_NO_MATCH) ||
		   memberstruct->type != MRCL_M_ANY)
	    {
	      com_err(whoami, status, "while changing tag on member %s of %s",
		      memberstruct->name, listname);
	      success = 0;
	      break;
	    }
	case MRCL_M_STRING:
	  membervec[1] = "STRING";
	  status = mr_query("tag_member_of_list", 4, membervec,
			    NULL, NULL);
	  if (status == MR_STRING && memberstruct->type == MRCL_M_ANY)
	    {
	      com_err(whoami, 0, " Unable to find member %s on list %s",
		      memberstruct->name, listname);
	      success = 0;
	    }
	  else if (status != MR_SUCCESS)
	    {
	      com_err(whoami, status, "while retagging member %s on %s",
		      memberstruct->name, listname);
	      success = 0;
	    }
	  break;
	case MRCL_M_KERBEROS:
	  membervec[1] = "KERBEROS";
	  status = mr_query("tag_member_of_list", 4, membervec,
			    NULL, NULL);
	  if (status == MR_STRING || status == MR_NO_MATCH)
	    {
	      /* Try canonicalizing the Kerberos principal and trying
	       * again.  If we succeed, print the message from mrcl.
	       * Otherwise, just pretend we never did this and print 
	       * the original error message.
	       */
	      mrcl_validate_kerberos_member(membervec[2], &membervec[2]);
	      if (mrcl_get_message())
		{
		  if (mr_query("tag_member_of_list", 4, membervec,
			       NULL, NULL) == MR_SUCCESS)
		    mrcl_com_err(whoami);
		  status = MR_SUCCESS;
		}
	    }
	  if (status != MR_SUCCESS)
	    {
	      com_err(whoami, status, "while changing tag on member %s of %s",
		      memberstruct->name, listname);
	      success = 0;
	    }
	case MRCL_M_MACHINE:
	  membervec[1] = "MACHINE";
	  status = mr_query("tag_member_of_list", 4, membervec,
			    NULL, NULL);
	  if (status != MR_SUCCESS)
	    {
	      com_err(whoami, status, "while adding member %s to %s",
		      memberstruct->name, listname);
	      success = 0;
	    }
	  break;
	case MRCL_M_ID:
	  membervec[1] = "ID";
	  status = mr_query("tag_member_of_list", 4, membervec,
			    NULL, NULL);
	  if (status != MR_SUCCESS)
	    {
	      com_err(whoami, status, "while adding member %s to %s",
		      memberstruct->name, listname);
	      success = 0;
	    }
	  break;
	}
    }

  /* Display the members of the list now, if requested */
  if (memberflg)
    {
      if (recursflg)
	recursive_display_list_members();
      else
	{
	  status = mr_query(showtags ? "get_tagged_members_of_list" :
			    "get_members_of_list", 1, &listname,
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
#define USAGE_OPTIONS_FORMAT "  %-39s%s\n"
  fprintf(stderr, "Usage: %s listname [options]\n", argv[0]);
  fprintf(stderr, "Options are\n");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-v    | -verbose",
	  "-C   | -create");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-m    | -members",
	  "-R   | -rename newname");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-u    | -users",
	  "-P   | -public");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-l    | -lists",
	  "-NP  | -private");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-s    | -strings",
	  "-A   | -active");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-k    | -kerberos",
	  "-I   | -inactive");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-mach | -machines",
	  "-id  | -ids");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-i    | -info",
	  "-V   | -visible");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-r    | -recursive",
	  "-H   | -hidden");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-a    | -add member",
	  "-M   | -mail");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-d    | -delete member",
	  "-NM  | -notmail");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-al   | -addlist filename",
	  "-G   | -group");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-dl   | -deletelist filename",
	  "-NG  | -notgroup");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-f    | -file filename",
	  "-N   | -nfs");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-at   | -addtagged member tag",
	  "-NN  | -notnfs");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-ct   | -changetag member tag",
	  "-mm  | -mailman");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-t    | -tags",
	  "-nmm | -notmailman");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-D    | -desc description",
	  "-ms  | -mailman_server server");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-O    | -owner owner",
	  "-MA  | -memacl membership_acl"); 
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-p    | -pacs",
	  "-np  | -notpacs"); 
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-n    | -noauth",
	  "-db  | -database host[:port]");
  exit(1);
}


/* Display the members stored in the queue */

void show_list_member(struct mrcl_ace_type *memberstruct)
{
  char *s = "";

  switch (memberstruct->type)
    {
    case MRCL_M_USER:
      if (!showusers)
	return;
      s = "USER";
      break;
    case MRCL_M_LIST:
      if (!showlists)
	return;
      s = "LIST";
      break;
    case MRCL_M_STRING:
      if (!showstrings)
	return;
      s = "STRING";
      break;
    case MRCL_M_KERBEROS:
      if (!showkerberos)
	return;
      s = "KERBEROS";
      break;
    case MRCL_M_MACHINE:
      if (!showmachines)
	return;
      s = "MACHINE";
      break;
    case MRCL_M_ID:
      if (!showids)
	return;
      s = "ID";
      break;
    case MRCL_M_ANY:
      printf("%s\n", memberstruct->name);
      return;
    }

  if (verbose)
    printf("%s:%s", s, memberstruct->name);
  else
    {
      if (memberstruct->type == MRCL_M_LIST)
	printf("LIST:%s", memberstruct->name);
      else if (memberstruct->type == MRCL_M_KERBEROS)
	printf("KERBEROS:%s", memberstruct->name);
      else if (memberstruct->type == MRCL_M_STRING &&
	       !strchr(memberstruct->name, '@'))
	printf("STRING:%s", memberstruct->name);
      else if (memberstruct->type == MRCL_M_MACHINE)
	printf("MACHINE:%s", memberstruct->name);
      else if (memberstruct->type == MRCL_M_ID)
	printf("ID:%s", memberstruct->name);
      else
	printf("%s", memberstruct->name);
    }
  if (showtags && *(memberstruct->tag))
    printf(" (%s)\n", memberstruct->tag);
  else
    printf("\n");
}


/* Show the retrieved information about a list */

int show_list_info(int argc, char **argv, void *hint)
{
  printf("List: %s\n", argv[L_NAME]);
  printf("Description: %s\n", argv[L_DESC]);
  printf("Flags: %s, %s, and %s\n",
	 atoi(argv[L_ACTIVE]) ? "active" : "inactive",
	 atoi(argv[L_PUBLIC]) ? "public" : "private",
	 atoi(argv[L_HIDDEN]) ? "hidden" : "visible");
  printf("%s is %sa maillist and is %sa group", argv[L_NAME],
	 atoi(argv[L_MAILLIST]) ? "" : "not ",
	 atoi(argv[L_GROUP]) ? "" : "not ");
  if (atoi(argv[L_GROUP]))
    {
      if (atoi(argv[L_NFSGROUP]))
	printf(" (and an NFS group)");
      printf(" with GID %d\n", atoi(argv[L_GID]));
    }
  else
    printf("\n");
  if (atoi(argv[L_MAILMAN]))
    printf("%s is a Mailman list on server %s\n", argv[L_NAME],
	   argv[L_MAILMAN_SERVER]);
  printf("%s is %sa PACS list\n", argv[L_NAME], 
	 atoi(argv[L_PACSLIST]) ? "" : "not ");
  printf("Owner: %s %s\n", argv[L_ACE_TYPE], argv[L_ACE_NAME]);
  if (strcmp(argv[L_MEMACE_TYPE], "NONE"))
    printf("Membership ACL: %s %s\n", argv[L_MEMACE_TYPE], 
	   argv[L_MEMACE_NAME]);
  printf("Last modified by %s with %s on %s\n", 
	 argv[L_MODBY], argv[L_MODWITH], argv[L_MODTIME]);
  return MR_CONT;
}


/* Copy retrieved information about a list into a new argv */

int save_list_info(int argc, char **argv, void *hint)
{
  char **nargv = hint;

  for (argc = 0; argc < 16; argc++)
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
  struct mrcl_ace_type *m, *m1, *data;

  lists = sq_create();
  members = sq_create();
  m = malloc(sizeof(struct mrcl_ace_type));
  m->type = MRCL_M_LIST;
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
	  if (m1->type == MRCL_M_LIST)
	    unique_add_member(lists, m1);
	  else
	    unique_add_member(members, m1);
	}
    }
  savecount = count = sq_count_elts(members);
  data = malloc(count * sizeof(struct mrcl_ace_type));
  count = 0;
  while (sq_get_data(members, &m))
    memcpy(&data[count++], m, sizeof(struct mrcl_ace_type));
  qsort(data, count, sizeof(struct mrcl_ace_type), membercmp);
  for (count = 0; count < savecount; count++)
    show_list_member(&data[count]);
}


/* add a struct mrcl_ace_type to a queue if that member isn't already there. */

void unique_add_member(struct save_queue *q, struct mrcl_ace_type *m)
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
  struct mrcl_ace_type *m;

  m = malloc(sizeof(struct mrcl_ace_type));
  switch (argv[0][0])
    {
    case 'U':
      m->type = MRCL_M_USER;
      break;
    case 'L':
      m->type = MRCL_M_LIST;
      break;
    case 'S':
      m->type = MRCL_M_STRING;
      break;
    case 'K':
      m->type = MRCL_M_KERBEROS;
      break;
    case 'M':
      m->type = MRCL_M_MACHINE;
      break;
    case 'I':
      m->type = MRCL_M_ID;
      break;
    }
  m->name = strdup(argv[1]);
  if (argc == 3)
    m->tag = strdup(argv[2]);
  else
    m->tag = strdup("");
  sq_save_data(q, m);
  return MR_CONT;
}


/* Open file, parse members from file, and put them on the specified queue */
void get_members_from_file(char *filename, struct save_queue *queue)
{
  FILE *in;
  char buf[BUFSIZ];
  struct mrcl_ace_type *memberstruct;

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
      memberstruct = mrcl_parse_member(buf);
      if (memberstruct)
	sq_save_data(queue, memberstruct);
      else
	com_err(whoami, 0, "Unable to parse member: invalid format.");
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
  const struct mrcl_ace_type *m1 = mem1, *m2 = mem2;

  if (m1->type == MRCL_M_ANY || m2->type == MRCL_M_ANY || (m1->type == m2->type))
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

char *get_username(void)
{
  char *username;

  username = getenv("USER");
  if (!username)
    {
      username = mrcl_krb_user();
      if (!username)
	{
	  com_err(whoami, 0, "Could not determine username");
	  exit(1);
	}
    }
  return username;
}
