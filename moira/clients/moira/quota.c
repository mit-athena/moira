#if (!defined(lint) && !defined(SABER))
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/quota.c,v 1.1 1988-07-08 18:26:20 kit Exp $";
#endif lint

/*	This is the file ***FILE for allmaint, the SMS client that allows
 *      a user to maintaint most important parts of the SMS database.
 *	It Contains: 
 *	
 *	Created: 	
 *	By:		
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/quota.c,v $
 *      $Author: kit $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/quota.c,v 1.1 1988-07-08 18:26:20 kit Exp $
 *	
 *  	Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */


#include <stdio.h>
#include <strings.h>
#include <sms.h>
#include <menu.h>

#include "mit-copyright.h"
#include "allmaint.h"
#include "allmaint_funcs.h"
#include "globals.h"
#include "infodefs.h"

static char * def_quota = NULL;	
  
#define DEFAULT_FILESYS DEFAULT_NONE
#define DEFAULT_USER user	/* this is the user who started sms. */

/*	Function Name: GetDefaultUserQuota
 *	Description: gets the user quota from sms, and caches the value.
 *	Arguments: override - if true, go to sms and override the cache.
 *	Returns: none.
 *      NOTE: Using a queue here is pretty useless, but StoreInfo expects
 *            one, and it works, so why fuck with it.
 */

static char *
GetDefaultUserQuota(override)
Bool override;
{
  register int status;
  char ** info;
  struct qelem * top = NULL;
  static char *val[] = {"def_quota", NULL};

  if (override || (def_quota == NULL)) {
    if ( (status = sms_query("get_value", CountArgs(val), 
			   val, StoreInfo, (char *) &top)) != SMS_SUCCESS) {
	com_err(program_name, status, " in ShowDefaultQuota");
	if (def_quota == NULL) {
	  Put_message("No default Quota Found, setting default to 0.");
	  def_quota = Strsave("0");
	}
	else
	  Put_message("No default Quota Found, retaining old value.");
      }
    else {
      top = QueueTop(top);
      info = (char **) top->q_data;
      FreeAndClear(&def_quota, TRUE);
      def_quota = Strsave(info[0]);
      FreeQueue(top);
    }
  }
  return(def_quota);
}
    
/*	Function Name: PrintDefaultQuota
 *	Description: Prints default quota info in a meaningful way.
 *	Arguments: value of the default quota.
 *	Returns: none.
 */

static void
PrintDefaultQuota(quota)
char * quota;
{
    char temp_buf[BUFSIZ];
    Put_message("");
    sprintf(temp_buf,"The default quota is %s Kb.", quota);
    Put_message(temp_buf);
}

/*	Function Name: PrintQuota
 *	Description: Prints a users quota information.
 *	Arguments: info - a pointer to the quota information:
 *	Returns: none.
 */

static char *
PrintQuota(info)
char ** info;
{
    char buf[BUFSIZ];
    Put_message("");
    sprintf(buf, "Filsystem: %-45s User: %s",info[Q_FILESYS], info[Q_LOGIN]);
    Put_message(buf);
    sprintf(buf, "Machine: %-20s Directory: %-15s Quota: %s",
		   info[Q_MACHINE], info[Q_DIRECTORY], info[Q_QUOTA]);
    Put_message(buf);
    return(info[Q_FILESYS]);
}

/*	Function Name: GetQuotaArgs
 *	Description: gets quota args from the user
 *	Arguments: quota - if TRUE the get quota too.
 *	Returns: the arguments.
 */

static char **
GetQuotaArgs(quota)
Bool quota;
{
  char ** args = (char **) malloc( MAX_ARGS_SIZE * sizeof(char *) );
  
  if (args == NULL) {
    Put_message("Could not allocate memory in GetQuotaArgs.");
    return(NULL);
  }

  args[Q_FILESYS] = Strsave(DEFAULT_FILESYS);
  args[Q_LOGIN] = Strsave(DEFAULT_USER);
  if (quota) {
    args[Q_QUOTA] = Strsave(GetDefaultUserQuota(FALSE));
    args[3] = NULL;		/* NULL terminate. */
  }
  else 
    args[2] = NULL;		/* NULL terminate. */

  /* Get filesystem. */

  GetValueFromUser("Filesystem", &args[Q_FILESYS]);
  if (quota)			/* We need an exact entry. */
    if (!ValidName(args[Q_FILESYS]))
      return(NULL);

  /* Get and check username. */

  GetValueFromUser("Username", &args[Q_LOGIN]);
  if (!ValidName(args[Q_LOGIN])) return(NULL);

  if (quota) {			/* Get and check quota. */
    GetValueFromUser("Quota", &args[Q_QUOTA]);
    if (!ValidName(args[Q_QUOTA])) return(NULL);
  }
  return(args);
}  

/* ------------------------- Show Quota Info ------------------------- */

/*	Function Name: ShowDefaultQuota
 *	Description: This prints out a default quota for the system.
 *	Arguments: none
 *	Returns: DM_NORMAL.
 */

int
ShowDefaultQuota()
{
  PrintDefaultQuota(GetDefaultUserQuota(TRUE));
  return (DM_NORMAL);
}

/*	Function Name: ChangeDefaultQuota
 *	Description: Changes the System Wide default quota.
 *	Arguments: argc, argv - New quota in argv[1].
 *	Returns: DM_NORMAL.
 */

