/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/zephyr.c,v 1.2 1991-01-04 16:59:49 mar Exp $
 *
 * Zephyr ACL routines for the Moira client
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 * 
 * For further information on copyright and distribution see the
 * file mit-copyright.h
 */

#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <moira.h>
#include <moira_site.h>
#include <menu.h>
#include <mit-copyright.h>

#include "defs.h"
#include "f_defs.h"
#include "globals.h"


/* Set the default values for zephyr additions. */

static char **
SetDefaults(info, name)
char **info;
char *name;
{
    info[ZA_CLASS] = strsave(name);
    info[ZA_XMT_TYPE] = strsave("LIST");
    info[ZA_SUB_TYPE] = strsave("LIST");
    info[ZA_IWS_TYPE] = strsave("LIST");
    info[ZA_IUI_TYPE] = strsave("LIST");
    info[ZA_XMT_ID] = strsave("empty");
    info[ZA_SUB_ID] = strsave("empty");
    info[ZA_IWS_ID] = strsave("empty");
    info[ZA_IUI_ID] = strsave("empty");
    info[ZA_MODTIME] = info[ZA_MODBY] = info[ZA_MODWITH] = NULL;
    info[ZA_END] = NULL;
    return(info);
}


/* Get info from database */

static struct qelem *
GetZephyrInfo(name)
char *name;
{
    int stat;
    struct qelem *elem = NULL;

    if ((stat = do_mr_query("get_zephyr_class", 1, &name, StoreInfo,
			    (char *)&elem)) != 0) {
	com_err(program_name, stat, " in GetZephyrInfo");
	return(NULL);
    }
    return(QueueTop(elem));
}


/* Print zephyr acl info */

static void
PrintZephyrInfo(info)
char **info;
{
    char buf[BUFSIZ];

    if (!info) {
	Put_message("PrintZephyrInfo called with null info!");
	return;
    }
    sprintf(buf, "        Zephyr class: %s", info[ZA_CLASS]);
    Put_message(buf);
    if (!strcmp(info[ZA_XMT_ID], "WILDCARD")) {
	free(info[ZA_XMT_ID]);
	info[ZA_XMT_ID] = strsave("*.*@*");
    }
    sprintf(buf, "Transmit ACL:           %s %s", info[ZA_XMT_TYPE],
	    strcasecmp(info[ZA_XMT_TYPE], "NONE")?info[ZA_XMT_ID]:"");
    Put_message(buf);
    if (!strcmp(info[ZA_SUB_ID], "WILDCARD")) {
	free(info[ZA_SUB_ID]);
	info[ZA_SUB_ID] = strsave("*.*@*");
    }
    sprintf(buf, "Subscription ACL:       %s %s", info[ZA_SUB_TYPE],
	    strcasecmp(info[ZA_SUB_TYPE], "NONE")?info[ZA_SUB_ID]:"");
    Put_message(buf);
    if (!strcmp(info[ZA_IWS_ID], "WILDCARD")) {
	free(info[ZA_IWS_ID]);
	info[ZA_IWS_ID] = strsave("*.*@*");
    }
    sprintf(buf, "Instance Wildcard ACL:  %s %s", info[ZA_IWS_TYPE],
	    strcasecmp(info[ZA_IWS_TYPE], "NONE")?info[ZA_IWS_ID]:"");
    Put_message(buf);
    if (!strcmp(info[ZA_IUI_ID], "WILDCARD")) {
	free(info[ZA_IUI_ID]);
	info[ZA_IUI_ID] = strsave("*.*@*");
    }
    sprintf(buf, "Instance Indentity ACL: %s %s", info[ZA_IUI_TYPE],
	    strcasecmp(info[ZA_IUI_TYPE], "NONE")?info[ZA_IUI_ID]:"");
    Put_message(buf);
    sprintf(buf, MOD_FORMAT, info[ZA_MODBY], info[ZA_MODTIME], 
	    info[ZA_MODWITH]);
    Put_message(buf);
}


