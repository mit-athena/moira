#ifndef lint
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/cluster.c,v 1.1 1988-06-09 14:12:48 kit Exp $";
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
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/cluster.c,v 1.1 1988-06-09 14:12:48 kit Exp $
 *	
 *  	Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

/* BTW: for anyone who cares MCD is short for Machine, Cluster, Data. */

#include "mit-copyright.h"
#include "allmaint.h"
#include "globals.h"
#include "infodefs.h"

#include <stdio.h>
#include <strings.h>
#include <sms.h>
#include <menu.h>

#define MCD_MACHINE  0
#define MCD_CLUSTER  1
#define MCD_DATA     2
#define MCD_MAP      3

extern char *whoami;
extern char * CanonicalizeHostname();

/*	Function Name: PrintMachInfo
 *	Description: This function Prints out the Machine info in 
 *                   a coherent form.
 *	Arguments: info - array of information about a machine.
 *	Returns: none.
 */

void
PrintMachInfo(info)
char ** info;
{
    char buf[BUFSIZ];

    sprintf(buf, "Machine: %s\tType: %s", info[M_NAME], info[M_TYPE]);
    Put_message(buf);
    sprintf(buf, "Last Modified at %s, by %s with %s",info[M_MODTIME],
	    info[M_MODBY], info[M_MODWITH]);
    Put_message(buf);
}

/*	Function Name: PrintClusterInfo
 *	Description: This function Prints out the cluster info 
 *                   in a coherent form.
 *	Arguments: info - array of information about a cluster.
 *	Returns: none.
 */

void
PrintClusterInfo(info)
char ** info;
{
    char buf[BUFSIZ];

    sprintf(buf, "Cluster: %s", info[C_NAME]);
    Put_Message(buf);
    sprintf(buf,"Description: %-20s, Location: %-20s", info[C_DESCRIPT], 
	    info[C_LOCATION]);
    Put_message(buf);
    sprintf(buf, "Last Modified at %s, by %s with %s",info[C_MODTIME],
	    info[C_MODBY], info[C_MODWITH]);
    Put_message(buf);
}

/*	Function Name: PrintClusterData
 *	Description: Prints the Data on a cluster
 *	Arguments: info a pointer to the data array.
 *	Returns: none
 */

void
PrintClusterData(info)
char ** info;
{
    char buf[BUFSIZ];
    sprintf(buf, "Cluster: %-30s, Label: %-20s, Data: %-20s",
	    info[CD_NAME], info[CD_LABEL], info[CD_DATA]);
    Put_message(buf);
}

/*	Function Name: GetMCInfo.
 *	Description: This function stores info about a machine.
 *                   type - type of data we are trying to retrieve.
 *                   name1 - the name of argv[0] for the call.
 *                   name2 - the name of argv[1] for the call.
 *	Returns: the top element of a queue containing the data or NULL.
 */

struct qelem *
StoreMCInfo(type, name1, name2)
int type;
char * name1, *name2;
{

    int stat;
    struct qelem * elem = NULL;
    char * args[2];

    switch (type) {
    case MCD_MACHINE:
	if ( (stat = sms_query("get_machine", 1, &name1,
			       StoreInfo, &elem)) != 0) {
	    com_err(whoami, stat, " in get_machine.");
	    return(NULL);
	}
	break;
    case MCD_CLUSTER:
	if ( (stat = sms_query("get_cluster",  1, &name1,
			       StoreInfo, &elem)) != 0) {
	    com_err(whoami, stat, " in get_cluster.");
	    return(NULL);
	}
	break;
    case MCD_MAP:
	args[0] = name1;
	args[1] = name2;
	if ( (stat = sms_query("get_machine_to_cluster_map", 2, args,
			       StoreInfo, &elem)) != 0) {
	    com_err(whoami, stat, " in get_machine_to_cluster_map.");
	    return(NULL);
	}
	break;
    case MCD_DATA:
	args[0] = name1;
	args[1] = name2;
	if ( (stat = sms_query("get_cluster_data", 2, args,
			       StoreInfo, &elem)) != 0) {
	    com_err(whoami, stat, " in get_cluster_data.");
	    return(NULL);
	}
    }
    return(QueueTop(elem));
}

