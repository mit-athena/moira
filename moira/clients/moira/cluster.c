#if (!defined(lint) && !defined(SABER))
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/cluster.c,v 1.5 1988-07-08 18:24:51 kit Exp $";
#endif lint

/*	This is the file cluseter.c for allmaint, the SMS client that allows
 *      a user to maintaint most important parts of the SMS database.
 *	It Contains: 
 *	
 *	Created: 	4/22/88
 *	By:		Chris D. Peterson
 *      Based upon:  Clusermaint.c by marcus: 87/07/22
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/cluster.c,v $
 *      $Author: kit $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/cluster.c,v 1.5 1988-07-08 18:24:51 kit Exp $
 *	
 *  	Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

/* BTW: for anyone who cares MCD is short for Machine, Cluster, Data. */

#include <stdio.h>
#include <strings.h>
#include <sms.h>
#include <menu.h>

#include "mit-copyright.h"
#include "allmaint.h"
#include "allmaint_funcs.h"
#include "globals.h"
#include "infodefs.h"

#define MACHINE  0
#define CLUSTER  1
#define DATA     2
#define MAP      3

#define M_DEFAULT_TYPE     DEFAULT_NONE

#define C_DEFAULT_DESCRIPT DEFAULT_NONE
#define C_DEFAULT_LOCATION DEFAULT_NONE

#define CD_DEFAULT_LABEL   DEFAULT_NONE
#define CD_DEFAULT_DATA    DEFAULT_NONE

/* -------------------- Set Defaults -------------------- */

/*	Function Name: SetMachineDefaults
 *	Description: sets machine defaults.
 *	Arguments: info - an array to put the defaults into.
 *                 name - Canonacalized name of the machine.
 *	Returns: info - the array.
 */

static char **
SetMachineDefaults(info, name)
char ** info, *name;
{
    info[M_NAME] = Strsave(name);
    info[M_TYPE] = Strsave(M_DEFAULT_TYPE);
    info[M_MODBY] = info[M_MODTIME] = info[M_MODWITH] = info[M_END] = NULL;
    return(info);
}

/*	Function Name: SetClusterDefaults
 *	Description: sets Cluster defaults.
 *	Arguments: info - an array to put the defaults into.
 *                 name - name of the Cluster.
 *	Returns: info - the array.
 */

static char **
SetClusterDefaults(info, name)
char ** info, *name;
{
    info[C_NAME] = Strsave(name);
    info[C_DESCRIPT] = Strsave(C_DEFAULT_DESCRIPT);
    info[C_LOCATION] = Strsave(C_DEFAULT_LOCATION);
    info[C_MODBY] = info[C_MODTIME] = info[C_MODWITH] = info[C_END] = NULL;
    return(info);
}

/* -------------------- General Functions -------------------- */

/*	Function Name: PrintMachInfo
 *	Description: This function Prints out the Machine info in 
 *                   a coherent form.
 *	Arguments: info - array of information about a machine.
 *	Returns: The name of the Machine
 */

static char *
PrintMachInfo(info)
char ** info;
{
    char buf[BUFSIZ];

    Put_message("");
    sprintf(buf, "Machine: %-30s Type: %s", info[M_NAME], info[M_TYPE]);
    Put_message(buf);
    sprintf(buf, MOD_FORMAT, info[M_MODBY], info[M_MODTIME], info[M_MODWITH]);
    Put_message(buf);
    return(info[M_NAME]);
}

/*	Function Name: PrintClusterInfo
 *	Description: This function Prints out the cluster info 
 *                   in a coherent form.
 *	Arguments: info - array of information about a cluster.
 *	Returns: The name of the cluster.
 */

static char *
PrintClusterInfo(info)
char ** info;
{
    char buf[BUFSIZ];

    Put_message("");
    sprintf(buf, "Cluster:     %s", info[C_NAME]);
    Put_message(buf);
    sprintf(buf, "Description: %s", info[C_DESCRIPT]);
    Put_message(buf);
    sprintf(buf, "Location:    %s", info[C_LOCATION]);
    Put_message(buf);
    sprintf(buf, MOD_FORMAT, info[C_MODBY], info[C_MODTIME], info[C_MODWITH]);
    Put_message(buf);
    return(info[C_NAME]);
}

