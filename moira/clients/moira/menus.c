/* $Id: menus.c,v 1.40 2000-01-28 00:31:53 danw Exp $
 *
 *	This is the file menus.c for the Moira Client, which allows users
 *      to quickly and easily maintain most parts of the Moira database.
 *	It Contains: All menu definitions.
 *
 *	Created: 	4/11/88
 *	By:		Chris D. Peterson
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <stdio.h>

#include "mit-copyright.h"
#include <moira.h>
#include "defs.h"
#include "f_defs.h"
#include "globals.h"

#include <stdio.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/menus.c,v 1.40 2000-01-28 00:31:53 danw Exp $");

/* ------------------------- Second Tier Menus ------------------------- */

/*
 * Cluster Data Menu
 */

Menu cluster_data_menu = {
  NULLFUNC,
  NULLFUNC,
  "Cluster Data Menu",
  4,
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
    { RemoveClusterData, NULLMENU, 3, {
      {"delete", "Remove Data from a given Cluster"},
      {"clus", "Cluster's Name: "},
      {"label", "Label Identifing the data: "},
    } },
    SIMPLEFUNC("verbose", "Toggle Verbosity of Delete", ToggleVerboseMode)
  }
};

/*
 * List Information Menu
 */

Menu list_info_menu = {
  NULLFUNC,
  NULLFUNC,
  "List Information Menu",
  3,
  {
    SIMPLEFUNC("member",
	       "Show all lists to which a given member belongs",
	       ListByMember),
    SIMPLEFUNC("admin",
	       "Show all items which a given member can administer",
	       ListByAdministrator),
    SIMPLEFUNC("public", "Show all public mailing lists",
	       ListAllPublicMailLists),
  }
};

/*
 * List Member Tags Menu
 */

Menu list_tags_menu = {
  NULL,
  NULL,
  "List Member Tags Menu",
  2,
  {
    SIMPLEFUNC("change", "Change the tag on a list member", TagMember),
    SIMPLEFUNC("show", "Show all list members with tags", ListMembersWithTags),
  }
};

/*
 * List Member Menu
 */

Menu list_member_menu = {
  ListmaintMemberMenuEntry,
  ListmaintMemberMenuExit,
  NULL,
  8,
  {
    SIMPLEFUNC("add", "Add a member to this list", AddMember),
    SIMPLEFUNC("remove", "Remove a member from this list", DeleteMember),
    SIMPLEFUNC("all", "Show the members of this list", ListAllMembers),
    SIMPLEFUNC("user", "Show the members of type USER", ListUserMembers),
    SIMPLEFUNC("list", "Show the members of type LIST", ListListMembers),
    SIMPLEFUNC("string", "Show the members of type STRING",
	       ListStringMembers),
    SUBMENU("tags", "List Member Tags Menu", &list_tags_menu),
    SIMPLEFUNC("verbose", "Toggle Verbosity of Delete", ToggleVerboseMode)
  }
};

/*
 *  Machine To Cluster Mappings Menu
 */

Menu mappings_menu = {
  NULLFUNC,
  NULLFUNC,
  "Machine To Cluster Mappings Menu",
  4,
  {
    { MachineToClusterMap, NULLMENU, 3, {
      {"map", "Show Machine to cluster mapping"},
      {"name", "Machine's Name: "},
      {"cluster", "Cluster's Name: "},
    } },
    { AddMachineToCluster, NULLMENU, 3, {
      {"addcluster", "Add machine to a cluster"},
      {"name", "Machine's Name: "},
      {"cluster", "Cluster's Name: "},
    } },
    { RemoveMachineFromCluster, NULLMENU, 3, {
      {"remcluster", "Remove machines from clusters"},
      {"name", "Machine's Name: "},
      {"cluster", "Cluster's Name: "},
    } },
    SIMPLEFUNC("verbose", "Toggle Verbosity of Delete", ToggleVerboseMode),
  }
};

/*
 * Post Office Box Menu
 */

