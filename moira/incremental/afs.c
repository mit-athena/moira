/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/incremental/afs.c,v 1.51 1993-11-30 11:50:03 mar Exp $
 *
 * Do AFS incremental updates
 *
 * Copyright (C) 1989,1992 by the Massachusetts Institute of Technology
 * for copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <strings.h>

#include <krb.h>
#include <moira.h>
#include <moira_site.h>

#include <afs/param.h>
#include <afs/cellconfig.h>
#include <afs/venus.h>
#include <afs/ptclient.h>
#include <afs/pterror.h>

#define STOP_FILE "/moira/afs/noafs"
#define file_exists(file) (access((file), F_OK) == 0)

#if defined(vax) && !defined(__STDC__)
#define volatile
#endif

char *whoami;

/* Main stub routines */
int do_user();
int do_list();
int do_member();
int do_filesys();
int do_quota();

/* Support stub routines */
int run_cmd();
int add_user_lists();
int add_list_members();
int check_user();
int edit_group();
int pr_try();
int check_afs();

/* libprot.a routines */
extern long pr_Initialize();
extern long pr_CreateUser();
extern long pr_CreateGroup();
extern long pr_DeleteByID();
extern long pr_ChangeEntry();
extern long pr_SetFieldsEntry();
extern long pr_AddToGroup();
extern long pr_RemoveUserFromGroup();

static char tbl_buf[1024];
static struct member {
	int op;
	char list[33];
	char type[9];
	char member[129];
	struct member *next;
} *member_head = 0;

static int mr_connections=0;

main(argc, argv)
char **argv;
int argc;
{
    int beforec, afterc, i;
    char *table, **before, **after;

    for (i = getdtablesize() - 1; i > 2; i--)
      close(i);

    whoami = ((whoami = rindex(argv[0], '/')) ? whoami++ : argv[0]);

    table = argv[1];
    beforec = atoi(argv[2]);
    before = &argv[4];
    afterc = atoi(argv[3]);
    after = &argv[4 + beforec];

    setlinebuf(stdout);

    strcpy(tbl_buf, table);
    strcat(tbl_buf, " (");
    for (i = 0; i < beforec; i++) {
	if (i > 0)
	  strcat(tbl_buf, ",");
	strcat(tbl_buf, before[i]);
    }
    strcat(tbl_buf, ")->(");
    for (i = 0; i < afterc; i++) {
	if (i > 0)
	  strcat(tbl_buf, ",");
	strcat(tbl_buf, after[i]);
    }
    strcat(tbl_buf, ")");
#ifdef DEBUG
    com_err(whoami, 0, "%s", tbl_buf);
#endif

    initialize_sms_error_table();
    initialize_krb_error_table();

    if (!strcmp(table, "users")) {
	do_user(before, beforec, after, afterc);
    } else if (!strcmp(table, "list")) {
	do_list(before, beforec, after, afterc);
    } else if (!strcmp(table, "members")) {
	do_member(before, beforec, after, afterc);
    } else if (!strcmp(table, "filesys")) {
	do_filesys(before, beforec, after, afterc);
    } else if (!strcmp(table, "quota")) {
	do_quota(before, beforec, after, afterc);
    }
    exit(0);
}


