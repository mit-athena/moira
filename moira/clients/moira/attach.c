/* $Id$
 *
 *	This is the file attach.c for the Moira Client, which allows users
 *      to quickly and easily maintain most parts of the Moira database.
 *	It Contains: Functions for maintaining data used by Hesiod
 *                   to map courses/projects/users to their file systems,
 *                   and maintain filesys info.
 *
 *	Created: 	5/4/88
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

#include <sys/stat.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

RCSID("$HeadURL$ $Id$");

char *canonicalize_cell(char *c);
int GetAliasValue(int argc, char **argv, void *retval);
void RealDeleteFS(char **info, Bool one_item);
char *SortAfter(struct mqelem *elem, int count);
void RealDeleteFSAlias(char **info, Bool one_item);

#define FS_ALIAS_TYPE "FILESYS"

#define LABEL        0
#define MACHINE      1
#define GROUP        2
#define ALIAS        3
#define ALIAS_CHECK  4
#define FS_ALIASES   5

#define NO_MACHINE	 ("[NONE]")

#define DEFAULT_TYPE     ("AFS")
#define DEFAULT_MACHINE  DEFAULT_NONE
#define DEFAULT_PACK     DEFAULT_NONE
#define DEFAULT_ACCESS   ("w")
#define DEFAULT_COMMENTS DEFAULT_COMMENT
#define DEFAULT_OWNER    (user)
#define DEFAULT_OWNERS   ("wheel")
#define DEFAULT_CREATE   DEFAULT_YES
#define DEFAULT_L_TYPE   ("PROJECT")
#define DEFAULT_CELL	 ("athena.mit.edu")

/*	Function Name: SetDefaults
 *	Description: sets the default values for filesystem additions.
 *	Arguments: info - an array of char pointers to recieve defaults.
 *	Returns: char ** (this array, now filled).
 */

static char **SetDefaults(char **info, char *name)
{
  char buf[BUFSIZ];

  info[FS_NAME] =     strdup(name);
  info[FS_TYPE] =     strdup(DEFAULT_TYPE);
  info[FS_MACHINE] =  strdup(DEFAULT_MACHINE);
  info[FS_PACK] =     strdup(DEFAULT_PACK);
  sprintf(buf, "/mit/%s", name);
  info[FS_M_POINT] =  strdup(buf);
  info[FS_ACCESS] =   strdup(DEFAULT_ACCESS);
  info[FS_COMMENTS] = strdup(DEFAULT_COMMENTS);
  info[FS_OWNER] =    strdup(DEFAULT_OWNER);
  info[FS_OWNERS] =   strdup(DEFAULT_OWNERS);
  info[FS_CREATE] =   strdup(DEFAULT_CREATE);
  info[FS_L_TYPE] =   strdup(DEFAULT_L_TYPE);
  info[FS_MODTIME] = info[FS_MODBY] = info[FS_MODWITH] = info[FS_END] = NULL;
  return info;
}

/*	Function Name: GetFSInfo
 *	Description: Stores the info in a queue.
 *	Arguments: type - type of information to get.
 *                 name - name of the item to get information on.
 *	Returns: a pointer to the first element in the queue.
 */

static struct mqelem *GetFSInfo(int type, char *name)
{
  int stat;
  struct mqelem *elem = NULL;
  char *args[5];

  switch (type)
    {
    case LABEL:
      if ((stat = do_mr_query("get_filesys_by_label", 1, &name,
			      StoreInfo, &elem)))
	{
	  com_err(program_name, stat, " in GetFSInfo");
	  return NULL;
	}
      break;
    case MACHINE:
      if ((stat = do_mr_query("get_filesys_by_machine", 1, &name,
			      StoreInfo, &elem)))
	{
	  com_err(program_name, stat, " in GetFSInfo");
	  return NULL;
	}
      break;
    case GROUP:
      if ((stat = do_mr_query("get_filesys_by_group", 1, &name,
			      StoreInfo, &elem)))
	{
	  com_err(program_name, stat, " in GetFSInfo");
	  return NULL;
	}
      break;
    case ALIAS:
    case ALIAS_CHECK:
      args[ALIAS_NAME] = name;
      args[ALIAS_TYPE] = FS_ALIAS_TYPE;
      args[ALIAS_TRANS] = "*";
      if ((stat = do_mr_query("get_alias", 3, args, StoreInfo, &elem)))
	{
	  if (type != ALIAS_CHECK || stat != MR_NO_MATCH)
	    com_err(program_name, stat, " in get_alias.");
	  return NULL;
	}
      break;
    case FS_ALIASES:
      args[ALIAS_NAME] = "*";
      args[ALIAS_TYPE] = FS_ALIAS_TYPE;
      args[ALIAS_TRANS] = name;
      if ((stat = do_mr_query("get_alias", 3, args, StoreInfo, &elem)))
	{
	  if (stat != MR_NO_MATCH)
	    com_err(program_name, stat, " in get_alias.");
	  return NULL;
	}
      break;
    }

