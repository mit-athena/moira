/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/incremental/winad/winad.c,v 1.1 2000-06-06 04:38:34 zacheiss Exp $
 * 
 * Do incremental updates of a Windows Active Directory
 *
 * test parameters for reactivating a user account
 *  users 9 9 testaccount 1 sh account test A 3 2 3 testaccount 1 csh new login b 1 2 3
 */

#include <mit-copyright.h>
#ifdef _WIN32
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <lmaccess.h>
#endif
#include <ldap.h>
#include <stdio.h>
#include <moira.h>
#include <moira_site.h>
#include <krb5.h>
#include <krb.h>
#include <gsssasl.h>
#include <gssldap.h>

#ifndef _WIN32
#include <sys/utsname.h>

#define UF_SCRIPT               0x0001
#define UF_ACCOUNTDISABLE       0x0002
#define UF_HOMEDIR_REQUIRED     0x0008
#define UF_LOCKOUT              0x0010
#define UF_PASSWD_NOTREQD       0x0020
#define UF_PASSWD_CANT_CHANGE   0x0040

#define UF_TEMP_DUPLICATE_ACCOUNT       0x0100
#define UF_NORMAL_ACCOUNT               0x0200
#define UF_INTERDOMAIN_TRUST_ACCOUNT    0x0800
#define UF_WORKSTATION_TRUST_ACCOUNT    0x1000
#define UF_SERVER_TRUST_ACCOUNT         0x2000

#ifndef BYTE
#define BYTE unsigned char
#endif
typedef unsigned int DWORD;
typedef unsigned long ULONG;

typedef struct _GUID
{
  unsigned long Data1;
  unsigned short Data2;
  unsigned short Data3;
  unsigned char Data4[8];
} GUID;

typedef struct _SID_IDENTIFIER_AUTHORITY { 
  BYTE Value[6]; 
} SID_IDENTIFIER_AUTHORITY, *PSID_IDENTIFIER_AUTHORITY; 

typedef struct _SID {
  BYTE  Revision;
  BYTE  SubAuthorityCount;
  SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
  DWORD SubAuthority[512];
} SID;
#endif/*!WIN32*/

#define SUBSTITUTE  1
#define REPLACE     2

#define MEMBER_ADD          1
#define MEMBER_REMOVE       2
#define MEMBER_CHANGE_NAME  3
#define MEMBER_ACTIVATE     4
#define MEMBER_DEACTIVATE   5

#define GROUP_CREATE            1
#define GROUP_DELETE            2
#define GROUP_MOVE_MEMBERS      3
#define GROUP_UPDATE_MEMBERS    4

typedef struct lk_entry {
  int     op;
  int     length;
  int     ber_value;
  char    *dn;
  char    *attribute;
  char    *value;
  char    *member;
  char    *type;
  char    *list;
  struct  lk_entry *next;
} LK_ENTRY;

#define LDAP_BERVAL struct berval

LK_ENTRY *member_base = NULL;
char group_ou[] = "OU=groups,OU=athena";
char *whoami;
static int mr_connections = 0;

int add_list_members(int ac, char **av, void *group);
int add_user_lists(int ac, char **av, void *user);
int member_update(LDAP *ldap_handle, char *dn_path, LDAPMessage *ldap_entry, 
		  char *login_name, char *new_login_name, char *ldap_hostname,
		  int operation, LK_ENTRY *group_base);
int group_update(LDAP *ldap_handle, char *ldap_hostname, 
		 char *before_group_name, char *after_group_name,
		 int operation, LK_ENTRY *user_list);
int check_user(int ac, char **av, void *ustate);
int construct_newvalues(LK_ENTRY *linklist_base, int modvalue_count, 
			int newValue_count, char *oldValue, char *newValue,
			LDAPMod ***newValueArray, char ****modvalues,
			int type);
int convert_domain_to_dn(char *domain, char **bind_path);
void delete_user(LDAP *ldap_handle, LDAPMessage *ldap_entry);
void do_list(LDAP *ldap_handle, char *dn_path, char *ldap_hostname, 
	     char **before, int beforec, char **after, int afterc);
void do_user(LDAP *ldap_handle, LDAPMessage *ldap_entry, char *ldap_hostname, 
	     char *dn_path, char **before, int beforec, char **after, 
	     int afterc);
void do_member(LDAP *ldap_handle, char *dn_path, char *ldap_hostname,
	       char **before, int beforec, char **after, int afterc);
void edit_group(int op, char *group, char *type, char *member,
		LDAP *ldap_handle, char *dn_path, char *ldap_hostname);
int linklist_create_entry(char *attribute, char *value, 
			  LK_ENTRY **linklist_entry);
int linklist_build(LDAP *ldap_handle, char *dn_path, char *search_exp, 
		   char **attr_array, LK_ENTRY **linklist_base, 
		   int *linklist_count);
void linklist_free(LK_ENTRY *linklist_base);
void free_values(int newvalue_count, LDAPMod **newvalue_array, 
		 char ***modvalues);
void get_distinguished_name(LDAP *ldap_handle, LDAPMessage *ldap_entry, 
			    char *distinguished_name);
int moira_disconnect(void);
int moira_connect(void);
void print_to_screen(const char *fmt, ...);
int retrieve_attributes(LDAP *ldap_handle, LDAPMessage *ldap_entry, 
			char *distinguished_name, LK_ENTRY **linklist_current);
int retrieve_entries(LDAP *ldap_handle, LDAPMessage *ldap_entry, 
		     LK_ENTRY **linklist_base, int *linklist_count);
int retrieve_values(LDAP *ldap_handle, LDAPMessage *ldap_entry, 
		    char *Attribute, char *distinguished_name, 
		    LK_ENTRY **linklist_current);

int main(int argc, char **argv)
{
  unsigned long rc;
  int beforec, afterc, Max_wait_time = 500, Max_size_limit = LDAP_NO_LIMIT;
  char ldap_hostname[256], search_exp[1024];
  char *dn_path, *table, **before, **after;
  ULONG version = LDAP_VERSION3;
  LDAP *ldap_handle;
  LDAPMessage *ldap_entry;
  FILE            *fptr;
  
  whoami = ((whoami = (char *)strrchr(argv[0], '/')) ? whoami+1 : argv[0]);
  /*
   *   if (argc < 4)
   *       exit(1);
   *
   *   beforec = atoi(argv[2]);
   *   afterc = atoi(argv[3]);
   *
   *   if (argc < (4 + beforec + afterc))
   *       exit(1);
   *
   *   table = argv[1];
   *   before = &argv[4];
   *  after = &argv[4 + beforec];
   */
  memset(ldap_hostname, '\0', sizeof(ldap_hostname));
  if ((fptr = fopen("winad.cfg", "r")) != NULL)
    {
      fread(ldap_hostname, sizeof(char), sizeof(ldap_hostname), fptr);
      fclose(fptr);
    }
  if (strlen(ldap_hostname) == 0)
    strcpy(ldap_hostname, "windows.mit.edu");
  
  initialize_sms_error_table();
  initialize_krb_error_table();
  
  memset(search_exp, '\0', sizeof(search_exp));
  ldap_entry = NULL;
  dn_path = NULL;
  convert_domain_to_dn(ldap_hostname, &dn_path);
  if (dn_path == NULL)
    exit(1);
  
  ldap_handle = ldap_open(ldap_hostname, LDAP_PORT);
  if (ldap_handle == NULL)
    exit(1);
  rc = ldap_set_option(ldap_handle, LDAP_OPT_PROTOCOL_VERSION, &version);
  rc = ldap_set_option(ldap_handle, LDAP_OPT_TIMELIMIT, 
		       (void *)&Max_wait_time);
  rc = ldap_set_option(ldap_handle, LDAP_OPT_SIZELIMIT, 
		       (void *)&Max_size_limit);
  rc = ldap_set_option(ldap_handle, LDAP_OPT_REFERRALS, LDAP_OPT_OFF);
  rc = ldap_adgssapi_bind(ldap_handle, dn_path, GSSSASL_NO_SECURITY_LAYER);
  if (rc != LDAP_SUCCESS) 
    exit(1);
  
  /* used for testing
   * do_list(ldap_handle, dn_path, ldap_hostname, before, beforec, after, 
   *         afterc);
   */
  
  rc = group_update(ldap_handle, ldap_hostname, "testaccountF", NULL, 
		    GROUP_CREATE, NULL);
  printf("first rc = %d\n", rc);
  if (rc != 0)
    exit(1);
  rc = group_update(ldap_handle, ldap_hostname, "pismere-team", 
		    "testaccountF", GROUP_MOVE_MEMBERS, NULL);
  printf("second rc = %d\n", rc);
  if (rc != 0)
    exit(1);
  rc = group_update(ldap_handle, ldap_hostname, "testaccountF", NULL, 
		    GROUP_DELETE, NULL);
  printf("third rc = %d\n", rc);
  exit(1);
  
  if (!strcmp(table, "users"))
    {
      sprintf(search_exp, "(sAMAccountName=%s)", before[U_NAME]);
      if ((rc = ldap_search_s(ldap_handle, dn_path, LDAP_SCOPE_SUBTREE, 
			      search_exp, NULL, 0, &ldap_entry)) 
	  == LDAP_SUCCESS)
	do_user(ldap_handle, ldap_entry, ldap_hostname, dn_path, before, 
		beforec, after, afterc);
      rc = ldap_msgfree(ldap_entry);
    }
  else if (!strcmp(table, "list"))
    do_list(ldap_handle, dn_path, ldap_hostname, before, beforec, after, 
	    afterc);
  else if (!strcmp(table, "imembers"))
    do_member(ldap_handle, dn_path, ldap_hostname, before, beforec, after, 
	      afterc);
  /*
   *   else if (!strcmp(table, "filesys"))
   *       do_filesys(before, beforec, after, afterc);
   *   else if (!strcmp(table, "quota"))
   *       do_quota(before, beforec, after, afterc);
   */
  rc = ldap_unbind_s(ldap_handle);
  free(dn_path);
  
  exit(0);
}

