/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/incremental/winad/winad.c,v 1.49 2006-10-30 17:34:51 zacheiss Exp $
/* winad.incr arguments examples
 *
 * arguments when moira creates the account - ignored by winad.incr since the account is unusable.
 * users 0 11 #45198 45198 /bin/cmd cmd Last First Middle 0 950000001 2000 121049
 *   login, unix_uid, shell, winconsoleshell, last, first, middle, status, mitid, type, moiraid
 *
 * arguments for creating or updating a user account 
 * users 11 11 username 45206 /bin/cmd cmd Last First Middle 2 950000001 STAFF 121058  PathToHomeDir PathToProfileDir username 45206 /bin/cmd cmd Last First Middle 1 950000001 STAFF 121058 PathToHomeDir PathToProfileDir
 * users 11 11 #45206 45206 /bin/cmd cmd Last First Middle 0 950000001 STAFF 121058  PathToHomeDir PathToProfileDir newuser 45206 /bin/cmd cmd Last First Middle 2 950000001 STAFF 121058 PathToHomeDir PathToProfileDir
 *   login, unix_uid, shell, winconsoleshell, last, first, middle, status, mitid, type, moiraid
 *
 * arguments for deactivating/deleting a user account
 * users 11 11 username 45206 /bin/cmd cmd Last First Middle 1 950000001 STAFF 121058  PathToHomeDir PathToProfileDir username 45206 /bin/cmd cmd Last First Middle 3 950000001 STAFF 121058 PathToHomeDir PathToProfileDir
 * users 11 11 username 45206 /bin/cmd cmd Last First Middle 2 950000001 STAFF 121058  PathToHomeDir PathToProfileDir username 45206 /bin/cmd cmd Last First Middle 3 950000001 STAFF 121058 PathToHomeDir PathToProfileDir
 *   login, unix_uid, shell, winconsoleshell, last, first, middle, status, mitid, type, moiraid
 *
 * arguments for reactivating a user account
 * users 11 11 username 45206 /bin/cmd cmd Last First Middle 3 950000001 STAFF 121058 username 45206 /bin/cmd cmd Last First Middle 1 950000001 STAFF 121058
 * users 11 11 username 45206 /bin/cmd cmd Last First Middle 3 950000001 STAFF 121058 username 45206 /bin/cmd cmd Last First Middle 2 950000001 STAFF 121058
 *   login, unix_uid, shell, winconsoleshell, last, first, middle, status, mitid, type, moiraid
 *
 * arguments for changing user name
 * users 11 11 oldusername 45206 /bin/cmd cmd Last First Middle 1 950000001 STAFF 121058 PathToHomeDir PathToProfileDir newusername 45206 /bin/cmd cmd Last First Middle 1 950000001 STAFF 121058 PathToHomeDir PathToProfileDir
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
 *
 * arguments for adding a file system
 * filesys 0 12 username AFS ATHENA.MIT.EDU /afs/athena.mit.edu/user/n/e/username /mit/username w descripton username wheel 1 HOMEDIR 101727
 *
 * arguments for deleting a file system
 * filesys 12 0 username AFS ATHENA.MIT.EDU /afs/athena.mit.edu/user/n/e/username /mit/username w descripton username wheel 1 HOMEDIR 101727
 *
 * arguments when moira creates a container (OU).
 * containers 0 8 machines/test/bottom description location contact USER 105316 2222 [none]
 *
 * arguments when moira deletes a container (OU).
 * containers 8 0 machines/test/bottom description location contact USER 105316 2222 groupname
 *
 * arguments when moira modifies a container information (OU).
 * containers 8 8 machines/test/bottom description location contact USER 105316 2222 groupname machines/test/bottom description1 location contact USER 105316 2222 groupname
 *
 * arguments when moira adds a machine from an OU
 * table name, beforec, afterc, machine_name, container_name, mach_id, cnt_id
 * mcntmap 0 5 DAVIDT.MIT.EDU dttest/dttest1 76767 46 groupname
 *
 * arguments when moira removes a machine from an OU
 * table name, beforec, afterc, machine_name, container_name, mach_id, cnt_id
 * mcntmap 0 5 DAVIDT.MIT.EDU dttest/dttest1 76767 46 groupname
 *
*/
#include <mit-copyright.h>
#ifdef _WIN32
#include <winsock2.h>
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

#define OWNER_SECURITY_INFORMATION       (0x00000001L)
#define GROUP_SECURITY_INFORMATION       (0x00000002L)
#define DACL_SECURITY_INFORMATION        (0x00000004L)
#define SACL_SECURITY_INFORMATION        (0x00000008L)

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

#define QUERY_VERSION   -1
#define PRIMARY_REALM   "ATHENA.MIT.EDU"
#define PRIMARY_DOMAIN  "win.mit.edu"
#define PRODUCTION_PRINCIPAL "sms"
#define TEST_PRINCIPAL       "smstest"

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

/* container arguments */
#define CONTAINER_NAME       0
#define CONTAINER_DESC       1
#define CONTAINER_LOCATION   2
#define CONTAINER_CONTACT    3
#define CONTAINER_TYPE       4
#define CONTAINER_ID         5
#define CONTAINER_ROWID      6
#define CONTAINER_GROUP_NAME 7

/*mcntmap arguments*/
#define OU_MACHINE_NAME        0
#define OU_CONTAINER_NAME      1
#define OU_MACHINE_ID          2
#define OU_CONTAINER_ID        3
#define OU_CONTAINER_GROUP     4

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

#define N_SD_BER_BYTES   5
#define LDAP_BERVAL struct berval
#define MAX_SERVER_NAMES 32

#define HIDDEN_GROUP                "HiddenGroup.g"
#define HIDDEN_GROUP_WITH_ADMIN     "HiddenGroupWithAdmin.g"
#define NOT_HIDDEN_GROUP            "NotHiddenGroup.g"
#define NOT_HIDDEN_GROUP_WITH_ADMIN "NotHiddenGroupWithAdmin.g"

#define ADD_ATTR(t, v, o) 		\
  mods[n] = malloc(sizeof(LDAPMod));	\
  mods[n]->mod_op = o;	\
  mods[n]->mod_type = t; 		\
  mods[n++]->mod_values = v

#define DEL_ATTR(t, o) 		\
  DelMods[i] = malloc(sizeof(LDAPMod));	\
  DelMods[i]->mod_op = o;	\
  DelMods[i]->mod_type = t; 		\
  DelMods[i++]->mod_values = NULL

#define DOMAIN_SUFFIX   "MIT.EDU"
#define DOMAIN  "DOMAIN:"
#define PRINCIPALNAME  "PRINCIPAL:"
#define SERVER  "SERVER:"
#define MSSFU   "SFU:"
#define SFUTYPE "30"

char    PrincipalName[128];
#ifndef _WIN32
#define KRB5CCNAME "KRB5CCNAME=/tmp/krb5cc_winad.incr"
#define KRBTKFILE "KRBTKFILE=/tmp/tkt_winad.incr"
#define KEYTABFILE "/etc/krb5.keytab"
#else
#define KRB5CCNAME "KRB5CCNAME=\\tmp\\krb5cc_winad.incr"
#define KRBTKFILE "KRBTKFILE=\\tmp\\tkt_winad.incr"
#define KEYTABFILE "\\keytabs\\krb5.keytab"
#endif

LK_ENTRY *member_base = NULL;
static char tbl_buf[1024];
char  kerberos_ou[] = "OU=kerberos,OU=moira";
char  contact_ou[] = "OU=strings,OU=moira";
char  user_ou[] = "OU=users,OU=moira";
char  group_ou_distribution[] = "OU=mail,OU=lists,OU=moira";
char  group_ou_root[] = "OU=lists,OU=moira";
char  group_ou_security[] = "OU=group,OU=lists,OU=moira";
char  group_ou_neither[] = "OU=special,OU=lists,OU=moira";
char  group_ou_both[] = "OU=mail,OU=group,OU=lists,OU=moira";
char  orphans_machines_ou[] = "OU=Machines,OU=Orphans";
char  orphans_other_ou[] = "OU=Other,OU=Orphans";
char  security_template_ou[] = "OU=security_templates";
char *whoami;
char ldap_domain[256];
char *ServerList[MAX_SERVER_NAMES];
int  mr_connections = 0;
int  callback_rc;
char default_server[256];
static char tbl_buf[1024];
int  UseSFU30 = 0;
int  NoChangeConfigFile;

extern int set_password(char *user, char *password, char *domain);

int ad_get_group(LDAP *ldap_handle, char *dn_path, char *group_name, 
                char *group_membership, char *MoiraId, char *attribute,
                LK_ENTRY **linklist_base, int *linklist_count,
              char *rFilter);
void AfsToWinAfs(char* path, char* winPath);
int ad_connect(LDAP **ldap_handle, char *ldap_domain, char *dn_path, 
               char *Win2kPassword, char *Win2kUser, char *default_server,
               int connect_to_kdc, char **ServerList);
void ad_kdc_disconnect();
int ad_server_connect(char *connectedServer, char *domain);
int attribute_update(LDAP *ldap_handle, char *distinguished_name, 
                      char *attribute_value, char *attribute, char *user_name);
int BEREncodeSecurityBits(ULONG uBits, char *pBuffer);
int checkADname(LDAP *ldap_handle, char *dn_path, char *Name);
void check_winad(void);
int check_user(LDAP *ldap_handle, char *dn_path, char *UserName, char *MoiraId);
/* containers */
int container_adupdate(LDAP *ldap_handle, char *dn_path, char *dName, 
                       char *distinguishedName, int count, char **av);
void container_check(LDAP *ldap_handle, char *dn_path, char *name);
int container_create(LDAP *ldap_handle, char *dn_path, int count, char **av);
int container_delete(LDAP *ldap_handle, char *dn_path, int count, char **av);
int container_get_distinguishedName(LDAP *ldap_handle, char *dn_path, 
                                     char *distinguishedName, int count, char **av);
void container_get_dn(char *src, char *dest);
void container_get_name(char *src, char *dest);
int container_move_objects(LDAP *ldap_handle, char *dn_path, char *dName);
int container_rename(LDAP *ldap_handle, char *dn_path, int beforec, char **before, 
                     int afterc, char **after);
int container_update(LDAP *ldap_handle, char *dn_path, int beforec, char **before,
                     int afterc, char **after);

int GetAceInfo(int ac, char **av, void *ptr);
int GetServerList(char *ldap_domain, char **MasterServe);
int get_group_membership(char *group_membership, char *group_ou, 
                         int *security_flag, char **av);
int get_machine_ou(LDAP *ldap_handle, char *dn_path, char *member, char *machine_ou, char *pPtr);
int Moira_container_group_create(char **after);
int Moira_container_group_delete(char **before);
int Moira_groupname_create(char *GroupName, char *ContainerName,
			   char *ContainerRowID);
int Moira_container_group_update(char **before, char **after);
int Moira_process_machine_container_group(char *MachineName, char* groupName,
					  int DeleteMachine);
int Moira_addGroupToParent(char *origContainerName, char *GroupName);
int Moira_getContainerGroup(int ac, char **av, void *ptr);
int Moira_getGroupName(char *origContainerName, char *GroupName,
		       int ParentFlag);
int Moira_setContainerGroup(char *ContainerName, char *GroupName);
int ProcessAce(LDAP *ldap_handle, char *dn_path, char *group_name, char *Type,
               int UpdateGroup, int *ProcessGroup);
int process_group(LDAP *ldap_handle, char *dn_path, char *MoiraId, 
                  char *group_name, char *group_ou, char *group_membership, 
                  int group_security_flag, int type);
int process_lists(int ac, char **av, void *ptr);
int ProcessGroupSecurity(LDAP *ldap_handle, char *dn_path, char *TargetGroupName, 
                         int HiddenGroup, char *AceType, char *AceName);
int ProcessMachineName(int ac, char **av, void *ptr);
void ReadConfigFile();
void StringTrim(char *StringToTrim);
int user_create(int ac, char **av, void *ptr);
int user_change_status(LDAP *ldap_handle, char *dn_path, 
                       char *user_name, char *MoiraId, int operation);
int user_delete(LDAP *ldap_handle, char *dn_path, 
                char *u_name, char *MoiraId);
int user_rename(LDAP *ldap_handle, char *dn_path, char *before_user_name, 
                char *user_name);
int user_update(LDAP *ldap_handle, char *dn_path, char *user_name,
                char *uid, char *MitId, char *MoiraId, int State,
                char *WinHomeDir, char *WinProfileDir);
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
int machine_check(LDAP *ldap_handle, char *dn_path, char *machine_name);
int machine_GetMoiraContainer(int ac, char **av, void *ptr);
int machine_get_moira_container(LDAP *ldap_handle, char *dn_path, char *machine_name, char *container_name);
int machine_move_to_ou(LDAP *ldap_handle, char *dn_path, char *MoiraMachineName, char *DestinationOu);
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
int SetHomeDirectory(LDAP *ldap_handle, char *user_name, char *DistinguishedName,
                     char *WinHomeDir, char *WinProfileDir,
                     char **homedir_v, char **winProfile_v,
                     char **drives_v, LDAPMod **mods, 
                     int OpType, int n);

void SwitchSFU(LDAPMod **mods, int *UseSFU30, int n);
int check_string(char *s);
int check_container_name(char* s);

int mr_connect_cl(char *server, char *client, int version, int auth);

void do_container(LDAP *ldap_handle, char *dn_path, char *ldap_hostname,
             char **before, int beforec, char **after, int afterc);
void do_list(LDAP *ldap_handle, char *dn_path, char *ldap_hostname,
             char **before, int beforec, char **after, int afterc);
void do_user(LDAP *ldap_handle, char *dn_path, char *ldap_hostname, 
             char **before, int beforec, char **after, int afterc);
void do_member(LDAP *ldap_handle, char *dn_path, char *ldap_hostname,
               char **before, int beforec, char **after, int afterc);
void do_mcntmap(LDAP *ldap_handle, char *dn_path, char *ldap_hostname,
                char **before, int beforec, char **after, int afterc);
int linklist_create_entry(char *attribute, char *value,
                          LK_ENTRY **linklist_entry);
int linklist_build(LDAP *ldap_handle, char *dn_path, char *search_exp, 
                   char **attr_array, LK_ENTRY **linklist_base, 
                   int *linklist_count, unsigned long ScopeType);
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
int GetMachineName(char *MachineName);
int tickets_get_k5();
int get_tickets();
int destroy_cache(void);
int dest_tkt(void);