  return QueueTop(elem);
}

/*	Function Name: PrintFSAlias
 *	Description: Prints a filesystem alias
 *	Arguments: info - an array contains the strings of info.
 *	Returns: the name of the filesys - used be QueryLoop().
 */

static char *PrintFSAlias(char **info)
{
  char buf[BUFSIZ];

  sprintf(buf, "Alias: %-25s Filesystem: %s", info[ALIAS_NAME],
	  info[ALIAS_TRANS]);
  Put_message(buf);
  return info[ALIAS_NAME];
}

static char aliasbuf[256];

static char *PrintFSAliasList(char **info)
{
  if (strlen(aliasbuf) == 0)
    snprintf(aliasbuf, sizeof(aliasbuf), "Aliases: %s", info[ALIAS_NAME]);
  else
    {
      strncat(aliasbuf, ", ", sizeof(aliasbuf));
      strncat(aliasbuf, info[ALIAS_NAME], sizeof(aliasbuf));
    }
}

static char labelbuf[256];

static char *GetFSLabel(char **info)
{
  snprintf(labelbuf, sizeof(labelbuf), "%s", info[ALIAS_TRANS]);
}

static int fsgCount = 1;

static void PrintFSGMembers(char **info)
{
  char print_buf[BUFSIZ];

  sprintf(print_buf, "  %d. Filesystem: %-32s (sort key: %s)",
	  fsgCount++, info[0], info[1]);
  Put_message(print_buf);
}


/*	Function Name: PrintFSInfo
 *	Description: Prints the filesystem information.
 *	Arguments: info - a pointer to the filesystem information.
 *	Returns: none.
 */

static char *PrintFSInfo(char **info)
{
  char print_buf[BUFSIZ];
  struct mqelem *top;

  FORMFEED;

  if (!strcmp(info[FS_TYPE], "FSGROUP") || !strcmp(info[FS_TYPE], "MUL"))
    {
      int stat;
      struct mqelem *elem = NULL;

      if (!strcmp(info[FS_TYPE], "MUL"))
	sprintf(print_buf, "%20s Multiple Filesystem: %s", " ", info[FS_NAME]);
      else
	sprintf(print_buf, "%20s Filesystem Group: %s", " ", info[FS_NAME]);
      Put_message(print_buf);

      top = GetFSInfo(FS_ALIASES, info[FS_NAME]);
      if (top != NULL) {
	*aliasbuf = 0;
	Loop(top, (void (*)(char **))PrintFSAliasList);
	Put_message(aliasbuf);
	FreeQueue(top);		/* clean the queue. */
      }

      sprintf(print_buf, "Comments: %s", info[FS_COMMENTS]);
      Put_message(print_buf);
      sprintf(print_buf, MOD_FORMAT, info[FS_MODBY], info[FS_MODTIME],
	      info[FS_MODWITH]);
      Put_message(print_buf);
      Put_message("Containing the filesystems (in order):");
      if ((stat = do_mr_query("get_fsgroup_members", 1, &info[FS_NAME],
			      StoreInfo, &elem)))
	{
	  if (stat == MR_NO_MATCH)
	    Put_message("    [no members]");
	  else
	    com_err(program_name, stat, " in PrintFSInfo");
	}
      else
	{
	  fsgCount = 1;
	  Loop(QueueTop(elem), PrintFSGMembers);
	  FreeQueue(elem);
	}
    }
  else
    {
      sprintf(print_buf, "%20s Filesystem: %s", " ", info[FS_NAME]);
      Put_message(print_buf);

      top = GetFSInfo(FS_ALIASES, info[FS_NAME]);
      if (top != NULL) {
	*aliasbuf = 0;
	Loop(top, (void (*)(char **))PrintFSAliasList);
	Put_message(aliasbuf);
	FreeQueue(top);		/* clean the queue. */
      }

      sprintf(print_buf, "Type: %-40s Machine: %-15s",
	      info[FS_TYPE], info[FS_MACHINE]);
      Put_message(print_buf);
      sprintf(print_buf, "Default Access: %-2s Packname: %-17s Mountpoint %s ",
	      info[FS_ACCESS], info[FS_PACK], info[FS_M_POINT]);
      Put_message(print_buf);
      sprintf(print_buf, "Comments: %s", info[FS_COMMENTS]);
      Put_message(print_buf);
      sprintf(print_buf, "User Ownership: %-30s Group Ownership: %s",
	      info[FS_OWNER], info[FS_OWNERS]);
      Put_message(print_buf);
      sprintf(print_buf, "Update Fileserver: %-27s Locker Type: %s",
	      atoi(info[FS_CREATE]) ? "ON" : "OFF", info[FS_L_TYPE]);
      Put_message(print_buf);
      sprintf(print_buf, MOD_FORMAT, info[FS_MODBY], info[FS_MODTIME],
	      info[FS_MODWITH]);
      Put_message(print_buf);
    }
  return info[FS_NAME];
}