void do_list(LDAP *ldap_handle, char *dn_path, char *ldap_hostname, 
	     char  **before, int beforec, char **after, int afterc)
{
  int agid = 0, bgid = 0, ahide, bhide;
  long rc, id;
  char *av[2];
  
  if (beforec > L_GID && atoi(before[L_ACTIVE]) && atoi(before[L_GROUP]))
    {
      bgid = atoi(before[L_GID]);
      bhide = atoi(before[L_HIDDEN]);
    }
  if (afterc > L_GID && atoi(after[L_ACTIVE]) && atoi(after[L_GROUP]))
    {
      agid = atoi(after[L_GID]);
      ahide = atoi(after[L_HIDDEN]);
    }
  
  if (agid == 0 && bgid == 0)
    return;
  
  if (agid && bgid)
    {
      if (strcmp(after[L_NAME], before[L_NAME]))
	{
	  com_err(whoami, 0, "Changing group %s to %s",
		  before[L_NAME], after[L_NAME]);
	  
	  if (!(rc = group_update(ldap_handle, ldap_hostname, 
				  after[L_NAME], NULL, GROUP_CREATE, NULL)))
	    {
	      if (!(rc = group_update(ldap_handle, ldap_hostname, 
				      before[L_NAME], after[L_NAME], 
				      GROUP_MOVE_MEMBERS, NULL)))
		{
		  rc = group_update(ldap_handle, ldap_hostname, 
				    before[L_NAME], NULL, GROUP_DELETE, 
				    NULL);
		}
	    }
	  if (rc)
	    {
	      critical_alert("incremental", "Couldn't change group %s "
			     "to %s", before[L_NAME], after[L_NAME]);
	    }
	}
      return;
    }
  if (bgid)
    {
      com_err(whoami, 0, "Deleting group %s", before[L_NAME]);
      rc = group_update(ldap_handle, ldap_hostname, before[L_NAME], NULL, 
			GROUP_DELETE, NULL);
      if (rc)
	{
	  critical_alert("incremental",
			 "Couldn't delete group %s (id %d)",
			 before[L_NAME], -bgid);
	}
      return;
    }
  if (agid)
    {
      id = -agid;
      com_err(whoami, 0, "Creating group %s", after[L_NAME]);
      rc = group_update(ldap_handle, ldap_hostname, after[L_NAME], NULL, 
			GROUP_CREATE, NULL);
      if (rc)
	{
	  critical_alert("incremental", "Couldn't create group %s (id %d)",
			 after[L_NAME], id);
	  return;
	}
      
      if (beforec < L_ACTIVE)
	return;
      
      rc = moira_connect();
      if (rc)
	{
	  critical_alert("incremental",
			 "Error contacting Moira server to resolve %s: %s",
			 after[L_NAME], error_message(rc));
	  return;
	}
      av[0] = after[L_NAME];
      rc = mr_query("get_end_members_of_list", 1, av,
		    add_list_members, after[L_NAME]);
      if (rc)
	{
	  critical_alert("incremental",
			 "Couldn't retrieve full membership of list %s: %s",
			 after[L_NAME], error_message(rc));
	}
      rc = group_update(ldap_handle, ldap_hostname, after[L_NAME], NULL, 
			GROUP_UPDATE_MEMBERS, member_base);
      linklist_free(member_base);
      moira_disconnect();
      return;
    }
}

#define LM_EXTRA_ACTIVE	  (LM_END)
#define LM_EXTRA_PUBLIC   (LM_END+1)
#define LM_EXTRA_HIDDEN   (LM_END+2)
#define LM_EXTRA_MAILLIST (LM_END+3)
#define LM_EXTRA_GROUP    (LM_END+4)
#define LM_EXTRA_GID      (LM_END+5)
#define LM_EXTRA_END      (LM_END+6)

void do_member(LDAP *ldap_handle, char *dn_path, char *ldap_hostname,
	       char **before, int beforec, char **after, int afterc)
{
  if (afterc)
    {
      if (afterc < LM_EXTRA_END)
	return;
      else
	{
	  if (!atoi(after[LM_EXTRA_ACTIVE]) || !atoi(after[LM_EXTRA_GROUP]))
	    return;
	}
      edit_group(1, after[LM_LIST], after[LM_TYPE], after[LM_MEMBER], 
		 ldap_handle, dn_path, ldap_hostname);
    }
  else if (beforec)
    {
      if (beforec < LM_EXTRA_END)
	return;
      else
	{
	  if (!atoi(before[LM_EXTRA_ACTIVE]) || !atoi(before[LM_EXTRA_GROUP]))
	    return;
	}
      edit_group(0, before[LM_LIST], before[LM_TYPE], before[LM_MEMBER],
		 ldap_handle, dn_path, ldap_hostname);
    }
}

