/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/incremental/winad/winad.c,v 1.19 2001-07-16 22:19:04 zacheiss Exp $
/* winad.incr arguments examples
 *
 *
 * arguments when moira creates the account - ignored by winad.incr since the account is unusable.
 * users 0 11 #45198 45198 /bin/cmd cmd Last First Middle 0 950000001 2000 121049
 *   login, unix_uid, shell, winconsoleshell, last, first, middle, status, mitid, type, moiraid
 *
 * arguments for creating or updating a user account 
 * users 11 11 username 45206 /bin/cmd cmd Last First Middle 2 950000001 STAFF 121058 username 45206 /bin/cmd cmd Last First Middle 1 950000001 STAFF 121058
 * users 11 11 #45206 45206 /bin/cmd cmd Last First Middle 0 950000001 STAFF 121058 newuser 45206 /bin/cmd cmd Last First Middle 2 950000001 STAFF 121058
 *   login, unix_uid, shell, winconsoleshell, last, first, middle, status, mitid, type, moiraid
 *
 * arguments for deactivating/deleting a user account
 * users 11 11 username 45206 /bin/cmd cmd Last First Middle 1 950000001 STAFF 121058 username 45206 /bin/cmd cmd Last First Middle 3 950000001 STAFF 121058
 * users 11 11 username 45206 /bin/cmd cmd Last First Middle 2 950000001 STAFF 121058 username 45206 /bin/cmd cmd Last First Middle 3 950000001 STAFF 121058
 *   login, unix_uid, shell, winconsoleshell, last, first, middle, status, mitid, type, moiraid
 *
 * arguments for reactivating a user account
 * users 11 11 username 45206 /bin/cmd cmd Last First Middle 3 950000001 STAFF 121058 username 45206 /bin/cmd cmd Last First Middle 1 950000001 STAFF 121058
 * users 11 11 username 45206 /bin/cmd cmd Last First Middle 3 950000001 STAFF 121058 username 45206 /bin/cmd cmd Last First Middle 2 950000001 STAFF 121058
 *   login, unix_uid, shell, winconsoleshell, last, first, middle, status, mitid, type, moiraid
 *
 * arguments for changing user name
 * users 11 11 oldusername 45206 /bin/cmd cmd Last First Middle 1 950000001 STAFF 121058 newusername 45206 /bin/cmd cmd Last First Middle 1 950000001 STAFF 121058
 *   login, unix_uid, shell, winconsoleshell, last, first, middle, status, mitid, type, moiraid
 *
 * arguments for expunging a user
 * users 11 0 username 45198 /bin/cmd cmd Last First Middle 0 950000001 2000 121049
 *   login, unix_uid, shell, winconsoleshell, last, first, middle, status, mitid, type, moiraid
 *
 * arguments for creating a "special" group/list
 * list 0 11 listname 1 1 0 0 0 -1 NONE 0 description 92616
 *   listname, active, publicflg, hidden, maillist, grouplist, gid, acl_type, acl_id, description, moiraid
 * 
 * arguments for creating a "mail" group/list
 * list 0 11 listname 1 1 0 1 0 -1 NONE 0 description 92616
 *   listname, active, publicflg, hidden, maillist, grouplist, gid, acl_type, acl_id, description, moiraid
 *
 * arguments for creating a "group" group/list
 * list 0 11 listname 1 1 0 0 1 -1 NONE 0 description 92616
 *   listname, active, publicflg, hidden, maillist, grouplist, gid, acl_type, acl_id, description, moiraid
 *
 * arguments for creating a "group/mail" group/list
 * list 0 11 listname 1 1 0 1 1 -1 NONE 0 description 92616
 *   listname, active, publicflg, hidden, maillist, grouplist, gid, acl_type, acl_id, description, moiraid
 *
 * arguments to add a USER member to group/list
 * imembers 0 12 listname USER userName 1 1 0 0 0 -1 1 92616 121047
 *   list_name, user_type, name, active, publicflg, hidden, maillist, grouplist, gid, userStatus, moiraListId, moiraUserId
 *
 * arguments to add a STRING or KERBEROS member to group/list
 * imembers 0 10 listname STRING stringName 1 1 0 0 0 -1 92616
 * imembers 0 10 listlistnameName KERBEROS kerberosName 1 1 0 0 0 -1 92616
 *   list_name, user_type, name, active, publicflg, hidden, maillist, grouplist, gid, moiraListId
 *
 * NOTE: group members of type LIST are ignored.
 *
 * arguments to remove a USER member to group/list
 * imembers 12 0 listname USER userName 1 1 0 0 0 -1 1 92616 121047
 *   list_name, user_type, name, active, publicflg, hidden, maillist, grouplist, gid, userStatus, moiraListId, moiraUserId
 *
 * arguments to remove a STRING or KERBEROS member to group/list
 * imembers 10 0 listname STRING stringName 1 1 0 0 0 -1 92616
 * imembers 10 0 listname KERBEROS kerberosName 1 1 0 0 0 -1 92616
 *   list_name, user_type, name, active, publicflg, hidden, maillist, grouplist, gid, moiraListId
 *
 * NOTE: group members of type LIST are ignored.
 *
 * arguments for renaming a group/list
 * list 11 11 oldlistname 1 1 0 0 0 -1 NONE 0 description 92616 newlistname 1 1 0 0 0 -1 description 0 92616
 *   name, active, publicflg, hidden, maillist, grouplist, gid, acl_type, acl_id, description, moiraListId
 *
 * arguments for deleting a group/list
 * list 11 0 listname 1 1 0 0 0 -1 NONE 0 description 92616
 *   name, active, publicflg, hidden, maillist, grouplist, gid, acl_type, acl_id, description, moiraListId

 * arguments for adding a file system
 * filesys 0 12 username AFS ATHENA.MIT.EDU /afs/athena.mit.edu/user/n/e/username /mit/username w descripton username wheel 1 HOMEDIR 101727
 *
 * arguments for deleting a file system
 * filesys 12 0 username AFS ATHENA.MIT.EDU /afs/athena.mit.edu/user/n/e/username /mit/username w descripton username wheel 1 HOMEDIR 101727
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

#define WINADCFG "/moira/winad/winad.cfg"
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

#ifndef WINADCFG
#define WINADCFG "winad.cfg"
#endif

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

#define CHECK_GROUPS    1
#define CLEANUP_GROUPS  2

#define AD_NO_GROUPS_FOUND        -1
#define AD_WRONG_GROUP_DN_FOUND   -2
#define AD_MULTIPLE_GROUPS_FOUND  -3
#define AD_INVALID_NAME           -4
#define AD_LDAP_FAILURE           -5
#define AD_INVALID_FILESYS        -6
#define AD_NO_ATTRIBUTE_FOUND     -7
#define AD_NO_OU_FOUND            -8
#define AD_NO_USER_FOUND          -9

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
char  kerberos_ou[] = "OU=kerberos,OU=moira";
char  contact_ou[] = "OU=strings,OU=moira";
char  user_ou[] = "OU=users,OU=moira";
char  group_ou_distribution[] = "OU=mail,OU=lists,OU=moira";
char  group_ou_root[] = "OU=lists,OU=moira";
char  group_ou_security[] = "OU=group,OU=lists,OU=moira";
char  group_ou_neither[] = "OU=special,OU=lists,OU=moira";
char  group_ou_both[] = "OU=mail,OU=group,OU=lists,OU=moira";
char *whoami;
char ldap_domain[256];
int  mr_connections = 0;
int  callback_rc;
char default_server[256];
static char tbl_buf[1024];

extern int set_password(char *user, char *password, char *domain);

int ad_get_group(LDAP *ldap_handle, char *dn_path, char *group_name, 
                char *group_membership, char *MoiraId, char *attribute,
                LK_ENTRY **linklist_base, int *linklist_count,
              char *rFilter);
void AfsToWinAfs(char* path, char* winPath);
int ad_connect(LDAP **ldap_handle, char *ldap_domain, char *dn_path, 
               char *Win2kPassword, char *Win2kUser, char *default_server,
               int connect_to_kdc);
void ad_kdc_disconnect();
void check_winad(void);
int check_user(LDAP *ldap_handle, char *dn_path, char *UserName, char *MoiraId);
int filesys_process(LDAP *ldap_handle, char *dn_path, char *fs_name, 
                    char *fs_type, char *fs_pack, int operation);
int get_group_membership(char *group_membership, char *group_ou, 
                         int *security_flag, char **av);
int process_group(LDAP *ldap_handle, char *dn_path, char *MoiraId, 
                  char *group_name, char *group_ou, char *group_membership, 
                  int group_security_flag, int type);
int process_lists(int ac, char **av, void *ptr);
int user_create(int ac, char **av, void *ptr);
int user_change_status(LDAP *ldap_handle, char *dn_path, 
                       char *user_name, char *MoiraId, int operation);
int user_delete(LDAP *ldap_handle, char *dn_path, 
                char *u_name, char *MoiraId);
int user_rename(LDAP *ldap_handle, char *dn_path, char *before_user_name, 
                char *user_name);
int user_update(LDAP *ldap_handle, char *dn_path, char *user_name,
                char *uid, char *MitId, char *MoiraId, int State);
void change_to_lower_case(char *ptr);
int contact_create(LDAP *ld, char *bind_path, char *user, char *group_ou);
int group_create(int ac, char **av, void *ptr);
int group_delete(LDAP *ldap_handle, char *dn_path, 
                 char *group_name, char *group_membership, char *MoiraId);
int group_rename(LDAP *ldap_handle, char *dn_path, 
                 char *before_group_name, char *before_group_membership, 
                 char *before_group_ou, int before_security_flag, char *before_desc,
                 char *after_group_name, char *after_group_membership, 
                 char *after_group_ou, int after_security_flag, char *after_desc,
                 char *MoiraId, char *filter);
int make_new_group(LDAP *ldap_handle, char *dn_path, char *MoiraId, 
                   char *group_name, char *group_ou, char *group_membership, 
                   int group_security_flag, int updateGroup);
int member_list_build(int ac, char **av, void *ptr);
int member_add(LDAP *ldap_handle, char *dn_path, char *group_name, 
                        char *group_ou, char *group_membership, 
                        char *user_name, char *pUserOu, char *MoiraId);
int member_remove(LDAP *ldap_handle, char *dn_path, char *group_name, 
                  char *group_ou, char *group_membership, char *user_name,
                  char *pUserOu, char *MoiraId);
int populate_group(LDAP *ldap_handle, char *dn_path, char *group_name, 
                   char *group_ou, char *group_membership, 
                   int group_security_flag, char *MoiraId);
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

  for (i = 1; i < argc; i++)
    {
      strcat(tbl_buf, argv[i]);
      strcat(tbl_buf, " ");
    }
  com_err(whoami, 0, "%s", tbl_buf);

  check_winad();

  memset(ldap_domain, '\0', sizeof(ldap_domain));
  if ((fptr = fopen(WINADCFG, "r")) != NULL)
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
  for (i = 0; i < 5; i++)
    {
      if (!(rc = ad_connect(&ldap_handle, ldap_domain, dn_path, "", "", default_server, 1)))
         break;
      sleep(2);
    }
  if (rc)
    {
  	  critical_alert("incremental", "winad.incr cannot connect to any server in domain %s", ldap_domain);
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
  int   abort_flag;

  abort_flag = 0;

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
      abort_flag = 0;
      while (1)
      {
        if ((rc = filesys_process(ldap_handle, dn_path, after[FS_NAME], 
                    after[FS_TYPE], after[FS_PACK], LDAP_MOD_ADD)) != LDAP_NO_SUCH_OBJECT)
          {
            if (rc != LDAP_SUCCESS)
              com_err(whoami, 0, "Couldn't process filesys %s", after[FS_NAME]);
            break;
          }
        if (abort_flag == 1)
          break;
        sleep(1);
        abort_flag = 1;
        if (rc = moira_connect())
          {
            critical_alert("AD incremental",
                           "Error contacting Moira server : %s",
                           error_message(rc));
            return;
          }
        av[0] = after[FS_NAME];
        call_args[0] = (char *)ldap_handle;
        call_args[1] = dn_path;
        call_args[2] = "";
        call_args[3] = NULL;
        sid_base = NULL;
        sid_ptr = &sid_base;
        callback_rc = 0;
        if (rc = mr_query("get_user_account_by_login", 1, av, user_create,
                          call_args))
          {
            moira_disconnect();
            com_err(whoami, 0, "Couldn't process filesys %s", after[FS_NAME]);
            break;
          }
        if (callback_rc)
          {
            moira_disconnect();
            com_err(whoami, 0, "Couldn't process filesys %s", after[FS_NAME]);
            break;
          }
        if (sid_base != NULL)
          {
            sid_update(ldap_handle, dn_path);
            linklist_free(sid_base);
            sid_base = NULL;
          }
        moira_disconnect();
      }
      goto cleanup;
    }

  btype = !strcmp(before[FS_TYPE], "AFS");
  bcreate = atoi(before[FS_CREATE]);
  if (afterc < FS_CREATE)
    {
      if (btype && bcreate)
        {
          if (rc = filesys_process(ldap_handle, dn_path, before[FS_NAME], 
                      before[FS_TYPE], before[FS_PACK], LDAP_MOD_DELETE))
            {
              com_err(whoami, 0, "Couldn't delete filesys %s", before[FS_NAME]);
            }
        }
      return;
    }

  if (!acreate)
    return;

  if (!atype && !btype)
    {
      if (strcmp(before[FS_TYPE], "ERR") || strcmp(after[FS_TYPE], "ERR"))
        {
          com_err(whoami, 0, "Filesystem %s or %s is not AFS",
                  before[FS_NAME], after[FS_NAME]);
          return;
        }
    }
  com_err(whoami, 0, "Processing filesys %s", after[FS_NAME]);
  abort_flag = 0;
  while (1)
  {
    if ((rc = filesys_process(ldap_handle, dn_path, after[FS_NAME], 
                    after[FS_TYPE], after[FS_PACK], LDAP_MOD_ADD)) != LDAP_NO_SUCH_OBJECT)
      {
        if (rc != LDAP_SUCCESS)
          com_err(whoami, 0, "Couldn't process filesys %s", after[FS_NAME]);
        break;
      }
    if (abort_flag == 1)
      break;
    sleep(1);
    abort_flag = 1;
    if (rc = moira_connect())
      {
        critical_alert("AD incremental",
                       "Error contacting Moira server : %s",
                       error_message(rc));
        return;
      }
    av[0] = after[FS_NAME];
    call_args[0] = (char *)ldap_handle;
    call_args[1] = dn_path;
    call_args[2] = "";
    call_args[3] = NULL;
    sid_base = NULL;
    sid_ptr = &sid_base;
    callback_rc = 0;
    if (rc = mr_query("get_user_account_by_login", 1, av, user_create,
                      call_args))
      {
        moira_disconnect();
        com_err(whoami, 0, "Couldn't process filesys %s", after[FS_NAME]);
        break;
      }
    if (callback_rc)
      {
        moira_disconnect();
        com_err(whoami, 0, "Couldn't process filesys %s", after[FS_NAME]);
        break;
      }
    if (sid_base != NULL)
      {
        sid_update(ldap_handle, dn_path);
        linklist_free(sid_base);
        sid_base = NULL;
      }
    moira_disconnect();
  }

cleanup:
  return;
}

#define L_LIST_DESC 9
#define L_LIST_ID   10

void do_list(LDAP *ldap_handle, char *dn_path, char *ldap_hostname,
             char **before, int beforec, char **after, int afterc)
{
  int   updateGroup;
  long  rc;
  char  group_membership[6];
  char  list_id[32];
  int   security_flag;
  char  filter[128];
  char  group_ou[256];
  char  before_list_id[32];
  char  before_group_membership[1];
  int   before_security_flag;
  char  before_group_ou[256];
  LK_ENTRY *ptr = NULL;

  if (beforec == 0 && afterc == 0)
    return;

  memset(list_id, '\0', sizeof(list_id));
  memset(before_list_id, '\0', sizeof(before_list_id));
  memset(before_group_ou, '\0', sizeof(before_group_ou));
  memset(before_group_membership, '\0', sizeof(before_group_membership));
  memset(group_ou, '\0', sizeof(group_ou));
  memset(group_membership, '\0', sizeof(group_membership));
  updateGroup = 0;

  if (beforec > L_GID)
    {
      if (beforec < L_LIST_ID)
        return;
      if (beforec > L_LIST_DESC)
        {
          strcpy(before_list_id, before[L_LIST_ID]);
        }
      before_security_flag = 0;
      get_group_membership(before_group_membership, before_group_ou, &before_security_flag, before);
    }
  if (afterc > L_GID)
    {
      if (afterc < L_LIST_ID)
        return;
      if (afterc > L_LIST_DESC)
        {
          strcpy(list_id, before[L_LIST_ID]);
        }
      security_flag = 0;
      get_group_membership(group_membership, group_ou, &security_flag, after);
    }

  if ((beforec == 0) && (afterc == 0)) /*this case should never happen*/
    return;

  updateGroup = 0;
  if (beforec)
    {
      updateGroup = 1;
      if ((rc = process_group(ldap_handle, dn_path, before_list_id, before[L_NAME], 
                              before_group_ou, before_group_membership, 
                              before_security_flag, CHECK_GROUPS)))
        {
          if (rc == AD_NO_GROUPS_FOUND)
            updateGroup = 0;
          else
            {
              if ((rc == AD_WRONG_GROUP_DN_FOUND) || (rc == AD_MULTIPLE_GROUPS_FOUND))
                {
                  rc = process_group(ldap_handle, dn_path, before_list_id, before[L_NAME], 
                                     before_group_ou, before_group_membership, 
                                     before_security_flag, CLEANUP_GROUPS);
                }
              if ((rc != AD_NO_GROUPS_FOUND) && (rc != 0))
                {
                  com_err(whoami, 0, "Could not change list name from %s to %s",
                          before[L_NAME], after[L_NAME]);
                  return;
                }
              if (rc == AD_NO_GROUPS_FOUND)
                updateGroup = 0;
            }
        }
    }
  
  if ((beforec != 0) && (afterc != 0))
    {
      if (((strcmp(after[L_NAME], before[L_NAME])) || 
          ((!strcmp(after[L_NAME], before[L_NAME])) && 
           (strcmp(before_group_ou, group_ou)))) &&
          (updateGroup == 1))
        {
          com_err(whoami, 0, "Changing list name from %s to %s",
                  before[L_NAME], after[L_NAME]);
          if ((strlen(before_group_ou) == 0) || (strlen(before_group_membership) == 0) ||
              (strlen(group_ou) == 0) || (strlen(group_membership) == 0))
            {
              com_err(whoami, 0, "%s", "couldn't find the group OU's");
              return;
            }
          memset(filter, '\0', sizeof(filter));
          if ((rc = group_rename(ldap_handle, dn_path, 
                                 before[L_NAME], before_group_membership, 
                                 before_group_ou, before_security_flag, before[L_LIST_DESC],
                                 after[L_NAME], group_membership, 
                                 group_ou, security_flag, after[L_LIST_DESC],
                                 list_id, filter)))
            {
              if (rc != AD_NO_GROUPS_FOUND)
                {
                  com_err(whoami, 0, "Could not change list name from %s to %s",
                          before[L_NAME], after[L_NAME]);
                  return;
                }
              updateGroup = 0;
            }
          beforec = 0;
        }
      else
        beforec = 0;
    }

  if (beforec)
    {
      if ((strlen(before_group_ou) == 0) || (strlen(before_group_membership) == 0))
        {
          com_err(whoami, 0, "couldn't find the group OU for group %s", before[L_NAME]);
          return;
        }
      com_err(whoami, 0, "Deleting group %s", before[L_NAME]);
      rc = group_delete(ldap_handle, dn_path, before[L_NAME], 
                        before_group_membership, before_list_id);
      return;
    }
  if (afterc)
    {
      if (!updateGroup)
        {
          com_err(whoami, 0, "Creating group %s", after[L_NAME]);
          if (rc = process_group(ldap_handle, dn_path, list_id, after[L_NAME], 
                                  group_ou, group_membership, 
                                  security_flag, CHECK_GROUPS))
            {
              if (rc != AD_NO_GROUPS_FOUND)
                {
                  if ((rc == AD_WRONG_GROUP_DN_FOUND) || (rc == AD_MULTIPLE_GROUPS_FOUND))
                    {
                      rc = process_group(ldap_handle, dn_path, list_id, after[L_NAME], 
                                         group_ou, group_membership, 
                                         security_flag, CLEANUP_GROUPS);
                    }
                  if (rc)
                    {
                      com_err(whoami, 0, "Could not create list %s", after[L_NAME]);
                      return;
                    }
                }
            }
        }
      else
        com_err(whoami, 0, "Updating group %s information", after[L_NAME]);

      if (rc = moira_connect())
        {
          critical_alert("AD incremental",
                         "Error contacting Moira server : %s",
                         error_message(rc));
          return;
        }

      if (make_new_group(ldap_handle, dn_path, list_id, after[L_NAME], 
                         group_ou, group_membership, security_flag, updateGroup))
        {
          moira_disconnect();
          return;
        }
      if (atoi(after[L_ACTIVE]))
        {
          populate_group(ldap_handle, dn_path, after[L_NAME], group_ou, 
                         group_membership, security_flag, list_id);
        }
      moira_disconnect();
    }

  return;
}

