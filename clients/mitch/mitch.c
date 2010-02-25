/* $Id: mitch.c 3956 2010-01-05 20:56:56Z zacheiss $
 *
 * Command line oriented Moira containers tool.
 *
 * Garry Zacheiss <zacheiss@mit.edu>, January 2003
 *
 * Inspired by blanche
 *
 * Copyright (C) 2002 by the Massachusetts Institute of Technology.
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

RCSID("$HeadURL: svn+ssh://svn.mit.edu/moira/trunk/moira/clients/mitch/mitch.c $ $Id: mitch.c 3956 2010-01-05 20:56:56Z zacheiss $");

struct owner_type {
  int type;
  char *name;
};

struct mqelem {
  struct mqelem *q_forw;
  struct mqelem *q_back;
  void *q_data;
};

struct string_list {
  char *string;
  struct string_list *next;
};

#define M_ANY		0
#define M_USER		1
#define M_LIST		2
#define M_KERBEROS	3
#define M_NONE		4

char *typename[] = { "ANY", "USER", "LIST", "KERBEROS", "NONE" };

/* argument parsing macro */
#define argis(a, b) (!strcmp(*arg + 1, a) || !strcmp(*arg + 1, b))

/* flags from command line */
int info_flag, update_flag, create_flag, delete_flag, list_subcon_flag;
int list_mach_flag, update_mach_flag, verbose, noauth, unformatted_flag;
int recurse_flag;

struct string_list *container_add_queue, *container_remove_queue;

char *containername, *whoami;

char *newname, *desc, *location, *contact;
int public;
struct owner_type *owner, *memacl;

void usage(char **argv);
int store_container_info(int argc, char **argv, void *hint);
void show_container_info(char **argv);
int show_container_list(int argc, char **argv, void *hint);
void show_container_info_unformatted(char **argv);
int show_container_list_unformatted(int argc, char **argv, void *hint);
int show_machine_in_container(int argc, char **argv, void *hint);
int show_subcontainers_of_container(int argc, char **argv, void *hint);
struct owner_type *parse_member(char *s);
struct string_list *add_to_string_list(struct string_list *old_list, char *s);
int wrap_mr_query(char *handle, int argc, char **argv,
		  int (*callback)(int, char **, void *), void *callarg);
void print_query(char *query_name, int argc, char **argv);

