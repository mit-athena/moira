/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/incremental/winad/winad.c,v 1.11 2001-05-02 21:28:55 zacheiss Exp $
/* test parameters for creating a user account - done 
 * users 10 10 a_chen 31275 sh cmd Lastname Firstname Middlename 0 950000000 STAFF a_chen 31275 sh cmd Lastname Firstname Middlename 2 950000000 STAFF
 * users 10 10 a_chen 31275 sh cmd Lastname Firstname Middlename 2 950000000 STAFF a_chen 31275 sh cmd Lastname Firstname Middlename 1 950000000 STAFF
 *   login, unix_uid, shell, winconsoleshell, last, first, middle, status, clearid, type
 *
 * test parameters for deactivating/deleting a user account - done
 * users 10 10 testacc 31275 sh cmd Lastname Firstname Middlename 1 950000000 STAFF testacc 31275 sh cmd Lastname Firstname Middlename 3 950000000 STAFF 
 * users 10 10 testacc 31275 sh cmd Lastname Firstname Middlename 2 950000000 STAFF testacc 31275 sh cmd Lastname Firstname Middlename 3 950000000 STAFF 
 *   login, unix_uid, shell, winconsoleshell, last, first, middle, status, clearid, type
 * comment: clearid is the MIT ID
 *
 * test parameters for reactivating a user account - done
 * users 10 10 testacc 31275 sh cmd Lastname Firstname Middlename 3 950000000 STAFF testacc 31275 sh cmd Lastname Firstname Middlename 2 950000000 STAFF 
 *   login, unix_uid, shell, winconsoleshell, last, first, middle, status, clearid, type
 *
 * test parameters for updating user account info - done
 * users 10 10 testacc 31275 sh cmd Lastname Firstname Middlename 2 950000000 STAFF testacc 31275 sh cmd newLastname Firstname Middlename 2 950000000 STAFF 
 * users 10 10 6_d0006 950 sh cmd Lastname Firstname Middlename 1 900012345 STAFF 6_d0006 950 sh cmd Lastname Firstname Middlename 1 950012345 STAFF
 *   login, unix_uid, shell, winconsoleshell, last, first, middle, status, clearid, type
 *   currently, if the unix_id doesn't change, only the U_UID or U_MITID fields will be updated
 *
 * test parameters for changing user name - testing
 * users 10 10 testacc 31275 sh cmd Lastname Firstname Middlename 2 950000000 STAFF testacc1 31275 sh cmd Lastname Firstname Middlename 2 950000000 STAFF
 * users 10 10 testacc 31275 sh cmd Lastname Firstname Middlename 1 950000000 STAFF testacc1 31275 sh cmd Lastname Firstname Middlename 1 950000000 STAFF
 *   login, unix_uid, shell, winconsoleshell, last, first, middle, status, clearid, type
 *
 * test parameters for add member to group/list - done
 * imembers 0 10 pismere-team USER dtanner 1 1 0 1 1 -1 1
 * imembers 0 9 pismere-team STRING hope@ful.net 1 1 0 1 1 -1
 *   list_name, user_type, name, active, publicflg, hidden, maillist, grouplist, gid
 *
 * test parameters for remove member from group/list - done
 * imembers 10 0 pismere-team USER dtanner 1 1 0 1 1 -1 1
 * imembers 9 0 pismere-team STRING hope@ful.net 1 1 0 1 1 -1
 *   list_name, user_type, name, active, publicflg, hidden, maillist, grouplist, gid
 *
 * test parameters for creating and/or populating a group/list - done
 * list 0 10 pismere-team 1 1 0 1 0 -1 USER 95260 description
 *   name, active, publicflg, hidden, maillist, grouplist, gid, acl_type, acl_id, description
 * 
 * test parameters for deleting a group/list - done
 * list 10 0 pismere-team 1 1 0 1 0 -1 USER 95260 description
 *   name, active, publicflg, hidden, maillist, grouplist, gid, acl_type, acl_id, description
 *
 * test parameters for renaming a group/list - done
 *  list 10 10 adtestlist 1 1 0 1 0 -1 USER 95260 description pismere-team 1 1 0 1 1 -1 USER 95260 description
 *  list 10 10 pismere-team 1 1 0 1 1 -1 USER 95260 description adtestlist1 1 1 0 1 0 -1 USER 95260 description
 *   name, active, publicflg, hidden, maillist, grouplist, gid, acl_type, acl_id, description
*/
#include <mit-copyright.h>
#ifdef _WIN32
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <lmaccess.h>
#endif
#include <hesiod.h>
#include <string.h>
#include <ldap.h>
#include <stdio.h>
#include <moira.h>
#include <moira_site.h>
#include <mrclient.h>
#include <krb5.h>
#include <krb.h>
#include <gsssasl.h>
#include <gssldap.h>
#include "kpasswd.h"

#ifdef _WIN32
#ifndef ECONNABORTED
#define ECONNABORTED WSAECONNABORTED
#endif
#ifndef ECONNREFUSED
#define ECONNREFUSED WSAECONNREFUSED
#endif
#ifndef EHOSTUNREACH
#define EHOSTUNREACH WSAEHOSTUNREACH
#endif
#define krb5_xfree free
#define F_OK 0
#define sleep(A) Sleep(A * 1000);
#endif /* _WIN32 */

#ifndef _WIN32
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <sys/utsname.h>
#include <unistd.h>

#define strnicmp(A,B,C) strncasecmp(A,B,C)
#define UCHAR unsigned char

#define UF_SCRIPT               0x0001
#define UF_ACCOUNTDISABLE       0x0002
#define UF_HOMEDIR_REQUIRED     0x0008
#define UF_LOCKOUT              0x0010
#define UF_PASSWD_NOTREQD       0x0020
#define UF_PASSWD_CANT_CHANGE   0x0040
#define UF_DONT_EXPIRE_PASSWD   0x10000

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

#define AFS "/afs/"
#define WINAFS "\\\\afs\\all\\"

#define ADS_GROUP_TYPE_GLOBAL_GROUP         0x00000002
#define ADS_GROUP_TYPE_DOMAIN_LOCAL_GROUP   0x00000004
#define ADS_GROUP_TYPE_LOCAL_GROUP          0x00000004
#define ADS_GROUP_TYPE_UNIVERSAL_GROUP      0x00000008
#define ADS_GROUP_TYPE_SECURITY_ENABLED     0x80000000

#define QUERY_VERSION -1
#define PRIMARY_REALM "ATHENA.MIT.EDU"

#define SUBSTITUTE  1
#define REPLACE     2

#define USERS         0
#define GROUPS        1

#define MEMBER_ADD          1
#define MEMBER_REMOVE       2
#define MEMBER_CHANGE_NAME  3
#define MEMBER_ACTIVATE     4
#define MEMBER_DEACTIVATE   5
#define MEMBER_CREATE       6

#define MOIRA_ALL       0x0
#define MOIRA_USERS     0x1
#define MOIRA_KERBEROS  0x2
#define MOIRA_STRINGS   0x4
#define MOIRA_LISTS     0x8

#define ADFS_ADD    1
#define ADFS_DELETE 2

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

#define STOP_FILE "/moira/winad/nowinad"
#define file_exists(file) (access((file), F_OK) == 0)

#define LDAP_BERVAL struct berval
#define MAX_SERVER_NAMES 32

#define ADD_ATTR(t, v, o) 		\
  mods[n] = malloc(sizeof(LDAPMod));	\
  mods[n]->mod_op = o;	\
  mods[n]->mod_type = t; 		\
  mods[n++]->mod_values = v

LK_ENTRY *member_base = NULL;
LK_ENTRY *sid_base = NULL;
LK_ENTRY **sid_ptr = NULL;
static char tbl_buf[1024];
char  kerberos_ou[] = "OU=kerberos, OU=moira";
char  contact_ou[] = "OU=strings, OU=moira";
char  user_ou[] = "OU=users, OU=moira";
char  group_ou_distribution[] = "OU=mail, OU=lists, OU=moira";
char  group_ou_root[] = "OU=lists, OU=moira";
char  group_ou_security[] = "OU=group, OU=lists, OU=moira";
char  group_ou_neither[] = "OU=special, OU=lists, OU=moira";
char  group_ou_both[] = "OU=mail, OU=group, OU=lists, OU=moira";
char *whoami;
char group_manager[64];
char ldap_domain[256];
char list_type[32];
char GroupType[2];
int  maillist_flag;
int  group_flag;
int  mr_connections = 0;
int  callback_rc;
char default_server[256];

extern int set_password(char *user, char *password, char *domain);

void AfsToWinAfs(char* path, char* winPath);
int ad_connect(LDAP **ldap_handle, char *ldap_domain, char *dn_path, 
               char *Win2kPassword, char *Win2kUser, char *default_server,
               int connect_to_kdc);
void ad_kdc_disconnect();
void check_winad(void);
void expand_groups(LDAP *ldap_handle, char *dn_path, char *group_name);
int filesys_process(int ac, char **av, void *ptr);
int user_create(int ac, char **av, void *ptr);
int user_change_status(int ac, char **av, void *ptr);
int user_delete(LDAP *ldap_handle, char *dn_path, char *u_name);
int user_rename(int ac, char **av, void *ptr);
int user_update(int ac, char **av, void *ptr);
int contact_create(LDAP *ld, char *bind_path, char *user, char *group_ou);
int get_group_info(int ac, char**av, void *ptr);
int group_create(int ac, char **av, void *ptr);
int group_delete(int ac, char **av, void *ptr);
int group_ad_delete(LDAP *ldap_handle, char *dn_path, char *group_name);
int group_list_build(int ac, char **av, void *ptr);
int group_rename(int ac, char **av, void *ptr);
int list_list_build(int ac, char **av, void *ptr);
int member_list_build(int ac, char **av, void *ptr);
int member_list_process(LDAP *ldap_handle, char *dn_path, char *group_name, 
                        char *group_ou, char *group_membership, char *group_gid,
                        int operation);
int member_remove(LDAP *ldap_handle, char *dn_path, char *group_name, 
                  char *group_ou, char *group_membership, char *group_gid);
int sid_update(LDAP *ldap_handle, char *dn_path);
int check_string(char *s);
void convert_b_to_a(char *string, UCHAR *binary, int length);
int mr_connect_cl(char *server, char *client, int version, int auth);

void do_filesys(LDAP *ldap_handle, char *dn_path, char *ldap_hostname,
             char **before, int beforec, char **after, int afterc);
void do_list(LDAP *ldap_handle, char *dn_path, char *ldap_hostname,
             char **before, int beforec, char **after, int afterc);
void do_user(LDAP *ldap_handle, char *dn_path, char *ldap_hostname, 
             char **before, int beforec, char **after, int afterc);
void do_member(LDAP *ldap_handle, char *dn_path, char *ldap_hostname,
               char **before, int beforec, char **after, int afterc);
int linklist_create_entry(char *attribute, char *value,
                          LK_ENTRY **linklist_entry);
int linklist_build(LDAP *ldap_handle, char *dn_path, char *search_exp, 
                   char **attr_array, LK_ENTRY **linklist_base, 
                   int *linklist_count);
void linklist_free(LK_ENTRY *linklist_base);

int retrieve_attributes(LDAP *ldap_handle, LDAPMessage *ldap_entry, 
                        char *distinguished_name, LK_ENTRY **linklist_current);
int retrieve_entries(LDAP *ldap_handle, LDAPMessage *ldap_entry, 
                     LK_ENTRY **linklist_base, int *linklist_count);
int retrieve_values(LDAP *ldap_handle, LDAPMessage *ldap_entry, 
                    char *Attribute, char *distinguished_name, 
                    LK_ENTRY **linklist_current);

int construct_newvalues(LK_ENTRY *linklist_base, int modvalue_count, 
                        char *oldValue, char *newValue,
                        char ***modvalues, int type);
void free_values(char **modvalues);

int convert_domain_to_dn(char *domain, char **bind_path);
void get_distinguished_name(LDAP *ldap_handle, LDAPMessage *ldap_entry, 
                            char *distinguished_name);