char *canonicalize_cell(char *c)
{
#ifdef _WIN32
  return c;
#else /* !_WIN32 */
  struct stat stbuf;
  char path[512];
  int count;

  sprintf(path, "/afs/%s", c);
  if (lstat(path, &stbuf) || !stbuf.st_mode & S_IFLNK)
    return c;
  count = readlink(path, path, sizeof(path));
  if (count < 1)
    return c;
  path[count] = 0;
  free(c);
  return strdup(path);
#endif /* _WIN32 */
}


int GetAliasValue(int argc, char **argv, void *retval)
{
  *(char **)retval = strdup(argv[2]);
  return MR_CONT;
}

/*	Function Name: AskFSInfo.
 *	Description: This function askes the user for information about a
 *                   machine and saves it into a structure.
 *	Arguments: info - a pointer the the structure to put the
 *                             info into.
 *                 name - add a newname field? (T/F)
 *	Returns: none.
 */

static char **AskFSInfo(char **info, Bool name)
{
  char temp_buf[BUFSIZ], *newname, access_type[32];
  int fsgroup = 0, newdefaults = 0;

  Put_message("");
  sprintf(temp_buf, "Changing Attributes of filesystem %s.", info[FS_NAME]);
  Put_message(temp_buf);
  Put_message("");

  if (name)
    {
      newname = strdup(info[FS_NAME]);
      if (GetValueFromUser("The new name for this filesystem",
			   &newname) == SUB_ERROR)
	return NULL;
    }

  strcpy(temp_buf, info[FS_TYPE]);
  if (GetTypeFromUser("Filesystem's Type", "filesys", &info[FS_TYPE]) ==
      SUB_ERROR)
    return NULL;
  if (!strcasecmp(info[FS_TYPE], "FSGROUP") ||
      !strcasecmp(info[FS_TYPE], "MUL"))
    fsgroup++;
  if (strcasecmp(info[FS_TYPE], temp_buf))
    newdefaults++;
  if (fsgroup)
    {
      free(info[FS_MACHINE]);
      info[FS_MACHINE] = strdup(NO_MACHINE);
    }
  else
    {
      if (!strcasecmp(info[FS_TYPE], "AFS"))
	{
	  if (!name || newdefaults)
	    {
	      free(info[FS_MACHINE]);
	      info[FS_MACHINE] = strdup(DEFAULT_CELL);
	    }
	  if (GetValueFromUser("Filesystem's Cell", &info[FS_MACHINE]) ==
		SUB_ERROR)
	    return NULL;
	  info[FS_MACHINE] = canonicalize_cell(info[FS_MACHINE]);
	}
      else
	{
	  if (GetValueFromUser("Filesystem's Machine", &info[FS_MACHINE]) ==
	      SUB_ERROR)
	    return NULL;
	  info[FS_MACHINE] = canonicalize_hostname(info[FS_MACHINE]);
	}
    }
  if (!fsgroup)
    {
      char *p;
      int fsltypelen, fsnamelen;

      if (GetTypeFromUser("Filesystem's lockertype", "lockertype",
			  &info[FS_L_TYPE]) == SUB_ERROR)
	return NULL;

      if (!strcasecmp(info[FS_TYPE], "AFS"))
	{
	  char *path, *args[3], *p;
	  int status, depth;

	  if (!name || newdefaults)
	    {
	      free(info[FS_PACK]);
	      lowercase(info[FS_MACHINE]);
	      sprintf(temp_buf, "%s:%s", info[FS_MACHINE], info[FS_L_TYPE]);
	      args[0] = temp_buf;
	      args[1] = "AFSPATH";
	      args[2] = "*";
	      path = "???";
	      status = do_mr_query("get_alias", 3, args, GetAliasValue, &path);
	      if (status == MR_SUCCESS)
		{
		  p = strchr(path, ':');
		  if (p)
		    {
		      *p = '\0';
		      depth = atoi(++p);
		    }
		  else
		    depth = 0;
		  sprintf(temp_buf, "/afs/%s/%s", info[FS_MACHINE], path);
		  if (depth >= 0)
		    {
		      for (p = info[FS_NAME];
			   *p && (p - info[FS_NAME]) < depth; p++)
			{
			  if (islower(*p))
			    {
			      strcat(temp_buf, "/x");
			      temp_buf[strlen(temp_buf) - 1] = *p;
			    }
			  else
			    {
			      sprintf(temp_buf, "/afs/%s/%s/other",
				      info[FS_MACHINE], path);
			      break;
			    }
			}
		    }
		  else if (depth == -1)
		    {
		      if (isdigit(info[FS_NAME][0]))
			{
			  strcat(temp_buf, "/");
			  depth = strlen(temp_buf);
			  for (p = info[FS_NAME]; *p && isdigit(*p); p++)
			    {
			      temp_buf[depth++] = *p;
			      temp_buf[depth] = 0;
			    }
			}
		      else
			strcat(temp_buf, "/other");
		    }
		  else
		    {
			/* no default */
		    }
		  strcat(temp_buf, "/");
		  strcat(temp_buf, info[FS_NAME]);
		  free(path);
		}
	      else
		{
		  sprintf(temp_buf, "/afs/%s/%s/%s", info[FS_MACHINE],
			  lowercase(info[FS_L_TYPE]), info[FS_NAME]);
		}

	      info[FS_PACK] = strdup(temp_buf);
	    }
	  
	} 
      else if (!strcasecmp(info[FS_TYPE], "NFS"))
	{
	  char buf[BUFSIZ];

	  if (!strcasecmp(info[FS_L_TYPE], "SITE"))
	    {
	      sprintf(buf, "/site/%s", info[FS_NAME]);

	      free(info[FS_PACK]);
	      info[FS_PACK] = strdup(buf);

	      free(info[FS_M_POINT]);
	      info[FS_M_POINT] = strdup(buf);
	    }
	}
      
      /* If the lockername ends in ".lockertype" strip that.
       * eg.  the SITE locker "foo.site" becomes just "foo"
       */
      fsltypelen = strlen(info[FS_L_TYPE]);
      fsnamelen = strlen(info[FS_PACK]);
      p = (info[FS_PACK] + fsnamelen - fsltypelen);
      if (!strcasecmp(p, info[FS_L_TYPE]) && *(p-1) == '.')
	*(p-1) = '\0';
      fsnamelen = strlen(info[FS_M_POINT]);
      p = (info[FS_M_POINT] + fsnamelen - fsltypelen);
      if (!strcasecmp(p, info[FS_L_TYPE]) && *(p-1) == '.')
	*(p-1) = '\0';

      if (GetValueFromUser("Filesystem's Pack Name", &info[FS_PACK]) ==
	  SUB_ERROR)
	return NULL;
      if (GetValueFromUser("Filesystem's Mount Point", &info[FS_M_POINT]) ==
	  SUB_ERROR)
	return NULL;
      sprintf(access_type, "fs_access_%s", info[FS_TYPE]);
      if (GetTypeFromUser("Filesystem's Default Access", access_type,
			  &info[FS_ACCESS]) == SUB_ERROR)
	return NULL;
    }
  if (GetValueFromUser("Comments about this Filesystem", &info[FS_COMMENTS])
      == SUB_ERROR)
    return NULL;
  if (GetValueFromUser("Filesystem's owner (user)", &info[FS_OWNER]) ==
      SUB_ERROR)
    return NULL;
  if (GetValueFromUser("Filesystem's owners (group)", &info[FS_OWNERS]) ==
      SUB_ERROR)
    return NULL;
  if (!fsgroup)
    if (GetYesNoValueFromUser("Propagate changes to fileserver",
			      &info[FS_CREATE]) == SUB_ERROR)
      return NULL;

  FreeAndClear(&info[FS_MODTIME], TRUE);
  FreeAndClear(&info[FS_MODBY], TRUE);
  FreeAndClear(&info[FS_MODWITH], TRUE);

  if (name)			/* slide the newname into the #2 slot. */
    SlipInNewName(info, newname);

  return info;
}


