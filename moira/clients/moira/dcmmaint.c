/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/dcmmaint.c,v 1.1 1988-08-30 17:47:46 mar Exp $
 */

#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <sms.h>
#include <menu.h>

#include "mit-copyright.h"
#include "defs.h"
#include "f_defs.h"
#include "globals.h"
#include "infodefs.h"

char *whoami;

static char buf[BUFSIZ];
char *unparse_interval(), *canonicalize_hostname(), *pgets(), *strsave();
char *atot();

#define DCM_ENABLE_FLAG 0

genable(argc, argv, hint)
int argc;
char **argv;
int *hint;
{
    *hint = atoi(argv[0]);
    return(SMS_CONT);
}

enabledcm(argc, argv)
int argc;
char **argv;
{
    char *qargv[2];
    int value, status;

    qargv[0] = "dcm_enable";
    if (status = sms_query("get_value", 1, qargv, genable, (char *)&value))
      com_err(whoami, status, " while getting value of dcm_enable");
    sprintf(buf, "%d", value);
    qargv[1] = pgets("Enable value", buf);
    if (status = sms_query("update_value", 2, qargv, Scream, NULL))
      com_err(whoami, status, " while updating value of dcm_enable");
    free(qargv[1]);
    return(DM_NORMAL);
}


int shserv(argc, argv)
int argc;
char **argv;
{
    char tmp[64], *dfgen;

    Put_message("Name       Type     Owner          Modified");
    sprintf(tmp, "%s:%s", argv[11], argv[12]);
    sprintf(buf, "%-10s %-8s %-14s by %s on %s with %s", argv[0], argv[6],
	    tmp, argv[14], argv[13], argv[15]);
    Put_message(buf);
    sprintf(buf, "  Interval %s, Target:%s, Script:%s",
	    unparse_interval(atoi(argv[1])), argv[2], argv[3]);
    Put_message(buf);
    if (atoi(argv[9]))
      sprintf(tmp, "Error %d: %s", atoi(argv[9]), argv[10]);
    else
      strcpy(tmp, "No error");
    dfgen = strsave(atot(argv[4]));
    sprintf(buf, "  Generated %s; Last Checked %s", dfgen, atot(argv[5]));
    free(dfgen);
    Put_message(buf);
    sprintf(buf, "  %s/%s/%s", atoi(argv[7]) ? "Enabled" : "Disabled",
	    atoi(argv[8]) ? "InProgress" : "Idle", tmp);
    Put_message(buf);
    Put_message("");
    return(SMS_CONT);
}


showserv(argc, argv)
int argc;
char **argv;
{
    char *qargv[1];
    int status;	
 
    qargv[0] = argv[1];
    if (status = sms_query("get_server_info", 1, qargv, shserv, NULL))
      com_err(whoami, status, " while getting server info");
    return(DM_NORMAL);
}


char *unparse_interval(i)
int i;
{
    static char uibuf[10];

    sprintf(uibuf, "%02d:%02d:00", i / 60, i % 60);
    return(uibuf);
}

addserv(argc, argv)
int argc;
char **argv;
{
    char *qargv[8], tmp[BUFSIZ];
    int status;

    qargv[0] = argv[1];
    qargv[1] = pgets("Interval", "1440");
    sprintf(tmp, "/tmp/%s.out", argv[1]);
    qargv[2] = pgets("Target", tmp);
    sprintf(tmp, "/u1/sms/bin/%s.sh", argv[1]);
    qargv[3] = pgets("Script", tmp);
    qargv[4] = pgets("Service type", "UNIQUE");
    qargv[5] = pgets("Enable", "1");
    qargv[6] = pgets("ACE type", "list");
    qargv[7] = pgets("ACE name", "dbadmin");
    if (status = sms_query("add_server_info", 8, qargv, Scream, NULL))
      com_err(whoami, status, " while updating server info");
    return(DM_NORMAL);
}


gserv(argc, argv, cargv)
int argc;
char **argv;
char **cargv;
{
    cargv[0] = argv[0];
    cargv[1] = argv[1];
    cargv[2] = argv[2];
    cargv[3] = argv[3];
    cargv[4] = argv[6];
    cargv[5] = argv[7];
    cargv[6] = argv[11];
    cargv[7] = argv[12];
    return(SMS_CONT);
}


char *pgets(prompt, def)
char *prompt;
char *def;
{
    char ibuf[BUFSIZ], pbuf[BUFSIZ];

    sprintf(pbuf, "%s [%s]: ", prompt, def);
    Prompt_input(pbuf, ibuf, BUFSIZ);
    if (!strlen(ibuf))
      return(strsave(def));
    else
      return(strsave(ibuf));
}