int moira_disconnect(void);
int moira_connect(void);
void print_to_screen(const char *fmt, ...);

int main(int argc, char **argv)
{
  unsigned long   rc;
  int             beforec;
  int             afterc;
  int             i;
  char            *table;
  char            **before;
  char            **after;
  LDAP            *ldap_handle;
  FILE            *fptr;
  char            dn_path[256];

  whoami = ((whoami = (char *)strrchr(argv[0], '/')) ? whoami+1 : argv[0]);

  if (argc < 4)
    {
      com_err(whoami, 0, "%s", "argc < 4");
      exit(1);
    }
  beforec = atoi(argv[2]);
  afterc = atoi(argv[3]);

  if (argc < (4 + beforec + afterc))
    {
      com_err(whoami, 0, "%s", "argc < (4 + breforec + afterc)");
      exit(1);
    }

  table = argv[1];
  before = &argv[4];
  after = &argv[4 + beforec];

  strcpy(tbl_buf, table);
  strcat(tbl_buf, " (");
  for (i = 0; i < beforec; i++)
    {
      if (i > 0)
        strcat(tbl_buf, ",");
      strcat(tbl_buf, before[i]);
    }
  strcat(tbl_buf, ")->(");
  for (i = 0; i < afterc; i++)
    {
      if (i > 0)
        strcat(tbl_buf, ",");
      strcat(tbl_buf, after[i]);
    }
  strcat(tbl_buf, ")");
  check_winad();
  
  memset(ldap_domain, '\0', sizeof(ldap_domain));
  if ((fptr = fopen("/moira/winad/winad.cfg", "r")) != NULL)
    {
      fread(ldap_domain, sizeof(char), sizeof(ldap_domain), fptr);
      fclose(fptr);
    }
  if (strlen(ldap_domain) == 0)
    strcpy(ldap_domain, "win.mit.edu");
  initialize_sms_error_table();
  initialize_krb_error_table();

  memset(default_server, '\0', sizeof(default_server));
  memset(dn_path, '\0', sizeof(dn_path));
  if (ad_connect(&ldap_handle, ldap_domain, dn_path, "", "", default_server, 1))
    {
      com_err(whoami, 0, "%s %s", "cannot connect to any server in domain ",
              ldap_domain);
      exit(1);
    }

  for (i = 0; i < (int)strlen(table); i++)
    table[i] = tolower(table[i]);
  if (!strcmp(table, "users"))
    do_user(ldap_handle, dn_path, ldap_domain, before, beforec, after,
            afterc);
  else if (!strcmp(table, "list"))
    do_list(ldap_handle, dn_path, ldap_domain, before, beforec, after,
            afterc);
  else if (!strcmp(table, "imembers"))
    do_member(ldap_handle, dn_path, ldap_domain, before, beforec, after,
              afterc);
  else if (!strcmp(table, "filesys"))
    do_filesys(ldap_handle, dn_path, ldap_domain, before, beforec, after,
               afterc);
/*
  else if (!strcmp(table, "quota"))
    do_quota(before, beforec, after, afterc);
*/

  ad_kdc_disconnect();
  rc = ldap_unbind_s(ldap_handle);
  exit(0);
}

void do_filesys(LDAP *ldap_handle, char *dn_path, char *ldap_hostname,
             char **before, int beforec, char **after, int afterc)
{
  long  rc;
  char  *av[3];
  char  *call_args[7];
  int   acreate;
  int   atype;
  int   bcreate;
  int   btype;

  if (rc = moira_connect())
    {
      critical_alert("AD incremental",
                     "Error contacting Moira server : %s",
                     error_message(rc));
      return;
    }

  if (afterc < FS_CREATE)
    atype = acreate = 0;
  else
    {
      atype = !strcmp(after[FS_TYPE], "AFS");
      acreate = atoi(after[FS_CREATE]);
    }

  if (beforec < FS_CREATE)
    {
      if (acreate == 0 || atype == 0)
        goto cleanup;
      com_err(whoami, 0, "Processing filesys %s", after[FS_NAME]);
      av[0] = after[FS_NAME];
      call_args[0] = (char *)ldap_handle;
      call_args[1] = dn_path;
      call_args[2] = after[FS_NAME];
      call_args[3] = (char *)ADFS_ADD;
      if (rc = mr_query("get_filesys_by_label", 1, av, filesys_process, call_args))
        {
          critical_alert("AD incremental", "Couldn't process filesys %s : %s",
                          after[FS_NAME], error_message(rc));
          goto cleanup;
        }
      goto cleanup;
    }

  btype = !strcmp(before[FS_TYPE], "AFS");
  bcreate = atoi(before[FS_CREATE]);
  if (afterc < FS_CREATE)
    {
      if (btype && bcreate)
        {
          av[0] = before[FS_NAME];
          av[1] = before[FS_TYPE];
          call_args[0] = (char *)ldap_handle;
          call_args[1] = dn_path;
          call_args[2] = before[FS_NAME];
          call_args[3] = (char *)ADFS_DELETE;
          if (filesys_process(beforec, before, (void *)call_args))
            {
              critical_alert("AD incremental", "Couldn't delete filesys %s : %s",
                             before[FS_NAME], error_message(rc));
            }
        }
      goto cleanup;
    }

  if (!acreate)
    goto cleanup;

  if (!atype && !btype)
    {
      if (strcmp(before[FS_TYPE], "ERR") || strcmp(after[FS_TYPE], "ERR"))
        {
          critical_alert("incremental", "Filesystem %s or %s is not AFS: "
                         "Operation not supported", before[FS_NAME], after[FS_NAME]);
          goto cleanup;
        }
    }
  com_err(whoami, 0, "Processing filesys %s", after[FS_NAME]);
  av[0] = after[FS_NAME];
  call_args[0] = (char *)ldap_handle;
  call_args[1] = dn_path;
  call_args[2] = after[FS_NAME];
  call_args[3] = (char *)ADFS_ADD;
  if (rc = mr_query("get_filesys_by_label", 1, av, filesys_process, call_args))
    {
      critical_alert("AD incremental", "Couldn't process filesys %s : %s",
                     after[FS_NAME], error_message(rc));
      goto cleanup;
    }
cleanup:
  moira_disconnect();
  return;
}
void do_list(LDAP *ldap_handle, char *dn_path, char *ldap_hostname,
             char **before, int beforec, char **after, int afterc)
{
  int     ahide;
  int     bhide;
  int     apublic;
  int     bpublic;
  int     bgroup;
  int     agroup;
  int     amaillist;
  int     bmaillist;
  int     bstatus;
  int     astatus;
  long    rc;
  char    *av[3];
  char    *call_args[7];

  if (beforec == 0 && afterc == 0)
    return;

  astatus = bstatus = 0;
  ahide = bhide = 0;
  apublic = bpublic = 0;
  amaillist = bmaillist = 0;
  if (beforec != 0)
    {
      if (atoi(before[L_ACTIVE]))
        {
          bstatus = atoi(before[L_ACTIVE]);
          bhide = atoi(before[L_HIDDEN]);
          bpublic = atoi(before[L_PUBLIC]);
          bmaillist = atoi(before[L_MAILLIST]);
          bgroup = atoi(before[L_GROUP]);
        }
     }
  if (afterc != 0)
    {
      if (atoi(after[L_ACTIVE]))
        {
          astatus = atoi(after[L_ACTIVE]);
          ahide = atoi(after[L_HIDDEN]);
          apublic = atoi(after[L_PUBLIC]);
          amaillist = atoi(after[L_MAILLIST]);
          agroup = atoi(after[L_GROUP]);
        }
    }

  if (rc = moira_connect())
    {
      critical_alert("AD incremental",
                     "Error contacting Moira server : %s",
                     error_message(rc));
      return;
    }

  if (astatus && bstatus)
    {
      if ((bmaillist == amaillist) && (bgroup == agroup) &&
          (!strcmp(before[L_NAME], after[L_NAME])))
        return;
      com_err(whoami, 0, "Changing group %s to %s",
              before[L_NAME], after[L_NAME]);

      av[0] = after[L_NAME];
      call_args[0] = (char *)ldap_handle;
      call_args[1] = dn_path;
      call_args[2] = before[L_NAME];
      call_args[3] = before[L_MAILLIST];
      call_args[4] = before[L_GROUP];
      call_args[5] = NULL;
      callback_rc = 0;
      if (rc = mr_query("get_list_info", 1, av, group_rename, call_args))
        {
           if (callback_rc != LDAP_NO_SUCH_OBJECT)
             {
               critical_alert("AD incremental",
                              "Could not change list %s to %s : %s",
                              before[L_NAME], 
                              after[L_NAME], error_message(rc));
               goto cleanup;
             }
           callback_rc = LDAP_NO_SUCH_OBJECT;
         }
      if (callback_rc != LDAP_NO_SUCH_OBJECT)
        goto cleanup;
      bstatus = 0;
    }
  if (bstatus)
    {
      com_err(whoami, 0, "Deleting group %s", before[L_NAME]);
      rc = group_ad_delete(ldap_handle, dn_path, before[L_NAME]);
      goto cleanup;
    }
  if (astatus)
    {
      com_err(whoami, 0, "Creating group %s", after[L_NAME]);

      sleep(1);
      av[0] = after[L_NAME];
      call_args[0] = (char *)ldap_handle;
      call_args[1] = dn_path;
      call_args[2] = after[L_NAME];
      call_args[3] = NULL;
      call_args[4] = NULL;
      call_args[5] = NULL;
      call_args[6] = (char *)(MOIRA_USERS | MOIRA_KERBEROS | MOIRA_STRINGS);
      sid_base = NULL;
      sid_ptr = &sid_base;
      if (rc = mr_query("get_list_info", 1, av, group_create, call_args))
        {
          critical_alert("AD incremental", "Couldn't create list %s : %s",
                         after[L_NAME], error_message(rc));
            goto cleanup;
        }
      if (sid_base != NULL)
        {
          sid_update(ldap_handle, dn_path);
          linklist_free(sid_base);
          sid_base = NULL;
        }

      if (afterc == 0)
        goto cleanup;
      sleep(1);

      member_base = NULL;
      if (!(rc = mr_query("get_end_members_of_list", 1, av, member_list_build,
                          call_args)))
        {
          if (member_base != NULL)
            {
              rc = member_list_process(ldap_handle, dn_path, after[L_NAME],
                                       call_args[3], call_args[4], call_args[5],
                                       MOIRA_USERS | MOIRA_KERBEROS | MOIRA_STRINGS);
              expand_groups(ldap_handle, dn_path, after[L_NAME]);
            }
        }
      else
        {
          critical_alert("AD incremental",
                         "Error contacting Moira server to resolve %s : %s",
                         after[L_NAME], error_message(rc));
        }
      linklist_free(member_base);
      member_base = NULL;
      goto cleanup;
    }
cleanup:
  moira_disconnect();
}

#define LM_EXTRA_ACTIVE  (LM_END)