/*	Function Name: PrintClusterData
 *	Description: Prints the Data on a cluster
 *	Arguments: info a pointer to the data array.
 *	Returns: The name of the cluster.
 */

static char *
PrintClusterData(info)
char ** info;
{
    char buf[BUFSIZ];

    Put_message("");
    sprintf(buf, "Cluster: %-20s Label: %-15s Data: %s",
	    info[CD_NAME], info[CD_LABEL], info[CD_DATA]);
    Put_message(buf);
    return(info[CD_NAME]);
}

/*	Function Name: PrintMCMap
 *	Description: Prints the data about a machine to cluster mapping.
 *	Arguments: info a pointer to the data array.
 *	Returns: none
 */

static char *
PrintMCMap(info)
char ** info;
{
    char buf[BUFSIZ];
    sprintf(buf, "Cluster: %-30s Machine: %-20s",
	    info[MAP_CLUSTER], info[MAP_MACHINE]);
    Put_message(buf);
    return("");			/* Used by QueryLoop(). */
}

/*	Function Name: GetMCInfo.
 *	Description: This function stores info about a machine.
 *                   type - type of data we are trying to retrieve.
 *                   name1 - the name of argv[0] for the call.
 *                   name2 - the name of argv[1] for the call.
 *	Returns: the top element of a queue containing the data or NULL.
 */

struct qelem *
GetMCInfo(type, name1, name2)
int type;
char * name1, *name2;
{

    int stat;
    struct qelem * elem = NULL;
    char * args[2];

    switch (type) {
    case MACHINE:
	if ( (stat = sms_query("get_machine", 1, &name1,
			       StoreInfo, &elem)) != 0) {
	    com_err(program_name, stat, " in get_machine.");
	    return(NULL);
	}
	break;
    case CLUSTER:
	if ( (stat = sms_query("get_cluster",  1, &name1,
			       StoreInfo, &elem)) != 0) {
	    com_err(program_name, stat, " in get_cluster.");
	    return(NULL);
	}
	break;
    case MAP:
	args[MAP_MACHINE] = name1;
	args[MAP_CLUSTER] = name2;
	if ( (stat = sms_query("get_machine_to_cluster_map", 2, args,
			       StoreInfo, &elem)) != 0) {
	    com_err(program_name, stat, " in get_machine_to_cluster_map.");
	    return(NULL);
	}
	break;
    case DATA:
	args[CD_NAME] = name1;
	args[CD_LABEL] = name2;
	if ( (stat = sms_query("get_cluster_data", 2, args,
			       StoreInfo, &elem)) != 0) {
	    com_err(program_name, stat, " in get_cluster_data.");
	    return(NULL);
	}
    }
    return(QueueTop(elem));
}

/*	Function Name: AskMCDInfo.
 *	Description: This function askes the user for information about a 
 *                   machine and saves it into a structure.
 *	Arguments: info - a pointer the information to ask about
 *                 type - type of information - MACHINE
 *                                              CLUSTER
 *                                              DATA
 *                 name - T/F : change the name of this type.
 *	Returns: none.
 */

