/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/moira_cb.c,v 1.1 1991-05-30 22:00:01 mar Exp $
 */

#include <stdio.h>
#include <moira.h>
#include <moira_site.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <Xm/Xm.h>
#include "mmoira.h"


/******* temporary ********/
char *atot(s) char *s; { return s; }



int DisplayCallback(argc, info, form)
int argc;
char **info;
EntryForm *form;
{
    char buf[1024], name[128];
    char *MOD_FORMAT = "Modified by %s at %s with %s\n";
    int i;

    switch (form->menu->operation) {
    case MM_SHOW_USER:
       	sprintf(name, "%s, %s %s", info[U_LAST], info[U_FIRST], info[U_MIDDLE]);
	sprintf(buf, "Login name: %-20s Full name: %s\n", info[U_NAME], name);
	add_to_display(buf);
	sprintf(buf, "User id: %-23s Login shell %-10s Class: %s\n", 
		info[U_UID], info[U_SHELL], info[U_CLASS]);
	add_to_display(buf);
	sprintf(buf, "Account is: %-20s Encrypted MIT ID number: %s\n",
		user_states[atoi(info[U_STATE])], info[U_MITID]);
	add_to_display(buf);
	sprintf(buf, MOD_FORMAT,
		info[U_MODBY], info[U_MODTIME],info[U_MODWITH]);
	add_to_display(buf);
	break;
    case MM_SHOW_FINGER:
	sprintf(buf, "Nickname: %-29s Department: %s\n", info[F_NICKNAME],
		info[F_MIT_DEPT]);
	add_to_display(buf);
	sprintf(buf, "finger %s =>\n", info[F_NAME]);
	add_to_display(buf);
	sprintf(buf, "Login name: %-27s In real life: %s\n", info[F_NAME],
		info[F_FULLNAME]);
	add_to_display(buf);
	sprintf(buf, "Home: %-33s Home phone: %s\n", info[F_HOME_ADDR],
		info[F_HOME_PHONE]);
	add_to_display(buf);
	sprintf(buf, "Office: %-31s Office phone: %s\n", info[F_OFFICE_ADDR],
		info[F_OFFICE_PHONE]);
	add_to_display(buf);
	break;
    case MM_SHOW_POBOX:
	sprintf(buf, "Address: %-10s Box: %-35s Type: %s\n", info[PO_NAME],
		info[PO_BOX], info[PO_TYPE]);
	add_to_display(buf);
	sprintf(buf, MOD_FORMAT, info[4], info[3], info[5]);
	add_to_display(buf);
	break;
    case MM_SHOW_KRBMAP:
	sprintf(buf, "User: %-9s Principal: %s\n",
		info[KMAP_USER], info[KMAP_PRINCIPAL]);
	add_to_display(buf);
	break;
    case MM_SHOW_FILSYS:
    case MM_SHOW_FSGROUP:
	if (!strcmp(info[FS_TYPE], "FSGROUP")) {
	    int stat;
/*	    struct qelem *elem = NULL; */

	    sprintf(buf,"%20s Filesystem Group: %s\n", " ", info[FS_NAME]);
	    add_to_display(buf);

	    sprintf(buf,"Comments: %s\n",info[FS_COMMENTS]);
	    add_to_display(buf);
	    sprintf(buf, MOD_FORMAT, info[FS_MODBY], info[FS_MODTIME], 
		    info[FS_MODWITH]);
	    add_to_display(buf);
	    add_to_display("Containing the filesystems (in order):");
/*
	    if ((stat = do_moira_query("get_fsgroup_members", 1, &info[FS_NAME],
				    StoreInfo, (char *)&elem)) != 0) {
		if (stat == MR_NO_MATCH)
		  add_to_display("    [no members]");
		else
		  com_err(program_name, stat, NULL);
	    } else {
		fsgCount = 1;
		Loop(QueueTop(elem), (void *) PrintFSGMembers);
		FreeQueue(elem);
	    }
*/
	} else {
	    sprintf(buf,"%20s Filesystem: %s\n", " ", info[FS_NAME]);
	    add_to_display(buf);
	    sprintf(buf,"Type: %-40s Machine: %-15s\n",
		    info[FS_TYPE], info[FS_MACHINE]);
	    add_to_display(buf);
	    sprintf(buf,"Default Access: %-2s Packname: %-17s Mountpoint %s \n",
		    info[FS_ACCESS], info[FS_PACK], info[FS_M_POINT]);
	    add_to_display(buf);
	    sprintf(buf,"Comments: %s\n",info[FS_COMMENTS]);
	    add_to_display(buf);
	    sprintf(buf, "User Ownership: %-30s Group Ownership: %s\n",
		    info[FS_OWNER], info[FS_OWNERS]);
	    add_to_display(buf);
	    sprintf(buf, "Auto Create: %-34s Locker Type: %s\n",
		    atoi(info[FS_CREATE]) ? "ON" : "OFF", 
		    info[FS_L_TYPE]);
	    add_to_display(buf);
	    sprintf(buf, MOD_FORMAT, info[FS_MODBY], info[FS_MODTIME], 
		    info[FS_MODWITH]);
	    add_to_display(buf);
	}
	break;
    case MM_SHOW_NFS:
	sprintf(buf,"Machine: %-20s Directory: %-15s Device: %s\n",
		info[NFS_NAME], info[NFS_DIR], info[NFS_DEVICE]);
	add_to_display(buf);
    
	sprintf(buf, "Status: %s\n",
		format_filesys_type(atoi(info[NFS_STATUS])));
	add_to_display(buf);
	sprintf(buf, "Quota Allocated: %-17s Size: %s\n",
		info[NFS_ALLOC], info[NFS_SIZE]);
	add_to_display(buf);
	sprintf(buf, MOD_FORMAT, info[NFS_MODBY], info[NFS_MODTIME],
		info[NFS_MODWITH]);
	add_to_display(buf);
	break;
    case MM_SHOW_QUOTA:
	if (!strcmp(info[Q_TYPE], "ANY"))
	  sprintf(buf, "Filesystem: %s\n", info[Q_FILESYS]);
	else
	  sprintf(buf, "Filesystem: %-45s %s %s\n", info[Q_FILESYS],
		  info[Q_TYPE], info[Q_NAME]);
	add_to_display(buf);
	sprintf(buf, "Machine: %-20s Directory: %-15s\n",
		info[Q_MACHINE], info[Q_DIRECTORY]);
	add_to_display(buf);
	sprintf(buf, "Quota: %s\n", info[Q_QUOTA]);
	add_to_display(buf);
	sprintf(buf, MOD_FORMAT, info[Q_MODBY], info[Q_MODTIME], info[Q_MODWITH]);
	add_to_display(buf);
	break;
    case MM_SHOW_LIST:
	(void) sprintf(buf, "%20sList: %s\n", "", info[L_NAME]);
	add_to_display(buf);
	(void) sprintf(buf, "Description: %s\n", info[L_DESC]);
	add_to_display(buf);
	if ( atoi(info[L_MAILLIST]))
	  add_to_display("This list is a mailing list.\n");
	else
	  add_to_display("This list is NOT a mailing list.\n");
	if (atoi(info[L_GROUP])) {
	    (void) sprintf(buf,"This list is a Group and its ID number is %s\n",
			   info[L_GID]);
	    add_to_display(buf);
	} else
	  add_to_display("This list is NOT a Group.\n");
	if (strcmp(info[L_ACE_TYPE],"NONE") == 0)
	  add_to_display("This list has no Administrator, how strange?!\n");
	else {
	    sprintf(buf, "The Administrator of this list is the %s: %s\n",
		    info[L_ACE_TYPE], info[L_ACE_NAME]);
	    add_to_display(buf);
	}
	(void) sprintf(buf, "This list is: %s, %s, and %s\n",
		       atoi(info[L_ACTIVE]) ? "active" : "inactive",
		       atoi(info[L_PUBLIC]) ? "public" : "private",
		       atoi(info[L_HIDDEN]) ? "hidden" : "visible");
	add_to_display(buf);
	sprintf(buf, MOD_FORMAT, info[L_MODBY], info[L_MODTIME], info[L_MODWITH]);
	add_to_display(buf);
	break;
    case MM_SHOW_MACH:
	sprintf(buf, "Machine: %-30s Type: %s\n", info[M_NAME], info[M_TYPE]);
	add_to_display(buf);
	sprintf(buf, MOD_FORMAT, info[M_MODBY], info[M_MODTIME], info[M_MODWITH]);
	add_to_display(buf);
	break;
    case MM_SHOW_CLUSTER:
	sprintf(buf, "Cluster:     %s\n", info[C_NAME]);
	add_to_display(buf);
	sprintf(buf, "Description: %s\n", info[C_DESCRIPT]);
	add_to_display(buf);
	sprintf(buf, "Location:    %s\n", info[C_LOCATION]);
	add_to_display(buf);
	sprintf(buf, MOD_FORMAT, info[C_MODBY], info[C_MODTIME], info[C_MODWITH]);
	add_to_display(buf);
	break;
    case MM_SHOW_CLDATA:
	sprintf(buf, "Cluster: %-20s Label: %-15s Data: %s\n",
		info[CD_NAME], info[CD_LABEL], info[CD_DATA]);
	add_to_display(buf);
	break;
    case MM_SHOW_MEMBERS:
	if (argc == 2)
	  sprintf(buf, "%-9s %s\n", info[0], info[1]);
	else
	  sprintf(buf, "%s\n", info[0]);
	add_to_display(buf);
	break;
    case MM_STATS:
	sprintf(buf, "Table: %-20s Modified: %s\n", info[0], info[5]);
	add_to_display(buf);
	sprintf(buf, "  %-8D appends, %-8d updates, %-8d deletes\n",
		info[2], info[3], info[4]);
	add_to_display(buf);
	break;
    case MM_CLIENTS:
	{
	    unsigned long host_address;
	    struct hostent *host_entry;

	    host_address = inet_addr(info[1]);
	    if (host_address != NULL) {
		host_entry = gethostbyaddr((char *)&host_address, 4, AF_INET);
		if (host_entry != NULL) {
		    free(info[1]);
		    info[1] = strsave(host_entry->h_name);
		}
	    }
	}
	sprintf(buf, "Principal %s on %s (%s)\n", info[0], info[1], info[2]);
	add_to_display(buf);
	sprintf(buf, "  Connected at %s, client %s\n", info[3], info[4]);
	add_to_display(buf);
	break;
    case MM_SHOW_VALUE:
	sprintf(buf, "Variable: %-20s Value: %s\n", stringval(form, 0),
		info[0]);
	add_to_display(buf);
	break;
    case MM_SHOW_ALIAS:
	sprintf(buf, "Alias: %-20s Type: %-8s Value: %s\n",
		info[0], info[1], info[2]);
	add_to_display(buf);
	break;
    case MM_SHOW_HOST:
	sprintf(buf, "%s:%s  mod by %s on %s with %s\n", info[SH_MACHINE],
		info[SH_SERVICE], info[SH_MODBY], info[SH_MODTIME],
		info[SH_MODWITH]);
	Put_message(buf);
	if (atoi(info[SH_HOSTERROR]))
	  sprintf(name, "Error %d: %s", atoi(info[SH_HOSTERROR]),
		  info[SH_ERRMSG]);
	else
	  strcpy(name, "No error");
	sprintf(buf, "  %s/%s/%s/%s/%s\n",
		atoi(info[SH_ENABLE]) ? "Enabled" : "Disabled",
		atoi(info[SH_SUCCESS]) ? "Success" : "Failure",
		atoi(info[SH_INPROGRESS]) ? "InProgress" : "Idle",
		atoi(info[SH_OVERRIDE]) ? "Override" : "Normal", name);
	Put_message(buf);
	Put_message("  Last Try             Last Success         Value1    Value2    Value3\n");
	strcpy(name, atot(info[SH_LASTTRY]));
	sprintf(buf, "  %-20s %-20s %-9d %-9d %s\n", name,
		atot(info[SH_LASTSUCCESS]), atoi(info[SH_VALUE1]),
		atoi(info[SH_VALUE2]), info[SH_VALUE3]);
	Put_message(buf);
	break;
    case MM_SHOW_DQUOTA:
	sprintf(buf, "The default quota is %s Kbytes.\n", info[0]);
	Put_message(buf);
	break;
    case MM_SHOW_DCM:
	if (argc == 2)
	  sprintf(buf, "%s:%s\n", info[0], info[1]);
	else
	  sprintf(buf, "%s\n", info[0]);
	Put_message(buf);
	break;
    default:
	for (i = 0; i < argc; i++) {
	    if (i != 0) add_to_display(", ");
	    add_to_display(info[i]);
	}
	add_to_display("\n");
    }
    return(MR_CONT);
}