Menu pobox_menu = {
  NULLFUNC,
  NULLFUNC,
  "Post Office Box Menu",
  5,
  {
    {GetUserPOBox, NULLMENU, 2, {
      {"show", "Show a user's post office box"},
      {"login name", "login name: "}
    } },
    {SetUserPOBox, NULLMENU, 2, {
      {"set", "Set (Add or Change) a user's post office box"},
      {"login name", "login name: "}
    } },
    {SplitUserPOBox, NULLMENU, 2, {
      {"split", "Split a user's post office box"},
      {"login name", "login name: "}
    } },
    {RemoveUserPOBox, NULLMENU, 2, {
      {"remove", "Remove a user's post office box"},
      {"login name", "login name: "}
    } },
    SIMPLEFUNC("verbose", "Toggle Verbosity of Delete", ToggleVerboseMode)
  }
};

/*
 * Quota Menu
 */

Menu quota_menu = {
  NULLFUNC,
  NULLFUNC,
  "Quota Menu",
  8,
  {
    SIMPLEFUNC("shdef", "Show default user quota (in KB)", ShowDefaultQuota),
    {ChangeDefaultQuota, NULLMENU, 2, {
      {"chdef", "Change default user quota"},
      {"quota", "New quota (in KB): "}
    } },
    SIMPLEFUNC("getquota", "Show a quota on a filesystem",
	       GetQuota),
    SIMPLEFUNC("getfsquotas", "Show all quotas on a filesystem",
	       GetQuotaByFilesys),
    SIMPLEFUNC("addquota", "Add a new disk quota on a filesystem",
	       AddQuota),
    SIMPLEFUNC("upquota", "Change a quota on a filesystem",
	       UpdateQuota),
    SIMPLEFUNC("delquota", "Delete a quota on a filesystem",
	       DeleteQuota),
    SIMPLEFUNC("verbose", "Toggle Verbosity of Delete", ToggleVerboseMode)
  }
};

Menu fsgroup_menu = {
  NULLFUNC,
  NULLFUNC,
  "Filesystem Group Menu",
  6,
  {
    { GetFS, NULLMENU, 2, {
      {"get", "Get individual or group filesystem information"},
      {"name", "Name of Filesystem: "}
    } },
    { AddFSToGroup, NULLMENU, 3, {
      {"add", "Add a new filesystem to an FS group"},
      {"group", "FS Group name: "},
      {"filsys", "Filesystem name: "}
    } },
    { RemoveFSFromGroup, NULLMENU, 3, {
      {"remove", "Remove a filesystem from an FS group"},
      {"group", "FS Group name: "},
      {"filsys", "Filesystem name: "}
    } },
    { ChangeFSGroupOrder, NULLMENU, 2, {
      {"order", "Change the order of filesystems in a group"},
      {"group", "FS Group name: "}
    } },
    SIMPLEFUNC("help", "Info on Filesystem Groups", FSGroupHelp),
    SIMPLEFUNC("verbose", "Toggle Verbosity of Delete", ToggleVerboseMode)
  }
};

Menu service_menu = {
  NULLFUNC,
  NULLFUNC,
  "DCM Service Menu",
  6,
  {
    { showserv, NULLMENU, 2, {
      { "showserv", "Show service information" },
      { "service name", "Name of service: " },
    } },
    { addserv, NULLMENU, 2, {
      { "addserv", "Add a new service" },
      { "service name", "Name of service: " },
    } },
    { updateserv, NULLMENU, 2, {
      { "updateserv", "Update service information" },
      { "service name", "Name of service: " },
    } },
    { resetsrverr, NULLMENU, 2, {
      { "resetsrverr", "Reset service error" },
      { "service name", "Name of service: " },
    } },
    { resetsrvc, NULLMENU, 2, {
      { "resetsrvc", "Reset service state" },
      { "service name", "Name of service: " },
    } },
    { delserv, NULLMENU, 2, {
      { "delserv", "Delete service info" },
      { "service name", "Name of service: " },
    } },
  }
};

