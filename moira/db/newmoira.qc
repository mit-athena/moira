/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/db/newmoira.qc,v 1.2 1988-09-12 17:37:17 mar Exp $
 *
 * Creates a new sms database
 *
 * Copyright 1988 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <stdio.h>
#include "../server/query.h"

static int ingres_errno = 0;
int ingerr();

extern char *tables[];
extern struct query Queries1[], Queries2[];
extern int QueryCount1, QueryCount2;

char *values[] = { "users_id", "mach_id", "clu_id", "list_id",
		   "nfsphys_id", "filsys_id", "strings_id", "dcm_enable",
		   0 };

struct alias { char *name;
	       char *type;
	       char *trans;
	   } aliases[] = {
		   {"ace_type", "TYPE", "USER"},
		   {"ace_type", "TYPE", "LIST"},
		   {"ace_type", "TYPE", "NONE"},
		   {"alias", "TYPE", "FILESYS"},
		   {"alias", "TYPE", "TYPE"},
		   {"alias", "TYPE", "SERVICE"},
		   {"alias", "TYPE", "TYPEDATA"},
		   {"alias", "TYPE", "VALUE"},
		   {"boolean", "TYPE", "FALSE"},
		   {"boolean", "TYPE", "DONTCARE"},
		   {"boolean", "TYPE", "TRUE"},
		   {"class", "TYPE", "STAFF"},
		   {"class", "TYPE", "1988"},
		   {"class", "TYPE", "1989"},
		   {"class", "TYPE", "1990"},
		   {"class", "TYPE", "1991"},
		   {"class", "TYPE", "1992"},
		   {"class", "TYPE", "FACULTY"},
		   {"class", "TYPE", "G"},
		   {"class", "TYPE", "GUEST"},
		   {"class", "TYPE", "UNKNOWN"},
		   {"filesys", "TYPE", "ERR"},
		   {"filesys", "TYPE", "NFS"},
		   {"filesys", "TYPE", "RVD"},
		   {"gaus", "TYPE", "LIST"},
		   {"gaus", "TYPE", "USER"},
		   {"gaus", "TYPE", "RLIST"},
		   {"gaus", "TYPE", "RUSER"},
		   {"lockertype", "TYPE", "HOMEDIR"},
		   {"lockertype", "TYPE", "OTHER"},
		   {"lockertype", "TYPE", "PROJECT"},
		   {"lockertype", "TYPE", "SYSTEM"},
		   {"lockertype", "TYPE", "UNKNOWN"},
		   {"mac_type", "TYPE", "RT"},
		   {"mac_type", "TYPE", "VAX"},
		   {"member", "TYPE", "LIST"},
		   {"member", "TYPE", "STRING"},
		   {"member", "TYPE", "USER"},
		   {"pobox", "TYPE", "SMTP"},
		   {"pobox", "TYPE", "NONE"},
		   {"pobox", "TYPE", "POP"},
		   {"rmember", "TYPE", "LIST"},
		   {"rmember", "TYPE", "STRING"},
		   {"rmember", "TYPE", "USER"},
		   {"rmember", "TYPE", "RLIST"},
		   {"rmember", "TYPE", "RSTRING"},
		   {"rmember", "TYPE", "RUSER"},
		   {"service", "TYPE", "REPLICAT"},
		   {"service", "TYPE", "UNIQUE"},
		   {"LIST", "TYPEDATA", "list"},
		   {"NONE", "TYPEDATA", "none"},
		   {"POP", "TYPEDATA", "machine"},
		   {"RLIST", "TYPEDATA", "list"},
		   {"RSTRING", "TYPEDATA", "string"},
		   {"RUSER", "TYPEDATA", "user"},
		   {"SMTP", "TYPEDATA", "string"},
		   {"STRING", "TYPEDATA", "string"},
		   {"USER", "TYPEDATA", "user"},	
		   0 };