/* Called with moira data that is to be modified. */

int ModifyCallback(argc, argv, form)
int argc;
char **argv;
EntryForm *form;
{
    EntryForm *f;
    char *fn;
    int count, i, offset;

    count = -1;
    offset = 0;
    switch (form->menu->operation) {
    case MM_MOD_USER:
	fn = "mod_user";
	count = U_MODTIME;
	break;
    case MM_MOD_FINGER:
	fn = "mod_finger";
	count = F_MODTIME - 1;
	offset = 1;
	break;
    case MM_REGISTER:
	if (form->extrastuff)
	  form->extrastuff = NULL;
	else
	  form->extrastuff = strsave(argv[U_UID]);
	return;
	break;
    case MM_MOD_LIST:
	fn = "mod_list";
	count = L_MODTIME;
	break;
    case MM_MOD_FILSYS:
	fn = "mod_filsys";
	count = FS_MODTIME;
	break;
    }

    if (count > 0) {
	f = GetAndClearForm(fn);
	if (f == NULL) {
	    display_error("Unknown form in ModifyCallback!\n");
	    return;
	}
	f->extrastuff = form->extrastuff;
	f->menu = form->menu;
	for (i = 0; i < count; i++)
	  if (f->inputlines[i]->type == FT_BOOLEAN)
	    f->inputlines[i]->returnvalue.boolean =
	      strcmp(argv[i + offset], "0") ? 1 : 0;
	  else
	    StoreField(f, i, argv[i + offset]);
    } else {
	display_error("Unknown function in ModifyCallback!\n");
	return;
    }
}



