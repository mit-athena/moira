#if (!defined(lint) && !defined(SABER))
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/menus.c,v 1.5 1988-07-08 18:25:50 kit Exp $";
#endif lint

/*	This is the file menu.c for allmaint, the SMS client that allows
 *      a user to maintaint most important parts of the SMS database.
 *	It Contains: all menu definitions for allmaint.
 *	
 *	Created: 	4/11/88
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/menus.c,v $
 *      $Author: kit $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/menus.c,v 1.5 1988-07-08 18:25:50 kit Exp $
 *	
 *  	Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include <stdio.h>
#include <menu.h>

#include "mit-copyright.h"
#include "allmaint.h"
#include "allmaint_funcs.h"
#include "globals.h"

/*
 * Attachmaint Menus
 */

Menu attachmaint_filemenu = {
  NULLFUNC, 
  NULLFUNC, 
  "Filesystem Maintenance",
  4,
  {
    { GetFS, NULLMENU, 2, {
      {"get", "Get Filesystem Name Information"},
      {"name", "Name of Filesystem: "}
    } },
    { AddFS, NULLMENU, 2, {
      {"add", "Add New Filesystem to Database"},
      {"name", "name: "},
    } },
    { ChangeFS, NULLMENU, 2, {
      {"change", "Update Filesystem Information"},
      {"name", "name: "},
    } },
    { DeleteFS, NULLMENU, 2, {
      {"delete", "Delete Filesystem"},
      {"name", "Filesystem Name: "}
    } } 
  }
};

Menu attachmaint_top_menu = {
  NULLFUNC, 
  NULLFUNC, 
  "Attach/Filesystem Maintenance", 
  6,
  {
    SUBMENU("filesystem", "Filesystem Work", &attachmaint_filemenu),
    { GetFSAlias, NULLMENU, 2, {
      {"check", "Check An Association"},
      {"name", "alias name : "}
    } },
    { CreateFSAlias, NULLMENU, 2, {
      {"alias", "Associate with a Filesystem"},
      {"name", "alias name : "},
    } },
    { DeleteFSAlias, NULLMENU, 2, {
      {"unalias", "Disassociate from a Filesystem"},
      {"name", "alias name : "},
    } },
    SIMPLEFUNC("verbose", "Toggle Verbosity of Delete", ToggleVerboseMode),
    SIMPLEFUNC("help", "Help ..", AttachHelp)
  }
};

/*
 * Clustermaint Menus
 */

Menu clustermaint_machine_menu = {
  NULLFUNC,
  NULLFUNC,
  "Machine Maintenence",
  7,
  {
    { ShowMachineInfo, NULLMENU, 2, {
      {"show","Get machine information"},
      {"name","Machine's Name: "}
    } },
    { AddMachine, NULLMENU, 2, {
      {"add","Add a new machine"},
      {"name","Machine's Name: "},
    } },
    { UpdateMachine, NULLMENU, 2, {
      {"update","Update machine information"},
      {"name","Machine's Name: "},
    } },
    { DeleteMachine, NULLMENU, 2, {
      {"delete","Delete this machine"},
      {"name","Machine's Name: "}
    } },
    { MachineToClusterMap, NULLMENU, 3, {
      {"map", "Show Machine to cluster mapping"},
      {"name", "Machine's Name: "},
      {"cluster", "Cluster's Name: "},
    } },
    { AddMachineToCluster, NULLMENU, 3, {
      {"addcluster","Add machines to a clusters"},
      {"name","Machine's Name: "},
      {"cluster", "Cluster's Name: "},
    } },
    { RemoveMachineFromCluster, NULLMENU, 3, {
      {"remcluster", "Remove machines from clusters"},
      {"name","Machine's Name: "},
      {"cluster", "Cluster's Name: "},
    } },
  }
};

Menu clustermaint_cluster_menu = {
  NULLFUNC,
  NULLFUNC,
  "Cluster Information",
  7,
  {
    { ShowClusterInfo, NULLMENU, 2, {
      {"show", "Get cluster information"},
      {"name", "Cluster's Name: "}
    } },
    { MachinesInCluster, NULLMENU, 2, {
      {"machines", "List all machines in a cluster"},
      {"clus", "Cluster's Name: "}
    } },
    { AddCluster, NULLMENU, 2, {
      {"add", "Add a new cluster"},
      {"name","Cluster's Name: "},
    } },
    { UpdateCluster, NULLMENU, 2, {
      {"update", "Update cluster information"},
      {"name","Cluster's Name: "},
    } },
    { DeleteCluster, NULLMENU, 2, {
      {"delete", "Delete this cluster"},
      {"name", "Cluster's Name: "}
    } },
    { AddMachineToCluster, NULLMENU, 3, {
      {"addcluster","Add machines to a clusters"},
      {"name","Machine's Name: "},
      {"cluster", "Cluster's Name: "},
    } },
    { RemoveMachineFromCluster, NULLMENU, 3, {
      {"remcluster", "Remove machines from clusters"},
      {"name","Machine's Name: "},
      {"cluster", "Cluster's Name: "},
    } },
  }
};

