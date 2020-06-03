/* $Id: afs.c 3973 2010-02-02 19:15:44Z zacheiss $
 *
 * Do route-server incremental updates
 */

#include <moira.h>
#include <moira_site.h>
#include <moira_schema.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <com_err.h>
#include <krb5.h>

#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define STOP_FILE "/moira/route-server/noroute"
#define CREDS_FILE "/moira/route-server/creds"
#define USER_FILE "/moira/route-server/user"
#define DATABASE_FILE "/moira/route-server/database"
#define SERVERS_FILE "/moira/route-server/servers"

#define MAX_SERVERS 32

#define file_exists(file) (access((file), F_OK) == 0)

#define MOIRA_SVR_PRINCIPAL "sms"

#define STATUS_POS 9
#define OPT_POS 14

void check_route_server(void);
int get_creds(char *file, char **buffer);
int get_user(char *file, char **buffer);
int get_database(char *file, char **buffer);
int get_servers(char *file, char **buffer, int *nservers);
int update_route_server(char *state, char *address);
int moira_connect(void);
int moira_disconnect(void);
int save_host_addresses(int argc, char **argv, void *sq);
void do_machine(char **before, int beforec, char **after, int afterc);
void do_hostaddress(char **before, int beforec, char **after, int afterc);

char *whoami;
static char tbl_buf[1024];
int mr_connections = 0;

int main(int argc, char **argv)
{
  int beforec, afterc, i, astate = 0, bstate = 0, activate;
  char *table, *before_address, *after_address, **before, **after;
  int before_use, after_use, before_status, after_status;
  char *after_state;
  char *buffer;
  long status;
  int code;
  char *av[2];
  struct save_queue *host_addresses;
  char *hostalias;

  whoami = ((whoami = strrchr(argv[0], '/')) ? whoami+1 : argv[0]);
  
  if (argc < 4)
    {
      com_err(whoami, 0, "Unable to process %s", "argc < 4");
      exit(1);
    }

  if (argc < (4 + atoi(argv[2]) + atoi(argv[3])))
    {
      com_err(whoami, 0, "Unable to process %s",
              "argc < (4 + beforec + afterc)");
      exit(1);
    }

  table = argv[1];
  beforec = atoi(argv[2]);
  before = &argv[4];
  afterc = atoi(argv[3]);
  after = &argv[4 + beforec];

  strcpy(tbl_buf, table);
  strcat(tbl_buf, " ");
  strcat(tbl_buf, argv[2]);
  strcat(tbl_buf, " ");
  strcat(tbl_buf, argv[3]);
  strcat(tbl_buf, " ");
  for (i = 0; i < beforec; i++)
    {
      if (i > 0)
        strcat(tbl_buf, " ");
      strcat(tbl_buf, before[i]);
    }
  strcat(tbl_buf, " ");
  for (i = 0; i < afterc; i++)
    {
      if (i > 0)
        strcat(tbl_buf, " ");
      strcat(tbl_buf, after[i]);
    }

  com_err(whoami, 0, "%s", tbl_buf);

  check_route_server();
  
  if (!strcmp(table, "machine"))
    do_machine(before, beforec, after, afterc);
  else if (!strcmp(table, "hostaddress"))
    do_hostaddress(before, beforec, after, afterc);

  exit(0);
}

/* If we got an incremental on the machine table, address can't have changed */
void do_machine(char **before, int beforec, char **after, int afterc)
{
  int before_use, after_use, before_status, after_status;
  char *after_state, *hostaddress;
  int rc;
  char *av[2];
  struct save_queue *host_addresses;

  host_addresses = sq_create();

  if (afterc > OPT_POS)
    after_use = atoi(after[OPT_POS]);
  if (afterc > STATUS_POS)
    after_status = atoi(after[STATUS_POS]);
  if (beforec > OPT_POS) 
    before_use = atoi(before[OPT_POS]);
  if (beforec > STATUS_POS)
    before_status = atoi(before[STATUS_POS]);

  /* If the host was created or deleted, it can't have any addresses */
  if (beforec != 0 && afterc != 0)
    {
      /* host was modified, so we'll need to process all of its addresses */
      if (after_use == 0 || after_status == 3) 
	{
	  after_state = "none";
	}
      else if (after_use == 1 || after_use == 2) 
	{
	  after_state = "optout";
	}
      else if (after_use == 3)
	{
	  after_state = "blackhole";
	}
      else 
	{
	  critical_alert(whoami, "incremental", 
			 "Unexpected machine use %d", after_use);
	  exit(1);
	}

      if (rc = moira_connect())
	{
	  critical_alert(whoami, "incremental", "Error contacting Moira server: %s",
			 error_message(rc));
	  exit(1);
	}

      /* Get all addresses of machine */
      av[0] = after[0];
      av[1] = "IPV4";

      rc = mr_query("get_host_addresses", 2, av, save_host_addresses, host_addresses);
      if (rc && rc != MR_NO_MATCH)
	{
          critical_alert(whoami, "Unable to retrieve addresses for %s: %s", after[0], error_message(rc));
	  exit(1);
        }

      moira_disconnect();

      while (sq_get_data(host_addresses, &hostaddress))
	update_route_server(after_state, hostaddress);
    }
}