/* --------------- Filesystem Menu ------------- */

/*	Function Name: GetFS
 *	Description: Get Filesystem information by name.
 *	Arguments: argc, argv - name of filsys in argv[1].
 *	Returns: DM_NORMAL.
 */

int GetFS(int argc, char **argv)
{
  struct mqelem *top;
  char *fs_label;

  fs_label = argv[1];

  top = GetFSInfo(ALIAS_CHECK, argv[1]);
  if (top != NULL) {
    Loop(top, (void (*)(char **))GetFSLabel);
    fs_label = labelbuf;
  }

  top = GetFSInfo(LABEL, fs_label); /* get info. */
  Loop(top, (void (*)(char **))PrintFSInfo);
  FreeQueue(top);		/* clean the queue. */
  return DM_NORMAL;
}

/*	Function Name: GetFSM
 *	Description: Get Filesystem information by machine.
 *	Arguments: argc, argv - name of server in argv[1].
 *	Returns: DM_NORMAL.
 */

int GetFSM(int argc, char **argv)
{
  struct mqelem *top;

  argv[1] = canonicalize_hostname(strdup(argv[1]));
  top = GetFSInfo(MACHINE, argv[1]); /* get info. */
  Loop(top, (void (*)(char **))PrintFSInfo);
  FreeQueue(top);		/* clean the queue. */
  return DM_NORMAL;
}

/*	Function Name: RealDeleteFS
 *	Description: Does the real deletion work.
 *	Arguments: info - array of char *'s containing all useful info.
 *                 one_item - a Boolean that is true if only one item
 *                              in queue that dumped us here.
 *	Returns: none.
 */

