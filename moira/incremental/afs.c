/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/incremental/afs.c,v 1.9 1989-12-14 13:58:40 mar Exp $
 *
 * Do AFS incremental updates
 *
 * Copyright (C) 1989 by the Massachusetts Institute of Technology
 * for copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <sms.h>
#include <sms_app.h>
#include <sys/file.h>

#define file_exists(file) (access((file), F_OK) == 0)

char prs[64], fs[64];

char *whoami;

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
    sprintf(prs, "%s/prs", BIN_DIR);
    sprintf(fs, "%s/fs", BIN_DIR);

    if (!strcmp(table, "users")) {
	do_user(before, beforec, after, afterc);
    } else if (!strcmp(table, "list")) {
	do_list(before, beforec, after, afterc);
    } else if (!strcmp(table, "members")) {
	do_member(before, beforec, after, afterc);
    } else if (!strcmp(table, "filesys")) {
	do_filesys(before, beforec, after, afterc);
    } else if (!strcmp(table, "nfsquota")) {
	do_quota(before, beforec, after, afterc);
    }
    exit(0);
}


do_cmd(cmd)
char *cmd;
{
    int success = 0, tries = 0;

    while (success == 0 && tries < 3) {
	com_err(whoami, 0, "Executing command: %s", cmd);
	if (system(cmd) == 0)
	  success++;
	if (!success) {
	    tries++;
	    sleep(5 * 60);
	}
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
    int bstate, astate;
    char cmd[512];

    cmd[0] = bstate = astate = 0;
    if (afterc > U_STATE)
      astate = atoi(after[U_STATE]);
    if (beforec > U_STATE)
      bstate = atoi(before[U_STATE]);
    if (astate == 2) astate = 1;
    if (bstate == 2) bstate = 1;

    if (astate != 1 && bstate != 1)
      return;
    if (astate == 1 && bstate != 1) {
	sprintf(cmd, "%s newuser -name %s -id %s",
		prs, after[U_NAME], after[U_UID]);
	do_cmd(cmd);
	return;
    } else if (astate != 1 && bstate == 1) {
	sprintf(cmd, "%s delete %s", prs, before[U_NAME]);
	do_cmd(cmd);
	return;
    }

    if (beforec > U_UID && afterc > U_UID &&
	strcmp(before[U_UID], after[U_UID])) {
	/* change UID, & possibly user name here */
	exit(1);
    }

    if (beforec > U_NAME && afterc > U_NAME &&
	strcmp(before[U_NAME], after[U_NAME])) {
	sprintf(cmd, "%s chname -oldname %s -newname %s",
		prs, before[U_NAME], after[U_NAME]);
	do_cmd(cmd);
    }
}



do_list(before, beforec, after, afterc)
char **before;
int beforec;
char **after;
int afterc;
{
    char cmd[512];
    int agid, bgid;

    cmd[0] = agid = bgid = 0;
    if (beforec > L_GID && atoi(before[L_ACTIVE]) && atoi(before[L_GROUP]))
      bgid = atoi(before[L_GID]);
    if (afterc > L_GID && atoi(after[L_ACTIVE]) && atoi(after[L_GROUP]))
      agid = atoi(after[L_GID]);

    if (bgid == 0 && agid != 0) {
	sprintf(cmd,
		"%s create -name system:%s -id %s -owner system:administrators",
		prs, after[L_NAME], after[L_GID]);
	do_cmd(cmd);
	return;
    }
    if (agid == 0 && bgid != 0) {
	sprintf(cmd, "%s delete -name system:%s", prs, before[L_NAME]);
	do_cmd(cmd);
	return;
    }
    if (agid == 0 && bgid == 0)
      return;
    if (strcmp(before[L_NAME], after[L_NAME])) {
	sprintf(cmd,
		"%s chname -oldname system:%s -newname system:%s",
		prs, before[L_NAME], after[L_NAME]);
	do_cmd(cmd);
	return;
    }
}


do_member(before, beforec, after, afterc)
char **before;
int beforec;
char **after;
int afterc;
{
    char cmd[512];

    if (beforec == 0 && !strcmp(after[LM_TYPE], "USER")) {
	sprintf(cmd, "%s add -user %s -group system:%s",
		prs, after[LM_MEMBER], after[LM_LIST]);
	do_cmd(cmd);
	return;
    }
    if (afterc == 0 && !strcmp(before[LM_TYPE], "USER")) {
	sprintf(cmd, "%s remove -user %s -group system:%s",
		prs, before[LM_MEMBER], before[LM_LIST]);
	do_cmd(cmd);
	return;
    }
}


do_filesys(before, beforec, after, afterc)
char **before;
int beforec;
char **after;
int afterc;
{
    if (afterc < FS_CREATE)
      return;
    if (!strcmp("AFS", after[FS_TYPE]) &&
	!strncmp("/afs/", after[FS_PACK], 5) &&
	atoi(after[FS_CREATE]) &&
	!file_exists(after[FS_PACK])) {
	critical_alert("incremental", "unable to create locker %s",
		       after[FS_PACK]);
    }
}


do_quota(before, beforec, after, afterc)
char **before;
int beforec;
char **after;
int afterc;
{
    char cmd[512];

    if (!(afterc >= Q_DIRECTORY && !strncmp("/afs", after[Q_DIRECTORY], 4)) &&
	!(beforec >= Q_DIRECTORY && !strncmp("/afs", before[Q_DIRECTORY], 4)))
      return;
    if (afterc >= Q_LOGIN && strcmp("\\[nobody\\]", after[Q_LOGIN]))
      return;
    if (afterc != 0) {
	sprintf(cmd, "%s setquota -dir %s -quota %s",
		fs, after[Q_DIRECTORY], after[Q_QUOTA]);
	do_cmd(cmd);
	return;
    }
}