do_user(before, beforec, after, afterc)
char **before;
int beforec;
char **after;
int afterc;
{
    int astate, bstate, auid, buid, code;
    char *av[2];

    auid = buid = astate = bstate = 0;
    if (afterc > U_STATE) astate = atoi(after[U_STATE]);
    if (beforec > U_STATE) bstate = atoi(before[U_STATE]);
    if (afterc > U_UID) auid = atoi(after[U_UID]);
    if (beforec > U_UID) buid = atoi(before[U_UID]);

    /* We consider "half-registered" users to be active */
    if (astate == 2) astate = 1;
    if (bstate == 2) bstate = 1;

    if (astate != 1 && bstate != 1)		/* inactive user */
	return;

    if (astate == bstate && auid == buid && 
	!strcmp(before[U_NAME], after[U_NAME]))
	/* No AFS related attributes have changed */
	return;

    if (astate == bstate) {
	/* Only a modify has to be done */
	com_err(whoami, 0, "Changing user %s (uid %d) to %s (uid %d)",
	       before[U_NAME], buid, after[U_NAME], auid);

	code = pr_try(pr_ChangeEntry, before[U_NAME], after[U_NAME], auid, "");
	if (code) {
	    critical_alert("incremental",
			   "Couldn't change user %s (id %d) to %s (id %d): %s",
			   before[U_NAME], buid, after[U_NAME], auid,
			   error_message(code));
	}
	return;
    }
    if (bstate == 1) {
	com_err(whoami, 0, "Deleting user %s (uid %d)",
	       before[U_NAME], buid);

	code = pr_try(pr_DeleteByID, buid);
	if (code && code != PRNOENT) {
	    critical_alert("incremental",
			   "Couldn't delete user %s (id %d): %s",
			   before[U_NAME], buid, error_message(code));
	}
	return;
    }
    if (astate == 1) {
	com_err(whoami, 0, "%s user %s (uid %d)",
	       ((bstate != 0) ? "Reactivating" : "Creating"),
	       after[U_NAME], auid);
	
	code = pr_try(pr_CreateUser, after[U_NAME], &auid);
	if (code) {
	    critical_alert("incremental",
			   "Couldn't create user %s (id %d): %s",
			   after[U_NAME], auid, error_message(code));
	    return;
	}

	if (bstate != 0) {
	    /* Reactivating a user; get his group list */
	    code = moira_connect();
	    if (code) {
		critical_alert("incremental",
			       "Error contacting Moira server to retrieve grouplist of user %s: %s",
			       after[U_NAME], error_message(code));
		return;
	    }
	    av[0] = "ruser";
	    av[1] = after[U_NAME];
	    code = mr_query("get_lists_of_member", 2, av,
			    add_user_lists, after[U_NAME]);
	    if (code && code != MR_NO_MATCH)
		critical_alert("incremental",
			       "Couldn't retrieve membership of user %s: %s",
			       after[U_NAME], error_message(code));
	    moira_disconnect();
	}
	return;
    }
}