main(argc, argv)
int argc;
char **argv;
{
    char buffer[256];
##  char *table, *user, *value, *query, *trans, *type, *db;
##  int i, flag;

    if (argc == 2)
      db = argv[1];
    else
      db = "sms";
    IIseterr(ingerr);
##  ingres db
    if (ingres_errno) {
	printf("Unable to open the database\n");
	exit(1);
    }

    strcpy(buffer, "root");
    user = buffer;
    while (buffer[0]) {
	for (i = 0; table = tables[i]; i++) {
##	    define permit all on table to user
	    if (ingres_errno) {
		printf("Unable to give %s permission to access table %s\n",
		       user, table);
		exit(1);
	    }
	}
	printf("Enter the name of another user to be given direct access to\n");
	printf("the database, or [RETURN] if none: ");
	fflush(stdout);
	gets(user);
    }

    for (i = 0; table = tables[i]; i++) {
##	repeat append tblstats (#table = @table, modtime = "now")
	if (ingres_errno) {
	    printf("Unable to initialize tblstats for %s\n", table);
	    exit(1);
	}
    }

    for (i = 0; value = values[i]; i++) {
##	repeat append values (#name = @value, #value = 1)
	if (ingres_errno) {
	    printf("Unable to install initial value of %s\n", value);
	    exit(1);
	}
    }

    printf("What is the starting UID to assign: ");
    fflush(stdout);
    gets(buffer);
    i = atoi(buffer);
##  append values (#name = "uid", #value = i)
    if (ingres_errno) {
	printf("Unable to install value for uid\n");
	exit(1);
    }

    printf("What is the starting GID to assign: ");
    fflush(stdout);
    gets(buffer);
    i = atoi(buffer);
##  append values (#name = "gid", #value = i)
    if (ingres_errno) {
	printf("Unable to install value for gid\n");
	exit(1);
    }

    printf("What is the default NFS quota to assign: ");
    fflush(stdout);
    gets(buffer);
    i = atoi(buffer);
##  append values (#name = "def_quota", #value = i)
    if (ingres_errno) {
	printf("Unable to install value for def_quota\n");
	exit(1);
    }

    /* place holder records */
##  append users (login = "[nobody]", potype="NONE")
    if (ingres_errno) {
	printf("Unable to install 0 user\n");
	exit(1);
    }
##  append machine (name = "[none]", #type="NONE")
    if (ingres_errno) {
	printf("Unable to install 0 machine\n");
	exit(1);
    }
##  append cluster (name = "[unassigned]")
    if (ingres_errno) {
	printf("Unable to install 0 cluster\n");
	exit(1);
    }
##  append list (name = "[none]", acl_type = "NONE")
    if (ingres_errno) {
	printf("Unable to install 0 list\n");
	exit(1);
    }

    /* magic for ALL_USERS */
##  append users (login = "default", users_id = 1, potype="NONE")
    if (ingres_errno) {
	printf("Unable to install default user\n");
	exit(1);
    }
##  append list (name = "default", list_id = 1, acl_type = "NONE")
    if (ingres_errno) {
	printf("Unable to install default list\n");
	exit(1);
    }
##  append members (list_id = 1, member_type = "USER", member_id = 1)
    if (ingres_errno) {
	printf("Unable to install default user on default list\n");
	exit(1);
    }

    /* Priveleged users */
##  append users (login = "root", users_id = 2, potype="NONE")
    if (ingres_errno) {
	printf("Unable to install root user\n");
	exit(1);
    }
##  append list (name = "dbadmin", list_id = 2, acl_type = "LIST", acl_id = 2)
    if (ingres_errno) {
	printf("Unable to install default dbadmin list\n");
	exit(1);
    }

    printf("What is the name of a priveleged user: ");
    fflush(stdout);
    gets(buffer);
    user = buffer;
##  append users (login = user, users_id = 3, potype="NONE")
    if (ingres_errno) {
	printf("Unable to install user %s\n", buffer);
	exit(1);
    }
##  append members (list_id = 2, member_type = "USER", member_id = 3)
    if (ingres_errno) {
	printf("Unable to put user %s on list dbadmin\n", buffer);
	exit(1);
    }

    /* And the capacls */
    for (i = 0; i < QueryCount1; i++) {
	query = Queries1[i].name;
	value = Queries1[i].shortname;
##	repeat append capacls (capability = @query, tag = @value, list_id = 2)
	if (ingres_errno) {
	    printf("Unable to install query ACL for %s (%s)\n", query, value);
	    exit(1);
	}
    }
    for (i = 0; i < QueryCount2; i++) {
	query = Queries2[i].name;
	value = Queries2[i].shortname;
##	repeat retrieve (flag = any(capacls.tag where capacls.tag = @value))
	if (!flag) {
##	  repeat append capacls (capability = @query, tag = @value, list_id = 2)
	  if (ingres_errno) {
	      printf("Unable to install query ACL for %s (%s)\n", query, value);
	      exit(1);
	  }
        }
    }
##  append capacls (capability = "trigger_dcm", tag = "tdcm", list_id = 2)

    /* and the aliases */
    for (i = 0; aliases[i].name ; i++) {
	value = aliases[i].name;
	type = aliases[i].type;
	trans = aliases[i].trans;
##	repeat append alias (#name = @value, #type = @type, #trans = @trans)
    }

##  exit
    printf("done.\n");
    exit(0);
}


