/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/dcmmaint.c,v 1.4 1988-10-05 11:41:21 mar Exp $
 *
 * Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <sms.h>
#include <sms_app.h>
#include <menu.h>

#include "mit-copyright.h"
#include "defs.h"
#include "f_defs.h"
#include "globals.h"
#include "infodefs.h"

extern char *whoami;
static char buf[BUFSIZ];
char *unparse_interval(), *canonicalize_hostname();
char *atot();

#define DCM_ENABLE_FLAG 0

genable(argc, argv, info)
int argc;
char **argv;
char **info;
{
    info[1] = strsave(argv[0]);
    return(SMS_CONT);
}

EnableDcm(argc, argv)
int argc;
char **argv;
{
    int status;
    char *info[3];

    info[0] = "dcm_enable";
    if (status = do_sms_query("get_value", 1, info, genable, info))
      com_err(whoami, status, " while getting value of dcm_enable");
    GetValueFromUser("Enable value", &info[1]);
    if (status = do_sms_query("update_value", 2, info, Scream, NULL))
      com_err(whoami, status, " while updating value of dcm_enable");
    FreeAndClear(&info[0], FALSE);
    FreeAndClear(&info[1], TRUE);
    return(DM_NORMAL);
}


int shserv(argc, argv)
int argc;
char **argv;
{
    char tmp[64], *dfgen;

    Put_message("Name       Type     Owner       Modified");
    sprintf(tmp, "%s:%s", argv[SVC_ACE_TYPE], argv[SVC_ACE_NAME]);
    sprintf(buf, "%-10s %-8s %-11s by %s on %s with %s", argv[SVC_SERVICE],
	    argv[SVC_TYPE], tmp, argv[SVC_MODBY], argv[SVC_MODTIME],
	    argv[SVC_MODWITH]);
    Put_message(buf);
    sprintf(buf, "  Interval %s, Target:%s, Script:%s",
	    unparse_interval(atoi(argv[SVC_INTERVAL])), argv[SVC_TARGET],
	    argv[SVC_SCRIPT]);
    Put_message(buf);
    if (atoi(argv[SVC_HARDERROR]))
      sprintf(tmp, "Error %d: %s", atoi(argv[SVC_HARDERROR]), argv[SVC_ERRMSG]);
    else
      strcpy(tmp, "No error");
    dfgen = strsave(atot(argv[SVC_DFGEN]));
    sprintf(buf, "  Generated %s; Last Checked %s", dfgen,
	    atot(argv[SVC_DFCHECK]));
    free(dfgen);
    Put_message(buf);
    sprintf(buf, "  %s/%s/%s", atoi(argv[SVC_ENABLE]) ? "Enabled" : "Disabled",
	    atoi(argv[SVC_INPROGRESS]) ? "InProgress" : "Idle", tmp);
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
    if (status = do_sms_query("get_server_info", 1, qargv, shserv, NULL))
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
    int status;
    char *info[8];

    initserv(argv[1], info);
    askserv(info);
    if (status = do_sms_query("add_server_info", 8, info, Scream, NULL))
      com_err(whoami, status, " while updating server info");
    FreeInfo(info);
    return(DM_NORMAL);
}

askserv(info)
char **info;
{
    GetValueFromUser("Interval", &info[SC_INTERVAL]);
    GetValueFromUser("Target", &info[SC_TARGET]);
    GetValueFromUser("Script", &info[SC_SCRIPT]);
    GetTypeFromUser("Service type", "service", &info[SC_TYPE]);
    GetValueFromUser("Enable", &info[SC_ENABLE]);
    GetTypeFromUser("ACE type", "ace_type", &info[SC_ACE_TYPE]);
    if (strcasecmp("none", info[SC_ACE_TYPE]))
      GetValueFromUser("Ace name", &info[SC_ACE_NAME]);
}


gserv(argc, argv, cargv)
int argc;
char **argv;
char **cargv;
{
    cargv[SC_SERVICE] = strsave(argv[SVC_SERVICE]);
    cargv[SC_INTERVAL] = strsave(argv[SVC_INTERVAL]);
    cargv[SC_TARGET] = strsave(argv[SVC_TARGET]);
    cargv[SC_SCRIPT] = strsave(argv[SVC_SCRIPT]);
    cargv[SC_TYPE] = strsave(argv[SVC_TYPE]);
    cargv[SC_ENABLE] = strsave(argv[SVC_ENABLE]);
    cargv[SC_ACE_TYPE] = strsave(argv[SVC_ACE_TYPE]);
    cargv[SC_ACE_NAME] = strsave(argv[SVC_ACE_NAME]);
    cargv[SC_END] = NULL;
    return(SMS_CONT);
}