Menu host_menu = {
  NULLFUNC,
  NULLFUNC,
  "DCM Host Menu",
  7,
  {
    { showhost, NULLMENU, 3, {
      { "showhost", "Show service/host tuple information" },
      { "service name", "Name of service: " },
      { "host name", "Name of machine: " },
    } },
    { addhost, NULLMENU, 3, {
      { "addhost", "Add a new service/host tuple" },
      { "service name", "Name of service: " },
      { "host name", "Name of machine: " },
    } },
    { updatehost, NULLMENU, 3, {
      { "updatehost", "Update a service/host tuple" },
      { "service name", "Name of service: " },
      { "host name", "Name of machine: " },
    } },
    { resethosterr, NULLMENU, 3, {
      { "resethosterr", "Reset service/host error" },
      { "service name", "Name of service: " },
      { "host name", "Name of machine: " },
    } },
    { resethost, NULLMENU, 3, {
      { "resethost", "Reset service/host state" },
      { "service name", "Name of service: " },
      { "host name", "Name of machine: " },
    } },
    { sethostor, NULLMENU, 3, {
      { "override", "Set service/host override" },
      { "service name", "Name of service: " },
      { "host name", "Name of machine: " },
    } },
    { delhost, NULLMENU, 3, {
      { "delhost", "Delete service/host tuple" },
      { "service name", "Name of service: " },
      { "host name", "Name of machine: " },
    } },
  }
};

/* ------------------------- First Tier Menus ------------------------- */

/*
 * Cluster Menu
 */

Menu cluster_menu = {
  NULLFUNC,
  NULLFUNC,
  "Cluster Menu",
  7,
  {
    { ShowClusterInfo, NULLMENU, 2, {
      {"show", "Get cluster information"},
      {"name", "Cluster's Name: "}
    } },
    { AddCluster, NULLMENU, 2, {
      {"add", "Add a new cluster"},
      {"name", "Cluster's Name: "},
    } },
    { UpdateCluster, NULLMENU, 2, {
      {"update", "Update cluster information"},
      {"name", "Cluster's Name: "},
    } },
    { DeleteCluster, NULLMENU, 2, {
      {"delete", "Delete this cluster"},
      {"name", "Cluster's Name: "}
    } },
    SUBMENU("mappings", "Machine To Cluster Mappings Menu", &mappings_menu),
    SUBMENU("c_data", "Cluster Data Menu", &cluster_data_menu),
    SIMPLEFUNC("verbose", "Toggle Verbosity of Delete", ToggleVerboseMode)
  }
};

/*
 * Filesystem Menu
 */

Menu filesys_menu = {
  NULLFUNC,
  NULLFUNC,
  "Filesystem Menu",
  10,
  {
    { GetFS, NULLMENU, 2, {
      {"get", "Get Filesystem Name Information"},
      {"name", "Name of Filesystem: "}
    } },
    { GetFSM, NULLMENU, 2, {
      {"getmach", "Get Filesystems by Machine"},
      {"name", "Name of Server: "}
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
    } },
    SUBMENU("fsgroups", "File System Groups Menu", &fsgroup_menu),
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
    SUBMENU("quotas", "Quota Menu", &quota_menu),
/*    SIMPLEFUNC("verbose", "Toggle Verbosity of Delete", ToggleVerboseMode), */
/*    SIMPLEFUNC("help", "Help ..", AttachHelp) */
  }
};

Menu subnet_menu = {
  NULLFUNC,
  NULLFUNC,
  "Network Menu",
  5,
  {
    { ShowSubnetInfo, NULLMENU, 2, {
      {"show", "Get network information"},
      {"name", "Network's Name: "}
    } },
    { AddSubnet, NULLMENU, 2, {
      {"add", "Add a new network"},
      {"name", "Network's Name: "},
    } },
    { UpdateSubnet, NULLMENU, 2, {
      {"update", "Update network information"},
      {"name", "Network's Name: "},
    } },
    { DeleteSubnet, NULLMENU, 2, {
      {"delete", "Delete this network"},
      {"name", "Network's Name: "}
    } },
    SIMPLEFUNC("verbose", "Toggle Verbosity of Delete", ToggleVerboseMode)
  }
};