#define HOSTADDRESS_ADDR_POS 3
#define HOSTADDRESS_OPT_POS 6
#define HOSTADDRESS_STATUS_POS 10

/* If an address has an been added, the state can't have changed */
void do_hostaddress(char **before, int beforec, char **after, int afterc)
{
  int before_use, after_use, before_status, after_status;
  char *after_state, *hostaddress, *before_hostaddress, *after_hostaddress;

  if (afterc > HOSTADDRESS_OPT_POS)
    after_use = atoi(after[HOSTADDRESS_OPT_POS]);
  if (afterc > HOSTADDRESS_STATUS_POS)
    after_status = atoi(after[HOSTADDRESS_STATUS_POS]);
  if (beforec > HOSTADDRESS_OPT_POS)
    before_use = atoi(before[HOSTADDRESS_OPT_POS]);
  if (beforec > HOSTADDRESS_STATUS_POS)
    before_status = atoi(before[HOSTADDRESS_STATUS_POS]);

  /* If the address was just created, set its status to the host's current state */
  if (beforec == 0)
    {
      hostaddress = after[HOSTADDRESS_ADDR_POS];

      if (after_use == 0 || after_status == 3)
        {
          after_state = "none";
        }
      else if (after_use == 1 || after_use == 2)
        {
          after_state = "optout";
        }
      else if (after_use == 3)
        {
          after_state = "blackhole";
        }
      else
        {
          critical_alert(whoami, "incremental",
                         "Unexpected machine use %d", after_use);
          exit(1);
        }

      if (after_use == 1 || after_use == 2 || after_use == 3) 
	{
	  if (after_status == 0 || after_status == 1) 
	    {
	      update_route_server(after_state, hostaddress);
	    }
	}
    }
  /* address was deleted, state is always none */
  else if (afterc == 0)
    {
      hostaddress = before[HOSTADDRESS_ADDR_POS];
      update_route_server("none", hostaddress);
    }
  /* update, address might've changed if this was a NAT -> routable upgrade.
   * If it didn't, no action required */
  else
    {
      before_hostaddress = before[HOSTADDRESS_ADDR_POS];
      after_hostaddress = after[HOSTADDRESS_ADDR_POS];

      if (after_use == 0 || after_status == 3) 
        {
          after_state = "none";
        }
      else if (after_use == 1 || after_use == 2) 
        {
          after_state = "optout";
        }
      else if (after_use == 3)
        {
          after_state = "blackhole";
        }
      else 
        {
          critical_alert(whoami, "incremental", 
                         "Unexpected machine use %d", after_use);
          exit(1);
        }

      if (strcmp(before_hostaddress, after_hostaddress))
	{
	  update_route_server("none", before_hostaddress);
	  update_route_server(after_state, after_hostaddress);
        }
    }
}

int get_creds(char *file, char **buffer)
{
  int creds;
  struct stat statb;
  
  if (stat(file, &statb) == -1)
    {
      return MR_MISSINGFILE;
    }
  
  (*buffer) = malloc(statb.st_size + 1);
  if (!(*buffer))
    {
      return MR_NO_MEM;
    }

  creds = open(file, 0, O_RDONLY);
  if (creds)
    {
      read(creds, (*buffer), statb.st_size);
      close(creds);
      (*buffer)[statb.st_size - 1] = '\0'; 
      return MR_SUCCESS;
    }
  else
    return errno;

  return MR_SUCCESS;
}

int get_user(char *file, char **buffer)
{
  int creds;
  struct stat statb;
  
  if (stat(file, &statb) == -1)
    {
      return MR_MISSINGFILE;
    }
  
  (*buffer) = malloc(statb.st_size + 1);
  if (!(*buffer))
    {
      return MR_NO_MEM;
    }

  creds = open(file, 0, O_RDONLY);
  if (creds)
    {
      read(creds, (*buffer), statb.st_size);
      close(creds);
      (*buffer)[statb.st_size - 1] = '\0'; 
      return MR_SUCCESS;
    }
  else
    return errno;

  return MR_SUCCESS;
}