initserv(name, argv)
char *name;
char **argv;
{
    char tmp[BUFSIZ];

    argv[SC_SERVICE] = strsave(name);
    argv[SC_INTERVAL] = strsave("1440");
    sprintf(tmp, "/tmp/%s.out", name);
    argv[SC_TARGET] = strsave(tmp);
    sprintf(tmp, "/u1/sms/bin/%s.sh", name);
    argv[SC_SCRIPT] = strsave(tmp);
    argv[SC_TYPE] = strsave("UNIQUE");
    argv[SC_ENABLE] = strsave("1");
    argv[SC_ACE_TYPE] = strsave("LIST");
    argv[SC_ACE_NAME] = strsave("dbadmin");
    argv[SC_END] = NULL;
}


updateserv(argc, argv)
int argc;
char **argv;
{
    char *qargv[9];
    int status;

    qargv[0] = (char *)argv[1];
    if (status = do_sms_query("get_server_info", 1,
			      qargv, gserv, (char *)qargv)) {
	com_err(whoami, status, " while getting server info");
	return(DM_NORMAL);
    }
    askserv(qargv);
    if (status = do_sms_query("update_server_info", 8, qargv, Scream, NULL))
      com_err(whoami, status, " while updating server info");
    return(DM_NORMAL);
}


int shhost(argc, argv)
int argc;
char **argv;
{
    char tmp[64], *ltt;

    sprintf(buf, "%s:%s  mod by %s on %s with %s", argv[SH_MACHINE],
	    argv[SH_SERVICE], argv[SH_MODBY], argv[SH_MODTIME],
	    argv[SH_MODWITH]);
    Put_message(buf);
    if (atoi(argv[SH_HOSTERROR]))
      sprintf(tmp, "Error %d: %s", atoi(argv[SH_HOSTERROR]), argv[SH_ERRMSG]);
    else
      strcpy(tmp, "No error");
    sprintf(buf, "  %s/%s/%s/%s/%s",
	    atoi(argv[SH_ENABLE]) ? "Enabled" : "Disabled",
	    atoi(argv[SH_SUCCESS]) ? "Success" : "Failure",
	    atoi(argv[SH_INPROGRESS]) ? "InProgress" : "Idle",
	    atoi(argv[SH_OVERRIDE]) ? "Override" : "Normal", tmp);
    Put_message(buf);
    Put_message("  Last Try             Last Success         Value1    Value2    Value3");
    ltt = strsave(atot(argv[SH_LASTTRY]));
    sprintf(buf, "  %-20s %-20s %-9d %-9d %s", ltt, atot(argv[SH_LASTSUCCESS]),
	    atoi(argv[SH_VALUE1]), atoi(argv[SH_VALUE2]), argv[SH_VALUE3]);
    free(ltt);
    Put_message(buf);
    Put_message("");
    return(SMS_CONT);
}


showhost(argc, argv)
int argc;
char **argv;
{
    char *qargv[2];
    int status;

    qargv[SHI_SERVICE] = argv[1];
    qargv[SHI_MACHINE] = canonicalize_hostname(argv[2]);
    if (status = do_sms_query("get_server_host_info", 2, qargv, shhost, NULL))
      com_err(whoami, status, " getting server/host info");
    return(DM_NORMAL);
}


resetsrverr(argc, argv)
int argc;
char  **argv;
{
    int status;
    if (status = do_sms_query("reset_server_error", 1, &argv[1], Scream, NULL))
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
    if (status = do_sms_query("set_server_internal_flags", 6, qargv,
			      Scream, NULL))
      com_err(whoami, status, " while resetting server error");
    return(DM_NORMAL);
}


resethosterr(argc, argv)
int argc;
char  **argv;
{
    int status;

    argv[2] = canonicalize_hostname(argv[2]);
    if (status = do_sms_query("reset_server_host_error", 2, &argv[1],
			      Scream, NULL))
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
    if (status = do_sms_query("set_server_host_internal", 9, qargv,
			      Scream, NULL))
      com_err(whoami, status, " while resetting server/host error");
    return(DM_NORMAL);
}


sethostor(argc, argv)
int argc;
char  **argv;
{
    int status;

    argv[2] = canonicalize_hostname(argv[2]);
    if (status = do_sms_query("set_server_host_override", 2, &argv[1],
			      Scream, NULL))
      com_err(whoami, status, " while setting server/host override");
    return(DM_NORMAL);
}