void do_user(LDAP *ldap_handle, LDAPMessage *ldap_entry, char *ldap_hostname, 
	     char *dn_path, char **before, int beforec, char **after, 
	     int afterc)
{
  int astate = 0, bstate = 0, auid = 0, buid = 0, rc;
  char *av[2];
  
  if (afterc > U_STATE)
    astate = atoi(after[U_STATE]);
  if (beforec > U_STATE)
    bstate = atoi(before[U_STATE]);
  if (afterc > U_UID)
    auid = atoi(after[U_UID]);
  if (beforec > U_UID)
    buid = atoi(before[U_UID]);
  
  if (astate == 2)
    astate = 1;
  if (bstate == 2)
    bstate = 1;
  
  if (astate != 1 && bstate != 1)		/* inactive user */
    return;
  
  if ((astate == bstate) && (auid == buid) && 
      !strcmp(before[U_NAME], after[U_NAME]))
    return;
  
  if (astate == bstate)
    {
      com_err(whoami, 0, "Changing user %s to %s", before[U_NAME], 
	      after[U_NAME]);
      rc = member_update(ldap_handle, dn_path, ldap_entry, before[U_NAME], 
			 after[U_NAME],  ldap_hostname, MEMBER_CHANGE_NAME, 
			 NULL);
      if (rc)
	{
	  critical_alert("incremental",
			 "Couldn't change user %s (id %d) to %s (id %d)",
			 before[U_NAME], buid, after[U_NAME], auid);
	}
      return;
    }
  if (bstate == 1)
    {
      com_err(whoami, 0, "Deleting user %s", before[U_NAME]);
      rc = member_update(ldap_handle, dn_path, ldap_entry, before[U_NAME], 
			 NULL, ldap_hostname, MEMBER_DEACTIVATE, NULL);
      if (rc)
	{
	  critical_alert("incremental", "Couldn't deactivate user %s (id %d)",
			 before[U_NAME], buid);
	}
      return;
    }
  if (astate == 1)
    {
      com_err(whoami, 0, "%s user %s",
	      ((bstate != 0) ? "Reactivating" : "Creating"),
	      after[U_NAME]);
      
      rc = member_update(ldap_handle, dn_path, ldap_entry, after[U_NAME], 
			 NULL, ldap_hostname, MEMBER_ACTIVATE, NULL);
      if (rc)
	{
	  critical_alert("incremental", "Couldn't activate user %s (id %d)",
			 after[U_NAME], auid);
	  return;
	}
      if (bstate != 0)
	{
	  /* Reactivating a user; get his group list */
	  rc = moira_connect();
	  if (rc)
	    {
	      critical_alert("incremental", "Error contacting Moira server "
			     "to retrieve grouplist of user %s: %s",
			     after[U_NAME], error_message(rc));
	      return;
	    }
	  av[0] = "ruser";
	  av[1] = after[U_NAME];
	  member_base = NULL;
	  rc = mr_query("get_lists_of_member", 2, av, add_user_lists, 
			after[U_NAME]);
	  if (rc && rc != MR_NO_MATCH)
	    {
	      critical_alert("incremental",
			     "Couldn't retrieve membership of user %s: %s",
			     after[U_NAME], error_message(rc));
	    }
	  else
	    {
	      rc = member_update(ldap_handle, dn_path, ldap_entry, 
				 after[U_NAME], NULL,  ldap_hostname, 
				 MEMBER_ADD, member_base);
	      linklist_free(member_base);
	    }
	  moira_disconnect();
	}
      return;
    }
}

int group_update(LDAP *ldap_handle, char *ldap_hostname, char *group_name,
		 char *after_group_name, int operation, LK_ENTRY *user_list)
{
  char distinguished_name[1024], filter_exp[1024], temp[256];
  char ***modvalues, *attr_array[3], *dn_path = NULL;
  int newvalue_count, group_count, sPtr_count;
  LDAPMod **newvalue_array;
  LK_ENTRY *group_base;
  LK_ENTRY **sPtr;
  LK_ENTRY *pPtr;
  ULONG rc = 0;
  
  convert_domain_to_dn(ldap_hostname, &dn_path);
  
  newvalue_array = NULL;
  newvalue_count = 0;
  modvalues = NULL;
  group_base = NULL;
  group_count = 0;
  newvalue_count = 0;
  newvalue_array = NULL;
  modvalues = NULL;
  if ((newvalue_array = calloc(1, (5+1) * sizeof(LDAPMod *))) == NULL)
    {
      rc = 1;
      goto cleanup;
    }
  newvalue_array[0] = NULL;
  newvalue_array[1] = NULL;
  newvalue_array[2] = NULL;
  newvalue_array[3] = NULL;
  newvalue_array[4] = NULL;
  if ((modvalues = calloc(1, (5+1) * sizeof(char **))) == NULL)
    {
      rc = 1;
      goto cleanup;
    }
  modvalues[0] = NULL;
  modvalues[1] = NULL;
  modvalues[2] = NULL;
  modvalues[3] = NULL;
  modvalues[4] = NULL;
  
  if (operation == GROUP_DELETE)
    {
      sprintf(filter_exp, "(sAMAccountName=%s)", group_name);
      attr_array[0] = "distinguishedName";
      attr_array[1] = NULL;
      if ((rc = linklist_build(ldap_handle, dn_path, filter_exp, attr_array, 
			       &group_base, &group_count)) != 0)
	goto cleanup;
      if (group_count == 1)
	rc = ldap_delete_s(ldap_handle, group_base->value);
      if (rc != LDAP_SUCCESS)
	{
	  critical_alert("incremental",
			 "Couldn't delete group %s: %s",
			 group_name, ldap_err2string(rc));
	  goto cleanup;
	}
    }
  else if (operation == GROUP_CREATE)
    {
      linklist_create_entry("cn", group_name, &group_base);
      group_count = 1;
      if ((rc = construct_newvalues(group_base, group_count, newvalue_count,
				    NULL, NULL, &newvalue_array, &modvalues, 
				    REPLACE)) == 1)
	goto cleanup;
      newvalue_array[newvalue_count]->mod_op = LDAP_MOD_ADD;
      newvalue_array[newvalue_count]->mod_type = "cn";
      newvalue_array[newvalue_count]->mod_values = modvalues[newvalue_count];
      linklist_free(group_base);
      group_base = NULL;
      
      ++newvalue_count;
      linklist_create_entry("name", group_name, &group_base);
      group_count = 1;
      if ((rc = construct_newvalues(group_base, group_count, newvalue_count,
				    NULL, NULL, &newvalue_array, &modvalues, 
				    REPLACE)) == 1)
	goto cleanup;
      newvalue_array[newvalue_count]->mod_op = LDAP_MOD_ADD;
      newvalue_array[newvalue_count]->mod_type = "name";
      newvalue_array[newvalue_count]->mod_values = modvalues[newvalue_count];
      linklist_free(group_base);
      group_base = NULL;
      
      ++newvalue_count;
      linklist_create_entry("objectClass","top", &group_base);
      linklist_create_entry("objectClass", "group", &group_base->next);
      group_count = 2;
      if ((rc = construct_newvalues(group_base, group_count, newvalue_count,
				    NULL, NULL, &newvalue_array, &modvalues, 
				    REPLACE)) == 1)
	goto cleanup;
      newvalue_array[newvalue_count]->mod_op = LDAP_MOD_ADD;
      newvalue_array[newvalue_count]->mod_type = "objectClass";
      newvalue_array[newvalue_count]->mod_values = modvalues[newvalue_count];
      linklist_free(group_base);
      group_base = NULL;
      
      ++newvalue_count;
      linklist_create_entry("sAMAccountName", group_name, &group_base);
      group_count = 1;
      if ((rc = construct_newvalues(group_base, group_count, newvalue_count,
				    NULL, NULL, &newvalue_array, &modvalues, 
				    REPLACE)) == 1)
	goto cleanup;
      newvalue_array[newvalue_count]->mod_op = LDAP_MOD_ADD;
      newvalue_array[newvalue_count]->mod_type = "sAMAccountName";
      newvalue_array[newvalue_count]->mod_values = modvalues[newvalue_count];
      linklist_free(group_base);
      group_base = NULL;
      
      sprintf(temp,"CN=%s,%s,%s", group_name, group_ou, dn_path);
      rc = ldap_add_s(ldap_handle, temp, newvalue_array);
      if (rc != LDAP_SUCCESS)
	{
	  critical_alert("incremental",
			 "Couldn't create group %s: %s",
			 group_name, ldap_err2string(rc));
	  goto cleanup;
	}
    }
  else if (operation == GROUP_UPDATE_MEMBERS)
    {
      sprintf(filter_exp, "(sAMAccountName=%s)", group_name);
      attr_array[0] = "distinguishedName";
      attr_array[1] = NULL;
      if ((rc = linklist_build(ldap_handle, dn_path, filter_exp, attr_array, 
			       &group_base, &group_count)) != 0)
	goto cleanup;
      if (group_count != 1)
	{
	  rc = 1;
	  goto cleanup;
	}
      strcpy(distinguished_name, group_base->value);
      linklist_free(group_base);
      
      pPtr = member_base;
      group_base = NULL;
      group_count = 0;
      sPtr = &group_base;
      while (pPtr)
	{
	  sprintf(filter_exp, "(sAMAccountName=%s)", pPtr->member);
	  attr_array[0] = "distinguishedName";
	  attr_array[1] = NULL;
	  if ((*sPtr) != NULL)
	    sPtr = &((*sPtr)->next);
	  sPtr_count = 0;
	  if ((rc = linklist_build(ldap_handle, dn_path, filter_exp, 
				   attr_array, sPtr, &sPtr_count)) != 0)
	    goto cleanup;
	  if (sPtr_count != 0)
	    group_count += sPtr_count;
	  pPtr = pPtr->next;
	}
      if (group_count != 0)
	{
	  if ((rc = construct_newvalues(group_base, group_count, 
					newvalue_count, NULL, NULL,
					&newvalue_array, &modvalues, 
					REPLACE)) == 1)
	    goto cleanup;
	  linklist_free(group_base);
	  newvalue_array[newvalue_count]->mod_type = "member";
	  newvalue_array[newvalue_count]->mod_values = 
	    modvalues[newvalue_count];
	  newvalue_array[newvalue_count]->mod_op = LDAP_MOD_ADD;
	  rc = ldap_modify_s(ldap_handle, distinguished_name, newvalue_array);
	  group_count = 0;
	  group_base = NULL;
	  if (rc != LDAP_SUCCESS)
	    {
	      critical_alert("incremental",
			     "Couldn't add users to group %s: %s",
			     group_name, ldap_err2string(rc));
	      goto cleanup;
            }
	}
    }
  else if (operation == GROUP_MOVE_MEMBERS)
    {
      sprintf(filter_exp, "(sAMAccountName=%s)", after_group_name);
      attr_array[0] = "distinguishedName";
      attr_array[1] = NULL;
      if ((rc = linklist_build(ldap_handle, dn_path, filter_exp, attr_array, 
			       &group_base, &group_count)) != 0)
	goto cleanup;
      if (group_count != 1)
	{
	  rc = 1;
	  goto cleanup;
	}
      strcpy(distinguished_name, group_base->value);
      linklist_free(group_base);
      
      group_base = NULL;
      group_count = 0;
      sprintf(filter_exp, "(sAMAccountName=%s)", group_name);
      attr_array[0] = "member";
      attr_array[1] = NULL;
      if ((rc = linklist_build(ldap_handle, dn_path, filter_exp, attr_array, 
			       &group_base, &group_count)) != 0)
	goto cleanup;
      
      if (group_count != 0)
	{
	  if ((rc = construct_newvalues(group_base, group_count, 
					newvalue_count, NULL, NULL,
					&newvalue_array, &modvalues, 
					REPLACE)) == 1)
	    goto cleanup;
	  linklist_free(group_base);
	  newvalue_array[newvalue_count]->mod_type = "member";
	  newvalue_array[newvalue_count]->mod_values = 
	    modvalues[newvalue_count];
	  newvalue_array[newvalue_count]->mod_op = LDAP_MOD_ADD;
	  rc = ldap_modify_s(ldap_handle, distinguished_name, newvalue_array);
	  group_count = 0;
	  group_base = NULL;
	  if (rc != LDAP_SUCCESS)
	    {
	      critical_alert("incremental",
			     "Couldn't add users to group %s: %s",
			     group_name, ldap_err2string(rc));
	      goto cleanup;
	    }
	}
    }
  
 cleanup:
  free_values(newvalue_count, newvalue_array, modvalues);
  linklist_free(group_base);
  if (dn_path != NULL)
    free(dn_path);
  if (rc == 1)
    critical_alert("incremental",
		   "Couldn't process group %s: %s",
		   group_name, "Cannot calloc - out of memory");
  return(rc);
}

