/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/incremental/afs.c,v 1.30 1992-07-17 15:10:59 probe Exp $
 *
 * Do AFS incremental updates
 *
 * Copyright (C) 1989,1992 by the Massachusetts Institute of Technology
 * for copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

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
#define PR_TRIES 2
#define PR_DELAY 5

#define file_exists(file) (access((file), F_OK) == 0)

char *whoami;

main(argc, argv)
char **argv;
int argc;
{
    int beforec, afterc, i;
    char *table, **before, **after;
    char buf[1024];

    for (i = getdtablesize() - 1; i > 2; i--)
      close(i);

    table = argv[1];
    beforec = atoi(argv[2]);
    before = &argv[4];
    afterc = atoi(argv[3]);
    after = &argv[4 + beforec];
    whoami = argv[0];

    strcpy(buf, table);
    strcat(buf, " (");
    for (i = 0; i < beforec; i++) {
	if (i > 0)
	  strcat(buf, ",");
	strcat(buf, before[i]);
    }
    strcat(buf, ")->(");
    for (i = 0; i < afterc; i++) {
	if (i > 0)
	  strcat(buf, ",");
	strcat(buf, after[i]);
    }
    strcat(buf, ")");
#ifdef DEBUG
    printf("%s\n", buf);
#endif

    initialize_sms_error_table();
    initialize_krb_error_table();

    for (i=0; file_exists(STOP_FILE); i++) {
	if (i > 30) {
	    critical_alert("incremental",
			   "AFS incremental failed (%s exists): %s",
			   STOP_FILE, buf);
	    exit(1);
	}
	sleep(60);
    }

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


do_cmd(cmd)
char *cmd;
{
    int success = 0, tries = 0;

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
    if (atoi(av[5])) {
	sleep(1);				/* give ptserver some time */
	edit_group(1, av[0], "USER", user);
    }
}


do_user(before, beforec, after, afterc)
char **before;
int beforec;
char **after;
int afterc;
{
    int astate, bstate, auid, buid, code, tries;
    char hostname[64];
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

    code=pr_Initialize(1, AFSCONF_CLIENTNAME, 0);
    if (code) {
	critical_alert("incremental", "Couldn't initialize libprot: %s",
		       error_message(code));
	return;
    }
    
    if (astate == bstate) {
	/* Only a modify has to be done */
	tries = 0;
	while (code=pr_ChangeEntry(before[U_NAME], after[U_NAME], auid, "")) {
	    if (++tries > PR_TRIES)
		break;
	    if (code == UNOQUORUM) { sleep(90); continue; }
	    sleep(PR_DELAY);
	}
	if (code) {
	    critical_alert("incremental",
			   "Couldn't change user %s (id %d) to %s (id %d): %s",
			   before[U_NAME], buid, after[U_NAME], auid,
			   error_message(code));
	}
	return;
    }
    if (bstate == 1) {
	tries = 0;
	while (code = pr_DeleteByID(buid)) {
	    if (++tries > PR_TRIES)
		break;
	    if (code == UNOQUORUM) { sleep(90); continue; }
	    sleep(PR_DELAY);
	}
	if (code && code != PRNOENT) {
	    critical_alert("incremental",
			   "Couldn't delete user %s (id %d): %s",
			   before[U_NAME], buid, error_message(code));
	}
	return;
    }
    if (astate == 1) {
	tries = 0;
	while (code = pr_CreateUser(after[U_NAME], &auid)) {
	    if (++tries > PR_TRIES)
		break;
	    if (code == UNOQUORUM) { sleep(90); continue; }
	    sleep(PR_DELAY);
	}
	if (code) {
	    critical_alert("incremental",
			   "Couldn't create user %s (id %d): %s",
			   after[U_NAME], auid, error_message(code));
	    return;
	}

	if (beforec) {
	    /* Reactivating a user; get his group list */
	    gethostname(hostname, sizeof(hostname));
	    code = mr_connect(hostname);
	    if (!code) code = mr_auth("afs.incr");
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
	    if (code)
		critical_alert("incremental",
			       "Couldn't retrieve membership of user %s: %s",
			       after[U_NAME], error_message(code));
	    mr_disconnect();
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
    int ahide, bhide, tries;
    long code, id;
    char hostname[64];
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

    code=pr_Initialize(1, AFSCONF_CLIENTNAME, 0);
    if (code) {
	critical_alert("incremental", "Couldn't initialize libprot: %s",
		       error_message(code));
	return;
    }

    if (agid && bgid) {
	if (strcmp(after[L_NAME], before[L_NAME])) {
	    /* Only a modify is required */
	    strcpy(g1, "system:");
	    strcpy(g2, "system:");
	    strcat(g1, before[L_NAME]);
	    strcat(g2, after[L_NAME]);
	    tries = 0;
	    while (code = pr_ChangeEntry(g1, g2, -agid, "")) {
		if (++tries > PR_TRIES)
		    break;
		if (code == UNOQUORUM) { sleep(90); continue; }
		sleep(PR_DELAY);
	    }
	    if (code) {
		critical_alert("incremental",
			       "Couldn't change group %s (id %d) to %s (id %d): %s",
			       before[L_NAME], -bgid, after[L_NAME], -agid,
			       error_message(code));
	    }
	}
	if (ahide != bhide) {
	    tries = 0;
	    while (code = pr_SetFieldsEntry
		   (-agid, PR_SF_ALLBITS,
		    (ahide ?PRP_STATUS_ANY :PRP_GROUP_DEFAULT)>>PRIVATE_SHIFT,
		    0 /*ngroups*/, 0 /*nusers*/)) {
		if (++tries > PR_TRIES)
		    break;
		if (code == UNOQUORUM) { sleep(90); continue; }
		sleep(PR_DELAY);
	    }
	    if (code) {
		critical_alert("incremental",
			       "Couldn't set flags of group %s: %s",
			       after[L_NAME], error_message(code));
	    }
	}
	return;
    }
    if (bgid) {
	tries = 0;
	while (code = pr_DeleteByID(-bgid)) {
	    if (++tries > PR_TRIES)
		break;
	    if (code == UNOQUORUM) { sleep(90); continue; }
	    sleep(PR_DELAY);
	}
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
	tries = 0;
	while (code = pr_CreateGroup(g1, g2, &id)) {
	    if (++tries > PR_TRIES)
		break;
	    if (code == UNOQUORUM) { sleep(90); continue; }
	    sleep(PR_DELAY);
	}
	if (code) {
	    critical_alert("incremental",
			   "Couldn't create group %s (id %d): %s",
			   after[L_NAME], id, error_message(code));
	    return;
	}
	if (ahide) {
	    tries = 0;
	    while (code = pr_SetFieldsEntry
		   (-agid, PR_SF_ALLBITS,
		    (ahide ?PRP_STATUS_ANY :PRP_GROUP_DEFAULT)>>PRIVATE_SHIFT,
		    0 /*ngroups*/, 0 /*nusers*/)) {
		if (++tries > PR_TRIES)
		    break;
		if (code == UNOQUORUM) { sleep(90); continue; }
		sleep(PR_DELAY);
	    }
	    if (code) {
		critical_alert("incremental",
			       "Couldn't set flags of group %s: %s",
			       after[L_NAME], error_message(code));
	    }
	}

	sleep(1);				/* give ptserver some time */

	/* We need to make sure the group is properly populated */
	if (beforec < L_ACTIVE || atoi(before[L_ACTIVE]) == 0) return;

	gethostname(hostname, sizeof(hostname));
	code = mr_connect(hostname);
	if (!code) code = mr_auth("afs.incr");
	if (code) {
	    critical_alert("incremental",
			   "Error contacting Moira server to resolve %s: %s",
			   after[L_NAME], error_message(code));
	    return;
	}
	av[0] = "LIST";
	av[1] = after[L_NAME];
	get_members(2, av, after[L_NAME]);

	mr_disconnect();
	return;
    }
}


do_member(before, beforec, after, afterc)
char **before;
int beforec;
char **after;
int afterc;
{
    int code;
    char *p;
    
    if ((beforec < 4 || !atoi(before[LM_END])) &&
	(afterc < 4 || !atoi(after[LM_END])))
	return;

    code=pr_Initialize(1, AFSCONF_CLIENTNAME, 0);
    if (code) {
	critical_alert("incremental", "Couldn't initialize libprot: %s",
		       error_message(code));
	return;
    }

    if (afterc) 
	edit_group(1, after[LM_LIST], after[LM_TYPE], after[LM_MEMBER]);
    if (beforec)
	edit_group(0, before[LM_LIST], before[LM_TYPE], before[LM_MEMBER]);
}


get_members(ac, av, group)
    int ac;
    char *av[];
    char *group;
{
    int code=0;

    if (strcmp(av[0], "LIST")) {
	sleep(1);				/* give ptserver some time */
	edit_group(1, group, av[0], av[1]);
    } else {
	code = mr_query("get_end_members_of_list", 1, &av[1],
			get_members, group);
	if (code)
	    critical_alert("incremental",
			   "Couldn't retrieve full membership of %s: %s",
			   group, error_message(code));
    }
    return code;
}


edit_group(op, group, type, member)
    int op;
    char *group;
    char *type;
    char *member;
{
    char *p = 0;
    char buf[PR_MAXNAMELEN];
    int (*fn)();
    int code;
    int tries = 0;
    static char local_realm[REALM_SZ+1] = "";
    extern long pr_AddToGroup(), pr_RemoveUserFromGroup();

    fn = op ? pr_AddToGroup : pr_RemoveUserFromGroup;
    
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

    strcpy(buf, "system:");
    strcat(buf, group);
    sleep(1);					/* give ptserver some time */
    while (code = (*fn)(member, buf)) {
	if (++tries > PR_TRIES)
	    break;
	if (code == UNOQUORUM) { sleep(90); continue; }
	sleep(PR_DELAY);
    }
    if (code) {
	if (op==0 && code == PRNOENT) return;
	if (op==1 && code == PRIDEXIST) return;
	if (strcmp(type, "KERBEROS") || code != PRNOENT) {
	    critical_alert("incremental",
			   "Couldn't %s %s %s %s: %s",
			   op ? "add" : "remove", member,
			   op ? "to" : "from", buf,
			   error_message(code));
	}
    }
    if (p) *p = '@';
}


do_filesys(before, beforec, after, afterc)
char **before;
int beforec;
char **after;
int afterc;
{
    char cmd[1024];
    
    if (beforec < FS_CREATE) {
	if (afterc < FS_CREATE || atoi(after[FS_CREATE])==0 ||
	    strcmp(after[FS_TYPE], "AFS"))
	    return;

	/* new locker creation */
	sprintf(cmd, "%s/perl -I%s %s/afs_create.pl %s %s %s %s %s %s",
		BIN_DIR, BIN_DIR, BIN_DIR,
		after[FS_NAME], after[FS_L_TYPE], after[FS_MACHINE],
		after[FS_PACK], after[FS_OWNER], after[FS_OWNERS]);
	do_cmd(cmd);
	return;
    }

    /* What do we do?  When do we use FS_CREATE?
     * 
     * TYPE change:  AFS->ERR, ERR->AFS: rename/unmount/remount
     * LOCKERTYPE change: rename/remount
     * PACK change: remount
     * LABEL change: rename/remount
     * Deletion: rename/unmount
     */
    if (afterc < FS_CREATE) {
	if (!strcmp(before[FS_TYPE], "AFS"))
	    critical_alert("incremental",
			   "Could not delete AFS filesystem %s: Operation not supported\n",
			   before[FS_NAME]);
	return;
    }

    if (!strcmp(after[FS_TYPE], "AFS")) {
	if (strcmp(before[FS_TYPE], "AFS")) {
	    critical_alert("incremental",
			   "Cannot convert %s to an AFS filesystem: Operation not supported\n",
			   after[FS_NAME]);
	} else {
	    critical_alert("incremental",
			   "Cannot change attributes of AFS filesystem %s: Operation not supported\n",
			   after[FS_NAME]);
	}
	return;
    }
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
    do_cmd(cmd);
    return;
}
