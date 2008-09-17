/* $Id$
 *
 * Zephyr ACL routines for the Moira client
 *
 * Copyright (C) 1990-1998 by the Massachusetts Institute of Technology.
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

RCSID("$Header$");

void RealDeleteZephyr(char **info, Bool one_item);

/* Set the default values for zephyr additions. */

static char **SetDefaults(char **info, char *name)
{
  info[ZA_CLASS] = strdup(name);
  info[ZA_XMT_TYPE] = strdup("NONE");
  info[ZA_SUB_TYPE] = strdup("NONE");
  info[ZA_IWS_TYPE] = strdup("NONE");
  info[ZA_IUI_TYPE] = strdup("NONE");
  info[ZA_OWNER_TYPE] = strdup("NONE");
  info[ZA_XMT_ID] = strdup("");
  info[ZA_SUB_ID] = strdup("");
  info[ZA_IWS_ID] = strdup("");
  info[ZA_IUI_ID] = strdup("");
  info[ZA_OWNER_ID] = strdup("");
  info[ZA_MODTIME] = info[ZA_MODBY] = info[ZA_MODWITH] = NULL;
  info[ZA_END] = NULL;
  return info;
}


/* Get info from database */

static struct mqelem *GetZephyrInfo(char *name)
{
  int stat;
  struct mqelem *elem = NULL;

  if ((stat = do_mr_query("get_zephyr_class", 1, &name, StoreInfo, &elem)))
    {
      com_err(program_name, stat, " in GetZephyrInfo");
      return NULL;
    }
  return QueueTop(elem);
}


/* Print zephyr acl info */

static char *PrintZephyrInfo(char **info)
{
  char buf[BUFSIZ];

  if (!info)
    {
      Put_message("PrintZephyrInfo called with null info!");
      return NULL;
    }
  sprintf(buf, "        Zephyr class: %s", info[ZA_CLASS]);
  Put_message(buf);
  sprintf(buf, "Transmit ACL:           %s %s", info[ZA_XMT_TYPE],
	  (strcasecmp(info[ZA_XMT_TYPE], "NONE") && 
	   strcasecmp(info[ZA_XMT_TYPE], "ALL")) ? info[ZA_XMT_ID] : "");
  Put_message(buf);
  sprintf(buf, "Subscription ACL:       %s %s", info[ZA_SUB_TYPE],
	  (strcasecmp(info[ZA_SUB_TYPE], "NONE") && 
	   strcasecmp(info[ZA_SUB_TYPE], "ALL")) ? info[ZA_SUB_ID] : "");
  Put_message(buf);
  sprintf(buf, "Instance Wildcard ACL:  %s %s", info[ZA_IWS_TYPE],
	  (strcasecmp(info[ZA_IWS_TYPE], "NONE") && 
	   strcasecmp(info[ZA_IWS_TYPE], "ALL")) ? info[ZA_IWS_ID] : "");
  Put_message(buf);
  sprintf(buf, "Instance Identity ACL: %s %s", info[ZA_IUI_TYPE],
	  (strcasecmp(info[ZA_IUI_TYPE], "NONE") && 
	   strcasecmp(info[ZA_IUI_TYPE], "ALL")) ? info[ZA_IUI_ID] : "");
  Put_message(buf);
  sprintf(buf, "Owner:                  %s %s", info[ZA_OWNER_TYPE], 
	  (strcasecmp(info[ZA_OWNER_TYPE], "NONE") && 
	   strcasecmp(info[ZA_OWNER_TYPE], "ALL")) ? info[ZA_OWNER_ID] : "");
  Put_message(buf);
  sprintf(buf, MOD_FORMAT, info[ZA_MODBY], info[ZA_MODTIME], info[ZA_MODWITH]);
  Put_message(buf);

  return info[ZA_CLASS];
}


/* Get info from user.  Expects info to already be filled in with defaults */

