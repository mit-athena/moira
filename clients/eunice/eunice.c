/* $Id: eunice.c 4051 2011-09-08 18:46:37Z zacheiss $
 *
 * Command line oriented Moira print queue tool.
 *
 * Code based on the blanche command line tool and the moira curses tool.
 * Copyright (C) 2009 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 * 
 * Mark Manley, mmanley@MIT.EDU, 12/09
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

RCSID("$HeadURL: svn+ssh://svn.mit.edu/moira/trunk/moira/clients/eunice/eunice.c $ $Id: eunice.c 4051 2011-09-08 18:46:37Z zacheiss $");

struct member {
  int type;
  char *name, *tag;
};

struct string_list {
  char *string;
  struct string_list *next;
};

const char *deflogserver = "WSLOGGER.MIT.EDU";

/* legacy variables that we need to set regardless */
const char *quotaserver = "[NONE]";
const char *pageprice = "10";

/* argument parsing macro */
#define argis(a, b) (!strcmp(*arg + 1, a) || !strcmp(*arg + 1, b))

/* flags from command line */
int info_flag, verbose, noauth, duplex_default, hold_default;
int create_flag, delete_flag, rename_flag, ka, banner, update_flag, update_hwaddr_flag;
char *lpracl, *lpcacl, *report_list;
char *contact, *newname, *printserver, *type, *hwtype, *mac, *hostname, *queuename;
char *duplexname, *logserver, *location, *realname, *pr_status;

char *queuename, *whoami, *testqueue;

struct string_list *hwaddr_add_queue, *hwaddr_remove_queue;

void usage(char **argv);
int show_printer_info(char *queuename);
int save_printer_info(int argc, char **argv, void *hint);
int show_hwaddrs(int argc, char **argv, void *hint);
void recursive_display_list_members(void);
char *get_username(void);
int wrap_mr_query(char *handle, int argc, char **argv,
		  int (*callback)(int, char **, void *), void *callarg);
void print_query(char *query_name, int argc, char **argv);
int CountArgs(char **info);
struct string_list *add_to_string_list(struct string_list *old_list, char *s);

static char *states[] = {
  "Reserved (0)",
  "Active (1)",
  "Active, No IP ACL (2)",
  "Deleted (3)",
  "Active, BOOTP only (4)"
};

static char *PrnState(int state)
{
  static char buf[BUFSIZ];
  
  if (state < 0 || state > 4)
    {
      sprintf(buf, "Unknown (%d)", state);
      return buf;
    }
  return states[state];
}