int member_update(LDAP *ldap_handle, char *cn_path, LDAPMessage *ldap_entry, 
		  char *login_name, char *new_login_name, char *ldap_hostname,
		  int operation, LK_ENTRY *group_list)
{
  char distinguished_name[1024], filter_exp[1024], temp[64];
  char ***modvalues;  /* each *mod has a char **modvalue */
  char *attr_array[3], *dn_path = NULL;
  int newvalue_count, memberOf_count, group_count, processGroup_count;
  LDAPMod     **newvalue_array;
  LK_ENTRY    *processGroup;
  LK_ENTRY    *processGroup_base;
  LK_ENTRY    *memberOf_base;
  LK_ENTRY    *group_base;
  LK_ENTRY    *gPtr;
  LK_ENTRY    *sPtr;
  LK_ENTRY    *pPtr;
  ULONG       rc;
  ULONG       ulongValue;
  
  memset(distinguished_name, '\0', sizeof(distinguished_name));
  get_distinguished_name(ldap_handle, ldap_entry, distinguished_name);
  if (strlen(distinguished_name) == 0)
    return(1);
  
  convert_domain_to_dn(ldap_hostname, &dn_path);
  
  memberOf_base = NULL;
  processGroup_base = NULL;
  processGroup = NULL;
  newvalue_array = NULL;
  newvalue_count = 0;
  modvalues = NULL;
  group_count = 0;
  group_base = NULL;
  
  if ((newvalue_array = calloc(1, (3+1) * sizeof(LDAPMod *))) == NULL)
    {
      rc = 1;
      goto cleanup;
    }
  newvalue_array[0] = NULL;
  newvalue_array[1] = NULL;
  newvalue_array[2] = NULL;
  newvalue_array[3] = NULL;
  if ((modvalues = calloc(1, (3+1) * sizeof(char **))) == NULL)
    {
      rc = 1;
      goto cleanup;
    }
  modvalues[0] = NULL;
  modvalues[1] = NULL;
  modvalues[2] = NULL;
  modvalues[3] = NULL;
  
  if (operation == MEMBER_CHANGE_NAME)
    {
      sprintf(filter_exp, "(sAMAccountName=%s)", login_name);
      attr_array[0] = "sAMAccountName";
      attr_array[1] = NULL;
      if ((rc = linklist_build(ldap_handle, dn_path, filter_exp, attr_array, 
			       &group_base, &group_count)) != 0)
	goto cleanup;
      
      if (group_count != 0)
	{
	  if ((rc = construct_newvalues(group_base, group_count, 
					newvalue_count, login_name, 
					new_login_name, &newvalue_array, 
					&modvalues, REPLACE)) == 1)
	    goto cleanup;
	  newvalue_array[newvalue_count]->mod_op = LDAP_MOD_REPLACE ;
	  newvalue_array[newvalue_count]->mod_type = "sAMAccountName";
	  newvalue_array[newvalue_count]->mod_values = 
	    modvalues[newvalue_count];
	  linklist_free(group_base);
	  group_count = 0;
	  group_base = NULL;
	  ++newvalue_count;
	}
      
      sprintf(filter_exp, "(sAMAccountName=%s)", login_name);
      attr_array[0] = "UserPrincipalName";
      attr_array[1] = NULL;
      if ((rc = linklist_build(ldap_handle, dn_path, filter_exp, attr_array, 
			       &group_base, &group_count)) != 0)
	goto cleanup;
      if (group_count != 0)
	{
	  if ((rc = construct_newvalues(group_base, group_count, 
					newvalue_count, login_name, 
					new_login_name, &newvalue_array, 
					&modvalues, SUBSTITUTE)) == 1)
	    goto cleanup;
	  newvalue_array[newvalue_count]->mod_op = LDAP_MOD_REPLACE ;
	  newvalue_array[newvalue_count]->mod_type = "UserPrincipalName";
	  newvalue_array[newvalue_count]->mod_values = 
	    modvalues[newvalue_count];
	  linklist_free(group_base);
	  group_count = 0;
	  group_base = NULL;
	  ++newvalue_count;
	}
      
      sprintf(filter_exp, "(sAMAccountName=%s)", login_name);
      attr_array[0] = "AltSecurityIdentities";
      attr_array[1] = NULL;
      if ((rc = linklist_build(ldap_handle, dn_path, filter_exp, attr_array, 
			       &group_base, &group_count)) != 0)
	goto cleanup;
      if (group_count != 0)
	{
	  if ((rc = construct_newvalues(group_base, group_count, 
					newvalue_count, login_name, 
					new_login_name, &newvalue_array, 
					&modvalues, SUBSTITUTE)) == 1)
	    goto cleanup;
	  newvalue_array[newvalue_count]->mod_op = LDAP_MOD_REPLACE ;
	  newvalue_array[newvalue_count]->mod_type = "AltSecurityIdentities";
	  newvalue_array[newvalue_count]->mod_values = 
	    modvalues[newvalue_count];
	  linklist_free(group_base);
	  group_count = 0;
	  group_base = NULL;
	  ++newvalue_count;
	}
      
      if (newvalue_count != 0)
	rc = ldap_modify_s(ldap_handle, distinguished_name, newvalue_array);
      if (rc != LDAP_SUCCESS)
	{
	  critical_alert("incremental",
			 "Couldn't process user %s: %s",
			 login_name, ldap_err2string(rc));
	}
      goto cleanup;
    }
  if ((operation == MEMBER_ACTIVATE) ||
      (operation == MEMBER_DEACTIVATE))
    {    
      sprintf(filter_exp, "(sAMAccountName=%s)", login_name);
      attr_array[0] = "UserAccountControl";
      attr_array[1] = NULL;
      if ((rc = linklist_build(ldap_handle, dn_path, filter_exp, attr_array, 
			       &group_base, &group_count)) != 0)
	goto cleanup;
      
      if (group_count != 0)
	{
	  ulongValue = atoi((*group_base).value);
	  if (operation == MEMBER_DEACTIVATE)
	    ulongValue |= UF_ACCOUNTDISABLE;
	  else    
	    ulongValue &= ~UF_ACCOUNTDISABLE;
	  sprintf(temp, "%ld", ulongValue);
	  if ((rc = construct_newvalues(group_base, group_count, 
					newvalue_count, (*group_base).value, 
					temp, &newvalue_array, &modvalues, 
					REPLACE)) == 1)
	    goto cleanup;
	  newvalue_array[newvalue_count]->mod_op = LDAP_MOD_REPLACE ;
	  newvalue_array[newvalue_count]->mod_type = "UserAccountControl";
	  newvalue_array[newvalue_count]->mod_values = 
	    modvalues[newvalue_count];
	  ++newvalue_count;
	  rc = ldap_modify_s(ldap_handle, distinguished_name, newvalue_array);
	  if (rc != LDAP_SUCCESS)
	    {
	      critical_alert("incremental",
			     "Couldn't process user %s: %s",
			     login_name, ldap_err2string(rc));
	    }
	}
      goto cleanup;
    }
  
  sprintf(filter_exp, "(objectClass=group)");
  attr_array[0] = "cn";
  attr_array[1] = NULL;
  if ((rc = linklist_build(ldap_handle, dn_path, filter_exp, attr_array, 
			   &group_base, &group_count)) != 0)
    goto cleanup;
  
  sPtr = group_list;
  while (sPtr != NULL)
    {
      gPtr = group_base;
      while (gPtr != NULL)
	{
	  if (!strcmp(sPtr->list, gPtr->value))
	    {
	      if (sPtr->dn != NULL)
		free(sPtr->dn);
	      sPtr->dn = calloc(1, strlen(gPtr->dn) + 1);
	      memset(sPtr->dn, '\0', strlen(gPtr->dn) + 1);
	      strcpy(sPtr->dn, gPtr->dn);
	      if (sPtr->attribute != NULL)
		free(sPtr->attribute);
	      sPtr->attribute = calloc(1, strlen(gPtr->attribute) + 1);
	      memset(sPtr->attribute, '\0', strlen(gPtr->attribute) + 1);
	      strcpy(sPtr->attribute, gPtr->attribute);
	      if (sPtr->value != NULL)
		free(sPtr->value);
	      sPtr->value = calloc(1, strlen(gPtr->value) + 1);
	      memset(sPtr->value, '\0', strlen(gPtr->value) + 1);
	      strcpy(sPtr->value, gPtr->value);
	      break;
	    }
	  gPtr = gPtr->next;
	}
      sPtr = sPtr->next;
    }
  
  memberOf_count = 0;
  memberOf_base = NULL;
  sprintf(filter_exp, "(sAMAccountName=%s)", login_name);
  attr_array[0] = "memberOf";
  attr_array[1] = NULL;
  if ((rc = linklist_build(ldap_handle, dn_path, filter_exp, attr_array, 
			   &memberOf_base, &memberOf_count)) != 0)
    goto cleanup;
  
  processGroup_count = 0;
  processGroup_base = NULL;
  sPtr = group_list;
  while (sPtr)
    {
      if (sPtr->dn == NULL)
	{
	  sPtr = sPtr->next;
	  continue;
	}
      gPtr = memberOf_base;
      while (gPtr)
	{
	  if (!strcmp(gPtr->value, sPtr->dn))
	    break;
	  gPtr = gPtr->next;
	}
      if (sPtr->dn != NULL)
	{
	  if (((gPtr == NULL) && (operation == MEMBER_ADD)) ||
	      ((gPtr != NULL) && (operation == MEMBER_REMOVE)))
	    {
	      gPtr = calloc(1, sizeof(LK_ENTRY));
	      memset(gPtr, '\0', sizeof(LK_ENTRY));
	      gPtr->attribute = calloc(1, strlen("member") + 1);
	      memset(gPtr->attribute, '\0', strlen("member") + 1);
	      strcpy(gPtr->attribute, "member");
	      gPtr->dn = calloc(1, strlen(sPtr->dn) + 1);
	      memset(gPtr->dn, '\0', strlen(sPtr->dn) + 1);
	      strcpy(gPtr->dn, sPtr->dn);
	      gPtr->value = calloc(1, strlen(distinguished_name) + 1);
	      memset(gPtr->value, '\0', strlen(distinguished_name) + 1);
	      strcpy(gPtr->value, distinguished_name);
	      if (processGroup_base != NULL)
		gPtr->next = processGroup_base;
	      processGroup_base = gPtr;
	      gPtr = NULL;
	      ++processGroup_count;
	    }
	}
      sPtr = sPtr->next;
    }
  
  linklist_free(group_base);
  linklist_free(processGroup);
  group_base = NULL;
  processGroup = NULL;
  sPtr = processGroup_base;
  while (sPtr)
    {
      newvalue_count = 0;
      newvalue_array = NULL;
      modvalues = NULL;
      processGroup = NULL;
      if ((newvalue_array = calloc(1, (1+1) * sizeof(LDAPMod *))) == NULL)
	{
	  rc = 1;
	  goto cleanup;
	}
      newvalue_array[0] = NULL;
      newvalue_array[1] = NULL;
      if ((modvalues = calloc(1, (1+1) * sizeof(char **))) == NULL)
	{
	  rc = 1;
	  goto cleanup;
	}
      modvalues[0] = NULL;
      modvalues[1] = NULL;
      
      if (operation == MEMBER_REMOVE)
	{
	  processGroup_count = 0;
	  processGroup = NULL;
	  sprintf(filter_exp, "(distinguishedName=%s)", sPtr->dn);
	  attr_array[0] = "member";
	  attr_array[1] = NULL;
	  if ((rc = linklist_build(ldap_handle, dn_path, filter_exp, 
				   attr_array,  &processGroup, 
				   &processGroup_count)) != 0)
	    goto cleanup;
	  gPtr = processGroup;
	  pPtr = NULL;
	  while (gPtr)
	    {
	      if (!strcmp(gPtr->value, sPtr->value))
		{
		  if (pPtr != NULL)
		    pPtr->next = gPtr->next;
		  else
		    processGroup = gPtr->next;
		  gPtr->next = NULL;
		  linklist_free(gPtr);
		  --processGroup_count;
		  break;
		}
	      pPtr = gPtr;
	      gPtr = gPtr->next;
	    }
	  if ((rc = construct_newvalues(processGroup, processGroup_count, 
					newvalue_count, NULL, NULL,
					&newvalue_array, &modvalues, 
					REPLACE)) == 1)
	    goto cleanup;
	  newvalue_array[newvalue_count]->mod_op = LDAP_MOD_REPLACE ;
	  pPtr = processGroup;
	}
      else
	{
	  processGroup = calloc(1, sizeof(LK_ENTRY));
	  memset(processGroup, '\0', sizeof(LK_ENTRY));
	  memcpy(processGroup, sPtr, sizeof(LK_ENTRY));
	  processGroup->next = NULL;
	  processGroup_count = 1;
	  if ((rc = construct_newvalues(processGroup, processGroup_count, 
					newvalue_count, NULL, NULL,
					&newvalue_array, &modvalues, 
					REPLACE)) == 1)
	    goto cleanup;
	  newvalue_array[newvalue_count]->mod_op = LDAP_MOD_ADD;
	  pPtr = sPtr;
	}
      newvalue_array[newvalue_count]->mod_type = "member";
      newvalue_array[newvalue_count]->mod_values = modvalues[newvalue_count];
      rc = ldap_modify_s(ldap_handle, sPtr->dn, newvalue_array);
      if (operation == MEMBER_REMOVE)
	linklist_free(processGroup);
      else
	free(processGroup);
      processGroup = NULL;
      newvalue_count = 1;
      free_values(newvalue_count, newvalue_array, modvalues);
      newvalue_array = NULL;
      modvalues = NULL;
      if (rc != LDAP_SUCCESS)
	{
	  critical_alert("incremental",
			 "Couldn't process user %s: %s",
			 login_name, ldap_err2string(rc));
	  goto cleanup;
	}
      sPtr = sPtr->next;
    }
  
 cleanup:
  free_values(newvalue_count, newvalue_array, modvalues);
  linklist_free(memberOf_base);
  linklist_free(group_base);
  linklist_free(processGroup_base);
  free(dn_path);
  if (rc == 1)
    critical_alert("incremental",
		   "Couldn't process user %s: %s",
		   login_name, "Cannot calloc - out of memory");
  return(rc);
}