char **
AskMCDInfo(info, type, name)
char ** info;
int type;
Bool name;
{
    char temp_buf[BUFSIZ], *newname;

    switch (type) {
    case MACHINE:
	sprintf(temp_buf, "Setting the information for the Machine %s.",
		info[M_NAME]);
	break;
    case CLUSTER:
	sprintf(temp_buf, "Setting the information for the Cluster %s.",
		info[C_NAME]);
	break;
    case DATA:
	sprintf(temp_buf, "Setting the Data for the Cluster %s.",
		info[CD_NAME]);
	break;
    }
    Put_message(temp_buf);

    if (name) {
	switch (type) {
	case MACHINE:
	    newname = Strsave(info[M_NAME]);
	    GetValueFromUser("The new name for this machine? ", &newname);
	    strcpy(temp_buf, CanonicalizeHostname(newname));
	    free(newname);
	    newname = Strsave(temp_buf);
	    break;
	case CLUSTER:
	    newname = Strsave(info[C_NAME]);
	    GetValueFromUser("The new name for this cluster? ",
			     &newname);
	    break;
	default:
	    Put_message("Unknown type in AskMCDInfo, programmer botch");
	    return(NULL);
	}
    }

    switch(type) {
    case MACHINE:
	GetValueFromUser("Machine's Type:", &info[M_TYPE]);
	FreeAndClear(&info[M_MODTIME], TRUE);
	FreeAndClear(&info[M_MODBY], TRUE);
	FreeAndClear(&info[M_MODWITH], TRUE);
	break;
    case CLUSTER:
	GetValueFromUser("Cluster's Description:", &info[C_DESCRIPT]);
	GetValueFromUser("Cluster's Location:", &info[C_LOCATION]);
	FreeAndClear(&info[C_MODTIME], TRUE);
	FreeAndClear(&info[C_MODBY], TRUE);
	FreeAndClear(&info[C_MODWITH], TRUE);
	break;
    case DATA:
	GetValueFromUser("Label defining this data?", &info[CD_LABEL]);
	GetValueFromUser("The data itself ? ", &info[CD_DATA]);
	break;
    }

/* 
 * Slide the newname into the #2 slot, this screws up all future references 
 * to this list.
 */
    if (name)			
	SlipInNewName(info, newname);

    return(info);
}

/* -----------  Machine Menu ----------- */

/*	Function Name: ShowMachineInfo
 *	Description: This function shows the information about a machine.
 *	Arguments: argc, argv - the name of the machine in argv[1].
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
ShowMachineInfo(argc, argv)
int argc;
char **argv;
{
    struct qelem *top;

    top = GetMCInfo(MACHINE, CanonicalizeHostname(argv[1]), (char *) NULL);
    Loop(top, ( (void *) PrintMachInfo) );
    FreeQueue(top);
    return(DM_NORMAL);
}

/*	Function Name: AddMachine
 *	Description: This function adds a new machine to the database.
 *	Arguments: argc, argv - the name of the machine in argv[1].
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
AddMachine(argc, argv)
int argc;
char **argv;
{
    char **args, *info[MAX_ARGS_SIZE], *name;
    int stat;

    if (!ValidName(argv[1]))	/* Checks for wildcards. */
	return(DM_NORMAL);
/* 
 * Check to see if this machine already exists. 
 */
    name =  CanonicalizeHostname(argv[1]);

    if ( (stat = sms_query("get_machine", 1, &name, NullFunc, NULL)) == 0) {
	Put_message("This machine already exists.");
	return(DM_NORMAL);
    }
    else if (stat != SMS_NO_MATCH) {
	com_err(program_name, stat, " in AddMachine.");
	return(DM_NORMAL);
    }

    args = AskMCDInfo(SetMachineDefaults(info, name), MACHINE, FALSE);

/*
 * Actually create the new Machine.
 */
    
    if ( (stat = sms_query("add_machine", CountArgs(args), 
			   args, Scream, NULL)) != 0)
	com_err(program_name, stat, " in AddMachine.");

    FreeInfo(info);
    return(DM_NORMAL);
}

/*	Function Name: RealUpdateMachine
 *	Description: Performs the actual update of the machine data.
 *	Arguments: info - the information on the machine to update.
 *                 junk - an UNUSED Boolean.
 *	Returns: none.
 */

/* ARGSUSED */
static void
RealUpdateMachine(info, junk)
char ** info;
Bool junk;
{
    register int stat;
    char ** args = AskMCDInfo(info, MACHINE, TRUE);
    if ( (stat = sms_query("update_machine", CountArgs(args), 
			   args, Scream, NULL)) != 0)
	com_err(program_name, stat, " in UpdateMachine.");
    else
	Put_message("Machine sucessfully updated.");
}