Menu cname_menu = {
  NULLFUNC,
  NULLFUNC,
  "Host Alias Menu",
  4,
  {
    { ShowCname, NULLMENU, 3, {
      {"show", "Get host alias information"},
      {"alias", "Alias Name (Cname): "},
      {"host", "Canonical Host Name: "}
    } },
    { AddCname, NULLMENU, 3, {
      {"add", "Add a new host alias"},
      {"alias", "Alias Name (Cname): "},
      {"host", "Canonical Host Name: "}
    } },
    { DeleteCname, NULLMENU, 3, {
      {"delete", "Delete this alias"},
      {"alias", "Alias Name (Cname): "},
      {"host", "Canonical Host Name: "}
    } },
    SIMPLEFUNC("verbose", "Toggle Verbosity of Delete", ToggleVerboseMode)
  }
};

/*
 * Machine Menu
 */

Menu machine_menu = {
  NULLFUNC,
  NULLFUNC,
  "Machine Menu",
  7,
  {
    { ShowMachineInfo, NULLMENU, 2, {
      {"show", "Get machine information"},
      {"name", "Machine's Name: "}
    } },
    { ShowMachineQuery, NULLMENU, 5, {
      {"query", "Lookup machines"},
      {"name", "Machine Name (or leave empty to match any): "},
      {"address", "Address in dotted notation (or leave empty to match any): "},
      {"location", "Location (or leave empty to match any): "},
      {"network", "Network (or leave empty to match any): "}
    } },
    { AddMachine, NULLMENU, 2, {
      {"add", "Add a new machine"},
      {"network", "Network assignment: "},
    } },
    { UpdateMachine, NULLMENU, 2, {
      {"update", "Update machine information"},
      {"name", "Machine's Name: "},
    } },
    { DeleteMachine, NULLMENU, 2, {
      {"delete", "Delete this machine"},
      {"name", "Machine's Name: "}
    } },
    SUBMENU("cnames", "Alias names for machines", &cname_menu),
    SUBMENU("mappings", "Machine To Cluster Mappings Menu", &mappings_menu),
  }
};

/*
 * List Menu
 */

Menu list_menu = {
  NULLFUNC,
  NULLFUNC,
  "List Menu",
  9,
  {
    { ShowListInfo, NULLMENU, 2, {
      {"show", "Display information about a list"},
      {"list name", "Name of list: "}
    } },
    { NULLFUNC, &list_member_menu, 2, {
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
    SIMPLEFUNC("query_remove",
	       "Interactively remove an item from all lists",
	       InterRemoveItemFromLists),
    { NULLFUNC, &list_member_menu, 2, {
      {"members", "Member Menu - Change/Show Members of a List."},
      {"list name", "Name of list: "}
    } },
    SUBMENU("list_info", "List Info Menu",
	    &list_info_menu),
    SUBMENU("quotas", "Quota Menu", &quota_menu),
/*	SIMPLEFUNC("verbose", "Toggle Verbosity of Delete", ToggleVerboseMode), */
    SIMPLEFUNC("help", "Print Help", ListHelp)
  }
};

/*
 * NFS Physical Menu
 */

Menu nfsphys_menu = {
  NULLFUNC,
  NULLFUNC,
  "NFS Physical Menu",
  6,
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
    SUBMENU("quotas", "Quota Menu", &quota_menu),
    SIMPLEFUNC("verbose", "Toggle Verbosity of Delete", ToggleVerboseMode),
/*    SIMPLEFUNC("help", "Help", NFSHelp), */
  }
};

Menu krbmap_menu = {
  NULLFUNC,
  NULLFUNC,
  "User Kerberos Mapping Menu",
  3,
  {
    { GetKrbmap, NULLMENU, 3, {
      { "show", "Show Kerberos Mappings" },
      { "user", "User login name: " },
      { "principal", "Principal: " },
    } },
    { AddKrbmap, NULLMENU, 3, {
      { "add", "Add Kerberos Mapping" },
      { "user", "User login name: " },
      { "principal", "Principal (this is case sensitive): " },
    } },
    { DeleteKrbmap, NULLMENU, 3, {
      { "delete", "Delete Kerberos Mapping" },
      { "user", "User login name: " },
      { "principal", "Principal: " },
    } },
  }
};

/*
 * User Menu
 */

Menu user_menu = {
  NULLFUNC,
  NULLFUNC,
  "User Menu",
  11,
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
    {ShowUserById, NULLMENU, 2, {
      {"id", "Show user information by ID number"},
      {"ID number", "ID number: "}
    } },
    {UpdateUser, NULLMENU, 2, {
      {"modify", "Change all user fields"},
      {"login", "Login name: "}
    } },
    SIMPLEFUNC("adduser", "Add a new user to the database",
	       AddNewUser),
    SIMPLEFUNC("register", "Register a user", RegisterUser),
    {DeactivateUser, NULLMENU, 2, {
      {"deactivate", "Deactivate user"},
      {"login", "Login name: "},
    } },
    {DeleteUser, NULLMENU, 2, {
      {"expunge", "Expunge user"},
      {"login", "Login name: "}
    } },
    SUBMENU("pobox", "Post Office Box Menu", &pobox_menu),
    SUBMENU("krbmap", "User Kerberos Mappings", &krbmap_menu),
  }
};