int construct_newvalues(LK_ENTRY *linklist_base, int modvalue_count, 
			int newvalue_count, char *oldValue, char *newValue,
			LDAPMod ***newValueArray, char ****modvalues,
			int type)
{
  LK_ENTRY *linklist_ptr;
  int i;
  char *cPtr;
  
  if (((*newValueArray)[newvalue_count] = 
       calloc(1, sizeof(LDAPMod))) == NULL)
    return(1);

  if (((*modvalues)[newvalue_count] = 
       calloc(1, (modvalue_count + 1) * sizeof(char *))) == NULL)
    return(1);

  for (i = 0; i < (modvalue_count + 1); i++)
    (*modvalues)[newvalue_count][i] = NULL;
  if (modvalue_count != 0)
    {
      linklist_ptr = linklist_base;
      for (i = 0; i < modvalue_count; i++)
	{
	  if ((oldValue != NULL) && (newValue != NULL))
	    {
	      if ((cPtr = (char *)strstr(linklist_ptr->value, oldValue)) 
		  != (char *)NULL)
		{
		  if (type == REPLACE)
		    {
		      if (((*modvalues)[newvalue_count][i] = 
			   calloc(1, strlen(newValue) + 1)) == NULL)
			return(1);
		      memset((*modvalues)[newvalue_count][i], '\0', 
			     strlen(newValue) + 1);
		      strcpy((*modvalues)[newvalue_count][i], newValue);
		    }
		  else
		    {
		      if (((*modvalues)[newvalue_count][i] = 
			   calloc(1, (int)(cPtr - linklist_ptr->value) + 
				  (linklist_ptr->length - strlen(oldValue)) + 
				  strlen(newValue) + 1)) == NULL)
			return(1);
		      memset((*modvalues)[newvalue_count][i], '\0', 
			     (int)(cPtr - linklist_ptr->value) + 
			     (linklist_ptr->length - strlen(oldValue)) + 
			     strlen(newValue) + 1);
		      memcpy((*modvalues)[newvalue_count][i], 
			     linklist_ptr->value, 
			     (int)(cPtr - linklist_ptr->value));
		      strcat((*modvalues)[newvalue_count][i], newValue);
		      strcat((*modvalues)[newvalue_count][i], 
			     &linklist_ptr->value[(int)(cPtr - linklist_ptr->value) + strlen(oldValue)]);
		    }
		}
	      else
		{
		  (*modvalues)[newvalue_count][i] = 
		    calloc(1, linklist_ptr->length + 1);
		  memset((*modvalues)[newvalue_count][i], 
			 '\0', linklist_ptr->length + 1);
		  memcpy((*modvalues)[newvalue_count][i], 
			 linklist_ptr->value, linklist_ptr->length);
		}
	    }
	  else
	    {
	      (*modvalues)[newvalue_count][i] = 
		calloc(1, linklist_ptr->length + 1);
	      memset((*modvalues)[newvalue_count][i], '\0', 
		     linklist_ptr->length + 1);
	      memcpy((*modvalues)[newvalue_count][i], linklist_ptr->value, 
		     linklist_ptr->length);
	    }
	  linklist_ptr = linklist_ptr->next;
	}
      (*modvalues)[newvalue_count][i] = NULL;
    }
  return(0);
}