/*	Function Name: UpdateMachine
 *	Description: This function adds a new machine to the database.
 *	Arguments: argc, argv - the name of the machine in argv[1].
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
UpdateMachine(argc, argv)
int argc;
char **argv;
{
    struct qelem *top = GetMCInfo( MACHINE,  CanonicalizeHostname(argv[1]),
				   (char *) NULL);
    QueryLoop(top, NullPrint, RealUpdateMachine, "Update the machine");

    FreeQueue(top);
    return(DM_NORMAL);
}

/*	Function Name: CheckAndRemoveFromCluster
 *	Description: This func tests to see if a machine is in a cluster.
 *                   and if so then removes it
 *	Arguments: name - name of the machine (already Canonocalized).
 *                 ask_user- query the user before removing if from clusters?
 *	Returns: SMS_ERROR if machine left in a cluster, or sms_error.
 */

int 
CheckAndRemoveFromCluster(name, ask_user)
char * name;
Bool ask_user;
{
    register int stat, ret_value;
    Bool delete_it;
    char *args[10], temp_buf[BUFSIZ], *ptr;
    struct qelem *top, *elem = NULL;
    
    ret_value = SUB_NORMAL;	/* initialize ret_value. */
    args[0] = name;
    args[1] = "*";
    stat = sms_query("get_machine_to_cluster_map", 2, args, 
			 StoreInfo, &elem);
    if (stat && stat != SMS_NO_MATCH) {
	com_err(program_name, stat, " in get_machine_to_cluster_map.");
	return(DM_NORMAL);
    }
    if (stat == SMS_SUCCESS) {
	elem = top = QueueTop(elem);
	if (ask_user) {
	    sprintf(temp_buf, "%s is assigned to the following clusters.",
		    name);
	    Put_message(temp_buf);
	    Loop(top, (void *) PrintMCMap);
	    ptr = "Remove this machine from ** ALL ** these clusters?";
	    if (YesNoQuestion(ptr, FALSE) == TRUE) /* may return -1. */
		delete_it = TRUE;
	    else {
		Put_message("Aborting...");
		FreeQueue(top);
		return(SUB_ERROR);
	    }
	}
	else
	    delete_it = TRUE;

	if (delete_it) {
	    while (elem != NULL) {
		char **info = (char **) elem->q_data;
		if ( (stat = sms_query( "delete_machine_from_cluster",
				       2, info, Scream, NULL)) != 0) {
		    ret_value = SUB_ERROR;
		    com_err(program_name, stat, 
			    " in delete_machine_from_cluster.");
		    sprintf(temp_buf, 
			    "Machine %s ** NOT ** removed from cluster %s.",
			    info[MAP_MACHINE], info[MAP_CLUSTER]);
		    Put_message(temp_buf);
		}
		elem = elem->q_forw;
	    }
	}
    }
    return(ret_value);
}

/*	Function Name: RealDeleteMachine
 *	Description: Actually Deletes the Machine.
 *	Arguments: info - nescessary information stored as an array of char *'s
 *                 one_machine - a boolean, true if there is only one item in
 *                               the query.
 *	Returns: none.
 */

static void
RealDeleteMachine(info, one_machine)
char ** info;
Bool one_machine;
{
    register int stat;
    char temp_buf[BUFSIZ];

    sprintf(temp_buf, "Are you sure you want to delete the machine %s (y/n)? ",
	    info[M_NAME]);
    if(!one_machine || Confirm(temp_buf)) {
	if (CheckAndRemoveFromCluster(info[M_NAME], TRUE) != SUB_ERROR) {
	    if ( (stat = sms_query("delete_machine", 1,
				   &info[M_NAME], Scream, NULL)) != 0) {
		com_err(program_name, stat, " in DeleteMachine.");
		sprintf(temp_buf, "%s ** NOT ** deleted.", 
			info[M_NAME]);
		Put_message(temp_buf);
	    }
	    else {
		sprintf(temp_buf, "%s successfully Deleted.", info[M_NAME]);
		Put_message(temp_buf);
	    }
	}
    }
}

/*	Function Name: DeleteMachine
 *	Description: This function removes a machine from the data base.
 *	Arguments: argc, argv - the machines name int argv[1].
 *	Returns: DM_NORMAL.
 */

/* Perhaps we should remove the cluster if it has no machine now. */