do_list(before, beforec, after, afterc)
char **before;
int beforec;
char **after;
int afterc;
{
    register int agid, bgid;
    int ahide, bhide;
    long code, id;
    char g1[PR_MAXNAMELEN], g2[PR_MAXNAMELEN];
    char *av[2];

    agid = bgid = 0;
    if (beforec > L_GID && atoi(before[L_ACTIVE]) && atoi(before[L_GROUP])) {
	bgid = atoi(before[L_GID]);
	bhide = atoi(before[L_HIDDEN]);
    }
    if (afterc > L_GID && atoi(after[L_ACTIVE]) && atoi(after[L_GROUP])) {
	agid = atoi(after[L_GID]);
	ahide = atoi(after[L_HIDDEN]);
    }

    if (agid == 0 && bgid == 0)			/* Not active groups */
	return;

    if (agid && bgid) {
	if (strcmp(after[L_NAME], before[L_NAME])) {
	    /* Only a modify is required */
	    strcpy(g1, "system:");
	    strcpy(g2, "system:");
	    strcat(g1, before[L_NAME]);
	    strcat(g2, after[L_NAME]);

	    com_err(whoami, 0, "Changing group %s (gid %d) to %s (gid %d)",
		   before[L_NAME], bgid, after[L_NAME], agid);

	    code = pr_try(pr_ChangeEntry, g1, g2, -agid, "");
	    if (code) {
		critical_alert("incremental",
			       "Couldn't change group %s (id %d) to %s (id %d): %s",
			       before[L_NAME], -bgid, after[L_NAME], -agid,
			       error_message(code));
	    }
	}
	if (ahide != bhide) {
	    com_err(whoami, 0, "Making group %s (gid %d) %s",
		   after[L_NAME], agid,
		   (ahide ? "hidden" : "visible"));
	    code = pr_try(pr_SetFieldsEntry, -agid, PR_SF_ALLBITS,
			  (ahide ? PRP_STATUS_ANY : PRP_GROUP_DEFAULT) >>PRIVATE_SHIFT,
			  0 /*ngroups*/, 0 /*nusers*/);
	    if (code) {
		critical_alert("incremental",
			       "Couldn't set flags of group %s: %s",
			       after[L_NAME], error_message(code));
	    }
	}
	return;
    }
    if (bgid) {
	com_err(whoami, 0, "Deleting group %s (gid %d)",
	       before[L_NAME], bgid);
	code = pr_try(pr_DeleteByID, -bgid);
	if (code && code != PRNOENT) {
	    critical_alert("incremental",
			   "Couldn't delete group %s (id %d): %s",
			   before[L_NAME], -bgid, error_message(code));
	}
	return;
    }
    if (agid) {
	strcpy(g1, "system:");
	strcat(g1, after[L_NAME]);
	strcpy(g2, "system:administrators");
	id = -agid;
	com_err(whoami, 0, "Creating %s group %s (gid %d)",
	       (ahide ? "hidden" : "visible"), after[L_NAME], agid);
	code = pr_try(pr_CreateGroup, g1, g2, &id);
	if (code) {
	    critical_alert("incremental",
			   "Couldn't create group %s (id %d): %s",
			   after[L_NAME], id, error_message(code));
	    return;
	}
	if (ahide) {
	    code = pr_try(pr_SetFieldsEntry, -agid, PR_SF_ALLBITS,
			  (ahide ? PRP_STATUS_ANY : PRP_GROUP_DEFAULT) >>PRIVATE_SHIFT,
			  0 /*ngroups*/, 0 /*nusers*/);
	    if (code) {
		critical_alert("incremental",
			       "Couldn't set flags of group %s: %s",
			       after[L_NAME], error_message(code));
	    }
	}

	/* We need to make sure the group is properly populated */
	if (beforec < L_ACTIVE) return;

	code = moira_connect();
	if (code) {
	    critical_alert("incremental",
			   "Error contacting Moira server to resolve %s: %s",
			   after[L_NAME], error_message(code));
	    return;
	}
	av[0] = after[L_NAME];
	code = mr_query("get_end_members_of_list", 1, av,
			add_list_members, after[L_NAME]);
	if (code)
	    critical_alert("incremental",
			   "Couldn't retrieve full membership of list %s: %s",
			   after[L_NAME], error_message(code));
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

do_member(before, beforec, after, afterc)
char **before;
int beforec;
char **after;
int afterc;
{
    int code;
    char *p;

    if (afterc) {
 	if (afterc < LM_EXTRA_END) {
 	    return;
 	} else if (afterc < LM_EXTRA_END) {
 	    if (!atoi(after[LM_EXTRA_GROUP])) return;
 	} else
	  if (!atoi(after[LM_EXTRA_ACTIVE]) && !atoi(after[LM_EXTRA_GROUP]))
	    return;
 	
  	edit_group(1, after[LM_LIST], after[LM_TYPE], after[LM_MEMBER]);
    } else if (beforec) {
 	if (beforec < LM_EXTRA_END) {
 	    return;
 	} else if (beforec < LM_EXTRA_END) {
 	    if (!atoi(before[LM_EXTRA_GROUP])) return;
 	} else
	  if (!atoi(before[LM_EXTRA_ACTIVE]) && !atoi(before[LM_EXTRA_GROUP]))
	    return;
	edit_group(0, before[LM_LIST], before[LM_TYPE], before[LM_MEMBER]);
    }
}


do_filesys(before, beforec, after, afterc)
char **before;
int beforec;
char **after;
int afterc;
{
    char cmd[1024];
    int acreate, atype, btype;

    if (afterc < FS_CREATE) {
	atype = acreate = 0;
    } else {
	atype = !strcmp(after[FS_TYPE], "AFS");
	acreate = atoi(after[FS_CREATE]);
    }

    if (beforec < FS_CREATE) {
	if (acreate == 0 || atype == 0) return;

	/* new locker creation */
	sprintf(cmd, "%s/perl -I%s %s/afs_create.pl %s %s %s %s %s %s",
		BIN_DIR, BIN_DIR, BIN_DIR,
		after[FS_NAME], after[FS_L_TYPE], after[FS_MACHINE],
		after[FS_PACK], after[FS_OWNER], after[FS_OWNERS]);
	run_cmd(cmd);
	return;
    }
    
    btype = !strcmp(before[FS_TYPE], "AFS");
    if (afterc < FS_CREATE) {
	if (btype)
	    critical_alert("incremental",
			   "Cannot delete AFS filesystem %s: Operation not supported",
			   before[FS_NAME]);
	return;
    }

    if (!acreate)
	return;

    /* Are we dealing with AFS lockers (could be type ERR lockers) */
    if (!atype && !btype)
	if (strcmp(before[FS_TYPE], "ERR") || strcmp(after[FS_TYPE], "ERR"))
	    return;
						    
    /* By now, we know we are simply changing AFS filesystem attributes.
     * Operations supported:
     *    Name change:  rename/remount
     *    Path change:  remount
     *    Type change:  ERR<-->AFS
     */

#if 0
    if (strcmp(before[FS_OWNER], after[FS_OWNER]) ||
	strcmp(before[FS_OWNERS], after[FS_OWNERS]))
    {
	critical_alert("incremental",
		       "Cannot change ownership of filesystem %s: Operation not yet supported",
		       after[FS_NAME]);
    }
#endif

    sprintf(cmd, "%s/perl -I%s %s/afs_rename.pl %s %s %s %s %s %s %s %s %s %s",
	    BIN_DIR, BIN_DIR, BIN_DIR,
	    before[FS_NAME], before[FS_MACHINE], before[FS_TYPE],
	    before[FS_L_TYPE], before[FS_PACK],
	    after[FS_NAME], after[FS_MACHINE], after[FS_TYPE],
	    after[FS_L_TYPE], after[FS_PACK]);
    run_cmd(cmd);
}


do_quota(before, beforec, after, afterc)
char **before;
int beforec;
char **after;
int afterc;
{
    char cmd[1024];

    if (afterc < Q_DIRECTORY || strcmp("ANY", after[Q_TYPE]) ||
	strncmp("/afs/", after[Q_DIRECTORY], 5))
	return;

    sprintf(cmd, "%s/perl -I%s %s/afs_quota.pl %s %s",
	    BIN_DIR, BIN_DIR, BIN_DIR,
	    after[Q_DIRECTORY], after[Q_QUOTA]);
    run_cmd(cmd);
    return;
}


run_cmd(cmd)
char *cmd;
{
    int success=0, tries=0;

    check_afs();
    
    while (success == 0 && tries < 2) {
	if (tries++)
	    sleep(90);
	com_err(whoami, 0, "Executing command: %s", cmd);
	if (system(cmd) == 0)
	    success++;
    }
    if (!success)
	critical_alert("incremental", "failed command: %s", cmd);
}


add_user_lists(ac, av, user)
    int ac;
    char *av[];
    char *user;
{
    if (atoi(av[L_ACTIVE]) && atoi(av[L_GROUP]))	/* active group ? */
	edit_group(1, av[L_NAME], "USER", user);
    return 0;
}


add_list_members(ac, av, group)
    int ac;
    char *av[];
    char *group;
{
    edit_group(1, group, av[0], av[1]);
    return 0;
}


check_user(ac, av, ustate)
    int ac;
    char *av[];
    int *ustate;
{
    *ustate = atoi(av[U_STATE]);
    return 0;
}


edit_group(op, group, type, member)
    int op;
    char *group;
    char *type;
    char *member;
{
    char *p = 0;
    char buf[PR_MAXNAMELEN];
    int code, ustate;
    static char local_realm[REALM_SZ+1] = "";
    struct member *m;

    /* The following KERBEROS code allows for the use of entities
     * user@foreign_cell.
     */
    if (!local_realm[0])
	krb_get_lrealm(local_realm, 1);
    if (!strcmp(type, "KERBEROS")) {
	p = index(member, '@');
	if (p && !strcasecmp(p+1, local_realm))
	    *p = 0;
    } else if (strcmp(type, "USER"))
	return;					/* invalid type */

    /* Cannot risk doing another query during a callback */
    /* We could do this simply for type USER, but eventually this may also
     * dynamically add KERBEROS types to the prdb, and we will need to do
     * a query to look up the uid of the null-instance user */
    if (mr_connections) {
	m = (struct member *)malloc(sizeof(struct member));
	if (!m) {
	    critical_alert("incremental", "Out of memory");
	    exit(1);
	}
	m->op = op;
	strcpy(m->list, group);
	strcpy(m->type, type);
	strcpy(m->member, member);
	m->next = member_head;
	member_head = m;
	return;
    }

    strcpy(buf, "system:");
    strcat(buf, group);
    com_err(whoami, 0, "%s %s %s group %s",
	   (op ? "Adding" : "Removing"), member,
	   (op ? "to" : "from"), group);
    code = 0;
    code=pr_try(op ? pr_AddToGroup : pr_RemoveUserFromGroup, member, buf);
    if (code) {
	if (op==1 && code == PRIDEXIST) return;	/* Already added */

	if (code == PRNOENT) {			/* Something is missing */
	    if (op==0) return;			/* Already deleted */
	    if (!strcmp(type, "KERBEROS"))	/* Special instances; ok */
		return;

	    /* Check whether the member being added is an active user */
	    code = moira_connect();
	    if (!code) code = mr_query("get_user_by_login", 1, &member,
				       check_user, &ustate);
	    if (code) {
		critical_alert("incremental",
			       "Error contacting Moira server to lookup user %s: %s",
			       member, error_message(code));
	    }

	    /* We don't use moira_disconnect()
	     * because we may already be in the routine.
	     */
	    mr_disconnect();
	    mr_connections--;

	    if (!code && ustate!=1 && ustate!=2) return; /* inactive user */
	    code = PRNOENT;
	}

	critical_alert("incremental",
		       "Couldn't %s %s %s %s: %s",
		       op ? "add" : "remove", member,
		       op ? "to" : "from", buf,
		       error_message(code));
    }
}


long pr_try(fn, a1, a2, a3, a4, a5, a6, a7, a8)
    long (*fn)();
    char *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8;
{
    static int initd=0;
    volatile register long code;
    register int tries = 0;
#ifdef DEBUG
    char fname[64];
#endif

    check_afs();

    if (initd) {
	code=pr_Initialize(0, AFSCONF_CLIENTNAME, 0);
    } else {
	code = 0;
	initd = 1;
    }
    if (!code) code=pr_Initialize(1, AFSCONF_CLIENTNAME, 0);
    if (code) {
	critical_alert("incremental", "Couldn't initialize libprot: %s",
		       error_message(code));
	return;
    }

    sleep(1);					/* give ptserver room */

    while (code = (*fn)(a1, a2, a3, a4, a5, a6, a7, a8)) {
#ifdef DEBUG
	long t;
	t = time(0);
	if (fn == pr_AddToGroup) strcpy(fname, "pr_AddToGroup");
	else if (fn == pr_RemoveUserFromGroup)
	    strcpy(fname, "pr_RemoveUserFromGroup");
	else if (fn == pr_CreateUser) strcpy(fname, "pr_CreateUser");
	else if (fn == pr_CreateGroup) strcpy(fname, "pr_CreateGroup");
	else if (fn == pr_DeleteByID) strcpy(fname, "pr_DeleteByID");
	else if (fn == pr_ChangeEntry) strcpy(fname, "pr_ChangeEntry");
	else if (fn == pr_SetFieldsEntry) strcpy(fname, "pr_SetFieldsEntry");
	else if (fn == pr_AddToGroup) strcpy(fname, "pr_AddToGroup");
	else
	    sprintf(fname, "pr_??? (0x%08x)", (long)fn);

	com_err(whoami, code, "- %s failed (try %d @%u)", fname, tries+1, t);
#endif
	if (++tries > 2) break;		/* 3 tries */
	
	if (code == UNOQUORUM) sleep(90);
	else sleep(15);

	/* Re-initialize the prdb connection */
    	code=pr_Initialize(0, AFSCONF_CLIENTNAME, 0);
    	if (!code) code=pr_Initialize(1, AFSCONF_CLIENTNAME, 0);
	if (code) {
	    critical_alert("incremental", "Couldn't re-initialize libprot: %s",
			   error_message(code));
	    initd = 0;				/* we lost */
	    break;
	}
    }
    return code;
}


check_afs()
{
    int i;
    
    for (i=0; file_exists(STOP_FILE); i++) {
	if (i > 30) {
	    critical_alert("incremental",
			   "AFS incremental failed (%s exists): %s",
			   STOP_FILE, tbl_buf);
	    exit(1);
	}
	sleep(60);
    }
}


moira_connect()
{
    static char hostname[64];
    long code;

    if (!mr_connections++) {
#ifdef DEBUG
	code = mr_connect("moira");
#else
	gethostname(hostname, sizeof(hostname));
	code = mr_connect(hostname);
#endif
	if (!code) code = mr_auth("afs.incr");
	return code;    
    }
    return 0;
}

moira_disconnect()
{
    struct member *m;
    
    if (!--mr_connections) {
	mr_disconnect();
	while(m = member_head) {
	    edit_group(m->op, m->list, m->type, m->member);
	    member_head = m->next;
	    free(m);
	}
    }
    return 0;
}