int linklist_build(LDAP *ldap_handle, char *dn_path, char *search_exp, 
		   char **attr_array, LK_ENTRY **linklist_base,
		   int *linklist_count)
{
  ULONG rc = 0;
  LDAPMessage *ldap_entry;
  
  ldap_entry = NULL;
  (*linklist_base) = NULL;
  (*linklist_count) = 0;
  if ((rc = ldap_search_s(ldap_handle, dn_path, LDAP_SCOPE_SUBTREE, 
			  search_exp, attr_array, 0, &ldap_entry)) 
      != LDAP_SUCCESS)
    return(0);
  rc = retrieve_entries(ldap_handle, ldap_entry, linklist_base, 
			linklist_count);
  
  ldap_msgfree(ldap_entry);
  return(rc);
}

int retrieve_entries(LDAP *ldap_handle, LDAPMessage *ldap_entry, 
		     LK_ENTRY **linklist_base, int *linklist_count)
{
  char distinguished_name[1024];
  LK_ENTRY *linklist_ptr;
  int rc;
  
  memset(distinguished_name, '\0', sizeof(distinguished_name));
  get_distinguished_name(ldap_handle, ldap_entry, distinguished_name);
  
  if ((ldap_entry = ldap_first_entry(ldap_handle, ldap_entry)) == NULL)
    return(0);
  
  if ((rc = retrieve_attributes(ldap_handle, ldap_entry, distinguished_name, 
				linklist_base)) != 0)
    return(rc);
  
  while ((ldap_entry = ldap_next_entry(ldap_handle, ldap_entry)) != NULL)
    {
      if ((rc = retrieve_attributes(ldap_handle, ldap_entry, 
				    distinguished_name, linklist_base)) != 0)
	return(rc);
    }
  
  linklist_ptr = (*linklist_base);
  (*linklist_count) = 0;
  while (linklist_ptr != NULL)
    {
      ++(*linklist_count);
      linklist_ptr = linklist_ptr->next;
    }
  return(0);
}

int retrieve_attributes(LDAP *ldap_handle, LDAPMessage *ldap_entry, 
			char *distinguished_name, LK_ENTRY **linklist_current)
{
  char *Attribute;
  BerElement *ptr;
  
  ptr = NULL;
  if ((Attribute = ldap_first_attribute(ldap_handle, ldap_entry, &ptr)) 
      != NULL)
    {
      retrieve_values(ldap_handle, ldap_entry, Attribute, distinguished_name, 
		      linklist_current);
      ldap_memfree(Attribute);
      while ((Attribute = ldap_next_attribute(ldap_handle, ldap_entry, ptr)) 
	     != NULL)
	{
	  retrieve_values(ldap_handle, ldap_entry, Attribute, 
			  distinguished_name, linklist_current);
	  ldap_memfree(Attribute);
	}
    }
  return(0);
}

