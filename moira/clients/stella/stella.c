/* $Id$
 *
 * Command line oriented Moira host tool.
 *
 * kolya@MIT.EDU, January 2000
 *
 * Somewhat based on blanche
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

#ifdef _WIN32
typedef unsigned long in_addr_t;
#else
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#endif

RCSID("$HeadURL$ $Id$");

struct mqelem {
  struct mqelem *q_forw;
  struct mqelem *q_back;
  void *q_data;
};

struct string_list {
  char *string;
  struct string_list *next;
};

/* argument parsing macro */
#define argis(a, b) (!strcmp(*arg + 1, a) || !strcmp(*arg + 1, b))

/* flags from command line */
int info_flag, update_flag, create_flag, delete_flag, list_map_flag;
int update_alias_flag, update_address_flag, update_map_flag, verbose, noauth;
int list_container_flag, update_container_flag, unformatted_flag;
int list_identifier_flag, update_identifier_flag;
int list_resource_records_flag, update_resource_records_flag;
int set_host_opt_flag, set_ttl_flag, set_ptr_flag;
int add_dynamic_host_flag, show_host_usage_flag;

struct string_list *alias_add_queue, *alias_remove_queue;
struct string_list *address_add_queue, *address_remove_queue;
struct string_list *map_add_queue, *map_remove_queue;
struct string_list *container_add_queue, *container_remove_queue;
struct string_list *identifier_add_queue, *identifier_remove_queue;
struct string_list *record_add_queue, *record_remove_queue;

char *hostname, *whoami;

char *newname, *address, *network, *h_status, *vendor, *model;
char *os, *location, *contact, *billing_contact, *account_number;
char *adm_cmt, *op_cmt, *opt, *ttl, *ptr;

struct sockaddr_in sa;
struct sockaddr_in6 sa6;
struct mrcl_ace_type *owner;
struct mrcl_identifier_type *identifier;

void usage(char **argv);
int store_host_info(int argc, char **argv, void *hint);
void show_host_info(char **argv);
void show_host_info_unformatted(char **argv);
int show_machine_in_cluster(int argc, char **argv, void *hint);
int show_machine_in_container(int argc, char **argv, void *hint);
int show_host_identifiers(int argc, char **argv, void *hint);
int show_host_rr(int argc, char **argv, void *hint);
int show_host_usage(int argc, char **argv, void *hint);
int show_dynamic_hostname(int argc, char **argv, void *hint);
struct string_list *add_to_string_list(struct string_list *old_list, char *s);
int wrap_mr_query(char *handle, int argc, char **argv,
		  int (*callback)(int, char **, void *), void *callarg);
void print_query(char *query_name, int argc, char **argv);
char *get_username(void);
int store_query_info(int argc, char **argv, void *data);
struct mqelem *queue_top(struct mqelem *elem);