ghost(argc, argv, cargv)
int argc;
char **argv;
char **cargv;
{
    cargv[SHI_SERVICE] = strsave(argv[SH_SERVICE]);
    cargv[SHI_MACHINE] = strsave(argv[SH_MACHINE]);
    cargv[SHI_ENABLE] = strsave(argv[SH_ENABLE]);
    cargv[SHI_VALUE1] = strsave(argv[SH_VALUE1]);
    cargv[SHI_VALUE2] = strsave(argv[SH_VALUE2]);
    cargv[SHI_VALUE3] = strsave(argv[SH_VALUE3]);
    cargv[SHI_END] = NULL;
    return(SMS_CONT);
}


inithost(info)
char **info;
{
    info[SHI_MACHINE] = strsave(canonicalize_hostname(info[SH_MACHINE]));
    info[SHI_ENABLE] = strsave("1");
    info[SHI_VALUE1] = strsave("0");
    info[SHI_VALUE2] = strsave("0");
    info[SHI_VALUE3] = strsave("");
    info[SHI_END] = NULL;
}


askhost(info)
char **info;
{
    GetValueFromUser("Enable", &info[SHI_ENABLE]);
    GetValueFromUser("Value 1", &info[SHI_VALUE1]);
    GetValueFromUser("Value 2", &info[SHI_VALUE2]);
    GetValueFromUser("Value 3", &info[SHI_VALUE3]);
}


updatehost(argc, argv)
int argc;
char **argv;
{
    char *info[SHI_END+1];
    int status;

    info[SHI_SERVICE] = strsave(argv[1]);
    info[SHI_MACHINE] = strsave(canonicalize_hostname(argv[2]));
    if (status = do_sms_query("get_server_host_info", 2, info, ghost, 
			      (char *)info)) {
	com_err(whoami, status, " while getting server/host info");
	return(DM_NORMAL);
    }
    askhost(info);
    if (status = do_sms_query("update_server_host_info", SHI_END, info,
			      Scream, NULL))
      com_err(whoami, status, " while updating server/host info");
    FreeInfo(info);
    return(DM_NORMAL);
}


addhost(argc, argv)
int argc;
char **argv;
{
    char *info[SHI_END+1];
    int status;

    info[SHI_SERVICE] = strsave(argv[1]);
    info[SHI_MACHINE] = strsave(canonicalize_hostname(argv[2]));
    inithost(info);
    askhost(info);
    if (status = do_sms_query("add_server_host_info", 6, info, Scream, NULL))
      com_err(whoami, status, " while adding server/host info");
    FreeInfo(info);
    return(DM_NORMAL);
}


delserv(argc, argv)
int argc;
char **argv;
{
    int status;

    if (status = do_sms_query("delete_server_info", 1, &argv[1], Scream, NULL))
      com_err(whoami, status, " while deleting server info");
    return(DM_NORMAL);
}


delhost(argc, argv)
int argc;
char **argv;
{
    int status;

    argv[2] = canonicalize_hostname(argv[2]);
    if (status = do_sms_query("delete_server_host_info", 2, &argv[1],
			      Scream, NULL))
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


InProgress()
{
    char *argv[6];
    int status, count = 0;

    argv[0] = argv[2] = "DONTCARE";
    argv[1] = "TRUE";
    if ((status = do_sms_query("qualified_get_server", 3, argv, pserv, 
			       (char *)&count)) &&
	status != SMS_NO_MATCH)
      com_err(whoami, status, " while getting servers");
    argv[0] = "*";
    argv[1] = argv[2] = argv[3] = argv[5] = "DONTCARE";
    argv[4] = "TRUE";
    if ((status = do_sms_query("qualified_get_server_host",6,argv,phost,
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

DcmFailed()
{
    char *argv[6];
    int status, count = 0;

    argv[0] = argv[1] = "DONTCARE";
    argv[2] = "TRUE";
    if ((status = do_sms_query("qualified_get_server", 3, argv, pserv, 
			       (char *)&count)) &&
	status != SMS_NO_MATCH)
      com_err(whoami, status, " while getting servers");
    argv[0] = "*";
    argv[1] = argv[2] = argv[3] = argv[4] = "DONTCARE";
    argv[5] = "TRUE";
    if ((status = do_sms_query("qualified_get_server_host",6,argv,phost,
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


Dcm()
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