/* ARGSUSED */
int
DeleteMachine(argc, argv)
int argc;
char **argv;
{
    struct qelem *top;

    top = GetMCInfo(MACHINE, CanonicalizeHostname(argv[1]), (char *) NULL);
    QueryLoop(top, PrintMachInfo, RealDeleteMachine, "Delete the machine");
    FreeQueue(top);
    return(DM_NORMAL);
}

/*	Function Name: AddMachineToCluster
 *	Description: This function adds a machine to a cluster
 *	Arguments: argc, argv - The machine name is argv[1].
 *                              The cluster name in argv[2].
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int 
AddMachineToCluster(argc, argv)
int argc;
char ** argv;
{
    int stat;
    char *machine, *cluster, temp_buf[BUFSIZ], *args[10];
    Bool add_it, one_machine, one_cluster;
    struct qelem * melem, *mtop, *celem, *ctop;

    machine = CanonicalizeHostname(argv[1]);
    cluster = argv[2];

    celem = ctop = GetMCInfo(CLUSTER,  cluster, (char *) NULL);
    melem = mtop = GetMCInfo(MACHINE,  machine, (char *) NULL);

    one_machine = (QueueCount(mtop) == 1);
    one_cluster = (QueueCount(ctop) == 1);

    /* No good way to use QueryLoop() here, sigh */

    while (melem != NULL) {
	char ** minfo = (char **) melem->q_data;
	while (celem != NULL) {
	    char ** cinfo = (char **) celem->q_data;
	    if (one_machine && one_cluster) 
		add_it = TRUE;
	    else {
		sprintf(temp_buf,"Add machine %s to cluster %s (y/n/q) ?",
			minfo[M_NAME], cinfo[C_NAME]);
		switch (YesNoQuitQuestion(temp_buf, FALSE)) {
		case TRUE:
		    add_it = TRUE;
		    break;
		case FALSE:
		    add_it = FALSE;
		    break;
		default:
		    Put_message("Aborting...");
		    FreeQueue(ctop);
		    FreeQueue(mtop);
		    return(DM_NORMAL);
		}
	    }
	    if (add_it) {
		args[0] = minfo[M_NAME];
		args[1] = cinfo[C_NAME];
		stat = sms_query("add_machine_to_cluster", 2, args,
				 Scream, NULL);
		switch (stat) {
		case SMS_SUCCESS:
		    break;
		case SMS_EXISTS:
		    sprintf(temp_buf, "%s is already in cluster %s",
			    minfo[M_NAME], cinfo[C_NAME]);
		    Put_message(temp_buf);
		    break;
		default:
		    com_err(program_name, stat, " in AddMachineToCluster.");
		    break;
		}
	    }
	    celem = celem->q_forw;
	}
	celem = ctop;		/* reset cluster element. */
	melem = melem->q_forw;
    }
    FreeQueue(ctop);
    FreeQueue(mtop);		    
    return(DM_NORMAL);
}

/*	Function Name: RealRemoveMachineFromCluster
 *	Description: This function actually removes the machine from its 
 *                   cluster.
 *	Arguments: info - all information nescessary to perform the removal.
 *                 one_map - True if there is only one case, and we should
 *                           confirm.
 *	Returns: none.
 */

static void
RealRemoveMachineFromCluster(info, one_map)
char ** info;
Bool one_map;
{
    char temp_buf[BUFSIZ];
    register int stat;

    sprintf(temp_buf, "Remove %s from the cluster %s", 
	    info[MAP_MACHINE], info[MAP_MACHINE]);
    if (!one_map || Confirm(temp_buf)) {
	if ( (stat = sms_query("delete_machine_from_cluster", 2, 
			       info, Scream, NULL)) != 0 )
	    com_err(program_name, stat, " in delete_machine_from_cluster");
	else {
	    sprintf(temp_buf, "%s has been removed from the cluster %s.",
		    info[MAP_MACHINE], info[MAP_CLUSTER]);
	    Put_message(temp_buf);
	}
    }
    else
	Put_message("Machine not removed.");
}

/*	Function Name: RemoveMachineFromCluster
 *	Description: Removes this machine form a specific cluster.
 *	Arguments: argc, argv - Name of machine in argv[1].
 *                              Name of cluster in argv[2].
 *	Returns: none.
 */