void RealDeleteFS(char **info, Bool one_item)
{
  int stat;
  char temp_buf[BUFSIZ];

  /*
   * Deletions are performed if the user hits 'y' on a list of multiple
   * filesystem, or if the user confirms on a unique alias.
   */
  sprintf(temp_buf, "Are you sure that you want to delete filesystem %s",
	  info[FS_NAME]);
  if (!one_item || Confirm(temp_buf))
    {
      if ((stat = do_mr_query("delete_filesys", 1,
			      &info[FS_NAME], NULL, NULL)))
	com_err(program_name, stat, " filesystem not deleted.");
      else
	Put_message("Filesystem deleted.");
    }
  else
    Put_message("Filesystem not deleted.");
}

/*	Function Name: DeleteFS
 *	Description: Delete a filesystem give its name.
 *	Arguments: argc, argv - argv[1] is the name of the filesystem.
 *	Returns: none.
 */

int DeleteFS(int argc, char **argv)
{
  struct mqelem *elem = GetFSInfo(LABEL, argv[1]);
  QueryLoop(elem, PrintFSInfo, RealDeleteFS, "Delete the Filesystem");

  FreeQueue(elem);
  return DM_NORMAL;
}

/*	Function Name: RealChangeFS
 *	Description: performs the actual change to the filesys.
 *	Arguments: info - the information
 *                 junk - an unused boolean.
 *	Returns: none.
 */

static void RealChangeFS(char **info, Bool junk)
{
  int stat;
  char **args;
  extern Menu nfsphys_menu;

  args = AskFSInfo(info, TRUE);
  if (!args)
    {
      Put_message("Aborted.");
      return;
    }
  stat = do_mr_query("update_filesys", CountArgs(args), args, NULL, NULL);
  switch (stat)
    {
    case MR_NFS:
      Put_message("That NFS filesystem is not exported.");
      if (YesNoQuestion("Fix this now (Y/N)", TRUE) == TRUE)
	{
	  Do_menu(&nfsphys_menu, 0, NULL);
	  if (YesNoQuestion("Retry filesystem update now (Y/N)", TRUE) == TRUE)
	    {
	      if ((stat = do_mr_query("update_filesys", CountArgs(args), args,
				      NULL, NULL)))
		com_err(program_name, stat, " filesystem not updated");
	      else
		Put_message("filesystem sucessfully updated.");
	    }
	}
      break;
    case MR_SUCCESS:
      break;
    default:
      com_err(program_name, stat, " in UpdateFS");
    }
}

/*	Function Name: ChangeFS
 *	Description: change the information in a filesys record.
 *	Arguments: arc, argv - value of filsys in argv[1].
 *	Returns: DM_NORMAL.
 */

int ChangeFS(int argc, char **argv)
{
  struct mqelem *elem = GetFSInfo(LABEL, argv[1]);
  QueryLoop(elem, NullPrint, RealChangeFS, "Update the Filesystem");

  FreeQueue(elem);
  return DM_NORMAL;
}

/*	Function Name: AddFS
 *	Description: change the information in a filesys record.
 *	Arguments: arc, argv - name of filsys in argv[1].
 *	Returns: DM_NORMAL.
 */