updateserv(argc, argv)
int argc;
char **argv;
{
    char *qargv[8];
    int status;

    qargv[0] = (char *)argv[1];
    if (status = sms_query("get_server_info", 1, qargv,gserv,(char *)qargv)) {
	com_err(whoami, status, " while getting server info");
	return(DM_NORMAL);
    }
    qargv[1] = pgets("Interval", qargv[1]);
    qargv[2] = pgets("Target", qargv[2]);
    qargv[3] = pgets("Script", qargv[3]);
    qargv[4] = pgets("Service type", qargv[4]);
    qargv[5] = pgets("Enable", qargv[5]);
    qargv[6] = pgets("ACE type", qargv[6]);
    qargv[7] = pgets("ACE name", qargv[7]);
    if (status = sms_query("update_server_info", 8, qargv, Scream, NULL))
      com_err(whoami, status, " while updating server info");
    return(DM_NORMAL);
}


int shhost(argc, argv)
int argc;
char **argv;
{
    char tmp[64], *ltt;

    sprintf(buf, "%s:%s  mod by %s on %s with %s", argv[1], argv[0],
	    argv[14], argv[13], argv[15]);
    Put_message(buf);
    if (atoi(argv[6]))
      sprintf(tmp, "Error %d: %s", atoi(argv[6]), argv[7]);
    else
      strcpy(tmp, "No error");
    sprintf(buf, "  %s/%s/%s/%s/%s",
	    atoi(argv[2]) ? "Enabled" : "Disabled",
	    atoi(argv[4]) ? "Success" : "Failure",
	    atoi(argv[5]) ? "InProgress" : "Idle",
	    atoi(argv[3]) ? "Override" : "Normal", tmp);
    Put_message(buf);
    Put_message("  Last Try             Last Success         Value1    Value2    Value3");
    ltt = strsave(atot(argv[8]));
    sprintf(buf, "  %-20s %-20s %-9d %-9d %s", ltt, atot(argv[9]),
	    atoi(argv[10]), atoi(argv[11]), argv[12]);
    free(ltt);
    Put_message(buf);
    Put_message("");
    return(SMS_CONT);
}


showhost(argc, argv)
int argc;
char **argv;
{
    char *qargv[1];
    int status;

    qargv[0] = argv[1];
    qargv[1] = canonicalize_hostname(argv[2]);
    if (status = sms_query("get_server_host_info", 2, qargv, shhost, NULL))
      com_err(whoami, status, " getting server/host info");
    return(DM_NORMAL);
}


resetsrverr(argc, argv)
int argc;
char  **argv;
{
    int status;
    if (status = sms_query("reset_server_error", 1, &argv[1], Scream, NULL))
      com_err(whoami, status, " while resetting server error");
    return(DM_NORMAL);
}


resetsrvc(argc, argv)
int argc;
char **argv;
{
    int status;
    char *qargv[5];

    qargv[0] = argv[1];
    qargv[1] = "0";
    qargv[2] = "0";
    qargv[3] = "0";
    qargv[4] = "0";
    qargv[5] = "";
    if (status = sms_query("set_server_internal_flags", 6, qargv, Scream, NULL))
      com_err(whoami, status, " while resetting server error");
    return(DM_NORMAL);
}


resethosterr(argc, argv)
int argc;
char  **argv;
{
    int status;

    argv[2] = canonicalize_hostname(argv[2]);
    if (status = sms_query("reset_server_host_error",2, &argv[1], Scream, NULL))
      com_err(whoami, status, " while resetting server/host error");
    return(DM_NORMAL);
}


resethost(argc, argv)
int argc;
char **argv;
{
    int status;
    char *qargv[9];

    qargv[0] = argv[1];
    qargv[1] = canonicalize_hostname(argv[2]);
    qargv[2] = "0";
    qargv[3] = "0";
    qargv[4] = "0";
    qargv[5] = "0";
    qargv[6] = "";
    qargv[7] = "0";
    qargv[8] = "0";
    if (status = sms_query("set_server_host_internal", 9, qargv, Scream, NULL))
      com_err(whoami, status, " while resetting server/host error");
    return(DM_NORMAL);
}


sethostor(argc, argv)
int argc;
char  **argv;
{
    int status;

    argv[2] = canonicalize_hostname(argv[2]);
    if (status = sms_query("set_server_host_override",2,&argv[1], Scream, NULL))
      com_err(whoami, status, " while setting server/host override");
    return(DM_NORMAL);
}


ghost(argc, argv, cargv)
int argc;
char **argv;
char **cargv;
{
    cargv[0] = argv[0];
    cargv[1] = argv[1];
    cargv[2] = argv[2];
    cargv[3] = argv[10];
    cargv[4] = argv[11];
    cargv[5] = argv[12];
    return(SMS_CONT);
}