void do_member(LDAP *ldap_handle, char *dn_path, char *ldap_hostname,
               char **before, int beforec, char **after, int afterc)
{
  char  *call_args[7];
  char  *av[2];
  char  group_name[128];
  char  user_name[128];
  char  user_type[128];
  int   rc;

  if (afterc)
    {
      if (!atoi(after[LM_EXTRA_ACTIVE]))
        return;
      strcpy(user_name, after[LM_MEMBER]);
      strcpy(group_name, after[LM_LIST]);
      strcpy(user_type, after[LM_TYPE]);

    }
  else if (beforec)
    {
      if (!atoi(before[LM_EXTRA_ACTIVE]))
          return;
      strcpy(user_name, before[LM_MEMBER]);
      strcpy(group_name, before[LM_LIST]);
      strcpy(user_type, before[LM_TYPE]);
    }

  if (rc = moira_connect())
    {
      critical_alert("AD incremental", 
                     "Moira error retrieving grouplist of user %s : %s",
                     user_name, error_message(rc));
      return;
    }
  com_err(whoami, 0, "Updating list %s membership for user %s.", group_name,
          user_name);
  av[0] = group_name;
  call_args[0] = (char *)ldap_handle;
  call_args[1] = dn_path;
  call_args[2] = group_name;
  call_args[3] = NULL;
  call_args[4] = NULL;
  call_args[5] = NULL;
  call_args[6] = (char *)(MOIRA_USERS | MOIRA_KERBEROS | MOIRA_STRINGS);
  member_base = NULL;
  sid_base = NULL;
  sid_ptr = &sid_base;
  if (!(rc = mr_query("get_list_info", 1, av, group_create, call_args)))
    {
      if (sid_base != NULL)
        {
          sid_update(ldap_handle, dn_path);
          linklist_free(sid_base);
          sid_base = NULL;
        }
      member_base = NULL;
      sleep(1);

      if (!(rc = mr_query("get_end_members_of_list", 1, av, member_list_build,
                          call_args)))
        {
          if (member_base == NULL)
            {
              member_remove(ldap_handle, dn_path, group_name,
                            call_args[3], call_args[4], call_args[5]);
            }
          else
            {
              rc = member_list_process(ldap_handle, dn_path, group_name,
                                       call_args[3], call_args[4], call_args[5],
                                       MOIRA_USERS | MOIRA_KERBEROS | MOIRA_STRINGS);
              expand_groups(ldap_handle, dn_path, group_name);
            }
        }
    }
  if (rc)
    {
      if (afterc)
        critical_alert("AD incremental", "Couldn't add %s to group %s ",
                       user_name, group_name);
      else
        critical_alert("AD incremental", "Couldn't remove %s from group %s ",
                       user_name, group_name);
    }
  linklist_free(member_base);
  member_base = NULL;
  if (call_args[3] != NULL)
    free(call_args[3]);
  if (call_args[4] != NULL)
    free(call_args[4]);
  moira_disconnect();
}


void do_user(LDAP *ldap_handle, char *dn_path, char *ldap_hostname, 
             char **before, int beforec, char **after, 
             int afterc)
{
  int   rc;
  char  *av[2];
  char  *call_args[6];
  int   astate;
  int   bstate;

  if ((beforec == 0) || (afterc == 0))
    return;

  astate = 0;
  bstate = 0;
  if (afterc > U_STATE)
    astate = atoi(after[U_STATE]);
  if (beforec > U_STATE)
    bstate = atoi(before[U_STATE]);

  if (astate == 2)
    astate = 1;
  if (bstate == 2)
    bstate = 1;

  if ((bstate == 0) && (astate == 0))
    return;

  if (rc = moira_connect())
    {
      critical_alert("AD incremental", 
                     "Error connection to Moira : %s",
                     error_message(rc));
      return;
    }

  if (astate == bstate)
    {
      if (!strcmp(before[U_NAME], after[U_NAME]))
        {
          com_err(whoami, 0, "Updating user %s info", before[U_NAME]);
          av[0] = before[U_NAME];
          call_args[0] = (char *)ldap_handle;
          call_args[1] = dn_path;
          sid_base = NULL;
          sid_ptr = &sid_base;
          callback_rc = 0;
          if (rc = mr_query("get_user_account_by_login", 1, av, user_update,
                            call_args))
            {
              if (callback_rc != LDAP_NO_SUCH_OBJECT)
                {
                  critical_alert("AD incremental",
                                 "Could not update user %s info : %s",
                                 before[U_NAME], 
                                 error_message(rc));
                  goto cleanup;
                }
            }
        }
      else
        {
          com_err(whoami, 0, "Changing user %s to %s", before[U_NAME],
                  after[U_NAME]);
          av[0] = after[U_NAME];
          call_args[0] = (char *)ldap_handle;
          call_args[1] = dn_path;
          call_args[2] = (char *)MEMBER_ACTIVATE;
          call_args[3] = before[U_NAME];
          sid_base = NULL;
          sid_ptr = &sid_base;
          callback_rc = 0;
          if (rc = mr_query("get_user_account_by_login", 1, av, user_rename,
                            call_args))
            {
              if (callback_rc != LDAP_NO_SUCH_OBJECT)
                {
                  critical_alert("AD incremental",
                                 "Could not change user %s to %s : %s",
                                 before[U_NAME], 
                                 after[U_NAME], error_message(rc));
                  goto cleanup;
                }
            }
        }
      if (callback_rc != LDAP_NO_SUCH_OBJECT)
        goto cleanup;
      bstate = 0;
    }
  if (bstate == 1)
    {
      com_err(whoami, 0, "Deactivate user %s in the AD", before[U_NAME]);
      av[0] = before[U_NAME];
      call_args[0] = (char *)ldap_handle;
      call_args[1] = dn_path;
      call_args[2] = (char *)MEMBER_DEACTIVATE;
      if (rc = mr_query("get_user_account_by_login", 1, av, user_change_status,
                        call_args))
        {
          critical_alert("AD incremental",
                         "Couldn't deactivate user %s in the AD : %s",
                         before[U_NAME], error_message(rc));
        }
      goto cleanup;
    }
  if (astate == 1)
    {
      com_err(whoami, 0, "%s user %s", "Creating/Reactivating",
              after[U_NAME]);

      av[0] = after[U_NAME];
      call_args[0] = (char *)ldap_handle;
      call_args[1] = dn_path;
      call_args[2] = (char *)MEMBER_ACTIVATE;
      call_args[3] = NULL;
      sid_base = NULL;
      sid_ptr = &sid_base;
      if (rc = mr_query("get_user_account_by_login", 1, av, user_create,
                        call_args))
        {
          critical_alert("AD incremental", "Couldn't create/activate user %s : %s",
                         after[U_NAME], error_message(rc));
          goto cleanup;
        }
      if (sid_base != NULL)
        {
          sid_update(ldap_handle, dn_path);
          linklist_free(sid_base);
        }
    }
cleanup:
  moira_disconnect();
}

int construct_newvalues(LK_ENTRY *linklist_base, int modvalue_count, 
                        char *oldValue, char *newValue,
                        char ***modvalues, int type)
{
  LK_ENTRY    *linklist_ptr;
  int         i;
  char        *cPtr;

  if (((*modvalues) = calloc(1, (modvalue_count + 1) * sizeof(char *)))
      == NULL)
    {
      return(1);
    }
  for (i = 0; i < (modvalue_count + 1); i++)
    (*modvalues)[i] = NULL;
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
                      if (((*modvalues)[i] = calloc(1, strlen(newValue) + 1))
                          == NULL)
                        return(1);
                      memset((*modvalues)[i], '\0', strlen(newValue) + 1);
                      strcpy((*modvalues)[i], newValue);
                    }
                  else
                    {
                      if (((*modvalues)[i] = calloc(1, 
                                        (int)(cPtr - linklist_ptr->value) + 
                                        (linklist_ptr->length - strlen(oldValue)) + 
                                        strlen(newValue) + 1)) == NULL)
                        return(1);
                      memset((*modvalues)[i], '\0', 
                             (int)(cPtr - linklist_ptr->value) + 
                             (linklist_ptr->length - strlen(oldValue)) + 
                             strlen(newValue) + 1);
                      memcpy((*modvalues)[i], linklist_ptr->value, 
                             (int)(cPtr - linklist_ptr->value));
                      strcat((*modvalues)[i], newValue);
                      strcat((*modvalues)[i], 
     &linklist_ptr->value[(int)(cPtr - linklist_ptr->value) + strlen(oldValue)]);
                    }
                }
              else
                {
                  (*modvalues)[i] = calloc(1, linklist_ptr->length + 1);
                  memset((*modvalues)[i], '\0', linklist_ptr->length + 1);
                  memcpy((*modvalues)[i], linklist_ptr->value,
                         linklist_ptr->length);
                }
            }
        else
            {
              (*modvalues)[i] = calloc(1, linklist_ptr->length + 1);
              memset((*modvalues)[i], '\0', linklist_ptr->length + 1);
              memcpy((*modvalues)[i], linklist_ptr->value,
                     linklist_ptr->length);
            }
          linklist_ptr = linklist_ptr->next;
        }
      (*modvalues)[i] = NULL;
    }
  return(0);
}


int linklist_build(LDAP *ldap_handle, char *dn_path, char *search_exp, 
                   char **attr_array, LK_ENTRY **linklist_base,
                   int *linklist_count)
{
  ULONG       rc;
  LDAPMessage *ldap_entry;

  rc = 0;
  ldap_entry = NULL;
  (*linklist_base) = NULL;
  (*linklist_count) = 0;
  if ((rc = ldap_search_s(ldap_handle, dn_path, LDAP_SCOPE_SUBTREE, 
                          search_exp, attr_array, 0, &ldap_entry))
      != LDAP_SUCCESS)
    return(0);
  rc = retrieve_entries(ldap_handle, ldap_entry, linklist_base, linklist_count);

  ldap_msgfree(ldap_entry);
  return(rc);
}


