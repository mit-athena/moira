/*	This is the file f_defs.h for the SMS Client, which allows a nieve
 *      user to quickly and easily maintain most parts of the SMS database.
 *	It Contains: useful definitions.
 *	
 *	Created: 	4/12/88
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/f_defs.h,v $
 *      $Author: mar $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/f_defs.h,v 1.16 1991-10-29 14:57:50 mar Exp $
 *	
 *  	Copyright 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include "mit-copyright.h"

#ifndef _f_defs_
#define _f_defs_

/* general Library functions. */

char * malloc();

/* attach.c */

int GetFS();			/* Get information about named filsys. */
int GetFSM();			/* Get filesys by machine */
int AddFS();			/* Adds a new Filsys to the database. */
int ChangeFS();			/* Changes information in a Filsys entry. */
int DeleteFS();			/* Delete a filesystem from the database. */

int AddFSToGroup();		/* Add a filesystem to an fsgroup */
int RemoveFSFromGroup();	/* remove a filesystem from an fsgroup */
int ChangeFSGroupOrder();	/* change the sortorder on an fsgrouop */
int FSGroupHelp();		/* print info about fsgroups */

int GetFSAlias();		/* Gets the value for a Filsys alias. */
int CreateFSAlias();		/* Creates a new alias in the database. */
int DeleteFSAlias();		/* Deletes an FS alias from the database. */

int AttachHelp();		/* Help information specific to filsys. */

/* cluster.c */

int ShowMachineInfo();		/* Show information on a(many) machine(s). */
int AddMachine();		/* add a new machine to teh database. */
int UpdateMachine();		/* updates machine information. */
int DeleteMachine();		/* delete a machine from the database. */
int MachineToClusterMap();	/* show machines in clusters. */
int AddMachineToCluster();	/* add a machine to a new cluster. */
int RemoveMachineFromCluster();	/* remove a machine from a cluster. */

int ShowClusterInfo();		/* Show information about a cluster. */
int AddCluster();		/* adds a new cluster to the database. */
int UpdateCluster();		/* updates information about a cluster. */
int DeleteCluster();		/* removes a cluste from the database. */

int ShowClusterInfo();		/* show information about a cluster. */
int MachinesInCluster();	/* list all machines in a give cluster. */
int AddCluster();		
int UpdateCluster();		/* add, update, and delete clusters. */
int DeleteCluster();

int ShowClusterData();		/* Show, add, and remove data  */
int AddClusterData();		/* associated with a cluster. */
int RemoveClusterData();

/* delete.c */

int DeleteList(), DeleteUser();

/* lists.c */

int ListByMember();		/* list all list to which a member belongs. */
int ListByAdministrator();	/* list all list to which a given member can
				 administer. */
int ListAllGroups();		
int ListAllPublicMailLists();	/* List misc. types of lists. */
int ListAllMailLists();

int AddMember(), DeleteMember(); /* add and delete mamber of current list. */
int ListAllMembers();
int ListUserMembers();		/* List various members of the current list. */
int ListListMembers();
int ListStringMembers();

int ShowListInfo();		/* show info about a list. */
int UpdateList();		/* change info about a list. */
int InterRemoveItemFromLists();	/* Interactivly remove an item from all lists*/

int ListmaintMemberMenuEntry();	/* entry and exit functions for member menu. */
int ListmaintMemberMenuExit();	
int ListHelp();			/* help function for lists. */

/* nfs.c */

int ShowNFSService();		/* functions that manipulate nfs physical  */
int AddNFSService();		/* services. */
int UpdateNFSService();
int DeleteNFSService();

/* pobox.c */

int GetUserPOBox();		/* operate on user post office boxes. */
int SetUserPOBox();
int RemoveUserPOBox();

/* quota.c */

int ShowDefaultQuota();		/* show or change system default quota. */
int ChangeDefaultQuota();

int GetQuota();	        	/* modify a user's quota. */
int GetQuotaByFilesys();
int AddQuota();
int UpdateQuota();
int DeleteQuota();

/* user.c */

int ShowUserByLogin();		/* misc. ways to show user. */
int ShowUserByName();
int ShowUserByClass();
int AddNewUser();		/* add a new user to the database. */
int RegisterUser();		/* register a user. */
int UpdateUser();		/* modify info about a user. */
int DeactivateUser();		/* change a user's status to DELETED */
int ChangeUserPOBox();		/* change the PO box of a user. */
int DeleteUserByUid();		/* detete a user by uid. */
int GetKrbmap();		/* fetch a user->kerberos mapping */
int AddKrbmap();
int DeleteKrbmap();

/* printer.c */
int AddPcap();
int GetPcap();
int ChngPcap();
int DeletePcap();
int GetPalladium(), AddPalladium(), ChngPalladium(), DeletePalladium();
int ShowPalladiumAlias(), AddPalladiumAlias(), DeletePalladiumAlias();

/* zephyr.c */
int AddZephyr();
int GetZephyr();
int ChngZephyr();
int DeleteZephyr();

/* dcm.c */
int EnableDcm(), InProgress(), DcmFailed(), Dcm();
int enabledcm(), showserv(), addserv(), updateserv(), delserv();
int showhost(), resetsrverr(), resetsrvc(), resethosterr();
int resethost(), sethostor(), addhost(), updatehost(), delhost();
int inprogress(), failed(), dcm();

/* misc.c */
int TableStats(), ShowClients(), ShowValue(), ShowAlias();

/* utils.c */

/* 
 * For descriptions of all of these functions see the 
 * internals documentation.
 */

void EncryptMITID();		/* Encrypts an mit id number. */
char **CopyInfo();		/* Copies a NULL terminated array of strings */
void FreeInfo();		/* Frees a NULL terminated array of strings. */
void FreeQueue();		/* Frees an entire queue. */
void FreeAndClear();		/* Frees a single pointer. */
void Loop();			/* Performs an operation on every item in
				   a queue. */
void QueryLoop();		/* Queries the user before performing 
				   an operation on every item in
				   a queue. */
void RemoveHyphens();		/* Removes the hyphens from a string. */
void SlipInNewName();		/* Slips a new name into an argument list in
				   the correct place. */
int CountArgs();		/* Counts the strings in a NULL terminated 
				   array. */
int GetValueFromUser();		/* Updates value by asking the user. */
int GetYesNoValueFromUser();	/* Updates a yes/no value by asking the user.*/
int GetFSTypes();		/* Gets Filesystem types from a user. */
int NullFunc();			/* NULL return function. */
int Print();			/* Prints out array of strings. */
int PrintByType();		/* Prints out array of strings by type. */
int PrintHelp();		/* Prints out help information. */
int StoreInfo();		/* Stores info returned from and SMS query
				   in a queue. */
int Scream();			/* Query Return function that yells if called*/
int ToggleVerboseMode();	/* Turns on and off the verbose variable. */
char *CanonicalizeHostname();	/* this is what it does. */
char *NullPrint();		/* Prints nothing, returns argv[0]. */
char *Strsave();		/* allocated space for a copy of the string. */
struct qelem * QueueTop();	/* Finds the top of a queue. */
Bool Confirm();			/* Confirms a delete operation, with user. */
Bool YesNoQuestion();		/* Asks a user a yes no type question. */
Bool YesNoQuitQuestion();	/* Asks a user a yes - no - quit question. */

#endif /* _f_defs_ */		/* Do Not Add Anything After This Line */