int main(int argc, char **argv)
{
  int status, success;
  char **arg = argv;
  char *server = NULL;

  /* clear all flags & lists */
  info_flag = update_flag = create_flag = delete_flag = list_subcon_flag = 0;
  list_mach_flag = update_mach_flag = verbose = noauth = unformatted_flag = 0;
  recurse_flag = 0;
  public = -1;
  container_add_queue = container_remove_queue = NULL;
  newname = desc = location = contact = NULL;
  owner = memacl = NULL;
  whoami = argv[0];

  success = 1;

  /* parse args */
  while (++arg - argv < argc)
    {
      if (**arg == '-')
	{
	  if (argis("i", "info"))
	    info_flag++;
	  else if (argis("C", "create"))
	    create_flag++;
	  else if (argis("D", "delete"))
	    delete_flag++;
	  else if (argis("R", "rename")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      update_flag++;
	      newname = *arg;
	    } else
	      usage(argv);
	  }
	  else if (argis("d", "desc")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      update_flag++;
	      desc = *arg;
	    } else
	      usage(argv);
	  }
	  else if (argis("L", "location")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      update_flag++;
	      location = *arg;
	    } else
	      usage(argv);
	  }
	  else if (argis("c", "contact")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      update_flag++;
	      contact = *arg;
	    } else
	      usage(argv);
	  }
	  else if (argis("P", "public"))
	    {
	      update_flag++;
	      public = 1;
	    }
	  else if (argis("NP", "private"))
	    {
	      update_flag++;
	      public = 0;
	    }
	  else if (argis("O", "owner")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      update_flag++;
	      owner = parse_member(*arg);
	    } else
	      usage(argv);
	  }
	  else if (argis("MA", "memacl")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      update_flag++;
	      memacl = parse_member(*arg);
	    } else
	      usage(argv);
	  }
	  else if (argis("ls", "listsub"))
	    list_subcon_flag++;
	  else if (argis("lm", "listmach"))
	    list_mach_flag++;
	  else if (argis("am", "addmach")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      container_add_queue = 
		add_to_string_list(container_add_queue, *arg);
	    } else
	      usage(argv);
	    update_mach_flag++;
	  }
	  else if (argis("dm", "deletemach")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      container_remove_queue =
		add_to_string_list(container_remove_queue, *arg);
	    } else
	      usage(argv);
	    update_mach_flag++;
	  }
	  else if (argis("r", "recursive"))
	    recurse_flag++;
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
      else if (containername == NULL)
	containername = *arg;
      else
	usage(argv);
    }
  if (containername == NULL)
    usage(argv);

  /* default to info_flag if nothing else was specified */
  if(!(info_flag        || update_flag    || create_flag || delete_flag || \
       list_subcon_flag || list_mach_flag || update_mach_flag)) {
    info_flag++;
  }

  /* fire up Moira */
  status = mrcl_connect(server, "mitch", 9, !noauth);
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
      char *argv[15];
      int cnt;

      for (cnt = 0; cnt < 11; cnt ++) {
	argv[cnt] = "";
      }

      argv[CON_NAME] = containername;
      argv[CON_PUBLIC] = (public == 1) ? "1" : "0";
      argv[CON_DESCRIPT] = desc ? desc : "none";
      if (location)
	argv[CON_LOCATION] = location;
      if (contact)
	argv[CON_CONTACT] = contact;

      if (memacl)
	{
	  if (memacl->type == M_ANY)
	    {
	      status = wrap_mr_query("get_user_account_by_login", 1,
				     &memacl->name, NULL, NULL);
	      if (status == MR_NO_MATCH)
		memacl->type = M_LIST;
	      else
		memacl->type = M_USER;
	    }
	  argv[CON_MEMACE_TYPE] = typename[memacl->type];
	  argv[CON_MEMACE_NAME] = memacl->name;
	  if (memacl->type == M_KERBEROS)
	    {
	      status = mrcl_validate_kerberos_member(argv[CON_MEMACE_NAME],
						     &argv[CON_MEMACE_NAME]);
	      if (mrcl_get_message())
		mrcl_com_err(whoami);
	      if (status == MRCL_REJECT)
		exit(1);
	    }
	}
      else
	argv[CON_MEMACE_TYPE] = argv[CON_MEMACE_NAME] = "NONE";

      if (owner)
	{
	  argv[CON_OWNER_NAME] = owner->name;
	  switch (owner->type)
	    {
	    case M_ANY:
	    case M_USER:
	      argv[CON_OWNER_TYPE] = "USER";
	      status = wrap_mr_query("add_container", 9, argv, NULL, NULL);
	      if (owner->type != M_ANY || status != MR_USER)
		break;

	    case M_LIST:
	      argv[CON_OWNER_TYPE] = "LIST";
	      status = wrap_mr_query("add_container", 9, argv, NULL, NULL);
	      break;

	    case M_KERBEROS:
	      argv[CON_OWNER_TYPE] = "KERBEROS";
	      status = mrcl_validate_kerberos_member(argv[CON_OWNER_TYPE],
						     &argv[CON_OWNER_TYPE]);
	      if (mrcl_get_message())
		mrcl_com_err(whoami);
	      if (status == MRCL_REJECT)
		exit(1);
	      status = wrap_mr_query("add_container", 9, argv, NULL, NULL);
	      break;
	    case M_NONE:
	      argv[CON_OWNER_TYPE] = argv[CON_OWNER_NAME] = "NONE";
	      status = wrap_mr_query("add_containr", 9, argv, NULL, NULL);
	      break;
	    }
	}
      else
	{
	  argv[CON_OWNER_TYPE] = argv[CON_OWNER_NAME] = "NONE";
	  status = wrap_mr_query("add_container", 9, argv, NULL, NULL);
	}

      if (status)
	{
	  com_err(whoami, status, "while creating container.");
	  exit(1);
	}
    }
  else if (update_flag)
    {
      char *old_argv[15];
      char *argv[15];
      char *args[2];

      args[0] = containername;

      status = wrap_mr_query("get_container", 1, args, store_container_info,
			     old_argv);
      if (status)
	{
	  com_err(whoami, status, "while getting container information.");
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

      argv[CON_NAME] = containername;
      if (newname)
	argv[CON_NAME + 1] = newname;
      argv[CON_PUBLIC + 1] = (public == 1) ? "1" : "0";
      if (desc)
	argv[CON_DESCRIPT + 1] = desc;
      if (location)
	argv[CON_LOCATION + 1] = location;
      if (contact)
	argv[CON_CONTACT + 1] = contact;

      if (memacl)
	{
	  if (memacl->type == M_ANY)
	    {
	      status = wrap_mr_query("get_user_account_by_login", 1,
				     &memacl->name, NULL, NULL);
	      if (status == MR_NO_MATCH)
		memacl->type = M_LIST;
	      else
		memacl->type = M_USER;
	    }
	  argv[CON_MEMACE_TYPE + 1] = typename[memacl->type];
	  argv[CON_MEMACE_NAME + 1] = memacl->name;
	  if (memacl->type == M_KERBEROS)
	    {
	      status = mrcl_validate_kerberos_member(argv[CON_MEMACE_NAME + 1],
						     &argv[CON_MEMACE_NAME + 1]);
	      if (mrcl_get_message())
		mrcl_com_err(whoami);
	      if (status == MRCL_REJECT)
		exit(1);
	    }
	}

      if (owner)
	{
	  argv[CON_OWNER_NAME + 1] = owner->name;
	  switch (owner->type)
	    {
	    case M_ANY:
	    case M_USER:
	      argv[CON_OWNER_TYPE + 1] = "USER";
	      status = wrap_mr_query("update_container", 10, argv, NULL, NULL);
	      if (owner->type != M_ANY || status != MR_USER)
		break;

	    case M_LIST:
	      argv[CON_OWNER_TYPE + 1] = "LIST";
	      status = wrap_mr_query("update_container", 10, argv, NULL, NULL);
	      break;

	    case M_KERBEROS:
	      argv[CON_OWNER_TYPE + 1] = "KERBEROS";
	      status = mrcl_validate_kerberos_member(argv[CON_OWNER_NAME + 1],
						     &argv[CON_OWNER_NAME + 1]);
	      if (mrcl_get_message())
		mrcl_com_err(whoami);
	      if (status == MRCL_REJECT)
		exit(1);
	      status = wrap_mr_query("update_container", 10, argv, NULL, NULL);
	      break;
	      
	    case M_NONE:
	      argv[CON_OWNER_TYPE + 1] = argv[CON_OWNER_NAME + 1] = "NONE";
	      status = wrap_mr_query("update_container", 10, argv, NULL, NULL);
	      break;
	    }
	}
      else
	status = wrap_mr_query("update_container", 10, argv, NULL, NULL);

      if (status)
	{
	  com_err(whoami, status, "while updating container.");
	  success = 0;
	}
      else if (newname)
	containername = newname;
    }

  /* add machines to container */
  if (container_add_queue) {
    struct string_list *q = container_add_queue;

    while (q) {
      char *args[2];

      args[0] = canonicalize_hostname(strdup(q->string));
      args[1] = containername;
      status = wrap_mr_query("add_machine_to_container", 2, args, NULL, NULL);
      if (status)
	{
	  com_err(whoami, status, "while adding machine to container.");
	  exit(1);
	}

      q = q->next;
    }
  }

  /* delete machines from container */
  if (container_remove_queue) {
    struct string_list *q = container_remove_queue;

    while (q) {
      char *args[2];

      args[0] = canonicalize_hostname(strdup(q->string));
      args[1] = containername;
      status = wrap_mr_query("delete_machine_from_container", 2, args, NULL,
			     NULL);
      if (status)
	{
	  com_err(whoami, status, "while removing machine from container.");
	  exit(1);
	}

      q = q->next;
    }
  }

  if (info_flag)
    {
      char *args[2];
      char *argv[20];

      args[0] = containername;
      status = wrap_mr_query("get_container", 1, args, store_container_info,
			     argv);
      if (status)
	{
	  com_err(whoami, status, "while getting container information.");
	  exit(1);
	}

      if (unformatted_flag)
	show_container_info_unformatted(argv);
      else
	show_container_info(argv);
    }

  if (delete_flag)
    {
      char *args[2];

      args[0] = containername;
      status = wrap_mr_query("delete_container", 1, args, NULL, NULL);
      if (status)
	{
	  com_err(whoami, status, "while deleting container.");
	  exit(1);
	}
    }

  if (list_mach_flag)
    {
      char *args[3];

      args[0] = containername;
      args[1] = (recurse_flag == 1) ? "1" : "0";
      status = wrap_mr_query("get_machines_of_container", 2, args,
			     show_machine_in_container, NULL);
      if (status)
	{
	  com_err(whoami, status, "while getting machines of container.");
	  exit(1);
	}
    }

  if (list_subcon_flag)
    {
      char *args[3];

      args[0] = containername;
      args[1] = (recurse_flag == 1) ? "1" : "0";
      status = wrap_mr_query("get_subcontainers_of_container", 2, args,
			     show_subcontainers_of_container, NULL);
      if (status)
	{
	  com_err(whoami, status, "while getting subcontainers of container.");
	  exit(1);
	}
    }

  mr_disconnect();
  exit(success ? 0 : 1);
}