int AddFS(int argc, char **argv)
{
  char *info[MAX_ARGS_SIZE], **args, buf[BUFSIZ];
  int stat;
  extern Menu nfsphys_menu;

  if (!ValidName(argv[1]))
    return DM_NORMAL;

  if (!(stat = do_mr_query("get_filesys_by_label", 1, argv + 1,
			      NULL, NULL)))
    {
      Put_message ("A Filesystem by that name already exists.");
      return DM_NORMAL;
    }
  else if (stat != MR_NO_MATCH)
    {
      com_err(program_name, stat, " in AddFS");
      return DM_NORMAL;
    }

  if (!(args = AskFSInfo(SetDefaults(info, argv[1]), FALSE)))
    {
      Put_message("Aborted.");
      return DM_NORMAL;
    }

  stat = do_mr_query("add_filesys", CountArgs(args), args, NULL, NULL);
  switch (stat)
    {
    case MR_NFS:
      Put_message("That NFS filesystem is not exported.");
      if (YesNoQuestion("Fix this now (Y/N)", TRUE) == TRUE)
	{
	  Do_menu(&nfsphys_menu, 0, NULL);
	  if (YesNoQuestion("Retry filesystem creation now (Y/N)", TRUE)
	      == TRUE) {
	    if ((stat = do_mr_query("add_filesys", CountArgs(args), args,
				    NULL, NULL)))
	      com_err(program_name, stat, " in AddFS");
	    else
	      Put_message("Created.");
	  }
	}
      break;
    case MR_SUCCESS:
      break;
    default:
      com_err(program_name, stat, " in AddFS");
    }

  if (stat == MR_SUCCESS && !strcasecmp(info[FS_L_TYPE], "HOMEDIR"))
    {
      static char *val[] = {"def_quota", NULL};
      static char *def_quota = NULL;
      char *argv[Q_QUOTA + 1];
      struct mqelem *top = NULL;

      if (!def_quota)
	{
	  stat = do_mr_query("get_value", CountArgs(val), val,
			     StoreInfo, &top);
	  if (stat != MR_SUCCESS)
	    com_err(program_name, stat, " getting default quota");
	  else
	    {
	      top = QueueTop(top);
	      def_quota = strdup(((char **)top->q_data)[0]);
	      FreeQueue(top);
	    }
	}
      if (def_quota)
	{
	  sprintf(buf, "Give user %s a quota of %s on filesys %s (Y/N)",
		  info[FS_NAME], def_quota, info[FS_NAME]);
	  if (YesNoQuestion(buf, 1) == TRUE)
	    {
	      argv[Q_NAME] = argv[Q_FILESYS] = info[FS_NAME];
	      if (!strcmp(info[FS_TYPE], "NFS"))
		argv[Q_TYPE] = "USER";
	      else
		argv[Q_TYPE] = "ANY";
	      argv[Q_QUOTA] = def_quota;
	      if ((stat = do_mr_query("add_quota", Q_QUOTA + 1, argv, NULL,
				      NULL)))
		com_err(program_name, stat, " while adding quota");
	    }
	}
    }
  else if (stat == MR_SUCCESS)
    {
      if (YesNoQuestion("Assign a quota on this filesystem (Y/N)", 1) == TRUE)
	{
	  parsed_argc = 1;
	  parsed_argv[0] = info[FS_NAME];
	  AddQuota(parsed_argc, parsed_argv);
	}
    }

  FreeInfo(info);
  return DM_NORMAL;
}

/*	Function Name: SortAfter
 *	Description: choose a sortkey to cause an item to be added after
 *		the count element in the queue
 *	Arguments: queue of filesys names & sortkeys, queue count pointer
 *	Returns: sort key to use.
 */

char *SortAfter(struct mqelem *elem, int count)
{
  char *prev, *next, prevnext, *key, keybuf[9];

  /* first find the two keys we need to insert between */
  prev = "A";
  for (; count > 0; count--)
    {
      prev = ((char **)elem->q_data)[1];
      if (elem->q_forw)
	elem = elem->q_forw;
      else
	break;
    }
  if (count > 0)
    next = "Z";
  else
    next = ((char **)elem->q_data)[1];

  /* now copy the matching characters */
  for (key = keybuf; *prev && *prev == *next; next++)
    *key++ = *prev++;

  /* and set the last character */
  if (*prev == 0)
    *prev = prevnext = 'A';
  else
    prevnext = prev[1];
  if (prevnext == 0)
    prevnext = 'A';
  if (*next == 0)
    *next = 'Z';
  if (*next - *prev > 1)
    *key++ = (*next + *prev) / 2;
  else
    {
      *key++ = *prev;
      *key++ = (prevnext + 'Z') / 2;
    }
  *key = 0;
  return strdup(keybuf);
}

/*	Function Name: AddFSToGroup
 *	Description: add a filesystem to an FS group
 *	Arguments: arc, argv - name of group in argv[1], filesys in argv[2].
 *	Returns: DM_NORMAL.
 */

int AddFSToGroup(int argc, char **argv)
{
  int stat, count;
  struct mqelem *elem = NULL;
  char buf[BUFSIZ], *args[5], *bufp;

  if ((stat = do_mr_query("get_fsgroup_members", 1, argv + 1, StoreInfo,
			  &elem)))
    {
      if (stat != MR_NO_MATCH)
	com_err(program_name, stat, " in AddFSToGroup");
    }
  if (elem == NULL)
    {
      args[0] = argv[1];
      args[1] = argv[2];
      args[2] = "M";
      stat = do_mr_query("add_filesys_to_fsgroup", 3, args, NULL, NULL);
      if (stat)
	com_err(program_name, stat, " in AddFSToGroup");
      return DM_NORMAL;
    }
  elem = QueueTop(elem);
  fsgCount = 1;
  Loop(elem, PrintFSGMembers);
  sprintf(buf, "%d", QueueCount(elem));
  bufp = strdup(buf);
  if (GetValueFromUser("Enter number of filesystem it should follow "
		       "(0 to make it first):", &bufp) == SUB_ERROR)
    return DM_NORMAL;
  count = atoi(bufp);
  free(bufp);
  args[2] = SortAfter(elem, count);

  FreeQueue(QueueTop(elem));
  args[0] = argv[1];
  args[1] = argv[2];
  stat = do_mr_query("add_filesys_to_fsgroup", 3, args, NULL, NULL);
  if (stat == MR_EXISTS)
    {
      Put_message("That filesystem is already a member of the group.");
      Put_message("Use the order command if you want to change the "
		  "sorting order.");
    }
  else if (stat)
    com_err(program_name, stat, " in AddFSToGroup");
  return DM_NORMAL;
}