/* Get info from user.  Expects info to already be filled in with defaults */

static char **
AskZephyrInfo(info, rename)
char **info;
Bool rename;
{
    char buf[BUFSIZ], *newname;

    Put_message("");
    sprintf(buf, "Zephyr class restrictions for %s.", info[ZA_CLASS]);
    Put_message(buf);
    Put_message("");

    if (rename) {
	while (1) {
	    newname = Strsave(info[ZA_CLASS]);
	    if (GetValueFromUser("The new name for this class", &newname) ==
		SUB_ERROR)
	      return(NULL);
	    if (ValidName(newname))
	      break;
	}	  
    }

    if (GetTypeFromUser("What kind of transmit restriction", "ace_type",
			&info[ZA_XMT_TYPE]) == SUB_ERROR)
      return(NULL);
    if (strcasecmp(info[ZA_XMT_TYPE], "NONE") != 0) {
	if (!strcmp(info[ZA_XMT_ID], "WILDCARD")) {
	    free(info[ZA_XMT_ID]);
	    info[ZA_XMT_ID] = strsave("*.*@*");
	}
	sprintf(buf, "Which %s: ", info[ZA_XMT_TYPE]);
	if (GetValueFromUser(buf, &info[ZA_XMT_ID]) == SUB_ERROR)
	  return(NULL);
	if (!strcmp(info[ZA_XMT_ID], "*.*@*")) {
	    free(info[ZA_XMT_ID]);
	    info[ZA_XMT_ID] = strsave("WILDCARD");
	}
    }
    if (GetTypeFromUser("What kind of subscription restriction", "ace_type",
			&info[ZA_SUB_TYPE]) == SUB_ERROR)
      return(NULL);
    if (strcasecmp(info[ZA_SUB_TYPE], "NONE") != 0) {
	if (!strcmp(info[ZA_SUB_ID], "WILDCARD")) {
	    free(info[ZA_SUB_ID]);
	    info[ZA_SUB_ID] = strsave("*.*@*");
	}
	sprintf(buf, "Which %s: ", info[ZA_SUB_TYPE]);
	if (GetValueFromUser(buf, &info[ZA_SUB_ID]) == SUB_ERROR)
	  return(NULL);
	if (!strcmp(info[ZA_SUB_ID], "*.*@*")) {
	    free(info[ZA_SUB_ID]);
	    info[ZA_SUB_ID] = strsave("WILDCARD");
	}
    }
    if (GetTypeFromUser("What kind of wildcard instance restriction",
			"ace_type", &info[ZA_IWS_TYPE]) == SUB_ERROR)
      return(NULL);
    if (strcasecmp(info[ZA_IWS_TYPE], "NONE") != 0) {
	if (!strcmp(info[ZA_IWS_ID], "WILDCARD")) {
	    free(info[ZA_IWS_ID]);
	    info[ZA_IWS_ID] = strsave("*.*@*");
	}
	sprintf(buf, "Which %s: ", info[ZA_IWS_TYPE]);
	if (GetValueFromUser(buf, &info[ZA_IWS_ID]) == SUB_ERROR)
	  return(NULL);
	if (!strcmp(info[ZA_IWS_ID], "*.*@*")) {
	    free(info[ZA_IWS_ID]);
	    info[ZA_IWS_ID] = strsave("WILDCARD");
	}
    }
    if (GetTypeFromUser("What kind of instance identity restriction",
			"ace_type", &info[ZA_IUI_TYPE]) == SUB_ERROR)
      return(NULL);
    if (strcasecmp(info[ZA_IUI_TYPE], "NONE") != 0) {
	if (!strcmp(info[ZA_IUI_ID], "WILDCARD")) {
	    free(info[ZA_IUI_ID]);
	    info[ZA_IUI_ID] = strsave("*.*@*");
	}
	sprintf(buf, "Which %s: ", info[ZA_IUI_TYPE]);
	if (GetValueFromUser(buf, &info[ZA_IUI_ID]) == SUB_ERROR)
	  return(NULL);
	if (!strcmp(info[ZA_IUI_ID], "*.*@*")) {
	    free(info[ZA_IUI_ID]);
	    info[ZA_IUI_ID] = strsave("WILDCARD");
	}
    }
    FreeAndClear(&info[ZA_MODTIME], TRUE);
    FreeAndClear(&info[ZA_MODBY], TRUE);
    FreeAndClear(&info[ZA_MODWITH], TRUE);

    if (rename)
      SlipInNewName(info, newname);
    return(info);
}