/* callback when form is complete to process the data */

MoiraFormComplete(dummy1, form)
int dummy1;
EntryForm *form;
{
    char *qy, *argv[32], buf[256];
    int (*retfunc)(), argc, i;
    EntryForm *f;

    retfunc = DisplayCallback;
    argc = -1;

    for (i = 0; form->inputlines[i]; i++)
      if (form->inputlines[i]->type == FT_BOOLEAN)
	argv[i] = boolval(form, i) ? "1" : "0";
      else
	argv[i] = stringval(form, i);

    switch (form->menu->operation) {
    case MM_SHOW_USER:
	if (*stringval(form, 0)) {
	    qy = "get_user_by_login";
	    argv[0] = stringval(form, 0);
	    argc = 1;
	} else if (*stringval(form, 3)) {
	    qy = "get_user_by_uid";
	    argv[0] = stringval(form, 3);
	    argc = 1;
	} else if (*stringval(form, 4)) {
	    qy = "get_user_by_class";
	    argv[0] = stringval(form, 4);
	    argc = 1;
	} else if (*stringval(form, 1) == 0 &&
		   *stringval(form, 2) == 0) {
	    DisplayError("Must fill in at least one blank.");
	    return;
	} else {
	    if (*stringval(form, 1) == 0)
	      StoreField(form, 1, "*");
	    if (*stringval(form, 2) == 0)
	      StoreField(form, 2, "*");
	    qy = "get_user_by_name";
	    argv[0] = stringval(form, 1);
	    argv[1] = stringval(form, 2);
	    argc = 2;
	}
	break;
    case MM_ADD_USER:
	qy = "add_user";
	argv[U_STATE][1] = 0;
	if (argv[U_MITID][0] == '"')
	  strcpy(argv[U_MITID], stringval(form, U_MITID) + 1,
		 strlen(stringval(form, U_MITID) - 2));
	else
	  EncryptID(argv[U_MITID], argv[U_MITID], argv[U_FIRST], argv[U_LAST]);
	argc = U_MODTIME;
	break;
    case MM_REGISTER:
	if (*stringval(form, 2)) {
	    argv[0] = stringval(form, 2);
	} else {
	    argv[0] = stringval(form, 0);
	    argv[1] = stringval(form, 1);
	    form->extrastuff = NULL;
	    i = MoiraQuery("get_user_by_name", 2, argv,
			   ModifyCallback, (char *)form);
	    if (i) {
		com_err(program_name, i, "looking up user by name");
		return;
	    }
	    if (form->extrastuff == NULL) {
		display_error("Ambiguous user specification");
		return;
	    }
	    argv[0] = (char *)form->extrastuff;
	}
	qy = "register_user";
	argv[1] = stringval(form, 3);
	argv[2] = "1";
	argc = 3;
	break;
    case MM_MOD_USER:
	if (!strcmp(form->formname, "mod_user")) {
	    qy = "update_user";
	    for (i = 0; i < U_MODTIME; i++)
	      argv[i + 1] = stringval(form, i);
	    argv[0] = form->extrastuff;
	    argv[U_STATE + 1][1] = 0;
	    if (argv[U_MITID + 1][0] == '"') {
		strcpy(argv[U_MITID + 1], stringval(form, U_MITID) + 1);
		stringval(form, U_MITID)[strlen(stringval(form, U_MITID))-1] = 0;
	    } else
	      EncryptID(argv[U_MITID + 1], argv[U_MITID + 1],
			argv[U_FIRST + 1], argv[U_LAST + 1]);
	    argc = U_MODTIME + 1;
	    break;
	}
	qy = "get_user_by_login";
	argv[0] = stringval(form, 0);
	argc = 1;
	form->extrastuff = (caddr_t) "mod_user";
	retfunc = ModifyCallback;
	break;
    case MM_DEACTIVATE:
	qy = "update_user_status";
	argv[0] = stringval(form, 0);
	argv[1] = "3";
	argc = 2;
	break;
    case MM_EXPUNGE:
	display_error("Expunge is not yet implemented");
	return;
	break;
    case MM_SHOW_FINGER:
	qy = "get_finger_by_login";
	argc = 1;
	break;
    case MM_MOD_FINGER:
	if (!strcmp(form->formname, "mod_finger")) {
	    qy = "update_finger_by_login";
	    for (i = 0; i < F_MODTIME - 1; i++)
	      argv[i + 1] = stringval(form, i);
	    argv[0] = form->extrastuff;
	    argc = F_MODTIME;
	    break;
	}
	qy = "get_finger_by_login";
	argc = 1;
	form->extrastuff = (caddr_t) "mod_finger";
	retfunc = ModifyCallback;
	break;
    case MM_SHOW_KRBMAP:
	qy = "get_krbmap";
	if (!*stringval(form, 0))
	  argv[0] = "*";
	if (!*stringval(form, 1))
	  argv[1] = "*";
	argc = 2;
	break;
    case MM_ADD_KRBMAP:
	qy = "add_krbmap";
	argc = 2;
	break;
    case MM_DEL_KRBMAP:
	qy = "delete_krbmap";
	argc = 2;
	break;
    case MM_SHOW_POBOX:
	qy = "get_pobox";
	argc = 1;
	break;
    case MM_SET_POBOX:
	qy = "set_pobox";
	if (!strcmp(argv[1], "POP"))
	  argv[2] = stringval(form, 3);
	argc = 3;
	break;
    case MM_RESET_POBOX:
	qy = "set_pobox_pop";
	argc = 1;
	break;
    case MM_DEL_POBOX:
	qy = "set_pobox";
	argv[0] = stringval(form, 0);
	argv[1] = "NONE";
	argv[2] = "";
	argc = 3;
	break;
    case MM_SHOW_LIST:
	qy = "get_list_info";
	argc = 1;
	break;
    case MM_SHOW_ACE_USE:
	qy = "get_ace_use";
	if (boolval(form, 2)) {
	    sprintf(buf, "R%s", stringval(form, 0));
	    argv[0] = buf;
	}
	argc = 2;
	break;
    case MM_ADD_LIST:
	qy = "add_list";
	argc = L_MODTIME;
	break;
    case MM_MOD_LIST:
	if (!strcmp(form->formname, "mod_list")) {
	    qy = "update_list";
	    for (i = 0; i < L_MODTIME; i++)
	      argv[i + 1] = stringval(form, i);
	    argv[0] = form->extrastuff;
	    argc = L_MODTIME + 1;
	    break;
	}
	qy = "get_list_info";
	argc = 1;
	form->extrastuff = (caddr_t) "mod_list";
	retfunc = ModifyCallback;
	break;
    case MM_DEL_LIST:
	qy = "delete_list";
	argc = 1;
	break;
    case MM_SHOW_MEMBERS:
	if (*stringval(form, 0)) {
	    qy = "get_members_of_list";
	    argc = 1;
	} else {
	    qy = "get_lists_of_member";
	    argv[0] = stringval(form, 1);
	    if (boolval(form, 3)) {
		sprintf(buf, "R%s", stringval(form, 1));
		argv[0] = buf;
	    }
	    argv[1] = stringval(form, 2);
	    argc = 2;
	}
	break;
    case MM_ADD_MEMBER:
	qy = "add_member_to_list";
	argc = 3;
	break;
    case MM_DEL_MEMBER:
	qy = "delete_member_from_list";
	argc = 3;
	break;
    case MM_DEL_ALL_MEMBER:
	display_error("Not yet implemented.");
	return;
	break;
    case MM_SHOW_FILSYS:
	if (*stringval(form, 0)) {
	    qy = "get_filesys_by_label";
	    argv[0] = stringval(form, 0);
	    argc = 1;
	} else if (*stringval(form, 1)) {
	    argv[0] = canonicalize_hostname(stringval(form, 1));
	    if (*stringval(form, 2)) {
		qy = "get_filesys_by_nfsphys";
		argv[1] = stringval(form, 2);
		argc = 2;
	    } else {
		qy = "get_filesys_by_machine";
		argc = 1;
	    }
	} else if (*stringval(form, 3)) {
	    qy = "get_filesys_by_group";
	    argv[0] = stringval(form, 3);
	    argc = 1;
	}
	break;
    case MM_ADD_FILSYS:
	qy = "add_filesys";
	argv[FS_MACHINE] = canonicalize_hostname(stringval(form, FS_MACHINE));
	if (!strcmp(stringval(form, FS_TYPE), "AFS") ||
	    !strcmp(stringval(form, FS_TYPE), "FSGROUP"))
	  argv[FS_MACHINE] = "\\[NONE\\]";
	argc = FS_MODTIME;
	break;
    case MM_MOD_FILSYS:
	if (!strcmp(form->formname, "mod_filsys")) {
	    qy = "update_filsys";
	    for (i = 0; i < FS_MODTIME; i++)
	      argv[i + 1] = stringval(form, i);
	    argv[0] = form->extrastuff;
	    argc = FS_MODTIME + 1;
	    break;
	}
	qy = "get_filesys_by_label";
	argv[0] = stringval(form, 0);
	argc = 1;
	form->extrastuff = (caddr_t) "mod_filsys";
	retfunc = ModifyCallback;
	break;
    }

    if (argc == -1) {
	display_error("Unknown function in form callback.\n");
	return;
    }
    i = MoiraQuery(qy, argc, argv, retfunc, (char *)form);
    if (i) {
	com_err(program_name, i, "executing database query");
	return;
    }
    
    f = NULL;
    if (form->extrastuff && (f = GetForm((char *)(form->extrastuff)))) {
	f->extrastuff = (caddr_t) strsave(stringval(form, 0));
	f->menu = form->menu;
    }

    switch (form->menu->operation) {
    case MM_MOD_USER:
	if (f) {
	    qy = strsave(stringval(f, U_MITID));
	    sprintf(stringval(f, U_MITID), "\"%s\"", qy);
	    free(qy);
	    f->inputlines[U_STATE]->keywords = user_states;
	    StoreField(f, U_STATE, user_states[atoi(stringval(f, U_STATE))]);
	    GetKeywords(f, U_CLASS, "class");
	} else
	  add_to_display("Done.\n");
	break;
    case MM_MOD_FINGER:
	if (!f)
	  add_to_display("Done.\n");	  
	break;
    case MM_MOD_LIST:
	if (f)
	  GetKeywords(f, L_ACE_TYPE, "ace_type");
	else
	  add_to_display("Done.\n");	  
	break;
    case MM_MOD_FILSYS:
	if (f) {
	    GetKeywords(f, FS_TYPE, "filesys");
	    sprintf(buf, "fs_access_%s", stringval(f, FS_TYPE));
	    GetKeywords(f, FS_ACCESS, buf);
	    GetKeywords(f, FS_L_TYPE, "lockertype");
	    if (!strcmp(stringval(f, FS_MACHINE), "[NONE]"))
	      StoreField(f, FS_MACHINE, "\\[NONE\\]");
	} else
	  add_to_display("Done.\n");	  
	break;
    case MM_ADD_HOST:
    case MM_DEL_HOST:
    case MM_CLEAR_HOST:
    case MM_RESET_HOST:
    case MM_ADD_SERVICE:
    case MM_DEL_SERVICE:
    case MM_CLEAR_SERVICE:
    case MM_RESET_SERVICE:
    case MM_ENABLE_DCM:
    case MM_TRIGGER_DCM:
    case MM_ADD_ZEPHYR:
    case MM_DEL_ZEPHYR:
    case MM_ADD_PCAP:
    case MM_DEL_PCAP:
    case MM_ADD_CLDATA:
    case MM_DEL_CLDATA:
    case MM_ADD_MCMAP:
    case MM_DEL_MCMAP:
    case MM_ADD_CLUSTER:
    case MM_DEL_CLUSTER:
    case MM_ADD_MACH:
    case MM_DEL_MACH:
    case MM_ADD_MEMBER:
    case MM_DEL_MEMBER:
    case MM_DEL_ALL_MEMBER:
    case MM_ADD_LIST:
    case MM_DEL_LIST:
    case MM_ADD_QUOTA:
    case MM_DEL_QUOTA:
    case MM_SET_DQUOTA:
    case MM_ADD_NFS:
    case MM_DEL_NFS:
    case MM_ADD_FS_ALIAS:
    case MM_DEL_FS_ALIAS:
    case MM_ADD_FSGROUP:
    case MM_MOV_FSGROUP:
    case MM_DEL_FSGROUP:
    case MM_ADD_FILSYS:
    case MM_DEL_FILSYS:
    case MM_ADD_KRBMAP:
    case MM_DEL_KRBMAP:
    case MM_SET_POBOX:
    case MM_DEL_POBOX:
    case MM_ADD_USER:
    case MM_REGISTER:
    case MM_DEACTIVATE:
    case MM_EXPUNGE:
	add_to_display("Done.\n");
    }

    if (f)
      DisplayForm(f);
}


