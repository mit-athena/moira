/* $Id: dcmmaint.c 3956 2010-01-05 20:56:56Z zacheiss $
 *
 * DCM-related functions for Moira client
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include <moira_site.h>
#include "defs.h"
#include "f_defs.h"
#include "globals.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

RCSID("$HeadURL: svn+ssh://svn.mit.edu/moira/trunk/moira/clients/moira/dcmmaint.c $ $Id: dcmmaint.c 3956 2010-01-05 20:56:56Z zacheiss $");

extern char *whoami;
static char buf[BUFSIZ];

int genable(int argc, char **argv, void *info);
int shserv(int argc, char **argv, void *hint);
char *unparse_interval(int i);
char **askserv(char **info);
int gserv(int argc, char **argv, void *hint);
void initserv(char *name, char **argv);
int shhost(int argc, char **argv, void *hint);
int ghost(int argc, char **argv, void *hint);
void inithost(char **info);
char **askhost(char **info);
int pserv(int argc, char **argv, void *count);
int phost(int argc, char **argv, void *count);

#define DCM_ENABLE_FLAG 0

int genable(int argc, char **argv, void *info)
{
  ((char **)info)[1] = strdup(argv[0]);
  return MR_CONT;
}

int EnableDcm(int argc, char **argv)
{
  int status;
  char *info[3];

  info[0] = "dcm_enable";
  if ((status = do_mr_query("get_value", 1, info, genable, info)))
    com_err(whoami, status, " while getting value of dcm_enable");
  if (GetValueFromUser("Enable value", &info[1]) == SUB_ERROR)
    return DM_NORMAL;
  if ((status = do_mr_query("update_value", 2, info, NULL, NULL)))
    com_err(whoami, status, " while updating value of dcm_enable");
  FreeAndClear(&info[0], FALSE);
  FreeAndClear(&info[1], TRUE);
  return DM_NORMAL;
}


int shserv(int argc, char **argv, void *hint)
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
  dfgen = strdup(atot(argv[SVC_DFGEN]));
  sprintf(buf, "  Generated %s; Last Checked %s", dfgen,
	  atot(argv[SVC_DFCHECK]));
  free(dfgen);
  Put_message(buf);
  sprintf(buf, "  %s/%s/%s", atoi(argv[SVC_ENABLE]) ? "Enabled" : "Disabled",
	  atoi(argv[SVC_INPROGRESS]) ? "InProgress" : "Idle", tmp);
  Put_message(buf);
  Put_message("");
  return MR_CONT;
}


int showserv(int argc, char **argv)
{
  char *qargv[1];
  int status;

  qargv[0] = argv[1];
  if ((status = do_mr_query("get_server_info", 1, qargv, shserv, NULL)))
    com_err(whoami, status, " while getting server info");
  return DM_NORMAL;
}


char *unparse_interval(int i)
{
  static char uibuf[10];

  sprintf(uibuf, "%02d:%02d:00", i / 60, i % 60);
  return uibuf;
}

int addserv(int argc, char **argv)
{
  int status;
  char *info[SC_END + 1];

  initserv(argv[1], info);
  if (!askserv(info))
    {
      Put_message("Aborted.");
      return DM_NORMAL;
    }
  if ((status = do_mr_query("add_server_info", SC_END, info, NULL, NULL)))
    com_err(whoami, status, " while updating server info");
  FreeInfo(info);
  return DM_NORMAL;
}

char **askserv(char **info)
{
  if (GetValueFromUser("Interval", &info[SC_INTERVAL]) == SUB_ERROR)
    return NULL;
  if (GetValueFromUser("Target", &info[SC_TARGET]) == SUB_ERROR)
    return NULL;
  if (GetValueFromUser("Script", &info[SC_SCRIPT]) == SUB_ERROR)
    return NULL;
  if (GetTypeFromUser("Service type", "service", &info[SC_TYPE]) == SUB_ERROR)
    return NULL;
  if (GetValueFromUser("Enable", &info[SC_ENABLE]) == SUB_ERROR)
    return NULL;
  if (GetTypeFromUser("ACE type", "ace_type", &info[SC_ACE_TYPE]) == SUB_ERROR)
    return NULL;
  if (strcasecmp("none", info[SC_ACE_TYPE]))
    {
      if (GetValueFromUser("Ace name", &info[SC_ACE_NAME]) == SUB_ERROR)
	return NULL;
    }
  return info;
}


int gserv(int argc, char **argv, void *hint)
{
  char **cargv = hint;

  cargv[SC_SERVICE] = strdup(argv[SVC_SERVICE]);
  cargv[SC_INTERVAL] = strdup(argv[SVC_INTERVAL]);
  cargv[SC_TARGET] = strdup(argv[SVC_TARGET]);
  cargv[SC_SCRIPT] = strdup(argv[SVC_SCRIPT]);
  cargv[SC_TYPE] = strdup(argv[SVC_TYPE]);
  cargv[SC_ENABLE] = strdup(argv[SVC_ENABLE]);
  cargv[SC_ACE_TYPE] = strdup(argv[SVC_ACE_TYPE]);
  cargv[SC_ACE_NAME] = strdup(argv[SVC_ACE_NAME]);
  cargv[SC_END] = NULL;
  return MR_CONT;
}

void initserv(char *name, char **argv)
{
  char tmp[BUFSIZ];

  argv[SC_SERVICE] = strdup(name);
  argv[SC_INTERVAL] = strdup("1440");
  sprintf(tmp, "/tmp/%s.out", name);
  argv[SC_TARGET] = strdup(tmp);
  sprintf(tmp, "%s/%s.sh", BIN_DIR, name);
  argv[SC_SCRIPT] = strdup(tmp);
  argv[SC_TYPE] = strdup("UNIQUE");
  argv[SC_ENABLE] = strdup("1");
  argv[SC_ACE_TYPE] = strdup("LIST");
  argv[SC_ACE_NAME] = strdup("dbadmin");
  argv[SC_END] = NULL;
}


int updateserv(int argc, char **argv)
{
  char *qargv[SC_END + 1];
  int status;

  qargv[0] = (char *)argv[1];
  if ((status = do_mr_query("get_server_info", 1, qargv, gserv, qargv)))
    {
      com_err(whoami, status, " while getting server info");
      return DM_NORMAL;
    }
  if (!askserv(qargv))
    {
      Put_message("Aborted.");
      return DM_NORMAL;
    }
  if ((status = do_mr_query("update_server_info", SC_END, qargv, NULL, NULL)))
    com_err(whoami, status, " while updating server info");
  return DM_NORMAL;
}


int shhost(int argc, char **argv, void *hint)
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
  ltt = strdup(atot(argv[SH_LASTTRY]));
  sprintf(buf, "  %-20s %-20s %-9d %-9d %s", ltt, atot(argv[SH_LASTSUCCESS]),
	  atoi(argv[SH_VALUE1]), atoi(argv[SH_VALUE2]), argv[SH_VALUE3]);
  free(ltt);
  Put_message(buf);
  Put_message("");
  return MR_CONT;
}


int showhost(int argc, char **argv)
{
  char *qargv[2];
  int status;

  qargv[SHI_SERVICE] = argv[1];
  qargv[SHI_MACHINE] = canonicalize_hostname(strdup(argv[2]));
  if ((status = do_mr_query("get_server_host_info", 2, qargv, shhost, NULL)))
    com_err(whoami, status, " getting server/host info");
  return DM_NORMAL;
}


int resetsrverr(int argc, char **argv)
{
  int status;
  if ((status = do_mr_query("reset_server_error", 1, &argv[1], NULL, NULL)))
    com_err(whoami, status, " while resetting server error");
  return DM_NORMAL;
}


int resetsrvc(int argc, char **argv)
{
  int status;
  char *qargv[6], buf[BUFSIZ];

  sprintf(buf, "Reset state for service %s (Y/N)", argv[1]);
  if (!Confirm(buf))
    return DM_NORMAL;
  qargv[0] = argv[1];
  qargv[1] = "0";
  qargv[2] = "0";
  qargv[3] = "0";
  qargv[4] = "0";
  qargv[5] = "";
  if ((status = do_mr_query("set_server_internal_flags", 6, qargv,
			    NULL, NULL)))
    com_err(whoami, status, " while resetting server error");
  return DM_NORMAL;
}


int resethosterr(int argc, char **argv)
{
  int status;

  argv[2] = canonicalize_hostname(strdup(argv[2]));
  if ((status = do_mr_query("reset_server_host_error", 2, &argv[1],
			    NULL, NULL)))
    com_err(whoami, status, " while resetting server/host error");
  return DM_NORMAL;
}


int resethost(int argc, char **argv)
{
  int status;
  char *qargv[9], buf[BUFSIZ];

  sprintf(buf, "Reset state for service %s on host %s (Y/N)", argv[1],
	  canonicalize_hostname(strdup(argv[2])));
  if (!Confirm(buf))
    return DM_NORMAL;
  qargv[0] = argv[1];
  qargv[1] = canonicalize_hostname(strdup(argv[2]));
  qargv[2] = "0";
  qargv[3] = "0";
  qargv[4] = "0";
  qargv[5] = "0";
  qargv[6] = "";
  qargv[7] = "0";
  qargv[8] = "0";
  if ((status = do_mr_query("set_server_host_internal", 9, qargv,
			    NULL, NULL)))
    com_err(whoami, status, " while resetting server/host error");
  return DM_NORMAL;
}


int sethostor(int argc, char **argv)
{
  int status;

  argv[2] = canonicalize_hostname(strdup(argv[2]));
  if ((status = do_mr_query("set_server_host_override", 2, &argv[1],
			    NULL, NULL)))
    com_err(whoami, status, " while setting server/host override");
  return DM_NORMAL;
}


int ghost(int argc, char **argv, void *hint)
{
  char **cargv = hint;

  cargv[SHI_SERVICE] = strdup(argv[SH_SERVICE]);
  cargv[SHI_MACHINE] = strdup(argv[SH_MACHINE]);
  cargv[SHI_ENABLE] = strdup(argv[SH_ENABLE]);
  cargv[SHI_VALUE1] = strdup(argv[SH_VALUE1]);
  cargv[SHI_VALUE2] = strdup(argv[SH_VALUE2]);
  cargv[SHI_VALUE3] = strdup(argv[SH_VALUE3]);
  cargv[SHI_END] = NULL;
  return MR_CONT;
}


void inithost(char **info)
{
  info[SHI_MACHINE] = canonicalize_hostname(strdup(info[SH_MACHINE]));
  info[SHI_ENABLE] = strdup("1");
  info[SHI_VALUE1] = strdup("0");
  info[SHI_VALUE2] = strdup("0");
  info[SHI_VALUE3] = strdup("");
  info[SHI_END] = NULL;
}


char **askhost(char **info)
{
  if (GetValueFromUser("Enable", &info[SHI_ENABLE]) == SUB_ERROR)
    return NULL;
  if (GetValueFromUser("Value 1", &info[SHI_VALUE1]) == SUB_ERROR)
    return NULL;
  if (GetValueFromUser("Value 2", &info[SHI_VALUE2]) == SUB_ERROR)
    return NULL;
  if (GetValueFromUser("Value 3", &info[SHI_VALUE3]) == SUB_ERROR)
    return NULL;
  return info;
}


int updatehost(int argc, char **argv)
{
  char *info[SHI_END + 1];
  int status;

  info[SHI_SERVICE] = strdup(argv[1]);
  info[SHI_MACHINE] = canonicalize_hostname(strdup(argv[2]));
  if ((status = do_mr_query("get_server_host_info", 2, info, ghost, info)))
    {
      com_err(whoami, status, " while getting server/host info");
      return DM_NORMAL;
    }
  if (!askhost(info))
    {
      Put_message("Aborted.");
      return DM_NORMAL;
    }
  if ((status = do_mr_query("update_server_host_info", SHI_END, info,
			    NULL, NULL)))
    com_err(whoami, status, " while updating server/host info");
  FreeInfo(info);
  return DM_NORMAL;
}


int addhost(int argc, char **argv)
{
  char *info[SHI_END + 1];
  int status;

  info[SHI_SERVICE] = strdup(argv[1]);
  info[SHI_MACHINE] = canonicalize_hostname(strdup(argv[2]));
  inithost(info);
  if (!askhost(info))
    {
      Put_message("Aborted.");
      return DM_NORMAL;
    }
  if ((status = do_mr_query("add_server_host_info", SHI_END, info,
			    NULL, NULL)))
    com_err(whoami, status, " while adding server/host info");
  FreeInfo(info);
  return DM_NORMAL;
}


int delserv(int argc, char **argv)
{
  int status;

  if ((status = do_mr_query("delete_server_info", 1, &argv[1], NULL, NULL)))
    com_err(whoami, status, " while deleting server info");
  return DM_NORMAL;
}


int delhost(int argc, char **argv)
{
  int status;

  argv[2] = canonicalize_hostname(strdup(argv[2]));
  if ((status = do_mr_query("delete_server_host_info", 2, &argv[1],
			    NULL, NULL)))
    com_err(whoami, status, " while deleting server/host info");
  return DM_NORMAL;
}


int pserv(int argc, char **argv, void *count)
{
  sprintf(buf, "Service %s", argv[0]);
  Put_message(buf);
  (*(int *)count)++;
  return MR_CONT;
}

int phost(int argc, char **argv, void *count)
{
  sprintf(buf, "Host %s:%s", argv[0], argv[1]);
  Put_message(buf);
  (*(int *)count)++;
  return MR_CONT;
}


int InProgress(int argc, char **oargv)
{
  char *argv[6];
  int status, count = 0;

  argv[0] = argv[2] = "DONTCARE";
  argv[1] = "TRUE";
  if ((status = do_mr_query("qualified_get_server", 3, argv, pserv,
			    &count)) && status != MR_NO_MATCH)
    com_err(whoami, status, " while getting servers");
  argv[0] = "*";
  argv[1] = argv[2] = argv[3] = argv[5] = "DONTCARE";
  argv[4] = "TRUE";
  if ((status = do_mr_query("qualified_get_server_host", 6, argv, phost,
			    &count)) && status != MR_NO_MATCH)
    com_err(whoami, status, " while getting server/hosts");
  if (!count)
    strcpy(buf, "Nothing is in progress at this time");
  else
    sprintf(buf, "%d update%s in progress at this time", count,
	    count == 1 ? "" : "s");
  Put_message(buf);
  return DM_NORMAL;
}

int DcmFailed(int argc, char **oargv)
{
  char *argv[6];
  int status, count = 0;

  argv[0] = argv[1] = "DONTCARE";
  argv[2] = "TRUE";
  if ((status = do_mr_query("qualified_get_server", 3, argv, pserv,
			    &count)) && status != MR_NO_MATCH)
    com_err(whoami, status, " while getting servers");
  argv[0] = "*";
  argv[1] = argv[2] = argv[3] = argv[4] = "DONTCARE";
  argv[5] = "TRUE";
  if ((status = do_mr_query("qualified_get_server_host", 6, argv, phost,
			    &count)) && status != MR_NO_MATCH)
    com_err(whoami, status, " while getting server/hosts");
  if (!count)
    strcpy(buf, "Nothing has failed at this time");
  else
    sprintf(buf, "%d thing%s ha%s failed at this time", count,
	    count == 1 ? "" : "s", count == 1 ? "s" : "ve");
  Put_message(buf);
  return DM_NORMAL;
}


int Dcm(int argc, char **argv)
{
  int status;
  if (Confirm("Are you sure you want to start a DCM now"))
    {
      if ((status = mr_do_update()))
	com_err(whoami, status, " while triggering update");
    }
  return DM_NORMAL;
}