int main(int argc, char **argv)
{
  unsigned long   rc;
  int             beforec;
  int             afterc;
  int             i;
  int             j;
  int             OldUseSFU30;
  char            *table;
  char            **before;
  char            **after;
  LDAP            *ldap_handle;
  char            dn_path[256];

  whoami = ((whoami = (char *)strrchr(argv[0], '/')) ? whoami+1 : argv[0]);

  if (argc < 4)
    {
      com_err(whoami, 0, "Unable to process %s", "argc < 4");
      exit(1);
    }
  beforec = atoi(argv[2]);
  afterc = atoi(argv[3]);

  if (argc < (4 + beforec + afterc))
    {
      com_err(whoami, 0, "Unable to process %s", "argc < (4 + breforec + afterc)");
      exit(1);
    }

  table = argv[1];
  before = &argv[4];
  after = &argv[4 + beforec];

  if (!strcmp(table, "filesys"))
    exit(0);

  if (afterc == 0)
    after = NULL;
  if (beforec == 0)
    before = NULL;

  for (i = 1; i < argc; i++)
    {
      strcat(tbl_buf, argv[i]);
      strcat(tbl_buf, " ");
    }
  com_err(whoami, 0, "%s", tbl_buf);

  memset(PrincipalName, '\0', sizeof(PrincipalName));
  memset(ldap_domain, '\0', sizeof(ldap_domain));
  memset(ServerList, '\0', sizeof(ServerList[0]) * MAX_SERVER_NAMES);
  UseSFU30 = 0;
  NoChangeConfigFile = 0;

  check_winad();

  ReadConfigFile();
  OldUseSFU30 = UseSFU30;

  get_tickets();

  initialize_sms_error_table();
  initialize_krb_error_table();

  memset(default_server, '\0', sizeof(default_server));
  memset(dn_path, '\0', sizeof(dn_path));
  for (i = 0; i < 5; i++)
    {
      ldap_handle = (LDAP *)NULL;
      if (!(rc = ad_connect(&ldap_handle, ldap_domain, dn_path, "", "", 
                            default_server, 1, ServerList)))
         break;
      if (ldap_handle == NULL)
        {
          if (!NoChangeConfigFile)
            {
              for (j = 0; j < MAX_SERVER_NAMES; j++)
                {
                  if (ServerList[j] != NULL)
                    {
                      free(ServerList[j]);
                      ServerList[j] = NULL;
                    }
                }
              GetServerList(ldap_domain, ServerList);
            }
        }
    }

  if ((rc) || (ldap_handle == NULL))
    {
  	  critical_alert("incremental", "winad.incr cannot connect to any server in domain %s", ldap_domain);
      destroy_cache();
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
  else if (!strcmp(table, "containers"))
    do_container(ldap_handle, dn_path, ldap_domain, before, beforec, after,
                 afterc);
  else if (!strcmp(table, "mcntmap"))
    do_mcntmap(ldap_handle, dn_path, ldap_domain, before, beforec, after,
               afterc);
  if (OldUseSFU30 != UseSFU30)
    {
      if (!NoChangeConfigFile)
          GetServerList(ldap_domain, ServerList);
    }
  ad_kdc_disconnect();
  for (i = 0; i < MAX_SERVER_NAMES; i++)
    {
      if (ServerList[i] != NULL)
        {
          free(ServerList[i]);
          ServerList[i] = NULL;
        }
    }
  rc = ldap_unbind_s(ldap_handle);
  destroy_cache();
  exit(0);
}

void do_mcntmap(LDAP *ldap_handle, char *dn_path, char *ldap_hostname,
                char **before, int beforec, char **after, int afterc)
{
    char    MoiraContainerName[128];
    char    ADContainerName[128];
    char    MachineName[1024];
    char    OriginalMachineName[1024];
    long    rc;
    int     DeleteMachine;
    char    MoiraContainerGroup[64];

    DeleteMachine = 0;
    memset(ADContainerName, '\0', sizeof(ADContainerName));
    memset(MoiraContainerName, '\0', sizeof(MoiraContainerName));

    if ((beforec == 0) && (afterc == 0))
        return;

    if (rc = moira_connect())
    {
        critical_alert("AD incremental",
                       "Error contacting Moira server : %s",
                       error_message(rc));
        return;
    }

    if ((beforec != 0) && (afterc == 0)) /*remove a machine*/
    {
        strcpy(OriginalMachineName, before[OU_MACHINE_NAME]);
        strcpy(MachineName, before[OU_MACHINE_NAME]);
        strcpy(MoiraContainerGroup, before[OU_CONTAINER_GROUP]);
        DeleteMachine = 1;
        com_err(whoami, 0, "removing machine %s from %s", OriginalMachineName, before[OU_CONTAINER_NAME]);
    }
    else if ((beforec == 0) && (afterc != 0)) /*add a machine*/
    {
        strcpy(OriginalMachineName, after[OU_MACHINE_NAME]);
        strcpy(MachineName, after[OU_MACHINE_NAME]);
        strcpy(MoiraContainerGroup, after[OU_CONTAINER_GROUP]);
        com_err(whoami, 0, "adding machine %s to container %s", OriginalMachineName, after[OU_CONTAINER_NAME]);
    }
    else
      {
        moira_disconnect();
        return;
      }

    rc = GetMachineName(MachineName);
    if (strlen(MachineName) == 0)
    {
        moira_disconnect();
        com_err(whoami, 0, "Unable to find alais for machine %s in Moira", OriginalMachineName);
        return;
    }
    Moira_process_machine_container_group(MachineName, MoiraContainerGroup,
					  DeleteMachine);
    if (machine_check(ldap_handle, dn_path, MachineName))
    {
        com_err(whoami, 0, "Unable to find machine %s (alias %s) in AD.", OriginalMachineName, MachineName);
        moira_disconnect();
        return;
    }
    memset(MoiraContainerName, '\0', sizeof(MoiraContainerName));
    machine_get_moira_container(ldap_handle, dn_path, MachineName, MoiraContainerName);
    if (strlen(MoiraContainerName) == 0)
    {
        com_err(whoami, 0, "Unable to fine machine %s (alias %s) container in Moira - moving to orphans OU.",
                OriginalMachineName, MachineName);
        machine_move_to_ou(ldap_handle, dn_path, MachineName, orphans_machines_ou);
        moira_disconnect();
        return;
    }
    container_get_dn(MoiraContainerName, ADContainerName);
    if (MoiraContainerName[strlen(MoiraContainerName) - 1] != '/')
       strcat(MoiraContainerName, "/");
    container_check(ldap_handle, dn_path, MoiraContainerName);
    machine_move_to_ou(ldap_handle, dn_path, MachineName, ADContainerName);
    moira_disconnect();
    return;
}

void do_container(LDAP *ldap_handle, char *dn_path, char *ldap_hostname,
             char **before, int beforec, char **after, int afterc)
{
  long rc;

  if ((beforec == 0) && (afterc == 0))
    return;

  if (rc = moira_connect())
    {
      critical_alert("AD incremental", "Error contacting Moira server : %s",
		     error_message(rc));
      return;
    }

  if ((beforec != 0) && (afterc == 0)) /*delete a new container*/
    {
      com_err(whoami, 0, "deleting container %s", before[CONTAINER_NAME]);
      container_delete(ldap_handle, dn_path, beforec, before);
      Moira_container_group_delete(before);
      moira_disconnect();
      return;
    }
  if ((beforec == 0) && (afterc != 0)) /*create a container*/
    {
      com_err(whoami, 0, "creating container %s", after[CONTAINER_NAME]);
      container_check(ldap_handle, dn_path, after[CONTAINER_NAME]);
      container_create(ldap_handle, dn_path, afterc, after);
      Moira_container_group_create(after);
      moira_disconnect();
      return;
    }

  if (strcasecmp(before[CONTAINER_NAME], after[CONTAINER_NAME]))
    {
      com_err(whoami, 0, "renaming container %s to %s", before[CONTAINER_NAME], after[CONTAINER_NAME]);
      container_rename(ldap_handle, dn_path, beforec, before, afterc, after);
      Moira_container_group_update(before, after);
      moira_disconnect();
      return;
    }
  com_err(whoami, 0, "updating container %s information", after[CONTAINER_NAME]);
  container_update(ldap_handle, dn_path, beforec, before, afterc, after);
  Moira_container_group_update(before, after);
  moira_disconnect();
  return;
}

#define L_LIST_DESC 9
#define L_LIST_ID   10

void do_list(LDAP *ldap_handle, char *dn_path, char *ldap_hostname,
             char **before, int beforec, char **after, int afterc)
{
  int   updateGroup;
  int   ProcessGroup;
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
          strcpy(list_id, after[L_LIST_ID]);
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
                  com_err(whoami, 0, "Unable to process list %s",
                          before[L_NAME]);
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
              com_err(whoami, 0, "%s", "Unable to find the group OU's");
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
                  com_err(whoami, 0, "Unable to change list name from %s to %s",
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
          com_err(whoami, 0, "Unable to find the group OU for group %s", before[L_NAME]);
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
                      com_err(whoami, 0, "Unable to create list %s", after[L_NAME]);
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

      ProcessGroup = 0;
      if (ProcessAce(ldap_handle, dn_path, after[L_NAME], "LIST", 0, &ProcessGroup))
        return;
      if (ProcessGroup)
        {
          if (ProcessAce(ldap_handle, dn_path, after[L_NAME], "LIST", 1, &ProcessGroup))
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
  char  machine_ou[256];
  char  *args[16];
  char  **ptr;
  char  *av[7];
  char  *call_args[7];
  char  *pUserOu;
  char  NewMachineName[1024];
  int   security_flag;
  int   rc;
  int   ProcessGroup;

  pUserOu = NULL;
  ptr = NULL;
  memset(moira_list_id, '\0', sizeof(moira_list_id));
  memset(moira_user_id, '\0', sizeof(moira_user_id));
  if (afterc)
    {
      if (afterc < LM_EXTRA_GID)
        return;
      if (!atoi(after[LM_EXTRA_ACTIVE]))
        {
        com_err(whoami, 0, "Unable to add %s to group %s : group not active", after[2], after[0]);
        return;
        }
      ptr = after;
      if (!strcasecmp(ptr[LM_TYPE], "LIST"))
	return;
      strcpy(user_name, after[LM_MEMBER]);
      strcpy(group_name, after[LM_LIST]);
      strcpy(user_type, after[LM_TYPE]);
      if (!strcasecmp(ptr[LM_TYPE], "MACHINE"))
        {
          if (afterc > LM_EXTRA_GROUP)
            {
              strcpy(moira_list_id, after[LMN_LIST_ID]);
              strcpy(moira_user_id, after[LM_LIST_ID]);
            }
        }
      else if (!strcasecmp(ptr[LM_TYPE], "USER"))
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
        {
          com_err(whoami, 0, "Unable to add %s to group %s : group not active", before[2], before[0]);
          return;
        }
      ptr = before;
      if (!strcasecmp(ptr[LM_TYPE], "LIST"))
	return;
      strcpy(user_name, before[LM_MEMBER]);
      strcpy(group_name, before[LM_LIST]);
      strcpy(user_type, before[LM_TYPE]);
      if (!strcasecmp(ptr[LM_TYPE], "MACHINE"))
        {
          if (beforec > LM_EXTRA_GROUP)
            {
              strcpy(moira_list_id, before[LMN_LIST_ID]);
              strcpy(moira_user_id, before[LM_LIST_ID]);
            }
        }
      else if (!strcasecmp(ptr[LM_TYPE], "USER"))
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
    {
      com_err(whoami, 0, "Unable to process group : beforec = %d, afterc = %d", beforec, afterc);
      return;
    }

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
      com_err(whoami, 0, "Unable to find the group OU for group %s", group_name);
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
                    com_err(whoami, 0, "Unable to add %s to group %s - unable to process group", user_name, group_name);
                  else
                    com_err(whoami, 0, "Unable to remove %s from group %s - unable to process group", user_name, group_name);
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
      ProcessGroup = 0;
      if (ProcessAce(ldap_handle, dn_path, ptr[LM_LIST], "LIST", 0, &ProcessGroup))
        return;
      if (ProcessGroup)
        {
          if (ProcessAce(ldap_handle, dn_path, ptr[LM_LIST], "LIST", 1, &ProcessGroup))
            return;
        }
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
      if (!strcasecmp(ptr[LM_TYPE], "MACHINE"))
        {
          memset(machine_ou, '\0', sizeof(machine_ou));
	  memset(NewMachineName, '\0', sizeof(NewMachineName));
          if (get_machine_ou(ldap_handle, dn_path, ptr[LM_MEMBER], machine_ou, NewMachineName))
            return;
	  ptr[LM_MEMBER] = NewMachineName;
          pUserOu = machine_ou;
        }
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
          com_err(whoami, 0, "Unable to remove %s from group %s", user_name, group_name);
      return;
    }

  com_err(whoami, 0, "Adding %s to list %s", user_name, group_name);
  pUserOu = user_ou;

  if (!strcasecmp(ptr[LM_TYPE], "MACHINE"))
    {
      memset(machine_ou, '\0', sizeof(machine_ou));
      memset(NewMachineName, '\0', sizeof(NewMachineName));
      if (get_machine_ou(ldap_handle, dn_path, ptr[LM_MEMBER], machine_ou, NewMachineName))
        return;
      ptr[LM_MEMBER] = NewMachineName;
      pUserOu = machine_ou;
    }
  else if (!strcasecmp(ptr[LM_TYPE], "STRING"))
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

          callback_rc = 0;
          if (rc = mr_query("get_user_account_by_login", 1, av, user_create,
                            call_args))
            {
              moira_disconnect();
              com_err(whoami, 0, "Unable to create user %s : %s",
                      ptr[LM_MEMBER], error_message(rc));
              return;
            }
          if (callback_rc)
            {
              moira_disconnect();
              com_err(whoami, 0, "Unable to create user %s", ptr[LM_MEMBER]);
              return;
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
      com_err(whoami, 0, "Unable to add %s to group %s", user_name, group_name);
    }
  return;
}


#define U_USER_ID    10
#define U_HOMEDIR    11
#define U_PROFILEDIR 12

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

  if ((beforec == 0) && (afterc != 0)) 
    {
      /*this case only happens when the account*/
      /*account is first created but not usable*/
      com_err(whoami, 0, "Unable to process user %s because the user account is not yet usable", after[U_NAME]);
      return;
    }
  if ((beforec != 0) && (afterc == 0)) /*this case only happens when the account*/
    {                                  /*is expunged*/
      if (atoi(before[U_STATE]) == 0)
        {
          com_err(whoami, 0, "expunging user %s from AD", before[U_NAME]);
          user_delete(ldap_handle, dn_path, before[U_NAME], before_user_id);
        }
      else
        {
          com_err(whoami, 0, "Unable to process because user %s has been previously expungeded", before[U_NAME]);
        }
      return;
    }

  /*process anything that gets here*/
  if ((rc = check_user(ldap_handle, dn_path, before[U_NAME], 
                       before_user_id)) == AD_NO_USER_FOUND)
    {
      if (!check_string(after[U_NAME]))
        return;
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
      callback_rc = 0;
      if (rc = mr_query("get_user_account_by_login", 1, av, user_create,
                        call_args))
        {
          moira_disconnect();
          com_err(whoami, 0, "Unable to create user %s : %s",
                  after[U_NAME], error_message(rc));
          return;
        }
      if (callback_rc)
        {
          moira_disconnect();
          com_err(whoami, 0, "Unable to create user %s", after[U_NAME]);
          return;
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
                   after_user_id, atoi(after[U_STATE]),
                   after[U_HOMEDIR], after[U_PROFILEDIR]);
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
                   int *linklist_count, unsigned long ScopeType)
{
  ULONG       rc;
  LDAPMessage *ldap_entry;

  rc = 0;
  ldap_entry = NULL;
  (*linklist_base) = NULL;
  (*linklist_count) = 0;
  if ((rc = ldap_search_s(ldap_handle, dn_path, ScopeType, 
                          search_exp, attr_array, 0, &ldap_entry))
      != LDAP_SUCCESS)
      {
        if (rc != LDAP_SIZELIMIT_EXCEEDED)
          return(0);
      }

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
        rc = mr_krb5_auth("winad.incr");
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
      com_err(whoami, 0, "Unable to process invalid LDAP list name %s", before_group_name);
      return(AD_INVALID_NAME);
    }
  if (!check_string(after_group_name))
    {
      com_err(whoami, 0, "Unable to process invalid LDAP list name %s", after_group_name);
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
              "Unable to process multiple groups with MoiraId = %s exist in the AD",
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
                           &group_base, &group_count, LDAP_SCOPE_SUBTREE)) != 0)
    {
      com_err(whoami, 0, "Unable to get list %s dn : %s",
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
      com_err(whoami, 0, "Unable to rename list from %s to %s : %s",
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
      com_err(whoami, 0, "Unable to rename list from %s to %s : sAMAccountName not found",
              before_group_name, after_group_name);
      return(rc);
    }
  samAccountName_v[0] = sam_name;
  if (after_security_flag)
    groupTypeControl |= ADS_GROUP_TYPE_SECURITY_ENABLED;
  sprintf(groupTypeControlStr, "%ld", groupTypeControl);
  groupTypeControl_v[0] = groupTypeControlStr;
  mitMoiraId_v[0] = MoiraId;

  sprintf(new_dn, "cn=%s,%s,%s", after_group_name, after_group_ou, dn_path);
  rc = attribute_update(ldap_handle, new_dn, after_desc, "description", after_group_name);
  n = 0;
  ADD_ATTR("samAccountName", samAccountName_v, LDAP_MOD_REPLACE);
  ADD_ATTR("displayName", name_v, LDAP_MOD_REPLACE);
  ADD_ATTR("mitMoiraId", mitMoiraId_v, LDAP_MOD_REPLACE);
  ADD_ATTR("groupType", groupTypeControl_v, LDAP_MOD_REPLACE);
  mods[n] = NULL;
  if ((rc = ldap_modify_s(ldap_handle, new_dn, mods)) != LDAP_SUCCESS)
    {
      com_err(whoami, 0, "Unable to modify list data for %s after renaming: %s",
              after_group_name, ldap_err2string(rc));
    }
  for (i = 0; i < n; i++)
    free(mods[i]);
  return(rc);
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
  int  updateGroup;
  char **call_args;

  call_args = ptr;

  if (!check_string(av[L_NAME]))
    {
      com_err(whoami, 0, "Unable to process invalid LDAP list name %s", av[L_NAME]);
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
          com_err(whoami, 0, "Unable to create list %s in AD : %s",
                  av[L_NAME], ldap_err2string(rc));
          callback_rc = rc;
          return(rc);
        }
    }
  if ((rc == LDAP_ALREADY_EXISTS) || (updateGroup))
    {
      rc = attribute_update((LDAP *)call_args[0], new_dn, av[L_DESC], "description", av[L_NAME]);
      sprintf(info, "The Administrator of this list is: %s", av[L_ACE_NAME]);
      rc = attribute_update((LDAP *)call_args[0], new_dn, info, "info", av[L_NAME]);
      n = 0;
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
      rc = LDAP_SUCCESS;
      if (n != 0)
        {
          rc = ldap_modify_s((LDAP *)call_args[0], new_dn, mods);
          for (i = 0; i < n; i++)
              free(mods[i]);
          if (rc != LDAP_SUCCESS)
            {
              com_err(whoami, 0, "Unable to update list %s in AD : %s",
                      av[L_NAME], ldap_err2string(rc));
              callback_rc = rc;
              return(rc);
            }
        }
    }

  ProcessGroupSecurity((LDAP *)call_args[0], call_args[1], av[L_NAME], 
                       atoi(av[L_HIDDEN]),  av[L_ACE_TYPE], av[L_ACE_NAME]);

  return(LDAP_SUCCESS);
}

int ProcessGroupSecurity(LDAP *ldap_handle, char *dn_path, char *TargetGroupName, 
                         int HiddenGroup, char *AceType, char *AceName)
{
  char          filter_exp[1024];
  char          *attr_array[5];
  char          search_path[512];
  char          root_ou[128];
  char          TemplateDn[512];
  char          TemplateSamName[128];
  char          TargetDn[512];
  char          TargetSamName[128];
  char          AceSamAccountName[128];
  char          AceDn[256];
  unsigned char AceSid[128];
  unsigned char UserTemplateSid[128];
  char          acBERBuf[N_SD_BER_BYTES];
  char          GroupSecurityTemplate[256];
  int           AceSidCount;
  int           UserTemplateSidCount;
  int           group_count;
  int           n;
  int           i;
  int           rc;
  int           nVal;
  ULONG         dwInfo;
  int           array_count = 0;
  LDAPMod       *mods[20];
  LK_ENTRY      *group_base;
  LDAP_BERVAL   **ppsValues;
  LDAPControl sControl = {"1.2.840.113556.1.4.801",
                          { N_SD_BER_BYTES, acBERBuf },
                          TRUE
                         };
  LDAPControl *apsServerControls[] = {&sControl, NULL};
  LDAPMessage *psMsg;

  dwInfo = OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION;
  BEREncodeSecurityBits(dwInfo, acBERBuf);

  sprintf(search_path, "%s,%s", group_ou_root, dn_path);
  sprintf(filter_exp, "(sAMAccountName=%s_group)", TargetGroupName);
  attr_array[0] = "sAMAccountName";
  attr_array[1] = NULL;
  group_count = 0;
  group_base = NULL;
  if ((rc = linklist_build(ldap_handle, search_path, filter_exp, attr_array, 
                               &group_base, &group_count, LDAP_SCOPE_SUBTREE) != 0))
    return(1);
  if (group_count != 1)
    {
      linklist_free(group_base);
      return(1);
    }
  strcpy(TargetDn, group_base->dn);
  strcpy(TargetSamName, group_base->value);
  linklist_free(group_base);
  group_base = NULL;
  group_count = 0;

  UserTemplateSidCount = 0;
  memset(UserTemplateSid, '\0', sizeof(UserTemplateSid));
  memset(AceSamAccountName, '\0', sizeof(AceSamAccountName));
  memset(AceSid, '\0', sizeof(AceSid));
  AceSidCount = 0;
  group_base = NULL;
  group_count = 0;
  if (strlen(AceName) != 0)
    {
      if (!strcmp(AceType, "LIST"))
        {
          sprintf(AceSamAccountName, "%s_group", AceName);
          strcpy(root_ou, group_ou_root);
        }
      else if (!strcmp(AceType, "USER"))
        {
          sprintf(AceSamAccountName, "%s", AceName);
          strcpy(root_ou, user_ou);
        }
      if (strlen(AceSamAccountName) != 0)
        {
          sprintf(search_path, "%s", dn_path);
          sprintf(filter_exp, "(sAMAccountName=%s)", AceSamAccountName);
          attr_array[0] = "objectSid";
          attr_array[1] = NULL;
          group_count = 0;
          group_base = NULL;
          if ((rc = linklist_build(ldap_handle, search_path, filter_exp, attr_array, 
                                   &group_base, &group_count, LDAP_SCOPE_SUBTREE) != 0))
            return(1);
          if (group_count == 1)
            {
              strcpy(AceDn, group_base->dn);
              AceSidCount = group_base->length;
              memcpy(AceSid, group_base->value, AceSidCount);
            }
          linklist_free(group_base);
          group_base = NULL;
          group_count = 0;
        }
    }
  if (AceSidCount == 0)
    {
      com_err(whoami, 0, "Group %s: Administrator: %s, Type: %s - does not have an AD SID.", TargetGroupName, AceName, AceType);
      com_err(whoami, 0, "   Non-admin security group template will be used.");
    }
  else
    {
      sprintf(search_path, "%s,%s", security_template_ou, dn_path);
      sprintf(filter_exp, "(sAMAccountName=%s)", "UserTemplate.u");
      attr_array[0] = "objectSid";
      attr_array[1] = NULL;

      group_count = 0;
      group_base = NULL;
      if ((rc = linklist_build(ldap_handle, search_path, filter_exp, attr_array, 
                                   &group_base, &group_count, LDAP_SCOPE_SUBTREE) != 0))
        return(1);
      if ((rc != 0) || (group_count != 1))
        {
          com_err(whoami, 0, "Unable to process user security template: %s", "UserTemplate");
          AceSidCount = 0;
        }
      else
        {
          UserTemplateSidCount = group_base->length;
          memcpy(UserTemplateSid, group_base->value, UserTemplateSidCount);
        }
      linklist_free(group_base);
      group_base = NULL;
      group_count = 0;
    }

  if (HiddenGroup)
    {
      if (AceSidCount == 0)
        {
          strcpy(GroupSecurityTemplate, HIDDEN_GROUP);
          sprintf(filter_exp, "(sAMAccountName=%s)", HIDDEN_GROUP);
        }
      else
        {
          strcpy(GroupSecurityTemplate, HIDDEN_GROUP_WITH_ADMIN);
          sprintf(filter_exp, "(sAMAccountName=%s)", HIDDEN_GROUP_WITH_ADMIN);
        }
    }
  else
    {
      if (AceSidCount == 0)
        {
          strcpy(GroupSecurityTemplate, NOT_HIDDEN_GROUP);
          sprintf(filter_exp, "(sAMAccountName=%s)", NOT_HIDDEN_GROUP);
        }
      else
        {
          strcpy(GroupSecurityTemplate, NOT_HIDDEN_GROUP_WITH_ADMIN);
          sprintf(filter_exp, "(sAMAccountName=%s)", NOT_HIDDEN_GROUP_WITH_ADMIN);
        }
    }

  sprintf(search_path, "%s,%s", security_template_ou, dn_path);
  attr_array[0] = "sAMAccountName";
  attr_array[1] = NULL;
  group_count = 0;
  group_base = NULL;
  if ((rc = linklist_build(ldap_handle, search_path, filter_exp, attr_array, 
                               &group_base, &group_count, LDAP_SCOPE_SUBTREE) != 0))
    return(1);
  if (group_count != 1)
    {
      linklist_free(group_base);
      com_err(whoami, 0, "Unable to process group security template: %s - security not set", GroupSecurityTemplate);
      return(1);
    }
  strcpy(TemplateDn, group_base->dn);
  strcpy(TemplateSamName, group_base->value);
  linklist_free(group_base);
  group_base = NULL;
  group_count = 0;

  sprintf(filter_exp, "(sAMAccountName=%s)", TemplateSamName);
  rc = ldap_search_ext_s(ldap_handle,
                         TemplateDn,
                         LDAP_SCOPE_SUBTREE,
                         filter_exp,
                         NULL,
                         0,
                         apsServerControls,
                         NULL,
                         NULL,
                         0,
                         &psMsg);

  if ((psMsg = ldap_first_entry(ldap_handle, psMsg)) == NULL)
    {
      com_err(whoami, 0, "Unable to find group security template: %s - security not set", GroupSecurityTemplate);
      return(1);
    }
  ppsValues = ldap_get_values_len(ldap_handle, psMsg, "ntSecurityDescriptor");
  if (ppsValues == NULL)
    {
      com_err(whoami, 0, "Unable to find group security descriptor for group %s - security not set", GroupSecurityTemplate);
      return(1);
    }

  if (AceSidCount != 0)
    {
      for (nVal = 0; ppsValues[nVal] != NULL; nVal++)
        {
          for (i = 0; i < (int)(ppsValues[nVal]->bv_len - UserTemplateSidCount); i++)
            {
              if (!memcmp(&ppsValues[nVal]->bv_val[i], UserTemplateSid, UserTemplateSidCount))
                {
                  memcpy(&ppsValues[nVal]->bv_val[i], AceSid, AceSidCount);
                  break;
                }
            }
        }
    }

  n = 0;
  ADD_ATTR("ntSecurityDescriptor", (char **)ppsValues, LDAP_MOD_REPLACE | LDAP_MOD_BVALUES);
  mods[n] = NULL;

  rc = ldap_modify_s(ldap_handle, TargetDn, mods);
  for (i = 0; i < n; i++)
    free(mods[i]);
  ldap_value_free_len(ppsValues);
  ldap_msgfree(psMsg);
  if (rc != LDAP_SUCCESS)
    {
      com_err(whoami, 0, "Unable to set security settings for group %s : %s",
              TargetGroupName, ldap_err2string(rc));
      if (AceSidCount != 0)
        {
          com_err(whoami, 0, "Trying to set security for group %s without admin.",
                  TargetGroupName);
          if (rc = ProcessGroupSecurity(ldap_handle, dn_path, TargetGroupName, 
                                        HiddenGroup, "", ""))
            {
              com_err(whoami, 0, "Unable to set security for group %s.",
                      TargetGroupName);
              return(rc);
            }
        }
      return(rc);
    }
  return(rc);
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
      com_err(whoami, 0, "Unable to process invalid LDAP list name %s", group_name);
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

int BEREncodeSecurityBits(ULONG uBits, char *pBuffer)
{
    *pBuffer++ = 0x30;
    *pBuffer++ = 0x03;
    *pBuffer++ = 0x02;
    *pBuffer++ = 0x00;
    return(N_SD_BER_BYTES);
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
      com_err(whoami, 0, "Unable to find list %s in AD",
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
  if (rc == LDAP_UNWILLING_TO_PERFORM)
    rc = LDAP_SUCCESS;
  if (rc != LDAP_SUCCESS)
    {
      com_err(whoami, 0, "Unable to modify list %s members : %s",
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
      com_err(whoami, 0, "Unable to find list %s in AD",
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
      com_err(whoami, 0, "Unable to add %s to list %s as a member : %s",
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
      com_err(whoami, 0, "Unable to process invalid LDAP name %s", user);
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
      com_err(whoami, 0, "Unable to create contact %s : %s",
              user, ldap_err2string(rc));
      return(rc);
    }
  return(0);
}

int user_update(LDAP *ldap_handle, char *dn_path, char *user_name,
                char *Uid, char *MitId, char *MoiraId, int State,
                char *WinHomeDir, char *WinProfileDir)
{
  LDAPMod   *mods[20];
  LK_ENTRY  *group_base;
  int  group_count;
  char distinguished_name[512];
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
  int  OldUseSFU30;
  u_int userAccountControl = UF_NORMAL_ACCOUNT | UF_DONT_EXPIRE_PASSWD | UF_PASSWD_CANT_CHANGE;
  char filter[128];
  char *attr_array[3];
  char temp[256];

  if (!check_string(user_name))
    {
      com_err(whoami, 0, "Unable to process invalid LDAP user name %s", user_name);
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
                               &group_base, &group_count, LDAP_SCOPE_SUBTREE)) != 0)
        {
          com_err(whoami, 0, "Unable to process user %s : %s",
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
                               &group_base, &group_count, LDAP_SCOPE_SUBTREE)) != 0)
        {
          com_err(whoami, 0, "Unable to process user %s : %s",
                  user_name, ldap_err2string(rc));
          return(rc);
        }
    }

  if (group_count != 1)
    {
      com_err(whoami, 0, "Unable to find user %s in AD",
              user_name);
      linklist_free(group_base);
      return(AD_NO_USER_FOUND);
    }
  strcpy(distinguished_name, group_base->dn);

  linklist_free(group_base);
  group_count = 0;

  if ((strlen(MitId) != 0) && (MitId[0] == '9'))
    rc = attribute_update(ldap_handle, distinguished_name, MitId, "employeeID", user_name);
  else
    rc = attribute_update(ldap_handle, distinguished_name, "none", "employeeID", user_name);
  rc = attribute_update(ldap_handle, distinguished_name, Uid, "uid", user_name);
  rc = attribute_update(ldap_handle, distinguished_name, MoiraId, "mitMoiraId", user_name);

  n = 0;
  uid_v[0] = Uid;
  if (!UseSFU30)
    {
      ADD_ATTR("uidNumber", uid_v, LDAP_MOD_REPLACE);
    }
  else
    {
      ADD_ATTR("msSFU30UidNumber", uid_v, LDAP_MOD_REPLACE);
    }


  if ((State != US_NO_PASSWD) && (State != US_REGISTERED))
    userAccountControl |= UF_ACCOUNTDISABLE;
  sprintf(userAccountControlStr, "%ld", userAccountControl);
  userAccountControl_v[0] = userAccountControlStr;
  ADD_ATTR("userAccountControl", userAccountControl_v, LDAP_MOD_REPLACE);

  n = SetHomeDirectory(ldap_handle, user_name, distinguished_name, WinHomeDir, 
                       WinProfileDir, homedir_v, winProfile_v,
                       drives_v, mods, LDAP_MOD_REPLACE, n);

  mods[n] = NULL;
  if ((rc = ldap_modify_s(ldap_handle, distinguished_name, mods)) != LDAP_SUCCESS)
    {
        OldUseSFU30 = UseSFU30;
        SwitchSFU(mods, &UseSFU30, n);
        if (OldUseSFU30 != UseSFU30)
            rc = ldap_modify_s(ldap_handle, distinguished_name, mods);
        if (rc)
        {
            com_err(whoami, 0, "Unable to modify user data for %s : %s",
                user_name, ldap_err2string(rc));
        }
    }
  for (i = 0; i < n; i++)
    free(mods[i]);
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
      com_err(whoami, 0, "Unable to process invalid LDAP user name %s", before_user_name);
      return(AD_INVALID_NAME);
    }
  if (!check_string(user_name))
    {
      com_err(whoami, 0, "Unable to process invalid LDAP user name %s", user_name);
      return(AD_INVALID_NAME);
    }

  strcpy(user_name, user_name);
  sprintf(old_dn, "cn=%s,%s,%s", before_user_name, user_ou, dn_path);
  sprintf(new_dn, "cn=%s", user_name);
  if ((rc = ldap_rename_s(ldap_handle, old_dn, new_dn, NULL, TRUE, 
                           NULL, NULL)) != LDAP_SUCCESS)
    {
      com_err(whoami, 0, "Unable to rename user from %s to %s : %s",
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
      com_err(whoami, 0, "Unable to modify user data for %s after renaming : %s",
              user_name, ldap_err2string(rc));
    }
  for (i = 0; i < n; i++)
    free(mods[i]);
  return(rc);
}

int user_create(int ac, char **av, void *ptr)
{
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
  char *homedir_v[] = {NULL, NULL};
  char *winProfile_v[] = {NULL, NULL};
  char *drives_v[] = {NULL, NULL};
  char userAccountControlStr[80];
  char temp[128];
  u_int userAccountControl = UF_NORMAL_ACCOUNT | UF_DONT_EXPIRE_PASSWD | UF_PASSWD_CANT_CHANGE;
  int  n;
  int  rc;
  int  i;
  int  OldUseSFU30;
  char **call_args;
  char WinHomeDir[1024];
  char WinProfileDir[1024];

  call_args = ptr;

  if (!check_string(av[U_NAME]))
    {
      callback_rc = AD_INVALID_NAME;
      com_err(whoami, 0, "Unable to process invalid LDAP user name %s", av[U_NAME]);
      return(AD_INVALID_NAME);
    }

  memset(WinHomeDir, '\0', sizeof(WinHomeDir));
  memset(WinProfileDir, '\0', sizeof(WinProfileDir));
  strcpy(WinHomeDir, av[U_WINHOMEDIR]);
  strcpy(WinProfileDir, av[U_WINPROFILEDIR]);
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
      if (!UseSFU30)
        {
          ADD_ATTR("uidNumber", uid_v, LDAP_MOD_ADD);
        }
      else
        {
          ADD_ATTR("msSFU30UidNumber", uid_v, LDAP_MOD_ADD);
        }
    }
  if ((strlen(av[U_MITID]) != 0) && (av[U_MITID][0] == '9'))
      mitid_v[0] = av[U_MITID];
  else
      mitid_v[0] = "none";
  ADD_ATTR("employeeID", mitid_v, LDAP_MOD_ADD);

  n = SetHomeDirectory((LDAP *)call_args[0], user_name, new_dn, WinHomeDir, 
                       WinProfileDir, homedir_v, winProfile_v,
                       drives_v, mods, LDAP_MOD_ADD, n);

  mods[n] = NULL;

  rc = ldap_add_ext_s((LDAP *)call_args[0], new_dn, mods, NULL, NULL);
  if ((rc != LDAP_SUCCESS) && (rc != LDAP_ALREADY_EXISTS))
  {
      OldUseSFU30 = UseSFU30;
      SwitchSFU(mods, &UseSFU30, n);
      if (OldUseSFU30 != UseSFU30)
          rc = ldap_add_ext_s((LDAP *)call_args[0], new_dn, mods, NULL, NULL);
  }

  for (i = 0; i < n; i++)
    free(mods[i]);
  if ((rc != LDAP_SUCCESS) && (rc != LDAP_ALREADY_EXISTS))
    {
      com_err(whoami, 0, "Unable to create user %s : %s",
              user_name, ldap_err2string(rc));
      callback_rc = rc;
      return(rc);
    }
  if (rc == LDAP_SUCCESS)
    {
      if ((rc = set_password(sam_name, "", ldap_domain)) != 0)
        {
          ad_kdc_disconnect();
          tickets_get_k5();
          if (!ad_server_connect(default_server, ldap_domain))
            {
              com_err(whoami, 0, "Unable to set password for user %s : %s",
                      user_name, "cannot get changepw ticket from windows domain");
            }
          else
            {
              if ((rc = set_password(sam_name, "", ldap_domain)) != 0)
                {
                  com_err(whoami, 0, "Unable to set password for user %s : %ld",
                          user_name, rc);
                }
            }
        }
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
      com_err(whoami, 0, "Unable to process invalid LDAP user name %s", user_name);
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
                               &group_base, &group_count, LDAP_SCOPE_SUBTREE)) != 0)
        {
          com_err(whoami, 0, "Unable to process user %s : %s",
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
                               &group_base, &group_count, LDAP_SCOPE_SUBTREE)) != 0)
        {
          com_err(whoami, 0, "Unable to process user %s : %s",
                  user_name, ldap_err2string(rc));
          return(rc);
        }
    }

  if (group_count != 1)
    {
      linklist_free(group_base);
      com_err(whoami, 0, "Unable to find user %s in AD",
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
      com_err(whoami, 0, "Unable to change status of user %s : %s",
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
                               &group_base, &group_count, LDAP_SCOPE_SUBTREE)) != 0)
        {
          com_err(whoami, 0, "Unable to process user %s : %s",
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
                               &group_base, &group_count, LDAP_SCOPE_SUBTREE)) != 0)
        {
          com_err(whoami, 0, "Unable to process user %s : %s",
                  user_name, ldap_err2string(rc));
          goto cleanup;
        }
    }

  if (group_count != 1)
    {
      com_err(whoami, 0, "Unable to find user %s in AD",
              user_name);
      goto cleanup;
    }

  strcpy(distinguished_name, group_base->dn);
  if (rc = ldap_delete_s(ldap_handle, distinguished_name))
    {
      com_err(whoami, 0, "Unable to process user %s : %s",
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

int check_container_name(char *s)
{
  char  character;

  for (; *s; s++)
    {
      character = *s;
      if (isupper(character))
        character = tolower(character);

	  if (character == ' ')
		  continue;
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
      status = mr_krb5_auth(client);
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

int GetAceInfo(int ac, char **av, void *ptr)
{
  char **call_args;
  int   security_flag;

  call_args = ptr;

  strcpy(call_args[0], av[L_ACE_TYPE]);
  strcpy(call_args[1], av[L_ACE_NAME]);
  security_flag = 0;
  get_group_membership(call_args[2], call_args[3], &security_flag, av);
  return(LDAP_SUCCESS);

}

int checkADname(LDAP *ldap_handle, char *dn_path, char *Name)
{
  char filter[128];
  char *attr_array[3];
  int  group_count;
  int  rc;
  LK_ENTRY  *group_base;

  group_count = 0;
  group_base = NULL;

  sprintf(filter, "(sAMAccountName=%s)", Name);
  attr_array[0] = "sAMAccountName";
  attr_array[1] = NULL;
  if ((rc = linklist_build(ldap_handle, dn_path, filter, attr_array, 
                           &group_base, &group_count, LDAP_SCOPE_SUBTREE)) != 0)
    {
      com_err(whoami, 0, "Unable to process ACE name %s : %s",
              Name, ldap_err2string(rc));
      return(1);
    }

  linklist_free(group_base);
  group_base = NULL;
  if (group_count == 0)
    return(0);
  return(1);
}

#define MAX_ACE 7

int ProcessAce(LDAP *ldap_handle, char *dn_path, char *Name, char *Type, int UpdateGroup, int *ProcessGroup)
{
  char  *av[2];
  char  GroupName[256];
  char  *call_args[7];
  int   rc;
  char  *AceInfo[4];
  char  AceType[32];
  char  AceName[128];
  char  AceMembership[2];
  char  AceOu[256];
  char  temp[128];

  strcpy(GroupName, Name);

  if (strcasecmp(Type, "LIST"))
    return(1);
  while (1)
    {
      av[0] = GroupName;
      AceInfo[0] = AceType;
      AceInfo[1] = AceName;
      AceInfo[2] = AceMembership;
      AceInfo[3] = AceOu;
      memset(AceType, '\0', sizeof(AceType));
      memset(AceName, '\0', sizeof(AceName));
      memset(AceMembership, '\0', sizeof(AceMembership));
      memset(AceOu, '\0', sizeof(AceOu));
      callback_rc = 0;
      if (rc = mr_query("get_list_info", 1, av, GetAceInfo, AceInfo))
        {
          com_err(whoami, 0, "Unable to get ACE info for list %s : %s", GroupName, error_message(rc));
          return(1);
        }
      if (callback_rc)
        {
          com_err(whoami, 0, "Unable to get ACE info for list %s", GroupName);
          return(1);
        }
      if ((strcasecmp(AceType, "USER")) && (strcasecmp(AceType, "LIST")))
        return(0);
      strcpy(temp, AceName);
      if (!strcasecmp(AceType, "LIST"))
        sprintf(temp, "%s_group", AceName);
      if (!UpdateGroup)
        {
          if (checkADname(ldap_handle, dn_path, temp))
            return(0);
          (*ProcessGroup) = 1;
        }
      if (!strcasecmp(AceInfo[0], "LIST"))
        {
          if (make_new_group(ldap_handle, dn_path, "", AceName, AceOu, AceMembership, 0, UpdateGroup))
            return(1);
        }
      else if (!strcasecmp(AceInfo[0], "USER"))
        {
          av[0] = AceName;
          call_args[0] = (char *)ldap_handle;
          call_args[1] = dn_path;
          call_args[2] = "";
          call_args[3] = NULL;
          callback_rc = 0;
          if (rc = mr_query("get_user_account_by_login", 1, av, user_create, call_args))
            {
              com_err(whoami, 0, "Unable to process user ACE %s for group %s.", AceName, Name);
              return(1);
            }
          if (callback_rc)
            {
              com_err(whoami, 0, "Unable to process user Ace %s for group %s", AceName, Name);
              return(1);
            }
          return(0);
        }
      else
        return(1);
      if (!strcasecmp(AceType, "LIST"))
        {
          if (!strcasecmp(GroupName, AceName))
            return(0);
        }
      strcpy(GroupName, AceName);
    }
  return(1);
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
  callback_rc = 0;
  if (rc = mr_query("get_list_info", 1, av, group_create, call_args))
    {
      moira_disconnect();
      com_err(whoami, 0, "Unable to create list %s : %s", group_name, error_message(rc));
      return(rc);
    }
  if (callback_rc)
    {
      moira_disconnect();
      com_err(whoami, 0, "Unable to create list %s", group_name);
      return(callback_rc);
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
      com_err(whoami, 0, "Unable to populate list %s : %s", 
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
                           &group_base, &group_count, LDAP_SCOPE_SUBTREE)) != 0)
    {
      com_err(whoami, 0, "Unable to get list info with MoiraId = %s: %s",
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
                           &group_base, &group_count, LDAP_SCOPE_SUBTREE)) != 0)
    {
      com_err(whoami, 0, "Unable to get list name with MoiraId = %s: %s",
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
                           &group_base, &group_count, LDAP_SCOPE_SUBTREE)) != 0)
    {
      com_err(whoami, 0, 
              "Unable to get list description with MoiraId = %s: %s",
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
                               linklist_base, linklist_count, LDAP_SCOPE_SUBTREE)) != 0)
        {
          com_err(whoami, 0, "Unable to get list info with MoiraId = %s: %s",
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
                               linklist_base, linklist_count, LDAP_SCOPE_SUBTREE)) != 0)
        {
          com_err(whoami, 0, "Unable to get list info with MoiraId = %s: %s",
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
      if (!memcmp(&(*linklist_base)->value[3], group_name, strlen(group_name)))
        {
          strcpy(rFilter, filter);
          return(0);
        }
    }

  linklist_free((*linklist_base));
  (*linklist_base) = NULL;
  (*linklist_count) = 0;
  sprintf(filter, "(sAMAccountName=%s_group)", group_name);
  attr_array[0] = attribute;
  attr_array[1] = NULL;
  if ((rc = linklist_build(ldap_handle, dn_path, filter, attr_array, 
                           linklist_base, linklist_count, LDAP_SCOPE_SUBTREE)) != 0)
    {
      com_err(whoami, 0, "Unable to get list info with MoiraId = %s: %s",
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
                               &group_base, &group_count, LDAP_SCOPE_SUBTREE)) != 0)
        {
          com_err(whoami, 0, "Unable to process user %s : %s",
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
                               &group_base, &group_count, LDAP_SCOPE_SUBTREE)) != 0)
        {
          com_err(whoami, 0, "Unable to process user %s : %s",
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

void container_get_dn(char *src, char *dest)
{
  char *sPtr;
  char *array[20];
  char name[256];
  int  n;

  memset(array, '\0', 20 * sizeof(array[0]));

  if (strlen(src) == 0)
    return;
  strcpy(name, src);
  sPtr = name;
  n = 0;
  array[n] = name;
  ++n;
  while (*sPtr)
    {
      if ((*sPtr) == '/')
        {
          (*sPtr) = '\0';
          ++sPtr;
          array[n] = sPtr;
          ++n;
        }
      else
        ++sPtr;
    }
  strcpy(dest, "OU=");
  while (n != 0)
    {
      strcat(dest, array[n-1]);
      --n;
      if (n > 0)
        {
          strcat(dest, ",OU=");
        }
    }
  return;
}

void container_get_name(char *src, char *dest)
{
  char *sPtr;
  char *dPtr;

  if (strlen(src) == 0)
    return;
  sPtr = src;
  dPtr = src;
  while (*sPtr)
    {
      if ((*sPtr) == '/')
        {
          dPtr = sPtr;
          ++dPtr;
        }
      ++sPtr;
    }
  strcpy(dest, dPtr);
  return;
}

void container_check(LDAP *ldap_handle, char *dn_path, char *name)
{
  char cName[256];
  char *av[7];
  int  i;
  int  rc;

  strcpy(cName, name);
  for (i = 0; i < (int)strlen(cName); i++)
    {
      if (cName[i] == '/')
        {
          cName[i] = '\0';
          av[CONTAINER_NAME] = cName;
          av[CONTAINER_DESC] = "";
          av[CONTAINER_LOCATION] = "";
          av[CONTAINER_CONTACT] = "";
          av[CONTAINER_TYPE] = "";
          av[CONTAINER_ID] = "";
          av[CONTAINER_ROWID] = "";
          rc = container_create(ldap_handle, dn_path, 7, av);
          if (rc == LDAP_SUCCESS)
            {
              com_err(whoami, 0, "container %s created without a mitMoiraId", cName);
            }
          cName[i] = '/';
        }
    }

}

int container_rename(LDAP *ldap_handle, char *dn_path, int beforec, char **before, 
                     int afterc, char **after)
{
  char      dName[256];
  char      cName[256];
  char      new_cn[128];
  char      new_dn_path[256];
  char      temp[256];
  char      distinguishedName[256];
  char      *pPtr;
  int       rc;
  int       i;

  memset(cName, '\0', sizeof(cName));
  container_get_name(after[CONTAINER_NAME], cName);
  if (!check_container_name(cName))
    {
      com_err(whoami, 0, "Unable to process invalid LDAP container name %s", cName);
      return(AD_INVALID_NAME);
    }

  memset(distinguishedName, '\0', sizeof(distinguishedName));
  if (rc = container_get_distinguishedName(ldap_handle, dn_path, distinguishedName, beforec, before))
    return(rc);
  if (strlen(distinguishedName) == 0)
    {
      rc = container_create(ldap_handle, dn_path, afterc, after);
      return(rc);
    }

  strcpy(temp, after[CONTAINER_NAME]);
  pPtr = temp;
  for (i = 0; i < (int)strlen(temp); i++)
    {
      if (temp[i] == '/')
        {
          pPtr = &temp[i];
        }
    }
  (*pPtr) = '\0';

  container_get_dn(temp, dName);
  if (strlen(temp) != 0)
    sprintf(new_dn_path, "%s,%s", dName, dn_path);
  else
    sprintf(new_dn_path, "%s", dn_path);
  sprintf(new_cn, "OU=%s", cName);

  container_check(ldap_handle, dn_path, after[CONTAINER_NAME]);

  if ((rc = ldap_rename_s(ldap_handle, distinguishedName, new_cn, new_dn_path,
                          TRUE, NULL, NULL)) != LDAP_SUCCESS)
    {
      com_err(whoami, 0, "Unable to rename container from %s to %s : %s",
              before[CONTAINER_NAME], after[CONTAINER_NAME], ldap_err2string(rc));
      return(rc);
    }

  memset(dName, '\0', sizeof(dName));
  container_get_dn(after[CONTAINER_NAME], dName);
  rc = container_adupdate(ldap_handle, dn_path, dName, "", afterc, after);
  return(rc);
}

int container_delete(LDAP *ldap_handle, char *dn_path, int count, char **av)
{
  char      distinguishedName[256];
  int       rc;

  memset(distinguishedName, '\0', sizeof(distinguishedName));
  if (rc = container_get_distinguishedName(ldap_handle, dn_path, distinguishedName, count, av))
    return(rc);
  if (strlen(distinguishedName) == 0)
    return(0);
  if ((rc = ldap_delete_s(ldap_handle, distinguishedName)) != LDAP_SUCCESS)
    {
      if (rc == LDAP_NOT_ALLOWED_ON_NONLEAF)
        container_move_objects(ldap_handle, dn_path, distinguishedName);
      else
        com_err(whoami, 0, "Unable to delete container %s from AD : %s",
                av[CONTAINER_NAME], ldap_err2string(rc));
    }
  return(rc);
}

int container_create(LDAP *ldap_handle, char *dn_path, int count, char **av)
{
  char      *attr_array[3];
  LK_ENTRY  *group_base;
  int       group_count;
  LDAPMod   *mods[20];
  char      *objectClass_v[] = {"top", 
                           "organizationalUnit", 
                           NULL};

  char *ou_v[] = {NULL, NULL};
  char *name_v[] = {NULL, NULL};
  char *moiraId_v[] = {NULL, NULL};
  char *desc_v[] = {NULL, NULL};
  char *managedBy_v[] = {NULL, NULL};
  char dName[256];
  char cName[256];
  char managedByDN[256];
  char filter[256];
  char temp[256];
  int  n;
  int  i;
  int  rc;

  memset(filter, '\0', sizeof(filter));
  memset(dName, '\0', sizeof(dName));
  memset(cName, '\0', sizeof(cName));
  memset(managedByDN, '\0', sizeof(managedByDN));
  container_get_dn(av[CONTAINER_NAME], dName);
  container_get_name(av[CONTAINER_NAME], cName);

  if ((strlen(cName) == 0) || (strlen(dName) == 0))
    {
      com_err(whoami, 0, "Unable to process invalid LDAP container name %s", cName);
      return(AD_INVALID_NAME);
    }

  if (!check_container_name(cName))
    {
      com_err(whoami, 0, "Unable to process invalid LDAP container name %s", cName);
      return(AD_INVALID_NAME);
    }

  n = 0;
  ADD_ATTR("objectClass", objectClass_v, LDAP_MOD_ADD);
  name_v[0] = cName;
  ADD_ATTR("name", name_v, LDAP_MOD_ADD);
  ou_v[0] = cName;
  ADD_ATTR("ou", ou_v, LDAP_MOD_ADD);
  if (strlen(av[CONTAINER_ROWID]) != 0)
    {
      moiraId_v[0] = av[CONTAINER_ROWID];
      ADD_ATTR("mitMoiraId", moiraId_v, LDAP_MOD_ADD);
    }
  if (strlen(av[CONTAINER_DESC]) != 0)
    {
      desc_v[0] = av[CONTAINER_DESC];
      ADD_ATTR("description", desc_v, LDAP_MOD_ADD);
    }
  if ((strlen(av[CONTAINER_TYPE]) != 0) && (strlen(av[CONTAINER_ID]) != 0))
    {
	  if (!strcasecmp(av[CONTAINER_TYPE], "KERBEROS"))
	  {
		  if (!contact_create(ldap_handle, dn_path, av[CONTAINER_ID], kerberos_ou))
		  {
			sprintf(managedByDN, "CN=%s,%s,%s", av[CONTAINER_ID], kerberos_ou,dn_path);
			managedBy_v[0] = managedByDN;
            		ADD_ATTR("managedBy", managedBy_v, LDAP_MOD_ADD);
		  }
	  }
	  else
	  {
		if (!strcasecmp(av[CONTAINER_TYPE], "USER"))
        {
          sprintf(filter, "(&(cn=%s)(&(objectCategory=person)(objectClass=user)))", av[CONTAINER_ID]);
        }
		if (!strcasecmp(av[CONTAINER_TYPE], "LIST"))
        {
          sprintf(filter, "(&(objectClass=group)(cn=%s))", av[CONTAINER_ID]);
        }
		if (strlen(filter) != 0)
        {
          attr_array[0] = "distinguishedName";
          attr_array[1] = NULL;
          group_count = 0;
          group_base = NULL;
          if ((rc = linklist_build(ldap_handle, dn_path, filter, attr_array, 
                                   &group_base, &group_count, LDAP_SCOPE_SUBTREE)) == LDAP_SUCCESS)
            {
              if (group_count == 1)
                {
                  strcpy(managedByDN, group_base->value);
                  managedBy_v[0] = managedByDN;
                  ADD_ATTR("managedBy", managedBy_v, LDAP_MOD_ADD);
                }
              linklist_free(group_base);
              group_base = NULL;
              group_count = 0;
            }
        }
    }
  }
  mods[n] = NULL;

  sprintf(temp, "%s,%s", dName, dn_path);
  rc = ldap_add_ext_s(ldap_handle, temp, mods, NULL, NULL);
  for (i = 0; i < n; i++)
    free(mods[i]);
  if ((rc != LDAP_SUCCESS) && (rc != LDAP_ALREADY_EXISTS))
    {
      com_err(whoami, 0, "Unable to create container %s : %s",
              cName, ldap_err2string(rc));
      return(rc);
    }
  if (rc == LDAP_ALREADY_EXISTS)
    {
      if (strlen(av[CONTAINER_ROWID]) != 0)
        rc = container_adupdate(ldap_handle, dn_path, dName, "", count, av);
    }
  return(rc);
}

int container_update(LDAP *ldap_handle, char *dn_path, int beforec, char **before, 
                     int afterc, char **after)
{
  char distinguishedName[256];
  int  rc;

  memset(distinguishedName, '\0', sizeof(distinguishedName));
  if (rc = container_get_distinguishedName(ldap_handle, dn_path, distinguishedName, afterc, after))
    return(rc);
  if (strlen(distinguishedName) == 0)
    {
      rc = container_create(ldap_handle, dn_path, afterc, after);
      return(rc);
    }

  container_check(ldap_handle, dn_path, after[CONTAINER_NAME]);
  rc = container_adupdate(ldap_handle, dn_path, "", distinguishedName, afterc, after);

  return(rc);
}

int container_get_distinguishedName(LDAP *ldap_handle, char *dn_path, char *distinguishedName, int count, char **av)
{
  char      *attr_array[3];
  LK_ENTRY  *group_base;
  int       group_count;
  char      dName[256];
  char      cName[256];
  char      filter[512];
  int       rc;

  memset(filter, '\0', sizeof(filter));
  memset(dName, '\0', sizeof(dName));
  memset(cName, '\0', sizeof(cName));
  container_get_dn(av[CONTAINER_NAME], dName);
  container_get_name(av[CONTAINER_NAME], cName);

  if (strlen(dName) == 0)
    {
      com_err(whoami, 0, "Unable to process invalid LDAP container name %s", av[CONTAINER_NAME]);
      return(AD_INVALID_NAME);
    }

  if (!check_container_name(cName))
    {
      com_err(whoami, 0, "Unable to process invalid LDAP container name %s", cName);
      return(AD_INVALID_NAME);
    }

  sprintf(filter, "(&(objectClass=organizationalUnit)(mitMoiraId=%s))", av[CONTAINER_ROWID]);
  attr_array[0] = "distinguishedName";
  attr_array[1] = NULL;
  group_count = 0;
  group_base = NULL;
  if ((rc = linklist_build(ldap_handle, dn_path, filter, attr_array, 
                           &group_base, &group_count, LDAP_SCOPE_SUBTREE)) == LDAP_SUCCESS)
    {
      if (group_count == 1)
        {
          strcpy(distinguishedName, group_base->value);
        }
      linklist_free(group_base);
      group_base = NULL;
      group_count = 0;
    }
  if (strlen(distinguishedName) == 0)
    {
      sprintf(filter, "(&(objectClass=organizationalUnit)(distinguishedName=%s,%s))", dName, dn_path);
      attr_array[0] = "distinguishedName";
      attr_array[1] = NULL;
      group_count = 0;
      group_base = NULL;
      if ((rc = linklist_build(ldap_handle, dn_path, filter, attr_array, 
                               &group_base, &group_count, LDAP_SCOPE_SUBTREE)) == LDAP_SUCCESS)
        {
          if (group_count == 1)
            {
              strcpy(distinguishedName, group_base->value);
            }
          linklist_free(group_base);
          group_base = NULL;
          group_count = 0;
        }
    }
  return(0);
}

int container_adupdate(LDAP *ldap_handle, char *dn_path, char *dName, 
                       char *distinguishedName, int count, char **av)
{
  char      *attr_array[5];
  LK_ENTRY  *group_base;
  LK_ENTRY  *pPtr;
  LDAPMod   *mods[20];
  int       group_count;
  char      filter[512];
  char      *moiraId_v[] = {NULL, NULL};
  char      *desc_v[] = {NULL, NULL};
  char      *managedBy_v[] = {NULL, NULL};
  char      managedByDN[256];
  char      moiraId[64];
  char      desc[256];
  char      ad_path[512];
  int       rc;
  int       i;
  int       n;


  strcpy(ad_path, distinguishedName);
  if (strlen(dName) != 0)
    sprintf(ad_path, "%s,%s", dName, dn_path);

  sprintf(filter, "(&(objectClass=organizationalUnit)(distinguishedName=%s))", ad_path);
  if (strlen(av[CONTAINER_ID]) != 0)
    sprintf(filter, "(&(objectClass=organizationalUnit)(mitMoiraId=%s))", av[CONTAINER_ROWID]);
  attr_array[0] = "mitMoiraId";
  attr_array[1] = "description";
  attr_array[2] = "managedBy";
  attr_array[3] = NULL;
  group_count = 0;
  group_base = NULL;
  if ((rc = linklist_build(ldap_handle, dn_path, filter, attr_array, 
                           &group_base, &group_count, LDAP_SCOPE_SUBTREE)) != LDAP_SUCCESS)
    {
      com_err(whoami, 0, "Unable to retreive container info for %s : %s",
              av[CONTAINER_NAME], ldap_err2string(rc));
      return(rc);
    }
  memset(managedByDN, '\0', sizeof(managedByDN));
  memset(moiraId, '\0', sizeof(moiraId));
  memset(desc, '\0', sizeof(desc));
  pPtr = group_base;
  while (pPtr)
    {
      if (!strcasecmp(pPtr->attribute, "description"))
        strcpy(desc, pPtr->value);
      else if (!strcasecmp(pPtr->attribute, "managedBy"))
        strcpy(managedByDN, pPtr->value);
      else if (!strcasecmp(pPtr->attribute, "mitMoiraId"))
        strcpy(moiraId, pPtr->value);
      pPtr = pPtr->next;
    }
  linklist_free(group_base);
  group_base = NULL;
  group_count = 0;

  n = 0;
  if (strlen(av[CONTAINER_ROWID]) != 0)
    {
      moiraId_v[0] = av[CONTAINER_ROWID];
      ADD_ATTR("mitMoiraId", moiraId_v, LDAP_MOD_REPLACE);
    }
  if (strlen(av[CONTAINER_DESC]) != 0)
    {
      attribute_update(ldap_handle, ad_path, av[CONTAINER_DESC], "description", dName);
    }
  else
    {
      if (strlen(desc) != 0)
        {
          attribute_update(ldap_handle, ad_path, "", "description", dName);
        }
    }
  if ((strlen(av[CONTAINER_TYPE]) != 0) && (strlen(av[CONTAINER_ID]) != 0))
    {
	  if (!strcasecmp(av[CONTAINER_TYPE], "KERBEROS"))
	  {
		  if (!contact_create(ldap_handle, dn_path, av[CONTAINER_ID], kerberos_ou))
		  {
			sprintf(managedByDN, "CN=%s,%s,%s", av[CONTAINER_ID], kerberos_ou, dn_path);
			managedBy_v[0] = managedByDN;
            ADD_ATTR("managedBy", managedBy_v, LDAP_MOD_REPLACE);
		  }
		  else
		  {
			if (strlen(managedByDN) != 0)
			{
                attribute_update(ldap_handle, ad_path, "", "managedBy", dName);
			}
		  }
	  }
	  else
	  {
		memset(filter, '\0', sizeof(filter));
		if (!strcasecmp(av[CONTAINER_TYPE], "USER"))
        {
          sprintf(filter, "(&(cn=%s)(&(objectCategory=person)(objectClass=user)))", av[CONTAINER_ID]);
        }
		if (!strcasecmp(av[CONTAINER_TYPE], "LIST"))
        {
          sprintf(filter, "(&(objectClass=group)(cn=%s))", av[CONTAINER_ID]);
        }
		if (strlen(filter) != 0)
        {
          attr_array[0] = "distinguishedName";
          attr_array[1] = NULL;
          group_count = 0;
          group_base = NULL;
          if ((rc = linklist_build(ldap_handle, dn_path, filter, attr_array, 
                                   &group_base, &group_count, LDAP_SCOPE_SUBTREE)) == LDAP_SUCCESS)
            {
              if (group_count == 1)
                {
                  strcpy(managedByDN, group_base->value);
                  managedBy_v[0] = managedByDN;
                  ADD_ATTR("managedBy", managedBy_v, LDAP_MOD_REPLACE);
                }
              else
                {
                  if (strlen(managedByDN) != 0)
                    {
                        attribute_update(ldap_handle, ad_path, "", "managedBy", dName);
                    }
                }
              linklist_free(group_base);
              group_base = NULL;
              group_count = 0;
            }
        }
		else
        {
          if (strlen(managedByDN) != 0)
            {
                attribute_update(ldap_handle, ad_path, "", "managedBy", dName);
            }
        }
	  }
    }
  mods[n] = NULL;
  if (n == 0)
    return(LDAP_SUCCESS);

  rc = ldap_modify_s(ldap_handle, ad_path, mods);
  for (i = 0; i < n; i++)
    free(mods[i]);
  if (rc != LDAP_SUCCESS)
    {
        com_err(whoami, 0, "Unable to modify container info for %s : %s",
                av[CONTAINER_NAME], ldap_err2string(rc));
        return(rc);
    }
  return(rc);
}

int container_move_objects(LDAP *ldap_handle, char *dn_path, char *dName)
{
  char      *attr_array[3];
  LK_ENTRY  *group_base;
  LK_ENTRY  *pPtr;
  int       group_count;
  char      filter[512];
  char      new_cn[128];
  char      temp[256];
  int       rc;
  int       NumberOfEntries = 10;
  int       i;
  int       count;

  rc = ldap_set_option(ldap_handle, LDAP_OPT_SIZELIMIT, &NumberOfEntries);

  for (i = 0; i < 3; i++)
    {
      memset(filter, '\0', sizeof(filter));
      if (i == 0)
        {
          strcpy(filter, "(!(|(objectClass=computer)(objectClass=organizationalUnit)))");
          attr_array[0] = "cn";
          attr_array[1] = NULL;
        }
      else if (i == 1)
        {
          strcpy(filter, "(objectClass=computer)");
          attr_array[0] = "cn";
          attr_array[1] = NULL;
        }
      else
        {
          strcpy(filter, "(objectClass=organizationalUnit)");
          attr_array[0] = "ou";
          attr_array[1] = NULL;
        }

      while (1)
        {
          if ((rc = linklist_build(ldap_handle, dName, filter, attr_array, 
                                   &group_base, &group_count, LDAP_SCOPE_SUBTREE)) != LDAP_SUCCESS)
            {
              break;
            }
          if (group_count == 0)
            break;
          pPtr = group_base;
          while(pPtr)
            {
              if (!strcasecmp(pPtr->attribute, "cn"))
                {
                  sprintf(new_cn, "cn=%s", pPtr->value);
                  if (i == 0)
                    sprintf(temp, "%s,%s", orphans_other_ou, dn_path);
                  if (i == 1)
                    sprintf(temp, "%s,%s", orphans_machines_ou, dn_path);
                  count = 1;
                  while (1)
                    {
                      rc = ldap_rename_s(ldap_handle, pPtr->dn, new_cn, temp,
                                         TRUE, NULL, NULL);
                      if (rc == LDAP_ALREADY_EXISTS)
                        {
                          sprintf(new_cn, "cn=%s_%d", pPtr->value, count);
                          ++count;
                        }
                      else
                        break;
                    }
                }
              else if (!strcasecmp(pPtr->attribute, "ou"))
                {
                  rc = ldap_delete_s(ldap_handle, pPtr->dn);
                }
              pPtr = pPtr->next;
            }
          linklist_free(group_base);
          group_base = NULL;
          group_count = 0;
        }
    }
  return(0);
}

int get_machine_ou(LDAP *ldap_handle, char *dn_path, char *member, char *machine_ou, char *NewMachineName)
{
  LK_ENTRY  *group_base;
  int  group_count;
  int  i;
  char filter[128];
  char *attr_array[3];
  char cn[256];
  char dn[256];
  char temp[256];
  char *pPtr;
  int   rc;

  strcpy(NewMachineName, member);
  rc = moira_connect();
  rc = GetMachineName(NewMachineName);
  moira_disconnect();
  if (strlen(NewMachineName) == 0)
    {
      com_err(whoami, 0, "Unable to find alais for machine %s in Moira", member);
      return(1);
    }

  pPtr = NULL;
  pPtr = strchr(NewMachineName, '.');
  if (pPtr != NULL)
    (*pPtr) = '\0';

  group_base = NULL;
  group_count = 0;
  sprintf(filter, "(sAMAccountName=%s$)", NewMachineName);
  attr_array[0] = "cn";
  attr_array[1] = NULL;
  sprintf(temp, "%s", dn_path);
  if ((rc = linklist_build(ldap_handle, temp, filter, attr_array, 
                        &group_base, &group_count, LDAP_SCOPE_SUBTREE)) != 0)
    {
      com_err(whoami, 0, "Unable to process machine %s : %s",
              member, ldap_err2string(rc));
      return(1);
    }
  if (group_count != 1)
    {
      com_err(whoami, 0, "Unable to process machine %s : machine not found in AD",
              NewMachineName);
      return(1);
    }
  strcpy(dn, group_base->dn);
  strcpy(cn, group_base->value);
  for (i = 0; i < (int)strlen(dn); i++)
    dn[i] = tolower(dn[i]);
  for (i = 0; i < (int)strlen(cn); i++)
    cn[i] = tolower(cn[i]);
  linklist_free(group_base);
  pPtr = NULL;
  pPtr = strstr(dn, cn);
  if (pPtr == NULL)
    {
      com_err(whoami, 0, "Unable to process machine %s",
              member);
      return(1);
    }
  pPtr += strlen(cn) + 1;
  strcpy(machine_ou, pPtr);
  pPtr = NULL;
  pPtr = strstr(machine_ou, "dc=");
  if (pPtr == NULL)
    {
      com_err(whoami, 0, "Unable to process machine %s",
              member);
      return(1);
    }
  --pPtr;
  (*pPtr) = '\0';
  return(0);
}

int machine_move_to_ou(LDAP *ldap_handle, char * dn_path, char *MoiraMachineName, char *DestinationOu)
{

    char        NewCn[128];
    char        OldDn[512];
    char        MachineName[128];
    char        filter[128];
    char        *attr_array[3];
    char        NewOu[256];
    char        *cPtr = NULL;
    int         group_count;
    long        rc;
    LK_ENTRY    *group_base;

    group_count = 0;
    group_base = NULL;

    strcpy(MachineName, MoiraMachineName);
    rc = GetMachineName(MachineName);
    if (strlen(MachineName) == 0)
    {
        com_err(whoami, 0, "Unable to find alais for machine %s in Moira", MoiraMachineName);
        return(1);
    }

    cPtr = strchr(MachineName, '.');
    if (cPtr != NULL)
        (*cPtr) = '\0';
    sprintf(filter, "(sAMAccountName=%s$)", MachineName);
    attr_array[0] = "sAMAccountName";
    attr_array[1] = NULL;
    if ((rc = linklist_build(ldap_handle, dn_path, filter, attr_array, &group_base, 
                             &group_count, LDAP_SCOPE_SUBTREE)) != 0)
    {
        com_err(whoami, 0, "Unable to process machine %s : %s",
                MoiraMachineName, ldap_err2string(rc));
        return(1);
    }

    if (group_count == 1)
        strcpy(OldDn, group_base->dn);
    linklist_free(group_base);
    group_base = NULL;
    if (group_count != 1)
    {
        com_err(whoami, 0, "Unable to find machine %s in AD: %s", MoiraMachineName);
        return(1);
    }
    sprintf(NewOu, "%s,%s", DestinationOu, dn_path);
    cPtr = strchr(OldDn, ',');
    if (cPtr != NULL)
    {
        ++cPtr;
        if (!strcasecmp(cPtr, NewOu))
            return(0);
    }
    sprintf(NewCn, "CN=%s", MachineName);
    rc = ldap_rename_s(ldap_handle, OldDn, NewCn, NewOu, TRUE, NULL, NULL);
    return(rc);
}

int machine_check(LDAP *ldap_handle, char *dn_path, char *machine_name)
{
    char    Name[128];
    char    *pPtr;
    int     rc;

    memset(Name, '\0', sizeof(Name));
    strcpy(Name, machine_name);
    pPtr = NULL;
    pPtr = strchr(Name, '.');
    if (pPtr != NULL)
        (*pPtr) = '\0';
    strcat(Name, "$");
    return(!(rc = checkADname(ldap_handle, dn_path, Name)));
}

int machine_get_moira_container(LDAP *ldap_handle, char *dn_path, char *machine_name, char *container_name)
{
    int     rc;
    char    *av[2];
    char    *call_args[2];

    av[0] = machine_name;
    call_args[0] = (char *)container_name;
    rc = mr_query("get_machine_to_container_map", 1, av, machine_GetMoiraContainer,
                   call_args);
    return(rc);
}

int machine_GetMoiraContainer(int ac, char **av, void *ptr)
{
    char **call_args;
    
    call_args = ptr;
    strcpy(call_args[0], av[1]);
    return(0);
}

int Moira_container_group_create(char **after)
{
  long rc;
  char GroupName[64];
  char *argv[20];
  
  memset(GroupName, '\0', sizeof(GroupName));
  rc = Moira_groupname_create(GroupName, after[CONTAINER_NAME], 
			      after[CONTAINER_ROWID]);
  if (rc)
    return rc;
  
  argv[L_NAME] = GroupName;
  argv[L_ACTIVE] = "1";
  argv[L_PUBLIC] = "0";
  argv[L_HIDDEN] = "0";
  argv[L_MAILLIST] = "0";
  argv[L_GROUP] = "1";
  argv[L_GID] = UNIQUE_GID;
  argv[L_NFSGROUP] = "0";
  argv[L_MAILMAN] = "0";
  argv[L_MAILMAN_SERVER] = "[NONE]";
  argv[L_DESC] = "auto created container group";
  argv[L_ACE_TYPE] = "USER";
  argv[L_MEMACE_TYPE] = "USER";
  argv[L_ACE_NAME] = "sms";
  argv[L_MEMACE_NAME] = "sms";

  if (rc = mr_query("add_list", 15, argv, NULL, NULL))
    {
      com_err(whoami, 0, "Unable to create container group %s for container %s: %s",
	      GroupName, after[CONTAINER_NAME], error_message(rc));
    }

  Moira_setContainerGroup(after[CONTAINER_NAME], GroupName);
  Moira_addGroupToParent(after[CONTAINER_NAME], GroupName);
  
  return(rc);
}

int Moira_container_group_update(char **before, char **after)
{
  long rc;
  char BeforeGroupName[64];
  char AfterGroupName[64];
  char *argv[20];
  
  if (!strcasecmp(after[CONTAINER_NAME], before[CONTAINER_NAME]))
    return(0);

  memset(BeforeGroupName, '\0', sizeof(BeforeGroupName));
  Moira_getGroupName(after[CONTAINER_NAME], BeforeGroupName, 0);
  if (strlen(BeforeGroupName) == 0)
    return(0);

  memset(AfterGroupName, '\0', sizeof(AfterGroupName));
  rc = Moira_groupname_create(AfterGroupName, after[CONTAINER_NAME], 
			      after[CONTAINER_ROWID]);
  if (rc)
    return rc;

  if (strcasecmp(BeforeGroupName, AfterGroupName))
    {
      argv[L_NAME] = BeforeGroupName;
      argv[L_NAME + 1] = AfterGroupName;
      argv[L_ACTIVE + 1] = "1";
      argv[L_PUBLIC + 1] = "0";
      argv[L_HIDDEN + 1] = "0";
      argv[L_MAILLIST + 1] = "0";
      argv[L_GROUP + 1] = "1";
      argv[L_GID + 1] = UNIQUE_GID;
      argv[L_NFSGROUP + 1] = "0";
      argv[L_MAILMAN + 1] = "0";
      argv[L_MAILMAN_SERVER + 1] = "[NONE]";
      argv[L_DESC + 1] = "auto created container group";
      argv[L_ACE_TYPE + 1] = "USER";
      argv[L_MEMACE_TYPE + 1] = "USER";
      argv[L_ACE_NAME + 1] = "sms";
      argv[L_MEMACE_NAME + 1] = "sms";
      
      if (rc = mr_query("update_list", 16, argv, NULL, NULL))
	{
	  com_err(whoami, 0, "Unable to rename container group from %s to %s: %s",
		  BeforeGroupName, AfterGroupName, error_message(rc));
	}
    }
  
  return(rc);
}

int Moira_container_group_delete(char **before)
{
  long rc = 0;
  char *argv[13];
  char GroupName[64];
  char ParentGroupName[64];
  
  memset(ParentGroupName, '\0', sizeof(ParentGroupName));
  Moira_getGroupName(before[CONTAINER_NAME], ParentGroupName, 1);

  memset(GroupName, '\0', sizeof(GroupName));
  if (strcmp(before[CONTAINER_GROUP_NAME], "[none]"))
    strcpy(GroupName, before[CONTAINER_GROUP_NAME]);
  
  if ((strlen(ParentGroupName) != 0) && (strlen(GroupName) != 0))
    {
      argv[0] = ParentGroupName;
      argv[1] = "LIST";
      argv[2] = GroupName;
      if (rc = mr_query("delete_member_from_list", 3, argv, NULL, NULL))
	{
	  com_err(whoami, 0, "Unable to delete container group %s from list: %s",
		  GroupName, ParentGroupName, error_message(rc));
	}
    }
  
  if (strlen(GroupName) != 0)
    {
      argv[0] = GroupName;
      if (rc = mr_query("delete_list", 1, argv, NULL, NULL))
	{
	  com_err(whoami, 0, "Unable to delete container group %s : %s",
		  GroupName, error_message(rc));
	}
     }
  
  return(rc);
}

int Moira_groupname_create(char *GroupName, char *ContainerName,
			   char *ContainerRowID)
{
  char *ptr;
  char *ptr1;
  char temp[64];
  char newGroupName[64];
  char tempGroupName[64];
  char tempgname[64];
  char *argv[1];
  int  i;
  long rc;

  strcpy(temp, ContainerName);
  
  ptr1 = strrchr(temp, '/');
  if (ptr1 != NULL)
  {
    *ptr1 = '\0';
    ptr = ++ptr1;
    ptr1 = strrchr(temp, '/');
    if (ptr1 != NULL)
    {
        sprintf(tempgname, "%s-%s", ++ptr1, ptr);
    }
    else
        strcpy(tempgname, ptr);
  }
  else
    strcpy(tempgname, temp);

  if (strlen(tempgname) > 25)
    tempgname[25] ='\0';

  sprintf(newGroupName, "cnt-%s", tempgname);

  /* change everything to lower case */
  ptr = newGroupName;
  while (*ptr)
    {
      if (isupper(*ptr))
	*ptr = tolower(*ptr);
      if (*ptr == ' ')
	*ptr = '-';
      ptr++;
    }

  strcpy(tempGroupName, newGroupName);
  i = (int)'0';
  /* append 0-9 then a-z if a duplicate is found */
  while(1)
    {
      argv[0] = newGroupName;
      if (rc = mr_query("get_list_info", 1, argv, NULL, NULL))
	{
	  if (rc == MR_NO_MATCH)
	    break;
	  com_err(whoami, 0, "Moira error while creating group name for container %s : %s",
		  ContainerName, error_message(rc));
	  return rc;
	}
      sprintf(newGroupName, "%s-%c", tempGroupName, i);
      if (i == (int)'z')
	{
	  com_err(whoami, 0, "Unable to find a unique group name for container %s: too many duplicate container names",
		  ContainerName);
	  return 1;
	}
      if (i == '9')
	i = 'a';
      else
	i++;
    }

  strcpy(GroupName, newGroupName);
  return(0);
}

int Moira_setContainerGroup(char *origContainerName, char *GroupName)
{
  long rc;
  char *argv[3];
  
  argv[0] = origContainerName;
  argv[1] = GroupName;

  if ((rc = mr_query("set_container_list", 2, argv, NULL, NULL)))
    {
         com_err(whoami, 0, "Unable to set container group %s in container %s: %s",
                 GroupName, origContainerName, error_message(rc));
    }

  return(0);
}

 int Moira_addGroupToParent(char *origContainerName, char *GroupName)
 {
   char ContainerName[64];
   char ParentGroupName[64];
   char *argv[3];
   long rc;

   strcpy(ContainerName, origContainerName);

   Moira_getGroupName(ContainerName, ParentGroupName, 1);
   /* top-level container */
   if (strlen(ParentGroupName) == 0)
     return(0);
   
   argv[0] = ParentGroupName;
   argv[1] = "LIST";
   argv[2] = GroupName;
   if ((rc = mr_query("add_member_to_list", 3, argv, NULL, NULL)))
     {
       com_err(whoami, 0, "Unable to add container group %s to parent group %s: %s",
	       GroupName, ParentGroupName, error_message(rc));
     }
   return(0);
 }

int Moira_getContainerGroup(int ac, char **av, void *ptr)
{
  char **call_args;
  
  call_args = ptr;
  strcpy(call_args[0], av[1]);
  return(0);
}

int Moira_getGroupName(char *origContainerName, char *GroupName,
		       int ParentFlag)
{
  char ContainerName[64];
  char *argv[3];
  char *call_args[3];
  char *ptr;
  long rc;

  strcpy(ContainerName, origContainerName);

  if (ParentFlag)
    {
      ptr = strrchr(ContainerName, '/');
      if (ptr != NULL)
	(*ptr) = '\0';
      else
	return(0);
    }

  argv[0] = ContainerName;
  argv[1] = NULL;
  call_args[0] = GroupName;
  call_args[1] = NULL;

  if (!(rc = mr_query("get_container_list", 1, argv, Moira_getContainerGroup,
		      call_args)))
    {
      if (strlen(GroupName) != 0)
	return(0);
    }

  if (rc)
    com_err(whoami, 0, "Unable to get container group from container %s: %s",
	    ContainerName, error_message(rc));
  else
    com_err(whoami, 0, "Unable to get container group from container %s",
	    ContainerName);
     return(0);
}

int Moira_process_machine_container_group(char *MachineName, char* GroupName, 
					  int DeleteMachine)
{
  char *argv[3];
  long rc;

  if (strcmp(GroupName, "[none]") == 0)
    return 0;

  argv[0] = GroupName;
  argv[1] = "MACHINE";
  argv[2] = MachineName;
  if (!DeleteMachine)
    rc = mr_query("add_member_to_list", 3, argv, NULL, NULL);
  else
    rc = mr_query("delete_member_from_list", 3, argv, NULL, NULL);
  if (rc)
    {
      com_err(whoami, 0, "Unable to add machine %s to container group%s: %s",
	      MachineName, GroupName, error_message(rc));
    }
  return(0);
}

int GetMachineName(char *MachineName)
{
	char    *args[2];
    char    NewMachineName[1024];
    char    *szDot;
    int     rc = 0;
    int     i;
    DWORD   dwLen = 0;
    char    *call_args[2];

    // If the address happens to be in the top-level MIT domain, great!
    strcpy(NewMachineName, MachineName);
    for (i = 0; i < (int)strlen(NewMachineName); i++)
        NewMachineName[i] = toupper(NewMachineName[i]);
    szDot = strchr(NewMachineName,'.');
    if ((szDot) && (!strcasecmp(szDot+1, DOMAIN_SUFFIX)))
    {
        return(0);
    }
   	
    // If not, see if it has a Moira alias in the top-level MIT domain.
    memset(NewMachineName, '\0', sizeof(NewMachineName));
	args[0] = "*";
    args[1] = MachineName;
    call_args[0] = NewMachineName;
    call_args[1] = NULL;
    if (rc = mr_query("get_hostalias", 2, args, ProcessMachineName, call_args))
    {
        com_err(whoami, 0, "Unable to resolve machine name %s : %s",
                MachineName, error_message(rc));
        strcpy(MachineName, "");
        return(0);
    }

    if (strlen(NewMachineName) != 0)
        strcpy(MachineName, NewMachineName);
    else
        strcpy(MachineName, "");
    return(0);

}

int ProcessMachineName(int ac, char **av, void *ptr)
{
    char    **call_args;
    char    MachineName[1024];
    char    *szDot;
    int     i;

    call_args = ptr;
    if (strlen(call_args[0]) == 0)
    {
        strcpy(MachineName, av[0]);
        for (i = 0; i < (int)strlen(MachineName); i++)
            MachineName[i] = toupper(MachineName[i]);
        szDot = strchr(MachineName,'.');
        if ((szDot) && (!strcasecmp(szDot+1,DOMAIN_SUFFIX)))
        {
            strcpy(call_args[0], MachineName);
        }
    }
    return(0);
}

void SwitchSFU(LDAPMod **mods, int *UseSFU30, int n)
{
    int i;

    if (*UseSFU30)
    {
        for (i = 0; i < n; i++)
        {
            if (!strcmp(mods[i]->mod_type, "msSFU30UidNumber"))
                mods[i]->mod_type = "uidNumber";
        }
        (*UseSFU30) = 0;
    }
    else
    {
        for (i = 0; i < n; i++)
        {
            if (!strcmp(mods[i]->mod_type, "uidNumber"))
                mods[i]->mod_type = "msSFU30UidNumber";
        }
        (*UseSFU30) = 1;
    }
}

int SetHomeDirectory(LDAP *ldap_handle, char *user_name, char *DistinguishedName,
                     char *WinHomeDir, char *WinProfileDir,
                     char **homedir_v, char **winProfile_v,
                     char **drives_v, LDAPMod **mods, 
                     int OpType, int n)
{
    char **hp;
    char cWeight[3];
    char cPath[1024];
    char path[1024];
    char winPath[1024];
    char winProfile[1024];
    char homeDrive[8];
    int  last_weight;
    int  i;
    int  rc;
    LDAPMod *DelMods[20];

    memset(homeDrive, '\0', sizeof(homeDrive));
    memset(path, '\0', sizeof(path));
    memset(winPath, '\0', sizeof(winPath));
    memset(winProfile, '\0', sizeof(winProfile));
    hp = NULL;
    if ((!strcasecmp(WinHomeDir, "[afs]")) || (!strcasecmp(WinProfileDir, "[afs]")))
    {
        if ((hp = hes_resolve(user_name, "filsys")) != NULL)
        {
            memset(cWeight, 0, sizeof(cWeight));
            memset(cPath, 0, sizeof(cPath));
            last_weight = 1000;
            i = 0;
            while (hp[i] != NULL)
            {
                if (sscanf(hp[i], "%*s %s", cPath))
                {
                    if (strnicmp(cPath, AFS, strlen(AFS)) == 0)
                    {
                        if (sscanf(hp[i], "%*s %*s %*s %*s %s", cWeight))
                        {
                            if (atoi(cWeight) < last_weight)
                            {
                                strcpy(path, cPath);
                                last_weight = (int)atoi(cWeight);
                            }
                        }
                        else 
                            strcpy(path, cPath);
                    }
                }
              ++i;
            }
            if (strlen(path))
            {
                if (!strnicmp(path, AFS, strlen(AFS)))
                {
                    AfsToWinAfs(path, winPath);
                    strcpy(winProfile, winPath);
                    strcat(winProfile, "\\.winprofile");
                }
            }
        }
        else
            return(n);
    }

    if ((!strcasecmp(WinHomeDir, "[dfs]")) || (!strcasecmp(WinProfileDir, "[dfs]")))
    {
        sprintf(path, "\\\\%s\\dfs\\profiles\\%c\\%s", ldap_domain, user_name[0], user_name);
	if (!strcasecmp(WinProfileDir, "[dfs]"))
	  {
	    strcpy(winProfile, path);
	    strcat(winProfile, "\\.winprofile");
	  }
	if (!strcasecmp(WinHomeDir, "[dfs]"))
	  strcpy(winPath, path);
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

    if (!strcasecmp(WinHomeDir, "[local]"))
        memset(winPath, '\0', sizeof(winPath));
    else if (!strcasecmp(WinHomeDir, "[afs]") || !strcasecmp(WinHomeDir, "[dfs]"))
    {
        strcpy(homeDrive, "H:");
    }
    else
    {
        strcpy(winPath, WinHomeDir);
        if (!strncmp(WinHomeDir, "\\\\", 2))
        {
            strcpy(homeDrive, "H:");
        }        
    }

    // nothing needs to be done if WinProfileDir is [afs].
    if (!strcasecmp(WinProfileDir, "[local]"))
        memset(winProfile, '\0', sizeof(winProfile));
    else if (strcasecmp(WinProfileDir, "[afs]") && strcasecmp(WinProfileDir, "[dfs]"))
    {
        strcpy(winProfile, WinProfileDir);
    }

    if (strlen(winProfile) != 0)
    {
        if (winProfile[strlen(winProfile) - 1] == '\\')
            winProfile[strlen(winProfile) - 1] = '\0';
    }
    if (strlen(winPath) != 0)
    {
        if (winPath[strlen(winPath) - 1] == '\\')
            winPath[strlen(winPath) - 1] = '\0';
    }

    if ((winProfile[1] == ':') && (strlen(winProfile) == 2))
        strcat(winProfile, "\\");
    if ((winPath[1] == ':') && (strlen(winPath) == 2))
        strcat(winPath, "\\");

    if (strlen(winPath) == 0)
    {
        if (OpType == LDAP_MOD_REPLACE)
        {
            i = 0;
            DEL_ATTR("homeDirectory", LDAP_MOD_DELETE);
            DelMods[i] = NULL;
            //unset homeDirectory attribute for user.
            rc = ldap_modify_s(ldap_handle, DistinguishedName, DelMods);
            free(DelMods[0]);
        }
    }
    else
    {
        homedir_v[0] = strdup(winPath);
        ADD_ATTR("homeDirectory", homedir_v, OpType);
    }

    if (strlen(winProfile) == 0)
    {
        if (OpType == LDAP_MOD_REPLACE)
        {
            i = 0;
            DEL_ATTR("profilePath", LDAP_MOD_DELETE);
            DelMods[i] = NULL;
            //unset profilePate attribute for user.
            rc = ldap_modify_s(ldap_handle, DistinguishedName, DelMods);
            free(DelMods[0]);
        }
    }
    else
    {
        winProfile_v[0] = strdup(winProfile);
        ADD_ATTR("profilePath", winProfile_v, OpType);
    }

    if (strlen(homeDrive) == 0)
    {
        if (OpType == LDAP_MOD_REPLACE)
        {
            i = 0;
            DEL_ATTR("homeDrive", LDAP_MOD_DELETE);
            DelMods[i] = NULL;
            //unset homeDrive attribute for user
            rc = ldap_modify_s(ldap_handle, DistinguishedName, DelMods);
            free(DelMods[0]);
        }
    }
    else
    {
        drives_v[0] = strdup(homeDrive);
        ADD_ATTR("homeDrive", drives_v, OpType);
    }

    return(n);
}

int GetServerList(char *ldap_domain, char **ServerList)
{
  unsigned long   rc;
  int             group_count;
  int             UseSFU30;
  int             Count;
  int             i;
  int             k;
  int             ServerListFound;
  char            default_server[256];
  char            dn_path[256];
  char            *attr_array[3];
  char            *sPtr;
  char            base[128];
  char            filter[128];
  LK_ENTRY        *group_base;
  LK_ENTRY        *gPtr;
  LDAP            *ldap_handle;
  FILE            *fptr;

  memset(default_server, '\0', sizeof(default_server));
  memset(dn_path, '\0', sizeof(dn_path));
  for (i = 0; i < MAX_SERVER_NAMES; i++)
    {
      if (ServerList[i] != NULL)
        {
          free(ServerList[i]);
          ServerList[i] = NULL;
        }
    }
  if (rc = ad_connect(&ldap_handle, ldap_domain, dn_path, "", "", default_server, 0, 
                      ServerList))
      return(1);
  memset(ServerList, '\0', sizeof(ServerList[0]) * MAX_SERVER_NAMES);
  group_count = 0;
  group_base = NULL;
  Count = 0;
  ServerListFound = 0;

  strcpy(filter, "(&(objectClass=rIDManager)(fSMORoleOwner=*))");
  attr_array[0] = "fSMORoleOwner";
  attr_array[1] = NULL;
  if (!(rc = linklist_build(ldap_handle, dn_path, filter, attr_array, 
                           &group_base, &group_count, LDAP_SCOPE_SUBTREE)) != 0)
    {
      if (group_count != 0)
        {
          sPtr = strstr(group_base->value, ",CN=");
          if (sPtr != NULL)
            {
              sPtr += strlen(",CN=");
              if (ServerList[0] == NULL)
                  ServerList[0] = calloc(1, 256);
              strcpy(ServerList[0], sPtr);
              sPtr = strstr(ServerList[0], ",");
              if (sPtr != NULL)
                  (*sPtr) = '\0';
              ++Count;
              ServerListFound = 1;
            }
        }
    }
  linklist_free(group_base);

  group_count = 0;
  group_base = NULL;
  attr_array[0] = "cn";
  attr_array[1] = NULL;
  strcpy(filter, "(cn=*)");
  sprintf(base, "cn=Servers,cn=Default-First-Site-Name,cn=Sites,cn=Configuration,%s", dn_path);

  if (!(rc = linklist_build(ldap_handle, base, filter, attr_array, 
                           &group_base, &group_count, LDAP_SCOPE_ONELEVEL)) != 0)
    {
      if (group_count != 0)
        {
          gPtr = group_base;
          while (gPtr != NULL)
            {
              if (ServerListFound != 0)
                {
                  if (!strcasecmp(ServerList[0], gPtr->value))
                    {
                      gPtr = gPtr->next;
                      continue;
                    }
                }
              if (Count < MAX_SERVER_NAMES)
                {
                  if (ServerList[Count] == NULL)
                      ServerList[Count] = calloc(1, 256);
                  strcpy(ServerList[Count], gPtr->value);
                  gPtr = gPtr->next;
                  ++Count;
                }
            }
        }
    }
  linklist_free(group_base);

  UseSFU30 = 0;
  group_count = 0;
  group_base = NULL;

  strcpy(filter, "(cn=msSFU-30-Uid-Number)");
  sprintf(base, "cn=schema,cn=configuration,%s", dn_path);

  if (!(rc = linklist_build(ldap_handle, base, filter, NULL, 
                           &group_base, &group_count, LDAP_SCOPE_SUBTREE)) != 0)
    {
      if (group_count != 0)
        {
          UseSFU30 = 1;
        }
    }
  linklist_free(group_base);
  group_count = 0;
  group_base = NULL;

  if ((fptr = fopen(WINADCFG, "w+")) != NULL)
    {
      fprintf(fptr, "%s %s\n", DOMAIN, ldap_domain);
      if (strlen(PrincipalName) != 0)
          fprintf(fptr, "%s %s\n", PRINCIPALNAME, PrincipalName);
      if (UseSFU30)
          fprintf(fptr, "%s %s\n", MSSFU, SFUTYPE);
      for (i = 0; i < MAX_SERVER_NAMES; i++)
        {
          if (ServerList[i] != NULL)
            {
              fprintf(fptr, "%s %s\n", SERVER, ServerList[i]);
            }
        }
      fclose(fptr);
    }
  ldap_unbind_s(ldap_handle);

  for (i = 0; i < MAX_SERVER_NAMES; i++)
    {
      if (ServerList[i] != NULL)
        {
          if (ServerList[i][strlen(ServerList[i]) - 1] == '\n')
              ServerList[i][strlen(ServerList[i]) - 1] = '\0';
          strcat(ServerList[i], ".");
          strcat(ServerList[i], ldap_domain);
          for (k = 0; k < (int)strlen(ServerList[i]); k++)
              ServerList[i][k] = toupper(ServerList[i][k]);
        }
    }

  return(0);
}

int attribute_update(LDAP *ldap_handle, char *distinguished_name, 
                      char *attribute_value, char *attribute, char *user_name)
{
  char      *mod_v[] = {NULL, NULL};
  LDAPMod   *DelMods[20];
  LDAPMod   *mods[20];
  int       n;
  int       i;
  int       rc;

  if (strlen(attribute_value) == 0)
    {
      i = 0;
      DEL_ATTR(attribute, LDAP_MOD_DELETE);
      DelMods[i] = NULL;
      rc = ldap_modify_s(ldap_handle, distinguished_name, DelMods);
      free(DelMods[0]);
    }
    else
    {
      n = 0;
      mod_v[0] = attribute_value;
      ADD_ATTR(attribute, mod_v, LDAP_MOD_REPLACE);
      mods[n] = NULL;
      if ((rc = ldap_modify_s(ldap_handle, distinguished_name, mods)) != LDAP_SUCCESS)
        {
          free(mods[0]);
          n = 0;
          mod_v[0] = attribute_value;
          ADD_ATTR(attribute, mod_v, LDAP_MOD_ADD);
          mods[n] = NULL;
          if ((rc = ldap_modify_s(ldap_handle, distinguished_name, mods)) != LDAP_SUCCESS)
            {
              com_err(whoami, 0, "Unable to change the %s attribute for %s in the AD : %s",
                      attribute, user_name, ldap_err2string(rc));
            }
        }
      free(mods[0]);
    }
  return(rc);
}

int tickets_get_k5()
{
    char temp[128];
    char KinitPath[128];
    int  retval;
    int  i;
    static char EnvVar[128];
    static char EnvVar1[128];

    strcpy(EnvVar, KRB5CCNAME);
    retval = putenv(EnvVar);
    strcpy(EnvVar1, KRBTKFILE);
    retval = putenv(EnvVar1);

    for (i = 0; i < (int)strlen(PrincipalName); i++)
        PrincipalName[i] = tolower(PrincipalName[i]);
    if (strlen(PrincipalName) == 0)
    {
        strcpy(PrincipalName, PRODUCTION_PRINCIPAL);
        if (strcasecmp(ldap_domain, PRIMARY_DOMAIN))
            strcpy(PrincipalName, TEST_PRINCIPAL);
    }

    memset(KinitPath, '\0',sizeof(KinitPath));
#ifndef _WIN32
    strcpy(KinitPath, "/usr/athena/bin/");
#endif
    sprintf(temp, "%skinit -k -t %s %s", KinitPath, KEYTABFILE, PrincipalName);
    retval = system(temp);
    if (retval)
        return(-1);
    return(0);
}

int get_tickets()
{

    if (tickets_get_k5())
    {
        sleep(1);
        if (tickets_get_k5())
        {
            critical_alert("AD incremental", "%s",
                           "winad.incr incremental failed (unable to get kerberos tickets)");
            exit(1);
        }
    }
    return(0);
}

int destroy_cache(void)
{
  krb5_context		context;
  krb5_ccache			cache;
  krb5_error_code	rc;

  context = NULL;
  cache = NULL;
  if (!krb5_init_context(&context))
    {
      if (!krb5_cc_default(context, &cache))
        rc = krb5_cc_destroy(context, cache);
    }
  if (context != NULL)
    krb5_free_context(context);
  dest_tkt();

  return(rc);
}


void StringTrim(char *StringToTrim)
{
    char    *cPtr;
    char    temp[256];
    int     i;

    if (strlen(StringToTrim) == 0)
        return;

    cPtr = StringToTrim;
    while (isspace(*cPtr))
    {
        ++cPtr;
    }
    strcpy(temp, cPtr);
    if (strlen(temp) == 0)
    {
        strcpy(StringToTrim, temp);
        return;
    }
    while (1)
    {
        i = strlen(temp);
        if (i == 0)
            break;
        if (!isspace(temp[i-1]))
            break;
        temp[i-1] = '\0';
    }

    strcpy(StringToTrim, temp);
    return;
}

void ReadConfigFile()
{
    int     Count;
    int     i;
    int     k;
    char    temp[256];
    char    temp1[256];
    FILE    *fptr;

    Count = 0;

    if ((fptr = fopen(WINADCFG, "r")) != NULL)
    {
        while (fgets(temp, sizeof(temp), fptr) != 0)
        {
            for (i = 0; i < (int)strlen(temp); i++)
                temp[i] = toupper(temp[i]);
            if (temp[strlen(temp) - 1] == '\n')
                temp[strlen(temp) - 1] = '\0';
            StringTrim(temp);
            if (strlen(temp) == 0)
                continue;
            if (!strncmp(temp, DOMAIN, strlen(DOMAIN)))
            {
                if (strlen(temp) > (strlen(DOMAIN)))
                {
                    strcpy(ldap_domain, &temp[strlen(DOMAIN)]);
                    StringTrim(ldap_domain);
                }
            }
            else if (!strncmp(temp, PRINCIPALNAME, strlen(PRINCIPALNAME)))
            {
                if (strlen(temp) > (strlen(PRINCIPALNAME)))
                {
                    strcpy(PrincipalName, &temp[strlen(PRINCIPALNAME)]);
                    StringTrim(PrincipalName);
                }
            }
            else if (!strncmp(temp, SERVER, strlen(SERVER)))
            {
                if (strlen(temp) > (strlen(SERVER)))
                {
                    ServerList[Count] = calloc(1, 256);
                    strcpy(ServerList[Count], &temp[strlen(SERVER)]);
                    StringTrim(ServerList[Count]);
                    ++Count;
                }
            }
            else if (!strncmp(temp, MSSFU, strlen(MSSFU)))
            {
                if (strlen(temp) > (strlen(MSSFU)))
                {
                    strcpy(temp1, &temp[strlen(MSSFU)]);
                    StringTrim(temp1);
                    if (!strcmp(temp1, SFUTYPE))
                        UseSFU30 = 1;
                }
            }
            else if (!strcasecmp(temp, "NOCHANGE"))
            {
                NoChangeConfigFile = 1;
            }
            else
            {
                if (strlen(ldap_domain) != 0)
                {
                    memset(ldap_domain, '\0', sizeof(ldap_domain));
                    break;
                }
                if (strlen(temp) != 0)
                    strcpy(ldap_domain, temp);
            }
        }
        fclose(fptr);
    }

    if (strlen(ldap_domain) == 0)
    {
        critical_alert("incremental", "%s",
                       "winad.incr cannot run due to a configuration error in winad.cfg");
        exit(1);
    }
    if (Count == 0)
        return;
    for (i = 0; i < Count; i++)
    {
        if (ServerList[i] != 0)
        {
            strcat(ServerList[i], ".");
            strcat(ServerList[i], ldap_domain);
            for (k = 0; k < (int)strlen(ServerList[i]); k++)
                ServerList[i][k] = toupper(ServerList[i][k]);
        }
    }

}