int main(int argc, char **argv)
{
  int status, success;
  char **arg = argv;
  char *uargv[2];
  char *pargv[PRN_END + 1];
  char *membervec[4];
  struct member *memberstruct;
  char *server = NULL, *p;
  int i;

  /* clear all flags & lists */
  i = info_flag = verbose = noauth = 0;
  create_flag = delete_flag = update_flag = update_hwaddr_flag = 0;
  ka = duplex_default = hold_default = banner = -1;
  location = lpracl = lpcacl = report_list = pr_status = NULL;
  logserver = duplexname = realname = newname = printserver = type = hwtype = mac = hostname = NULL;
  hwaddr_add_queue = hwaddr_remove_queue = NULL;
  contact = NULL;

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
	  else if (argis("L", "location"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
  		  update_flag++;
		  location = *arg;
		}
	      else
		usage(argv);
	    }
	  else if (argis("r", "remotename"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
  		  update_flag++;
		  realname = *arg;
		}
	      else
		usage(argv);
	    }
	  else if (argis("T", "type"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
  		  update_flag++;
		  type = *arg;
		}
	      else
		usage(argv);
	    }
	  else if (argis("s", "printserver"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
  		  update_flag++;
		  printserver = canonicalize_hostname(strdup(*arg));
		}
	      else
		usage(argv);
	    }
	  else if (argis("M", "model"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
  		  update_flag++;
		  hwtype = *arg;
		}
	      else
		usage(argv);
	    }
          else if (argis("S", "status"))
	    {
	      if (arg - argv < argc - 1) {
		int i;
		int len;
		
		arg++;
		update_flag++;
		pr_status = *arg;
		
		len = strlen(pr_status);
		for(i = 0; i < len; i++) {
		  if(!isdigit(pr_status[i])) {
		    printf("Error: status code %s is not numeric.\n", pr_status);
		    exit(1);
		  }
		}
	      } else
		usage(argv);
	    }
	  else if (argis("K", "kerbauth")) 
 	    {
	      ka = 1;
  	      update_flag++;
 	    }
	  else if (argis("NK", "nokerbauth"))
 	    {
	      ka = 0;
  	      update_flag++;
 	    }
	  else if (argis("DD", "duplexdefault"))
	    {
	      duplex_default = 1;
	      update_flag++;
	    }
	  else if (argis("ND", "noduplexdefault"))
	    {
	      duplex_default = 0;
	      update_flag++;
	    }
	  else if (argis("HD", "holddefault"))
	    {
	      hold_default = 1;
	      update_flag++;
	    }
	  else if (argis("NH", "noholddefault"))
	    {
	      hold_default = 0;
	      update_flag++;
	    }
	  else if (argis("H", "hostname"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
  		  update_flag++;
		  hostname = canonicalize_hostname(strdup(*arg));
		}
	      else
		usage(argv);
	    }
	  else if (argis("n", "noauth"))
	    noauth++;
	  else if (argis("v", "verbose"))
	    verbose++;
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
	  else if (argis("c", "contact"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
		  contact = *arg;
		  update_flag++;
		}
	      else
		usage(argv);
	    }
	  else if (argis("l", "lpcacl"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
		  lpcacl = *arg;
		  update_flag++;
		}
	      else
		usage(argv);
	    }
	  else if (argis("ac", "lpracl"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
		  lpracl = *arg;
		  update_flag++;
		}
	      else
		usage(argv);
	    }
	  else if (argis("rl", "reportlist"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
		  report_list = *arg;
		  update_flag++;
		}
	      else
		usage(argv);
	    }
	  else if (argis("ahw", "addhwaddr"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
		  hwaddr_add_queue=add_to_string_list(hwaddr_add_queue, *arg);
		}
	      else
		usage(argv);
	      update_hwaddr_flag++;
	    }
	  else if (argis("dhw", "delhwaddr"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
		  hwaddr_remove_queue=add_to_string_list(hwaddr_remove_queue, *arg);
		}
	      else
		usage(argv);
	      update_hwaddr_flag++;
	    }
	  else if (argis("b", "banner"))
	    {
	      update_flag++;
	      banner = 1;
	    }
	  else if (argis("nb", "nobanner"))
	    {
	      update_flag++;
	      banner = 0;
	    }
	  else if (argis("L", "logserver"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
	          update_flag++;
		  logserver = canonicalize_hostname(strdup(*arg));
		}
	      else
		usage(argv);
	    }
	  else if (argis("R", "rename"))
	    {
	      if (arg - argv < argc - 1)
		{
		  rename_flag++;
		  update_flag++;
		  ++arg;
		  newname = *arg;
		}
	      else
		usage(argv);
	    }
	  else if (argis("d", "duplex"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
	          update_flag++;
		  duplexname = *arg;
		}
	      else
		usage(argv);
	    }
	  else
	    usage(argv);
	}
      else if (queuename == NULL)
	queuename = *arg;
      else
	usage(argv);
    }
  if (queuename == NULL)
    usage(argv);


  if (!update_flag && !rename_flag && !delete_flag && !create_flag && !update_hwaddr_flag)
    info_flag++;

  /* fire up Moira */
  status = mrcl_connect(server, "eunice", 13, !noauth);
  if (status == MRCL_AUTH_ERROR)
    {
      com_err(whoami, 0, "Authentication error while working on queue %s",
	      queuename);
      com_err(whoami, 0, "Try the -noauth flag if you don't "
	      "need authentication.");
    }
  if (status)
    exit(2);

  if (create_flag)
    {
      if (hostname == NULL || type == NULL || hwtype == NULL )
        usage(argv);
    }

  for (i = 0; i < PRN_END + 1; i++)
      pargv[i] = NULL;

  /* check for name conflicts. */
  if (create_flag || rename_flag)
    {

      if (rename_flag)
	testqueue = newname;
      else
	testqueue = queuename;

      status = wrap_mr_query("get_printer", 1, &testqueue, NULL, NULL);
      if (status != MR_NO_MATCH) 
      {
	fprintf(stderr, "ERROR: A queue by that name already exists.\n");
	exit(1);
      }

      status = wrap_mr_query("get_printer_by_duplexname", 1, &testqueue, NULL, NULL);
      if (status != MR_NO_MATCH)
      {
	fprintf(stderr, "ERROR: A duplex queue by that name already exists.\n");
	exit(1);
      }

      if (duplexname) {
        status = wrap_mr_query("get_printer_by_duplexname", 1, &testqueue, NULL, NULL);
        if (status != MR_NO_MATCH)
	{
	  fprintf(stderr, "ERROR: A duplex queue by that name already exists.\n");
	  exit(1);
	}
      }

    }

  /* create if needed */
  if (create_flag)
    {
      if (realname == NULL)
        realname = queuename;	/* Variable for aliases */

      if (logserver == NULL)
        logserver = (char *) canonicalize_hostname(strdup(deflogserver));

      pargv[PRN_NAME] = queuename;
      pargv[PRN_TYPE] = type;
      pargv[PRN_HWTYPE] = hwtype;

      pargv[PRN_DUPLEXNAME] = (duplexname != NULL) ? duplexname : "";

      pargv[PRN_DUPLEXDEFAULT] = (duplex_default == 1) ? "1" : "0";
      pargv[PRN_HOLDDEFAULT] = (hold_default == 1) ? "1" : "0";

      if (pr_status)
	pargv[PRN_STATUS] = pr_status;
      else
	pargv[PRN_STATUS] = "1";

      pargv[PRN_HOSTNAME] = hostname;
      pargv[PRN_LOGHOST] = logserver;

      pargv[PRN_RM] = (printserver != NULL) ? printserver : "[ANY]";

      pargv[PRN_CONTACT] = (contact != NULL) ? contact : "";

      pargv[PRN_LOCATION] = (location != NULL) ? location : "";

      pargv[PRN_RP] = realname;
      pargv[PRN_RQ] = (char *) quotaserver;
      pargv[PRN_KA] = (ka == 1) ? "1" : "0";
      pargv[PRN_PC] = (char *) pageprice;

      if (lpracl != NULL) 
        {
          status = wrap_mr_query("get_list_info", 1, &lpracl,
                        NULL, NULL);
          if (status)
            {
               com_err(whoami, status, "while getting authentication list information");
               exit(1);
            }
          pargv[PRN_AC] = lpracl;
        }
      else
        pargv[PRN_AC] = "[none]";

      if (lpcacl != NULL) 
        {
          status = wrap_mr_query("get_list_info", 1, &lpcacl,
                        NULL, NULL);
          if (status)
            {
               com_err(whoami, status, "while getting authentication list information");
               exit(1);
            }
          pargv[PRN_LPC_ACL] = lpcacl;
        }
      else
        pargv[PRN_LPC_ACL] = "[none]";

      if (report_list != NULL)
        {
          status = wrap_mr_query("get_list_info", 1, &report_list,
				 NULL, NULL);
          if (status)
            {
	      com_err(whoami, status, "while getting authentication list information");
	      exit(1);
            }
          pargv[PRN_REPORT_LIST] = report_list;
        }
      else
        pargv[PRN_REPORT_LIST] = "[none]";

      pargv[PRN_BANNER] = (banner == 1) ? "1" : "0";

      status = wrap_mr_query("add_printer", CountArgs(pargv), pargv, NULL, NULL); 

      if (status)
	{
	  com_err(whoami, status, "while creating print queue.");
	  exit(1);
	}
      else
 	  printf ("%s: queue %s created.\n", whoami, queuename);
    }
  else if (update_flag)
    {

      status = wrap_mr_query("get_printer", 1, &queuename, save_printer_info, pargv);
      if (status)
	{
	  com_err(whoami, status, "while getting printer information");
	  exit(1);
	}

      pargv[0] = queuename;
      if (newname && rename_flag)
	pargv[PRN_NAME + 1] = newname;
      if (type != NULL)
	pargv[PRN_TYPE + 1] = type;
      if (hwtype != NULL)
	pargv[PRN_HWTYPE + 1] = hwtype;

      if (duplexname != NULL)
	pargv[PRN_DUPLEXNAME + 1] = duplexname;

      if (duplex_default != -1)
	pargv[PRN_DUPLEXDEFAULT + 1] = duplex_default ? "1" : "0";

      if (hold_default != -1)
	pargv[PRN_HOLDDEFAULT + 1] = hold_default ? "1" : "0";

      if (pr_status != NULL)
	pargv[PRN_STATUS + 1] = pr_status;

      if (hostname != NULL)
	pargv[PRN_HOSTNAME + 1] = hostname;

      if (logserver != NULL)
	pargv[PRN_LOGHOST + 1] = logserver;

      if (printserver != NULL)
	pargv[PRN_RM + 1] = printserver;

      if (realname != NULL)
	pargv[PRN_RP + 1] = realname;

      if (quotaserver != NULL && strcmp(quotaserver,"[NONE]"))
	pargv[PRN_RQ + 1] = (char *) quotaserver;

      if (ka != -1)
	pargv[PRN_KA + 1] = ka ? "1" : "0";
      if (lpracl != NULL)
	pargv[PRN_AC + 1] = lpracl;
      if (lpcacl != NULL)
	pargv[PRN_LPC_ACL + 1] = lpcacl;
      if (report_list != NULL)
	pargv[PRN_REPORT_LIST + 1] = report_list;
      if (banner != -1)
	pargv[PRN_BANNER + 1] = banner ? "1" : "0";
      if (location != NULL)
	pargv[PRN_LOCATION + 1] = location;
      if (contact != NULL)
	pargv[PRN_CONTACT + 1] = contact;

       pargv[PRN_MODBY + 1] = pargv[PRN_MODTIME + 1] = pargv[PRN_MODWITH + 1] = NULL;

      status = wrap_mr_query("update_printer", CountArgs(pargv), pargv, NULL, NULL);
      if (status)
	{
	  com_err(whoami, status, "while updating print queue.");
	  exit(1);
	}
       else
         {
	   if (rename_flag) 
 	     printf ("%s: queue %s renamed to %s.\n", whoami, queuename, newname);
	   else
 	     printf ("%s: queue %s updated.\n", whoami, queuename);
         }
    }

  /* display printer info if requested to */
  if (info_flag)
    show_printer_info(queuename);

  /* add hwaddrs if necessary */
  if (hwaddr_add_queue) {
    struct string_list *q = hwaddr_add_queue;

    status = wrap_mr_query("get_printer", 1, &queuename, save_printer_info, pargv);
    if (status)
      { 
	com_err(whoami, status, "while getting printer information");
	exit(1);
      }

    if (hostname == NULL)
      uargv[0] = pargv[PRN_HOSTNAME + 1];
    else
      uargv[0] = hostname;

    while(q) {

      uargv[1] = q->string;

      status = wrap_mr_query("add_host_hwaddr", 2, uargv, NULL, NULL);
      if (status)
	{
	  com_err(whoami, status, "while adding host hardware address");
	  exit(1);
	}

      q = q->next;
    }
  }

  /* delete hwaddrs if necessary */
  if (hwaddr_remove_queue) {
    struct string_list *q = hwaddr_remove_queue;

    status = wrap_mr_query("get_printer", 1, &queuename, save_printer_info, pargv);
    if (status)
      {
        com_err(whoami, status, "while getting printer information");
        exit(1);
      }

    if (hostname == NULL)
      uargv[0] = pargv[PRN_HOSTNAME + 1];
    else
      uargv[0] = hostname;

    while(q) {

      uargv[1] = q->string;

      status = wrap_mr_query("delete_host_hwaddr", 2, uargv, NULL, NULL);
      if (status)
        {
          com_err(whoami, status, "while deleting host hardware address");
          exit(1);
        }

      q = q->next;
    }
  }
  
  if (delete_flag)
    {
      status = wrap_mr_query("delete_printer", 1, &queuename,
			NULL, NULL);
      if (status)
	{
	  com_err(whoami, status, "while deleting printer");
	  exit(1);
	}
       else
 	  printf ("%s: queue %s deleted.\n", whoami, queuename);
    }

  /* We're done! */
  mr_disconnect();
  exit(success ? 0 : 1);
}

