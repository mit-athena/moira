/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/incremental/afs.c,v 1.1 1989-08-18 17:31:26 mar Exp $
 *
 * Do AFS incremental updates
 *
 * Copyright (C) 1989 by the Massachusetts Institute of Technology
 * for copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <sms.h>
#include <sms_app.h>
#include <sys/param.h>
#include <krb.h>
#include <krb_et.h>
#include <afs/auth.h>
#include <pwd.h>

#define LOCALCELL "sms_test.mit.edu"

char *whoami;

main(argc, argv)
char **argv;
int argc;
{
    int beforec, afterc;
    char *table, **before, **after;

    table = argv[1];
    beforec = atoi(argv[2]);
    before = &argv[4];
    afterc = atoi(argv[3]);
    after = &argv[4 + beforec];
    whoami = argv[0];

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
    } else if (!strcmp(table, "nfsquota")) {
	do_quota(before, beforec, after, afterc);
    }
    unlog();
    exit(0);
}


do_cmd(cmd)
char *cmd;
{
    char realm[REALM_SZ + 1];
    static int inited = 0;
    int success = 0, tries = 0;
    CREDENTIALS *c, *get_ticket();
    struct passwd *pw;
    char buf[128];

    while (success == 0 && tries < 3) {
	if (!inited) {
	    if (krb_get_lrealm(realm) != KSUCCESS)
	      (void) strcpy(realm, KRB_REALM);
	    sprintf(buf, "/tmp/tkt_%d_afsinc", getpid());
	    krb_set_tkt_string(buf);
	    
	    if (((pw = getpwnam("smsdba")) == NULL) ||
		((c = get_ticket("sms", "", realm, LOCALCELL)) == NULL) ||
		(setpag() < 0) ||
		(setreuid(pw->pw_uid, pw->pw_uid) < 0) ||
		aklog(c, LOCALCELL)) {
		com_err(whoami, 0, "failed to authenticate");
	    } else
	      inited++;
	}

	if (inited) {
	    com_err(whoami, 0, "Executing command: %s", cmd);
	    if (system(cmd) == 0)
	      success++;
	}
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

    if (astate != 1 && bstate != 1)
      return;
    if (astate == 1 && bstate != 1) {
	sprintf(cmd, "prs newuser -name %s -id %s -cell %s",
		after[U_NAME], after[U_UID], LOCALCELL);
	do_cmd(cmd);
	return;
    } else if (astate != 1 && bstate == 1) {
	sprintf(cmd, "prs delete %s -cell %s", before[U_NAME], LOCALCELL);
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
	sprintf(cmd, "prs chname -oldname %s -newname %s -cell %s",
		before[U_NAME], after[U_NAME], LOCALCELL);
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
	sprintf(cmd, "prs create -name system:%s -id %s -cell %s",
		after[L_NAME], after[L_GID], LOCALCELL);
	do_cmd(cmd);
	return;
    }
    if (agid == 0 && bgid != 0) {
	sprintf(cmd, "prs delete -name system:%s -cell %s",
		before[L_NAME], LOCALCELL);
	do_cmd(cmd);
	return;
    }
    if (agid == 0 && bgid == 0)
      return;
    if (strcmp(before[L_NAME], after[L_NAME])) {
	sprintf(cmd,
		"prs chname -oldname system:%s -newname system:%s -cell %s",
		before[L_NAME], after[L_NAME], LOCALCELL);
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
	sprintf(cmd, "prs add -user %s -group system:%s -cell %s",
		after[LM_MEMBER], after[LM_LIST], LOCALCELL);
	do_cmd(cmd);
	return;
    }
    if (afterc == 0 && !strcmp(before[LM_TYPE], "USER")) {
	sprintf(cmd, "prs delete -user %s -group system:%s -cell %s",
		before[LM_MEMBER], before[LM_LIST], LOCALCELL);
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
    if (afterc != 0) {
	sprintf(cmd, "fs setquota -dir %s -quota %s",
		after[Q_DIRECTORY], after[Q_QUOTA]);
	do_cmd(cmd);
	return;
    }
}


CREDENTIALS *get_ticket(name, instance, realm, cell)
char *name;
char *instance;
char *realm;
char *cell;
{
    static CREDENTIALS c;
    int status;

    status = krb_get_svc_in_tkt(name, instance, realm,
				"krbtgt", realm, 1, KEYFILE);
    if (status != 0) {
	com_err(whoami, status+ERROR_TABLE_BASE_krb, "getting initial ticket from srvtab");
	return(NULL);
    }
    status = krb_get_cred("afs", cell, realm, &c);
    if (status != 0) {
	status = get_ad_tkt("afs", cell, realm, 255);
	if (status == 0)
	  status = krb_get_cred("afs", cell, realm, &c);
    }
    if (status != 0) {
	com_err(whoami, status+ERROR_TABLE_BASE_krb, "getting service ticket");
	return(NULL);
    }
    return(&c);
}


aklog(c, cell)
CREDENTIALS *c;
char *cell;
{
	struct ktc_principal aserver;
	struct ktc_token atoken;
	
	atoken.kvno = c->kvno;
	strcpy(aserver.name, "afs");
	strcpy(aserver.instance, "");
	strcpy(aserver.cell, cell);

	atoken.startTime = c->issue_date;
	atoken.endTime = c->issue_date + (c->lifetime * 5 * 60);
	bcopy (c->session, &atoken.sessionKey, 8);
	atoken.ticketLen = c->ticket_st.length;
	bcopy (c->ticket_st.dat, atoken.ticket, atoken.ticketLen);
	
	return(ktc_SetToken(&aserver, &atoken, NULL));
}


unlog()
{
    ktc_ForgetToken("afs");
}