int get_database(char *file, char **buffer)
{
  int creds;
  struct stat statb;
  
  if (stat(file, &statb) == -1)
    {
      return MR_MISSINGFILE;
    }
  
  (*buffer) = malloc(statb.st_size + 1);
  if (!(*buffer))
    {
      return MR_NO_MEM;
    }

  creds = open(file, 0, O_RDONLY);
  if (creds)
    {
      read(creds, (*buffer), statb.st_size);
      close(creds);
      (*buffer)[statb.st_size - 1] = '\0'; 
      return MR_SUCCESS;
    }
  else
    return errno;

  return MR_SUCCESS;
}

int get_servers(char *file, char **buffer, int *nservers)
{
  FILE *servers;
  struct stat statb;
  char line[BUFSIZ];
  char **temp;
  int i;
  
  (*nservers) = 0;
  
  if (stat(file, &statb) == -1)
    {
      return MR_MISSINGFILE;
    }
  
  servers = fopen(file, "r");
  if (servers != NULL)
    {
      while(fgets(line, sizeof(line), servers) != NULL)
	{
	  buffer[(*nservers)] = malloc(sizeof(line));
	  line[strlen(line) - 1] = '\0';
	  strcpy(buffer[(*nservers)], line);
	  (*nservers)++;
	}
    }
  else
    return errno;

  return MR_SUCCESS;
}

int update_route_server(char *state, char *address)
{
  MYSQL *conn;
  MYSQL_RES *res;
  char *server; 
  char *user;
  char *password;
  char *database;
  char *servers[MAX_SERVERS];
  char sql[1024];
  int i, nservers;

  if (get_servers(SERVERS_FILE, servers, &nservers))
    {
      critical_alert(whoami, "incremental", 
		     "Unable to retrieve database server names");
      exit(1);
    }
  
  if (get_user(USER_FILE, &user)) 
    {
      critical_alert(whoami, "incremental", 
		     "Unable to retrieve database user");
      exit(1);
    }

  if (get_creds(CREDS_FILE, &password)) 
    {
      critical_alert(whoami, "incremental", 
		     "Unable to retrieve database credentials");
      exit(1);
    }

  if (get_database(DATABASE_FILE, &database)) 
    {
      critical_alert(whoami, "incremental", 
		     "Unable to retrieve database name");
      exit(1);
    }

  for (i = 0; i < nservers ; i++) 
    {
      server = servers[i];
      conn = mysql_init(NULL);

      com_err(whoami, 0, "POSTing %s -> %s to %s", address, state, server); 
      
      /* Connect to database */
      if (!mysql_real_connect(conn, server, user, password, database, 
			      0, NULL, 0))
	{
	  critical_alert(whoami, "incremental", 
			 "Unable to connect to database %s", 
			 mysql_error(conn));
	  exit(1);
	}

      sprintf(sql, "replace into routes(ip, state) values('%s', '%s')",
	      address, state);
  
      /* send SQL query */
      if (mysql_query(conn, sql)) 
	{
	  critical_alert(whoami, "incremental", 
			 "Unable to execute query %s", mysql_error(conn));
	  exit(1);
	}

      mysql_close(conn);
    }

  free(password);
  return 0;
}
  
void check_route_server(void)
{
  int i;

  for (i = 0; file_exists(STOP_FILE); i++)
    {
      if (i > 30)
	{
	  critical_alert(whoami, "incremental",
			 "Route-server incremental failed (%s exists): %s",
			 STOP_FILE, tbl_buf);
	  exit(1);
	}
      sleep(60);
    }
}

int save_host_addresses(int argc, char **argv, void *sq)
{
  struct save_queue *q = sq;
  char *hostaddress;

  hostaddress = strdup(argv[3]);
  sq_save_data(q, hostaddress);

  return MR_CONT;
}

int moira_connect(void)
{
  int code;

  if (!mr_connections++)
    {
      struct utsname uts;
      uname(&uts);
      code = mr_connect(uts.nodename);
      /* Note we don't authenticate */
      if (!code)
	code = mr_version(-1);
      return code;
    }
  return 0;
}

int moira_disconnect(void)
{
  struct member *m;

  if (!--mr_connections)
    {
      mr_disconnect();
    }
  return 0;
}