void usage(char **argv)
{
#define USAGE_OPTIONS_FORMAT "  %-39s%s\n"
  fprintf(stderr, "Usage: %s containername [options]\n", argv[0]);
  fprintf(stderr, "Options are\n");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-C   | -create",
	  "-O   | -owner owner");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-D   | -delete",
	  "-d   | -desc description");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-R   | -rename newname",
	  "-L   | -location location");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-i   | -info",
	  "-c   | -contact contact");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-P   | -public",
	  "-NP  | -private");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-O   | -owner owner",
	  "-MA  | -memacl membership_acl");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-ls  | -listsub",
	  "-lm  | -listmach");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-am  | -addmach hostname",
	  "-dm  | -deletemach hostname");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-r   | -recursive",
	  "-u   | -unformatted");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-v   | -verbose",
	  "-n   | -noauth");
  fprintf(stderr, "  %-39s\n" , "-db  | -database host[:port]");
  exit(1);
}

/* Retrieve information about a container */
int store_container_info(int argc, char **argv, void *hint)
{
  int i;
  char **nargv = hint;

  for(i = 0; i < argc; i++)
    nargv[i] = strdup(argv[i]);

  return MR_CONT;
}

void show_container_info(char **argv)
{
  char tbuf[256];
  char *args[2];
  struct mqelem *elem = NULL;
  int status;

  printf("Container:      %-16s    Public:      %s\n", argv[CON_NAME], 
	 argv[CON_PUBLIC]);
  args[0] = argv[CON_NAME];
  status = wrap_mr_query("get_container_list", 1, args, show_container_list,
			 &elem);
  if (status && status != MR_NO_MATCH)
    com_err(whoami, status, "while getting container list.");
  printf("Description:    %-16s\n", argv[CON_DESCRIPT]);
  printf("Location:       %-16s    Contact:     %s\n", argv[CON_LOCATION],
	 argv[CON_CONTACT]);
  sprintf(tbuf, "%s %s", argv[CON_OWNER_TYPE],
	  strcmp(argv[CON_OWNER_TYPE], "NONE") ? argv[CON_OWNER_NAME] : "");
  printf("Owner:          %-16s\n", tbuf);
  sprintf(tbuf, "%s %s", argv[CON_MEMACE_TYPE],
	  strcmp(argv[CON_MEMACE_TYPE], "NONE") ? argv[CON_MEMACE_NAME] : "");
  printf("Membership ACL: %-16s\n", tbuf);
  printf("\n");
  printf("Last mod by %s at %s with %s.\n", argv[CON_MODBY], argv[CON_MODTIME],
	 argv[CON_MODWITH]);
}