#define LM_EXTRA_ACTIVE	  (LM_END)
#define LM_EXTRA_PUBLIC   (LM_END+1)
#define LM_EXTRA_HIDDEN   (LM_END+2)
#define LM_EXTRA_MAILLIST (LM_END+3)
#define LM_EXTRA_GROUP    (LM_END+4)
#define LM_EXTRA_GID      (LM_END+5)
#define LMN_LIST_ID       (LM_END+6)
#define LM_LIST_ID        (LM_END+7)
#define LM_USER_ID        (LM_END+8)
#define LM_EXTRA_END      (LM_END+9)

void do_member(LDAP *ldap_handle, char *dn_path, char *ldap_hostname,
               char **before, int beforec, char **after, int afterc)
{
  char  group_name[128];
  char  user_name[128];
  char  user_type[128];
  char  moira_list_id[32];
  char  moira_user_id[32];
  char  group_membership[1];
  char  group_ou[256];
  char  *args[16];
  char  **ptr;
  char  *av[7];
  char  *call_args[7];
  char  *pUserOu;
  int   security_flag;
  int   rc;

  pUserOu = NULL;
  ptr = NULL;
  memset(moira_list_id, '\0', sizeof(moira_list_id));
  memset(moira_user_id, '\0', sizeof(moira_user_id));
  if (afterc)
    {
      if (afterc < LM_EXTRA_GID)
        return;
      if (!atoi(after[LM_EXTRA_ACTIVE]))
        return;
      ptr = after;
      if (!strcasecmp(ptr[LM_TYPE], "LIST"))
        return;
      strcpy(user_name, after[LM_MEMBER]);
      strcpy(group_name, after[LM_LIST]);
      strcpy(user_type, after[LM_TYPE]);
      if (!strcasecmp(ptr[LM_TYPE], "USER"))
        {
          if (afterc > LMN_LIST_ID)
            {
              strcpy(moira_list_id, after[LM_LIST_ID]);
              strcpy(moira_user_id, after[LM_USER_ID]);
            }
        }
      else
        {
          if (afterc > LM_EXTRA_GID)
            strcpy(moira_list_id, after[LMN_LIST_ID]);
        }
    }
  else if (beforec)
    {
      if (beforec < LM_EXTRA_GID)
        return;
      if (!atoi(before[LM_EXTRA_ACTIVE]))
          return;
      ptr = before;
      if (!strcasecmp(ptr[LM_TYPE], "LIST"))
        return;
      strcpy(user_name, before[LM_MEMBER]);
      strcpy(group_name, before[LM_LIST]);
      strcpy(user_type, before[LM_TYPE]);
      if (!strcasecmp(ptr[LM_TYPE], "USER"))
        {
          if (beforec > LMN_LIST_ID)
            {
              strcpy(moira_list_id, before[LM_LIST_ID]);
              strcpy(moira_user_id, before[LM_USER_ID]);
            }
        }
      else
        {
          if (beforec > LM_EXTRA_GID)
            strcpy(moira_list_id, before[LMN_LIST_ID]);
        }
    }

  if (ptr == NULL)
    return;

  args[L_NAME] = ptr[LM_LIST];
  args[L_ACTIVE] = ptr[LM_EXTRA_ACTIVE];
  args[L_PUBLIC] = ptr[LM_EXTRA_PUBLIC];
  args[L_HIDDEN] = ptr[LM_EXTRA_HIDDEN];
  args[L_MAILLIST] = ptr[LM_EXTRA_MAILLIST];
  args[L_GROUP] = ptr[LM_EXTRA_GROUP];
  args[L_GID] = ptr[LM_EXTRA_GID];

  security_flag = 0;
  memset(group_ou, '\0', sizeof(group_ou));
  get_group_membership(group_membership, group_ou, &security_flag, args);
  if (strlen(group_ou) == 0)
    {
      com_err(whoami, 0, "couldn't find the group OU for group %s", group_name);
      return;
    }
  if (rc = process_group(ldap_handle, dn_path, moira_list_id, group_name, group_ou, group_membership, security_flag, CHECK_GROUPS))
    {
      if (rc != AD_NO_GROUPS_FOUND)
        {
          if (rc = process_group(ldap_handle, dn_path, moira_list_id, group_name, group_ou, group_membership, security_flag, CLEANUP_GROUPS))
            {
              if (rc != AD_NO_GROUPS_FOUND)
                {
                  if (afterc)
                    com_err(whoami, 0, "Couldn't add %s to group %s - unable to process group", user_name, group_name);
                  else
                    com_err(whoami, 0, "Couldn't remove %s from group %s - unable to process group", user_name, group_name);
                  return;
                }
            }
        }
    }
  if (rc == AD_NO_GROUPS_FOUND)
    {
      if (rc = moira_connect())
        {
          critical_alert("AD incremental",
                         "Error contacting Moira server : %s",
                         error_message(rc));
          return;
        }

      com_err(whoami, 0, "creating group %s", group_name);
      if (make_new_group(ldap_handle, dn_path, moira_list_id, ptr[LM_LIST], 
                         group_ou, group_membership, security_flag, 0))
        {
          moira_disconnect();
          return;
        }
      if (atoi(ptr[LM_EXTRA_ACTIVE]))
        {
          populate_group(ldap_handle, dn_path, ptr[LM_LIST], group_ou, 
                         group_membership, security_flag, moira_list_id);
        }
      moira_disconnect();
    }
  rc = 0;
  if (beforec)
    {
      com_err(whoami, 0, "removing user %s from list %s", user_name, group_name);
      pUserOu = user_ou;
      if (!strcasecmp(ptr[LM_TYPE], "STRING"))
        {
          if (contact_create(ldap_handle, dn_path, ptr[LM_MEMBER], contact_ou))
            return;
          pUserOu = contact_ou;
        }
      else if (!strcasecmp(ptr[LM_TYPE], "KERBEROS"))
        {
          if (contact_create(ldap_handle, dn_path, ptr[LM_MEMBER], kerberos_ou))
            return;
          pUserOu = kerberos_ou;
        }
      if (rc = member_remove(ldap_handle, dn_path, group_name,
                             group_ou, group_membership, ptr[LM_MEMBER], 
                             pUserOu, moira_list_id))
          com_err(whoami, 0, "couldn't remove %s from group %s", user_name, group_name);
      return;
    }

  com_err(whoami, 0, "Adding %s to list %s", user_name, group_name);
  pUserOu = user_ou;
  if (!strcasecmp(ptr[LM_TYPE], "STRING"))
    {
      if (contact_create(ldap_handle, dn_path, ptr[LM_MEMBER], contact_ou))
        return;
      pUserOu = contact_ou;
    }
  else if (!strcasecmp(ptr[LM_TYPE], "KERBEROS"))
    {
      if (contact_create(ldap_handle, dn_path, ptr[LM_MEMBER], kerberos_ou))
        return;
      pUserOu = kerberos_ou;
    }
  else if (!strcasecmp(ptr[LM_TYPE], "USER"))
    {
      if ((rc = check_user(ldap_handle, dn_path, ptr[LM_MEMBER], 
                           moira_user_id)) == AD_NO_USER_FOUND)
        {
          if (rc = moira_connect())
            {
              critical_alert("AD incremental", 
                             "Error connection to Moira : %s",
                             error_message(rc));
              return;
            }
          com_err(whoami, 0, "creating user %s", after[U_NAME]);
          av[0] = ptr[LM_MEMBER];
          call_args[0] = (char *)ldap_handle;
          call_args[1] = dn_path;
          call_args[2] = moira_user_id;
          call_args[3] = NULL;
          sid_base = NULL;
          sid_ptr = &sid_base;
          callback_rc = 0;
          if (rc = mr_query("get_user_account_by_login", 1, av, user_create,
                            call_args))
            {
              moira_disconnect();
              com_err(whoami, 0, "couldn't create user %s : %s",
                      ptr[LM_MEMBER], error_message(rc));
              return;
            }
          if (callback_rc)
            {
              moira_disconnect();
              com_err(whoami, 0, "couldn't create user %s", ptr[LM_MEMBER]);
              return;
            }
          sleep(1);
          if (sid_base != NULL)
            {
              sid_update(ldap_handle, dn_path);
              linklist_free(sid_base);
            }
        }
      else
        {
          if (rc != 0)
            return;
        }
      pUserOu = user_ou;
    }

  if (rc = member_add(ldap_handle, dn_path, group_name,
                  group_ou, group_membership, ptr[LM_MEMBER],
                  pUserOu, moira_list_id))
    {
      com_err(whoami, 0, "couldn't add %s to group %s", user_name, group_name);
    }
  return;
}