int main(int argc, char **argv)
{
  int status, success;
  char **arg = argv;
  char *server = NULL;

  /* clear all flags & lists */
  info_flag = update_flag = create_flag = list_map_flag = update_map_flag = 0;
  update_alias_flag = update_address_flag = verbose = noauth = 0;
  list_container_flag = update_container_flag = list_resource_records_flag = 0;
  list_identifier_flag = update_identifier_flag = show_host_usage_flag = 0;
  set_host_opt_flag = set_ttl_flag = set_ptr_flag = add_dynamic_host_flag = 0;
  newname = address = network = h_status = vendor = model = NULL;
  os = location = contact = billing_contact = account_number = adm_cmt = NULL;
  op_cmt = opt = NULL;
  owner = NULL;
  alias_add_queue = alias_remove_queue = NULL;
  address_add_queue = address_remove_queue = NULL;
  map_add_queue = map_remove_queue = NULL;
  container_add_queue = container_remove_queue = NULL;
  identifier_add_queue = identifier_remove_queue = NULL;
  record_add_queue = record_remove_queue = NULL;
  whoami = argv[0];

  success = 1;

  /* parse args, building addlist, dellist, & synclist */
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
	  else if (argis("O", "owner")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      update_flag++;
	      owner = mrcl_parse_member(*arg);
	      if (!owner || owner->type == MRCL_M_STRING || owner->type == MRCL_M_MACHINE)
		{
		  com_err(whoami, 0, "Invalid owner format. Must be one of USER, LIST, KERBEROS, or NONE.");
		  exit(1);
		}
	    } else
	      usage(argv);
	  }
	  else if (argis("S", "status")) {
	    if (arg - argv < argc - 1) {
	      int i;
	      int len;

	      arg++;
	      update_flag++;
	      h_status = *arg;

	      len = strlen(h_status);
	      for(i = 0; i < len; i++) {
		if(!isdigit(h_status[i])) {
		  printf("Error: status code %s is not numeric.\n", h_status);
		  exit(1);
		}
	      }
	    } else
	      usage(argv);
	  }
	  else if (argis("V", "vendor")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      update_flag++;
	      vendor = *arg;
	    } else
	      usage(argv);
	  }
	  else if (argis("M", "model")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      update_flag++;
	      model = *arg;
	    } else
	      usage(argv);
	  }
	  else if (argis("o", "os")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      update_flag++;
	      os = *arg;
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
	  else if (argis("bc", "billingcontact")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      update_flag++;
	      billing_contact = *arg;
	    } else
	      usage(argv);
	  }
	  else if (argis("an", "accountnumber")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      update_flag++;
	      account_number = *arg;
	    } else
	      usage(argv);
	  }
	  else if (argis("ac", "admcmt")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      update_flag++;
	      adm_cmt = *arg;
	    } else
	      usage(argv);
	  }
          /* This could be for either update_host or set_host_opt *
	     Don't set any flags and take our cues from the other *
	     arguments we're given.  */
	  else if (argis("oc", "opcmt")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      op_cmt = *arg;
	    } else
	      usage(argv);
	  }
	  else if (argis("a", "addalias")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      alias_add_queue=add_to_string_list(alias_add_queue, *arg);
	    } else
	      usage(argv);
	    update_alias_flag++;
	  }
	  else if (argis("d", "deletealias")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      alias_remove_queue=add_to_string_list(alias_remove_queue, *arg);
	    } else
	      usage(argv);
	    update_alias_flag++;
	  }
	  else if (argis("A", "address")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      address_add_queue=add_to_string_list(address_add_queue, *arg);
	    } else
	      usage(argv);
	    update_address_flag++;
	  }
	  else if (argis("DA", "deleteaddress")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      address_remove_queue=add_to_string_list(address_remove_queue, *arg);
	    } else
	      usage(argv);
	    update_address_flag++;
	  }
	  else if (argis("am", "addmap")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      map_add_queue=add_to_string_list(map_add_queue, *arg);
	    } else
	      usage(argv);
	    update_map_flag++;
	  }
	  else if (argis("dm", "deletemap")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      map_remove_queue=add_to_string_list(map_remove_queue, *arg);
	    } else
	      usage(argv);
	    update_map_flag++;
	  }
	  else if (argis("lm", "listmap"))
	    list_map_flag++;
	  else if (argis("acn", "addcontainer")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      container_add_queue = 
		add_to_string_list(container_add_queue, *arg);
	    } else
	      usage(argv);
	    update_container_flag++;
	  }
	  else if (argis("dcn", "deletecontainer")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      container_remove_queue =
		add_to_string_list(container_remove_queue, *arg);
	    } else
	      usage(argv);
	    update_container_flag++;
	  }
	  else if (argis("lcn", "listcontainer"))
	    list_container_flag++;
	  else if (argis("aid", "addidentifier")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      identifier_add_queue = add_to_string_list(identifier_add_queue, *arg);
	    } else
	      usage(argv);
	    update_identifier_flag++;
	  }
	  else if (argis("did", "delidentifier")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      identifier_remove_queue = add_to_string_list(identifier_remove_queue, *arg);
	    } else
	      usage(argv);
	    update_identifier_flag++;
	  }
          else if (argis("arr", "addrecord")) {
            if (arg - argv < argc - 1) {
              arg++;
              record_add_queue = add_to_string_list(record_add_queue, *arg);
            } else
              usage(argv);
            update_resource_records_flag++;
          }
          else if (argis("drr", "delrecord")) {
            if (arg - argv < argc - 1) {
              arg++;
              record_remove_queue = add_to_string_list(record_remove_queue, *arg);
            } else
              usage(argv);
            update_resource_records_flag++;
          }
	  else if (argis("oi", "optin")) {
	    set_host_opt_flag++;
	    opt = "0";
	  }
	  else if (argis("oo", "optout")) {
	    set_host_opt_flag++;
	    opt = "1";
	  }
	  else if (argis("opt", "option")) {
            if (arg - argv < argc - 1) {
              arg++;
              set_host_opt_flag++;
              opt = *arg;
            } else
              usage(argv);
          }
	  else if (argis("ttl", "setttl")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      set_ttl_flag++;
	      ttl = *arg;
	    } else
	      usage(argv);
	  }
	  else if (argis("ptr", "setptr")) {
	    if (arg - argv < argc - 1) {
	      arg++;
	      set_ptr_flag++;
	      ptr = *arg;
	    } else
	      usage(argv);
	  }
	  else if (argis("lid", "listidentifier"))
	    list_identifier_flag++;
	  else if (argis("lrr", "listrecords"))
	    list_resource_records_flag++;
	  else if (argis("su", "showusage"))
	    show_host_usage_flag++;
	  else if (argis("adh", "adddynamic")) {
	    add_dynamic_host_flag++;
            if (arg - argv < argc - 1) {
              arg++;
              owner = mrcl_parse_member(*arg);
	      if (!owner || owner->type == MRCL_M_STRING || owner->type == MRCL_M_MACHINE)
		{
		  com_err(whoami, 0, "Invalid owner format. Must be one of USER, LIST, KERBEROS, or NONE.");
		  exit(1);
		}
            } else {
	      owner = mrcl_parse_member(get_username());
	      if (!owner)
		{
		  com_err(whoami, 0, "Invalid owner format. Must be one of USER, LIST, KERBEROS, or NONE.");
		  exit(1);
		}
	    }
	  }
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
      else if (hostname == NULL)
	hostname = *arg;
      else
	usage(argv);
    }
  if (hostname == NULL && !add_dynamic_host_flag)
    usage(argv);

  if (op_cmt && !set_host_opt_flag)
    update_flag++;

  /* default to info_flag if nothing else was specified */
  if(!(info_flag   || update_flag   || create_flag     || \
       delete_flag || list_map_flag || update_map_flag || \
       update_alias_flag || update_address_flag || update_container_flag || \
       list_container_flag || update_identifier_flag || list_identifier_flag || \
       update_resource_records_flag || list_resource_records_flag || set_host_opt_flag || \
       set_ttl_flag || set_ptr_flag || add_dynamic_host_flag || show_host_usage_flag)) {
    info_flag++;
  }

  if (add_dynamic_host_flag && (info_flag || update_flag || create_flag || \
				delete_flag || list_map_flag || update_map_flag || \
				update_alias_flag || update_address_flag || update_container_flag || \
				list_container_flag || update_identifier_flag || list_resource_records_flag || \
				update_resource_records_flag || list_identifier_flag || set_host_opt_flag || \
				set_ttl_flag || set_ptr_flag ||	show_host_usage_flag))
    {
      com_err(whoami, 0, "-adh / -adddynamic option must be the only argument provided.");
      exit(1);
    }

  /* fire up Moira */
  status = mrcl_connect(server, "stella", 17, !noauth);
  if (status == MRCL_AUTH_ERROR)
    {
      com_err(whoami, 0, "Try the -noauth flag if you don't "
	      "need authentication.");
    }
  if (status)
    exit(2);

  /* If we were given an IP address (v4 or v6) see if we can turn it into a hostname */
  if (hostname)
    {
      if ((inet_pton(AF_INET, hostname, &(sa.sin_addr)) == 1) ||
	  (inet_pton(AF_INET6, hostname, &(sa6.sin6_addr)) == 1))
	{
	  char *args[2];
	  char *argv[30];

	  args[0] = "*";
	  args[1] = strdup(hostname);

	  status = wrap_mr_query("get_host_by_address", 2, args, store_host_info, argv);
	  if (status) {
	    com_err(whoami, status, "while looking up IP address.");
	  } else {
	    hostname = argv[0];
	  }
	}
    }

  /* create if needed */
  if (create_flag)
    {
      char *argv[30];
      int cnt;

      for (cnt = 0; cnt < 13; cnt++) {
	argv[cnt] = "";
      }

      argv[0] = canonicalize_hostname(strdup(hostname));

      if (vendor)
	argv[1] = vendor;
      if (model)
	argv[2] = model;
      if (os)
	argv[3] = os;
      if (location)
	argv[4] = location;
      if (contact)
	argv[5] = contact;
      if (billing_contact)
	argv[6] = billing_contact;
      if (account_number)
	argv[7] = account_number;
      if (h_status)
	argv[8] = h_status;
      else
	argv[8] = "1";
      if (adm_cmt)
	argv[11] = adm_cmt;
      if (op_cmt)
	argv[12] = op_cmt;

      if (owner)
	{
	  argv[10] = owner->name;
	  switch (owner->type)
	    {
	    case MRCL_M_ANY:
	    case MRCL_M_USER:
	      argv[9] = "USER";
	      status = wrap_mr_query("add_host", 13, argv, NULL, NULL);
	      if (owner->type != MRCL_M_ANY || status != MR_USER)
		break;

	    case MRCL_M_LIST:
	      argv[9] = "LIST";
	      status = wrap_mr_query("add_host", 13, argv, NULL, NULL);
	      break;

	    case MRCL_M_KERBEROS:
	      argv[9] = "KERBEROS";
	      status = mrcl_validate_kerberos_member(argv[10], &argv[10]);
	      if (mrcl_get_message())
		mrcl_com_err(whoami);
	      if (status == MRCL_REJECT)
		exit(1);
	      status = wrap_mr_query("add_host", 13, argv, NULL, NULL);
	      break;

	    case MRCL_M_NONE:
	      argv[9] = "NONE";
	      status = wrap_mr_query("add_host", 13, argv, NULL, NULL);
	      break;
	    }
	}
      else
	{
	  argv[9] = "NONE";
	  argv[10] = "NONE";

	  status = wrap_mr_query("add_host", 13, argv, NULL, NULL);
	}

      if (status)
	{
	  com_err(whoami, status, "while creating host.");
	  exit(1);
	}

    }
  else if (update_flag)
    {
      char *old_argv[30];
      char *argv[17];
      char *args[5];

      args[0] = canonicalize_hostname(strdup(hostname));
      args[1] = "*";

      status = wrap_mr_query("get_host", 2, args, store_host_info, old_argv);
      if (status)
	{
	  com_err(whoami, status, "while getting list information");
	  exit(1);
	}

      /* hostname */
      argv[1] = old_argv[0];
      /* vendor */
      argv[2] = old_argv[1];
      /* model */
      argv[3] = old_argv[2];
      /* os */
      argv[4] = old_argv[3];
      /* location */
      argv[5] = old_argv[4];
      /* contact */
      argv[6] = old_argv[5];
      /* billing contact */
      argv[7] = old_argv[6];
      /* account number */
      argv[8] = old_argv[7];
      /* argvs don't line up at this point because update_host doesn't take use as an argument */
      /* status */
      argv[9] = old_argv[9];
      /* skip status_change */
      /* ace_type */
      argv[10] = old_argv[11];
      /* ace_name */
      argv[11] = old_argv[12];
      /* admin comment */
      argv[12] = old_argv[13];
      /* ops comment */
      argv[13] = old_argv[14];

      argv[0] = canonicalize_hostname(strdup(hostname));
      if (newname)
	argv[1] = canonicalize_hostname(strdup(newname));
      if (vendor)
	argv[2] = vendor;
      if (model)
	argv[3] = model;
      if (os)
	argv[4] = os;
      if (location)
	argv[5] = location;
      if (contact)
	argv[6] = contact;
      if (billing_contact)
	argv[7] = billing_contact;
      if (account_number)
	argv[8] = account_number;
      if (h_status)
	argv[9] = h_status;
      if (adm_cmt)
	argv[12] = adm_cmt;
      if (op_cmt)
	argv[13] = op_cmt;

      if (owner)
	{
	  argv[11] = owner->name;
	  switch (owner->type)
	    {
	    case MRCL_M_ANY:
	    case MRCL_M_USER:
	      argv[10] = "USER";
	      status = wrap_mr_query("update_host", 14, argv, NULL, NULL);
	      if (owner->type != MRCL_M_ANY || status != MR_USER)
		break;

	    case MRCL_M_LIST:
	      argv[10] = "LIST";
	      status = wrap_mr_query("update_host", 14, argv, NULL, NULL);
	      break;

	    case MRCL_M_KERBEROS:
	      argv[10] = "KERBEROS";
	      status = mrcl_validate_kerberos_member(argv[11], &argv[11]);
	      if (mrcl_get_message())
		mrcl_com_err(whoami);
	      if (status == MRCL_REJECT)
		exit(1);
	      status = wrap_mr_query("update_host", 14, argv, NULL, NULL);
	      break;

	    case MRCL_M_NONE:
	      argv[10] = "NONE";
	      status = wrap_mr_query("update_host", 14, argv, NULL, NULL);
	      break;
	    }
	}
      else
	status = wrap_mr_query("update_host", 14, argv, NULL, NULL);

      if (status)
	com_err(whoami, status, "while updating host.");
      else if (newname)
	hostname = newname;
    }

  /* create aliases if necessary */
  if (alias_add_queue) {
    struct string_list *q = alias_add_queue;

    while(q) {
      char *alias = q->string;
      char *args[2];

      args[0] = partial_canonicalize_hostname(strdup(alias));
      args[1] = canonicalize_hostname(strdup(hostname));
      status = wrap_mr_query("add_hostalias", 2, args, NULL, NULL);
      if (status) {
	com_err(whoami, status, "while adding host alias");
	exit(1);
      }

      q = q->next;
    }
  }

  /* delete aliases if necessary */
  if (alias_remove_queue) {
    struct string_list *q = alias_remove_queue;

    while(q) {
      char *alias = q->string;
      char *args[2];

      args[0] = partial_canonicalize_hostname(strdup(alias));
      args[1] = canonicalize_hostname(strdup(hostname));
      status = wrap_mr_query("delete_hostalias", 2, args, NULL, NULL);
      if (status) {
	com_err(whoami, status, "while deleting host alias");
	exit(1);
      }

      q = q->next;
    }
  }

  if (address_add_queue) {
    struct string_list *q = address_add_queue;

    while (q) {
      /* string is of the form network:address */
      struct mrcl_netaddr_type *netaddr;
      char *args[3];

      netaddr = mrcl_parse_netaddr(q->string);
      if (!netaddr)
	{
	  com_err(whoami, 0, "Could not parse network address specification while adding host address.");
	  exit(1);
	}

      args[0] = canonicalize_hostname(strdup(hostname));
      args[1] = netaddr->network;
      args[2] = netaddr->address;

      status = wrap_mr_query("add_host_address", 3, args, NULL, NULL);
      if (status) {
	com_err(whoami, status, "while adding host address");
	exit(1);
      }

      q = q->next;
    }
  }

  if (address_remove_queue) {
    struct string_list *q = address_remove_queue;

    while (q) {
      /* string is of the form network:address */
      struct mrcl_netaddr_type *netaddr;
      char *args[3];

      netaddr = mrcl_parse_netaddr(q->string);
      if (!netaddr)
	{
	  com_err(whoami, 0, "Could not parse network address specification while removing host address.");
	  exit(1);
	}

      args[0] = canonicalize_hostname(strdup(hostname));
      args[1] = netaddr->network;
      args[2] = netaddr->address;

      status = wrap_mr_query("delete_host_address", 3, args, NULL, NULL);
      if (status) {
	com_err(whoami, status, "while removing host address");
	exit(1);
      }
      q = q->next;
    }
  }

  /* create cluster mappings */
  if (map_add_queue) {
    struct string_list *q = map_add_queue;

    while(q) {
      char *clustername = q->string;
      char *args[2];

      args[0] = canonicalize_hostname(strdup(hostname));
      args[1] = clustername;
      status = wrap_mr_query("add_machine_to_cluster", 2, args, NULL, NULL);
      if (status) {
	com_err(whoami, status, "while adding cluster mapping");
	exit(1);
      }

      q = q->next;
    }
  }

  /* delete cluster mappings */
  if (map_remove_queue) {
    struct string_list *q = map_remove_queue;

    while(q) {
      char *clustername = q->string;
      char *args[2];

      args[0] = canonicalize_hostname(strdup(hostname));
      args[1] = clustername;
      status = wrap_mr_query("delete_machine_from_cluster", 2, args,
			     NULL, NULL);
      if (status) {
	com_err(whoami, status, "while deleting cluster mapping");
	exit(1);
      }

      q = q->next;
    }
  }

  /* add container mappings */
  if (container_add_queue) {
    struct string_list *q = container_add_queue;

    while (q) {
      char *containername = q->string;
      char *args[2];

      args[0] = canonicalize_hostname(strdup(hostname));
      args[1] = containername;
      status = wrap_mr_query("add_machine_to_container", 2, args,
			     NULL, NULL);

      if (status) {
	com_err(whoami, status, "while adding container mapping");
	exit(1);
      }

      q = q->next;
    }
  }

  /* delete container mappings */
  if (container_remove_queue) {
    struct string_list *q = container_remove_queue;

    while (q) {
      char *containername = q->string;
      char *args[2];

      args[0] = canonicalize_hostname(strdup(hostname));
      args[1] = containername;
      status = wrap_mr_query("delete_machine_from_container", 2, args,
			     NULL, NULL);

      if (status) {
	com_err(whoami, status, "while deleting container mapping");
	exit(1);
      }

      q = q->next;
    }
  }

  /* add identifiers */
  if (identifier_add_queue) {
    struct string_list *q = identifier_add_queue;

    while (q) {
      char *args[3];

      args[0] = canonicalize_hostname(strdup(hostname));

      identifier = mrcl_parse_mach_identifier(q->string);
      args[2] = identifier->value;

      switch (identifier->type)
	{
        case MRCL_MID_ANY:
        case MRCL_MID_HWADDR:
          args[1] = "HWADDR";
          break;

        case MRCL_MID_DUID:
          args[1] = "DUID";
          break;
        }

      status = wrap_mr_query("add_host_identifier", 3, args,
			     NULL, NULL);

      if (status) {
	com_err(whoami, status, "while adding host identifier");
	exit(1);
      }

      q = q->next;
    }
  }

  /* delete identifiers */
  if (identifier_remove_queue) {
    struct string_list *q = identifier_remove_queue;
   
    while (q) {
      char *args[3];

      args[0] = canonicalize_hostname(strdup(hostname));

      identifier = mrcl_parse_mach_identifier(q->string);
      args[2] = identifier->value;

      switch (identifier->type)
        {
        case MRCL_MID_ANY:
        case MRCL_MID_HWADDR:
          args[1] = "HWADDR";
          break;

	case MRCL_MID_DUID:
          args[1] = "DUID";
          break;
        }

      status = wrap_mr_query("delete_host_identifier", 3, args,
			     NULL, NULL);

      if (status) {
	com_err(whoami, status, "while deleting host identifier");
	exit(1);
      }

      q = q->next;
    }
  }

  /* add host resource records */
  if (record_add_queue)
    {
      struct string_list *q = record_add_queue;

      while (q) {
	char *args[3];
	struct mrcl_addropt_type *addropt;

	addropt = mrcl_parse_addropt(q->string);
	if (!addropt)
	  {
	    com_err(whoami, 0, "Could not parse resource record specification while adding host resource record.");
	    exit(1);
	  }

	args[0] = canonicalize_hostname(strdup(hostname));
	args[1] = addropt->address;
	args[2] = addropt->opt;

	status = wrap_mr_query("add_host_resource_record", 3, args, NULL, NULL);
	if (status) {
	  com_err(whoami, status, "while adding host resource record");
	  exit(1);
	}

	q = q->next;
      }
    }

  /* delete host resource records */
  if (record_remove_queue)
    {
      struct string_list *q = record_remove_queue;

      while (q) {
        char *args[3];
        struct mrcl_addropt_type *addropt;

        addropt = mrcl_parse_addropt(q->string);
        if (!addropt)
          {
            com_err(whoami, 0, "Could not parse resource record specification while removing host resource record.");
            exit(1);
          }

	args[0] = canonicalize_hostname(strdup(hostname));
        args[1] = addropt->address;
        args[2] = addropt->opt;

        status = wrap_mr_query("delete_host_resource_record", 3, args, NULL, NULL);
        if (status) {
          com_err(whoami, status, "while removing host resource record");
          exit(1);
        }

        q = q->next;
      }
    }

  /* display host info if requested to */
  if (info_flag) {
    char *args[5];
    char *argv[30];

    args[0] = canonicalize_hostname(strdup(hostname));
    args[1] = "*";
    status = wrap_mr_query("get_host", 2, args, store_host_info, argv);

    /* We might be looking for an alias of a deleted host. */
    if (status && status == MR_NO_MATCH) {
      status = wrap_mr_query("get_hostalias", 2, args, store_host_info, argv);
      if (!status) {
	args[0] = strdup(argv[1]);
	status = wrap_mr_query("get_host", 2, args, store_host_info, argv);
      }
    }

    if (status) {
      com_err(whoami, status, "while getting host information");
      exit(1);
    }

    if (unformatted_flag)
      show_host_info_unformatted(argv);
    else
      show_host_info(argv);
  }

  /* list cluster mappings if needed */
  if (list_map_flag) {
    char *args[3];

    args[0] = canonicalize_hostname(strdup(hostname));
    args[1] = "*";
    status = wrap_mr_query("get_machine_to_cluster_map", 2, args,
		      show_machine_in_cluster, NULL);
    if (status)
      if (status != MR_NO_MATCH) {
        com_err(whoami, status, "while getting cluster mappings");
        exit(1);
      }
  }

  /* list container mappings if needed */
  if (list_container_flag) {
    char *argv[1];

    argv[0] = canonicalize_hostname(strdup(hostname));
    status = wrap_mr_query("get_machine_to_container_map", 1, argv,
			   show_machine_in_container, NULL);

    if (status)
      if (status != MR_NO_MATCH) {
	com_err(whoami, status, "while getting container mappings");
	exit(1);
      }
  }

  /* list identifier mappings if needed */
  if (list_identifier_flag) {
    char *argv[2];

    argv[0] = canonicalize_hostname(strdup(hostname));
    argv[1] = "*";

    status = wrap_mr_query("get_host_identifier_mapping", 2, argv,
			   show_host_identifiers, NULL);

    if (status)
      if (status != MR_NO_MATCH) {
	com_err(whoami, status, "while getting host identifier");
	exit(1);
      }
  }

  /* List DNS resource records */
  if (list_resource_records_flag) {
    char *argv[2];

    argv[0] = canonicalize_hostname(strdup(hostname));
    argv[1] = "*";

    status = wrap_mr_query("get_host_resource_record", 2, argv,
                           show_host_rr, NULL);

    if (status)
      if (status != MR_NO_MATCH) {
        com_err(whoami, status, "while getting host identifier");
        exit(1);
      }
  }

  /* show host usage */
  if (show_host_usage_flag)
    {
      char *argv[1];

      argv[0] = canonicalize_hostname(strdup(hostname));

      status = wrap_mr_query("get_host_usage", 1, argv,
			     show_host_usage, NULL);
      if (status)
	if (status != MR_NO_MATCH) {
	  com_err(whoami, status, "while getting host usage");
	  exit(1);
	}
    }

  if (set_host_opt_flag) {
    char *argv[3];

    /* Must specify non-null admin. comment if opting out */
    if (atoi(opt) == 1 && !op_cmt)
      {
	com_err(whoami, 0, "Must specify operational comment when opting out of default network security policy");
	exit(1);
      }
    else if (!op_cmt)
      op_cmt = "";

    argv[0] = canonicalize_hostname(strdup(hostname));
    argv[1] = opt;
    argv[2] = op_cmt;

    status = wrap_mr_query("shot", 3, argv, NULL, NULL);
    if (status) {
      com_err(whoami, status, "while setting host network security options");
      exit(1);
    }
  }

  if (set_ttl_flag) {
    char *argv[4];
    struct mrcl_addropt_type *addropt;
    struct mqelem *elem = NULL, *top = NULL;

    addropt = mrcl_parse_addropt(ttl);
    if (!addropt)
      {
	com_err(whoami, 0, "Couldn't parse address specification while setting host TTL");
	exit(1);
      }

    argv[0] = canonicalize_hostname(strdup(hostname));
    argv[1] = addropt->address;
    argv[2] = addropt->opt;

    if (!strcasecmp(argv[2], "default"))
      sprintf(argv[2], "%d", DEFAULT_TTL);

    /* Is argv[1] an IP address?  If not, assume it's a DNS RR type */
    if (inet_pton(AF_INET, argv[1], &(sa.sin_addr)) < 1 &&
	inet_pton(AF_INET6, argv[1], &(sa6.sin6_addr)) < 1)
      {
	/* Iterate over all records of this type */
	status = wrap_mr_query("get_host_resource_record", 2, argv, store_query_info, &elem);
	if (status) {
	  com_err(whoami, status, "while getting host resource records");
	  exit(1);
	}

	top = queue_top(elem);
	while (top)
	  {
	    argv[2] = ((char **)top->q_data)[2];
	    argv[3] = addropt->opt;

	    if (!strcasecmp(argv[3], "default"))
	      sprintf(argv[3], "%d", DEFAULT_TTL);

	    status = wrap_mr_query("set_host_resource_record_ttl", 4, argv, NULL, NULL);
	    if (status) {
	      com_err(whoami, status, "while setting host resource record TTL");
	      exit(1);
	    }

	    top = top->q_forw;
	  }
      }
    else
      {
	argv[2] = addropt->opt;

	if (!strcasecmp(argv[2], "default"))
	  sprintf(argv[2], "%d", DEFAULT_TTL);

	status = wrap_mr_query("set_host_address_ttl", 3, argv, NULL, NULL);
	if (status) {
	  com_err(whoami, status, "while setting host address TTL");
	  exit(1);
	}
      }
  }

  if (set_ptr_flag) {
    char *argv[3];
    struct mrcl_addropt_type *addropt;

    addropt = mrcl_parse_addropt(ptr);
    if (!addropt)
      {
	com_err(whoami, 0, "Couldn't parse address specification while setting host address PTR status");
	exit(1);
      }

    argv[0] = canonicalize_hostname(strdup(hostname));
    argv[1] = addropt->address;
    argv[2] = addropt->opt;

    status = wrap_mr_query("set_host_address_ptr", 3, argv, NULL, NULL);
    if (status) {
      com_err(whoami, status, "while setting host address PTR status");
      exit(1);
    }
  }

  if (delete_flag) {
    char *argv[1];

    argv[0] = canonicalize_hostname(strdup(hostname));
    status = wrap_mr_query("delete_host", 1, argv, NULL, NULL);
    if (status) {
      com_err(whoami, status, "while deleting host");
      exit(1);
    }
  }

  if (add_dynamic_host_flag) {
    char *argv[2];

    argv[1] = owner->name;
    switch (owner->type)
      {
      case MRCL_M_ANY:
      case MRCL_M_USER:
	argv[0] = "USER";
	status = wrap_mr_query("add_dynamic_host_record", 2, argv, show_dynamic_hostname, NULL);
	if (owner->type != MRCL_M_ANY || status != MR_USER)
	  break;

      case MRCL_M_LIST:
	argv[0] = "LIST";
	status = wrap_mr_query("add_dynamic_host_record", 2, argv, show_dynamic_hostname, NULL);
	break;

      case MRCL_M_KERBEROS:
	argv[0] = "KERBEROS";
	status = mrcl_validate_kerberos_member(argv[1], &argv[1]);
	if (mrcl_get_message())
	  mrcl_com_err(whoami);
	if (status == MRCL_REJECT)
	  exit(1);
	status = wrap_mr_query("add_dynamic_host_record", 2, argv, show_dynamic_hostname, NULL);
	break;

      case MRCL_M_NONE:
	argv[0] = "NONE";
	status = wrap_mr_query("add_dynamic_host_record", 2, argv, show_dynamic_hostname, NULL);
	break;
      }

    if (status) {
      com_err(whoami, status, "while adding dynamic host");
      exit(1);
    }
  }

  /* We're done! */
  mr_disconnect();
  exit(success ? 0 : 1);
}

