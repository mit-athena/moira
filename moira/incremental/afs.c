/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/incremental/afs.c,v 1.17 1992-05-30 20:17:09 probe Exp $
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

#include <moira.h>
#include <moira_site.h>

#include <afs/param.h>
#include <afs/cellconfig.h>
#include <afs/venus.h>
#include <afs/ptclient.h>
#include <afs/pterror.h>

#define file_exists(file) (access((file), F_OK) == 0)

char *whoami;
char *cellname = "ATHENA.MIT.EDU";

main(argc, argv)
char **argv;
int argc;
{
    int beforec, afterc, i;
    char *table, **before, **after;
#ifdef DEBUG
    char buf[1024];
#endif

    for (i = getdtablesize() - 1; i > 2; i--)
      close(i);

    table = argv[1];
    beforec = atoi(argv[2]);
    before = &argv[4];
    afterc = atoi(argv[3]);
    after = &argv[4 + beforec];
    whoami = argv[0];

#ifdef DEBUG
    sprintf(buf, "%s (", table);
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
    strcat(buf, ")\n");
    write(1,buf,strlen(buf));
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


do_cmd(cmd)
char *cmd;
{
    int success = 0, tries = 0;

    while (success == 0 && tries < 1) {
	if (tries++)
	    sleep(5*60);
	com_err(whoami, 0, "Executing command: %s", cmd);
	if (system(cmd) == 0)
	    success++;
    }
    if (!success)
	critical_alert("incremental", "failed command: %s", cmd);
}


do_user(before, beforec, after, afterc)
char **before;
int beforec;
char **after;
int afterc;
{
    int astate, bstate, auid, buid, code;

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
	code = pr_ChangeEntry(before[U_NAME], after[U_NAME], auid, "");
	if (code) {
	    critical_alert("incremental",
			   "Couldn't change user %s (id %d) to %s (id %d): %s",
			   before[U_NAME], buid, after[U_NAME], auid,
			   error_message(code));
	}
	return;
    }
    if (bstate == 1) {
	code = pr_DeleteByID(buid);
	if (code && code != PRNOENT) {
	    critical_alert("incremental",
			   "Couldn't delete user %s (id %d): %s",
			   before[U_NAME], buid, error_message(code));
	}
	return;
    }
    if (astate == 1) {
	code = pr_CreateUser(after[U_NAME], &auid);
	if (code) {
	    critical_alert("incremental",
			   "Couldn't create user %s (id %d): %s",
			   after[U_NAME], auid, error_message(code));
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
    int agid, bgid;
    long code, id;
    char g1[PR_MAXNAMELEN], g2[PR_MAXNAMELEN];

    agid = bgid = 0;
    if (beforec > L_GID && atoi(before[L_ACTIVE]) && atoi(before[L_GROUP]))
	bgid = atoi(before[L_GID]);
    if (afterc > L_GID && atoi(after[L_ACTIVE]) && atoi(after[L_GROUP]))
	agid = atoi(after[L_GID]);

    if (agid == 0 && bgid == 0)			/* Not active groups */
	return;
    if (agid == bgid && !strcmp(after[L_NAME], before[L_NAME]))
	return;					/* No change */

    code=pr_Initialize(1, AFSCONF_CLIENTNAME, 0);
    if (code) {
	critical_alert("incremental", "Couldn't initialize libprot: %s",
		       error_message(code));
	return;
    }

    if (agid && bgid) {
	/* Only a modify is required */
	strcpy(g1, "system:");
	strcpy(g2, "system:");
	strcat(g1, before[L_NAME]);
	strcat(g2, after[L_NAME]);
	code = pr_ChangeEntry(g1, g2, -agid, "");
	if (code) {
	    critical_alert("incremental",
			   "Couldn't change group %s (id %d) to %s (id %d): %s",
			   before[L_NAME], -bgid, after[L_NAME], -agid,
			   error_message(code));
	}
	return;
    }
    if (bgid) {
	code = pr_DeleteByID(-bgid);
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
	code = pr_CreateGroup(g1, g2, &id);
	if (code) {
	    critical_alert("incremental",
			   "Couldn't create group %s (id %d): %s",
			   after[L_NAME], -agid, error_message(code));
	    return;
	}

	/* We need to make sure the group is properly populated */
	if (beforec < L_ACTIVE || atoi(before[L_ACTIVE]) == 0) return;

	/* XXX - To be implemented */
	critical_alert("incremental",
		       "Status change for list %s; membership may be wrong",
		       after[L_NAME]);
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

    /* The following KERBEROS code allows for the use of entities
     * user@foreign_cell.
     */
    if (afterc && !strcmp(after[LM_TYPE], "KERBEROS")) {
	p = index(after[LM_MEMBER], '@');
	if (p && !strcasecmp(p+1, cellname))
	    *p = 0;
    }
    if (beforec && !strcmp(before[LM_TYPE], "KERBEROS")) {
	p = index(before[LM_MEMBER], '@');
	if (p && !strcasecmp(p+1, cellname))
	    *p = 0;
    }

    if (afterc) {
	if (!strcmp(after[LM_TYPE], "KERBEROS")) {
	    p = index(after[LM_MEMBER], '@');
	    if (p && !strcasecmp(p+1, cellname))
		*p = 0;
	} else if (strcmp(after[LM_TYPE], "USER"))
	    return;				/* invalid type */

	code = pr_AddToGroup(after[LM_MEMBER], after[LM_LIST]);
	if (code) {
	    if (strcmp(after[LM_TYPE], "KERBEROS") || code != PRNOENT) {
		critical_alert("incremental",
			       "Couldn't add %s to %s: %s",
			       after[LM_MEMBER], after[LM_LIST],
			       error_message(code));
		return;
	    }
	}
    }

    if (beforec) {
	if (!strcmp(before[LM_TYPE], "KERBEROS")) {
	    p = index(before[LM_MEMBER], '@');
	    if (p && !strcasecmp(p+1, cellname))
		*p = 0;
	} else if (strcmp(before[LM_TYPE], "USER"))
	    return;				/* invalid type */

	code = pr_RemoveUserFromGroup(before[LM_MEMBER], before[LM_LIST]);
	if (code && code != PRNOENT) {
	    critical_alert("incremental",
			   "Couldn't remove %s from %s: %s",
			   before[LM_MEMBER], before[LM_LIST],
			   error_message(code));
	    return;
	}
    }
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