/* ARGSUSED */
int
RemoveMachineFromCluster(argc, argv)
int argc;
char ** argv;
{
    struct qelem *elem = NULL;
    char buf[BUFSIZ], * args[10];
    register int stat;

    args[MAP_MACHINE] = CanonicalizeHostname(argv[1]);
    args[MAP_CLUSTER] = argv[2];
    args[MAP_END] = NULL;

    stat = sms_query("get_machine_to_cluster_map", CountArgs(args), args,
		     StoreInfo, &elem);
    if (stat == SMS_NO_MATCH) {
	sprintf(buf, "The machine %s is not is the cluster %s.",
		args[MAP_MACHINE], args[MAP_CLUSTER]);
	Put_message(buf);
	return(DM_NORMAL);
    }
    if (stat)
	com_err(program_name, stat, " in delete_machine_from_cluster");

    elem = QueueTop(elem);
    QueryLoop(elem, PrintMCMap, RealRemoveMachineFromCluster,
	      "Remove this machine from this cluster");

    FreeQueue(elem);
    return(DM_NORMAL);
}

/* ---------- Cluster Menu -------- */

/*	Function Name: ShowClusterInfo
 *	Description: Gets information about a cluser given its name.
 *	Arguments: argc, argc - the name of the cluster in in argv[1].
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
ShowClusterInfo(argc, argv)
int argc;
char ** argv;
{
    struct qelem *top;

    top = GetMCInfo(CLUSTER, CanonicalizeHostname(argv[1]), (char *) NULL);
    Loop(top, (void *) PrintClusterInfo);
    FreeQueue(top);
    return(DM_NORMAL);
}

/*	Function Name: AddCluster
 *	Description: Creates a new cluster.
 *	Arguments: argc, argv - the name of the new cluster is argv[1].
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
AddCluster(argc, argv)
int argc;
char ** argv;
{
    char **args, *info[MAX_ARGS_SIZE], *name = argv[1];
    int stat;
/* 
 * Check to see if this cluster already exists. 
 */
    if (!ValidName(name))
	return(DM_NORMAL);

    if ( (stat = sms_query("get_cluster", 1, &name, 
			   NullFunc, NULL)) == SMS_SUCCESS) {
	Put_message("This cluster already exists.");
	return(DM_NORMAL);
    }
    else if (stat != SMS_NO_MATCH) {
	com_err(program_name, stat, " in AddCluster.");
	return(DM_NORMAL);
    }
    args = AskMCDInfo(SetClusterDefaults(info, name), CLUSTER, FALSE);
/*
 * Actually create the new Cluster.
 */
    if ( (stat = sms_query("add_cluster", CountArgs(args), 
			   args, Scream, NULL)) != 0)
	com_err(program_name, stat, " in AddCluster.");

    FreeInfo(info);
    return(DM_NORMAL);
}

/*	Function Name: RealUpdateCluster
 *	Description: This function actually performs the cluster update.
 *	Arguments: info - all information nesc. for updating the cluster.
 *                 junk - an UNUSED boolean.
 *	Returns: none.
 */

/* ARGSUSED */
static void
RealUpdateCluster(info, junk)
char ** info;
Bool junk;
{
    register int stat;
    char ** args = AskMCDInfo(info, CLUSTER, TRUE);
    if ( (stat = sms_query("update_cluster", CountArgs(args), 
			   args, Scream, NULL)) != 0)
	com_err(program_name, stat, " in UpdateCluster.");
    else
	Put_message("Cluster successfully updated.");
}

/*	Function Name: UpdateCluster
 *	Description: This Function Updates a cluster
 *	Arguments: name of the cluster in argv[1].
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int 
UpdateCluster(argc, argv)
int argc;
char ** argv;
{
    struct qelem *top;    
    top = GetMCInfo( CLUSTER, argv[1], (char *) NULL );
    QueryLoop(top, NullPrint, RealUpdateCluster, "Update the cluster");

    FreeQueue(top);
    return(DM_NORMAL);
}

/*	Function Name: CheckAndRemoveMachine
 *	Description: This function checks and removes all machines from a
 *                   cluster.
 *	Arguments: name - name of the cluster.
 *                 ask_first - if TRUE, then we will query the user, before
 *                             deletion.
 *	Returns: SUB_ERROR if all machines not removed.
 */