Menu clustermaint_data_menu = {
  NULLFUNC,
  NULLFUNC,
  "Cluster Data Maintenence",
  3,
  {
    { ShowClusterData, NULLMENU, 3, {
      {"show", "Show Data on a given Cluster"},
      {"clus", "Cluster's Name: "},
      {"label", "Label Identifing the data: "},
    } },
    { AddClusterData, NULLMENU, 4, {
      {"add", "Add Data to a given Cluster"},
      {"clus", "Cluster Name: "},
      {"label", "Label Identifing the data: "},
      {"data", "The data to be added: "},
    } },
    { RemoveClusterData, NULLMENU, 4, {
      {"delete", "Remove Data to a given Cluster"},
      {"clus", "Cluster's Name: "},
      {"label", "Label Identifing the data: "},
      {"data", "The data to be removed: "},
    } } 
  }
};

Menu clustermaint_top_menu = {
  NULLFUNC,
  NULLFUNC,
  "Cluster Maintenence",
  5,
  {
    SUBMENU("machine", "Work on Machines", &clustermaint_machine_menu),
    SUBMENU("cluster", "Work on Clusters", &clustermaint_cluster_menu),
    SUBMENU("data", "Work with cluster data", &clustermaint_data_menu),
    {MachineToClusterMap, NULLMENU, 3, {
      {"map", "Machine to Cluster Mapping"},
      {"machine", "Machine Name: "},
      {"clus", "Cluster Name: "}
    } },
    SIMPLEFUNC("verbose", "Toggle Delete Confirmation", ToggleVerboseMode),
  }
};

/*
 * Dcmmaint menus
 */

/* Commenting out all DCM maint stuff. 

Menu dcmmaint_service_menu = {
  dcmmaint_service_menu_entry,
  dcmmaint_service_menu_exit,
  NULL,
  5,
  {
    SIMPLEFUNC("show", "Show fields of service", show_service),
    SIMPLEFUNC("interval", "Change the interval field", change_interval),
    SIMPLEFUNC("target_path", "Change the target_path field",
	       change_target_path),
    SIMPLEFUNC("script", "Change the script field", change_script),
    SIMPLEFUNC("dfgen",
	       "Change the date of the latest generation of the db file", 
	       change_dfgen)
  }
};

Menu dcmmaint_host_info_menu = {
  dcmmaint_host_info_menu_entry,
  dcmmaint_host_info_menu_exit,
  NULL,
  8,
  {
    SIMPLEFUNC("show", "Show values of entry", show_host_info),
    SIMPLEFUNC("last_time", "Change the last_time field", change_last_time),
    SIMPLEFUNC("success", "Change the success field", change_success),
    SIMPLEFUNC("override", "Change the override field", change_override),
    SIMPLEFUNC("enable", "Change the enable field", change_enable),
    SIMPLEFUNC("value1", "Change the value1 field", change_value1),
    SIMPLEFUNC("value2", "Change the value2 field", change_value2),
    SIMPLEFUNC("rvdacls", "Change the RVD ACLs (for RVD servers only)",
	       update_rvd_server)
  }
};

Menu dcmmaint_top_menu = {
  NULLFUNC,
  NULLFUNC,
  "Data Control Manager Maintenance",
  9,
  {
    SUBMENU("hi_change", "Modify a service-host entry", 
	    &dcmmaint_host_info_menu),
    SIMPLEFUNC("hi_add", "Create new service-host entry", do_host_info),
    SIMPLEFUNC("hi_delete", "Remove a service-host entry", do_host_info),
    SIMPLEFUNC("hi_list", "List service-host entries", do_host_info),
    SUBMENU("s_change", "Modify info for an existing service",
	    &dcmmaint_service_menu),
    SIMPLEFUNC("s_add", "Create new service", do_service),
    SIMPLEFUNC("s_delete", "Remove a service", do_service),
    SIMPLEFUNC("s_list", "List services", do_service),
    SIMPLEFUNC("help", "How to use this program", instructions)
  }
};

Commenting out all DCM maint stuff. */

/*
 * Listmaint Menus 
 */

