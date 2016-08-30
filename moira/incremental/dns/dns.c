/* $Id: dns.c 3973 2010-02-02 19:15:44Z zacheiss $
 *
 * Do dns incremental updates
 *
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

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <netdb.h>
#include <res_update.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define STOP_FILE "/moira/dns/nodns"
#define SERVERS_FILE "/moira/dns/servers"
#define HMAC_INTERNAL_KEY_FILE "/moira/dns/hmac_internal_key"
#define HMAC_EXTERNAL_KEY_FILE "/moira/dns/hmac_external_key"

#define INTERNAL_KEY_NAME "moira_internal"
#define EXTERNAL_KEY_NAME "moira_external"

#define DEFAULT_PTR_ZONE "18.IN-ADDR.ARPA"
#define DEFAULT_ZONE "MIT.EDU"

#define MAX_SERVERS 32
#define MAX_RESULTS 32

#define file_exists(file) (access((file), F_OK) == 0)

#define MOIRA_SVR_PRINCIPAL "sms"

#define MACH_NAME_POS 0
#define MACH_ID_POS 1
#define MACH_VENDOR_POS 2
#define MACH_MODEL_POS 3
#define MACH_OS_POS 4
#define MACH_STATUS_POS 9
#define MACH_ADDR_POS 10
#define MACH_SNET_ID_POS 15
#define MACH_TTL_POS 18

#define ALIAS_POS 0
#define ALIAS_NAME_POS 1
#define ALIAS_ID_POS 2

#define DEFAULT_TTL 1800

void check_dns(void);
int get_creds(char *file, char **buffer);
int get_hmac_key(char *file, char **buffer);
int get_user(char *file, char **buffer);
int get_database(char *file, char **buffer);
int get_servers(char *file, char **buffer, int *nservers);
int update_route_server(char *state, char *address);
int host_update(char *before_name, char *before_address, char *before_vendor,
		char *before_model, char *before_os, int before_ttl, 
		int before_mach_id, char *after_name, char *after_address,
		char *after_vendor, char *after_model, char *after_os, 
		int after_ttl, int after_mach_id, ns_tsig_key *tkey);
int host_delete(char *before_name, char *before_address,
		char *before_vendor, char *before_model, char *before_os, 
		int before_ttl, int before_mach_id, ns_tsig_key *tkey);
int alias_delete(char *before_alias, char *before_name,
		 int before_ttl, int before_mach_id, ns_tsig_key *tkey);
int host_add(char *after_name, char *after_address,
	     char *after_vendor, char *after_model, char *after_os, 
	     int after_ttl, int after_mach_id, ns_tsig_key *tkey);
int alias_add(char *after_alias, char *after_name,
	      int after_ttl, int after_mach_id, ns_tsig_key *tkey);
void do_machine(char **before, int beforec, char **after, int afterc);
void do_hostalias(char **before, int beforec, char **after, int afterc);
char *reverse_ip(char *ip);
int initialize_hmac_keys(void);
int moira_connect(void);
int moira_disconnect(void);
int save_hostaliases(int argc, char **argv, void *sq);
int delete_hostaliases(char *before_name, char *after_name, int before_mach_id);
int add_hostaliases(char *before_nane, char *after_name, int after_mach_id);

char *whoami;
static char tbl_buf[BUFSIZ];
int  mr_connections = 0;

ns_tsig_key *internal_tkey, *external_tkey;

int main(int argc, char **argv)
{
  int beforec, afterc, i, j, astate = 0, bstate = 0, activate;
  char *table, *before_address, *after_address, **before, **after,
    *before_name, *after_name, *before_vendor, *after_vendor, *before_model,
    *after_model, *before_os, *after_os, *before_snet_id, *after_snet_id;
  int before_use, after_use, before_status, after_status, before_mach_id, 
    after_mach_id, before_account_number, after_account_number, before_ttl,
    after_ttl;
  
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

  initialize_sms_error_table();

  com_err(whoami, 0, "%s", tbl_buf);

  if (!initialize_hmac_keys())
    {
      critical_alert(whoami, "incremental", "Unable to initialize HMAC keys");
      exit(1);
    }

  if (!strcmp(table, "machine"))
    do_machine(before, beforec, after, afterc);
  if (!strcmp(table, "hostalias"))
    do_hostalias(before, beforec, after, afterc);

  exit(0);
}

void do_machine(char **before, int beforec, char **after, int afterc)
{
  char *before_name, *after_name, *before_vendor, *after_vendor, *before_model,
    *after_model, *before_os, *after_os, *before_address, *after_address;
  int before_mach_id, after_mach_id, before_status, after_status, 
    before_ttl, after_ttl;

  if (afterc > MACH_NAME_POS) 
    after_name = after[MACH_NAME_POS];
  if (afterc > MACH_ID_POS)
    after_mach_id = atoi(after[MACH_ID_POS]);
  if (afterc > MACH_VENDOR_POS) 
    after_vendor = after[MACH_VENDOR_POS];
  if (afterc > MACH_MODEL_POS)
    after_model = after[MACH_MODEL_POS];
  if (afterc > MACH_OS_POS) 
    after_os = after[MACH_OS_POS];
  if (afterc > MACH_ADDR_POS) 
    after_address = after[MACH_ADDR_POS];
  if (afterc > MACH_STATUS_POS)
    after_status = atoi(after[MACH_STATUS_POS]);
  if (afterc > MACH_TTL_POS)
    after_ttl = atoi(after[MACH_TTL_POS]);
  if (beforec > MACH_NAME_POS) 
    before_name = before[MACH_NAME_POS];
  if (beforec > MACH_ID_POS)
    before_mach_id = atoi(before[MACH_ID_POS]);
  if (beforec > MACH_VENDOR_POS) 
    before_vendor = before[MACH_VENDOR_POS];
  if (beforec > MACH_MODEL_POS)
    before_model = before[MACH_MODEL_POS];
  if (beforec > MACH_OS_POS) 
    before_os = before[MACH_OS_POS];
  if (beforec > MACH_ADDR_POS) 
    before_address = before[MACH_ADDR_POS];
  if (beforec > MACH_STATUS_POS)
    before_status = atoi(before[MACH_STATUS_POS]);
  if (beforec > MACH_TTL_POS)
    before_ttl = atoi(before[MACH_TTL_POS]);

  check_dns();

  if (beforec == 0) /* Adding a new host */
    {
      if (strcmp(after_address, "unassigned"))
	{
	  if (after_status == 1) 
	    {
	      if (!host_add(after_name, after_address, after_vendor, 
			    after_model, after_os, after_ttl, after_mach_id, 
			    internal_tkey))
		{
		  critical_alert(whoami, "incremental", 
				 "Unable to add internal view host records "
				 "for %s", after_name);
		  return;
		}

	      if (!host_add(after_name, after_address, after_vendor, 
			    after_model, after_os, after_ttl, after_mach_id, 
			    external_tkey))
		{
		  critical_alert(whoami, "incremental", 
				 "Unable to add external view host records "
				 "for %s", after_name);
		  return;
		}
	    }
	  else
	    {
	      if (!host_delete(after_name, after_address, after_vendor, 
			       after_model, after_os, after_ttl, 
			       after_mach_id, internal_tkey))
		{
		  critical_alert(whoami, "incremental", 
				 "Unable to delete internal view host records "
				 "for %s", after_name);
		  return;
		}

	      if (!host_delete(after_name, after_address, after_vendor, 
			       after_model, after_os, after_ttl, 
			       after_mach_id, external_tkey))
		{
		  critical_alert(whoami, "incremental", 
				 "Unable to delete external view host records "
				 "for %s", after_name);
		  return;
		}
	    }
	}
    }
  else if (afterc == 0) /* Deleting a host */
    {
      if (strcmp(before_address, "unassigned"))
	{
	  if (before_status == 1) 
	    {
	      if (!host_delete(before_name, before_address, before_vendor, 
			       before_model, before_os, before_ttl, 
			       before_mach_id, internal_tkey))
		{
		  critical_alert(whoami, "incremental", 
				 "Unable to delete internal view host records "
				 "for %s", before_name);
		  return;
		}

	      if (!host_delete(before_name, before_address, before_vendor, 
			       before_model, before_os, before_ttl, 
			       before_mach_id, external_tkey))
		{
		  critical_alert(whoami, "incremental", 
				 "Unable to delete external view host records "
				 "for %s", before_name);
		  return;
		}
	    }
	}
    }
  else /* Host modification */
    {
      /* Did status change to something that shouldn't exist in DNS? */
      if (before_status == 1 && after_status != 1)
	{
	  if (!delete_hostaliases(before_name, after_name, before_mach_id))
	    {
	      /* delete_hostaliases() will call critical_alert so we don't need to. */
	      com_err(whoami, 0, "Failed to delete host aliases for %s", before_name);
	      return;
	    }

	  /* Only process records with a "real" IP address */
	  if (strcmp(before_address, "unassigned"))
	    {
	      com_err(whoami, 0, "Deleting host record for %s -> %s due to change to status %d",
		      before_name, before_address, after_status);

              if (!host_delete(before_name, before_address, before_vendor,
                               before_model, before_os, before_ttl,
                               before_mach_id, internal_tkey))
                {
                  critical_alert(whoami, "incremental",
                                 "Unable to delete internal view host records "
                                 "for %s", before_name);
                  return;
                }

              if (!host_delete(before_name, before_address, before_vendor,
                               before_model, before_os, before_ttl,
                               before_mach_id, external_tkey))
                {
                  critical_alert(whoami, "incremental",
                                 "Unable to delete external view host records "
                                 "for %s", before_name);
                  return;
                }
	    }
	  /* And we're done */
	  return;
	}

      /* Did status change to something that should exist in DNS? */
      if (before_status != 1 && after_status == 1)
        {
	  /* Only process records with a "real" IP address */
	  if (strcmp(after_address, "unassigned"))
            {
              com_err(whoami, 0, "Adding host record for %s -> %s",
                      after_name, after_address);

              if (!host_add(after_name, after_address, after_vendor,
                            after_model, after_os, after_ttl, after_mach_id,
                            internal_tkey))
                {
                  critical_alert(whoami, "incremental",
                                 "Unable to add internal view host records "
                                 "for %s", after_name);
                  return;
                }

              if (!host_add(after_name, after_address, after_vendor,
                            after_model, after_os, after_ttl, after_mach_id,
                            external_tkey))
                {
                  critical_alert(whoami, "incremental",
                                 "Unable to add external view host records "
                                 "for %s", after_name);
                  return;
                }
	      
	      if (!add_hostaliases(before_name, after_name, after_mach_id))
		{
		   /* add_hostaliases() will call critical_alert so we don't need to. */
		   com_err(whoami, 0, "Failed to add host aliases for %s", after_name);
		   return;
		 }
	      
	      /* And we're done */
	      return;
	    }
	}

       /* Did the address change? */
       if (strcmp(before_address, after_address)) 
	 {
	   if (strcmp(before_address, "unassigned"))
	     {
	       if (!delete_hostaliases(before_name, after_name, before_mach_id))
		 {
		   com_err(whoami, 0, "Failed to delete host aliases for %s", before_name);
		   return;
		 }

	       com_err(whoami, 0, "Deleting host record for %s -> %s", 
		       before_name, before_address);

	       if (!host_delete(before_name, before_address, before_vendor, 
				before_model, before_os, before_ttl, 
				before_mach_id, internal_tkey))
		 {
		   critical_alert(whoami, "incremental", 
				  "Unable to delete internal view host records "
				  "for %s", before_name);
		   return;
		 }

	       if (!host_delete(before_name, before_address, before_vendor, 
				before_model, before_os, before_ttl, 
				before_mach_id, external_tkey))
		 {
		   critical_alert(whoami, "incremental", 
				  "Unable to delete external view host records "
				  "for %s", before_name);
		   return;
		 }
	     }

	   if (strcmp(after_address, "unassigned")) 
	     {
	       com_err(whoami, 0, "Adding host record for %s -> %s", 
		       after_name, after_address);

	       if (!host_add(after_name, after_address, after_vendor, 
			     after_model, after_os, after_ttl, after_mach_id,
			     internal_tkey))
		 {
		   critical_alert(whoami, "incremental", 
				  "Unable to add internal view host records "
				  "for %s", after_name);
		   return;
		 }

	       if (!host_add(after_name, after_address, after_vendor, 
			     after_model, after_os, after_ttl, after_mach_id,
			     external_tkey))
		 {
		   critical_alert(whoami, "incremental", 
				  "Unable to add external view host records "
				  "for %s", after_name);
		   return;
		 }

	       if (!add_hostaliases(before_name, after_name, after_mach_id))
		 {
		   com_err(whoami, 0, "Failed to add host aliases for %s", after_name);
		   return;
		 }
	     }
	 }
       else /* Status and address are the same, update host metadata */
	 {

	   if (strcmp(before_name, after_name))
	     {
	       /* Handle CNAMEs */
	       if (!delete_hostaliases(before_name, after_name, before_mach_id))
		 {
		   com_err(whoami, 0, "Failed to delete host aliases for %s", before_name);
		   return;
		 }
	     }

	   if (strcmp(after_address, "unassigned"))
	     {
	       com_err(whoami, 0, "Updating host record before %s -> %s, "
		       "after %s -> %s", before_name, before_address, 
		       after_name, after_address);

	       if (!host_update(before_name, before_address, before_vendor, 
				before_model, before_os, before_ttl, 
				before_mach_id, after_name, after_address, 
				after_vendor, after_model, after_os, 
				after_ttl, after_mach_id, internal_tkey))
		 {
		   critical_alert(whoami, "incremental", 
				  "Unable to update internal view host " 
				  "records for %s -> %s", 
				  before_name, after_name);
		   return;
		 }

	       if (!host_update(before_name, before_address, before_vendor, 
				before_model, before_os, before_ttl, 
				before_mach_id, after_name, after_address, 
				after_vendor, after_model, after_os, 
				after_ttl, after_mach_id, external_tkey))
		 {
		   critical_alert(whoami, "incremental", 
				  "Unable to update external view host " 
				  "records for %s -> %s", 
				  before_name, after_name);
		   return;
		 }

	       if (strcmp(before_name, after_name))
		 {
		   /* Handle CNAMEs */
		   if (!add_hostaliases(before_name, after_name, after_mach_id))
		     {
		       com_err(whoami, 0, "Failed to add host aliases for %s", after_name);
		       return;
		     }
		 }
	     }
	 }
    }
}

 void do_hostalias(char **before, int beforec, char **after, int afterc)
 {
   char *before_name, *after_name, *before_alias, *after_alias;
   int before_mach_id, after_mach_id;

   if (afterc > ALIAS_POS) 
     after_alias = after[ALIAS_POS];
   if (afterc > ALIAS_ID_POS)
     after_mach_id = atoi(after[ALIAS_ID_POS]);
   if (afterc > ALIAS_NAME_POS) 
     after_name = after[ALIAS_NAME_POS];
   if (beforec > ALIAS_POS) 
     before_alias = before[ALIAS_POS];
   if (beforec > ALIAS_ID_POS)
     before_mach_id = atoi(before[ALIAS_ID_POS]);
   if (beforec > ALIAS_NAME_POS) 
     before_name = before[ALIAS_NAME_POS];

   check_dns();

   if (beforec == 0) 
     {
       com_err(whoami, 0, "Adding hostalias %s for %s", after_alias, 
	       after_name);

       if (!alias_add(after_alias, after_name, DEFAULT_TTL, after_mach_id, 
		      internal_tkey))
	 {
	   critical_alert(whoami, "incremental", 
			  "Unable to add internal view host alias for %s",
			  after_alias);
	   return;
	 }

       if (!alias_add(after_alias, after_name, DEFAULT_TTL, after_mach_id, 
		      external_tkey))
	 {
	   critical_alert(whoami, "incremental", 
			  "Unable to add external view host alias for %s",
			  after_alias);
	   return;
	 }
     }
   else if (afterc == 0) 
     {
       com_err(whoami, 0, "Deleting hostalias %s for %s", before_alias,
	       before_name);

       if (!alias_delete(before_alias, before_name, DEFAULT_TTL, 
			 before_mach_id, internal_tkey))
	 {
	   critical_alert(whoami, "incremental",
			  "Unable to delete internal view host alias for %s",
			  before_alias);
	   return;
	 }

       if (!alias_delete(before_alias, before_name, DEFAULT_TTL, 
			 before_mach_id, external_tkey))
	 {
	   critical_alert(whoami, "incremental",
			  "Unable to delete external view host alias for %s",
			  before_alias);
	   return;
	 }
     }
 }

 int get_hmac_key(char *file, char **buffer)
 {
   int keys;
   struct stat statb;

   if (stat(file, &statb) == -1)
     {
       return 0;
     }

   (*buffer) = malloc(statb.st_size + 1);
   if (!(*buffer))
     {
       return 0;
     }

   keys = open(file, 0, O_RDONLY);
   if (keys)
     {
       read(keys, (*buffer), statb.st_size);
       close(keys);
       (*buffer)[statb.st_size - 1] = '\0'; 
     }
   else
     return 0;

   close(keys);
   return 1;
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
       return 0;
     }

   servers = fopen(file, "r");
   if (servers != NULL)
     {
       while (fgets(line, sizeof(line), servers) != NULL)
	 {
	   buffer[(*nservers)] = malloc(sizeof(line));
	   line[strlen(line) - 1] = '\0';
	   strcpy(buffer[(*nservers)], line);
	   (*nservers)++;
	 }
     }
   else
     return 0;

   fclose(servers);
   return 1;
 }

 void check_dns(void)
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

 int do_update(char *host, char *data, char *vendor, char *model, char *os, 
	       int ttl, int mach_id, char *server, int class, int type, 
	       ns_tsig_key *tkey)
 {
   ns_updque updqueue;
   ns_updrec *rrecp;
   struct __res_state res;
   u_char packet[NS_MAXMSG];
   u_char answer[NS_MAXMSG];
   struct hostent *he;
   struct in_addr **addr_list;
   struct sockaddr_in addr;
   char ip[INET_ADDRSTRLEN];
   /* + strlen("mitMoiraId:") */
   char mitMoiraId[MACHINE_MACH_ID_SIZE + 12];
   char hinfo[MACHINE_VENDOR_SIZE + MACHINE_MODEL_SIZE + MACHINE_OS_SIZE + 3];
   int len;

   memset(&res, 0, sizeof(res));
   memset(&packet, 0, sizeof(packet));
   memset(&answer, 0, sizeof(answer));
   memset(&hinfo, 0, sizeof(hinfo));

   /* Setup the information for querying specific server */

   if ((he = gethostbyname(server)) == NULL) 
     {
       com_err(whoami, 0, "gethostbyname unable to resolve %s", server);
       return 0;
     }

   addr_list = (struct in_addr **) he->h_addr_list;

   if (addr_list[0] != NULL)
     {
       strcpy(ip, inet_ntoa(*addr_list[0]));
     }
   else 
     {
       com_err(whoami, 0, "No IP addresses returned for %s", server);
       return 0;
     }

   inet_pton(AF_INET, ip, &(addr.sin_addr));

   res_ninit(&res);

   res.nsaddr_list[0].sin_addr = addr.sin_addr;
   res.nscount = 1;
   res.retrans = 3;
   res.retry = 3;
   res.options &= ~RES_RECURSE;
   res.options &= ~(RES_DNSRCH | RES_DEFNAMES);

   /* 
    * Create a test update request first to determine if server is processing
    * updates or not
    */

   /* Delete the existing record */

   rrecp = res_mkupdrec(S_UPDATE, host, class, T_ANY, 0);

   rrecp->r_data = (unsigned char *)0;
   rrecp->r_opcode = DELETE;
   rrecp->r_size = 0;

   INIT_LIST(updqueue);
   APPEND(updqueue, rrecp, r_glink);

   /* Now create the new record */

   if (*data)
     {
       rrecp = res_mkupdrec(S_UPDATE, host, class, type, ttl);

       rrecp->r_data = data;
       rrecp->r_opcode = ADD;
       rrecp->r_size = strlen(data);

       APPEND(updqueue, rrecp, r_glink);
     }

   /* Add the hinfo to the record */

   if ((*vendor || * model) && *os)
     {
       if (*vendor && *model)
	 {
	   sprintf(hinfo, "%s/%s %s", vendor, model, os);
	 }
       else
	 {
	   sprintf(hinfo, "%s%s %s", vendor, model, os);
	 }

       rrecp = res_mkupdrec(S_UPDATE, host, class, T_HINFO, ttl);

       rrecp->r_data = hinfo;
       rrecp->r_opcode = ADD;
       rrecp->r_size = strlen(hinfo);

       APPEND(updqueue, rrecp, r_glink);
     }

   /* Update the mitMoiraId attribute */

   if (mach_id)
     {
       sprintf(mitMoiraId, "mitMoiraId:%d", mach_id);

       rrecp = res_mkupdrec(S_UPDATE, host, class, T_TXT, ttl);

       rrecp->r_data = mitMoiraId;
       rrecp->r_opcode = ADD;
       rrecp->r_size = strlen(mitMoiraId);

       APPEND(updqueue, rrecp, r_glink);
     }

   /* Prepend the zone information to the packet as required by the protocol */

   if (type == T_PTR)
     {
       rrecp = res_mkupdrec(S_ZONE, DEFAULT_PTR_ZONE, class, T_SOA, 0);
     }
   else
     {
       rrecp = res_mkupdrec(S_ZONE, DEFAULT_ZONE, class, T_SOA, 0);
     }

   PREPEND(updqueue, rrecp, r_glink);

   /* Create the packet to send to the server */

   if ((len = (res_nmkupdate(&res, HEAD(updqueue), packet, 
				  NS_MAXMSG))) <= 0)
     {
       com_err(whoami, 0, "Failed to create update packet");
       return 0;
     }

   if ((len = (res_nsendsigned(&res, packet, len, tkey, answer, 
				  sizeof(answer)))) <= 0)
     {
       com_err(whoami, 0, "Error sending dns update packet");
       return 0;
     }

   return 1;
 }

 int do_query(char *host, char *server, int class, int type, char **result, 
	      int *nresults, ns_tsig_key *tkey)
 {
   int len, i;
   ns_msg handle;
   ns_rr rr;
   struct sockaddr_in addr;
   struct __res_state res;
   struct hostent *he;
   struct in_addr **addr_list;
   u_char packet[NS_MAXMSG];
   u_char answer[NS_MAXMSG];
   char ip[INET_ADDRSTRLEN];
   HEADER *hp = (HEADER *) answer;
   int records;

   memset(&res, 0, sizeof(res));
   memset(&packet, 0, sizeof(packet));
   memset(&answer, 0, sizeof(answer));

   (*nresults) = 0;

   if ((he = gethostbyname(server)) == NULL) 
     {
       com_err(whoami, 0, "gethostbyname unable to resolve %s", server);
       return 0;
     }

   addr_list = (struct in_addr **) he->h_addr_list;

   if (addr_list[0] != NULL)
     {
       strcpy(ip, inet_ntoa(*addr_list[0]));
     }
   else 
     {
       com_err(whoami, 0, "No IP addresses returned for %s", server);
       return 0;
     }

   inet_pton(AF_INET, ip, &(addr.sin_addr));

   res_ninit(&res);

   res.nsaddr_list[0].sin_addr = addr.sin_addr;
   res.nscount = 1;
   res.retrans = 3;
   res.retry = 3;
   res.options &= ~RES_RECURSE;
   res.options &= ~(RES_DNSRCH | RES_DEFNAMES);

   hp->rcode = NOERROR;

   if ((len = (res_nmkquery(&res, QUERY, 
			    host, class, type, NULL, 0, 
			    NULL, packet, sizeof(packet)))) <= 0)
     {
       com_err(whoami, 0, "Failed to create query packet");
       return 0;
     }

   if ((len = (res_nsendsigned(&res, packet, len, tkey, answer, 
			       sizeof(answer)))) <= 0)
     {
       com_err(whoami, 0, "Error sending dns query packet");
       return 0;
     }

   if (ns_initparse(answer, len, &handle) < 0) {
     com_err(whoami, 0, "Unable to parse query response");
     return 0;
   }

   if (ns_msg_getflag(handle, ns_f_rcode) != ns_r_noerror && 
       ns_msg_getflag(handle, ns_f_rcode) != ns_r_nxdomain)
     {
       com_err(whoami, 0, "Unable to get response flags %d",
	       ns_msg_getflag(handle, ns_f_rcode));
       return 0;
     }

   records = ns_msg_count(handle, ns_s_an);

   if (records < 0)
     {
       com_err(whoami, 0, "Dns response contained no resource records");
       return 0;
     }

   for (i = 0; i < records; i++)
     {
       if (ns_parserr(&handle, ns_s_an, i, &rr)) {
	 com_err(whoami, 0, "Error parsing resource record");
	 return 0;
       }

       if (ns_rr_type(rr) != type) 
	 {
	   com_err(whoami, 0, "Resource record type does not match expected " 
		   "type");
	   return 0;
	 }

       if (ns_rr_class(rr) == ns_c_in && ns_rr_type(rr) == T_TXT) 
	 {
	   char *c = strdup((char *)(ns_rr_rdata(rr) + 1));
	   c[ns_rr_rdlen(rr) - 1] = '\0';

	   result[(*nresults)] = malloc(strlen(c) + 1);
	   strcpy(result[(*nresults)], c);
	   free(c);
	   (*nresults)++;
	 }

       if (ns_rr_class(rr) == ns_c_in && ns_rr_type(rr) == T_PTR) 
	 {
	   char buf[256];
	   ns_name_uncompress(answer, answer + len, ns_rr_rdata(rr), buf, 256);
	   result[(*nresults)] = malloc(MAXDNAME);
	   strcpy(result[(*nresults)], buf);
	   (*nresults)++;
	 }

       if (ns_rr_class(rr) == ns_c_in && ns_rr_type(rr) == T_A) 
	 {
	   uint8_t ip[4];
	   uint32_t ttl = ns_rr_ttl(rr);
	   memcpy(ip, ns_rr_rdata(rr), sizeof(ip));

	   result[(*nresults)] = malloc(4 + (3 * 4));
	   sprintf(result[(*nresults)], "%u.%u.%u.%u", ip[0], ip[1], ip[2],
		   ip[3]);
	   (*nresults)++;
	 }
     }

   res_nclose(&res);
   res_ndestroy(&res);

   return 1;
 }

 char *reverse_ip(char *ip)
 {
   in_addr_t addr;
   char reversed_ip[INET_ADDRSTRLEN];

   inet_pton(AF_INET, ip, &addr);

   addr =
     ((addr & 0xff000000) >> 24) |
     ((addr & 0x00ff0000) >>  8) |
     ((addr & 0x0000ff00) <<  8) |
     ((addr & 0x000000ff) << 24);

   inet_ntop(AF_INET, &addr, reversed_ip, sizeof(reversed_ip));
   return strdup(reversed_ip);
 }

 int host_update(char *before_name, char *before_address, char *before_vendor,
		 char *before_model, char *before_os, int before_ttl, 
		 int before_mach_id, char *after_name, char *after_address,
		 char *after_vendor, char *after_model, char *after_os, 
		 int after_ttl, int after_mach_id, ns_tsig_key *tkey)

 {
   char *servers[MAX_SERVERS];
   int nservers, nresults, i, j;
   char *results[MAX_RESULTS];
   int before_a_records, after_a_records, before_cname_records, 
     after_cname_records, before_txt_records, after_txt_records, 
     before_ptr_records, after_ptr_records;
   char mitMoiraId[BUFSIZ], before_mitMoiraId[BUFSIZ], after_mitMoiraId[BUFSIZ];
   char before_ptr_name[MAXDNAME], after_ptr_name[MAXDNAME];

   sprintf(before_mitMoiraId, "mitMoiraId:%d", before_mach_id);
   sprintf(after_mitMoiraId, "mitMoiraId:%d", after_mach_id);
   sprintf(before_ptr_name, "%s.IN-ADDR.ARPA", reverse_ip(before_address));
   sprintf(after_ptr_name, "%s.IN-ADDR.ARPA", reverse_ip(after_address));

   /* 
    * Lets fetch the DNS servers we are going to query
    */

   if (!get_servers(SERVERS_FILE, servers, &nservers))
     {
       com_err(whoami, 0, "Unable to retrieve dns server names");
       return 0;
     }

   /* 
    * First handle the A records
    * Fetch the txt records associated with the host record to determine 
    * whether or not its something under our management
    */

   for (i = 0; i < nservers; i++)
     {
       com_err(whoami, 0, "Querying server %s", servers[i]);

       memset(mitMoiraId, 0, sizeof(mitMoiraId));

       if (!do_query(before_name, servers[i], C_IN, T_A, results, &nresults, 
		     tkey))
	 {
	   com_err(whoami, 0, "Error in querying dns A records for %s", 
		   before_name);
	   return 0;
	 }

       before_a_records = nresults;

       for (j = 0; j < nresults; j++)
	 {
	   free(results[j]);
	 }

       if (!do_query(before_name, servers[i], C_IN, T_CNAME, results, 
		     &nresults, tkey))
	 {
	   com_err(whoami, 0, "Error in querying dns CNAME records for %s", 
		   before_name);
	   return 0;
	 }

       before_cname_records = nresults;

       for (j = 0; j < nresults; j++)
	 {
	   free(results[j]);
	 }

       /*
	* If the record exists in DNS lets fetch txt records for it
	*/

       if (before_a_records != 0)
	 {
	   if (!do_query(before_name, servers[i], C_IN, T_TXT, results, 
			 &nresults, tkey))
	     {
	       com_err(whoami, 0, "Error in querying dns TXT records for %s",
		       before_name);
	       return 0;
	     }

	   before_txt_records = nresults;

	   for (j = 0; j < nresults; j++) 
	     {
	       if (!strncmp(results[j], "mitMoiraId:", 11))
		 {
		   strcpy(mitMoiraId, results[j]);
		 }
	     }

	   for (j = 0; j < nresults; j++)
	     {
	       free(results[j]);
	     }
	 }

       /*
	* Lets check if we found an mitMoiraId or not and if so if it 
	* matches what we have in our records. 
	*/

       if (before_a_records)
	 {
	   if (!*mitMoiraId)
	     {
	       com_err(whoami, 0, "%s does not have mitMoiraId attribute, " 
		       "skipping", before_name);
	       return 0;
	     }

	   if (strcmp(mitMoiraId, before_mitMoiraId))
	     {
	       com_err(whoami, 0, "Mismatching mitMoiraId %s for %s does not "
		       "match moira value %s", before_name, before_mitMoiraId,
		       mitMoiraId);
	       return 0;
	     }
	 }

       /* Lets check if its a ptr record we are managing */

       if(!strncmp(before_address, DEFAULT_PTR_ZONE, 3))
	 {
	   memset(mitMoiraId, 0, sizeof(mitMoiraId));

	   if (!do_query(before_ptr_name, servers[i], C_IN, T_PTR, results, 
			 &nresults, tkey))
	     {
	       com_err(whoami, 0, "Error in querying dns PTR records for %s", 
		       before_ptr_name);
	       return 0;
	     }

	   before_ptr_records = nresults;

	   for (j = 0; j < nresults; j++)
	     {
	       free(results[j]);
	     }

	   if (!do_query(before_ptr_name, servers[i], C_IN, T_CNAME, results, 
			 &nresults, tkey))
	     {
	       com_err(whoami, 0, "Error in querying dns CNAME records for %s", 
		       before_ptr_name);
	       return 0;
	     }

	   before_cname_records = nresults;

	   for (j = 0; j < nresults; j++)
	     {
	       free(results[j]);
	     }

	   /*
	    * If the record exists in DNS lets fetch txt records for it
	    */

	   if (before_ptr_records != 0)
	     {
	       if (!do_query(before_ptr_name, servers[i], C_IN, T_TXT, results, 
			     &nresults, tkey))
		 {
		   com_err(whoami, 0, 
			   "Error in querying dns TXT records for %s",
			   before_ptr_name);
		   return 0;
		 }

	       before_txt_records = nresults;

	       for (j = 0; j < nresults; j++) 
		 {
		   if (!strncmp(results[j], "mitMoiraId:", 11))
		     {
		       strcpy(mitMoiraId, results[j]);
		     }
		 }

	       for (j = 0; j < nresults; j++)
		 {
		   free(results[j]);
		 }
	     }

	   /*
	    * Lets check if we found an mitMoiraId or not and if so if it 
	    * matches what we have in our records. 
	    */

	   if (before_ptr_records)
	     {
	       if (!*mitMoiraId)
		 {
		   com_err(whoami, 0, "%s does not have mitMoiraId attribute, "
			   "skipping", before_ptr_name);
		   return 0;
		 }

	       if (strcmp(mitMoiraId, before_mitMoiraId))
		 {
		   com_err(whoami, 0, "Mismatching mitMoiraId %s for %s does "
			   "not match moira value %s", before_ptr_name, 
			   before_mitMoiraId, mitMoiraId);
		   return 0;
		 }
	     }
	 }

       /* Check that the new name if it exists matches or doesn't exist */

       memset(mitMoiraId, 0, sizeof(mitMoiraId));

       if (!do_query(after_name, servers[i], C_IN, T_A, results, &nresults, 
		     tkey))
	 {
	   com_err(whoami, 0, "Error in querying dns A records for %s", 
		   after_name);
	   return 0;
	 }

       after_a_records = nresults;

       for (j = 0; j < nresults; j++)
	 {
	   free(results[j]);
	 }

       if (!do_query(after_name, servers[i], C_IN, T_CNAME, results, 
		     &nresults, tkey))
	 {
	   com_err(whoami, 0, "Error in querying dns CNAME records for %s", 
		   after_name);
	   return 0;
	 }

       after_cname_records = nresults;

       for (j = 0; j < nresults; j++)
	 {
	   free(results[j]);
	 }

       /*
	* If the record exists in DNS lets fetch txt records for it
	*/

       if (after_a_records != 0)
	 {
	   if (!do_query(after_name, servers[i], C_IN, T_TXT, results, 
			 &nresults, tkey))
	     {
	       com_err(whoami, 0, "Error in querying dns TXT records for %s",
		       after_name);
	       return 0;
	     }

	   after_txt_records = nresults;

	   for (j = 0; j < nresults; j++) 
	     {
	       if (!strncmp(results[j], "mitMoiraId:", 11))
		 {
		   strcpy(mitMoiraId, results[j]);
		 }
	     }

	   for (j = 0; j < nresults; j++)
	     {
	       free(results[j]);
	     }
	 }

       /*
	* Lets check if we found an mitMoiraId or not and if so if it 
	* matches what we have in our records. 
	*/

       if (after_a_records)
	 {
	   if (!*mitMoiraId)
	     {
	       com_err(whoami, 0, "%s does not have mitMoiraId attribute, " 
		       "skipping", after_name);
	       return 0;
	     }

	   if (strcmp(mitMoiraId, after_mitMoiraId))
	     {
	       com_err(whoami, 0, "Mismatching mitMoiraId %s for %s does not "
		       "match moira value %s", after_name, after_mitMoiraId,
		       mitMoiraId);
	       return 0;
	     }
	 }

       /* Lets check if its a ptr record we are managing */

       if(!strncmp(after_address, DEFAULT_PTR_ZONE, 3))
	 {
	   memset(mitMoiraId, 0, sizeof(mitMoiraId));

	   if (!do_query(after_ptr_name, servers[i], C_IN, T_PTR, results, 
			 &nresults, tkey))
	     {
	       com_err(whoami, 0, "Error in querying dns PTR records for %s", 
		       after_ptr_name);
	       return 0;
	     }

	   after_ptr_records = nresults;

	   for (j = 0; j < nresults; j++)
	     {
	       free(results[j]);
	     }

	   if (!do_query(after_ptr_name, servers[i], C_IN, T_CNAME, results, 
			 &nresults, tkey))
	     {
	       com_err(whoami, 0, "Error in querying dns CNAME records for %s", 
		       after_ptr_name);
	       return 0;
	     }

	   after_cname_records = nresults;

	   for (j = 0; j < nresults; j++)
	     {
	       free(results[j]);
	     }

	   /*
	    * If the record exists in DNS lets fetch txt records for it
	    */

	   if (after_ptr_records != 0)
	     {
	       if (!do_query(after_ptr_name, servers[i], C_IN, T_TXT, results, 
			     &nresults, tkey))
		 {
		   com_err(whoami, 0, 
			   "Error in querying dns TXT records for %s",
			   after_ptr_name);
		   return 0;
		 }

	       after_txt_records = nresults;

	       for (j = 0; j < nresults; j++) 
		 {
		   if (!strncmp(results[j], "mitMoiraId:", 11))
		     {
		       strcpy(mitMoiraId, results[j]);
		     }
		 }

	       for (j = 0; j < nresults; j++)
		 {
		   free(results[j]);
		 }
	     }

	   /*
	    * Lets check if we found an mitMoiraId or not and if so if it 
	    * matches what we have in our records. 
	    */

	   if (after_ptr_records)
	     {
	       if (!*mitMoiraId)
		 {
		   com_err(whoami, 0, "%s does not have mitMoiraId attribute, "
			   "skipping", after_ptr_name);
		   return 0;
		 }

	       if (strcmp(mitMoiraId, after_mitMoiraId))
		 {
		   com_err(whoami, 0, "Mismatching mitMoiraId %s for %s does "
			   "not match moira value %s", after_ptr_name, 
			   after_mitMoiraId, mitMoiraId);
		   return 0;
		 }
	     }
	 }
     }

   for (i = 0; i < nservers; i++)
     {
       /*
	* We know its one of our hosts or it does not exist yet in the DNS
	*/

       com_err(whoami, 0, "Updating server %s", servers[i]);

       if (!do_update(after_name, after_address, after_vendor, after_model, 
		      after_os, after_ttl, after_mach_id, servers[i], C_IN, 
		      T_A, tkey))
	 {
	   com_err(whoami, 0, "Error updating A record for %s", after_name);
	   return 0;
	 }

       if (!do_update(after_ptr_name, after_name, after_vendor, after_model, 
		      after_os, after_ttl, after_mach_id, servers[i], C_IN, 
		      T_PTR, tkey))
	 {
	   com_err(whoami, 0, "Error updating PTR record for %s", 
		   after_ptr_name);
	   return 0;
	 }
     }

   for (i = 0; i < nservers; i++)
     {
       free(servers[i]);
     }

   return 1;
 }

 int host_add(char *after_name, char *after_address,
	      char *after_vendor, char *after_model, char *after_os, 
	      int after_ttl, int after_mach_id, ns_tsig_key *tkey)

 {
   char *servers[MAX_SERVERS];
   int nservers, nresults, i, j;
   char *results[MAX_RESULTS];
   int before_a_records, after_a_records, before_cname_records, 
     after_cname_records, before_txt_records, after_txt_records, 
     before_ptr_records, after_ptr_records;
   char mitMoiraId[BUFSIZ], before_mitMoiraId[BUFSIZ], after_mitMoiraId[BUFSIZ];
   char before_ptr_name[MAXDNAME], after_ptr_name[MAXDNAME];

   sprintf(after_mitMoiraId, "mitMoiraId:%d", after_mach_id);
   sprintf(after_ptr_name, "%s.IN-ADDR.ARPA", reverse_ip(after_address));

   /* 
    * Lets fetch the DNS servers we are going to query
    */

   if (!get_servers(SERVERS_FILE, servers, &nservers))
     {
       com_err(whoami, 0, "Unable to retrieve dns server names");
       return 0;
     }

   /* 
    * First handle the A records
    * Fetch the txt records associated with the host record to determine 
    * whether or not its something under our management
    */

   for (i = 0; i < nservers; i++)
     {
       com_err(whoami, 0, "Querying server %s", servers[i]);

       memset(mitMoiraId, 0, sizeof(mitMoiraId));

       if (!do_query(after_name, servers[i], C_IN, T_A, results, &nresults, 
		     tkey))
	 {
	   com_err(whoami, 0, "Error in querying dns A records for %s", 
		   after_name);
	   return 0;
	 }

       after_a_records = nresults;

       for (j = 0; j < nresults; j++)
	 {
	   free(results[j]);
	 }

       if (!do_query(after_name, servers[i], C_IN, T_CNAME, results, &nresults, 
		     tkey))
	 {
	   com_err(whoami, 0, "Error in querying dns CNAME records for %s",
		   after_name);
	   return 0;
	 }

       after_cname_records = nresults;

       for (j = 0; j < nresults; j++)
	 {
	   free(results[j]);
	 }

       /*
	* If the record exists in DNS lets fetch txt records for it
	*/

       if (after_a_records != 0)
	 {
	   if (!do_query(after_name, servers[i], C_IN, T_TXT, results, 
			 &nresults, tkey))
	     {
	       com_err(whoami, 0, "Error in querying dns TXT records for %s", 
		       after_name);
	       return 0;
	     }

	   after_txt_records = nresults;

	   for (j = 0; j < nresults; j++) 
	     {
	       if (!strncmp(results[j], "mitMoiraId:", 11))
		 {
		   strcpy(mitMoiraId, results[j]);
		 }
	     }

	   for (j = 0; j < nresults; j++)
	     {
	       free(results[j]);
	     }
	 }

       /*
	* Lets check if we found an mitMoiraId or not and if so if it 
	* matches what we have in our records. 
	*/

       if (after_a_records)
	 {
	   if (!*mitMoiraId)
	     {
	       com_err(whoami, 0, "%s does not have mitMoiraId attribute, "
		       "skipping", after_name);
	       return 0;
	     }

	   if (strcmp(mitMoiraId, after_mitMoiraId))
	     {
	       com_err(whoami, 0, "Mismatching mitMoiraId %s for %s does not "
		       "match moira value %s", after_name, after_mitMoiraId,
		       mitMoiraId);
	       return 0;
	     }
	 }

       /* Check if the ip address is one we should be managing */

       if(!strncmp(after_address, DEFAULT_PTR_ZONE, 3)) 
	 {
	   memset(mitMoiraId, 0, sizeof(mitMoiraId));

	   if (!do_query(after_ptr_name, servers[i], C_IN, T_PTR, results, 
			 &nresults, tkey))
	     {
	       com_err(whoami, 0, "Error in querying dns PTR records for %s", 
		       after_ptr_name);
	       return 0;
	     }

	   after_ptr_records = nresults;

	   for (j = 0; j < nresults; j++)
	     {
	       free(results[j]);
	     }

	   if (!do_query(after_ptr_name, servers[i], C_IN, T_CNAME, results, 
			 &nresults, tkey))
	     {
	       com_err(whoami, 0, "Error in querying dns CNAME records for %s",
		       after_ptr_name);
	       return 0;
	     }

	   after_cname_records = nresults;

	   for (j = 0; j < nresults; j++)
	     {
	       free(results[j]);
	     }

	   /*
	    * If the record exists in DNS lets fetch txt records for it
	    */

	   if (after_ptr_records != 0)
	     {
	       if (!do_query(after_ptr_name, servers[i], C_IN, T_TXT, results, 
			     &nresults, tkey))
		 {
		   com_err(whoami, 0, 
			   "Error in querying dns TXT records for %s",
			   after_ptr_name);
		   return 0;
		 }

	       after_txt_records = nresults;

	       for (j = 0; j < nresults; j++) 
		 {
		   if (!strncmp(results[j], "mitMoiraId:", 11))
		     {
		       strcpy(mitMoiraId, results[j]);
		     }
		 }

	       for (j = 0; j < nresults; j++)
		 {
		   free(results[j]);
		 }
	     }

	   /*
	    * Lets check if we found an mitMoiraId or not and if so if it 
	    * matches what we have in our records. 
	    */

	   if (after_ptr_records)
	     {
	       if (!*mitMoiraId)
		 {
		   com_err(whoami, 0, "%s does not have mitMoiraId attribute, skipping\n", 
			  after_ptr_name);
		   return -1;
		 }

	       if (strcmp(mitMoiraId, after_mitMoiraId))
		 {
		   com_err(whoami, 0, "Mismatching mitMoiraId %s for %s does "
			   "not match moira value %s", after_ptr_name,
			   after_mitMoiraId, mitMoiraId);
		   return 0;
		 }
	     }
	 }
     }

   for (i = 0; i < nservers; i++)
     {
       /*
	* We know its one of our hosts or it does not exist yet in the DNS
	*/

       com_err(whoami, 0, "Updating server %s", servers[i]);

       if (!do_update(after_name, after_address, after_vendor, after_model, 
		      after_os, after_ttl, after_mach_id, servers[i], C_IN, 
		      T_A, tkey))
	 {

	   com_err(whoami, 0, "Error adding A record for %s", after_name);
	   return 0;
	 }

       if(!strncmp(after_address, DEFAULT_PTR_ZONE, 3)) 
	 {
	   if (!do_update(after_ptr_name, after_name, after_vendor, 
			  after_model, after_os, after_ttl, after_mach_id, 
			  servers[i], C_IN, T_PTR, tkey))
	     {
	       com_err(whoami, 0, "Error adding PTR record for %s",
		       after_ptr_name);
	       return 0;
	     }
	 }
     }

   for (i = 0; i < nservers; i++)
     {
       free(servers[i]);
     }

   return 1;
 }

 int alias_add(char *after_alias, char *after_name,
	       int after_ttl, int after_mach_id, ns_tsig_key *tkey)

 {
   char *servers[MAX_SERVERS];
   int nservers, nresults, i, j;
   char *results[MAX_RESULTS];
   int before_a_records, after_a_records, before_cname_records, 
     after_cname_records, before_txt_records, after_txt_records;
   char mitMoiraId[BUFSIZ], before_mitMoiraId[BUFSIZ], after_mitMoiraId[BUFSIZ];
   char before_ptr_name[MAXDNAME], after_ptr_name[MAXDNAME];

   sprintf(after_mitMoiraId, "mitMoiraId:%d", after_mach_id);

   /* 
    * Lets fetch the DNS servers we are going to query
    */

   if (!get_servers(SERVERS_FILE, servers, &nservers))
     {
       com_err(whoami, 0, "Unable to retrieve dns server names");
       return 0;
     }

   /* 
    * First handle the A records
    * Fetch the txt records associated with the host record to determine 
    * whether or not its something under our management
    */

   for (i = 0; i < nservers; i++)
     {
       if(strlen(after_name) > 8 && 
	  !strncasecmp(after_name + strlen(after_name) - 8, ".MIT.EDU", 8))
	 {
	   com_err(whoami, 0, "Querying server %s", servers[i]);

	   memset(mitMoiraId, 0, sizeof(mitMoiraId));

	   if (!do_query(after_name, servers[i], C_IN, T_A, results, &nresults, 
			 tkey))
	     {
	       com_err(whoami, 0, "Error in querying dns A records for %s",
		       after_name);
	       return 0;
	     }

	   after_a_records = nresults;

	   for (j = 0; j < nresults; j++)
	     {
	       free(results[j]);
	     }

	   if (!do_query(after_name, servers[i], C_IN, T_CNAME, results, 
			 &nresults, tkey))
	     {
	       com_err(whoami, 0, "Error in querying dns CNAME records for %s", 
		       after_name);
	       return 0;
	     }

	   after_cname_records = nresults;

	   for (j = 0; j < nresults; j++)
	     {
	       free(results[j]);
	     }

	   /*
	    * If the record exists in DNS lets fetch txt records for it
	    */

	   if (after_a_records != 0)
	     {
	       if (!do_query(after_name, servers[i], C_IN, T_TXT, results, 
			     &nresults, tkey))
		 {
		   com_err(whoami, 0, 
			   "Error in querying dns TXT records for %s",
			   after_name);
		   return 0;
		 }

	       after_txt_records = nresults;

	       for (j = 0; j < nresults; j++) 
		 {
		   if (!strncmp(results[j], "mitMoiraId:", 11))
		     {
		       strcpy(mitMoiraId, results[j]);
		     }
		 }

	       for (j = 0; j < nresults; j++)
		 {
		   free(results[j]);
		 }
	     }
	   else 
	     {
	       com_err(whoami, 0, "A record does not exist for %s", 
		       after_name);
	       return 0;
	     }

	   /*
	    * Lets check if we found an mitMoiraId or not and if so if it 
	    * matches what we have in our records. 
	    */

	   if (after_a_records)
	     {
	       if (!*mitMoiraId)
		 {
		   com_err(whoami, 0, "%s does not have mitMoiraId attribute, "
			   "skipping", after_name);
		   return 0;
		 }

	       if (strcmp(mitMoiraId, after_mitMoiraId))
		 {
		   com_err(whoami, 0, "Mismatching mitMoiraId %s for %s does "
			   "not match moira value %s", after_name, 
			   after_mitMoiraId, mitMoiraId);
		   return 0;
		 }
	     }

	   memset(mitMoiraId, 0, sizeof(mitMoiraId));

	 }
     }

   for (i = 0; i < nservers; i++)
     {
       /*
	* We know its one of our hosts or it does not exist yet in the DNS
	*/

       com_err(whoami, 0, "Updating server %s", servers[i]);

       if (!do_update(after_alias, after_name, "", "", "", after_ttl, 0, 
		      servers[i], C_IN, T_CNAME, tkey))
	 {
	   com_err(whoami, 0, "Error adding CNAME record for %s", 
		   after_alias);
	   return 0;
	 }
     }

   for (i = 0; i < nservers; i++)
     {
       free(servers[i]);
     }

   return 1;
 }

 int host_delete(char *before_name, char *before_address,
		 char *before_vendor, char *before_model, char *before_os, 
		 int before_ttl, int before_mach_id, ns_tsig_key *tkey)

 {
   char *servers[MAX_SERVERS];
   int nservers, nresults, i, j;
   char *results[MAX_RESULTS];
   int before_a_records, after_a_records, before_cname_records, 
     after_cname_records, before_txt_records, after_txt_records, 
     before_ptr_records, after_ptr_records;
   char mitMoiraId[BUFSIZ], before_mitMoiraId[BUFSIZ], after_mitMoiraId[BUFSIZ];
   char before_ptr_name[MAXDNAME], after_ptr_name[MAXDNAME];

   sprintf(before_mitMoiraId, "mitMoiraId:%d", before_mach_id);
   sprintf(before_ptr_name, "%s.IN-ADDR.ARPA", reverse_ip(before_address));

   /* 
    * Lets fetch the DNS servers we are going to query
    */

   if (!get_servers(SERVERS_FILE, servers, &nservers))
     {
       com_err(whoami, 0, "Unable to retrieve dns server names");
       return 0;
     }

   /* 
    * First handle the A records
    * Fetch the txt records associated with the host record to determine 
    * whether or not its something under our management
    */

   for (i = 0; i < nservers; i++)
     {
       com_err(whoami, 0, "Querying server %s", servers[i]);

       memset(mitMoiraId, 0, sizeof(mitMoiraId));

       if (!do_query(before_name, servers[i], C_IN, T_A, results, &nresults, 
		     tkey))
	 {
	   com_err(whoami, 0, "Error in querying dns A records for %s",
		   before_name);
	   return 0;
	 }

       before_a_records = nresults;

       for (j = 0; j < nresults; j++)
	 {
	   free(results[j]);
	 }

       if (!do_query(before_name, servers[i], C_IN, T_CNAME, results, 
		     &nresults, tkey))
	 {
	   com_err(whoami, 0, "Error in querying dns CNAME records for %s",
		   before_name);
	   return 0;
	 }

       before_cname_records = nresults;

       for (j = 0; j < nresults; j++)
	 {
	   free(results[j]);
	 }

       /*
	* If the record exists in DNS lets fetch txt records for it
	*/

       if (before_a_records != 0)
	 {
	   if (!do_query(before_name, servers[i], C_IN, T_TXT, results, 
			 &nresults, tkey))
	     {
	       com_err(whoami, 0, "Error in querying dns TXT records for %s",
		       before_name);
	       return 0;
	     }


	   before_txt_records = nresults;

	   for (j = 0; j < nresults; j++) 
	     {
	       if (!strncmp(results[j], "mitMoiraId:", 11))
		 {
		   strcpy(mitMoiraId, results[j]);
		 }
	     }

	   for (j = 0; j < nresults; j++)
	     {
	       free(results[j]);
	     }
	 }

       /*
	* Lets check if we found an mitMoiraId or not and if so if it 
	* matches what we have in our records. 
	*/

       if (before_a_records)
	 {
	   if (!*mitMoiraId)
	     {
	       com_err(whoami, 0, "%s does not have mitMoiraId attribute, "
		       "skipping", before_name);
	       return 0;
	     }

	   if (strcmp(mitMoiraId, before_mitMoiraId))
	     {
	       com_err(whoami, 0, "Mismatching mitMoiraId %s for %s does not "
		       "match moira value %s", before_name, before_mitMoiraId,
		       mitMoiraId);
	       return 0;
	     }
	 }

       /* Lets check if its a ptr record under our management */

       if(!strncmp(before_address, DEFAULT_PTR_ZONE, 3))
	 {
	   memset(mitMoiraId, 0, sizeof(mitMoiraId));

	   if (!do_query(before_ptr_name, servers[i], C_IN, T_PTR, results, 
			 &nresults, tkey))
	     {
	       com_err(whoami, 0, "Error in querying dns PTR records for %s",
		       before_ptr_name);
	       return 0;
	     }

	   before_ptr_records = nresults;

	   for (j = 0; j < nresults; j++)
	     {
	       free(results[j]);
	     }

	   if (!do_query(before_ptr_name, servers[i], C_IN, T_CNAME, results, 
			 &nresults, tkey))
	     {
	       com_err(whoami, 0, "Error in querying dns CNAME records for %s",
		       before_ptr_name);
	       return 0;
	     }

	   before_cname_records = nresults;

	   for (j = 0; j < nresults; j++)
	     {
	       free(results[j]);
	     }

	   /*
	    * If the record exists in DNS lets fetch txt records for it
	    */

	   if (before_ptr_records != 0)
	     {
	       if (!do_query(before_ptr_name, servers[i], C_IN, T_TXT, results, 
			     &nresults, tkey))
		 {
		   com_err(whoami, 0, 
			   "Error in querying dns TXT records for %s",
			   before_ptr_name);
		   return 0;
		 }

	       before_txt_records = nresults;

	       for (j = 0; j < nresults; j++) 
		 {
		   if (!strncmp(results[j], "mitMoiraId:", 11))
		     {
		       strcpy(mitMoiraId, results[j]);
		     }
		 }

	       for (j = 0; j < nresults; j++)
		 {
		   free(results[j]);
		 }
	     }

	   /*
	    * Lets check if we found an mitMoiraId or not and if so if it 
	    * matches what we have in our records. 
	    */

	   if (before_ptr_records)
	     {
	       if (!*mitMoiraId)
		 {
		   com_err(whoami, 0, "%s does not have mitMoiraId attribute, "
			   "skipping", before_ptr_name);
		   return 0;
		 }

	       if (strcmp(mitMoiraId, before_mitMoiraId))
		 {
		   com_err(whoami, 0, "Mismatching mitMoiraId %s for %s does "
			   "not match moira value %s", before_ptr_name,
			   before_mitMoiraId, mitMoiraId);
		   return 0;
		 }
	     }
	 }
     }

   for (i = 0; i < nservers; i++)
     {
       /*
	* We know its one of our hosts or it does not exist yet in the DNS
	* time to do the updates
	*/

       com_err(whoami, 0, "Updating server %s", servers[i]);

       if (!do_update(before_name, "", "", "", "", 0, 0, servers[i], C_IN, T_A,
		      tkey))
	 {
	   com_err(whoami, 0, "Error deleting A record for %s", before_name);
	   return 0;
	 }

       if(!strncmp(before_address, DEFAULT_PTR_ZONE, 3))
	 {
	   if (!do_update(before_ptr_name, "", "", "", "", 0, 0, servers[i], 
			  C_IN, T_PTR, tkey))
	     {
	       com_err(whoami, 0, "Error deleting PTR record for %s", 
		       before_ptr_name);
	       return 0;
	     }
	 }
     }

   for (i = 0; i < nservers; i++)
     {
       free(servers[i]);
     }

   return 1;
 }

 int alias_delete(char *before_alias, char *before_name,
		  int before_ttl, int before_mach_id, ns_tsig_key *tkey)

 {
   char *servers[MAX_SERVERS];
   int nservers, nresults, i, j;
   char *results[MAX_RESULTS];
   int before_a_records, after_a_records, before_cname_records, 
     after_cname_records, before_txt_records, after_txt_records;
   char mitMoiraId[BUFSIZ], before_mitMoiraId[BUFSIZ], after_mitMoiraId[BUFSIZ];
   char before_ptr_name[MAXDNAME], after_ptr_name[MAXDNAME];

   sprintf(before_mitMoiraId, "mitMoiraId:%d", before_mach_id);

   /* 
    * Lets fetch the DNS servers we are going to query
    */

   if (!get_servers(SERVERS_FILE, servers, &nservers))
     {
       com_err(whoami, 0, "Unable to retrieve dns server names");
       return 0;
     }

   /* 
    * First handle the A records
    * Fetch the txt records associated with the host record to determine 
    * whether or not its something under our management
    */

   for (i = 0; i < nservers; i++)
     {
       if(strlen(before_name) > 8 && 
	  !strncasecmp(before_name + strlen(before_name) - 8, ".MIT.EDU", 8))
	 {
	   com_err(whoami, 0, "Querying server %s", servers[i]);

	   memset(mitMoiraId, 0, sizeof(mitMoiraId));

	   if (!do_query(before_name, servers[i], C_IN, T_A, results, 
			 &nresults, tkey))
	     {
	       com_err(whoami, 0, "Error in querying dns A records for %s",
		       before_name);
	       return 0;
	     }

	   before_a_records = nresults;

	   for (j = 0; j < nresults; j++)
	     {
	       free(results[j]);
	     }

	   if (!do_query(before_name, servers[i], C_IN, T_CNAME, results, 
			 &nresults, tkey))
	     {
	       com_err(whoami, 0, "Error in querying dns CNAME records for %s",
		       before_name);
	       return 0;
	     }

	   before_cname_records = nresults;

	   for (j = 0; j < nresults; j++)
	     {
	       free(results[j]);
	     }

	   /*
	    * If the record exists in DNS lets fetch txt records for it
	    */

	   if (before_a_records != 0)
	     {
	       if (!do_query(before_name, servers[i], C_IN, T_TXT, results, 
			     &nresults, tkey))
		 {
		   com_err(whoami, 0, 
			   "Error in querying dns TXT records for %s",
			   before_name);
		   return 0;
		 }

	       before_txt_records = nresults;

	       for (j = 0; j < nresults; j++) 
		 {
		   if (!strncmp(results[j], "mitMoiraId:", 11))
		     {
		       strcpy(mitMoiraId, results[j]);
		     }
		 }

	       for (j = 0; j < nresults; j++)
		 {
		   free(results[j]);
		 }
	     }
	   else 
	     {
	       com_err(whoami, 0, 
		       "A records does not exist for %s", before_name);
	       return 0;
	     }

	   /*
	    * Lets check if we found an mitMoiraId or not and if so if it 
	    * matches what we have in our records. 
	    */

	   if (before_a_records)
	     {
	       if (!*mitMoiraId)
		 {
		   com_err(whoami, 0, "%s does not have mitMoiraId attribute, "
			   "skipping", before_name);
		   return 0;
		 }

	       if (strcmp(mitMoiraId, before_mitMoiraId))
		 {
		   com_err(whoami, 0, "Mismatching mitMoiraId %s for %s does "
			   "not match moira value %s", before_name, 
			   before_mitMoiraId, mitMoiraId);
		   return 0;
		 }
	     }

	   memset(mitMoiraId, 0, sizeof(mitMoiraId));

	 }
     }

   for (i = 0; i < nservers; i++)
     {
       /*
	* We know its one of our hosts or it does not exist yet in the DNS
	*/

       com_err(whoami, 0, "Updating server %s", servers[i]);

       if (!do_update(before_alias, "", "", "", "", 0, 0, servers[i], C_IN, 
		      T_CNAME, tkey))
	 {
	   com_err(whoami, 0, "Error deleting CNAME record for %s", 
		   before_alias);
	   return 0;
	 }
     }

   for (i = 0; i < nservers; i++)
     {
       free(servers[i]);
     }

   return 1;
 }

 int initialize_hmac_keys(void)
 {
   char *hmac_key;
   u_char *stream;
   int len;

   if (!get_hmac_key(HMAC_INTERNAL_KEY_FILE, &hmac_key))
     {
       com_err(whoami, 0, "Unable to retrieve internal HMAC key");
       return 0;
     }

   stream = malloc(sizeof(char *) * strlen(hmac_key) + 1);
   if (!stream)
     {
       com_err(whoami, 0, "Unable to allocate memory for internal hmac_key stream");
       return 0;
     }

   len = b64_pton(hmac_key, stream, strlen(hmac_key));

   internal_tkey = malloc(sizeof *internal_tkey);
   if (!internal_tkey)
     {
       com_err(whoami, 0, "Unable to allocate memory for internal_tkey");
       return 0;
     }

   memset(internal_tkey, 0, sizeof(*internal_tkey));

   internal_tkey->data = malloc(len * sizeof(char));
   if (!internal_tkey->data)
     {
       com_err(whoami, 0, "Unable to allocate memory for internal_tkey->data");
       return 0;
     }

   strcpy(internal_tkey->name, INTERNAL_KEY_NAME);
   strcpy(internal_tkey->alg, NS_TSIG_ALG_HMAC_MD5);
   memcpy(internal_tkey->data, stream, len);
   internal_tkey->len = len;

   if (!get_hmac_key(HMAC_EXTERNAL_KEY_FILE, &hmac_key))
     {
       com_err(whoami, 0, "Unable to retrieve external HMAC key");
       return 0;
     }

   stream = malloc(sizeof(char *) * strlen(hmac_key) + 1);
   if (!stream)
     {
       com_err(whoami, 0, "Unable to allocate memory for external hmac_key stream");
       return 0;
     }

   len = b64_pton(hmac_key, stream, strlen(hmac_key));

   external_tkey = malloc(sizeof *external_tkey);
   if (!external_tkey)
     {
       com_err(whoami, 0, "Unable to allocate memory for external_tkey");
       return 0;
     }

   memset(external_tkey, 0, sizeof(*external_tkey));

   external_tkey->data = malloc(len * sizeof(char));
   if (!external_tkey->data)
     {
       com_err(whoami, 0, "Unable to allocate memory for external_tkey->data");
       return 0;
     }

   strcpy(external_tkey->name, EXTERNAL_KEY_NAME);
   strcpy(external_tkey->alg, NS_TSIG_ALG_HMAC_MD5);
   memcpy(external_tkey->data, stream, len);
   external_tkey->len = len;

   return 1;
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

 int save_hostaliases(int argc, char **argv, void *sq)
 {
   struct save_queue *q = sq;
   char *hostalias;

   hostalias = strdup(argv[0]);
   sq_save_data(q, hostalias);

   return MR_CONT;
 }

 int delete_hostaliases(char *before_name, char *after_name, int before_mach_id)
 {
   int rc;
   char *args[3];
   char *hostalias;
   struct save_queue *host_aliases;

   args[0] = "*";
   args[1] = after_name;

   host_aliases = sq_create();

   if (rc = moira_connect())
     {
       critical_alert(whoami, "incremental", "Error contacting Moira server: %s",
		      error_message(rc));
       return 0;
     }

   if (rc = mr_query("get_hostalias", 2, args, save_hostaliases, host_aliases))
     {
       com_err(whoami, 0, "Unable to retrieve aliases for %s", after_name);
       moira_disconnect();
       return 0;
     }

   moira_disconnect();

   while (sq_get_data(host_aliases, &hostalias))
     {
       com_err(whoami, 0, "Deleting host alias %s for host %s",
	       hostalias, after_name);

       if (!alias_delete(hostalias, before_name, DEFAULT_TTL,
			 before_mach_id, internal_tkey))
	 {
	   critical_alert(whoami, "incremental",
			  "Unable to delete internal view host alias for %s",
			  hostalias);
	   return 0;
	 }

       if (!alias_delete(hostalias, before_name, DEFAULT_TTL,
			 before_mach_id, external_tkey))
	 {
	   critical_alert(whoami, "incremental",
			  "Unable to delete external view host alias for %s",
			  hostalias);
	   return 0;
	 }
     }

   sq_destroy(host_aliases);
   return 1;
 }

 int add_hostaliases(char *before_name, char *after_name, int after_mach_id)
 {
  int rc;
  char *args[3];
  char *hostalias;
  struct save_queue *host_aliases;

  args[0] = "*";
  args[1] = after_name;

  host_aliases = sq_create();

  if (rc = moira_connect())
    {
      critical_alert(whoami, "incremental", "Error contacting Moira server: %s",
		     error_message(rc));
      return 0;
    }

  if (rc = mr_query("get_hostalias", 2, args, save_hostaliases, host_aliases))
    {
      com_err(whoami, 0, "Unable to retrieve aliases for %s", after_name);
      moira_disconnect();
      return 0;
    }

  moira_disconnect();

  while (sq_get_data(host_aliases, &hostalias))
    {
      com_err(whoami, 0, "Adding host alias %s for host %s",
	      hostalias, after_name);

      if (!alias_add(hostalias, after_name, DEFAULT_TTL,
		     after_mach_id, internal_tkey))
	{
	  critical_alert(whoami, "incremental",
			 "Unable to add internal view host alias for %s",
			 hostalias);
	  return 0;
	}

      if (!alias_add(hostalias, after_name, DEFAULT_TTL,
		     after_mach_id, external_tkey))
	{
	  critical_alert(whoami, "incremental",
			 "Unable to add external view host alias for %s",
			 hostalias);
	  return 0;
	}
    }

  sq_destroy(host_aliases);
  return 1;
}