void usage(char **argv)
{
#define USAGE_OPTIONS_FORMAT "  %-39s%s\n"
  fprintf(stderr, "Usage: %s hostname [options]\n", argv[0]);
  fprintf(stderr, "Options are\n");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-C   | -create",
	  "-O   | -owner owner");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-D   | -delete",
	  "-S   | -status status");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-R   | -rename newname",
	  "-V   | -vendor vendor");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-a   | -addalias alias",
	  "-M   | -model model");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-d   | -deletealias alias",
	  "-L   | -location location");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-i   | -info",
	  "-o   | -os os");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-oc  | -opcmt op_cmt",
	  "-c   | -contact contact");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-ac  | -admcmt adm_cmt",
	  "-bc  | -billingcontact billing_contact");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-an  | -accountnumber account_number",
	  "-A   | -address network:address");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-DA  | -deleteaddress network:address",
	  "-am  | -addmap cluster");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-dm  | -deletemap cluster",
	  "-acn | -addcontainer container");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-dcn | -deletecontainer container",
	  "-lm  | -listmap");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-lcn | -listcontainer",
	  "-u   | -unformatted");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-v   | -verbose",
	  "-n   | -noauth");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-aid | -addidentifier identifier",
	  "-did | -delidentifier identifier");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-lid | -listidentifier",
	  "-oi  | -optin");
  fprintf(stderr,  USAGE_OPTIONS_FORMAT, "-oo  | -optout",
	  "-ttl | -setttl <address or record type>:ttl");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-adh | -adddynamic owner",
	  "-su  | -showusage");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-arr | -addrecord record",
	  "-drr | -delrecord record");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-lrr | -listrecords",
	  "-db  | -database host[:port]");
  exit(1);
}