/*	Function Name: AskInfo.
 *	Description: This function askes the user for information about a 
 *                   machine and saves it into a structure.
 *	Arguments: info - a pointer the information to ask about
 *                 type - type of information - MCD_MACHINE
 *                                              MCD_CLUSTER
 *                                              MCD_DATA
 *                 name - T/F : change the name of this type.
 *	Returns: none.
 */

void
AskInfo(mcs_info, type, name);
char ** mcs_info;
int type;
Bool name;
{
    char * temp_buf;

    switch (type) {
    case MCD_MACHINE:
	sprintf(temp_buf, "Changing the information for the Machine %s.",
		name);
	break;
    case MCD_CLUSTER:
	sprintf(temp_buf, "Changing the information for the Cluster %s.",
		name);
	break;
    case MCD_DATA:
	sprintf(temp_buf, "Changing the Data for the Cluster %s.",
		name);
	break;
    }
    Put_message(temp_buf);

    if (name) {
	info[NEWNAME] = Strsave(info->name);
	switch (type) {
	case MCD_MACHINE:
	    GetValueFromUser("The new name for this machine? ",
			     info[M_NEWNAME]);
	    break;
	case MCD_CLUSTER:
	    GetValueFromUser("The new name for this cluster? ",
			     info[C_NEWNAME]);
	    break;
	default:
	    Put_message("Unknown type in AskInfo, programmer botch");
	    return;
	}
    }

    switch(type) {
    case MCD_MACHINE:
	GetValueFromUser("Machine's Type:",info[M_TYPE]);
	FreeAndClear(&info[M_MODTIME], TRUE);
	FreeAndClear(&info[M_MODBY], TRUE);
	FreeAndClear(&info[M_MODWITH], TRUE);
	break;
    case MCD_CLUSTER:
	GetValueFromUser("Cluster's Description:",info[C_DESCRIPT]);
	GetValueFromUser("Cluster's Location:",info[C_LOCATION]);
	FreeAndClear(&info[C_MODTIME], TRUE);
	FreeAndClear(&info[C_MODBY], TRUE);
	FreeAndClear(&info[C_MODWITH], TRUE);
	break;
    case MCD_DATA:
	GetValueFromUser("Label defining this data?",info[CD_LABEL]);
	GetValueFromUser("The data itself ? ",info[CD_DATA]);
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
    char **info;
    struct qelem *elem, *top;

    top = elem = GetMCInfo(  CanonicalizeHostname(argv[1]), MCD_MACHINE);

    while (elem != NULL) {
	info = (char **) elem->q_data;
	PrintMachInfo(info);
	elem = elem->q_forw;
    }

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
/* 
 * Check to see if this machine already exists. 
 */
    name =  CanonicalizeHostname(argv[1]);

    if ( (stat = sms_query("get_machine", 1, &name, NullFunc, NULL)) == 0) {
	Put_message("This machine already exists.");
	return(DM_NORMAL);
    }
    else if (stat != SMS_NOMATCH) {
	com_err(whoami, status, " in AddMachine.");
	return(DM_NORMAL);
    }

    info[0] = name;
    args = AskMCDInfo(info, MCD_MACHINE, FALSE);

/*
 * Actually create the new Machine.
 */
    
    if ( (stat = sms_query("add_machine", CountArgs(args), 
			   args, NullFunc, NULL)) != 0)
	com_err(whoami, status, " in AddMachine.");

    FreeInfo(info);
    return(DM_NORMAL);
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
    char **args, **info;
    int stat;
    
    elem = top = GetMCDInfo( MCD_MACHINE,  CanonicalizeHostname(argv[1]) );

    while (elem != NULL) {
	info = (char **) elem->q_data;
	args = AskMCDInfo(info, MCD_MACHINE, TRUE);
	if ( (stat = sms_query("update_machine", CountArgs(args), 
			       args, Scream, NULL)) != 0)
	    com_err(whoami, status, " in UpdateMachine.");
	elem = elem->q_forw;
    }

    FreeQueue(top);
    return(DM_NORMAL);
}

/*	Function Name: DeleteMachine
 *	Description: This function removes a machine from the data base.
 *	Arguments: argc, argv - the machines name int argv[1].
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
DeleteMachine(argc,argv)
int argc;
char **argv;
{
    int stat;
    char * args[2], *name;

    name =  CanonicalizeHostname(argv[1]);

/* Should probabally check for wildcards, none allowed. */
/* Perhaps we should remove the cluster if it has no machine now. */

    args[0] = argv[1];
    args[1] = "*";
    stat = sms_query("get_machine_to_cluster_map", 2, args, Null, NULL);
    if (stat != SMS_NO_MATCH) {
	sprintf(temp_buf, "%s is assigned to cluster %s.",args[0], args[1]);
	Put_message(temp_buf);
	if ( YesNoQuestion("Would you like to remove it from this cluster?",
			   FALSE)) {
	    if ( (stat = sms_query( "delete_machine_from_cluster",
				 1, args, Null, NULL)) != 0)
		com_err(whoami, status, " in DeleteMachine.");
	}
	else {
	    Put_message("Aborting...");
	    return(DM_NORMAL);
	}
    }
    if(Confirm("Are you sure that you want to delete this machine?")) {
	if ( (stat = sms_query("delete_machine", 1, &name, Null, NULL)) != 0)
	    com_err(whoami, status, " in DeleteMachine.");
    }
    else 
	Put_message("Operation aborted.");

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

    if ( (stat = sms_query("add_machine_to_cluster", 2, args + 1,
				  Scream, NULL) != 0))
	com_err(whoami, status, " in AddMachineToCluster.");

    return(DM_NORMAL);
}