void usage(char **argv)
{
#define USAGE_OPTIONS_FORMAT "  %-39s%s\n"
  fprintf(stderr, "Usage: %s queue [options]\n", argv[0]);
  fprintf(stderr, "Options are\n");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-v   | -verbose",
	  "-C   | -create");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-D   | -delete",
	  "-R   | -rename newname");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-i   | -info",
	  "-L   | -location location");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-H   | -hostname host",
	  "-c   | -contact contact");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-r   | -remotename name",
	  "-T   | -type printer_type");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-M   | -model model",
	  "-s   | -printserver print_server");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-K   | -kerbauth",
	  "-b   | -banner");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-NK  | -nokerbauth",
	  "-nb  | -nobanner");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-DD  | -duplexdefault",
	  "-HD  | -holddefault");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-ND  | -noduplexdefault",
	  "-NH  | -noholddefault");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-S   | -status status",
	  "-rl  | -reportlist list");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-l   | -lpcacl list",
	  "-ac  | -lpracl list");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-d   | -duplex name",
	  "-n   | -noauth");
  fprintf(stderr, USAGE_OPTIONS_FORMAT, "-ahw | -addhwaddr hwaddr", 
	  "-dhw | -delhwaddr hwaddr");
  fprintf(stderr, "  %-39s\n" , "-db  | -database host[:port]");
  exit(1);
}