Menu listmaint_list_menu = {
    NULLFUNC,
    NULLFUNC,
    "Lists",
    5,
    {
        SIMPLEFUNC("member",
		   "List all lists to which a given member belongs",
		   ListByMember),
	SIMPLEFUNC("admin",
		   "List all items which a given member can administer",
		   ListByAdministrator),
	SIMPLEFUNC("groups", "List all lists which are groups",
		   ListAllGroups),
	SIMPLEFUNC("public", "List all public mailing lists",
		   ListAllPublicMailLists),
	SIMPLEFUNC("maillists", "List all mailing lists",
		   ListAllMailLists)
    }
};

Menu listmaint_member_menu = {
    ListmaintMemberMenuEntry,
    ListmaintMemberMenuExit,
    NULL,
    7,
    {
	SIMPLEFUNC("add", "Add a member to this list",
		   AddMember),
	SIMPLEFUNC("remove", "Remove a member from this list",
		   DeleteMember),
	SIMPLEFUNC("all", "Show the members of this list",
		   ListAllMembers),
	SIMPLEFUNC("user", "Show the members of type USER",
		   ListUserMembers),
	SIMPLEFUNC("list", "Show the members of type LIST",
		   ListListMembers),
	SIMPLEFUNC("string", "Show the members of type STRING",
		   ListStringMembers),
	SIMPLEFUNC("verbose", "Toggle Verbosity of Delete", 
		   ToggleVerboseMode)
    } 
};
    
Menu listmaint_top_menu = {
    NULLFUNC,
    NULLFUNC,
    "List Maintenance Menu",
    8,
    {
	{ ShowListInfo, NULLMENU, 2, {
	    {"show", "Display information about a list"},
	    {"list name", "Name of list: "}
	} },
        { NULLFUNC, &listmaint_member_menu, 2, {
	    {"add", "Create new List"},
	    {"list name", "Name of list: "}
	} },
	{ UpdateList, NULLMENU, 2, {
	    {"update", "Update characteristics of a list"},
	    {"list name", "Name of list: "}
	} },
	{ DeleteList, NULLMENU, 2, {
	    {"delete", "Delete a List"},
	    {"list name", "Name of list: "}
	} },
	{ NULLFUNC, &listmaint_member_menu, 2, {
	    {"members", "Change/Display the membership of a list"},
	    {"list name", "Name of list: "}
	} },
	SIMPLEFUNC("query_remove",
		   "Interactively remove an item from all lists",
		   InterRemoveItemFromLists),
	SUBMENU("list_lists", "Display inventory of lists",
		&listmaint_list_menu),
	SIMPLEFUNC("verbose", "Toggle Verbosity of Delete", 
		   ToggleVerboseMode),
	SIMPLEFUNC("help", "How to use this program", ListHelp)
    } 
};

/*
 * NFSmaint menus.
 */

Menu nfsmaint_top_menu = {
  NULLFUNC,
  NULLFUNC,
  "NFS Maintenence",
  5,
  {
    { ShowNFSService, NULLMENU, 2, {
      { "show", "Show an NFS server" },
      { "machine", "Machine Name: "},
    } },
    { AddNFSService, NULLMENU, 2, {
      { "add", "Add NFS server" },
      { "machine", "Machine Name: "},
    } },
    { UpdateNFSService, NULLMENU, 2, {
      { "update", "Update NFS server"},
      { "machine", "Machine Name: "},
    } },
    { DeleteNFSService, NULLMENU, 2, {
      { "delete", "Delete NFS server"},
      {"machine", "Machine Name: "},
    } },
    SIMPLEFUNC("verbose", "Toggle Verbosity of Delete", ToggleVerboseMode),
/*    SIMPLEFUNC("help", "Help", NFSHelp), */
  }
};

/*
 * Portmaint menus.
 */

/* Queries have not been implemented yet :-( 

Menu servmenu = {
  NULLFUNC,
  NULLFUNC,
  "Server Maintenence",
  8,
  {
    { getserv, NULLMENU, 2, {
      {"get", "Services Info"},
      {"serv", "Service Name: "}
    } },
    { addserv, NULLMENU, 4, {
      {"add", "Add Service"},
      {"srv", "Name: "},
      {"proto", "Protocol: "},
      {"port", "Port: "}
    } },
    { updserv, NULLMENU, 4, {
      {"update", "Update Service"},
      {"srv", "Name: "},
      {"proto", "Protocol: "},
      {"port", "Port: "}
    } },
    {delserv, NULLMENU, 3, {
      {"delete", "Delete Service"},
      {"serv", "Service Name: "},
      {"proto", "Protocol: "}
    } },
    {  getassc, NULLMENU, 2, {
      {"get_alias", "Get All Aliases for Service"},
      {"srv", "Service Name: "}
    } },
    { alas, NULLMENU, 3, {
      {"+", "Add Alias"},
      {"srv", "Service Name: "},
      {"als", "Alias: "}
    } },
    { alas, NULLMENU, 3, {
      {"-", "Delete Alias"},
      {"srv", "Service Name: "},
      {"als", "Alias: "}
    } },
    SIMPLEFUNC("toggle", "Toggle Verbosity of Delete", toggle_verb)
  }
};

*/
/*
 * smsmaint - TBS
 */