int retrieve_entries(LDAP *ldap_handle, LDAPMessage *ldap_entry, 
                     LK_ENTRY **linklist_base, int *linklist_count)
{
  char        distinguished_name[1024];
  LK_ENTRY    *linklist_ptr;
  int         rc;

  if ((ldap_entry = ldap_first_entry(ldap_handle, ldap_entry)) == NULL)
    return(0);

  memset(distinguished_name, '\0', sizeof(distinguished_name));
  get_distinguished_name(ldap_handle, ldap_entry, distinguished_name);

  if ((rc = retrieve_attributes(ldap_handle, ldap_entry, distinguished_name,
                                linklist_base)) != 0)
    return(rc);

  while ((ldap_entry = ldap_next_entry(ldap_handle, ldap_entry)) != NULL)
    {
      memset(distinguished_name, '\0', sizeof(distinguished_name));
      get_distinguished_name(ldap_handle, ldap_entry, distinguished_name);

      if ((rc = retrieve_attributes(ldap_handle, ldap_entry, distinguished_name,
                                    linklist_base)) != 0)
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
  char        *Attribute;
  BerElement  *ptr;

  ptr = NULL;
  if ((Attribute = ldap_first_attribute(ldap_handle, ldap_entry, &ptr)) != NULL)
    {
      retrieve_values(ldap_handle, ldap_entry, Attribute, distinguished_name,
                      linklist_current);
      ldap_memfree(Attribute);
      while ((Attribute = ldap_next_attribute(ldap_handle, ldap_entry, 
                                              ptr)) != NULL)
        {
          retrieve_values(ldap_handle, ldap_entry, Attribute,
                          distinguished_name, linklist_current);
          ldap_memfree(Attribute);
        }
    }
  ldap_ber_free(ptr, 0);
  return(0);
}

int retrieve_values(LDAP *ldap_handle, LDAPMessage *ldap_entry,
                    char *Attribute, char *distinguished_name,
                    LK_ENTRY **linklist_current)
{
  char        **str_value;
  char        temp[256];
  void        **Ptr;
  int         use_bervalue;
  LK_ENTRY    *linklist_previous;
  LDAP_BERVAL **ber_value;
  DWORD       ber_length;
#ifdef LDAP_DEBUG
  SID         *sid;
  GUID        *guid;
  int         i;
  int         intValue;
  DWORD       *subauth;
  SID_IDENTIFIER_AUTHORITY    *sid_auth;
  unsigned char   *subauth_count;
#endif /*LDAP_BEGUG*/

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

          if (((*linklist_current)->attribute = calloc(1, 
                                               strlen(Attribute) + 1)) == NULL)
            return(1);
          memset((*linklist_current)->attribute, '\0', strlen(Attribute) + 1);
          strcpy((*linklist_current)->attribute, Attribute);
          if (use_bervalue)
            {
              ber_length = (*(LDAP_BERVAL **)Ptr)->bv_len;
              if (((*linklist_current)->value = calloc(1, ber_length)) == NULL)
                return(1);
              memset((*linklist_current)->value, '\0', ber_length);
              memcpy((*linklist_current)->value, (*(LDAP_BERVAL **)Ptr)->bv_val, 
                                                  ber_length);
              (*linklist_current)->length = ber_length;
            }
          else
            {
              if (((*linklist_current)->value = calloc(1, 
                                                  strlen(*Ptr) + 1)) == NULL)
                return(1);
              memset((*linklist_current)->value, '\0', strlen(*Ptr) + 1);
              (*linklist_current)->length = strlen(*Ptr);
              strcpy((*linklist_current)->value, *Ptr);
            }
          (*linklist_current)->ber_value = use_bervalue;
          if (((*linklist_current)->dn = calloc(1, 
                                      strlen(distinguished_name) + 1)) == NULL)
            return(1);
          memset((*linklist_current)->dn, '\0', strlen(distinguished_name) + 1);
          strcpy((*linklist_current)->dn, distinguished_name);

#ifdef LDAP_DEBUG
          if (!strcmp(Attribute, "objectGUID"))
            {
              guid = (GUID *)((*linklist_current)->value);
              sprintf(temp, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", 
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
                    print_to_screen("     %20s :    %s\n", 
                                    "", "Account disabled");
                  else
                    print_to_screen("     %20s :    %s\n", 
                                    "", "Account active");
                  if (intValue & UF_HOMEDIR_REQUIRED)
                    print_to_screen("     %20s :    %s\n", 
                                    "", "Home directory required");
                  if (intValue & UF_LOCKOUT)
                    print_to_screen("     %20s :    %s\n", 
                                    "", "Account locked out");
                  if (intValue & UF_PASSWD_NOTREQD)
                    print_to_screen("     %20s :    %s\n", 
                                    "", "No password required");
                  if (intValue & UF_PASSWD_CANT_CHANGE)
                    print_to_screen("     %20s :    %s\n",
                                    "", "Cannot change password");
                  if (intValue & UF_TEMP_DUPLICATE_ACCOUNT)
                    print_to_screen("     %20s :    %s\n", 
                                    "", "Temp duplicate account");
                  if (intValue & UF_NORMAL_ACCOUNT)
                    print_to_screen("     %20s :    %s\n", 
                                    "", "Normal account");
                  if (intValue & UF_INTERDOMAIN_TRUST_ACCOUNT)
                    print_to_screen("     %20s :    %s\n", 
                                    "", "Interdomain trust account");
                  if (intValue & UF_WORKSTATION_TRUST_ACCOUNT)
                    print_to_screen("     %20s :    %s\n", 
                                    "", "Workstation trust account");
                  if (intValue & UF_SERVER_TRUST_ACCOUNT)
                    print_to_screen("     %20s :    %s\n", 
                                    "", "Server trust account");
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

int moira_connect(void)
{
  long    rc;
  char    HostName[64];

  if (!mr_connections++)
    {
#ifdef _WIN32
      memset(HostName, '\0', sizeof(HostName));
      strcpy(HostName, "ttsp");
      rc = mr_connect_cl(HostName, "winad.incr", QUERY_VERSION, 1);
/*det
      rc = mr_connect(HostName);
*/
#else
      struct utsname uts;
      uname(&uts);
      rc = mr_connect_cl(uts.nodename, "winad.incr", QUERY_VERSION, 1);
/*
      rc = mr_connect(uts.nodename);
*/
#endif /*WIN32*/
/*det
      if (!rc)
        rc = mr_auth("winad.incr");
*/
      return rc;
    }
  return 0;
}

void check_winad(void)
{
  int i;
  
  for (i = 0; file_exists(STOP_FILE); i++)
    {
      if (i > 30)
        {
          critical_alert("incremental",
                         "WINAD incremental failed (%s exists): %s",
                         STOP_FILE, tbl_buf);
          exit(1);
        }
      sleep(60);
    }
}

int moira_disconnect(void)
{

  if (!--mr_connections)
    {
      mr_disconnect();
    }
  return 0;
}

void get_distinguished_name(LDAP *ldap_handle, LDAPMessage *ldap_entry, 
                            char *distinguished_name)
{
  char    *CName;

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

int get_group_membership(char *group_membership, char *group_ou, 
                         int *security_flag, char **av)
{
  int  maillist_flag;
  int  group_flag;

  maillist_flag = atoi(av[L_MAILLIST]);
  group_flag = atoi(av[L_GROUP]);
  if (security_flag != NULL)
    (*security_flag) = 0;

  if ((maillist_flag) && (group_flag))
    {
      if (group_membership != NULL)
        group_membership[0] = 'B';
      if (security_flag != NULL)
        (*security_flag) = 1;
      if (group_ou != NULL)
        strcpy(group_ou, group_ou_both);
    }
  else if ((!maillist_flag) && (group_flag))
    {
      if (group_membership != NULL)
        group_membership[0] = 'S';
      if (security_flag != NULL)
        (*security_flag) = 1;
      if (group_ou != NULL)
        strcpy(group_ou, group_ou_security);
    }
  else if ((maillist_flag) && (!group_flag))
    {
      if (group_membership != NULL)
        group_membership[0] = 'D';
      if (group_ou != NULL)
        strcpy(group_ou, group_ou_distribution);
    }
  else
    {
      if (group_membership != NULL)
        group_membership[0] = 'N';
      if (group_ou != NULL)
        strcpy(group_ou, group_ou_neither);
    }
  return(0);
}

int get_group_info(int ac, char**av, void *ptr)
{
  char **call_args;

  call_args = ptr;

  if (!atoi(av[L_ACTIVE]))
    return(0);
  if (ptr == NULL)
    {
      get_group_membership(GroupType, NULL, NULL, av);
    }
  else
    {
    call_args[5] = av[L_NAME];
    get_group_membership(call_args[4], call_args[3], NULL, av);
    }

  return(0);
}

int group_rename(int ac, char **av, void *ptr)
{
  LDAPMod   *mods[20];
  char      old_dn[512];
  char      new_dn[512];
  char      new_dn_path[512];
  char      group_ou[256];
  char      sam_name[256];
  char      group_membership[2];
  char      filter_exp[4096];
  char      *attr_array[3];
  char      *name_v[] = {NULL, NULL};
  char      *samAccountName_v[] = {NULL, NULL};
  int       n;
  int       i;
  int       rc;
  int       security_flag;
  LK_ENTRY  *group_base;
  int       group_count;
  char      **call_args;
  char      *maillist_flag = NULL;
  char      *group_flag = NULL;

  call_args = ptr;

  if (!check_string(call_args[2]))
    {
      callback_rc = LDAP_NO_SUCH_OBJECT;
      return(0);
    }
  if (!check_string(av[L_NAME]))
    {
      critical_alert("AD incremental - list rename",
                      "invalid LDAP list name %s",
                      av[L_NAME]);
      return(0);
    }

  memset(group_ou, 0, sizeof(group_ou));
  memset(group_membership, 0, sizeof(group_membership));
  security_flag = 0;

  maillist_flag = av[L_MAILLIST];
  group_flag = av[L_GROUP];
  av[L_MAILLIST] = call_args[3];
  av[L_GROUP] = call_args[4];
  get_group_membership(group_membership, NULL, NULL, av);
  av[L_MAILLIST] = maillist_flag;
  av[L_GROUP] = group_flag;

  sprintf(filter_exp, "(sAMAccountName=%s_zZx%c)", call_args[2], group_membership[0]);
  attr_array[0] = "distinguishedName";
  attr_array[1] = NULL;
  if ((rc = linklist_build((LDAP *)call_args[0], call_args[1], filter_exp, attr_array, 
                           &group_base, &group_count)) != 0)
    {
      critical_alert("AD incremental - list rename",
                      "LDAP server unable to get list %s dn : %s",
                      call_args[2], ldap_err2string(rc));
      return(0);
    }
  if (group_count != 1)
    {
      critical_alert("AD incremental - list rename",
                      "LDAP server unable to find list %s in AD.",
                      call_args[2]);
      callback_rc = LDAP_NO_SUCH_OBJECT;
      return(0);
    }
  strcpy(old_dn, group_base->value);
  linklist_free(group_base);
  group_base = NULL;
  group_count = 0;

  get_group_membership(group_membership, group_ou, &security_flag, av);
  sprintf(sam_name, "%s_zZx%c", av[L_NAME], group_membership[0]);
  sprintf(new_dn_path, "%s,%s", group_ou, call_args[1]);
  sprintf(new_dn, "cn=%s", av[L_NAME]);
  if ((rc = ldap_rename_s((LDAP *)call_args[0], old_dn, new_dn, new_dn_path,
                          TRUE, NULL, NULL)) != LDAP_SUCCESS)
    {
      critical_alert("AD incremental - list rename",
                     "Couldn't rename list from %s to %s : %s",
                     call_args[2], av[L_NAME], ldap_err2string(rc));
      return(0);
    }

  name_v[0] = av[L_NAME];
  samAccountName_v[0] = sam_name;
  n = 0;
  ADD_ATTR("displayName", name_v, LDAP_MOD_REPLACE);
  ADD_ATTR("sAMAccountName", samAccountName_v, LDAP_MOD_REPLACE);
  mods[n] = NULL;
  sprintf(new_dn, "cn=%s,%s,%s", av[L_NAME], group_ou, call_args[1]);
  if ((rc = ldap_modify_s((LDAP *)call_args[0], new_dn, mods)) != LDAP_SUCCESS)
    {
       critical_alert("AD incremental - list rename", 
                      "After renaming, couldn't modify list data for %s : %s",
                       av[L_NAME], ldap_err2string(rc));
    }
  for (i = 0; i < n; i++)
    free(mods[i]);
  return(0);
}

int group_create(int ac, char **av, void *ptr)
{
  LDAPMod *mods[20];
  char new_dn[256];
  char group_ou[256];
  char new_group_name[256];
  char sam_group_name[256];
  char cn_group_name[256];
  char *cn_v[] = {NULL, NULL};
  char *objectClass_v[] = {"top", "group", NULL};
  char info[256];
  char *samAccountName_v[] = {NULL, NULL};
  char *managedBy_v[] = {NULL, NULL};
  char *altSecurityIdentities_v[] = {NULL, NULL};
  char *name_v[] = {NULL, NULL};
  char *desc_v[] = {NULL, NULL};
  char *info_v[] = {NULL, NULL};
  char *groupTypeControl_v[] = {NULL, NULL};
  char groupTypeControlStr[80];
  char group_membership[1];
  int  i;
  int  security_flag;
  u_int groupTypeControl = ADS_GROUP_TYPE_GLOBAL_GROUP;
  int  n;
  int  rc;
  int  sid_count;
  char filter_exp[256];
  char *attr_array[3];
  char **call_args;

  call_args = ptr;

  if (!atoi(av[L_ACTIVE]))
    return(0);
  if (!check_string(av[L_NAME]))
    {
      critical_alert("AD incremental - list create",
                      "invalid LDAP list name %s",
                      av[L_NAME]);
      return(0);
    }
  memset(group_ou, 0, sizeof(group_ou));
  memset(group_membership, 0, sizeof(group_membership));
  security_flag = 0;
  get_group_membership(group_membership, group_ou, &security_flag, av);
  call_args[3] = strdup(group_ou);
  call_args[4] = strdup(group_membership);
  call_args[5] = strdup(av[L_NAME]);

  if (security_flag)
    groupTypeControl |= ADS_GROUP_TYPE_SECURITY_ENABLED;
  sprintf(groupTypeControlStr, "%ld", groupTypeControl);
  groupTypeControl_v[0] = groupTypeControlStr;

  strcpy(new_group_name, av[L_NAME]);
  strcpy(sam_group_name, av[L_NAME]);
  strcpy(cn_group_name, av[L_NAME]);
  sprintf(&sam_group_name[strlen(sam_group_name)], 
          "_zZx%c", group_membership[0]);

  samAccountName_v[0] = sam_group_name;
  name_v[0] = new_group_name;
  cn_v[0] = new_group_name;

  sprintf(new_dn, "cn=%s,%s,%s", new_group_name, group_ou, call_args[1]);
  n = 0;
  ADD_ATTR("cn", cn_v, LDAP_MOD_ADD);
  ADD_ATTR("objectClass", objectClass_v, LDAP_MOD_ADD);
  ADD_ATTR("sAMAccountName", samAccountName_v, LDAP_MOD_ADD);
  ADD_ATTR("displayName", name_v, LDAP_MOD_ADD);
  ADD_ATTR("name", name_v, LDAP_MOD_ADD);
  if (strlen(av[L_DESC]) != 0)
    {
      desc_v[0] = av[L_DESC];
      ADD_ATTR("description", desc_v, LDAP_MOD_ADD);
    }
  ADD_ATTR("groupType", groupTypeControl_v, LDAP_MOD_ADD);
  if (strlen(av[L_ACE_NAME]) != 0)
    {
      sprintf(info, "The Administrator of this list is the LIST: %s", av[L_ACE_NAME]);
      info_v[0] = info;
      ADD_ATTR("info", info_v, LDAP_MOD_ADD);
    }
  mods[n] = NULL;

  rc = ldap_add_ext_s((LDAP *)call_args[0], new_dn, mods, NULL, NULL);

  for (i = 0; i < n; i++)
    free(mods[i]);
  if ((rc != LDAP_SUCCESS) && (rc != LDAP_ALREADY_EXISTS))
    {
      critical_alert("AD incremental - list rename",
                      "Unable to create list %s in AD : %s",
                      av[L_NAME], ldap_err2string(rc));
      return(0);
    }
  sprintf(filter_exp, "(sAMAccountName=%s)", sam_group_name);
  attr_array[0] = "objectSid";
  attr_array[1] = NULL;
  sid_count = 0;
  if ((rc = linklist_build((LDAP *)call_args[0], call_args[1], filter_exp, attr_array, 
                           sid_ptr, &sid_count)) == LDAP_SUCCESS)
    {
      if (sid_count == 1)
        {
          (*sid_ptr)->member = strdup(av[L_NAME]);
          (*sid_ptr)->type = (char *)GROUPS;
          sid_ptr = &(*sid_ptr)->next;
        }
    }
  return(0);
}

int group_delete(int ac, char **av, void *ptr)
{
  LK_ENTRY  *group_base;
  char      **call_args;
  char      *attr_array[3];
  char      filter_exp[1024];
  char      group_membership[1];
  char      group_ou[256];
  char      sam_group_name[256];
  int       security_flag;
  int       group_count;
  int       rc;

  call_args = ptr;

  if (!check_string(av[L_NAME]))
    {
      critical_alert("AD incremental - list delete",
                      "invalid LDAP list name %s",
                      av[L_NAME]);
      return(0);
    }
  memset(group_ou, 0, sizeof(group_ou));
  memset(group_membership, 0, sizeof(group_membership));
  security_flag = 0;
  get_group_membership(group_membership, group_ou, &security_flag, av);

  group_count = 0;
  group_base = NULL;
  attr_array[0] = "distinguishedName";
  attr_array[1] = NULL;
  strcpy(sam_group_name, av[L_NAME]);
  sprintf(&sam_group_name[strlen(sam_group_name)], "_zZx%c", 
          group_membership[0]);
  sprintf(filter_exp, "(sAMAccountName=%s)", sam_group_name);
  if ((rc = linklist_build((LDAP *)call_args[0], call_args[1], filter_exp, 
                           attr_array, &group_base, &group_count)) != 0)
    goto cleanup;
  if (group_count == 1)
    {
      if ((rc = ldap_delete_s((LDAP *)call_args[0], group_base->value)) != LDAP_SUCCESS)
        {
          critical_alert("AD incremental - list delete",
                         "Couldn't delete group %s : %s",
                         av[L_NAME], ldap_err2string(rc));
        }
    }
  else
    {
      critical_alert("AD incremental - list delete",
                      "Unable to find list %s in AD.",
                      av[L_NAME]);
    }
cleanup:
  linklist_free(group_base);
  return(0);
}

int group_ad_delete(LDAP *ldap_handle, char *dn_path, char *group_name)
{
  LK_ENTRY  *group_base;
  char      *attr_array[3];
  char      filter_exp[1024];
  char      sam_group_name[256];
  char      temp[512];
  int       group_count;
  int       rc;

  if (!check_string(group_name))
    {
      critical_alert("AD incremental - list AD delete",
                      "invalid LDAP list name %s",
                      group_name);
      return(0);
    }
  rc = 1;
  group_count = 0;
  group_base = NULL;
  attr_array[0] = "distinguishedName";
  attr_array[1] = NULL;
  strcpy(sam_group_name, group_name);
  sprintf(temp, "%s,%s", group_ou_root, dn_path);
  sprintf(filter_exp, "(sAMAccountName=%s_zZx*)", sam_group_name);
  if (linklist_build(ldap_handle, temp, filter_exp, attr_array, 
                           &group_base, &group_count) != 0)
    goto cleanup;
  if (group_count == 1)
    {
      if ((rc = ldap_delete_s(ldap_handle, group_base->value)) != LDAP_SUCCESS)
        {
          critical_alert("AD incremental - list AD delete",
                          "Unable to delete list %s from AD : %s",
                          group_name, ldap_err2string(rc));
          return(0);
        }
    }
  else
    {
      critical_alert("AD incremental - list AD delete",
                     "Unable to find list %s in AD.",
                      group_name);
    }
cleanup:
  linklist_free(group_base);
  return(0);
}

int group_list_build(int ac, char **av, void *ptr)
{
  LK_ENTRY    *linklist;
  char        **call_args;

  call_args = ptr;

  if (!atoi(av[L_ACTIVE]))
    return(0);
  if (!check_string(av[L_NAME]))
    return(0);
  linklist = calloc(1, sizeof(LK_ENTRY));
  if (!linklist)
    {
      critical_alert("AD incremental", "Out of memory");
      exit(1);
    }
  memset(linklist, '\0', sizeof(LK_ENTRY));
  linklist->op = 1;
  linklist->dn = NULL;
  linklist->list = calloc(1, strlen(av[L_NAME]) + 1);
  strcpy(linklist->list, av[L_NAME]);
  linklist->type = calloc(1, strlen("USER") + 1);
  strcpy(linklist->type, "USER");
  linklist->member = calloc(1, strlen(call_args[0]) + 1);
  strcpy(linklist->member, call_args[0]);
  linklist->next = member_base;
  member_base = linklist;
  return(0);
}

int member_list_build(int ac, char **av, void *ptr)
{
  LK_ENTRY  *linklist;
  char      temp[1024];
  char      **call_args;

  call_args = ptr;

  strcpy(temp, av[ACE_NAME]);
  if (!check_string(temp))
    return(0);
  if (!strcmp(av[ACE_TYPE], "USER"))
    {
      if (!((int)call_args[6] & MOIRA_USERS))
        return(0);
    }
  else if (!strcmp(av[ACE_TYPE], "STRING"))
    {
      if (!((int)call_args[6] & MOIRA_STRINGS))
        return(0);
      if (contact_create((LDAP *)call_args[0], call_args[1], temp, contact_ou))
        return(0);
    }
  else if (!strcmp(av[ACE_TYPE], "LIST"))
    {
      if (!((int)call_args[6] & MOIRA_LISTS))
        return(0);
    }
  else if (!strcmp(av[ACE_TYPE], "KERBEROS"))
    {
      if (!((int)call_args[6] & MOIRA_KERBEROS))
        return(0);
      if (contact_create((LDAP *)call_args[0], call_args[1], temp, kerberos_ou))
        return(0);
    }
  else
    return(0);

  linklist = member_base;
  while (linklist)
    {
    if (!strcasecmp(temp, linklist->member))
      return(0);
    linklist = linklist->next;
    }
  linklist = calloc(1, sizeof(LK_ENTRY));
  linklist->op = 1;
  linklist->dn = NULL;
  linklist->list = calloc(1, strlen(call_args[2]) + 1);
  strcpy(linklist->list, call_args[2]);
  linklist->type = calloc(1, strlen(av[ACE_TYPE]) + 1);
  strcpy(linklist->type, av[ACE_TYPE]);
  linklist->member = calloc(1, strlen(temp) + 1);
  strcpy(linklist->member, temp);
  linklist->next = member_base;
  member_base = linklist;
  return(0);
}

int list_list_build(int ac, char **av, void *ptr)
{
  LK_ENTRY  *linklist;
  char      temp[1024];
  char      **call_args;

  call_args = ptr;

  strcpy(temp, av[L_NAME]);
  if (!check_string(temp))
    return(0);

  linklist = member_base;
  while (linklist)
    {
    if (!strcasecmp(temp, linklist->member))
      return(0);
    linklist = linklist->next;
    }
  linklist = calloc(1, sizeof(LK_ENTRY));
  linklist->op = 1;
  linklist->dn = NULL;
  linklist->list = calloc(1, strlen(call_args[2]) + 1);
  strcpy(linklist->list, call_args[2]);
  linklist->type = calloc(1, strlen("LIST") + 1);
  strcpy(linklist->type, "LIST");
  linklist->member = calloc(1, strlen(temp) + 1);
  strcpy(linklist->member, temp);
  linklist->next = member_base;
  member_base = linklist;
  return(0);
}

int member_remove(LDAP *ldap_handle, char *dn_path, char *group_name, 
                  char *group_ou, char *group_membership, char *group_gid)
{
  char        distinguished_name[1024];
  char        **modvalues;
  char        filter_exp[4096];
  char        *attr_array[3];
  char        temp[256];
  int         group_count;
  int         i;
  int         n;
  LDAPMod     *mods[20];
  LK_ENTRY    *group_base;
  ULONG       rc;

  if (!check_string(group_name))
    return(0);
  strcpy(temp, group_name);
  sprintf(filter_exp, "(sAMAccountName=%s_zZx%c)", group_gid, group_membership[0]);
  attr_array[0] = "distinguishedName";
  attr_array[1] = NULL;
  if ((rc = linklist_build(ldap_handle, dn_path, filter_exp, attr_array, 
                           &group_base, &group_count)) != 0)
    {
      critical_alert("AD incremental - member remove",
                      "LDAP server unable to get list %s info : %s",
                      group_name, ldap_err2string(rc));
      goto cleanup;
    }
  if (group_count != 1)
    {
      critical_alert("AD incremental - member remove",
                      "LDAP server unable to find list %s in AD.",
                      group_name);
      goto cleanup;
    }
  strcpy(distinguished_name, group_base->value);
  linklist_free(group_base);
  group_base = NULL;
  group_count = 0;
  attr_array[0] = "member";
  attr_array[1] = NULL;
  if ((rc = linklist_build(ldap_handle, distinguished_name, filter_exp, attr_array, 
                           &group_base, &group_count)) != 0)
    {
      critical_alert("AD incremental - member remove",
                      "LDAP server unable to get list %s info : %s",
                      group_name, ldap_err2string(rc));
      goto cleanup;
    }

  modvalues = NULL;
  if (group_count != 0)
    {
      if ((rc = construct_newvalues(group_base, group_count, NULL, NULL,
                                    &modvalues, REPLACE)) == 1)
          goto cleanup;
      n = 0;
      ADD_ATTR("member", modvalues, LDAP_MOD_DELETE);
      mods[n] = NULL;
      rc = ldap_modify_s(ldap_handle, distinguished_name, mods);
      for (i = 0; i < n; i++)
        free(mods[i]);
      if (rc != LDAP_SUCCESS)
        {
          critical_alert("AD incremental - member remove",
                          "LDAP server unable to modify list %s members : %s",
                          group_name, ldap_err2string(rc));
          goto cleanup;
        }
      linklist_free(group_base);
      group_count = 0;
      group_base = NULL;
    }

cleanup:
  free_values(modvalues);
  linklist_free(group_base);
  return(rc);
}

#define USER_COUNT  5

int member_list_process(LDAP *ldap_handle, char *dn_path, char *group_name, 
                        char *group_ou, char *group_membership, char *group_gid,
                        int operation)
{
  char        distinguished_name[1024];
  char        **modvalues;
  char        filter_exp[4096];
  char        *attr_array[3];
  char        temp[256];
  char        group_member[256];
  char        *args[2];
  int         group_count;
  int         new_list_count;
  int         i;
  int         j;
  int         k;
  int         n;
  int         filter_count;
  LDAPMod     *mods[20];
  LK_ENTRY    *group_base;
  LK_ENTRY    *new_list;
  LK_ENTRY    *sPtr;
  LK_ENTRY    *pPtr;
  ULONG       rc;

  rc = 0;
  group_base = NULL;
  group_count = 0;
  modvalues = NULL;

  pPtr = member_base;
  while (pPtr)
    {
    ++group_count;
    pPtr = pPtr->next;
    }
  j = group_count/USER_COUNT;
  ++j;

  if (!check_string(group_name))
    return(0);
  strcpy(temp, group_name);
  sprintf(filter_exp, "(sAMAccountName=%s_zZx%c)", group_gid, group_membership[0]);
  attr_array[0] = "distinguishedName";
  attr_array[1] = NULL;
  if ((rc = linklist_build(ldap_handle, dn_path, filter_exp, attr_array, 
                           &group_base, &group_count)) != 0)
    {
      critical_alert("AD incremental - member list process",
                      "LDAP server unable to get list %s info : %s",
                      group_name, ldap_err2string(rc));
      goto cleanup;
    }
  if (group_count != 1)
    {
      critical_alert("AD incremental - member list process",
                      "LDAP server unable to find list %s in AD.",
                      group_name);
      goto cleanup;
    }
  strcpy(distinguished_name, group_base->value);
  linklist_free(group_base);
  group_base = NULL;
  group_count = 0;

  pPtr = member_base;
  for (i = 0; i < j; i++)
    {
      if (pPtr == NULL)
        break;
      memset(filter_exp, 0, sizeof(filter_exp));
      strcpy(filter_exp, "(|");
      filter_count = 0;
      for (k = 0; k < USER_COUNT; k++)
        {
          strcpy(group_member, pPtr->member);
          if (!check_string(group_member))
            {
              pPtr = pPtr->next;
              if (pPtr == NULL)
                break;
              continue;
            }
          if (!strcmp(pPtr->type, "LIST"))
            {
              if (!(operation & MOIRA_LISTS))
                continue;
              args[0] = pPtr->member;
              rc = mr_query("get_list_info", 1, args, get_group_info, NULL);
              sprintf(temp, "(sAMAccountName=%s_zZx%c)", group_member, GroupType[0]);
            }
          else if (!strcmp(pPtr->type, "USER"))
            {
              if (!(operation & MOIRA_USERS))
                continue;
              sprintf(temp, "(distinguishedName=cn=%s,%s,%s)", group_member, user_ou, dn_path);
            }
          else if (!strcmp(pPtr->type, "STRING"))
            {
              if (!(operation & MOIRA_STRINGS))
                continue;
              if ((group_membership[0] != 'B') && (group_membership[0] != 'D'))
                continue;
              sprintf(temp, "(distinguishedName=cn=%s,%s,%s)", group_member, contact_ou, dn_path);
            }
          else
            {
              if (!(operation & MOIRA_KERBEROS))
                continue;
              sprintf(temp, "(distinguishedName=cn=%s,%s,%s)", group_member, kerberos_ou, dn_path);
            }
          strcat(filter_exp, temp);
          ++filter_count;
          pPtr = pPtr->next;
          if (pPtr == NULL)
            break;
        }
      if (filter_count == 0)
        continue;
      strcat(filter_exp, ")");
      attr_array[0] = "distinguishedName";
      attr_array[1] = NULL;
      new_list = NULL;
      new_list_count = 0;
      if ((rc = linklist_build(ldap_handle, dn_path, filter_exp, attr_array, 
                               &new_list, &new_list_count)) != 0)
        {
          critical_alert("AD incremental - member list process",
                         "LDAP server unable to get list %s members from AD : %s",
                         group_name, ldap_err2string(rc));
          goto cleanup;
        }
      group_count += new_list_count;
      if (group_base == NULL)
        group_base = new_list;
      else
        {
          sPtr = group_base;
          while (sPtr)
            {
              if (sPtr->next != NULL)
                {
                  sPtr = sPtr->next;
                  continue;
                }
              sPtr->next = new_list;
              break;
            }
        }
    }

  modvalues = NULL;
  if (group_count != 0)
    {
      if ((rc = construct_newvalues(group_base, group_count, NULL, NULL,
                                    &modvalues, REPLACE)) == 1)
          goto cleanup;
      n = 0;
      ADD_ATTR("member", modvalues, LDAP_MOD_ADD);
      mods[n] = NULL;
      if ((rc = ldap_modify_s(ldap_handle, distinguished_name, mods)) 
          != LDAP_SUCCESS)
        {
          mods[0]->mod_op = LDAP_MOD_REPLACE;
          rc = ldap_modify_s(ldap_handle, distinguished_name, mods);
        }
      if (rc == LDAP_ALREADY_EXISTS)
        rc = LDAP_SUCCESS;
      for (i = 0; i < n; i++)
        free(mods[i]);
      linklist_free(group_base);
      group_count = 0;
      group_base = NULL;
      if (rc != LDAP_SUCCESS)
        {
          critical_alert("AD incremental - member list process",
                         "LDAP server unable to modify list %s members in AD : %s",
                         group_name, ldap_err2string(rc));
          goto cleanup;
        }
    }

cleanup:
  free_values(modvalues);
  linklist_free(group_base);
  return(0);
}

int contact_create(LDAP *ld, char *bind_path, char *user, char *group_ou)
{
  LDAPMod *mods[20];
  char new_dn[256];
  char cn_user_name[256];
  char contact_name[256];
  char *email_v[] = {NULL, NULL};
  char *cn_v[] = {NULL, NULL};
  char *contact_v[] = {NULL, NULL};
  char *objectClass_v[] = {"top", "person", 
                           "organizationalPerson", 
                           "contact", NULL};
  char *name_v[] = {NULL, NULL};
  char *desc_v[] = {NULL, NULL};
  int  n;
  int  rc;
  int  i;

  if (!check_string(user))
    {
      critical_alert("AD incremental - contact create",
                      "invalid LDAP name %s",
                      user);
      return(1);
    }
  strcpy(contact_name, user);
  sprintf(cn_user_name,"CN=%s,%s,%s", contact_name, group_ou, bind_path);
  cn_v[0] = cn_user_name;
  contact_v[0] = contact_name;
  name_v[0] = user;
  desc_v[0] = "Auto account created by Moira";
  email_v[0] = user;

  strcpy(new_dn, cn_user_name);
  n = 0;
  ADD_ATTR("cn", contact_v, LDAP_MOD_ADD);
  ADD_ATTR("objectClass", objectClass_v, LDAP_MOD_ADD);
  ADD_ATTR("name", name_v, LDAP_MOD_ADD);
  ADD_ATTR("displayName", name_v, LDAP_MOD_ADD);
  ADD_ATTR("description", desc_v, LDAP_MOD_ADD);
  if (!strcmp(group_ou, contact_ou))
    {
      ADD_ATTR("mail", email_v, LDAP_MOD_ADD);
    }
  mods[n] = NULL;

  rc = ldap_add_ext_s(ld, new_dn, mods, NULL, NULL);
  for (i = 0; i < n; i++)
    free(mods[i]);
  if ((rc != LDAP_SUCCESS) && (rc != LDAP_ALREADY_EXISTS))
    {
      n = 0;
      ADD_ATTR("cn", contact_v, LDAP_MOD_ADD);
      ADD_ATTR("objectClass", objectClass_v, LDAP_MOD_ADD);
      ADD_ATTR("name", name_v, LDAP_MOD_ADD);
      ADD_ATTR("displayName", name_v, LDAP_MOD_ADD);
      ADD_ATTR("description", desc_v, LDAP_MOD_ADD);
      mods[n] = NULL;
      rc = ldap_add_ext_s(ld, new_dn, mods, NULL, NULL);
      for (i = 0; i < n; i++)
        free(mods[i]);
    }
  if ((rc != LDAP_SUCCESS) && (rc != LDAP_ALREADY_EXISTS))
    {
      critical_alert("AD incremental - contact create", 
                     "could not create contact %s : %s",
                     user, ldap_err2string(rc));
      return(1);
    }
  return(0);
}

int user_update(int ac, char **av, void *ptr)
{
  LDAPMod   *mods[20];
  LK_ENTRY  *group_base;
  int  group_count;
  char distinguished_name[256];
  char user_name[256];
  char *uid_v[] = {NULL, NULL};
  char *mitid_v[] = {NULL, NULL};
  char *homedir_v[] = {NULL, NULL};
  char *winProfile_v[] = {NULL, NULL};
  char *drives_v[] = {NULL, NULL};
  int  n;
  int  rc;
  int  i;
  char **call_args;
  char filter_exp[256];
  char *attr_array[3];
  char **hp;
  char path[256];
  char winPath[256];
  char winProfile[256];

  call_args = ptr;

  if (!check_string(av[U_NAME]))
    {
      critical_alert("AD incremental - user update",
                      "invalid LDAP user name %s",
                      av[U_NAME]);
      return(0);
    }

  strcpy(user_name, av[U_NAME]);
  group_count = 0;
  group_base = NULL;
  sprintf(filter_exp, "(sAMAccountName=%s)", av[U_NAME]);
  attr_array[0] = "cn";
  attr_array[1] = NULL;
  if ((rc = linklist_build((LDAP *)call_args[0], call_args[1], filter_exp, attr_array, 
                           &group_base, &group_count)) != 0)
    {
      critical_alert("AD incremental - user update",
                     "LDAP server couldn't process user %s : %s",
                      user_name, ldap_err2string(rc));
      goto cleanup;
    }

  if (group_count != 1)
    {
      critical_alert("AD incremental - user update",
                     "LDAP server unable to find user %s in AD.",
                      user_name);
      callback_rc = LDAP_NO_SUCH_OBJECT;
      goto cleanup;
    }
  strcpy(distinguished_name, group_base->dn);

  n = 0;
  if (strlen(av[U_UID]) != 0)
    {
      uid_v[0] = av[U_UID];
      ADD_ATTR("uid", uid_v, LDAP_MOD_REPLACE);
      ADD_ATTR("uidNumber", uid_v, LDAP_MOD_REPLACE);
    }
  if (strlen(av[U_MITID]) != 0)
    {
      mitid_v[0] = av[U_MITID];
      ADD_ATTR("employeeID", mitid_v, LDAP_MOD_REPLACE);
    }
  if ((hp = hes_resolve(user_name, "filsys")) != NULL)
    {
      memset(path, 0, sizeof(path));
      memset(winPath, 0, sizeof(winPath));
      sscanf(hp[0], "%*s %s", path);
      if (strlen(path) && strnicmp(path, AFS, strlen(AFS)) == 0)
        {
          AfsToWinAfs(path, winPath);
          homedir_v[0] = winPath;
          ADD_ATTR("homeDirectory", homedir_v, LDAP_MOD_REPLACE);
          strcpy(winProfile, winPath);
          strcat(winProfile, "\\.winprofile");
          winProfile_v[0] = winProfile;
          ADD_ATTR("profilePath", winProfile_v, LDAP_MOD_REPLACE);
          drives_v[0] = "H:";
          ADD_ATTR("homeDrive", drives_v, LDAP_MOD_REPLACE);
        }
    }
  mods[n] = NULL;
  if (n != 0)
    {
      if ((rc = ldap_modify_s((LDAP *)call_args[0], distinguished_name, mods)) != LDAP_SUCCESS)
        {
           critical_alert("AD incremental - user update", 
                          "Couldn't modify user data for %s : %s",
                           user_name, ldap_err2string(rc));
        }
      for (i = 0; i < n; i++)
        free(mods[i]);
    }
  if (hp != NULL)
    {
      i = 0;
      while (hp[i])
        {
          free(hp[i]);
          i++;
        }
    }

cleanup:
  linklist_free(group_base);
  return(0);
}

int user_rename(int ac, char **av, void *ptr)
{
  LDAPMod *mods[20];
  char new_dn[256];
  char old_dn[256];
  char user_name[256];
  char upn[256];
  char temp[128];
  char *userPrincipalName_v[] = {NULL, NULL};
  char *altSecurityIdentities_v[] = {NULL, NULL};
  char *name_v[] = {NULL, NULL};
  char *samAccountName_v[] = {NULL, NULL};
  char *uid_v[] = {NULL, NULL};
  char *mitid_v[] = {NULL, NULL};
  int  n;
  int  rc;
  int  i;
  char **call_args;

  call_args = ptr;

  if ((atoi(av[U_STATE]) != US_REGISTERED) && (atoi(av[U_STATE]) != US_NO_PASSWD) && 
      (atoi(av[U_STATE]) != US_ENROLL_NOT_ALLOWED))
    return(0);
  if (!strncmp(av[U_NAME], "#", 1))
    return(0);
  if (!check_string(call_args[3]))
    {
    callback_rc = LDAP_NO_SUCH_OBJECT;
    return(0);
    }
  if (!check_string(av[U_NAME]))
    {
      critical_alert("AD incremental - user rename",
                      "invalid LDAP user name %s",
                      av[U_NAME]);
      return(0);
    }

  strcpy(user_name, av[U_NAME]);
  sprintf(old_dn, "cn=%s,%s,%s", call_args[3], user_ou, call_args[1]);
  sprintf(new_dn, "cn=%s", user_name);

  if ((rc = ldap_rename_s((LDAP *)call_args[0], old_dn, new_dn, NULL, TRUE, 
                           NULL, NULL)) != LDAP_SUCCESS)
    {
      if (rc == LDAP_NO_SUCH_OBJECT)
        {
          callback_rc = LDAP_NO_SUCH_OBJECT;
          return(0);
        }
      critical_alert("AD incremental - user rename", 
                     "Couldn't rename user from %s to %s : %s",
                      call_args[3], user_name, ldap_err2string(rc));
      return(0);
    }

  name_v[0] = user_name;
  sprintf(upn, "%s@%s", user_name, ldap_domain);
  userPrincipalName_v[0] = upn;
  sprintf(temp, "Kerberos:%s@%s", user_name, PRIMARY_REALM);
  altSecurityIdentities_v[0] = temp;
  samAccountName_v[0] = user_name;

  n = 0;
  ADD_ATTR("altSecurityIdentities", altSecurityIdentities_v, LDAP_MOD_REPLACE);
  ADD_ATTR("userPrincipalName", userPrincipalName_v, LDAP_MOD_REPLACE);
  ADD_ATTR("displayName", name_v, LDAP_MOD_REPLACE);
  ADD_ATTR("sAMAccountName", samAccountName_v, LDAP_MOD_REPLACE);
  if (strlen(av[U_UID]) != 0)
    {
      uid_v[0] = av[U_UID];
      ADD_ATTR("uid", uid_v, LDAP_MOD_REPLACE);
      ADD_ATTR("uidNumber", uid_v, LDAP_MOD_REPLACE);
    }
  if (strlen(av[U_MITID]) != 0)
    {
      mitid_v[0] = av[U_MITID];
      ADD_ATTR("employeeID", mitid_v, LDAP_MOD_REPLACE);
    }
  mods[n] = NULL;
  sprintf(new_dn, "cn=%s,%s,%s", user_name, user_ou, call_args[1]);
  if ((rc = ldap_modify_s((LDAP *)call_args[0], new_dn, mods)) != LDAP_SUCCESS)
    {
       critical_alert("AD incremental - user rename", 
                      "After renaming, couldn't modify user data for %s : %s",
                       user_name, ldap_err2string(rc));
    }
  for (i = 0; i < n; i++)
    free(mods[i]);
  return(0);
}

int filesys_process(int ac, char **av, void *ptr)
{
  char  distinguished_name[256];
  char  winPath[256];
  char  winProfile[256];
  char  fs_name[128];
  char  filter_exp[256];
  char  *attr_array[3];
  char  *homedir_v[] = {NULL, NULL};
  char  *winProfile_v[] = {NULL, NULL};
  char  *drives_v[] = {NULL, NULL};
  char  **call_args;
  int   group_count;
  int   n;
  int   rc;
  int   i;
  int   operation;
  LDAPMod   *mods[20];
  LK_ENTRY  *group_base;

  call_args = ptr;

  if (!check_string(av[FS_NAME]))
    {
      critical_alert("AD incremental - user filesys",
                      "invalid filesys name %s",
                      av[FS_NAME]);
      return(0);
    }


  if (strcmp(av[FS_TYPE], "AFS"))
    {
      critical_alert("AD incremental - user filesys",
                      "invalid filesys type %s",
                      av[FS_TYPE]);
      return(0);
    }

  strcpy(fs_name, av[FS_NAME]);
  group_count = 0;
  group_base = NULL;
  sprintf(filter_exp, "(sAMAccountName=%s)", av[FS_NAME]);
  attr_array[0] = "cn";
  attr_array[1] = NULL;
  if ((rc = linklist_build((LDAP *)call_args[0], call_args[1], filter_exp, attr_array, 
                           &group_base, &group_count)) != 0)
    {
      critical_alert("AD incremental - user update",
                     "LDAP server couldn't process filesys %s : %s",
                      fs_name, ldap_err2string(rc));
      goto cleanup;
    }

  if (group_count != 1)
    {
      critical_alert("AD incremental - user update",
                     "LDAP server unable to find user %s in AD.",
                      fs_name);
      callback_rc = LDAP_NO_SUCH_OBJECT;
      goto cleanup;
    }
  strcpy(distinguished_name, group_base->dn);

  operation = LDAP_MOD_ADD;
  if ((int)call_args[3] == ADFS_DELETE)
    operation = LDAP_MOD_DELETE;

  n = 0;
  if (operation == LDAP_MOD_ADD)
    {
      memset(winPath, 0, sizeof(winPath));
      AfsToWinAfs(av[FS_PACK], winPath);
      homedir_v[0] = winPath;
      drives_v[0] = "H:";
      memset(winProfile, 0, sizeof(winProfile));
      strcpy(winProfile, winPath);
      strcat(winProfile, "\\.winprofile");
      winProfile_v[0] = winProfile;
    }
  else
    {
      homedir_v[0] = NULL;
      drives_v[0] = NULL;
      winProfile_v[0] = NULL;
    }
  ADD_ATTR("profilePath", winProfile_v, operation);
  ADD_ATTR("homeDrive", drives_v, operation);
  ADD_ATTR("homeDirectory", homedir_v, operation);
  mods[n] = NULL;

  for (i = 1; i < 6; i++)
    {
      if ((rc = ldap_modify_s((LDAP *)call_args[0], distinguished_name, mods)) == LDAP_SUCCESS)
        break;
      sleep(20);
    }
  if (rc != LDAP_SUCCESS)
    {
      critical_alert("AD incremental - filesys update", 
                     "Couldn't modify user data for filesys %s : %s",
                     fs_name, ldap_err2string(rc));
    }
  for (i = 0; i < n; i++)
    free(mods[i]);

cleanup:
  return(0);
}

int user_create(int ac, char **av, void *ptr)
{
  LDAPMod *mods[20];
  char new_dn[256];
  char user_name[256];
  char sam_name[256];
  char *cn_v[] = {NULL, NULL};
  char *objectClass_v[] = {"top", "person", 
                           "organizationalPerson", 
                           "user", NULL};

  char *samAccountName_v[] = {NULL, NULL};
  char *altSecurityIdentities_v[] = {NULL, NULL};
  char *name_v[] = {NULL, NULL};
  char *desc_v[] = {NULL, NULL};
  char upn[256];
  char *userPrincipalName_v[] = {NULL, NULL};
  char *userAccountControl_v[] = {NULL, NULL};
  char *uid_v[] = {NULL, NULL};
  char *mitid_v[] = {NULL, NULL};
  char userAccountControlStr[80];
  char temp[128];
  u_int userAccountControl = UF_NORMAL_ACCOUNT | UF_DONT_EXPIRE_PASSWD | UF_PASSWD_CANT_CHANGE;
  int  n;
  int  rc;
  int  i;
  int  sid_count;
  char filter_exp[256];
  char *attr_array[3];
  char **call_args;

  call_args = ptr;

  if ((atoi(av[U_STATE]) != US_REGISTERED) && (atoi(av[U_STATE]) != US_NO_PASSWD) && 
      (atoi(av[U_STATE]) != US_ENROLL_NOT_ALLOWED))
    return(0);
  if (!strncmp(av[U_NAME], "#", 1))
    return(0);
  if (!check_string(av[U_NAME]))
    {
      critical_alert("AD incremental - user create",
                      "invalid LDAP user name %s",
                      av[U_NAME]);
      return(0);
    }

  strcpy(user_name, av[U_NAME]);
  sprintf(upn, "%s@%s", user_name, ldap_domain);
  sprintf(sam_name, "%s", av[U_NAME]);
  samAccountName_v[0] = sam_name;
  if (atoi(av[U_STATE]) == US_DELETED)
    userAccountControl |= UF_ACCOUNTDISABLE;
  sprintf(userAccountControlStr, "%ld", userAccountControl);
  userAccountControl_v[0] = userAccountControlStr;
  userPrincipalName_v[0] = upn;

  cn_v[0] = user_name;
  name_v[0] = user_name;
  desc_v[0] = "Auto account created by Moira";
  sprintf(temp, "Kerberos:%s@%s", user_name, PRIMARY_REALM);
  altSecurityIdentities_v[0] = temp;    
  sprintf(new_dn, "cn=%s,%s,%s", user_name, user_ou, call_args[1]);

  n = 0;
  ADD_ATTR("cn", cn_v, LDAP_MOD_ADD);
  ADD_ATTR("objectClass", objectClass_v, LDAP_MOD_ADD);
  ADD_ATTR("sAMAccountName", samAccountName_v, LDAP_MOD_ADD);
  ADD_ATTR("userPrincipalName", userPrincipalName_v, LDAP_MOD_ADD);
  ADD_ATTR("userAccountControl", userAccountControl_v, LDAP_MOD_ADD);
  ADD_ATTR("name", name_v, LDAP_MOD_ADD);
  ADD_ATTR("displayName", name_v, LDAP_MOD_ADD);
  ADD_ATTR("description", desc_v, LDAP_MOD_ADD);
  ADD_ATTR("altSecurityIdentities", altSecurityIdentities_v, LDAP_MOD_ADD);
  if (strlen(av[U_UID]) != 0)
    {
      uid_v[0] = av[U_UID];
      ADD_ATTR("uid", uid_v, LDAP_MOD_ADD);
      ADD_ATTR("uidNumber", uid_v, LDAP_MOD_ADD);
    }
  if (strlen(av[U_MITID]) != 0)
      mitid_v[0] = av[U_MITID];
  else
      mitid_v[0] = "none";
  ADD_ATTR("employeeID", mitid_v, LDAP_MOD_ADD);
  mods[n] = NULL;

  rc = ldap_add_ext_s((LDAP *)call_args[0], new_dn, mods, NULL, NULL);
  for (i = 0; i < n; i++)
    free(mods[i]);
  if (rc == LDAP_ALREADY_EXISTS)
    {
      rc = user_change_status(ac, av, ptr);
      return(0);
    }
  if ((rc != LDAP_SUCCESS) && (rc != LDAP_ALREADY_EXISTS))
    {
       critical_alert("AD incremental - user create", 
                      "could not create user %s : %s",
                       user_name, ldap_err2string(rc));
       return(0);
    }
  if (rc == LDAP_SUCCESS)
    {
      if ((rc = set_password(sam_name, "", ldap_domain)) != 0)
        {
          critical_alert("AD incremental - user create", 
                         "Couldn't set password for user %s : %ld",
                         user_name, rc);
        }
    }
  sprintf(filter_exp, "(sAMAccountName=%s)", av[U_NAME]);
  attr_array[0] = "objectSid";
  attr_array[1] = NULL;
  sid_count = 0;
  if ((rc = linklist_build((LDAP *)call_args[0], call_args[1], filter_exp, attr_array, 
                           sid_ptr, &sid_count)) == LDAP_SUCCESS)
    {
      if (sid_count == 1)
        {
          (*sid_ptr)->member = strdup(av[U_NAME]);
          (*sid_ptr)->type = (char *)USERS;
          sid_ptr = &(*sid_ptr)->next;
        }
    }
  return(0);
}

int user_change_status(int ac, char **av, void *ptr)
{
  char      filter_exp[1024];
  char      *attr_array[3];
  char      temp[256];
  char      distinguished_name[1024];
  char      user_name[512];
  char      **modvalues;
  LDAPMod   *mods[20];
  LK_ENTRY  *group_base;
  int       group_count;
  int       rc;
  int       i;
  int       n;
  int       operation;
  ULONG     ulongValue;
  char **call_args;

  call_args = ptr;

  if (!check_string(av[U_NAME]))
    {
      critical_alert("AD incremental - user change status",
                      "invalid LDAP user name %s",
                      av[U_NAME]);
      return(0);
    }
  strcpy(user_name, av[U_NAME]);
  operation = (int)call_args[2];
  group_count = 0;
  group_base = NULL;
  sprintf(filter_exp, "(sAMAccountName=%s)", av[U_NAME]);
  attr_array[0] = "UserAccountControl";
  attr_array[1] = NULL;
  if ((rc = linklist_build((LDAP *)call_args[0], call_args[1], filter_exp, attr_array, 
                           &group_base, &group_count)) != 0)
    {
      critical_alert("AD incremental - user change status",
                     "LDAP server couldn't process user %s : %s",
                      user_name, ldap_err2string(rc));
      goto cleanup;
    }

  if (group_count != 1)
    {
      critical_alert("AD incremental - user change status",
                     "LDAP server unable to find user %s in AD.",
                      user_name);
      goto cleanup;
    }

  strcpy(distinguished_name, group_base->dn);
  ulongValue = atoi((*group_base).value);
  if (operation == MEMBER_DEACTIVATE)
    ulongValue |= UF_ACCOUNTDISABLE;
  else    
    ulongValue &= ~UF_ACCOUNTDISABLE;
  sprintf(temp, "%ld", ulongValue);
  if ((rc = construct_newvalues(group_base, group_count, (*group_base).value, 
                                temp, &modvalues, REPLACE)) == 1)
      goto cleanup;
  linklist_free(group_base);
  group_base = NULL;
  group_count = 0;
  n = 0;
  ADD_ATTR("UserAccountControl", modvalues, LDAP_MOD_REPLACE);
  mods[n] = NULL;
  rc = ldap_modify_s((LDAP *)call_args[0], distinguished_name, mods);
  for (i = 0; i < n; i++)
    free(mods[i]);
  free_values(modvalues);
  if (rc != LDAP_SUCCESS)
    {
      critical_alert("AD incremental - user change status",
                     "LDAP server could not change status of user %s : %s",
                     user_name, ldap_err2string(rc));
    }
cleanup:
  linklist_free(group_base);
  return(0);
}

int user_delete(LDAP *ldap_handle, char *dn_path, char *u_name)
{
  char      filter_exp[1024];
  char      *attr_array[3];
  char      distinguished_name[1024];
  char      user_name[512];
  LK_ENTRY  *group_base;
  int       group_count;
  int       rc;

  if (!check_string(u_name))
    return(0);
  strcpy(user_name, u_name);
  group_count = 0;
  group_base = NULL;
  sprintf(filter_exp, "(sAMAccountName=%s)", user_name);
  attr_array[0] = "name";
  attr_array[1] = NULL;
  if ((rc = linklist_build(ldap_handle, dn_path, filter_exp, attr_array, 
                           &group_base, &group_count)) != 0)
    {
      critical_alert("AD incremental",
                     "LDAP server couldn't process user %s : %s",
                      user_name, ldap_err2string(rc));
      goto cleanup;
    }

  if (group_count != 1)
    {
      critical_alert("AD incremental - user change status",
                     "LDAP server unable to find user %s in AD.",
                      user_name);
      goto cleanup;
    }

  strcpy(distinguished_name, group_base->dn);
  if (rc = ldap_delete_s(ldap_handle, distinguished_name))
    {
      critical_alert("AD incremental",
                     "LDAP server couldn't process user %s : %s",
                      user_name, ldap_err2string(rc));
    }

cleanup:
  linklist_free(group_base);
  return(0);
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

void free_values(char **modvalues)
{
  int i;

  i = 0;
  if (modvalues != NULL)
    {
    while (modvalues[i] != NULL)
      {
        free(modvalues[i]);
        modvalues[i] = NULL;
        ++i;
      }
    free(modvalues);
  }
}

int sid_update(LDAP *ldap_handle, char *dn_path)
{
  LK_ENTRY      *ptr;
  int           rc;
  unsigned char temp[126];
  char          *av[3];

  ptr = sid_base;

  while (ptr != NULL)
    {
      memset(temp, 0, sizeof(temp));
      convert_b_to_a(temp, ptr->value, ptr->length);
      av[0] = ptr->member;
      av[1] = temp;
      if (ptr->type == (char *)GROUPS)
        {
          ptr->type = NULL;
          rc = mr_query("add_list_sid_by_name", 2, av, NULL, NULL);
        }
      else if (ptr->type == (char *)USERS)
        {
          ptr->type = NULL;
          rc = mr_query("add_user_sid_by_login", 2, av, NULL, NULL);
        }
      ptr = ptr->next;
    }
  return(0);
}

void convert_b_to_a(char *string, UCHAR *binary, int length)
{
  int   i;
  int   j;
  UCHAR tmp;

  j = 0;
  for (i = 0; i < length; i++)
    {
      tmp = binary[i];
      string[j] = tmp;
      string[j] >>= 4;
      string[j] &= 0x0f;
      string[j] += 0x30;
      if (string[j] > '9')
        string[j] += 0x27;
      ++j;
      string[j] = tmp & 0x0f;
      string[j] += 0x30;
      if (string[j] > '9')
        string[j] += 0x27;
      j++;
    }
  string[j] = 0;
}

static int illegalchars[] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* ^@ - ^O */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* ^P - ^_ */
  1, 1, 1, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, /* SPACE - / */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, /* 0 - ? */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* @ - O */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, /* P - _ */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ` - o */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, /* p - ^? */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

int check_string(char *s)
{
  char  character;

  for (; *s; s++)
    {
      character = *s;
      if (isupper(character))
        character = tolower(character);
      if (illegalchars[(unsigned) character])
        return 0;
    }
  return 1;
}

int mr_connect_cl(char *server, char *client, int version, int auth)
{
  int   status;
  char  *motd;
  char  temp[128];

  status = mr_connect(server);
  if (status)
    {
      com_err(whoami, status, "while connecting to Moira");
      return status;
    }

  status = mr_motd(&motd);
  if (status)
    {
      mr_disconnect();
      com_err(whoami, status, "while checking server status");
      return status;
    }
  if (motd)
    {
      sprintf(temp, "The Moira server is currently unavailable: %s", motd);
      com_err(whoami, status, temp);
      mr_disconnect();
      return status;
    }

  status = mr_version(version);
  if (status)
    {
      if (status == MR_UNKNOWN_PROC)
        {
          if (version > 2)
            status = MR_VERSION_HIGH;
          else
            status = MR_SUCCESS;
        }

      if (status == MR_VERSION_HIGH)
        {
          com_err(whoami, 0, "Warning: This client is running newer code than the server.");
                  com_err(whoami, 0, "Some operations may not work.");
        }
      else if (status && status != MR_VERSION_LOW)
        {
          com_err(whoami, status, "while setting query version number.");
          mr_disconnect();
          return status;
        }
    }

  if (auth)
    {
      status = mr_auth(client);
      if (status)
        {
          com_err(whoami, status, "while authenticating to Moira.");
          mr_disconnect();
          return status;
        }
    }

  return MR_SUCCESS;
}

void expand_groups(LDAP *ldap_handle, char *dn_path, char *group_name)
{
  LK_ENTRY  *group_base = NULL;
  LK_ENTRY  *ptr = NULL;
  char      *call_args[7];
  char      *av[2];
  int       rc;
  int       before_count;
  int       after_count;

  av[0] = "RLIST";
  av[1] = group_name;
  call_args[0] = (char *)ldap_handle;
  call_args[1] = dn_path;
  call_args[2] = group_name;
  call_args[3] = NULL;

  linklist_free(member_base);
  member_base = NULL;
  linklist_free(sid_base);
  sid_base = NULL;
  if (mr_query("get_lists_of_member", 2, av, list_list_build, call_args) == MR_NO_MATCH)
    return;
  if (member_base == NULL)
    return;
  while (1)
    {
      group_base = member_base;
      ptr = group_base;
      before_count = 0;
      while(ptr != NULL)
        {
          ++before_count;
          ptr = ptr->next;
        }
      ptr = group_base;
      while (ptr != NULL)
        {
          sleep(1);
          av[0] = "RLIST";
          av[1] = ptr->member;
          call_args[0] = (char *)ldap_handle;
          call_args[1] = dn_path;
          call_args[2] = ptr->member;
          call_args[3] = NULL;
          mr_query("get_lists_of_member", 2, av, list_list_build, call_args);
          ptr = ptr->next;
        }
      after_count = 0;
      ptr = group_base;
      while(ptr != NULL)
        {
          ++after_count;
          ptr = ptr->next;
        }
      if (before_count == after_count)
        break;
    }

  group_base = member_base;
  ptr = group_base;
  while (ptr != NULL)
    {
      sleep(1);
      member_base = NULL;
      sid_base = NULL;
      sid_ptr = &sid_base;
      av[0] = ptr->member;
      av[1] = NULL;
      call_args[0] = (char *)ldap_handle;
      call_args[1] = dn_path;
      call_args[2] = ptr->member;
      call_args[3] = NULL;
      call_args[4] = NULL;
      call_args[5] = NULL;
      call_args[6] = (char *)(MOIRA_USERS | MOIRA_KERBEROS | MOIRA_STRINGS);
      if (rc = mr_query("get_list_info", 1, av, group_create, call_args))
        {
          linklist_free(member_base);
          member_base = NULL;
          ptr = ptr->next;
          continue;
        }
      if (sid_base != NULL)
        {
          sid_update(ldap_handle, dn_path);
          linklist_free(sid_base);
          sid_base = NULL;
        }
      sleep(1);
      member_base = NULL;
      if (!(rc = mr_query("get_end_members_of_list", 1, av, member_list_build,
                          call_args)))
        {
          if (member_base == NULL)
            {
              member_remove(ldap_handle, dn_path, ptr->member,
                            call_args[3], call_args[4], call_args[5]);
            }
          else
            {
              rc = member_list_process(ldap_handle, dn_path, ptr->member,
                                       call_args[3], call_args[4], call_args[5],
                                       MOIRA_USERS | MOIRA_KERBEROS | MOIRA_STRINGS);
            }
        }
      linklist_free(member_base);
      member_base = NULL;
      if (call_args[3] != NULL)
        free(call_args[3]);
      if (call_args[4] != NULL)
        free(call_args[4]);
      call_args[3] = NULL;
      call_args[4] = NULL;
      call_args[5] = NULL;
      call_args[6] = NULL;
      ptr = ptr->next;
    }
  linklist_free(group_base);
  group_base = NULL;
  return;
}

void AfsToWinAfs(char* path, char* winPath)
{
    char* pathPtr;
    char* winPathPtr;
    strcpy(winPath, WINAFS);
    pathPtr = path + strlen(AFS);
    winPathPtr = winPath + strlen(WINAFS);

    while (*pathPtr)
    {
        if (*pathPtr == '/')
          *winPathPtr = '\\';
        else
          *winPathPtr = *pathPtr;

        pathPtr++;
        winPathPtr++;
    }
}