int
CheckAndRemoveMachines(name, ask_first)
char * name;
Bool ask_first;
{
    register int stat, ret_value;
    Bool delete_it;
    char *args[10], temp_buf[BUFSIZ], *ptr;
    struct qelem *top, *elem = NULL;
    
    ret_value = SUB_NORMAL;
    args[MAP_MACHINE] = "*";
    args[MAP_CLUSTER] = name;
    stat = sms_query("get_machine_to_cluster_map", 2, args, 
			 StoreInfo, &elem);
    if (stat && stat != SMS_NO_MATCH) {
	com_err(program_name, stat, " in get_machine_to_cluster_map.");
	return(DM_NORMAL);
    }
    if (stat == 0) {
	elem = top = QueueTop(elem);
	if (ask_first) {
	    sprintf(temp_buf,
		    "The cluster %s has the following machines in it:",
		    name);
	    Put_message(temp_buf);
	    while (elem != NULL) {
		char **info = (char **) elem->q_data;
		Print(1, &info[MAP_MACHINE], (char *) NULL);
		elem = elem->q_forw;
	    }
	    ptr = "Remove ** ALL ** these machines from this cluster?";

	    if (YesNoQuestion(ptr, FALSE) == TRUE) /* may return -1. */
		delete_it = TRUE;
	    else {
		Put_message("Aborting...");
		FreeQueue(top);
		return(SUB_ERROR);
	    }
	}
	else
	    delete_it = TRUE;

	if (delete_it) {
	    elem = top;
	    while (elem != 0) {
		char **info = (char **) elem->q_data;
		if ( (stat = sms_query( "delete_machine_from_cluster",
				       2, info, Scream, NULL)) != 0) {
		    ret_value = SUB_ERROR;
		    com_err(program_name, stat, 
			    " in delete_machine_from_cluster.");
		    sprintf(temp_buf, 
			    "Machine %s ** NOT ** removed from cluster %s.",
			    info[MAP_MACHINE], info[MAP_CLUSTER]);
		    Put_message(temp_buf);
		}
		elem = elem->q_forw;
	    }
	}
    }
    return(ret_value);
}

/*	Function Name: RealDeleteCluster
 *	Description: Actually performs the cluster deletion.
 *	Arguments: info - all information about this cluster.
 *                 one_cluster - If true then there was only one cluster in
 *                               the queue, and we should confirm.
 *	Returns: none.
 */

static void
RealDeleteCluster(info, one_cluster)
char ** info;
Bool one_cluster;
{
    register int stat;
    char temp_buf[BUFSIZ];
    
    sprintf(temp_buf, 
	    "Are you sure the you want to delete the cluster %s (y/n) ?", 
	    info[C_NAME]);
    if (!one_cluster || Confirm(temp_buf)) {
	if (CheckAndRemoveMachines(info[C_NAME], TRUE) != SUB_ERROR) {
	    if ( (stat = sms_query("delete_cluster", 1,
				   &info[C_NAME], Scream, NULL)) != 0) {
		com_err(program_name, stat, " in delete_cluster.");
		sprintf(temp_buf, "Cluster %s ** NOT ** deleted.", 
			info[C_NAME]);
		Put_message(temp_buf);
	    }
	    else {
		sprintf(temp_buf, "cluster %s sucesfully deleted.", 
			info[C_NAME]);
		Put_message(temp_buf);
	    }
	}
    }
}

/*	Function Name: DeleteCluster
 *	Description: This function removes a cluster from the database.
 *	Arguments: argc, argv - the name of the cluster is stored in argv[1].
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
DeleteCluster(argc, argv)
int argc;
char ** argv;
{
    struct qelem *top;

    top = GetMCInfo( CLUSTER, argv[1], (char *) NULL );
    QueryLoop(top, PrintClusterInfo, RealDeleteCluster, "Delete the cluster");

    FreeQueue(top);
    return(DM_NORMAL);
}
    
/* ----------- Cluster Data Menu -------------- */