/* 
 * Usermaint menu.
 */

Menu usermaint_pobox_menu = {
  NULLFUNC,
  NULLFUNC,
  "Edit User Post Office Boxes",
  3,
  {
    {GetUserPOBox, NULLMENU, 2, {
      {"show", "Show a user's post office box"},
      {"login name", "login name: "}
    } },
    {SetUserPOBox, NULLMENU, 2, {
      {"set", "Set (Add or Change) a user's post office box"},
      {"login name", "login name: "}
    } },
    {RemoveUserPOBox, NULLMENU, 2, {
      {"remove", "Remove a user's post office box"},
      {"login name", "login name: "}
    } },
  }
};
    
Menu usermaint_quota_menu = {
  NULLFUNC,
  NULLFUNC,
  "Edit User and Default Quotas",
  6,
  {
    SIMPLEFUNC("shdef", "Show default user quota (in KB)", ShowDefaultQuota),
    {ChangeDefaultQuota, NULLMENU, 2, {
      {"chdef", "Change default user quota"},
      {"quota", "New quota (in KB): "}
    } },
    SIMPLEFUNC("shquota", "Show a user's disk quota on a filesytem",
	       ShowUserQuota),
    SIMPLEFUNC("addquota", "Add a new disk quota for user on a filesytem",
	       AddUserQuota),
    SIMPLEFUNC("chquota", "Change a user's disk quota on a filesytem",
	       ChangeUserQuota),
    SIMPLEFUNC("rmquota", "Remove a user's disk quota on a filesytem",
	       RemoveUserQuota),
  } 
};

Menu usermaint_top_menu = {
  NULLFUNC,
  NULLFUNC,
  "SMS User Maintenance",
  9,
  {
    {ShowUserByLogin, NULLMENU, 2, {
       {"login", "Show user information by login name"},
       {"login name", "Desired login name: "}
     } },
    {ShowUserByName, NULLMENU, 3, {
      {"name", "Show user information by name"},
      {"first", "First name: "},
      {"last", "Last name: "}
    } },
    {ShowUserByClass, NULLMENU, 2, {
       {"class", "Show names of users in a given class"},
       {"login name", "Desired class: "}
     } },    
    {UpdateUser, NULLMENU, 2, {
      {"modify", "Change all user fields"},
      {"login", "Login name: "}
    } },
/*    SIMPLEFUNC("chpw", "Change a user's password", change_user_password), */
    SIMPLEFUNC("adduser", "Add a new user to the database", 
	       AddNewUser),
    SIMPLEFUNC("register", "Register a user", RegisterUser),
    {DeleteUser, NULLMENU, 2, {
      {"delete", "Delete user"},
      {"login", "Login name: "}
    } },
    {DeleteUserByUid, NULLMENU, 2, {
      {"udelete", "Delete user by uid"},
      {"uid", "User ID (not MIT ID!): "}
    } },
    SUBMENU("pobox", "User PO Boxes", &usermaint_pobox_menu),
    SUBMENU("quota", "User Quotas", &usermaint_quota_menu),
  }
};

/* 
 * Sms top menu
 */

Menu sms_top_menu = {
  NULLFUNC,
  NULLFUNC,
  "Sms Database Manipulation",
  5,
  {
    SUBMENU("attachmaint","open the attachmaint menu",&attachmaint_top_menu),
    SUBMENU("clustermaint","open the clustermaint menu",
	    &clustermaint_top_menu),
/*    SUBMENU("dcmmaint","open the dcmmaint menu",&dcmmaint_top_menu), */
    SUBMENU("listmaint","open the listmaint menu",&listmaint_top_menu),
    SUBMENU("nfsmaint","open the nfsmaint menu",&nfsmaint_top_menu),
/*    SUBMENU("portmaint","open the portmaint menu",&portmaint_top_menu), */
/*    SUBMENU("smsmaint","open the smsmaint menu",&smsmaint_top_menu), */
    SUBMENU("usermaint","open the usermaint menu",&usermaint_top_menu)
  }
};  
       