Menu dcm_menu = {
  NULLFUNC,
  NULLFUNC,
  "DCM Menu",
  6,
  {
    SIMPLEFUNC("enable", "Enable/disable DCM", EnableDcm),
    SUBMENU("service", "DCM Service Menu", &service_menu),
    SUBMENU("host", "DCM Host Menu", &host_menu),
    SIMPLEFUNC("active", "Display entries currently being updated",
	       InProgress),
    SIMPLEFUNC("failed", "Display entries with errors to be reset", DcmFailed),
    SIMPLEFUNC("dcm", "Invoke a DCM update now", Dcm),
    }
};

Menu printserver_menu = {
  NULLFUNC,
  NULLFUNC,
  "Print Server Menu",
  4,
  {
    { GetPrintSrv, NULLMENU, 2, {
      {"get", "Get Print Server Information"},
      {"name", "Name of server: "}
    } },
    { AddPrintSrv, NULLMENU, 2, {
      {"add", "Add a New Print Server"},
      {"name", "Name of server: "}
    } },
    { ChangePrintSrv, NULLMENU, 2, {
      {"change", "Modify a Print Server"},
      {"name", "Name of server: "}
    } },
    { DelPrintSrv, NULLMENU, 2, {
      {"delete", "Delete a Print Server"},
      {"name", "Name of server: "}
    } }
  }
};

/*
 * Printer Menu
 */

Menu printer_menu = {
  NULLFUNC,
  NULLFUNC,
  "Printer Menu",
  11,
  {
    { GetPrn, NULLMENU, 2, {
      {"get", "Get Printer Information"},
      {"name", "Name of Printer: "}
    } },
    { GetPrnByHostname, NULLMENU, 2, {
      {"hostname", "Look up Printer Information by Printer Hostname"},
      {"name", "Hostname of Printer: "}
    } },
    { GetPrnByEthernet, NULLMENU, 2, {
      {"ether", "Look up Printer Information by Ethernet address"},
      {"hwaddr", "Ethernet address of Printer: "}
    } },
    { GetPrnByRM, NULLMENU, 2, {
      {"rm", "Look up Printer Information by Print Spooler"},
      {"name", "Hostname of spooler: "}
    } },
    { GetPrnByLocation, NULLMENU, 2, {
      {"loc", "Look up Printer Information by Location"},
      {"location", "Location: "}
    } },
    { GetPrnByContact, NULLMENU, 2, {
      {"contact", "Look up Printer Information by Contact Info"},
      {"contact", "Contact: "}
    } },
    { AddPrn, NULLMENU, 2, {
      {"add", "Add New Printer to Database"},
      {"name", "name: "},
    } },
    { ChngPrn, NULLMENU, 2, {
      {"change", "Update Printer Information"},
      {"name", "name: "},
    } },
    { DeletePrn, NULLMENU, 2, {
      {"delete", "Delete Printer"},
      {"name", "Printer Name: "}
    } },
    { UpdateHWAddr, NULLMENU, 2, {
      {"hwaddr", "Update Printer Hardware Address"},
      {"name", "Printer hostname: "}
    } },
    SUBMENU("server", "Print Servers Menu", &printserver_menu),
  }
};

