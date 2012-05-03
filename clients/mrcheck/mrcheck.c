/* $Id: mrcheck.c 3956 2010-01-05 20:56:56Z zacheiss $
 *
 * Verify that all Moira updates are successful
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include <moira_site.h>
#include <mrclient.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define argis(a, b) (!strcmp(*arg + 1, a) || !strcmp(*arg + 1, b))

RCSID("$HeadURL: svn+ssh://svn.mit.edu/moira/trunk/moira/clients/mrcheck/mrcheck.c $ $Id: mrcheck.c 3956 2010-01-05 20:56:56Z zacheiss $");

char *atot(char *itime);
int process_server(int argc, char **argv, void *sqv);
void disp_svc(char **argv, char *msg);
int process_host(int argc, char **argv, void *sqv);
void disp_sh(char **argv, char *msg);
void usage(void);

char *whoami;
static int count = 0;
static time_t now;

struct service {
  char name[17];
  char update_int[10];
};


/* turn an ascii string containing the number of seconds since the epoch
 * into an ascii string containing the corresponding time & date
 */

char *atot(char *itime)
{
  time_t time;
  char *ct;

  time = atoi(itime);
  ct = ctime(&time);
  ct[24] = 0;
  return &ct[4];
}


/* Decide if the server has an error or not.  Also, save the name and
 * interval for later use.
 */

int process_server(int argc, char **argv, void *sqv)
{
  struct service *s;
  struct save_queue *sq = sqv;

  if (atoi(argv[SVC_ENABLE]))
    {
      s = malloc(sizeof(struct service));
      strcpy(s->name, argv[SVC_SERVICE]);
      strcpy(s->update_int, argv[SVC_INTERVAL]);
      sq_save_data(sq, s);
    }

  if (atoi(argv[SVC_HARDERROR]) && atoi(argv[SVC_ENABLE]))
    disp_svc(argv, "Error needs to be reset\n");
  else if (atoi(argv[SVC_HARDERROR]) ||
	   (!atoi(argv[SVC_ENABLE]) && atoi(argv[SVC_DFCHECK])))
    disp_svc(argv, "Should this be enabled?\n");
  else if (atoi(argv[SVC_ENABLE]) &&
	   60 * atoi(argv[SVC_INTERVAL]) + 86400 + atoi(argv[SVC_DFCHECK])
	   < now)
    disp_svc(argv, "Service has not been updated\n");

  return MR_CONT;
}


/* Format the information about a service. */

void disp_svc(char **argv, char *msg)
{
  char *tmp = strdup(atot(argv[SVC_DFGEN]));

  printf("Service %s Interval %s %s/%s/%s %s\n",
	 argv[SVC_SERVICE], argv[SVC_INTERVAL],
	 atoi(argv[SVC_ENABLE]) ? "Enabled" : "Disabled",
	 atoi(argv[SVC_INPROGRESS]) ? "InProgress" : "Idle",
	 atoi(argv[SVC_HARDERROR]) ? "Error" : "NoError",
	 atoi(argv[SVC_HARDERROR]) ? argv[SVC_ERRMSG] : "");
  printf("  Generated %s; Last checked %s\n", tmp, atot(argv[SVC_DFCHECK]));
  printf("  Last modified by %s at %s with %s\n",
	 argv[SVC_MODBY], argv[SVC_MODTIME], argv[SVC_MODWITH]);
  printf(" * %s\n", msg);
  count++;
  free(tmp);
}


/* Decide if the host has an error or not. */

int process_host(int argc, char **argv, void *sqv)
{
  struct service *s = NULL;
  struct save_queue *sq = sqv, *sq1;
  char *update_int = NULL;

  for (sq1 = sq->q_next; sq1 != sq; sq1 = sq1->q_next)
    {
      if ((s = (struct service *)sq1->q_data) &&
	  !strcmp(s->name, argv[SH_SERVICE]))
	break;
    }
  if (s && !strcmp(s->name, argv[SH_SERVICE]))
    update_int = s->update_int;

  if (atoi(argv[SH_HOSTERROR]) && atoi(argv[SH_ENABLE]))
    disp_sh(argv, "Error needs to be reset\n");
  else if (atoi(argv[SH_HOSTERROR]) ||
	   (!atoi(argv[SH_ENABLE]) && atoi(argv[SH_LASTTRY])))
    disp_sh(argv, "Should this be enabled?\n");
  else if (atoi(argv[SH_ENABLE]) && update_int &&
	   60 * atoi(update_int) + 86400 + atoi(argv[SH_LASTSUCCESS])
	   < now)
    disp_sh(argv, "Host has not been updated\n");

  return MR_CONT;
}


/* Format the information about a host. */

void disp_sh(char **argv, char *msg)
{
  char *tmp = strdup(atot(argv[SH_LASTTRY]));

  printf("Host %s:%s %s/%s/%s/%s/%s %s\n",
	 argv[SH_SERVICE], argv[SH_MACHINE],
	 atoi(argv[SH_ENABLE]) ? "Enabled" : "Disabled",
	 atoi(argv[SH_SUCCESS]) ? "Success" : "Failure",
	 atoi(argv[SH_INPROGRESS]) ? "InProgress" : "Idle",
	 atoi(argv[SH_OVERRIDE]) ? "Override" : "Normal",
	 atoi(argv[SH_HOSTERROR]) ? "Error" : "NoError",
	 atoi(argv[SH_HOSTERROR]) ? argv[SH_ERRMSG] : "");
  printf("  Last try %s; Last success %s\n", tmp, atot(argv[SH_LASTSUCCESS]));
  printf("  Last modified by %s at %s with %s\n",
	 argv[SH_MODBY], argv[SH_MODTIME], argv[SH_MODWITH]);
  printf(" * %s\n", msg);
  count++;
  free(tmp);
}



int main(int argc, char *argv[])
{
  char *args[2], buf[BUFSIZ];
  struct save_queue *sq;
  int status;
  int auth_required = 1;
  char **arg = argv;
  char *server = NULL;

  if ((whoami = strrchr(argv[0], '/')) == NULL)
    whoami = argv[0];
  else
    whoami++;

  /* parse our command line options */
  while (++arg - argv < argc)
    {
      if (**arg == '-')
	{
	  if (argis("n", "noauth"))
	    auth_required = 0;
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
      else
	usage();
    }

  if (mrcl_connect(server, "mrcheck", 2, auth_required) != MRCL_SUCCESS)
    exit(2);

  now = time(NULL);
  sq = sq_create();

  /* Check services first */
  args[0] = "*";
  if ((status = mr_query("get_server_info", 1, args, process_server, sq)) &&
      status != MR_NO_MATCH)
    com_err(whoami, status, " while getting servers");

  args[1] = "*";
  if ((status = mr_query("get_server_host_info", 2, args, process_host, sq)) &&
      status != MR_NO_MATCH)
    com_err(whoami, status, " while getting servers");

  if (!count)
    printf("Nothing has failed at this time\n");
  else
    printf("%d thing%s ha%s failed at this time\n", count,
	   count == 1 ? "" : "s", count == 1 ? "s" : "ve");

  mr_disconnect();
  exit(0);

punt:
  com_err(whoami, status, "%s", buf);
  mr_disconnect();
  exit(1);
}

void usage(void)
{
  fprintf(stderr, "Usage: %s [-noauth] [-db|-database server[:port]]\n",
	  whoami);
  exit(1);
}