/*ARGSUSED*/
int
ChangeDefaultQuota(argc, argv)
int argc;
char *argv[];
{
    register int status;
    char temp_buf[BUFSIZ];
    static char *newval[] = {
	"def_quota", NULL, NULL
    };

    if (!ValidName(argv[1]))
	return(DM_NORMAL);

    sprintf(temp_buf,"%s %s","Are you sure that you want to",
	    "change the default quota for all new users");
    if(Confirm(temp_buf)) {
	newval[1] = argv[1];
	if ( (status = sms_query("update_value", CountArgs(newval), 
			       newval, Scream, NULL)) == SMS_SUCCESS ) {
	  FreeAndClear(&def_quota, TRUE);
	  def_quota = Strsave(argv[1]);
	}
	else
	    com_err(program_name, status, " in update_value");
    }
    else
	Put_message("Quota not changed.");

    return (DM_NORMAL);
}

/* ------------------------- User Quotas ------------------------- */

/*	Function Name: ShowUserQuota
 *	Description: Shows the quota of a user.
 *	Arguments: none
 *	Returns: DM_NORMAL
 */

int
ShowUserQuota()
{
  struct qelem *top = NULL;
  register int status;
  char ** args;

  if ( (args = GetQuotaArgs(FALSE) ) == NULL)
    return(DM_NORMAL);

  if ( (status = sms_query("get_nfs_quota", CountArgs(args), args,
			   StoreInfo, (char *) &top)) != SMS_SUCCESS)
    com_err(program_name, status, " in get_nfs_quota");
  
  FreeInfo(args);		/* done with args free them. */
  free(args);

  top = QueueTop(top);
  Loop(top, (void *) PrintQuota);
  
  FreeQueue(top);
  return (DM_NORMAL);
}

/*	Function Name: AddUserQuota
 *	Description: Adds a new quota entry to the database.
 *	Arguments: argc, argv - name of the filesystem in argv[1].
 *	Returns: DM_NORMAL
 */

int
AddUserQuota()
{
  char ** args;
  register int status;
  
  if ( (args = GetQuotaArgs(TRUE) ) == NULL)
    return(DM_NORMAL);

  if ( (status = sms_query("add_nfs_quota", CountArgs(args), args,
			   Scream, (char *) NULL)) != SMS_SUCCESS)
    com_err(program_name, status, " in get_nfs_quota");
  
  FreeInfo(args);
  free(args);
  return(DM_NORMAL);
}

/*	Function Name: RealUpdateUser
 *	Description: Performs the actual update of the user information.
 *	Arguments: info - the information nesc. to update the user.
 *	Returns: none.
 */

static void
RealUpdateUser(info)
char ** info;
{
  register int status;
  char temp_buf[BUFSIZ];

  sprintf(temp_buf, "New quota for filesystem %s (in KB)", info[Q_FILESYS]);
  GetValueFromUser(temp_buf, &info[Q_QUOTA]);
  
  if (status = sms_query("update_nfs_quota", 3, info,
			 Scream, (char *) NULL) != SMS_SUCCESS) {
    com_err(program_name, status, " in update_nfs_quota");
    sprintf(temp_buf,"Could not perform quota change on %s",
	    info[Q_FILESYS]); 
    Put_message(temp_buf);
  }
}
  
/*	Function Name: ChangeUserQuota
 *	Description: This function allows quotas to be updated.
 *	Arguments: none.
 *	Returns: DM_NORMAL.
 */

int
ChangeUserQuota()
{
  int status;
  char **args;
  struct qelem *top = NULL;
  
  if ( (args = GetQuotaArgs(FALSE) ) == NULL)
    return(DM_NORMAL);

  if ( (status = sms_query("get_nfs_quota", 2, args,
			   StoreInfo, (char *) &top)) != SMS_SUCCESS)
    com_err(program_name, status, " in get_nfs_quota");
  
  FreeInfo(args);		/* done with args, free them. */
  free(args);
  top = QueueTop(top);
  Loop(top, RealUpdateUser);

  FreeQueue(top);
  return (DM_NORMAL);
}
  
/*	Function Name: RealRemoveUserQuota
 *	Description: Actually removes the user quota.
 *	Arguments: info - all information about this user quota.
 *                 one_item - true if there is only one item in the queue, and
 *                            we should confirm.
 *	Returns: none.
 */

static void
RealRemoveUserQuota(info, one_item)
char ** info;
Bool one_item;
{
  register int status;
  char temp_buf[BUFSIZ];

  sprintf(temp_buf,
	  "Do you really want to delete the user %s's quota on filesystem %s",
	  info[Q_LOGIN], info[Q_FILESYS]);

  if (!one_item || Confirm(temp_buf)) {
    if ( (status = sms_query("delete_nfs_quota", 2, info,
			     Scream, (char *) NULL)) != SMS_SUCCESS)
      com_err(program_name, status, " in delete_nfs_quota");
    else
      Put_message("Quota sucessfully removed.");
  }
  else
    Put_message("Aborted.");
}

/*	Function Name: RemoveUserQuota
 *	Description: Removes a users quota on a given filsystem
 *	Arguments: none.
 *	Returns: DM_NORMAL.
 */

int
RemoveUserQuota()
{
  register int status;
  char **args;
  struct qelem *top = NULL;

  if ( (args = GetQuotaArgs(FALSE) ) == NULL)
    return(DM_NORMAL);

  if ( (status = sms_query("get_nfs_quota", 2, args,
			   StoreInfo, (char *) &top)) != SMS_SUCCESS)
    com_err(program_name, status, " in get_nfs_quota");

  FreeInfo(args);
  free(args);
  top = QueueTop(top);
  QueryLoop(top, PrintQuota, RealRemoveUserQuota,
	    "Delete This users quota on filesystem");

  FreeQueue(top);
  return(DM_NORMAL);
}
