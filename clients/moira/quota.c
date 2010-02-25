/* $Id: quota.c 3956 2010-01-05 20:56:56Z zacheiss $
 *
 *	This is the file quota.c for the Moira Client, which allows users
 *      to quickly and easily maintain most parts of the Moira database.
 *	It Contains: Functions for manipulating the quota information.
 *
 *	Created: 	7/10/88
 *	By:		Chris D. Peterson
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
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
#include <stdlib.h>
#include <string.h>

RCSID("$HeadURL: svn+ssh://svn.mit.edu/moira/trunk/moira/clients/moira/quota.c $ $Id: quota.c 3956 2010-01-05 20:56:56Z zacheiss $");

int afsfilsyshelper(int argc, char **argv, void *hint);
int afsfilesys(char *name);

static char *def_quota = NULL;

#define DEFAULT_FILESYS DEFAULT_NONE
#define DEFAULT_USER user	/* this is the user who started moira. */
#define NOBODY	"[nobody]"
#define DEFAULT_QTYPE   "USER"


/*	Function Name: GetDefaultUserQuota
 *	Description: gets the user quota from moira, and caches the value.
 *	Arguments: override - if true, go to moira and override the cache.
 *	Returns: none.
 *      NOTE: Using a queue here is pretty useless, but StoreInfo expects
 *            one, and it works, so why fuck with it.
 */

static char *GetDefaultUserQuota(Bool override)
{
  int status;
  char **info;
  struct mqelem *top = NULL;
  static char *val[] = {"def_quota", NULL};

  if (override || (def_quota == NULL))
    {
      if ((status = do_mr_query("get_value", CountArgs(val), val,
				StoreInfo, &top)))
	{
	  com_err(program_name, status, " in ShowDefaultQuota");
	  if (!def_quota)
	    {
	      Put_message("No default Quota Found, setting default to 0.");
	      def_quota = strdup("0");
	    }
	  else
	    Put_message("No default Quota Found, retaining old value.");
	}
      else
	{
	  top = QueueTop(top);
	  info = top->q_data;
	  FreeAndClear(&def_quota, TRUE);
	  def_quota = strdup(info[0]);
	  FreeQueue(top);
	}
    }
  return def_quota;
}

/*	Function Name: PrintDefaultQuota
 *	Description: Prints default quota info in a meaningful way.
 *	Arguments: value of the default quota.
 *	Returns: none.
 */

static void PrintDefaultQuota(char *quota)
{
  char temp_buf[BUFSIZ];
  Put_message("");
  sprintf(temp_buf, "The default quota is %s Kb.", quota);
  Put_message(temp_buf);
}

/*	Function Name: PrintQuota
 *	Description: Prints a users quota information.
 *	Arguments: info - a pointer to the quota information:
 *	Returns: none.
 */

static char *PrintQuota(char **info)
{
  char buf[BUFSIZ];

  Put_message("");

  if (!strcmp(info[Q_TYPE], "ANY"))
    sprintf(buf, "Filesystem: %s", info[Q_FILESYS]);
  else
    sprintf(buf, "Filesystem: %-45s %s %s", info[Q_FILESYS],
	    info[Q_TYPE], info[Q_NAME]);
  Put_message(buf);
  sprintf(buf, "Machine: %-20s Directory: %-15s",
	  info[Q_MACHINE], info[Q_DIRECTORY]);
  Put_message(buf);
  sprintf(buf, "Quota: %s", info[Q_QUOTA]);
  Put_message(buf);
  sprintf(buf, MOD_FORMAT, info[Q_MODBY], info[Q_MODTIME], info[Q_MODWITH]);
  Put_message(buf);
  return info[Q_FILESYS];
}


int afsfilsyshelper(int argc, char **argv, void *hint)
{
  *(int *)hint = !strcmp(argv[FS_TYPE], "AFS");
  return MR_CONT;
}


