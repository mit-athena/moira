#if (!defined(lint) && !defined(SABER))
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/pobox.c,v 1.1 1988-07-08 18:26:16 kit Exp $";
#endif lint

/*	This is the file ***FILE for allmaint, the SMS client that allows
 *      a user to maintaint most important parts of the SMS database.
 *	It Contains: 
 *	
 *	Created: 	
 *	By:		
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/pobox.c,v $
 *      $Author: kit $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/pobox.c,v 1.1 1988-07-08 18:26:16 kit Exp $
 *	
 *  	Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */


#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <sms.h>
#include <menu.h>

#include "mit-copyright.h"
#include "allmaint.h"
#include "allmaint_funcs.h"
#include "globals.h"
#include "infodefs.h"

#define FOREIGN_BOX ("SMTP")
#define LOCAL_BOX ("POP")

/*	Function Name: PrintPOBox
 *	Description: Yet another specialized print function.
 *	Arguments: info - all info about this PO_box.
 *	Returns: SMS_CONT
 */

static void
PrintPOBox(info)
char ** info;
{
    char buf[BUFSIZ];
    
    sprintf(buf, "Address: %-10s Box: %-35s Type: %s", info[PO_NAME],
	    info[PO_BOX], info[PO_TYPE]);
    Put_message(buf);
}

/*	Function Name: RealPrintPOMachines
 *	Description: Actually does the printing for PrintPOMachines.
 *	Arguments: info, - name of machines in info[1].
 *	Returns: none.
 */

static void
RealPrintPOMachines(info)
char ** info;
{
    Print(1, info + 1, (char *) NULL);
}

/*	Function Name: PrintPOMachines
 *	Description: Prints all current post offices.
 *	Arguments: none.
 *	Returns: none.
 */

static void
PrintPOMachines()
{
    register int status;
    static char * args[] = {"pop", NULL};
    struct qelem * top = NULL;
    
    if ( (status = sms_query("get_server_locations", CountArgs(args), args,
			     StoreInfo, &top)) != SMS_SUCCESS)
	com_err(program_name, status, "in get_server_locations.");
    
    top = QueueTop(top);
    Loop(top, RealPrintPOMachines);
    FreeQueue(top);
}

/*	Function Name: GetUserPOBox
 *	Description: prints the users POBox information.
 *	Arguments: argc, argv - name of the user in argv[1].
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
GetUserPOBox(argc, argv)
int argc;
char ** argv;
{
    register int status;
    struct qelem * top = NULL;
    char buf[BUFSIZ];

    if (!ValidName(argv[1]))
	return(DM_NORMAL);
    
    switch (status = sms_query("get_pobox", 1, argv + 1, StoreInfo, &top)) {
    case SMS_NO_MATCH:
	Put_message("This user has no P.O. Box.");
	break;
    case SMS_SUCCESS:
	sprintf(buf,"Current pobox for user %s: \n", argv[1]);
	Put_message("");
	top = QueueTop(top);
	Loop(top, PrintPOBox);	/* should only return 1 box. */
	FreeQueue(top);
	break;
    default:
	com_err(program_name, status, "in get_pobox.");
    }
    return(DM_NORMAL);
}

/*	Function Name: GetNewLocalPOBox
 *	Description: get the machine for a new local pop Box for the user.
 *	Arguments: local_user - name of the local user.
 *	Returns: machine - name of the machine for then new pop box.
 */

static char *
GetNewLocalPOBox(local_user)
char * local_user;
{
    char temp_buf[BUFSIZ];

    sprintf(temp_buf, "%s did not have a previous local PO Box.", local_user);
    Put_message(temp_buf);
    sprintf(temp_buf,"%s %s", "Pick one of the following",
	    "machines for this user's Post Office.");
    Put_message(temp_buf);
    PrintPOMachines();
    Put_message("");
    Prompt_input("Which Machine? ", temp_buf, BUFSIZ);
    return( Strsave(temp_buf) );
}

/*	Function Name: SetUserPOBox
 *	Description: Addes or Chnages the P.O. Box for a user.
 *	Arguments: argc, argv - the login name of the user in argv[1].
 *	Returns: DM_NORMAL.
 */

int
SetUserPOBox(argc, argv)
int argc;
char **argv;
{
    register int status;
    char *type, temp_buf[BUFSIZ], *local_user, *args[10], box[BUFSIZ];
    local_user = argv[1];

    if (!ValidName(local_user))
	return(DM_NORMAL);
    
    (void) GetUserPOBox(argc, argv); /* print current info. */
    
    sprintf(temp_buf, "Assign %s a local PO Box (y/n)", local_user);
    switch (YesNoQuestion(temp_buf, TRUE)) {
    case TRUE:
	type = LOCAL_BOX;
	switch (YesNoQuestion("Use Previous Local Box (y/n)", TRUE)) {
	case TRUE:
	    switch (status = sms_query("set_pobox_pop", 1, 
				       &local_user, Scream, NULL)) {
	    case SMS_SUCCESS:
		return(DM_NORMAL);
	    case SMS_MACHINE:
		strcpy(box, GetNewLocalPOBox(local_user));
		break;
	    default:
		com_err(program_name, status, "in set_pobox_pop.");
		return(DM_NORMAL);
	    }
	case FALSE:
	    strcpy(box, GetNewLocalPOBox(local_user));
	    break;
	default:
	    return(DM_NORMAL);
	}
	break;
    case FALSE:
	type = FOREIGN_BOX;
	sprintf(temp_buf, "Set up a foreign PO Box for %s (y/n)", local_user);
	switch( YesNoQuestion(temp_buf, TRUE)) {
	case TRUE:
	    if (!Prompt_input("Foreign PO Box for this user? ", box, BUFSIZ))
		return(DM_NORMAL);
	    break;
	case FALSE:
	default:
	    return(DM_NORMAL);	/* ^C hit. */
	}
	
	args[PO_NAME] = local_user;
	args[PO_TYPE] = type;
	args[PO_BOX] = box;
	args[PO_END] = NULL;
	if ( (status = sms_query("set_pobox", CountArgs(args), args, 
				 Scream, NULL)) != SMS_SUCCESS )
	    com_err(program_name, status, " in ChangeUserPOBox");
	else
	    Put_message("PO Box assigned.");
    default:			/* ^C hit. */
	break;
    }
    return (DM_NORMAL);
}

/*	Function Name: RemoveUserPOBox
 *	Description: Removes this users POBox.
 *	Arguments: argc, argv - name of user in argv[1].
 *	Returns: DM_NORMAL.
 */

/*ARGSUSED */
int
RemoveUserPOBox(argc, argv)
int argc;
char ** argv;
{
    register int status;
    char temp_buf[BUFSIZ];
   
    if (!ValidName(argv[1]))
	return(DM_NORMAL);

    sprintf(temp_buf,
	    "Are you sure that you want to remove %s's PO Box (y/n)", argv[1]);
    
    if (Confirm(temp_buf)) {
	if ( (status = sms_query("delete_pobox", 1, argv + 1, Scream, NULL)) !=
	    SMS_SUCCESS)
	    com_err(program_name, status, "in delete_pobox.");
	else
	    Put_message("PO Box removed.");
    }
    return(DM_NORMAL);
}

/*
 * Local Variables:
 * mode: c
 * c-indent-level: 4
 * c-continued-statement-offset: 4
 * c-brace-offset: -4
 * c-argdecl-indent: 4
 * c-label-offset: -4
 * End:
 */

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