int retrieve_values(LDAP *ldap_handle, LDAPMessage *ldap_entry, 
		    char *Attribute, char *distinguished_name,
		    LK_ENTRY **linklist_current)
{
  char **str_value;
  char temp[256];
  void **Ptr;
  int use_bervalue;
  LK_ENTRY *linklist_previous;
  LDAP_BERVAL **ber_value;
  DWORD ber_length;
#ifdef LDAP_DEBUG
  SID *sid;
  GUID *guid;
  int i;
  int intValue;
  DWORD *subauth;
  SID_IDENTIFIER_AUTHORITY *sid_auth;
  unsigned char *subauth_count;
#endif /*LDAP_DEBUG*/

  use_bervalue = 0;
  memset(temp, '\0', sizeof(temp));
  if ((!strcmp(Attribute, "objectSid")) ||
      (!strcmp(Attribute, "objectGUID")))
    use_bervalue = 1;
  
  if (use_bervalue)
    {
      ber_value = ldap_get_values_len(ldap_handle, ldap_entry, Attribute);
      Ptr = (void **)ber_value;
      str_value = NULL;
    }
  else
    {
      str_value = ldap_get_values(ldap_handle, ldap_entry, Attribute);
      Ptr = (void **)str_value;
      ber_value = NULL;
    }
  if (Ptr != NULL)
    {
      for (; *Ptr; Ptr++) 
	{
	  if ((linklist_previous = calloc(1, sizeof(LK_ENTRY))) == NULL)
	    return(1);
	  memset(linklist_previous, '\0', sizeof(LK_ENTRY));
	  linklist_previous->next = (*linklist_current);
	  (*linklist_current) = linklist_previous;
	  
	  if (((*linklist_current)->attribute = 
	       calloc(1, strlen(Attribute) + 1)) == NULL)
	    return(1);
	  memset((*linklist_current)->attribute, '\0', strlen(Attribute) + 1);
	  strcpy((*linklist_current)->attribute, Attribute);
	  if (use_bervalue)
	    {
	      ber_length = (*(LDAP_BERVAL **)Ptr)->bv_len;
	      if (((*linklist_current)->value = calloc(1, ber_length)) == NULL)
		return(1);
	      memset((*linklist_current)->value, '\0', ber_length);
	      memcpy((*linklist_current)->value, 
		     (*(LDAP_BERVAL **)Ptr)->bv_val, ber_length);
	      (*linklist_current)->length = ber_length;
	    }
	  else
	    {
	      if (((*linklist_current)->value = calloc(1, strlen(*Ptr) + 1)) 
		  == NULL)
		return(1);
	      memset((*linklist_current)->value, '\0', strlen(*Ptr) + 1);
	      (*linklist_current)->length = strlen(*Ptr);
	      strcpy((*linklist_current)->value, *Ptr);
	    }
	  (*linklist_current)->ber_value = use_bervalue;
	  if (((*linklist_current)->dn = 
	       calloc(1, strlen(distinguished_name) + 1)) == NULL)
	    return(1);
	  memset((*linklist_current)->dn, '\0', 
		 strlen(distinguished_name) + 1);
	  strcpy((*linklist_current)->dn, distinguished_name);
	  
#ifdef LDAP_DEBUG
	  if (!strcmp(Attribute, "objectGUID"))
	    {
	      guid = (GUID *)((*linklist_current)->value);
	      sprintf(temp, 
		      "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", 
		      guid->Data1, guid->Data2, guid->Data3, 
		      guid->Data4[0], guid->Data4[1], guid->Data4[2], 
		      guid->Data4[3], guid->Data4[4], guid->Data4[5], 
		      guid->Data4[6], guid->Data4[7]);
	      print_to_screen("     %20s : {%s}\n", Attribute, temp);
	    }
	  else if (!strcmp(Attribute, "objectSid"))
	    {
	      sid = (SID *)((*(LDAP_BERVAL **)Ptr)->bv_val);
#ifdef _WIN32
	      print_to_screen("        Revision = %d\n", sid->Revision);
	      print_to_screen("        SID Identifier Authority:\n");
	      sid_auth = &sid->IdentifierAuthority;
	      if (sid_auth->Value[0])
		print_to_screen("            SECURITY_NULL_SID_AUTHORITY\n");
	      else if (sid_auth->Value[1])
		print_to_screen("            SECURITY_WORLD_SID_AUTHORITY\n");
	      else if (sid_auth->Value[2])
		print_to_screen("            SECURITY_LOCAL_SID_AUTHORITY\n");
	      else if (sid_auth->Value[3])
		print_to_screen("            SECURITY_CREATOR_SID_AUTHORITY\n");
	      else if (sid_auth->Value[5])
		print_to_screen("            SECURITY_NT_AUTHORITY\n");
	      else
		print_to_screen("            UNKNOWN SID AUTHORITY\n");
	      subauth_count = GetSidSubAuthorityCount(sid);
	      print_to_screen("        SidSubAuthorityCount = %d\n", 
			      *subauth_count);
	      print_to_screen("        SidSubAuthority:\n");
	      for (i = 0; i < *subauth_count; i++)
		{
		  if ((subauth = GetSidSubAuthority(sid, i)) != NULL)
		    print_to_screen("            %u\n", *subauth);
		}
#endif
	    }
	  else if ((!memcmp(Attribute, "userAccountControl", 
			    strlen("userAccountControl"))) ||
		   (!memcmp(Attribute, "sAMAccountType", 
			    strlen("sAmAccountType"))))
	    {
	      intValue = atoi(*Ptr);
	      print_to_screen("     %20s : %ld\n",Attribute, intValue);
	      if (!memcmp(Attribute, "userAccountControl", 
			  strlen("userAccountControl")))
		{
		  if (intValue & UF_ACCOUNTDISABLE)
		    print_to_screen("     %20s :    %s\n", "", 
				    "Account disabled");
		  else
		    print_to_screen("     %20s :    %s\n", "", 
				    "Account active");
		  if (intValue & UF_HOMEDIR_REQUIRED)
		    print_to_screen("     %20s :    %s\n", "", 
				    "Home directory required");
		  if (intValue & UF_LOCKOUT)
		    print_to_screen("     %20s :    %s\n", "", 
				    "Account locked out");
		  if (intValue & UF_PASSWD_NOTREQD)
		    print_to_screen("     %20s :    %s\n", "", 
				    "No password required");
		  if (intValue & UF_PASSWD_CANT_CHANGE)
		    print_to_screen("     %20s :    %s\n", "", 
				    "Cannot change password");
		  if (intValue & UF_TEMP_DUPLICATE_ACCOUNT)
		    print_to_screen("     %20s :    %s\n", "", 
				    "Temp duplicate account");
		  if (intValue & UF_NORMAL_ACCOUNT)
		    print_to_screen("     %20s :    %s\n", "", 
				    "Normal account");
		  if (intValue & UF_INTERDOMAIN_TRUST_ACCOUNT)
		    print_to_screen("     %20s :    %s\n", "", 
				    "Interdomain trust account");
		  if (intValue & UF_WORKSTATION_TRUST_ACCOUNT)
		    print_to_screen("     %20s :    %s\n", "", 
				    "Workstation trust account");
		  if (intValue & UF_SERVER_TRUST_ACCOUNT)
		    print_to_screen("     %20s :    %s\n", "", 
				    "Server trust account");
		}
	    }
	  else
	    {
	      print_to_screen("     %20s : %s\n",Attribute, *Ptr);
	    }
#endif /*LDAP_DEBUG*/
	}
      if (str_value != NULL)
	ldap_value_free(str_value);
      if (ber_value != NULL)
	ldap_value_free_len(ber_value);
    }
  (*linklist_current) = linklist_previous;
  return(0);
}

int add_user_lists(int ac, char **av, void *user)
{
  if (atoi(av[L_ACTIVE]) && atoi(av[L_GROUP]))	/* active group ? */
    edit_group(1, av[L_NAME], "USER", user, NULL, NULL, NULL);
  return 0;
}