/* Show alias information */

static int show_has_aliases;

int show_alias_info(int argc, char **argv, void *hint)
{
  if(!show_has_aliases++)
    printf("Aliases:  %s", argv[0]);
  else
    printf(", %s", argv[0]);

  return MR_CONT;
}

int show_alias_info_unformatted(int argc, char **argv, void *hint)
{
  if(!show_has_aliases++)
    printf("Alias:            %s", argv[0]);
  else
    printf(", %s", argv[0]);

  return MR_CONT;
}

/* Show TTL if not default */

int show_address_info(int argc, char **argv, void *hint)
{
  char tbuf[BUFSIZ];

  sprintf(tbuf, "%s:%s", argv[2], argv[3]);
  printf("Address:  %-16s    Network:    %-16s", tbuf, argv[1]);
  if (atoi(argv[5]) == 1)
    printf("    PTR record: yes");
  if (atoi(argv[4]) != DEFAULT_TTL)
    printf("    DNS TTL:  %-16s", argv[4]);
  printf("\n");

  return MR_CONT;
}

int show_address_info_unformatted(int argc, char **argv, void *hint)
{
  char tbuf[BUFSIZ];

  sprintf(tbuf, "%s:%s", argv[2], argv[3]);
  printf("Network,Address:  %s,%s", argv[1], tbuf);
  if (atoi(argv[5]) == 1)
    printf("    PTR record: yes");
  if (atoi(argv[4]) != DEFAULT_TTL)
    printf("    DNS TTL: %s\n", argv[4]);
  else
    printf("\n");

  return MR_CONT;
}