/*	Function Name: RemoveFSFromGroup
 *	Description: delete a filesystem from an FS group
 *	Arguments: arc, argv - name of group in argv[1].
 *	Returns: DM_NORMAL.
 */

int RemoveFSFromGroup(int argc, char **argv)
{
  int stat;
  char buf[BUFSIZ];

  sprintf(buf, "Delete filesystem %s from FS group %s", argv[2], argv[1]);
  if (!Confirm(buf))
    return DM_NORMAL;
  if ((stat = do_mr_query("remove_filesys_from_fsgroup", 2, argv + 1,
			  NULL, NULL)))
    com_err(program_name, stat, ", not removed.");
  return DM_NORMAL;
}

/*	Function Name: ChangeFSGroupOrder
 *	Description: change the sortkey on a filesys in an FSgroup
 *	Arguments: arc, argv - name of group in argv[1].
 *	Returns: DM_NORMAL.
 */

int ChangeFSGroupOrder(int argc, char **argv)
{
  int stat, src, dst, i;
  struct mqelem *elem = NULL, *top, *tmpelem;
  char buf[BUFSIZ], *bufp, *args[3];

  if ((stat = do_mr_query("get_fsgroup_members", 1, argv + 1, StoreInfo,
			  &elem)))
    {
      if (stat == MR_NO_MATCH)
	{
	  sprintf(buf,
		  "Ether %s is not a filesystem group or it has no members",
		  argv[1]);
	  Put_message(buf);
	}
      else
	com_err(program_name, stat, " in ChangeFSGroupOrder");
      return DM_NORMAL;
    }
  top = QueueTop(elem);
  fsgCount = 1;
  Loop(top, PrintFSGMembers);
  while (1)
    {
      bufp = strdup("1");
      if (GetValueFromUser("Enter number of the filesystem to move:",
			   &bufp) == SUB_ERROR)
	return DM_NORMAL;
      src = atoi(bufp);
      free(bufp);
      if (src < 0)
	{
	  Put_message("You must enter a positive number (or 0 to abort).");
	  continue;
	}
      else if (src == 0)
	{
	  Put_message("Aborted.");
	  return DM_NORMAL;
	}
      for (elem = top, i = src; i-- > 1 && elem->q_forw; elem = elem->q_forw)
	;
      if (i > 0)
	{
	  Put_message("You entered a number that is too high");
	  continue;
	}
      break;
    }
  while (1)
    {
      bufp = strdup("0");
      if (GetValueFromUser("Enter number of filesystem it should follow "
			   "(0 to make it first):", &bufp) == SUB_ERROR)
	return DM_NORMAL;
      dst = atoi(bufp);
      free(bufp);
      if (src == dst || src == dst + 1)
	{
	  Put_message("That has no effect on the sorting order!");
	  return DM_NORMAL;
	}
      if (dst < 0)
	{
	  Put_message("You must enter a non-negative number.");
	  continue;
	}
      for (tmpelem = top, i = dst; i-- > 1 && tmpelem->q_forw;
	   tmpelem = tmpelem->q_forw)
	;
      if (i > 0)
	{
	  Put_message("You entered a number that is too high");
	  continue;
	}
      break;
    }
  args[2] = SortAfter(top, dst);
  args[0] = argv[1];
  args[1] = ((char **)elem->q_data)[0];
  if ((stat = do_mr_query("remove_filesys_from_fsgroup", 2, args,
			  NULL, NULL)))
    {
      com_err(program_name, stat, " in ChangeFSGroupOrder");
      return DM_NORMAL;
    }
  if ((stat = do_mr_query("add_filesys_to_fsgroup", 3, args,
			     NULL, NULL)))
    com_err(program_name, stat, " in ChangeFSGroupOrder");
  return DM_NORMAL;
}


/* -------------- Top Level Menu ---------------- */

/*	Function Name: GetFSAlias
 *	Description: Gets the value for a Filesystem Alias.
 *	Arguments: argc, argv - name of alias in argv[1].
 *	Returns: DM_NORMAL.
 *      NOTES: There should only be one filesystem per alias, thus
 *             this will work correctly.
 */

int GetFSAlias(int argc, char **argv)
{
  struct mqelem *top;

  top = GetFSInfo(ALIAS, argv[1]);
  Put_message(" ");		/* blank line. */
  Loop(top, (void (*)(char **))PrintFSAlias);
  FreeQueue(top);
  return DM_NORMAL;
}

/*	Function Name: CreateFSAlias
 *	Description: Create an alias name for a filesystem
 *	Arguments: argc, argv - name of alias in argv[1].
 *	Returns: DM_NORMAL.
 *      NOTES:  This requires (name, type, transl)  I get {name, translation}
 *              from the user.  I provide type, which is well-known.
 */