#define U_USER_ID 10

void do_user(LDAP *ldap_handle, char *dn_path, char *ldap_hostname, 
             char **before, int beforec, char **after, 
             int afterc)
{
  int   rc;
  char  *av[7];
  char  after_user_id[32];
  char  before_user_id[32];
  char  *call_args[7];

  if ((beforec == 0) && (afterc == 0))
    return;

  memset(after_user_id, '\0', sizeof(after_user_id));
  memset(before_user_id, '\0', sizeof(before_user_id));
  if (beforec > U_USER_ID)
    strcpy(before_user_id, before[U_USER_ID]);
  if (afterc > U_USER_ID)
    strcpy(after_user_id, after[U_USER_ID]);

  if ((beforec == 0) && (afterc == 0)) /*this case should never happen */
    return;

  if ((beforec == 0) && (afterc != 0)) /*this case only happens when the account*/
    return;                            /*account is first created but not usable*/

  if ((beforec != 0) && (afterc == 0)) /*this case only happens when the account*/
    {                                  /*is expunged*/
      if (atoi(before[U_STATE]) == 0)
        {
          com_err(whoami, 0, "expunging user %s from AD", before[U_NAME]);
          user_delete(ldap_handle, dn_path, before[U_NAME], before_user_id);
        }
      return;
    }

  /*process anything that gets here*/
  if ((rc = check_user(ldap_handle, dn_path, before[U_NAME], 
                       before_user_id)) == AD_NO_USER_FOUND)
    {
      if (rc = moira_connect())
        {
          critical_alert("AD incremental", 
                         "Error connection to Moira : %s",
                         error_message(rc));
          return;
        }
      com_err(whoami, 0, "creating user %s", after[U_NAME]);

      av[0] = after[U_NAME];
      call_args[0] = (char *)ldap_handle;
      call_args[1] = dn_path;
      call_args[2] = after_user_id;
      call_args[3] = NULL;
      sid_base = NULL;
      sid_ptr = &sid_base;
      callback_rc = 0;
      if (rc = mr_query("get_user_account_by_login", 1, av, user_create,
                        call_args))
        {
          moira_disconnect();
          com_err(whoami, 0, "couldn't create user %s : %s",
                  after[U_NAME], error_message(rc));
          return;
        }
      if (callback_rc)
        {
          moira_disconnect();
          com_err(whoami, 0, "couldn't create user %s", after[U_NAME]);
          return;
        }
      sleep(1);
      if (sid_base != NULL)
        {
          sid_update(ldap_handle, dn_path);
          linklist_free(sid_base);
        }
      return;
    }
  else
    {
      if (rc != 0)
        return;
    }
  if (strcmp(before[U_NAME], after[U_NAME]))
    {
      if ((check_string(before[U_NAME])) && (check_string(after[U_NAME])))
        {
          com_err(whoami, 0, "changing user %s to %s", 
                  before[U_NAME], after[U_NAME]);
          if ((rc = user_rename(ldap_handle, dn_path, before[U_NAME], 
                                after[U_NAME])) != LDAP_SUCCESS)
            {
              return;
            }
        }
    }
  com_err(whoami, 0, "updating user %s information", after[U_NAME]);
  rc = user_update(ldap_handle, dn_path, after[U_NAME],
                   after[U_UID], after[U_MITID], 
                   after_user_id, atoi(after[U_STATE]));
  return;
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
          critical_alert("AD incremental",
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

int group_rename(LDAP *ldap_handle, char *dn_path, 
                 char *before_group_name, char *before_group_membership, 
                 char *before_group_ou, int before_security_flag, char *before_desc,
                 char *after_group_name, char *after_group_membership, 
                 char *after_group_ou, int after_security_flag, char *after_desc,
                 char *MoiraId, char *filter)
{
  LDAPMod   *mods[20];
  char      old_dn[512];
  char      new_dn[512];
  char      new_dn_path[512];
  char      sam_name[256];
  char      *attr_array[3];
  char      *mitMoiraId_v[] = {NULL, NULL};
  char      *name_v[] = {NULL, NULL};
  char      *desc_v[] = {NULL, NULL};
  char      *samAccountName_v[] = {NULL, NULL};
  char      *groupTypeControl_v[] = {NULL, NULL};
  u_int     groupTypeControl = ADS_GROUP_TYPE_GLOBAL_GROUP;
  char      groupTypeControlStr[80];
  int       n;
  int       i;
  int       rc;
  LK_ENTRY  *group_base;
  int       group_count;

  if (!check_string(before_group_name))
    {
      com_err(whoami, 0, "invalid LDAP list name %s", before_group_name);
      return(AD_INVALID_NAME);
    }
  if (!check_string(after_group_name))
    {
      com_err(whoami, 0, "invalid LDAP list name %s", after_group_name);
      return(AD_INVALID_NAME);
    }

  group_count = 0;
  group_base = NULL;
  if (rc = ad_get_group(ldap_handle, dn_path, before_group_name, 
                        before_group_membership, 
                        MoiraId, "distinguishedName", &group_base, 
                        &group_count, filter))
    return(rc);

  if (group_count == 0)
    {
      return(AD_NO_GROUPS_FOUND);
    }
  if (group_count != 1)
    {
      com_err(whoami, 0,
              "multiple groups with MoiraId = %s exist in the AD",
              MoiraId);
      return(AD_MULTIPLE_GROUPS_FOUND);
    }
  strcpy(old_dn, group_base->value);

  linklist_free(group_base);
  group_base = NULL;
  group_count = 0;
  attr_array[0] = "sAMAccountName";
  attr_array[1] = NULL;
  if ((rc = linklist_build(ldap_handle, dn_path, filter, attr_array, 
                           &group_base, &group_count)) != 0)
    {
      com_err(whoami, 0, "LDAP server unable to get list %s dn : %s",
              after_group_name, ldap_err2string(rc));
      return(rc);
    }
  if (group_count != 1)
    {
      com_err(whoami, 0,
              "Unable to get sAMAccountName for group %s", 
              before_group_name);
      return(AD_LDAP_FAILURE);
    }

  strcpy(sam_name, group_base->value);
  linklist_free(group_base);
  group_base = NULL;
  group_count = 0;

  sprintf(new_dn_path, "%s,%s", after_group_ou, dn_path);
  sprintf(new_dn, "cn=%s", after_group_name);
  if ((rc = ldap_rename_s(ldap_handle, old_dn, new_dn, new_dn_path,
                          TRUE, NULL, NULL)) != LDAP_SUCCESS)
    {
      com_err(whoami, 0, "Couldn't rename list from %s to %s : %s",
              before_group_name, after_group_name, ldap_err2string(rc));
      return(rc);
    }

  name_v[0] = after_group_name;
  if (!strncmp(&sam_name[strlen(sam_name) - strlen("_group")], "_group", strlen("_group")))
    {
      sprintf(sam_name, "%s_group", after_group_name);
    }
  else
    {
      com_err(whoami, 0, "Couldn't rename list from %s to %s : sAMAccountName not found",
              before_group_name, after_group_name);
      return(rc);
    }
  samAccountName_v[0] = sam_name;
  if (after_security_flag)
    groupTypeControl |= ADS_GROUP_TYPE_SECURITY_ENABLED;
  sprintf(groupTypeControlStr, "%ld", groupTypeControl);
  groupTypeControl_v[0] = groupTypeControlStr;
  n = 0;
  ADD_ATTR("samAccountName", samAccountName_v, LDAP_MOD_REPLACE);
  ADD_ATTR("displayName", name_v, LDAP_MOD_REPLACE);
  desc_v[0] = after_desc;
  if (strlen(after_desc) == 0)
    desc_v[0] = NULL;
  ADD_ATTR("description", desc_v, LDAP_MOD_REPLACE);
  mitMoiraId_v[0] = MoiraId;
  ADD_ATTR("mitMoiraId", mitMoiraId_v, LDAP_MOD_REPLACE);
  ADD_ATTR("groupType", groupTypeControl_v, LDAP_MOD_REPLACE);
  mods[n] = NULL;
  sprintf(new_dn, "cn=%s,%s,%s", after_group_name, after_group_ou, dn_path);
  if ((rc = ldap_modify_s(ldap_handle, new_dn, mods)) != LDAP_SUCCESS)
    {
      com_err(whoami, 0, "After renaming, couldn't modify list data for %s : %s",
              after_group_name, ldap_err2string(rc));
    }
  for (i = 0; i < n; i++)
    free(mods[i]);
  return(rc);
}

int group_create(int ac, char **av, void *ptr)
{
  LDAPMod *mods[20];
  LK_ENTRY  *group_base;
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
  char *member_v[] = {NULL, NULL};
  char *name_v[] = {NULL, NULL};
  char *desc_v[] = {NULL, NULL};
  char *info_v[] = {NULL, NULL};
  char *mitMoiraId_v[] = {NULL, NULL};
  char *groupTypeControl_v[] = {NULL, NULL};
  char groupTypeControlStr[80];
  char group_membership[1];
  int  i;
  int  security_flag;
  u_int groupTypeControl = ADS_GROUP_TYPE_GLOBAL_GROUP;
  int  n;
  int  rc;
  int  group_count;
  int  updateGroup;
  char filter[128];
  char *attr_array[3];
  char **call_args;

  call_args = ptr;

  if (!check_string(av[L_NAME]))
    {
      com_err(whoami, 0, "invalid LDAP list name %s", av[L_NAME]);
      return(AD_INVALID_NAME);
    }

  updateGroup = (int)call_args[4];
  memset(group_ou, 0, sizeof(group_ou));
  memset(group_membership, 0, sizeof(group_membership));
  security_flag = 0;
  get_group_membership(group_membership, group_ou, &security_flag, av);
  strcpy(new_group_name, av[L_NAME]);
  sprintf(new_dn, "cn=%s,%s,%s", new_group_name, group_ou, call_args[1]);
  if (security_flag)
    groupTypeControl |= ADS_GROUP_TYPE_SECURITY_ENABLED;
  
  sprintf(sam_group_name, "%s_group", av[L_NAME]);

  if (!updateGroup)
    {

      sprintf(groupTypeControlStr, "%ld", groupTypeControl);
      groupTypeControl_v[0] = groupTypeControlStr;

      strcpy(cn_group_name, av[L_NAME]);

      samAccountName_v[0] = sam_group_name;
      name_v[0] = new_group_name;
      cn_v[0] = new_group_name;

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
          sprintf(info, "The Administrator of this list is: %s", av[L_ACE_NAME]);
          info_v[0] = info;
          ADD_ATTR("info", info_v, LDAP_MOD_ADD);
        }
      if (strlen(call_args[5]) != 0)
        {
          mitMoiraId_v[0] = call_args[5];
          ADD_ATTR("mitMoiraId", mitMoiraId_v, LDAP_MOD_ADD);
        }
      mods[n] = NULL;

      rc = ldap_add_ext_s((LDAP *)call_args[0], new_dn, mods, NULL, NULL);

      for (i = 0; i < n; i++)
        free(mods[i]);
      if ((rc != LDAP_SUCCESS) && (rc != LDAP_ALREADY_EXISTS))
        {
          com_err(whoami, 0, "Unable to create/update list %s in AD : %s",
                  av[L_NAME], ldap_err2string(rc));
          callback_rc = rc;
          return(rc);
        }
    }
  if ((rc == LDAP_ALREADY_EXISTS) || (updateGroup))
    {
      n = 0;
      desc_v[0] = NULL;
      if (strlen(av[L_DESC]) != 0)
        desc_v[0] = av[L_DESC];
      ADD_ATTR("description", desc_v, LDAP_MOD_REPLACE);
      info_v[0] = NULL;
      if (strlen(av[L_ACE_NAME]) != 0)
        {
          sprintf(info, "The Administrator of this list is: %s", av[L_ACE_NAME]);
          info_v[0] = info;
        }
      ADD_ATTR("info", info_v, LDAP_MOD_REPLACE);
      if (strlen(call_args[5]) != 0)
        {
          mitMoiraId_v[0] = call_args[5];
          ADD_ATTR("mitMoiraId", mitMoiraId_v, LDAP_MOD_REPLACE);
        }
      if (!(atoi(av[L_ACTIVE])))
        {
          member_v[0] = NULL;
          ADD_ATTR("member", member_v, LDAP_MOD_REPLACE);
        }
      mods[n] = NULL;
      rc = ldap_modify_s((LDAP *)call_args[0], new_dn, mods);
      for (i = 0; i < n; i++)
        free(mods[i]);
    }

  sprintf(filter, "(sAMAccountName=%s)", sam_group_name);
  if (strlen(call_args[5]) != 0)
    sprintf(filter, "(&(objectClass=group) (mitMoiraId=%s))", call_args[5]);
  attr_array[0] = "objectSid";
  attr_array[1] = NULL;
  group_count = 0;
  group_base = NULL;
  if ((rc = linklist_build((LDAP *)call_args[0], call_args[1], filter, attr_array, 
                           &group_base, &group_count)) == LDAP_SUCCESS)
    {
      if (group_count != 1)
        {
          if (strlen(call_args[5]) != 0)
            {
              linklist_free(group_base);
              group_count = 0;
              group_base = NULL;
              sprintf(filter, "(sAMAccountName=%s)", sam_group_name);
              rc = linklist_build((LDAP *)call_args[0], call_args[1], filter, 
                                  attr_array, &group_base, &group_count);
            }
        }
      if (group_count == 1)
        {
          (*sid_ptr) = group_base;
          (*sid_ptr)->member = strdup(av[L_NAME]);
          (*sid_ptr)->type = (char *)GROUPS;
          sid_ptr = &(*sid_ptr)->next;
        }
      else
        {
          if (group_base != NULL)
            linklist_free(group_base);
        }
    }
  else
    {
      if (group_base != NULL)
        linklist_free(group_base);
    }
  return(LDAP_SUCCESS);
}

int group_delete(LDAP *ldap_handle, char *dn_path, char *group_name, 
                 char *group_membership, char *MoiraId)
{
  LK_ENTRY  *group_base;
  char      temp[512];
  char      filter[128];
  int       group_count;
  int       rc;

  if (!check_string(group_name))
    {
      com_err(whoami, 0, "invalid LDAP list name %s", group_name);
      return(AD_INVALID_NAME);
    }

  memset(filter, '\0', sizeof(filter));
  group_count = 0;
  group_base = NULL;
  sprintf(temp, "%s,%s", group_ou_root, dn_path);
  if (rc = ad_get_group(ldap_handle, temp, group_name, 
                        group_membership, MoiraId, 
                        "distinguishedName", &group_base, 
                        &group_count, filter))
    return(rc);

  if (group_count == 1)
    {
      if ((rc = ldap_delete_s(ldap_handle, group_base->value)) != LDAP_SUCCESS)
        {
          linklist_free(group_base);
          com_err(whoami, 0, "Unable to delete list %s from AD : %s",
                  group_name, ldap_err2string(rc));
          return(rc);
        }
      linklist_free(group_base);
    }
  else
    {
      linklist_free(group_base);
      com_err(whoami, 0, "Unable to find list %s in AD.", group_name);
      return(AD_NO_GROUPS_FOUND);
    }

  return(0);
}

int process_lists(int ac, char **av, void *ptr)
{
  int   rc;
  int   security_flag;
  char  group_ou[256];
  char  group_membership[2];
  char  **call_args;

  call_args = ptr;

  security_flag = 0;
  memset(group_ou, '\0', sizeof(group_ou));
  memset(group_membership, '\0', sizeof(group_membership));
  get_group_membership(group_membership, group_ou, &security_flag, av);
  rc = member_add((LDAP *)call_args[0], (char *)call_args[1], av[L_NAME],
                  group_ou, group_membership, call_args[2], 
                  (char *)call_args[3], "");
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
      if (!((int)call_args[3] & MOIRA_USERS))
        return(0);
    }
  else if (!strcmp(av[ACE_TYPE], "STRING"))
    {
      if (!((int)call_args[3] & MOIRA_STRINGS))
        return(0);
      if (contact_create((LDAP *)call_args[0], call_args[1], temp, contact_ou))
        return(0);
    }
  else if (!strcmp(av[ACE_TYPE], "LIST"))
    {
      if (!((int)call_args[3] & MOIRA_LISTS))
        return(0);
    }
  else if (!strcmp(av[ACE_TYPE], "KERBEROS"))
    {
      if (!((int)call_args[3] & MOIRA_KERBEROS))
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

int member_remove(LDAP *ldap_handle, char *dn_path, char *group_name, 
                  char *group_ou, char *group_membership, char *user_name,
                  char *UserOu, char *MoiraId)
{
  char        distinguished_name[1024];
  char        *modvalues[2];
  char        temp[256];
  char        filter[128];
  int         group_count;
  int         i;
  int         n;
  LDAPMod     *mods[20];
  LK_ENTRY    *group_base;
  ULONG       rc;

  if (!check_string(group_name))
    return(AD_INVALID_NAME);

  memset(filter, '\0', sizeof(filter));
  group_base = NULL;
  group_count = 0;
  if (rc = ad_get_group(ldap_handle, dn_path, group_name, 
                        group_membership, MoiraId, 
                        "distinguishedName", &group_base, 
                        &group_count, filter))
    return(rc);

  if (group_count != 1)
    {
      com_err(whoami, 0, "LDAP server unable to find list %s in AD",
              group_name);
      linklist_free(group_base);
      group_base = NULL;
      group_count = 0;
      goto cleanup;
    }
  strcpy(distinguished_name, group_base->value);
  linklist_free(group_base);
  group_base = NULL;
  group_count = 0;

  sprintf(temp, "CN=%s,%s,%s", user_name, UserOu, dn_path);
  modvalues[0] = temp;
  modvalues[1] = NULL;

  n = 0;
  ADD_ATTR("member", modvalues, LDAP_MOD_DELETE);
  mods[n] = NULL;
  rc = ldap_modify_s(ldap_handle, distinguished_name, mods);
  for (i = 0; i < n; i++)
    free(mods[i]);
  if ((!strcmp(UserOu, kerberos_ou)) || (!strcmp(UserOu, contact_ou)))
    {
      if (rc == LDAP_UNWILLING_TO_PERFORM)
        rc = LDAP_SUCCESS;
    }
  if (rc != LDAP_SUCCESS)
    {
      com_err(whoami, 0, "LDAP server unable to modify list %s members : %s",
              group_name, ldap_err2string(rc));
      goto cleanup;
    }

cleanup:
  return(rc);
}

int member_add(LDAP *ldap_handle, char *dn_path, char *group_name, 
               char *group_ou, char *group_membership, char *user_name, 
               char *UserOu, char *MoiraId)
{
  char        distinguished_name[1024];
  char        *modvalues[2];
  char        temp[256];
  char        filter[128];
  int         group_count;
  int         n;
  int         i;
  LDAPMod     *mods[20];
  LK_ENTRY    *group_base;
  ULONG       rc;

  if (!check_string(group_name))
    return(AD_INVALID_NAME);

  rc = 0;
  memset(filter, '\0', sizeof(filter));
  group_base = NULL;
  group_count = 0;
  if (rc = ad_get_group(ldap_handle, dn_path, group_name, 
                        group_membership, MoiraId, 
                        "distinguishedName", &group_base, 
                        &group_count, filter))
    return(rc);

  if (group_count != 1)
    {
      linklist_free(group_base);
      group_base = NULL;
      group_count = 0;
      com_err(whoami, 0, "LDAP server unable to find list %s in AD",
              group_name);
      return(AD_MULTIPLE_GROUPS_FOUND);
    }

  strcpy(distinguished_name, group_base->value);
  linklist_free(group_base);
  group_base = NULL;
  group_count = 0;

  sprintf(temp, "CN=%s,%s,%s", user_name, UserOu, dn_path);
  modvalues[0] = temp;
  modvalues[1] = NULL;

  n = 0;
  ADD_ATTR("member", modvalues, LDAP_MOD_ADD);
  mods[n] = NULL;
  rc = ldap_modify_s(ldap_handle, distinguished_name, mods);
  if (rc == LDAP_ALREADY_EXISTS)
    rc = LDAP_SUCCESS;
  if ((!strcmp(UserOu, contact_ou)) || (!strcmp(UserOu, kerberos_ou)))
    {
      if (rc == LDAP_UNWILLING_TO_PERFORM)
        rc = LDAP_SUCCESS;
    }
  for (i = 0; i < n; i++)
    free(mods[i]);
  if (rc != LDAP_SUCCESS)
    {
      com_err(whoami, 0, "LDAP server unable to add %s to list %s as a member : %s",
              user_name, group_name, ldap_err2string(rc));
    }

  return(rc);
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
      com_err(whoami, 0, "invalid LDAP name %s", user);
      return(AD_INVALID_NAME);
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
      com_err(whoami, 0, "could not create contact %s : %s",
              user, ldap_err2string(rc));
      return(rc);
    }
  return(0);
}

int user_update(LDAP *ldap_handle, char *dn_path, char *user_name,
                char *Uid, char *MitId, char *MoiraId, int State)
{
  LDAPMod   *mods[20];
  LK_ENTRY  *group_base;
  int  group_count;
  char distinguished_name[256];
  char *mitMoiraId_v[] = {NULL, NULL};
  char *uid_v[] = {NULL, NULL};
  char *mitid_v[] = {NULL, NULL};
  char *homedir_v[] = {NULL, NULL};
  char *winProfile_v[] = {NULL, NULL};
  char *drives_v[] = {NULL, NULL};
  char *userAccountControl_v[] = {NULL, NULL};
  char userAccountControlStr[80];
  int  n;
  int  rc;
  int  i;
  u_int userAccountControl = UF_NORMAL_ACCOUNT | UF_DONT_EXPIRE_PASSWD | UF_PASSWD_CANT_CHANGE;
  char filter[128];
  char *attr_array[3];
  char **hp;
  char path[256];
  char temp[256];
  char winPath[256];
  char winProfile[256];

  if (!check_string(user_name))
    {
      com_err(whoami, 0, "invalid LDAP user name %s", user_name);
      return(AD_INVALID_NAME);
    }

  group_count = 0;
  group_base = NULL;

  if (strlen(MoiraId) != 0)
    {
      sprintf(filter, "(&(objectClass=user)(mitMoiraId=%s))", MoiraId);
      attr_array[0] = "cn";
      attr_array[1] = NULL;
      if ((rc = linklist_build(ldap_handle, dn_path, filter, attr_array, 
                               &group_base, &group_count)) != 0)
        {
          com_err(whoami, 0, "LDAP server couldn't process user %s : %s",
                  user_name, ldap_err2string(rc));
          return(rc);
        }
    }
  if (group_count != 1)
    {
      linklist_free(group_base);
      group_base = NULL;
      group_count = 0;
      sprintf(filter, "(sAMAccountName=%s)", user_name);
      attr_array[0] = "cn";
      attr_array[1] = NULL;
      sprintf(temp, "%s,%s", user_ou, dn_path);
      if ((rc = linklist_build(ldap_handle, temp, filter, attr_array, 
                               &group_base, &group_count)) != 0)
        {
          com_err(whoami, 0, "LDAP server couldn't process user %s : %s",
                  user_name, ldap_err2string(rc));
          return(rc);
        }
    }

  if (group_count != 1)
    {
      com_err(whoami, 0, "LDAP server unable to find user %s in AD",
              user_name);
      linklist_free(group_base);
      return(AD_NO_USER_FOUND);
    }
  strcpy(distinguished_name, group_base->dn);

  linklist_free(group_base);
  group_count = 0;
  n = 0;
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
  uid_v[0] = Uid;
  if (strlen(Uid) == 0)
    uid_v[0] = NULL;
  ADD_ATTR("uid", uid_v, LDAP_MOD_REPLACE);
  ADD_ATTR("uidNumber", uid_v, LDAP_MOD_REPLACE);
  mitid_v[0] = MitId;
  if (strlen(MitId) == 0)
    mitid_v[0] = NULL;
  ADD_ATTR("employeeID", mitid_v, LDAP_MOD_REPLACE);
  mitMoiraId_v[0] = MoiraId;
  if (strlen(MoiraId) == 0)
    mitMoiraId_v[0] = NULL;
  ADD_ATTR("mitMoiraId", mitMoiraId_v, LDAP_MOD_REPLACE);
  if ((State != US_NO_PASSWD) && (State != US_REGISTERED))
    userAccountControl |= UF_ACCOUNTDISABLE;
  sprintf(userAccountControlStr, "%ld", userAccountControl);
  userAccountControl_v[0] = userAccountControlStr;
  ADD_ATTR("userAccountControl", userAccountControl_v, LDAP_MOD_REPLACE);
  mods[n] = NULL;
  if ((rc = ldap_modify_s(ldap_handle, distinguished_name, mods)) != LDAP_SUCCESS)
    {
      com_err(whoami, 0, "Couldn't modify user data for %s : %s",
              user_name, ldap_err2string(rc));
    }
  for (i = 0; i < n; i++)
    free(mods[i]);

  if (hp != NULL)
    {
      i = 0;
      while (hp[i])
        {
          free(hp[i]);
          i++;
        }
    }

  return(rc);
}

int user_rename(LDAP *ldap_handle, char *dn_path, char *before_user_name, 
                char *user_name)
{
  LDAPMod *mods[20];
  char new_dn[256];
  char old_dn[256];
  char upn[256];
  char temp[128];
  char *userPrincipalName_v[] = {NULL, NULL};
  char *altSecurityIdentities_v[] = {NULL, NULL};
  char *name_v[] = {NULL, NULL};
  char *samAccountName_v[] = {NULL, NULL};
  int  n;
  int  rc;
  int  i;

  if (!check_string(before_user_name))
    {
      com_err(whoami, 0, "invalid LDAP user name %s", before_user_name);
      return(AD_INVALID_NAME);
    }
  if (!check_string(user_name))
    {
      com_err(whoami, 0, "invalid LDAP user name %s", user_name);
      return(AD_INVALID_NAME);
    }

  strcpy(user_name, user_name);
  sprintf(old_dn, "cn=%s,%s,%s", before_user_name, user_ou, dn_path);
  sprintf(new_dn, "cn=%s", user_name);
  if ((rc = ldap_rename_s(ldap_handle, old_dn, new_dn, NULL, TRUE, 
                           NULL, NULL)) != LDAP_SUCCESS)
    {
      com_err(whoami, 0, "Couldn't rename user from %s to %s : %s",
              before_user_name, user_name, ldap_err2string(rc));
      return(rc);
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
  mods[n] = NULL;
  sprintf(new_dn, "cn=%s,%s,%s", user_name, user_ou, dn_path);
  if ((rc = ldap_modify_s(ldap_handle, new_dn, mods)) != LDAP_SUCCESS)
    {
      com_err(whoami, 0, "After renaming, couldn't modify user data for %s : %s",
              user_name, ldap_err2string(rc));
    }
  for (i = 0; i < n; i++)
    free(mods[i]);
  return(rc);
}

int filesys_process(LDAP *ldap_handle, char *dn_path, char *fs_name, 
                    char *fs_type, char *fs_pack, int operation)
{
  char  distinguished_name[256];
  char  winPath[256];
  char  winProfile[256];
  char  filter[128];
  char  *attr_array[3];
  char  *homedir_v[] = {NULL, NULL};
  char  *winProfile_v[] = {NULL, NULL};
  char  *drives_v[] = {NULL, NULL};
  int   group_count;
  int   n;
  int   rc;
  int   i;
  LDAPMod   *mods[20];
  LK_ENTRY  *group_base;

  if (!check_string(fs_name))
    {
      com_err(whoami, 0, "invalid filesys name %s", fs_name);
      return(AD_INVALID_NAME);
    }

  if (strcmp(fs_type, "AFS"))
    {
      com_err(whoami, 0, "invalid filesys type %s", fs_type);
      return(AD_INVALID_FILESYS);
    }

  group_count = 0;
  group_base = NULL;
  sprintf(filter, "(sAMAccountName=%s)", fs_name);
  attr_array[0] = "cn";
  attr_array[1] = NULL;
  if ((rc = linklist_build(ldap_handle, dn_path, filter, attr_array, 
                           &group_base, &group_count)) != 0)
    {
      com_err(whoami, 0, "LDAP server couldn't process filesys %s : %s",
              fs_name, ldap_err2string(rc));
      return(rc);
    }

  if (group_count != 1)
    {
      linklist_free(group_base);
      com_err(whoami, 0, "LDAP server unable to find user %s in AD",
              fs_name);
      return(LDAP_NO_SUCH_OBJECT);
    }
  strcpy(distinguished_name, group_base->dn);
  linklist_free(group_base);
  group_count = 0;

  n = 0;
  if (operation == LDAP_MOD_ADD)
    {
      memset(winPath, 0, sizeof(winPath));
      AfsToWinAfs(fs_pack, winPath);
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

  rc = ldap_modify_s(ldap_handle, distinguished_name, mods);
  if (rc != LDAP_SUCCESS)
    {
      com_err(whoami, 0, "Couldn't modify user data for filesys %s : %s",
              fs_name, ldap_err2string(rc));
    }
  for (i = 0; i < n; i++)
    free(mods[i]);

  return(rc);
}

int user_create(int ac, char **av, void *ptr)
{
  LK_ENTRY  *group_base;
  LDAPMod *mods[20];
  char new_dn[256];
  char user_name[256];
  char sam_name[256];
  char upn[256];
  char *cn_v[] = {NULL, NULL};
  char *objectClass_v[] = {"top", "person", 
                           "organizationalPerson", 
                           "user", NULL};

  char *samAccountName_v[] = {NULL, NULL};
  char *altSecurityIdentities_v[] = {NULL, NULL};
  char *mitMoiraId_v[] = {NULL, NULL};
  char *name_v[] = {NULL, NULL};
  char *desc_v[] = {NULL, NULL};
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
  int  group_count;
  char filter[128];
  char *attr_array[3];
  char **call_args;

  call_args = ptr;

  if (!check_string(av[U_NAME]))
    {
      callback_rc = AD_INVALID_NAME;
      com_err(whoami, 0, "invalid LDAP user name %s", av[U_NAME]);
      return(AD_INVALID_NAME);
    }

  strcpy(user_name, av[U_NAME]);
  sprintf(upn, "%s@%s", user_name, ldap_domain);
  sprintf(sam_name, "%s", av[U_NAME]);
  samAccountName_v[0] = sam_name;
  if ((atoi(av[U_STATE]) != US_NO_PASSWD) && (atoi(av[U_STATE]) != US_REGISTERED))
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
  if (strlen(call_args[2]) != 0)
    {
      mitMoiraId_v[0] = call_args[2];
      ADD_ATTR("mitMoiraId", mitMoiraId_v, LDAP_MOD_ADD);
    }
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
  if ((rc != LDAP_SUCCESS) && (rc != LDAP_ALREADY_EXISTS))
    {
      com_err(whoami, 0, "could not create user %s : %s",
              user_name, ldap_err2string(rc));
      callback_rc = rc;
      return(rc);
    }
  if (rc == LDAP_SUCCESS)
    {
      if ((rc = set_password(sam_name, "", ldap_domain)) != 0)
        {
          com_err(whoami, 0, "Couldn't set password for user %s : %ld",
                  user_name, rc);
        }
    }
  sprintf(filter, "(sAMAccountName=%s)", av[U_NAME]);
  if (strlen(call_args[2]) != 0)
    sprintf(filter, "(&(objectClass=user)(mitMoiraId=%s))", call_args[2]);
  attr_array[0] = "objectSid";
  attr_array[1] = NULL;
  group_count = 0;
  group_base = NULL;
  if ((rc = linklist_build((LDAP *)call_args[0], call_args[1], filter, attr_array, 
                           &group_base, &group_count)) == LDAP_SUCCESS)
    {
      if (group_count != 1)
        {
          if (strlen(call_args[2]) != 0)
            {
              linklist_free(group_base);
              group_count = 0;
              group_base = NULL;
              sprintf(filter, "(sAMAccountName=%s)", av[U_NAME]);
              rc = linklist_build((LDAP *)call_args[0], call_args[1], filter, 
                                  attr_array, &group_base, &group_count);
            }
        }
      if (group_count == 1)
        {
          (*sid_ptr) = group_base;
          (*sid_ptr)->member = strdup(av[U_NAME]);
          (*sid_ptr)->type = (char *)GROUPS;
          sid_ptr = &(*sid_ptr)->next;
        }
      else
        {
          if (group_base != NULL)
            linklist_free(group_base);
        }
    }
  else
    {
      if (group_base != NULL)
        linklist_free(group_base);
    }
  return(0);
}

int user_change_status(LDAP *ldap_handle, char *dn_path, 
                       char *user_name, char *MoiraId,
                       int operation)
{
  char      filter[128];
  char      *attr_array[3];
  char      temp[256];
  char      distinguished_name[1024];
  char      **modvalues;
  char      *mitMoiraId_v[] = {NULL, NULL};
  LDAPMod   *mods[20];
  LK_ENTRY  *group_base;
  int       group_count;
  int       rc;
  int       i;
  int       n;
  ULONG     ulongValue;

  if (!check_string(user_name))
    {
      com_err(whoami, 0, "invalid LDAP user name %s", user_name);
      return(AD_INVALID_NAME);
    }

  group_count = 0;
  group_base = NULL;

  if (strlen(MoiraId) != 0)
    {
      sprintf(filter, "(&(objectClass=user)(mitMoiraId=%s))", MoiraId);
      attr_array[0] = "UserAccountControl";
      attr_array[1] = NULL;
      if ((rc = linklist_build(ldap_handle, dn_path, filter, attr_array, 
                               &group_base, &group_count)) != 0)
        {
          com_err(whoami, 0, "LDAP server couldn't process user %s : %s",
                  user_name, ldap_err2string(rc));
          return(rc);
        }
    }
  if (group_count != 1)
    {
      linklist_free(group_base);
      group_count = 0;
      group_base = NULL;
      sprintf(filter, "(sAMAccountName=%s)", user_name);
      attr_array[0] = "UserAccountControl";
      attr_array[1] = NULL;
      if ((rc = linklist_build(ldap_handle, dn_path, filter, attr_array, 
                               &group_base, &group_count)) != 0)
        {
          com_err(whoami, 0, "LDAP server couldn't process user %s : %s",
                  user_name, ldap_err2string(rc));
          return(rc);
        }
    }

  if (group_count != 1)
    {
      linklist_free(group_base);
      com_err(whoami, 0, "LDAP server unable to find user %s in AD",
              user_name);
      return(LDAP_NO_SUCH_OBJECT);
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
  if (strlen(MoiraId) != 0)
    {
    mitMoiraId_v[0] = MoiraId;
    ADD_ATTR("mitMoiraId", mitMoiraId_v, LDAP_MOD_REPLACE);
    }
  mods[n] = NULL;
  rc = ldap_modify_s(ldap_handle, distinguished_name, mods);
  for (i = 0; i < n; i++)
    free(mods[i]);
  free_values(modvalues);
  if (rc != LDAP_SUCCESS)
    {
      com_err(whoami, 0, "LDAP server could not change status of user %s : %s",
              user_name, ldap_err2string(rc));
    }
cleanup:
  return(rc);
}

int user_delete(LDAP *ldap_handle, char *dn_path, 
                char *u_name, char *MoiraId)
{
  char      filter[128];
  char      *attr_array[3];
  char      distinguished_name[1024];
  char      user_name[512];
  LK_ENTRY  *group_base;
  int       group_count;
  int       rc;

  if (!check_string(u_name))
    return(AD_INVALID_NAME);

  strcpy(user_name, u_name);
  group_count = 0;
  group_base = NULL;

  if (strlen(MoiraId) != 0)
    {
      sprintf(filter, "(&(objectClass=user)(mitMoiraId=%s))", MoiraId);
      attr_array[0] = "name";
      attr_array[1] = NULL;
      if ((rc = linklist_build(ldap_handle, dn_path, filter, attr_array, 
                               &group_base, &group_count)) != 0)
        {
          com_err(whoami, 0, "LDAP server couldn't process user %s : %s",
                  user_name, ldap_err2string(rc));
          goto cleanup;
        }
    }
  if (group_count != 1)
    {
      linklist_free(group_base);
      group_count = 0;
      group_base = NULL;
      sprintf(filter, "(sAMAccountName=%s)", user_name);
      attr_array[0] = "name";
      attr_array[1] = NULL;
      if ((rc = linklist_build(ldap_handle, dn_path, filter, attr_array, 
                               &group_base, &group_count)) != 0)
        {
          com_err(whoami, 0, "LDAP server couldn't process user %s : %s",
                  user_name, ldap_err2string(rc));
          goto cleanup;
        }
    }

  if (group_count != 1)
    {
      com_err(whoami, 0, "LDAP server unable to find user %s in AD",
              user_name);
      goto cleanup;
    }

  strcpy(distinguished_name, group_base->dn);
  if (rc = ldap_delete_s(ldap_handle, distinguished_name))
    {
      com_err(whoami, 0, "LDAP server couldn't process user %s : %s",
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
      if (!ptr->member)
        continue;
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
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* @ - O */
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

int make_new_group(LDAP *ldap_handle, char *dn_path, char *MoiraId, 
                   char *group_name, char *group_ou, char *group_membership, 
                   int group_security_flag, int updateGroup)
{
  char  *av[3];
  char  *call_args[7];
  int   rc;

  av[0] = group_name;
  call_args[0] = (char *)ldap_handle;
  call_args[1] = dn_path;
  call_args[2] = group_name;
  call_args[3] = (char *)(MOIRA_USERS | MOIRA_KERBEROS | MOIRA_STRINGS);
  call_args[4] = (char *)updateGroup;
  call_args[5] = MoiraId;
  call_args[6] = NULL;
  sid_base = NULL;
  sid_ptr = &sid_base;
  callback_rc = 0;
  if (rc = mr_query("get_list_info", 1, av, group_create, call_args))
    {
      moira_disconnect();
      com_err(whoami, 0, "Couldn't create list %s : %s", group_name, error_message(rc));
      return(rc);
    }
  if (callback_rc)
    {
      moira_disconnect();
      com_err(whoami, 0, "Couldn't create list %s", group_name);
      return(callback_rc);
    }

  if (sid_base != NULL)
    {
      sid_update(ldap_handle, dn_path);
      linklist_free(sid_base);
      sid_base = NULL;
    }
  return(0);
}

int populate_group(LDAP *ldap_handle, char *dn_path, char *group_name, 
                   char *group_ou, char *group_membership, 
                   int group_security_flag, char *MoiraId)
{
  char      *av[3];
  char      *call_args[7];
  char      *pUserOu;
  LK_ENTRY  *ptr;
  int       rc;

  com_err(whoami, 0, "Populating group %s", group_name);
  av[0] = group_name;
  call_args[0] = (char *)ldap_handle;
  call_args[1] = dn_path;
  call_args[2] = group_name;
  call_args[3] = (char *)(MOIRA_USERS | MOIRA_KERBEROS | MOIRA_STRINGS);
  call_args[4] = NULL;
  member_base = NULL;
  if (rc = mr_query("get_end_members_of_list", 1, av,
                    member_list_build, call_args))
    {
      com_err(whoami, 0, "Couldn't populate list %s : %s", 
              group_name, error_message(rc));
      return(3);
    }
  if (member_base != NULL)
    {
      ptr = member_base;
      while (ptr != NULL)
        {
          if (!strcasecmp(ptr->type, "LIST"))
            {
              ptr = ptr->next;
              continue;
            }
          pUserOu = user_ou;
          if (!strcasecmp(ptr->type, "STRING"))
            {
              if (contact_create(ldap_handle, dn_path, ptr->member, contact_ou))
                return(3);
              pUserOu = contact_ou;
            }
          else if (!strcasecmp(ptr->type, "KERBEROS"))
            {
              if (contact_create(ldap_handle, dn_path, ptr->member, kerberos_ou))
                return(3);
              pUserOu = kerberos_ou;
            }
          rc = member_add(ldap_handle, dn_path, group_name,
                          group_ou, group_membership, ptr->member, 
                          pUserOu, MoiraId);
          ptr = ptr->next;
        }
      linklist_free(member_base);
      member_base = NULL;
    }
  return(0);
}

int process_group(LDAP *ldap_handle, char *dn_path, char *MoiraId, 
                  char *group_name, char *group_ou, char *group_membership, 
                  int group_security_flag, int type)
{
  char      before_desc[512];
  char      before_name[256];
  char      before_group_ou[256];
  char      before_group_membership[2];
  char      distinguishedName[256];
  char      ad_distinguishedName[256];
  char      filter[128];
  char      *attr_array[3];
  int       before_security_flag;
  int       group_count;
  int       rc;
  LK_ENTRY  *group_base;
  LK_ENTRY  *ptr;
  char      ou_both[512];
  char      ou_security[512];
  char      ou_distribution[512];
  char      ou_neither[512];

  memset(ad_distinguishedName, '\0', sizeof(ad_distinguishedName));
  sprintf(distinguishedName, "CN=%s,%s,%s", group_name, group_ou, dn_path);


  memset(filter, '\0', sizeof(filter));
  group_base = NULL;
  group_count = 0;
  if (rc = ad_get_group(ldap_handle, dn_path, group_name, 
                        "*", MoiraId, 
                        "distinguishedName", &group_base, 
                        &group_count, filter))
    return(rc);

  if (type == CHECK_GROUPS)
    {
      if (group_count == 1)
        {
          if (!strcasecmp(group_base->value, distinguishedName))
            {
              linklist_free(group_base);
              return(0);
            }
        }
      linklist_free(group_base);
      if (group_count == 0)
        return(AD_NO_GROUPS_FOUND);
      if (group_count == 1)
        return(AD_WRONG_GROUP_DN_FOUND);
      return(AD_MULTIPLE_GROUPS_FOUND);
    }
  if (group_count == 0)
    {
      return(AD_NO_GROUPS_FOUND);
    }
  if (group_count > 1)
    {
      ptr = group_base;
      while (ptr != NULL)
        {
          if (!strcasecmp(distinguishedName, ptr->value))
            break;
          ptr = ptr->next;
        }
      if (ptr == NULL)
        {
          com_err(whoami, 0, "%d groups with moira id = %s", group_count, MoiraId);
          ptr = group_base;
          while (ptr != NULL)
            {
              com_err(whoami, 0, "%s with moira id = %s", ptr->value, MoiraId);
              ptr = ptr->next;
            }
          linklist_free(group_base);
          return(AD_MULTIPLE_GROUPS_FOUND);
        } 
      ptr = group_base;
      while (ptr != NULL)
        {
          if (strcasecmp(distinguishedName, ptr->value))
            rc = ldap_delete_s(ldap_handle, ptr->value);
          ptr = ptr->next;
        }
      linklist_free(group_base);
      memset(filter, '\0', sizeof(filter));
      group_base = NULL;
      group_count = 0;
      if (rc = ad_get_group(ldap_handle, dn_path, group_name, 
                            "*", MoiraId, 
                            "distinguishedName", &group_base, 
                            &group_count, filter))
        return(rc);
      if (group_count == 0)
        return(AD_NO_GROUPS_FOUND);
      if (group_count > 1)
        return(AD_MULTIPLE_GROUPS_FOUND);
    }

  strcpy(ad_distinguishedName, group_base->value);
  linklist_free(group_base);
  group_base = NULL;
  group_count = 0;

  attr_array[0] = "sAMAccountName";
  attr_array[1] = NULL;
  if ((rc = linklist_build(ldap_handle, dn_path, filter, attr_array, 
                           &group_base, &group_count)) != 0)
    {
      com_err(whoami, 0, "LDAP server unable to get list info with MoiraId = %s: %s",
               MoiraId, ldap_err2string(rc));
      return(rc);
    }
  sprintf(filter, "(sAMAccountName=%s)", group_base->value);

  if (!strcasecmp(ad_distinguishedName, distinguishedName))
    {
      linklist_free(group_base);
      group_base = NULL;
      group_count = 0;
      return(0);
    }
  linklist_free(group_base);
  group_base = NULL;
  group_count = 0;
  memset(ou_both, '\0', sizeof(ou_both));
  memset(ou_security, '\0', sizeof(ou_security));
  memset(ou_distribution, '\0', sizeof(ou_distribution));
  memset(ou_neither, '\0', sizeof(ou_neither));
  memset(before_name, '\0', sizeof(before_name));
  memset(before_desc, '\0', sizeof(before_desc));
  memset(before_group_membership, '\0', sizeof(before_group_membership));
  attr_array[0] = "name";
  attr_array[1] = NULL;
  if ((rc = linklist_build(ldap_handle, dn_path, filter, attr_array, 
                           &group_base, &group_count)) != 0)
    {
      com_err(whoami, 0, "LDAP server unable to get list name with MoiraId = %s: %s",
              MoiraId, ldap_err2string(rc));
      return(rc);
    }
  strcpy(before_name, group_base->value);
  linklist_free(group_base);
  group_base = NULL;
  group_count = 0;
  attr_array[0] = "description";
  attr_array[1] = NULL;
  if ((rc = linklist_build(ldap_handle, dn_path, filter, attr_array, 
                           &group_base, &group_count)) != 0)
    {
      com_err(whoami, 0, 
              "LDAP server unable to get list description with MoiraId = %s: %s",
              MoiraId, ldap_err2string(rc));
      return(rc);
    }
  if (group_count != 0)
    {
      strcpy(before_desc, group_base->value);
      linklist_free(group_base);
      group_base = NULL;
      group_count = 0;
    }
  change_to_lower_case(ad_distinguishedName);  
  strcpy(ou_both, group_ou_both);
  change_to_lower_case(ou_both);
  strcpy(ou_security, group_ou_security);
  change_to_lower_case(ou_security);
  strcpy(ou_distribution, group_ou_distribution);
  change_to_lower_case(ou_distribution);
  strcpy(ou_neither, group_ou_neither);
  change_to_lower_case(ou_neither);
  if (strstr(ad_distinguishedName, ou_both))
    {
      strcpy(before_group_ou, group_ou_both);
      before_group_membership[0] = 'B';
      before_security_flag = 1;
    }
  else if (strstr(ad_distinguishedName, ou_security))
    {
      strcpy(before_group_ou, group_ou_security);
      before_group_membership[0] = 'S';
      before_security_flag = 1;
    }
  else if (strstr(ad_distinguishedName, ou_distribution))
    {
      strcpy(before_group_ou, group_ou_distribution);
      before_group_membership[0] = 'D';
      before_security_flag = 0;
    }
  else if (strstr(ad_distinguishedName, ou_neither))
    {
      strcpy(before_group_ou, group_ou_neither);
      before_group_membership[0] = 'N';
      before_security_flag = 0;
    }
  else
    return(AD_NO_OU_FOUND);
  rc = group_rename(ldap_handle, dn_path, before_name, before_group_membership, 
                    before_group_ou, before_security_flag, before_desc,
                    group_name, group_membership, group_ou, group_security_flag,
                    before_desc, MoiraId, filter);
  return(rc);
}

void change_to_lower_case(char *ptr)
{
  int i;

  for (i = 0; i < (int)strlen(ptr); i++)
    {
      ptr[i] = tolower(ptr[i]);
    }
}

int ad_get_group(LDAP *ldap_handle, char *dn_path, 
                 char *group_name, char *group_membership, 
                 char *MoiraId, char *attribute,
                 LK_ENTRY **linklist_base, int *linklist_count,
                 char *rFilter)
{
  LK_ENTRY  *pPtr;
  char  filter[128];
  char  *attr_array[3];
  int   rc;

  (*linklist_base) = NULL;
  (*linklist_count) = 0;
  if (strlen(rFilter) != 0)
    {
      strcpy(filter, rFilter);
      attr_array[0] = attribute;
      attr_array[1] = NULL;
      if ((rc = linklist_build(ldap_handle, dn_path, filter, attr_array, 
                               linklist_base, linklist_count)) != 0)
        {
          com_err(whoami, 0, "LDAP server unable to get list info with MoiraId = %s: %s",
                  MoiraId, ldap_err2string(rc));
         return(rc);
       }
    if ((*linklist_count) == 1)
      {
        strcpy(rFilter, filter);
        return(0);
      }
    }

  linklist_free((*linklist_base));
  (*linklist_base) = NULL;
  (*linklist_count) = 0;
  if (strlen(MoiraId) != 0)
    {
      sprintf(filter, "(&(objectClass=group)(mitMoiraId=%s))", MoiraId);
      attr_array[0] = attribute;
      attr_array[1] = NULL;
      if ((rc = linklist_build(ldap_handle, dn_path, filter, attr_array, 
                               linklist_base, linklist_count)) != 0)
        {
          com_err(whoami, 0, "LDAP server unable to get list info with MoiraId = %s: %s",
                  MoiraId, ldap_err2string(rc));
         return(rc);
       }
    }
  if ((*linklist_count) > 1)
    {
      com_err(whoami, 0, "multiple groups with mitMoiraId = %s", MoiraId);
      pPtr = (*linklist_base);
      while (pPtr)
        {
          com_err(whoami, 0, "groups %s has mitMoiraId = %s", pPtr->value, MoiraId);
          pPtr = pPtr->next;
        }
      linklist_free((*linklist_base));
      (*linklist_base) = NULL;
      (*linklist_count) = 0;
    }
  if ((*linklist_count) == 1)
    {
      strcpy(rFilter, filter);
      return(0);
    }

  linklist_free((*linklist_base));
  (*linklist_base) = NULL;
  (*linklist_count) = 0;
  sprintf(filter, "(sAMAccountName=%s_group)", group_name);
  attr_array[0] = attribute;
  attr_array[1] = NULL;
  if ((rc = linklist_build(ldap_handle, dn_path, filter, attr_array, 
                           linklist_base, linklist_count)) != 0)
    {
      com_err(whoami, 0, "LDAP server unable to get list info with MoiraId = %s: %s",
              MoiraId, ldap_err2string(rc));
      return(rc);
    }
  if ((*linklist_count) == 1)
    {
      strcpy(rFilter, filter);
      return(0);
    }

  return(0);
}

int check_user(LDAP *ldap_handle, char *dn_path, char *UserName, char *MoiraId)
{
  char filter[128];
  char *attr_array[3];
  char SamAccountName[64];
  int  group_count;
  int  rc;
  LK_ENTRY  *group_base;
  LK_ENTRY  *gPtr;

  group_count = 0;
  group_base = NULL;

  if (strlen(MoiraId) != 0)
    {
      sprintf(filter, "(&(objectClass=user)(mitMoiraId=%s))", MoiraId);
      attr_array[0] = "sAMAccountName";
      attr_array[1] = NULL;
      if ((rc = linklist_build(ldap_handle, dn_path, filter, attr_array, 
                               &group_base, &group_count)) != 0)
        {
          com_err(whoami, 0, "LDAP server couldn't process user %s : %s",
                  UserName, ldap_err2string(rc));
          return(rc);
        }
      if (group_count > 1)
        {
          com_err(whoami, 0, "multiple users exist with MoiraId = %s",
                  MoiraId);
          gPtr = group_base;
          while (gPtr)
            {
              com_err(whoami, 0, "user %s exist with MoiraId = %s",
                      gPtr->value, MoiraId);
              gPtr = gPtr->next;
            }
        }
    }
  if (group_count != 1)
    {
      linklist_free(group_base);
      group_count = 0;
      group_base = NULL;
      sprintf(filter, "(sAMAccountName=%s)", UserName);
      attr_array[0] = "sAMAccountName";
      attr_array[1] = NULL;
      if ((rc = linklist_build(ldap_handle, dn_path, filter, attr_array, 
                               &group_base, &group_count)) != 0)
        {
          com_err(whoami, 0, "LDAP server couldn't process user %s : %s",
                  UserName, ldap_err2string(rc));
          return(rc);
        }
    }

  if (group_count != 1)
    {
      linklist_free(group_base);
      return(AD_NO_USER_FOUND);
    }
  strcpy(SamAccountName, group_base->value);
  linklist_free(group_base);
  group_count = 0;
  rc = 0;
  if (strcmp(SamAccountName, UserName))
    {
      rc = user_rename(ldap_handle, dn_path, SamAccountName, 
                       UserName);
    }
  return(0);
}