/* Show assigned dynamic hostname */
int show_dynamic_hostname(int argc, char **argv, void *hint)
{
  printf("Assigned dynamic hostname: %s\n", argv[0]);

  return MR_CONT;
}

static char *states[] = {
  "Reserved (0)",
  "Active (1)",
  "Local (2)",
  "Deleted (3)"
};

static char *MacState(int state)
{
  static char buf[BUFSIZ];

  if (state < 0 || state > 3)
    {
      sprintf(buf, "Unknown (%d)", state);
      return buf;
    }
  return states[state];
}

/* Retrieve information about a host */

int store_host_info(int argc, char **argv, void *hint)
{
  int i;
  char **nargv = hint;

  for(i=0; i<argc; i++)
    nargv[i] = strdup(argv[i]);

  return MR_CONT;
}

void show_host_info(char **argv)
{
  char tbuf[256];
  char *args[3];
  struct mqelem *elem = NULL;
  int stat;

  printf("Machine:  %s\n", argv[M_NAME]);
  args[0] = "*";
  args[1] = argv[M_NAME];

  show_has_aliases = 0;
  stat = wrap_mr_query("get_hostalias", 2, args, show_alias_info, &elem);
  printf("\n");
  if (stat) {
    if (stat != MR_NO_MATCH)
      com_err(whoami, stat, "while getting aliases");
  } else {
    printf("\n");
  }

  args[0] = argv[M_NAME];
  args[1] = "*";
  
  stat = wrap_mr_query("get_host_addresses", 2, args, show_address_info, &elem);
  if (stat) {
    if (stat != MR_NO_MATCH)
      com_err(whoami, stat, "while getting addresses");
  } else {
    printf("\n");
  }

  sprintf(tbuf, "%s %s", argv[M_OWNER_TYPE],
          strcmp(argv[M_OWNER_TYPE], "NONE") ? argv[M_OWNER_NAME] : "");
  printf("Owner:    %-16s    Use data:   %s\n", tbuf, argv[M_INUSE]);
  printf("Status:   %-16s    Changed:    %s\n",
          MacState(atoi(argv[M_STAT])), argv[M_STAT_CHNG]);
  printf("\n");
  printf("Vendor:   %-16s    Location:        %s\n", argv[M_VENDOR], 
	 argv[M_LOC]);
  printf("Model:    %-16s    Contact:         %s\n", argv[M_MODEL], 
	 argv[M_CONTACT]);
  printf("OS:       %-16s    Billing Contact: %s\n", argv[M_OS], 
	 argv[M_BILL_CONTACT]);
  printf("Opt:      %-16s    Account Number:  %s\n", argv[M_USE],
	 argv[M_ACCT_NUMBER]);
  printf("\nAdm cmt: %s\n", argv[M_ACOMMENT]);
  printf("Op cmt:  %s\n", argv[M_OCOMMENT]);
  printf("\n");
  printf("Created  by %s on %s\n", argv[M_CREATOR], argv[M_CREATED]);
  printf("Last mod by %s at %s with %s.\n", argv[M_MODBY], argv[M_MODTIME], argv[M_MODWITH]);
}