/*	Function Name: RemoveMachineFromCluster
 *	Description: Removes this machine form a specific cluster.
 *	Arguments: argc, argv - Name of machine in argv[1].
 *	Returns: none.
 */

/* ARGSUSED */
int
RemoveMachineFromCluster(argc, argv)
int argc;
char ** argv;
{
    int stat, ans;
    struct qelem *elem, *top;
    char ** info;
    
    elem = top = GetMCDInfo(MCD_CLUSTER,  CanonicalizeHostname(argv[1]), "*");

    Put_message("This machine is the following clusters:");
    while (elem != NULL) {
	info = (char *) elem->q_data;
	Put_message(info[MAP_CLUSTER]);
	elem = elem->q_forw;
    }
    elem = top;

    if ( Confirm("Remove this machine from ** ALL ** these clusters?") ) {
	while (elem != NULL) {	/* delete all */
	    info = (char *) elem->q_data; 	 
	    if ( (stat =sms_query("delete_machine_from_cluster", 2, 
				  info, Scream, NULL)) != 0 )
		com_err(whoami, stat, " in delete_machine_from_cluster");
	    elem = elem->q_forw;
	}
    }
    else 
	while (elem != NULL) {	/* query delete. */
	    info = (char *) elem->q_data; 	
	    sprintf(buf, "Remove %13s from the cluster %30s? (y/n/q)", 
		    info[MAP_MACHINE], info[MAP_CLUSTER]);
	    ans = YesNoQuitQuestion(buf, FALSE);
	    if (ans == TRUE) 
		if ( (stat =sms_query("delete_machine_from_cluster", 2, 
				      info, Scream, NULL)) != 0 )
		    com_err(whoami, stat, " in delete_machine_from_cluster");
	    else if (ans != FALSE) /* quit. or ^C */
		break;
	    elem = elem->q_forw;
	}
    
    FreeQueue(top);
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
ShowClusterInfo(argc, argv);
int argc;
char ** argv;
{
    char **info;
    struct qelem *elem, *top;

    top = elem = GetMCInfo(argv[1], MCD_MACHINE);
    while (elem != NULL) {
	info = (char **) elem->q_data;
	PrintClusterInfo(info);
	elem = elem->q_forw;
    }
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
    char **args, *info[MAX_ARGS_SIZE], *name;
    int stat;
/* 
 * Check to see if this machine already exists. 
 */
    name = argv[1];

    if ( (stat = sms_query("get_cluster", 1, &name, NullFunc, NULL)) == 0) {
	Put_message("This cluster already exists.");
	return(DM_NORMAL);
    }
    else if (stat != SMS_NOMATCH) {
	com_err(whoami, status, " in AddCluster.");
	return(DM_NORMAL);
    }

    info[0] = name;
    args = AskMCDInfo(info, MCD_CLUSTER, FALSE);

/*
 * Actually create the new Cluster.
 */
    
    if ( (stat = sms_query("add_cluster", CountArgs(args), 
			   args, Scream, NULL)) != 0)
	com_err(whoami, status, " in AddCluster.");

    FreeInfo(info);
    return(DM_NORMAL);
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
    struct qelem *elem, *top;    
    char **args, **info;
    int stat;
    
    elem = top = GetMCDInfo( MCD_Cluster, argv[1], (char *) NULL );

    while (elem != NULL) {
	info = (char **) elem->q_data;
	args = AskMCDInfo(info, MCD_CLUSTER, TRUE);
	if ( (stat = sms_query("update_cluter", CountArgs(args), 
			       args, Scream, NULL)) != 0)
	    com_err(whoami, status, " in UpdateCluster.");
	elem = elem->q_forw;
    }

    FreeQueue(top);
    return(DM_NORMAL);
}

/*	Function Name: DeleteCluster
 *	Description: This function removes a cluster from the database.
 *	Arguments: argc, argv - the name of the cluster is stored in argv[1].
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
DeleteCluster(argc, argv);
int argc;
char ** argv;
{
    char * args[3];

/* Should probabally check for wildcards, none allowed. */

    args[MAP_MACHINE] = "*";
    args[MAP_CLUSTER] = argv[1];

    stat = sms_query("get_machine_to_cluster_map", 2, args, NullFunc, NULL);
    if (stat != SMS_NO_MATCH) {
	sprintf(temp_buf, "Cluster %s still has machines in it.",args[0]);
	Put_message(temp_buf);
	if ( YesNoQuestion("Would you like a list? (y/n)") == TRUE ) {
	    args[0] = "foo";	/* not used. */
	    args[1] = "*";
	    args[2] = argv[1];
	    MachineToClusterMap(3, args);
	}
	return(DM_NORMAL);
    }
    if(Confirm("Are you sure that you want to delete this cluster? ")) {
	if ( (stat = sms_query("delete_cluster", 
			       1, &args[MAP_CLUSTER], Scream, NULL)) != 0)
	    com_err(whoami, status, " in DeleteCluster.");
    }
    else 
	Put_message("Operation aborted.");

    return(DM_NORMAL);
}
    