updatehost(argc, argv)
int argc;
char **argv;
{
    char *qargv[6];
    int status;

    qargv[0] = (char *)argv[1];
    qargv[1] = canonicalize_hostname(argv[2]);
    if (status = sms_query("get_server_host_info", 2, qargv, ghost, 
			   (char *)qargv)) {
	com_err(whoami, status, " while getting server/host info");
	return(DM_NORMAL);
    }
    qargv[1] = canonicalize_hostname(argv[2]);
    qargv[2] = pgets("Enable", qargv[2]);
    qargv[3] = pgets("Value 1", qargv[3]);
    qargv[4] = pgets("Value 2", qargv[4]);
    qargv[5] = pgets("Value 3", qargv[5]);
    if (status = sms_query("update_server_host_info", 6, qargv, Scream, NULL))
      com_err(whoami, status, " while updating server/host info");
    return(DM_NORMAL);
}


addhost(argc, argv)
int argc;
char **argv;
{
    char *qargv[6];
    int status;

    qargv[0] = argv[1];
    qargv[1] = canonicalize_hostname(argv[2]);
    qargv[2] = pgets("Enable", "1");
    qargv[3] = pgets("Value 1", "0");
    qargv[4] = pgets("Value 2", "0");
    qargv[5] = pgets("Value 3", "");
    if (status = sms_query("add_server_host_info", 6, qargv, Scream, NULL))
      com_err(whoami, status, " while adding server/host info");
    return(DM_NORMAL);
}


delserv(argc, argv)
int argc;
char **argv;
{
    int status;

    if (status = sms_query("delete_server_info", 1, &argv[1], Scream, NULL))
      com_err(whoami, status, " while deleting server info");
    return(DM_NORMAL);
}


delhost(argc, argv)
int argc;
char **argv;
{
    int status;

    argv[2] = canonicalize_hostname(argv[2]);
    if (status = sms_query("delete_server_host_info",2, &argv[1], Scream, NULL))
      com_err(whoami, status, " while deleting server/host info");
    return(DM_NORMAL);
}


pserv(argc, argv, count)
int argc;
char **argv;
int *count;
{
    sprintf(buf, "Service %s", argv[0]);
    Put_message(buf);
    (*count)++;
    return(SMS_CONT);
}

phost(argc, argv, count)
int argc;
char **argv;
int *count;
{
    sprintf(buf, "Host %s:%s", argv[0], argv[1]);
    Put_message(buf);
    (*count)++;
    return(SMS_CONT);
}


inprogress()
{
    char *argv[6];
    int status, count = 0;

    argv[0] = argv[2] = "DONTCARE";
    argv[1] = "TRUE";
    if ((status = sms_query("qualified_get_server", 3, argv, pserv, 
			    (char *)&count)) &&
	status != SMS_NO_MATCH)
      com_err(whoami, status, " while getting servers");
    argv[0] = "*";
    argv[1] = argv[2] = argv[3] = argv[5] = "DONTCARE";
    argv[4] = "TRUE";
    if ((status = sms_query("qualified_get_server_host",6,argv,phost,
			    (char *)&count)) &&
	status != SMS_NO_MATCH)
      com_err(whoami, status, " while getting server/hosts");
    if (!count)
      strcpy(buf, "Nothing is in progress at this time");
    else
      sprintf(buf, "%d update%s in progress at this time", count,
	      count == 1 ? "" : "s");
    Put_message(buf);
    return(DM_NORMAL);
}

failed()
{
    char *argv[6];
    int status, count = 0;

    argv[0] = argv[1] = "DONTCARE";
    argv[2] = "TRUE";
    if ((status = sms_query("qualified_get_server", 3, argv, pserv, 
			    (char *)&count)) &&
	status != SMS_NO_MATCH)
      com_err(whoami, status, " while getting servers");
    argv[0] = "*";
    argv[1] = argv[2] = argv[3] = argv[4] = "DONTCARE";
    argv[5] = "TRUE";
    if ((status = sms_query("qualified_get_server_host",6,argv,phost,
			    (char *)&count)) &&
	status != SMS_NO_MATCH)
      com_err(whoami, status, " while getting server/hosts");
    if (!count)
      strcpy(buf, "Nothing has failed at this time");
    else
      sprintf(buf, "%d thing%s ha%s failed at this time", count,
	      count == 1 ? "" : "s", count == 1 ? "s" : "ve");
    Put_message(buf);
    return(DM_NORMAL);
}


dcm()
{
    int status;
    if (status = sms_do_update())
      com_err(whoami, status, " while triggering update");
    return(DM_NORMAL);
}


char *atot(itime)
char *itime;
{
    int time;
    char *ct, *ctime();

    time = atoi(itime);
    ct = ctime(&time);
    ct[24] = 0;
    return(&ct[4]);
}