void show_host_info_unformatted(char **argv)
{
  char *args[3];
  struct mqelem *elem = NULL;
  int stat;

  printf("Machine:          %s\n", argv[M_NAME]);

  args[0] = "*";
  args[1] = argv[M_NAME];
  show_has_aliases = 0;
  stat = wrap_mr_query("get_hostalias", 2, args, show_alias_info_unformatted, 
		       &elem);
  if (stat && stat != MR_NO_MATCH)
    com_err(whoami, stat, "while getting aliases");
  else
    printf("\n");

  args[0] = argv[M_NAME];
  args[1] = "*";
  stat = wrap_mr_query("get_host_addresses", 2, args, show_address_info_unformatted,
		       &elem);
  if (stat && stat != MR_NO_MATCH)
    com_err(whoami, stat, "while getting addresses");
  
  printf("Owner Type:       %s\n", argv[M_OWNER_TYPE]);
  printf("Owner:            %s\n", argv[M_OWNER_NAME]);
  printf("Status:           %s\n", MacState(atoi(argv[M_STAT])));
  printf("Changed:          %s\n", argv[M_STAT_CHNG]);
  printf("Use data:         %s\n", argv[M_INUSE]);
  printf("Vendor:           %s\n", argv[M_VENDOR]);
  printf("Model:            %s\n", argv[M_MODEL]);
  printf("OS:               %s\n", argv[M_OS]);
  printf("Location:         %s\n", argv[M_LOC]);
  printf("Contact:          %s\n", argv[M_CONTACT]);
  printf("Billing Contact:  %s\n", argv[M_BILL_CONTACT]);
  printf("Account Number:   %s\n", argv[M_ACCT_NUMBER]);
  printf("Opt:              %s\n", argv[M_USE]);
  printf("Adm cmt:          %s\n", argv[M_ACOMMENT]);
  printf("Op cmt:           %s\n", argv[M_OCOMMENT]);
  printf("Created by:       %s\n", argv[M_CREATOR]);
  printf("Created on:       %s\n", argv[M_CREATED]);
  printf("Last mod by:      %s\n", argv[M_MODBY]);
  printf("Last mod on:      %s\n", argv[M_MODTIME]);
  printf("Last mod with:    %s\n", argv[M_MODWITH]);
}