/* ----------- Cluster Data Menu -------------- */

/*	Function Name: ShowClusterData
 *	Description: This function shows the services for one cluster.
 *	Arguments: argc, argv - The name of the cluster is argv[1].
 *                              The name of the data in argv[2].
 *	Returns: DM_NORMAL.
 */

int
ShowClusterData(argc, argv)
int argc; 
char ** argv; 
{ 
    struct qelem *elem, *top;
    char **info;

    top = elem = GetMCDInfo(MCD_DATA, argv[1], argv[2]);
    while (elem != NULL) {
	info = (char **) elem->q_data;
	PrintClusterData(info);
	elem = elem->data;
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
    if( (stat = sms_query("add_cluster_data", 3, argv + 1,
			  Scream, (char *) NULL)) != 0)
	com_err(whoami, status, " in AddClusterData.");

}

/*	Function Name: DeleteClusterData
 *	Description: This function deletes a service
 *	Arguments: argv, argc:   argv[1] - the name of the cluster.
 *                               argv[2] - the label of the data.
 *                               argv[3] - the data.
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int DeleteClusterData(argc, argv)
int argc; 
char ** argv; 
{
    int stat;

    if(Confirm("Do you really want to delete this data?")) {
	if( (stat = sms_query("delete_cluster_data", 3, argv + 1,
			      Scream, (char *) NULL)) != 0)
	    com_err(whoami, status, " in DeleteClusterData.");
    } else 
	Put_message("Operation aborted.\n");

    return(DM_NORMAL);
}

/* Perhaps an interactive delete cluster data would be nice. */

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
  int stat;

  argv[1] =  CanonicalizeHostname(argv[1]);
  
  if (stat = sms_query("get_machine_to_cluster_map", 
		    argc-1, argv+1, Print, NULL) != 0)
      com_err(whoami, qstat, NULL);

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