/* Menu entry for get zephyr */
int
GetZephyr(argc, argv)
int argc;
char **argv;
{
    struct qelem *top;

    top = GetZephyrInfo(argv[1]);
    Loop(top, (void *) PrintZephyrInfo);
    FreeQueue(top);
    return (DM_NORMAL);
}


/* Does the real work of a deletion */

void
RealDeleteZephyr(info, one_item)
char **info;
Bool one_item;
{
    int stat;

    if ((stat = do_mr_query("delete_zephyr_class", 1, &info[ZA_CLASS],
			    Scream, NULL)) != 0)
      com_err(program_name, stat, " zephyr class restriction not deleted.");
    else
      Put_message("Zephyr class restriction deleted.");
}


/* Delete a zephyr class given it's name */

int
DeleteZephyr(argc, argv)
int argc;
char **argv;
{
    struct qelem *elem = GetZephyrInfo(argv[1]);
    QueryLoop(elem, PrintZephyrInfo, RealDeleteZephyr,
	      "Delete Zephyr class restriction for class ");
    FreeQueue(elem);
    return (DM_NORMAL);
}


/* Add a new zephyr class */

int
AddZephyr(argc, argv)
char **argv;
int argc;
{
    char *info[MAX_ARGS_SIZE], **args;
    int stat;

    if ( !ValidName(argv[1]) )
      return(DM_NORMAL);

    if ((stat = do_mr_query("get_zephyr_class", 1, argv + 1,
			    NullFunc, NULL)) == 0) {
	Put_message ("A Zephyr class by that name already exists.");
	return(DM_NORMAL);
    } else if (stat != MR_NO_MATCH) {
	com_err(program_name, stat, " in AddZehpyr");
	return(DM_NORMAL);
    } 

    args = AskZephyrInfo(SetDefaults(info, argv[1]), FALSE);
    if (args == NULL) {
	Put_message("Aborted.");
	return(DM_NORMAL);
    }

    if ((stat = do_mr_query("add_zephyr_class", CountArgs(args), args, 
			    NullFunc, NULL)) != 0)
      com_err(program_name, stat, " in AddZephyr");

    FreeInfo(info);
    return (DM_NORMAL);
}


/* Does the work of an update */

static void
RealUpdateZephyr(info, junk)
char **info;
Bool junk;
{
    int stat;
    char **args;
    
    if ((args = AskZephyrInfo(info, TRUE)) == NULL) {
	Put_message("Aborted.");
	return;
    }
    if ((stat = do_mr_query("update_zephyr_class", CountArgs(args), args, 
			    Scream, (char *) NULL)) != MR_SUCCESS) {
	com_err(program_name, stat, " in UpdateZephyr.");	
	Put_message("Zephyr class ** NOT ** Updated.");
    }
    else
      Put_message("Zephyr class successfully updated.");
}


/* Change zephyr info */

int
ChngZephyr(argc, argv)
int argc;
char **argv;
{
    struct qelem *top;

    top = GetZephyrInfo(argv[1], (char *) NULL);
    QueryLoop(top, NullPrint, RealUpdateZephyr, "Update class");

    FreeQueue(top);
    return(DM_NORMAL);
}