static char **AskZephyrInfo(char **info, Bool rename)
{
  char buf[BUFSIZ], *newname;

  Put_message("");
  sprintf(buf, "Zephyr class restrictions for %s.", info[ZA_CLASS]);
  Put_message(buf);
  Put_message("");

  if (rename)
    {
      while (1)
	{
	  newname = strdup(info[ZA_CLASS]);
	  if (GetValueFromUser("The new name for this class", &newname) ==
	      SUB_ERROR)
	    return NULL;
	  if (ValidName(newname))
	    break;
	}
    }

  if (GetTypeFromUser("What kind of transmit restriction", "zace_type",
		      &info[ZA_XMT_TYPE]) == SUB_ERROR)
    return NULL;
  if (strcasecmp(info[ZA_XMT_TYPE], "NONE") &&
      strcasecmp(info[ZA_XMT_TYPE], "ALL"))
    {
      sprintf(buf, "Which %s: ", info[ZA_XMT_TYPE]);
      if (GetValueFromUser(buf, &info[ZA_XMT_ID]) == SUB_ERROR)
	return NULL;
    }
  if (GetTypeFromUser("What kind of subscription restriction", "zace_type",
		      &info[ZA_SUB_TYPE]) == SUB_ERROR)
    return NULL;
  if (strcasecmp(info[ZA_SUB_TYPE], "NONE") &&
      strcasecmp(info[ZA_SUB_TYPE], "ALL"))
    {
      sprintf(buf, "Which %s: ", info[ZA_SUB_TYPE]);
      if (GetValueFromUser(buf, &info[ZA_SUB_ID]) == SUB_ERROR)
	return NULL;
    }
  if (GetTypeFromUser("What kind of wildcard instance restriction",
		      "zace_type", &info[ZA_IWS_TYPE]) == SUB_ERROR)
    return NULL;
  if (strcasecmp(info[ZA_IWS_TYPE], "NONE") &&
      strcasecmp(info[ZA_IWS_TYPE], "ALL"))
    {
      sprintf(buf, "Which %s: ", info[ZA_IWS_TYPE]);
      if (GetValueFromUser(buf, &info[ZA_IWS_ID]) == SUB_ERROR)
	return NULL;
    }
  if (GetTypeFromUser("What kind of instance identity restriction",
		      "zace_type", &info[ZA_IUI_TYPE]) == SUB_ERROR)
    return NULL;
  if (strcasecmp(info[ZA_IUI_TYPE], "NONE") &&
      strcasecmp(info[ZA_IUI_TYPE], "ALL"))
    {
      sprintf(buf, "Which %s: ", info[ZA_IUI_TYPE]);
      if (GetValueFromUser(buf, &info[ZA_IUI_ID]) == SUB_ERROR)
	return NULL;
    }
  if (GetTypeFromUser("What kind of Administrator", "ace_type",
		      &info[ZA_OWNER_TYPE]) == SUB_ERROR)
    return NULL;
  if (strcasecmp(info[ZA_OWNER_TYPE], "NONE"))
    {
      sprintf(buf, "Which %s: ", info[ZA_OWNER_TYPE]);
      if (GetValueFromUser(buf, &info[ZA_OWNER_ID]) == SUB_ERROR)
	return NULL;
    }
  FreeAndClear(&info[ZA_MODTIME], TRUE);
  FreeAndClear(&info[ZA_MODBY], TRUE);
  FreeAndClear(&info[ZA_MODWITH], TRUE);

  if (rename)
    SlipInNewName(info, newname);
  return info;
}


/* Menu entry for get zephyr */
int GetZephyr(int argc, char **argv)
{
  struct mqelem *top;

  top = GetZephyrInfo(argv[1]);
  Loop(top, (void (*)(char **)) PrintZephyrInfo);
  FreeQueue(top);
  return DM_NORMAL;
}


/* Does the real work of a deletion */

void RealDeleteZephyr(char **info, Bool one_item)
{
  int stat;

  if ((stat = do_mr_query("delete_zephyr_class", 1, &info[ZA_CLASS],
			  NULL, NULL)))
    com_err(program_name, stat, " zephyr class restriction not deleted.");
  else
    Put_message("Zephyr class restriction deleted.");
}


/* Delete a zephyr class given it's name */

int DeleteZephyr(int argc, char **argv)
{
  struct mqelem *elem = GetZephyrInfo(argv[1]);
  QueryLoop(elem, PrintZephyrInfo, RealDeleteZephyr,
	    "Delete Zephyr class restriction for class ");
  FreeQueue(elem);
  return DM_NORMAL;
}


/* Add a new zephyr class */

int AddZephyr(int argc, char **argv)
{
  char *info[MAX_ARGS_SIZE], **args;
  int stat;

  if (!ValidName(argv[1]))
    return DM_NORMAL;

  if (!(stat = do_mr_query("get_zephyr_class", 1, argv + 1, NULL, NULL)))
    {
      Put_message ("A Zephyr class by that name already exists.");
      return DM_NORMAL;
    }
  else if (stat != MR_NO_MATCH)
    {
      com_err(program_name, stat, " in AddZehpyr");
      return DM_NORMAL;
    }

  args = AskZephyrInfo(SetDefaults(info, argv[1]), FALSE);
  if (!args)
    {
      Put_message("Aborted.");
      return DM_NORMAL;
    }

  if ((stat = do_mr_query("add_zephyr_class", CountArgs(args), args,
			  NULL, NULL)))
    com_err(program_name, stat, " in AddZephyr");

  FreeInfo(info);
  return DM_NORMAL;
}


/* Does the work of an update */

static void RealUpdateZephyr(char **info, Bool junk)
{
  int stat;
  char **args;

  if (!(args = AskZephyrInfo(info, TRUE)))
    {
      Put_message("Aborted.");
      return;
    }
  if ((stat = do_mr_query("update_zephyr_class", CountArgs(args), args,
			  NULL, NULL)))
    {
      com_err(program_name, stat, " in UpdateZephyr.");
      Put_message("Zephyr class ** NOT ** Updated.");
    }
  else
    Put_message("Zephyr class successfully updated.");
}


/* Change zephyr info */

int ChngZephyr(int argc, char **argv)
{
  struct mqelem *top;

  top = GetZephyrInfo(argv[1]);
  QueryLoop(top, NullPrint, RealUpdateZephyr, "Update class");

  FreeQueue(top);
  return DM_NORMAL;
}