int show_machine_in_cluster(int argc, char **argv, void *hint)
{
  printf("Machine: %-30s Cluster: %-30s\n", argv[0], argv[1]);

  return MR_CONT;
}

int show_machine_in_container(int argc, char **argv, void *hint)
{
  printf("Machine: %-30s Container: %-25s\n", argv[0], argv[1]); 

  return MR_CONT;
}

int show_host_identifiers(int argc, char **argv, void *hint)
{
  printf("Machine: %-30s Identifier: %s:%s\n", argv[0], argv[1], argv[2]);

  return MR_CONT;
}

int show_host_rr(int argc, char **argv, void *hint)
{
  printf("Machine: %-16s   Type: %s  Value: %-24s", argv[0], argv[1], argv[2]);
  if (atoi(argv[3]) != DEFAULT_TTL)
    printf("    DNS TTL:  %-16s", argv[3]);
  printf("\n");

  return MR_CONT;
}

int show_host_usage(int argc, char **argv, void *hint)
{
  printf("Machine: %-40s Type: %-16s Use: %s\n", argv[0], argv[1], argv[2]);

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
  for(cnt=0; cnt<argc; cnt++)
    printf(" <%s>", argv[cnt]);
  printf("\n");
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

struct mqelem *queue_top(struct mqelem *elem)
{
  if (!elem)
    return NULL;
  while (elem->q_back)
    elem = elem->q_back;
  return elem;
}

int store_query_info(int argc, char **argv, void *data)
{
  char **info = malloc(argc * sizeof(char *));
  struct mqelem **old_elem = data;
  struct mqelem *new_elem = malloc(sizeof(struct mqelem));
  int count;

  if (!new_elem || !info)
    return MR_ABORT;

  for (count = 0; count < argc; count++)
    info[count] = strdup(argv[count]);
  info[count] = NULL;

  new_elem->q_data = info;
  if (!*old_elem)
    {
      new_elem->q_forw = NULL;
      new_elem->q_back = NULL;
    }
  else
    {
      new_elem->q_back = *old_elem;
      new_elem->q_forw = (*old_elem)->q_forw;
      (*old_elem)->q_forw = new_elem;
    }
  *old_elem = new_elem;

  return MR_CONT;
}

