/* $Id: acl.c,v 1.1 2000-01-07 21:14:04 danw Exp $
 *
 *	This is the file acl.c for the Moira Client, which allows users
 *      to quickly and easily maintain most parts of the Moira database.
 *	It Contains: Functions for handling generic ACLs.
 *
 * Copyright (C) 1999 by the Massachusetts Institute of Technology.
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

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/acl.c,v 1.1 2000-01-07 21:14:04 danw Exp $");

void RealDeleteACL(char **info, Bool one_item);
void ChangeACL(char **info, Bool one_item);

/*	Function Name: SetDefaults
 *	Description: sets the default values for ACL additions.
 *	Arguments: info - an array of char pointers to recieve defaults.
 *	Returns: char ** (this array, now filled).
 */

static char **SetDefaults(char **info, char *host, char *target)
{
  info[ACL_HOST] = strdup(host);
  info[ACL_TARGET] = strdup(target);
  info[ACL_KIND] = strdup("");
  info[ACL_LIST] = strdup("");
  info[ACL_MODTIME] = info[ACL_MODBY] = info[ACL_MODWITH] = NULL;

  info[ACL_END] = NULL;
  return info;
}

/*	Function Name: GetACLInfo
 *	Description: Stores the info in a queue.
 *	Arguments: host, target - ACL to get info on
 *	Returns: a pointer to the first element in the queue or null
 * 		if ACL not found.
 */

static struct mqelem *GetACLInfo(char *host, char *target)
{
  int stat;
  struct mqelem *elem = NULL;
  char *argv[2];

  argv[0] = canonicalize_hostname(strdup(host));
  argv[1] = target;
  stat = do_mr_query("get_acl", 2, argv, StoreInfo, &elem);
  free(argv[0]);

  if (stat)
    {
      com_err(program_name, stat, " in GetACLInfo");
      return NULL;
    }
  return QueueTop(elem);
}

/*	Function Name: PrintACLInfo
 *	Description: Yet another specialized print function.
 *	Arguments: info - all info about this ACL.
 *	Returns: a static buffer...
 */

static char *PrintACLInfo(char **info)
{
  static char name[BUFSIZ];
  char buf[BUFSIZ];
  int status;

  if (!info)		/* If no informaion */
    {
      Put_message("PrintACLInfo called with null info!");
      return NULL;
    }
  Put_message("");
  sprintf(buf, "Host: %s", info[ACL_HOST]);
  Put_message(buf);
  sprintf(buf, "Target file: %s", info[ACL_TARGET]);
  Put_message(buf);
  sprintf(buf, "Kind: %-20s List: %s", info[ACL_KIND], info[ACL_LIST]);
  Put_message(buf);

  sprintf(name, "%s:%s", info[ACL_HOST], info[ACL_TARGET]);
  return name;
}

/*	Function Name: AskACLInfo.
 *	Description: This function askes the user for information about an
 *                   ACL and saves it into a structure.
 *	Arguments: info - a pointer the the structure to put the
 *                        info into.
 *	Returns: none.
 */

static char **AskACLInfo(char **info)
{
  char temp_buf[BUFSIZ];
  char *args[3];
  char *s, *d;
  int status;

  Put_message("");
  info[ACL_HOST] = canonicalize_hostname(info[ACL_HOST]);
  sprintf(temp_buf, "ACL %s:%s.", info[ACL_HOST], info[ACL_TARGET]);
  Put_message(temp_buf);
  Put_message("");

  if (GetTypeFromUser("Kind of ACL", "acl_kind", &info[ACL_KIND]) ==
      SUB_ERROR)
    return NULL;
  if (GetValueFromUser("List name", &info[ACL_LIST]) == SUB_ERROR)
    return NULL;

  FreeAndClear(&info[ACL_MODTIME], TRUE);
  FreeAndClear(&info[ACL_MODBY], TRUE);
  FreeAndClear(&info[ACL_MODWITH], TRUE);

  return info;
}

/* ---------------- ACL Menu ------------------ */

/*	Function Name: GetACL
 *	Description: Get ACL information
 *	Arguments: argc, argv - host and target file
 *	Returns: DM_NORMAL.
 */

int GetACL(int argc, char **argv)
{
  struct mqelem *top;

  top = GetACLInfo(argv[1], argv[2]);
  Loop(top, (void (*)(char **)) PrintACLInfo);
  FreeQueue(top);		/* clean the queue. */
  return DM_NORMAL;
}

/*	Function Name: RealDeleteACL
 *	Description: Does the real deletion work.
 *	Arguments: info - array of char *'s containing all useful info.
 *                 one_item - a Boolean that is true if only one item
 *                              in queue that dumped us here.
 *	Returns: none.
 */

void RealDeleteACL(char **info, Bool one_item)
{
  int stat;

  if ((stat = do_mr_query("delete_acl", 2, &info[ACL_HOST], NULL, NULL)))
    com_err(program_name, stat, " ACL not deleted.");
  else
    Put_message("ACL deleted.");
}

/*	Function Name: DeleteACL
 *	Description: Delete an ACL given its name.
 *	Arguments: argc, argv - host/target of the ACL
 *	Returns: none.
 */

int DeleteACL(int argc, char **argv)
{
  struct mqelem *elem = GetACLInfo(argv[1], argv[2]);
  QueryLoop(elem, PrintACLInfo, RealDeleteACL, "Delete ACL");

  FreeQueue(elem);
  return DM_NORMAL;
}

/*	Function Name: AddACL
 *	Description: Add an ACL
 *	Arguments: arc, argv - host/target of the ACL
 *	Returns: DM_NORMAL.
 */

int AddACL(int argc, char **argv)
{
  char *info[MAX_ARGS_SIZE], **args, *host;
  int stat;

  argv[1] = canonicalize_hostname(strdup(argv[1]));
  if (!(stat = do_mr_query("get_acl", 2, argv + 1, NULL, NULL)))
    {
      Put_message ("An ACL for that host and target already exists.");
      free(argv[1]);
      return DM_NORMAL;
    }
  else if (stat != MR_NO_MATCH)
    {
      com_err(program_name, stat, " in AddACL");
      free(argv[1]);
      return DM_NORMAL;
    }

  args = AskACLInfo(SetDefaults(info, argv[1], argv[2]));
  free(argv[1]);
  if (!args)
    {
      Put_message("Aborted.");
      return DM_NORMAL;
    }

  if ((stat = do_mr_query("add_acl", CountArgs(args), args, NULL, NULL)))
    com_err(program_name, stat, " in AddACL");

  FreeInfo(info);
  return DM_NORMAL;
}