MoiraMenuComplete(m)
MenuItem *m;
{
    char *qy, *argv[32];
    int (*retfunc)(), argc, i;
    EntryForm dummy;

    retfunc = DisplayCallback;
    argc = -1;
    dummy.menu = m;

    switch (m->operation) {
    case MM_SHOW_MAILLIST:
	qy = "qualified_get_lists";
	argv[0] = argv[1] = argv[3] = "TRUE";
	argv[2] = "FALSE";
	argv[4] = "DONTCARE";
	argc = 5;
	break;
    case MM_SHOW_DQUOTA:
	qy = "get_value";
	argv[0] = "def_quota";
	argc = 1;
	break;
    case MM_SHOW_DCM:
	Put_message("Services and Hosts with failed updates:");
	argv[0] = argv[2] = "DONTCARE";
	argv[1] = "TRUE";
	i = MoiraQuery("qualified_get_server", 3, argv, retfunc, NULL);
	if (i && i != MR_NO_MATCH)
	  com_err(program_name, i, "executing database query");
	qy = "qualified_get_server_host";
	argv[0] = "*";
	argv[1] = argv[2] = argv[3] = argv[5] = "DONTCARE";
	argv[4] = "TRUE";
	argc = 6;
	break;
    case MM_STATS:
	qy = "get_all_table_stats";
	argc = 0;
	break;
    case MM_CLIENTS:
	qy = "_list_users";
	argc = 0;
	break;
    }
    if (argc == -1) {
	display_error("Unknown function in menu callback.\n");
	return;
    }
    i = MoiraQuery(qy, argc, argv, retfunc, (char *)&dummy);
    if (i)
      com_err(program_name, i, "executing database query");
}


/******* temporary ********/
DisplayError(msg) char *msg; { fprintf(stderr, "%s\n", msg); }
Put_message(msg) char *msg; { fputs( msg, stderr); }
add_to_display(msg) char *msg; { fputs(msg, stderr); }