int show_container_list(int argc, char **argv, void *hint)
{
  printf("Container's associated list is: LIST %s\n", argv[1]);

  return MR_CONT;
}

void show_container_info_unformatted(char **argv)
{
  char *args[2];
  struct mqelem *elem = NULL;
  int status;

  printf("Container:       %s\n", argv[CON_NAME]);
  args[0] = argv[CON_NAME];
  status = wrap_mr_query("get_container_list", 1, args,
			 show_container_list_unformatted, &elem);
  if (status && status != MR_NO_MATCH)
    com_err(whoami, status, "while getting container list.");
  else
    printf("\n");
  printf("Public:          %s\n", argv[CON_PUBLIC]);
  printf("Description:     %s\n", argv[CON_DESCRIPT]);
  printf("Location:        %s\n", argv[CON_LOCATION]);
  printf("Contact:         %s\n", argv[CON_CONTACT]);
  printf("Owner Type:      %s\n", argv[CON_OWNER_TYPE]);
  printf("Owner:           %s\n", argv[CON_OWNER_NAME]);
  printf("Memacl Type:     %s\n", argv[CON_MEMACE_TYPE]);
  printf("Memacl:          %s\n", argv[CON_MEMACE_NAME]);
  printf("Last mod by:     %s\n", argv[CON_MODBY]);
  printf("Last mod on:     %s\n", argv[CON_MODTIME]);
  printf("Last mod with:   %s\n", argv[CON_MODWITH]);
}

int show_container_list_unformatted(int argc, char **argv, void *hint)
{
  printf("Associated list: %s", argv[1]);

  return MR_CONT;
}

int show_machine_in_container(int argc, char **argv, void *hint)
{
  printf("Machine: %-30s Container: %-25s\n", argv[0], argv[1]); 

  return MR_CONT;
}

int show_subcontainers_of_container(int argc, char **argv, void *hint)
{
  printf("Container: %-25s\n", argv[0]);

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
  for(cnt=0; cnt<argc; cnt++)
    printf(" <%s>", argv[cnt]);
  printf("\n");
}