int afsfilesys(char *name)
{
  int status, ret = 0;
  char *argv[1];

  if (strchr(name, '*') || strchr(name, '?') || strchr(name, '\\'))
    return 0;
  argv[0] = name;
  status = do_mr_query("get_filesys_by_label", 1, argv, afsfilsyshelper, &ret);
  if (status == MR_SUCCESS)
    return ret;
  return status;
}


/*	Function Name: GetQuotaArgs
 *	Description: gets quota args from the user
 *	Arguments: quota - if TRUE the get quota too.
 *	Returns: the arguments.
 */

static char **GetQuotaArgs(Bool quota)
{
  char **args = malloc(MAX_ARGS_SIZE * sizeof(char *));
  int af;
  char *canon;

  if (!args)
    {
      Put_message("Could not allocate memory in GetQuotaArgs.");
      return NULL;
    }

  args[Q_FILESYS] = strdup(DEFAULT_FILESYS);
  args[Q_TYPE] = strdup(DEFAULT_QTYPE);
  args[Q_NAME] = strdup(DEFAULT_USER);
  if (quota)
    {
      args[Q_QUOTA] = strdup(GetDefaultUserQuota(FALSE));
      args[Q_QUOTA + 1] = NULL;	/* NULL terminate. */
    }
  else
    args[Q_NAME + 1] = NULL;	/* NULL terminate. */

  /* Get filesystem. */

  if (GetValueFromUser("Filesystem", &args[Q_FILESYS]) == SUB_ERROR)
    return NULL;
  if (quota && !ValidName(args[Q_FILESYS]))
    return NULL;

  af = afsfilesys(args[Q_FILESYS]);
  if (af != 0 && af != 1)
    {
      if (af == MR_NO_MATCH)
	Put_message("That filesystem does not exist.");
      else
	com_err(program_name, af, " in afsfilesys");
      return NULL;
    }
  if (af)
    {
      args[Q_TYPE] = strdup("ANY");
      args[Q_NAME] = strdup(NOBODY);
    }
  else
    {
      if (GetTypeFromUser("Quota type", "quota_type", &args[Q_TYPE]) ==
	  SUB_ERROR)
	return NULL;
      if (GetValueFromUser("Name", &args[Q_NAME]) == SUB_ERROR)
	return NULL;
      if (!ValidName(args[Q_NAME]))
	return NULL;
    }

  if (quota)			/* Get and check quota. */
    {
      if (GetValueFromUser("Quota", &args[Q_QUOTA]) == SUB_ERROR)
	return NULL;
      if (!ValidName(args[Q_QUOTA]))
	return NULL;
      canon = ParseQuotaString(args[Q_QUOTA]);
      if (!canon)
	return NULL;
      free(args[Q_QUOTA]);
      args[Q_QUOTA] = canon;
    }
  return args;
}

/*	Function Name: ShowDefaultQuota
 *	Description: This prints out a default quota for the system.
 *	Arguments: none
 *	Returns: DM_NORMAL.
 */

int ShowDefaultQuota(int argc, char **argv)
{
  PrintDefaultQuota(GetDefaultUserQuota(TRUE));
  return DM_NORMAL;
}

/*	Function Name: ChangeDefaultQuota
 *	Description: Changes the System Wide default quota.
 *	Arguments: argc, argv - New quota in argv[1].
 *	Returns: DM_NORMAL.
 */

int ChangeDefaultQuota(int argc, char *argv[])
{
  int status;
  char temp_buf[BUFSIZ];
  static char *newval[] = {
    "def_quota", NULL, NULL
  };

  if (!ValidName(argv[1]))
    return DM_NORMAL;

  sprintf(temp_buf, "%s %s", "Are you sure that you want to",
	  "change the default quota for all new users");
  if (Confirm(temp_buf))
    {
      newval[1] = argv[1];
      if ((status = do_mr_query("update_value", CountArgs(newval),
				newval, NULL, NULL)) == MR_SUCCESS)
	{
	  FreeAndClear(&def_quota, TRUE);
	  def_quota = strdup(argv[1]);
	}
      else
	com_err(program_name, status, " in update_value");
    }
  else
    Put_message("Quota not changed.");

  return DM_NORMAL;
}