/*
 * ingerr: (supposedly) called when Ingres indicates an error.
 * I have not yet been able to get this to work to intercept a
 * database open error.
 */

int ingerr(num)
    int *num;
{
    ingres_errno = *num;
    return *num;
}


/* dummy routines */
int access_list() {}
int access_visible_list() {}
int access_maillist() {}
int access_user() {}
int access_login() {}
int access_filesys() {}
int setup_ausr() {}
int setup_umac() {}
int setup_dmac() {}
int setup_uclu() {}
int setup_dclu() {}
int setup_dusr() {}
int setup_dsin() {}
int setup_dshi() {}
int setup_dlis() {}
int setup_afil() {}
int setup_ufil() {}
int setup_ssif() {}
int setup_sshi() {}
int setup_dnfq() {}
int uusr_login_check() {}
int set_modtime() {}
int set_modtime_by_id() {}
int set_user_modtime() {}
int set_user_modtime_by_id() {}
int set_finger_modtime() {}
int set_filesys_modtime() {}
int set_mach_modtime() {}
int set_mach_modtime_by_id() {}
int set_serverhost_modtime() {}
int set_zephyr_modtime() {}
int followup_amtl() {}
int followup_ausr() {}
int followup_delete_list() {}
int followup_add_pobox() {}
int followup_delete_pobox() {}
int followup_usha() {}
int followup_old_gpob() {}
int followup_gsha() {}
int followup_anfq() {}
int followup_gzcl() {}
int followup_fix_acl() {}
int followup_fix_modby() {}
int translate_ids() {}
int expand_list_flags() {}
int add_new_quota() {}
int count_members_of_list() {}
int get_list_is_group() {}
int get_list_is_maillist() {}
int add_locker() {}
int delete_locker() {}
int add_user_group() {}
int set_user_pobox() {}
int get_members_of_list() {}
int get_groups_of_user() {}
int get_groups_of_all_users() {}
int get_all_poboxes() {}
int get_new_poboxes() {}
int get_lists_of_administrator() {}
int followup_gpob() {}
int set_pobox() {}
int setup_spop() {}
int set_pobox_modtime() {}
int setup_dpob() {}
int set_cluster_modtime_by_id() {}
int access_service() {}
int followup_glin() {}
int qualified_get_server() {}
int qualified_get_serverhost() {}
int trigger_dcm() {}
int set_nfsphys_modtime() {}
int setup_alis() {}
int access_member() {}
int get_ace_use() {}
int access_qgli() {}
int qualified_get_lists() {}
int delete_current_quota() {}
int setup_add_group() {}
int setup_dfil() {}
int setup_dnfp() {}
int get_lists_of_member() {}
int access_all() {}
int get_list_info() {}
int register_user() {}
int access_vis_list_by_name() {}