/*	Function Name: ShowClusterData
 *	Description: This function shows the services for one cluster.
 *	Arguments: argc, argv - The name of the cluster is argv[1].
 *                              The label of the data in argv[2].
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
ShowClusterData(argc, argv)
int argc; 
char ** argv; 
{ 
    struct qelem *elem, *top;
    char **info;

    top = elem = GetMCInfo(DATA, argv[1], argv[2]);
    while (elem != NULL) {
	info = (char **) elem->q_data;
	PrintClusterData(info);
	elem = elem->q_forw;
    }
    FreeQueue(top);
    return(DM_NORMAL);
}

/*	Function Name: AddClusterData
 *	Description: This function adds some data to the cluster.
 *	Arguments: argv, argc:   argv[1] - the name of the cluster.
 *                               argv[2] - the label of the data.
 *                               argv[3] - the data.
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
AddClusterData(argc, argv)
int argc; 
char ** argv; 
{ 
    int stat;

    if( (stat = sms_query("add_cluster_data", 3, argv + 1,
			  Scream, (char *) NULL)) != 0)
	com_err(program_name, stat, " in AddClusterData.");

}

/*	Function Name: RealRemoveClusterData
 *	Description: actually removes the cluster data.
 *	Arguments: info - all info necessary to remove the cluster, in an array
 *                        of strings.
 *                 one_item - if true then the queue has only one elem and we
 *                            should confirm.
 *	Returns: none.
 */

static void
RealRemoveClusterData(info, one_item)
char ** info;
Bool one_item;
{
    register int stat;
    char * temp_ptr;

    Put_message(" ");
    temp_ptr = "Are you sure that you want to remove this cluster (y/n) ?";
    if (!one_item) PrintClusterData(info);
    if (!one_item || Confirm(temp_ptr)) {
	if( (stat = sms_query("delete_cluster_data", 3, info,
			      Scream, (char *) NULL)) != 0) {
	    com_err(program_name, stat, " in DeleteClusterData.");
	    Put_message("Data not removed.");
	}
	else
	    Put_message("Removal sucessful.");
    }
}

/*	Function Name: RemoveClusterData
 *	Description: This function removes data on a given cluster.
 *	Arguments: argv, argc:   argv[1] - the name of the cluster.
 *                               argv[2] - the label of the data.
 *                               argv[3] - the data.
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int 
RemoveClusterData(argc, argv)
int argc; 
char ** argv; 
{
    struct qelem *top;

    top = GetMCInfo(DATA, argv[1], argv[2]);
    QueryLoop(top, PrintClusterData, RealRemoveClusterData, 
	      "Remove this cluster data");

    FreeQueue(top);
    return(DM_NORMAL);
}

/*	Function Name: MachineToClusterMap
 *	Description: This Retrieves the mapping between machine and cluster
 *	Arguments: argc, argv - argv[1] -> machine name or wildcard.
 *                              argv[2] -> cluster name or wildcard.
 *	Returns: none.
 */

/* ARGSUSED */
int 
MachineToClusterMap(argc,argv)
int argc;
char **argv;
{
    struct qelem *elem, *top;

    top = elem = GetMCInfo(MAP, CanonicalizeHostname(argv[1]), argv[2]);
  
    Put_message("");		/* blank line on screen */
    while (elem != NULL) {
	char ** info = (char **) elem->q_data;
	PrintMCMap(info);
	elem = elem->q_forw;
    }

    FreeQueue(top);
    return(DM_NORMAL);
}

/*	Function Name: MachinesInCluster
 *	Description: Shows all machines in a give cluster.
 *	Arguments: argv, argc - name of cluster in argv[1].
 *	Returns: DM_NORMAL;
 */

/* ARGSUSED */
int
MachinesInCluster(argc, argv)
int argc;
char **argv;
{
    char *info[10];
    info[0] = argv[0];
    info[2] = argv[1];
    info[1] = "*";
    return(MachineToClusterMap(3, info));
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



