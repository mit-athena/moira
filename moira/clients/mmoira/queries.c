/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/queries.c,v 1.1 1991-05-31 13:02:44 mar Exp $
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
	    f->inputlines[i]->returnvalue.booleanvalue =
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
	XtUnmanageChild(form->formpointer);
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
	  AppendToLog("Done.\n");
	break;
    case MM_MOD_FINGER:
	if (!f)
	  AppendToLog("Done.\n");	  
	break;
    case MM_MOD_LIST:
	if (f)
	  GetKeywords(f, L_ACE_TYPE, "ace_type");
	else
	  AppendToLog("Done.\n");	  
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
	  AppendToLog("Done.\n");	  
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
	AppendToLog("Done.\n");
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
	AppendToLog("Services and Hosts with failed updates:");
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