/* ------------------------ Filesystem Quotas -------------------- */

/*      Function Name: GetQuota
 *      Description: Shows the quota of a filesystem w.r.t.
 *                   a group, user, or anybody (AFS)
 *      Arguments: none
 *      Returns: DM_NORMAL
 */

int GetQuota(int argc, char **argv)
{
  struct mqelem *top = NULL;
  int status;
  char **args;

  if (!(args = GetQuotaArgs(FALSE)))
    return DM_NORMAL;

  if ((status = do_mr_query("get_quota", CountArgs(args), args,
			    StoreInfo, &top)) != MR_SUCCESS)
    com_err(program_name, status, " in get_quota");

  FreeInfo(args);
  free(args);

  top = QueueTop(top);
  Loop(top, (void (*)(char **)) PrintQuota);

  FreeQueue(top);
  return DM_NORMAL;
}

/*        Function Name: GetQuotaByFilesys
 *        Description: Shows all quotas associated with the
 *                     given filesystem
 *        Arguments: none
 *        Returns: DM_NORMAL
 */

int GetQuotaByFilesys(int argc, char **argv)
{
  struct mqelem *top = NULL;
  int status;
  char **args = malloc(2 * sizeof(char *));

  if (!args)
    {
      Put_message("Could not allocate memory in GetQuotaByFilesys.");
      return DM_NORMAL;
    }

  args[0] = strdup("");
  args[1] = NULL;
  if (GetValueFromUser("Filesystem", &args[0]) == SUB_ERROR)
    return DM_NORMAL;

  if ((status = do_mr_query("get_quota_by_filesys", 1, args,
			    StoreInfo, &top)) != MR_SUCCESS)
    com_err(program_name, status, " in get_quota_by_filesys");

  FreeInfo(args);
  free(args);

  top = QueueTop(top);
  Loop(top, (void (*)(char **)) PrintQuota);

  FreeQueue(top);
  return DM_NORMAL;
}

/*        Function Name: AddQuota
 *        Description: Adds a new quota record for a filesystem
 *                     w.r.t. a user, group, or anybody (AFS).
 *        Arguments: None
 *        Returns: DM_NORMAL
 */
int AddQuota(int argc, char **argv)
{
  char **args;
  int status;

  if (!(args = GetQuotaArgs(TRUE)))
    return DM_NORMAL;

  if ((status = do_mr_query("add_quota", CountArgs(args), args,
			    NULL, NULL)) != MR_SUCCESS)
    com_err(program_name, status, " in add_quota");

  FreeInfo(args);
  free(args);
  return DM_NORMAL;
}

/*	Function Name: RealUpdateQuota
 *	Description: Performs the actual update of the quota
 *	Arguments: info - the information nesc. to update the quota.
 *	Returns: none.
 */

static void RealUpdateQuota(char **info)
{
  int status;
  char temp_buf[BUFSIZ];
  char *canon;

  sprintf(temp_buf, "New quota for filesystem %s (in KB)", info[Q_FILESYS]);
  if (GetValueFromUser(temp_buf, &info[Q_QUOTA]) == SUB_ERROR)
    {
      Put_message("Not changed.");
      return;
    }

  canon = ParseQuotaString(info[Q_QUOTA]);
  if (!canon)
    return;
  free(info[Q_QUOTA]);
  info[Q_QUOTA] = canon;

  if ((status = do_mr_query("update_quota", 4, info,
			    NULL, NULL)) != MR_SUCCESS)
      {
      com_err(program_name, status, " in update_quota");
      sprintf(temp_buf, "Could not perform quota change on %s",
	      info[Q_FILESYS]);
      Put_message(temp_buf);
    }
}