void edit_group(int op, char *group, char *type, char *member,
		LDAP *ldap_handle, char *dn_path, char *ldap_hostname)
{
  ULONG rc;
  char *p = 0;
  char search_exp[128];
  static char local_realm[REALM_SZ+1] = "";
  LK_ENTRY *linklist;
  LDAPMessage *ldap_entry; 
  
  /* The following KERBEROS rc allows for the use of entities
   * user@foreign_cell.
   */
  if (!local_realm[0])
    krb_get_lrealm(local_realm, 1);
  if (!strcmp(type, "KERBEROS"))
    {
      p = (char *)strchr(member, '@');
      if (p && !strcasecmp(p+1, local_realm))
	*p = 0;
    }
  else if (strcmp(type, "USER"))
    return;					/* invalid type */
  
  /* Cannot risk doing another query during a callback */
  /* We could do this simply for type USER, but eventually this may also
   * dynamically add KERBEROS types to the prdb, and we will need to do
   * a query to look up the uid of the null-instance user 
   */
  if (mr_connections)
    {
      linklist = calloc(1, sizeof(LK_ENTRY));
      if (!linklist)
	{
	  critical_alert("incremental", "Out of memory");
	  exit(1);
	}
      memset(linklist, '\0', sizeof(LK_ENTRY));
      linklist->op = op;
      linklist->dn = NULL;
      linklist->list = calloc(1, strlen(group) + 1);
      memset(linklist->list, '\0', strlen(group) + 1);
      strcpy(linklist->list, group);
      linklist->type = calloc(1, strlen(type) + 1);
      memset(linklist->type, '\0', strlen(type) + 1);
      strcpy(linklist->type, type);
      linklist->member = calloc(1, strlen(member) + 1);
      memset(linklist->member, '\0', strlen(member) + 1);
      strcpy(linklist->member, member);
      linklist->next = member_base;
      member_base = linklist;
      return;
    }
  
  com_err(whoami, 0, "%s %s %s group %s", (op ? "Adding" : "Removing"), 
	  member, (op ? "to" : "from"), group);
  sprintf(search_exp, "(sAMAccountName=%s)", member);
  if ((rc = ldap_search_s(ldap_handle, dn_path, LDAP_SCOPE_SUBTREE, 
			  search_exp, NULL, 0, &ldap_entry)) != LDAP_SUCCESS)
    {
      critical_alert("incremental", "Couldn't %s %s %s %s",
		     op ? "add" : "remove", member,
		     op ? "to" : "from", group);
      rc = ldap_msgfree(ldap_entry);
      return;
    }
  linklist = calloc(1, sizeof(LK_ENTRY));
  if (!linklist)
    {
      critical_alert("incremental", "Out of memory");
      exit(1);
    }
  memset(linklist, '\0', sizeof(LK_ENTRY));
  linklist->op = op;
  linklist->dn = NULL;
  linklist->list = calloc(1, strlen(group) + 1);
  memset(linklist->list, '\0', strlen(group) + 1);
  strcpy(linklist->list, group);
  linklist->type = calloc(1, strlen(type) + 1);
  memset(linklist->type, '\0', strlen(type) + 1);
  strcpy(linklist->type, type);
  linklist->member = calloc(1, strlen(member) + 1);
  memset(linklist->member, '\0', strlen(member) + 1);
  strcpy(linklist->member, member);
  linklist->next = NULL;
  
  rc = member_update(ldap_handle, dn_path, ldap_entry, member, NULL, 
		     ldap_hostname, op ? MEMBER_ADD : MEMBER_REMOVE, linklist);
  if (rc)
    {
      critical_alert("incremental", "Couldn't %s %s %s %s: %s",
		     op ? "add" : "remove", member,
		     op ? "to" : "from", group);
    }
  
  linklist_free(linklist);
  rc = ldap_msgfree(ldap_entry);
  /*
   *  rc = pr_try(op ? pr_AddToGroup : pr_RemoveUserFromGroup, member, buf);
   *  if (rc)
   *  {
   *  if (op==1 && rc == PRIDEXIST)
   *  return;
   *  if (rc == PRNOENT)
   *  {
   *  if (op == 0)
   *  return;
   * if (!strcmp(type, "KERBEROS"))
   *   return;
   *  rc = moira_connect();
   *  if (!rc)
   *  {
   *  rc = mr_query("get_user_by_login", 1, &member,
   *  (void *)check_user, (char *) &ustate);
   *  }
   *  if (rc)
   *  {
   *  critical_alert("incremental", "Error contacting Moira server "
   *  "to lookup user %s: %s", member,
   *  error_message(rc));
   *  }
   *  mr_disconnect();
   *  mr_connections--;
   *  if (!rc && ustate!=1 && ustate!=2)
   *  return;
   *  rc = PRNOENT;
   *  }
   *  critical_alert("incremental", "Couldn't %s %s %s %s: %s",
   *  op ? "add" : "remove", member,
   *  op ? "to" : "from", buf,
   *  error_message(rc));
   *  }
   */
}

int moira_connect(void)
{
  long rc;
  char HostName[64];
  
  if (!mr_connections++)
    {
#ifdef _WIN32
      memset(HostName, '\0', sizeof(HostName));
      strcpy(HostName, "ttsp");
      rc = mr_connect(HostName);
#else
      struct utsname uts;
      uname(&uts);
      rc = mr_connect(uts.nodename);
#endif /*WIN32*/
      if (!rc)
	rc = mr_auth("afs.incr");
      return rc;
    }
  return 0;
}

int moira_disconnect(void)
{
  if (!--mr_connections)
    mr_disconnect();
  return 0;
}

int add_list_members(int ac, char **av, void *group)
{
  edit_group(1, group, av[0], av[1], NULL, NULL, NULL);
  return 0;
}

int check_user(int ac, char **av, void *ustate)
{
  *(int *)ustate = atoi(av[U_STATE]);
  return 0;
}

void free_values(int newvalue_count, LDAPMod **newvalue_array, 
		 char ***modvalues)
{
  int i, j;
  
  if (newvalue_array != NULL)
    {
      i = 0;
      while (newvalue_array[i] != NULL)
	{
	  free(newvalue_array[i]);
	  newvalue_array[i]= NULL;
	  ++i;
	}
      free(newvalue_array);
      newvalue_array = NULL;
    }
  if (modvalues != NULL)
    {
      i = 0;
      while (modvalues[i] != NULL)
	{
	  j = 0;
	  while (modvalues[i][j] != NULL)
	    {
	      free(modvalues[i][j]);
	      modvalues[i][j] = NULL;
	      ++j;
	    }
	  free(modvalues[i]);
	  modvalues[i] = NULL;
	  ++i;
	}
      free(modvalues);
      modvalues = NULL;
    }
}

void linklist_free(LK_ENTRY *linklist_base)
{
  LK_ENTRY *linklist_previous;
  
  while (linklist_base != NULL)
    {
      if (linklist_base->dn != NULL)
	free(linklist_base->dn);
      if (linklist_base->attribute != NULL)
	free(linklist_base->attribute);
      if (linklist_base->value != NULL)
	free(linklist_base->value);
      if (linklist_base->member != NULL)
	free(linklist_base->member);
      if (linklist_base->type != NULL)
	free(linklist_base->type);
      if (linklist_base->list != NULL)
	free(linklist_base->list);
      linklist_previous = linklist_base;
      linklist_base = linklist_previous->next;
      free(linklist_previous);
    }
}

int convert_domain_to_dn(char *domain, char **dnp)
{
  char *fp, *dp;
  char dn[1024];
  int dnlen = 1;
  
  memset(dn, 0, sizeof(dn));
  strcpy(dn, "dc=");
  dp = dn+3;
  for (fp = domain; *fp; fp++)
    {
      if (*fp == '.') 
	{
	  strcpy(dp, ",dc=");
	  dp += 4;
	}
      else
	*dp++ = *fp;
    }
  
  *dnp = (char *)strdup(dn);
  return 0;
}

void get_distinguished_name(LDAP *ldap_handle, LDAPMessage *ldap_entry, 
			    char *distinguished_name)
{
  char *CName;
  
  CName = ldap_get_dn(ldap_handle, ldap_entry);
  if (CName == NULL)
    return;
  strcpy(distinguished_name, CName);
  ldap_memfree(CName);
}

int linklist_create_entry(char *attribute, char *value, 
			  LK_ENTRY **linklist_entry)
{
  (*linklist_entry) = calloc(1, sizeof(LK_ENTRY));
  if (!(*linklist_entry))
    {
      return(1);
    }
  memset((*linklist_entry), '\0', sizeof(LK_ENTRY));
  (*linklist_entry)->attribute = calloc(1, strlen(attribute) + 1);
  memset((*linklist_entry)->attribute, '\0', strlen(attribute) + 1);
  strcpy((*linklist_entry)->attribute, attribute);
  (*linklist_entry)->value = calloc(1, strlen(value) + 1);
  memset((*linklist_entry)->value, '\0', strlen(value) + 1);
  strcpy((*linklist_entry)->value, value);
  (*linklist_entry)->length = strlen(value);
  (*linklist_entry)->next = NULL;
  return(0);
}

void print_to_screen(const char *fmt, ...)
{
  va_list pvar;
  
  va_start(pvar, fmt);
  vfprintf(stderr, fmt, pvar);
  fflush(stderr);
  va_end(pvar);
}