static int show_has_hwaddrs;

int show_hwaddrs(int argc, char **argv, void *hint)
{
  if(!show_has_hwaddrs++)
    printf("Hardware Addresses:  %s", argv[1]);
  else
    printf(", %s", argv[1]);

  return MR_CONT;
}

int show_printer_info(char *queuename)
{
  char *pargv[PRN_END + 1];
  int status, banner, i;
  struct mqelem *elem = NULL;

  for (i = 0; i < PRN_END + 1; i++)
    pargv[i] = NULL;

  status = wrap_mr_query("get_printer", 1, &queuename, save_printer_info, pargv);
  if (status)
    {
      com_err(whoami, status, "while getting printer information");
      exit (1);
    }

  banner = atoi(pargv[PRN_BANNER + 1]);

  printf("Printer: %-18s Duplex queue: %-17s Status: %-10s\n", pargv[PRN_NAME + 1],
	 *pargv[PRN_DUPLEXNAME + 1] ? pargv[PRN_DUPLEXNAME + 1] : "[none]",
	 PrnState(atoi(pargv[PRN_STATUS + 1])));
  printf("Duplex by Default: %-8s Hold by Default: %-18s\n",
	 atoi(pargv[PRN_DUPLEXDEFAULT + 1]) ? "yes" : "no",
	 atoi(pargv[PRN_HOLDDEFAULT + 1]) ? "yes" : "no");
  printf("Type: %-10s Hardware type: %-10s\n",
          pargv[PRN_TYPE + 1], pargv[PRN_HWTYPE + 1]);
  printf("Printer hostname: %s\n", pargv[PRN_HOSTNAME + 1]);

  show_has_hwaddrs = 0;
  status = wrap_mr_query("get_host_hwaddr_mapping", 1, &pargv[PRN_HOSTNAME + 1], show_hwaddrs, &elem);
  if (status)
    {
      if (status != MR_NO_MATCH)
	com_err(whoami, status, "while getting hardware addresses");
      else
	printf("Hardware Addresses: none\n");
    }
  else
    printf("\n");
  
  printf("Printer log host: %s\n", pargv[PRN_LOGHOST + 1]);
  printf("Spool host: %s\n", pargv[PRN_RM + 1]);
  printf("Remote Printer Name: %-10s Banner page: %s\n", pargv[PRN_RP + 1],
          banner ? ( banner == PRN_BANNER_FIRST ? "Yes" : "Last" ) : "No");
  printf("Authentication: %-3s Price/page: %-3s  Quota Server: %s\n",
          atoi(pargv[PRN_KA + 1]) ? "yes" : "no", pargv[PRN_PC + 1], pargv[PRN_RQ + 1]);
  printf("Restrict list: %-23s  LPC ACL: %-23s\n",
          pargv[PRN_AC + 1], pargv[PRN_LPC_ACL + 1]);
  printf("Report list: %-23s\n", pargv[PRN_REPORT_LIST + 1]);
  printf("Location: %s\n", pargv[PRN_LOCATION + 1]);
  printf("Contact: %s\n", pargv[PRN_CONTACT + 1]);
  printf("\n");
  printf("Last mod by %s at %s with %s.\n", pargv[PRN_MODBY + 1], pargv[PRN_MODTIME + 1], pargv[PRN_MODWITH + 1]);

  return MR_CONT;
}


/* Copy retrieved information about a printer into a new argv */

int save_printer_info(int argc, char **argv, void *hint)
{
  char **nargv = hint;

  for (argc = 0; argc < PRN_END; argc++)
    nargv[argc + 1] = strdup(argv[argc]);
  return MR_CONT;
}

int wrap_mr_query(char *handle, int argc, char **argv,
		  int (*callback)(int, char **, void *), void *callarg) 
{
  int status = 0;

  if (verbose)
    print_query(handle, argc, argv);

  status = mr_query(handle, argc, argv, callback, callarg);

  return (status);
}

void print_query(char *query_name, int argc, char **argv)
{ 
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

/*      Function Name: CountArgs
 *      Description:  Retrieve the number of args in a null terminated
 *                     arglist.
 *      Arguments: info - the argument list.
 *      Returns: number if args in the list.
 */

int CountArgs(char **info)
{
  int number = 0;

  while (*info)
    {
      number++;
      info++;
    }

  return number;
}

struct string_list *add_to_string_list(struct string_list *old_list, char *s) {
  struct string_list *new_list;

  new_list = (struct string_list *)malloc(sizeof(struct string_list *));
  new_list->next = old_list;
  new_list->string = s;

  return new_list;
}