int CreateFSAlias(int argc, char **argv)
{
  int stat;
  struct mqelem *elem, *top;
  char *args[MAX_ARGS_SIZE], buf[BUFSIZ], **info;

  elem = NULL;

  if (!ValidName(argv[1]))
    return DM_NORMAL;

  args[ALIAS_NAME] = strdup(argv[1]);
  args[ALIAS_TYPE] = strdup(FS_ALIAS_TYPE);
  args[ALIAS_TRANS] = strdup("*");

  /*
   * Check to see if this alias already exists in the database, if so then
   * print out values, free memory used and then exit.
   */

  if (!(stat = do_mr_query("get_alias", 3, args, StoreInfo, &elem)))
    {
      top = elem = QueueTop(elem);
      while (elem)
	{
	  info = elem->q_data;
	  sprintf(buf, "The alias: %s currently describes the filesystem %s",
		  info[ALIAS_NAME], info[ALIAS_TRANS]);
	  Put_message(buf);
	  elem = elem->q_forw;
	}
      FreeQueue(top);
      return DM_NORMAL;
    }
  else if (stat != MR_NO_MATCH)
    {
      com_err(program_name, stat, " in CreateFSAlias.");
      return DM_NORMAL;
    }

  args[ALIAS_TRANS] = strdup("");
  args[ALIAS_END] = NULL;
  if (GetValueFromUser("Which filesystem will this alias point to?",
		       &args[ALIAS_TRANS]) == SUB_ERROR)
    return DM_NORMAL;

  if ((stat = do_mr_query("add_alias", 3, args, NULL, NULL)))
    com_err(program_name, stat, " in CreateFSAlias.");

  FreeInfo(args);
  return DM_NORMAL;
}

/*	Function Name: RealDeleteFSAlias
 *	Description: Does the real deletion work.
 *	Arguments: info - array of char *'s containing all useful info.
 *                 one_item - a Boolean that is true if only one item
 *                              in queue that dumped us here.
 *	Returns: none.
 */

void RealDeleteFSAlias(char **info, Bool one_item)
{
  int stat;
  char temp_buf[BUFSIZ];

  /*
   * Deletions are  performed if the user hits 'y' on a list of multiple
   * filesystem, or if the user confirms on a unique alias.
   */
  sprintf(temp_buf,
	  "Are you sure that you want to delete the filesystem alias %s",
	  info[ALIAS_NAME]);
  if (!one_item || Confirm(temp_buf))
    {
      if ((stat = do_mr_query("delete_alias", CountArgs(info),
			      info, NULL, NULL)))
	com_err(program_name, stat, " filesystem alias not deleted.");
      else
	Put_message("Filesystem alias deleted.");
    }
  else
    Put_message("Filesystem alias not deleted.");
}

/*	Function Name: DeleteFSAlias
 *	Description: Delete an alias name for a filesystem
 *	Arguments: argc, argv - name of alias in argv[1].
 *	Returns: DM_NORMAL.
 *      NOTES:  This requires (name, type, transl)  I get {name, translation}
 *              from the user.  I provide type, which is well-known.
 */

int DeleteFSAlias(int argc, char **argv)
{
  struct mqelem *elem = GetFSInfo(ALIAS, argv[1]);
  QueryLoop(elem, PrintFSAlias, RealDeleteFSAlias,
	    "Delete the Filesystem Alias");
  FreeQueue(elem);
  return DM_NORMAL;
}

/*	Function Name: AttachHelp
 *	Description: Print help info on attachmaint.
 *	Arguments: none
 *	Returns: DM_NORMAL.
 */

int AttachHelp(void)
{
  static char *message[] = {
    "These are the options:",
    "",
    "get - get information about a filesystem.",
    "add - add a new filesystem to the data base.",
    "update - update the information in the database on a filesystem.",
    "delete - delete a filesystem from the database.",
    "check - check information about association of a name and a filesys.",
    "alias - associate a name with a filesystem.",
    "unalias - disassociate a name with a filesystem.",
    "verbose - toggle the request for delete confirmation.",
    NULL,
  };

  return PrintHelp(message);
}

/*	Function Name: FSGroupHelp
 *	Description: Print help info on fsgroups.
 *	Arguments: none
 *	Returns: DM_NORMAL.
 */

int FSGroupHelp(int argc, char **argv)
{
  static char *message[] = {
    "A filesystem group is a named sorted list of filesystems.",
    "",
    "To create, modify, or delete a group itself, use the menu above",
    "  this one, and manipulate a filesystem of type FSGROUP.",
    "Options here are:",
    "  get - get info about a group and show its members",
    "  add - add a new member to a group.",
    "  remove - remove a member from a group.",
    "  order - change the sorting order of a group.",
    NULL
  };

  return PrintHelp(message);
}