/*        Function Name: UpdateQuota
 *        Description: Updates an existing quota for a filesytem
 *                     w.r.t. a user, group, or anybody.
 *        Arguments: None
 *        Returns: DM_NORMAL
 */

int UpdateQuota(int argc, char **argv)
{
  int status;
  char **args;
  struct mqelem *top = NULL;

  if (!(args = GetQuotaArgs(FALSE)))
    return DM_NORMAL;

  if ((status = do_mr_query("get_quota", CountArgs(args), args,
			    StoreInfo, &top)) != MR_SUCCESS)
    com_err(program_name, status, " in get_quota");

  FreeInfo(args);		/* done with args, free them. */
  free(args);
  top = QueueTop(top);
  Loop(top, RealUpdateQuota);

  FreeQueue(top);
  return DM_NORMAL;
}


/*	Function Name: RealDeleteQuota
 *	Description: Actually removes the quota
 *	Arguments: info - all information about this quota.
 *                 one_item - true if there is only one item in the queue, and
 *                            we should confirm.
 *	Returns: none.
 */

static void RealDeleteQuota(char **info, Bool one_item)
{
  int status;
  char temp_buf[BUFSIZ];

  if (!strcmp(info[Q_TYPE], "ANY"))
    {
      sprintf(temp_buf, "Do you really want to delete the quota on %s",
	      info[Q_FILESYS]);
    }
  else
    {
      sprintf(temp_buf, "Do you really want to delete the %s %s's quota on %s",
	      (strcmp(info[Q_TYPE], "USER") ? "group" : "user"), info[Q_NAME],
	      info[Q_FILESYS]);
    }

  if (!one_item || Confirm(temp_buf))
    {
      if ((status = do_mr_query("delete_quota", 3, info,
				NULL, NULL)) != MR_SUCCESS)
	com_err(program_name, status, " in delete_quota");
      else
	Put_message("Quota sucessfully removed.");
    }
  else
    Put_message("Aborted.");
}

/*        Function Name: DeleteQuota
 *        Description: Removes the quota record for a filesystem
 *                     w.r.t. a user, group, or anybody.
 *        Arguments: None
 *        Returns: DM_NORMAL
 */

int DeleteQuota(int argc, char **argv)
{
  int status;
  char **args;
  struct mqelem *top = NULL;

  if (!(args = GetQuotaArgs(FALSE)))
    return DM_NORMAL;

  if ((status = do_mr_query("get_quota", 3, args, StoreInfo, &top)))
    com_err(program_name, status, " in get_quota");

  FreeInfo(args);
  free(args);
  top = QueueTop(top);
  QueryLoop(top, PrintQuota, RealDeleteQuota,
	    "Delete this quota on filesystem");

  FreeQueue(top);
  return DM_NORMAL;
}

char *ParseQuotaString(char *quota)
{
  char *s, *value;
  float ngigs, nmegs;
  int calcvalue;

  s = quota;
  while (*s && (isdigit(*s) || (*s == '.')))
    s++;

  /* If we didn't find a unit specifier, just return the old value. */
  if (!*s)
    return strdup(quota);

  switch (*s) {
  case 'm':
  case 'M':
    /* value specified in megabytes. */
    if (!sscanf(quota, "%f2", &nmegs))
      return strdup(quota);
    calcvalue = (int)(nmegs * 1000);
    break;
  case 'g':
  case 'G':
    /* value specified in gigabytes. */
    if (!sscanf(quota, "%f2", &ngigs))
      return strdup(quota);
    calcvalue = (int)(ngigs * 1000 * 1000);
    break;
  default:
    /* Couldn't parse it.  Just return the old value. */
    return strdup(quota);
  }

  value = malloc(BUFSIZ);
  if (!value)
    return NULL;
  sprintf(value, "%d", calcvalue);
  return value;
}