/*
 * Zephyr Menu
 */

Menu zephyr_menu = {
  NULLFUNC,
  NULLFUNC,
  "Zephyr Menu",
  5,
  {
    { GetZephyr, NULLMENU, 2, {
      {"get", "Get Zephyr Class Information"},
      {"name", "Name of class: "}
    } },
    { AddZephyr, NULLMENU, 2, {
      {"add", "Add New Zephyr class restrictions"},
      {"name", "Class name: "},
    } },
    { ChngZephyr, NULLMENU, 2, {
      {"change", "Update Zephyr class restrictions"},
      {"name", "Class name: "},
    } },
    { DeleteZephyr, NULLMENU, 2, {
      {"delete", "Delete Zephyr class restriction"},
      {"name", "Class Name: "}
    } },
    { NULLFUNC, &list_member_menu, 2, {
      {"members", "Member Menu - Change/Show Members of a List."},
      {"list name", "Name of list: "}
    } },
  }
};

/*
 * ACL Menu
 */

Menu acl_menu = {
  NULLFUNC,
  NULLFUNC,
  "Generic ACL Menu",
  3,
  {
    { GetACL, NULLMENU, 3, {
      {"get", "Get ACL Information"},
      {"host", "Machine: "},
      {"target", "Target file: "}
    } },
    { AddACL, NULLMENU, 3, {
      {"add", "Add New ACL"},
      {"host", "Machine: "},
      {"target", "Target file: "}
    } },
    { DeleteACL, NULLMENU, 3, {
      {"delete", "Delete ACL"},
      {"host", "Machine: "},
      {"target", "Target file: "}
    } },
  }
};

/*
 * Miscellaneous Menu
 */

Menu misc_menu = {
  NULLFUNC,
  NULLFUNC,
  "Miscellaneous Menu",
  4,
  {
    SIMPLEFUNC("statistics", "Show database statistics", TableStats),
    SIMPLEFUNC("clients", "Show active Moira clients", ShowClients),
    { ShowValue, NULLMENU, 2, {
      {"getval", "Show a database variable value"},
      {"name", "variable name: "},
    } },
    { ShowAlias, NULLMENU, 3, {
      {"getalias", "Show an alias relation"},
      {"name", "Alias name: "},
      {"type", "Alias type: "},
    } },
  }
};


/* ------------------------- Root Menu ------------------------- */

/*
 * Moira top menu
 */

Menu moira_top_menu = {
  NULLFUNC,
  NULLFUNC,
  "Moira Database Manipulation",
  11,
  {
    SUBMENU("cluster", "Cluster Menu", &cluster_menu),
    SUBMENU("filesys", "Filesystem Menu", &filesys_menu),
    SUBMENU("list", "Lists and Group Menu", &list_menu),
    SUBMENU("machine", "Machine Menu", &machine_menu),
    SUBMENU("network", "Network Menu", &subnet_menu),
    SUBMENU("nfs", "NFS Physical Menu", &nfsphys_menu),
    SUBMENU("printer", "Printer Menu", &printer_menu),
    SUBMENU("user", "User Menu", &user_menu),
    SUBMENU("zephyr", "Zephyr ACLS Menu", &zephyr_menu),
    SUBMENU("dcm", "DCM Menu", &dcm_menu),
    SUBMENU("acl", "Generic ACL Menu", &acl_menu),
    SUBMENU("misc", "Miscellaneous Menu", &misc_menu)
  }
};
