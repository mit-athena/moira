/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/queries.c,v 1.8 1992-10-28 16:05:17 mar Exp $
 */

#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <moira.h>
#include <moira_site.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>
#include <X11/CompositeP.h>
#include <Xm/Xm.h>
#include "mmoira.h"
#ifdef GDSS
#include <des.h>
#include <krb.h>
#include <gdss.h>
#endif /* GDSS */


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
	count = 0;
	f = GetAndClearForm("mod_user");
	if (f == NULL) {
	    display_error("Unknown form in ModifyCallback!\n");
	    return;
	}
	f->extrastuff = form->extrastuff;
	f->menu = form->menu;
	for (i = 0; i < U_SIGNATURE; i++)
	  if (f->inputlines[i]->type == FT_BOOLEAN)
	    f->inputlines[i]->returnvalue.booleanvalue =
	      strcmp(argv[i + offset], "0") ? 1 : 0;
	  else
	    StoreField(f, i, argv[i + offset]);
	f->inputlines[U_SIGNATURE]->keyword_name = strsave(argv[U_SIGNATURE]);
	f->inputlines[U_SIGNATURE]->returnvalue.booleanvalue =
	  strcmp(argv[U_SECURE], "0") ? 1 : 0;
	f->inputlines[U_COMMENT]->keyword_name = strsave(argv[U_SECURE]);
	return;
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
    case MM_MOD_NFS:
	f = GetAndClearForm("mod_nfs");
	if (f == NULL) {
	    display_error("Unknown form in ModifyCallback of mod_nfs\n");
	    return;
	}
	f->extrastuff = form->extrastuff;
	f->menu = form->menu;
	StoreField(f, 0, argv[0]);
	StoreField(f, 1, argv[1]);
	StoreField(f, 2, argv[2]);
	f->inputlines[3]->keywords = nfs_states;
	for (i = 0; nfs_states[i]; i++)
	  if (atoi(nfs_states[i]) & atoi(argv[3]))
	    StoreField(f, 3, nfs_states[i]);
	if (atoi(argv[3]) & MR_FS_GROUPQUOTA)
	  f->inputlines[4]->returnvalue.booleanvalue = 1;
	else
	  f->inputlines[4]->returnvalue.booleanvalue = 0;
	StoreField(f, 5, argv[4]);
	StoreField(f, 6, argv[5]);
	return;
    case MM_MOD_MACH:
	fn = "mod_machine";
	count = M_MODTIME;
	break;
    case MM_MOD_CLUSTER:
	fn = "mod_cluster";
	count = C_MODTIME;
	break;
    case MM_MOD_PCAP:
	fn = "mod_printer";
	count = PCAP_MODTIME;
	break;
    case MM_MOD_SERVICE:
	fn = "mod_service";
	count = SH_MODTIME;
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


/* Generate a new cryptographic signature for the user record */
SignUser(argv, offset)
char **argv;
int offset;
{
#ifdef GDSS
    char buf[256];
    SigInfo si;
    int i;

    if (strcmp(argv[U_NAME + offset], UNIQUE_LOGIN)) {
	sprintf(buf, "%s:%s", argv[U_NAME + offset], argv[U_MITID + offset]);
	si.rawsig = NULL;
	i = GDSS_Verify(buf, strlen(buf), argv[U_SIGNATURE + offset], &si);
	/* If it's already signed OK, don't resign it. */
	if (i != GDSS_SUCCESS) {
	    free(argv[U_SIGNATURE + offset]);
	    argv[U_SIGNATURE + offset] = (char *) malloc(GDSS_Sig_Size() * 2);
	sign_again:
	    i = GDSS_Sign(buf, strlen(buf), argv[U_SIGNATURE + offset]);
	    if (i != GDSS_SUCCESS)
	      com_err(program_name, gdss2et(i),
		      "Failed to create signature");
	    else {
		unsigned char newbuf[256];
		si.rawsig = newbuf;
		i = GDSS_Verify(buf, strlen(buf),
				argv[U_SIGNATURE + offset], &si);
		if (strlen(newbuf) > 68) {
#ifdef DEBUG
		    AppendLog("Signature too long, trying again\n");
#endif /* DEBUG */
		    goto sign_again;
		}
	    }
#ifdef DEBUG
	    AppendLog("Made signature:");hex_dump(argv[U_SIGNATURE + offset]);
	} else {
	    AppendLog("Don't need to remake signature\n");
#endif /* DEBUG */
	}
    }
#else /* GDSS */
    argv[U_SIGNATURE + offset] = strsave("");
#endif /* GDSS */
}


/* when OK pressed */

MoiraFormComplete(dummy1, form)
int dummy1;
EntryForm *form;
{
    process_form(form, TRUE);
}


/* when APPLY pressed */

MoiraFormApply(dummy1, form)
int dummy1;
EntryForm *form;
{
    UserPrompt **p;
    int count;

    /* undocumented Motif internal routine to advance in tab group.
     * In this case we're going backwards because for some reason
     * the form advances whenever this button is pressed.
     * However, it doesn't seem to go backwards even though source 
     * implies that it should.  So we go forward until we wrap.
     */
    count = 0;
    for (p = form->inputlines; *p; p++)
      count++;
    while (count-- > 1)
      _XmMgrTraversal(form->formpointer, XmTRAVERSE_PREV_TAB_GROUP);
    process_form(form, FALSE);
}


int CollectData(argc, argv, form)
int argc;
char **argv;
EntryForm *form;
{
    struct save_queue *sq;

    sq = (struct save_queue *) form->extrastuff;
    sq_save_data(sq, strsave(argv[0]));
    return(MR_CONT);
}


/* callback when form is complete to process the data */

process_form(form, remove)
EntryForm *form;
int remove;
{
    char *qy, *argv[32], buf[256], *s, **aargv;
    int (*retfunc)(), argc, i;
    EntryForm *f;

    retfunc = DisplayCallback;
    argc = -1;

    for (i = 0; form->inputlines[i]; i++)
      argv[i] = StringValue(form, i);
    qy = form->menu->query;
    argc = form->menu->argc;

    switch (form->menu->operation) {
    case MM_SHOW_USER:
	if (*stringval(form, 0)) {
	    qy = "get_user_account_by_login";
	    argv[0] = stringval(form, 0);
	    argc = 1;
	} else if (*stringval(form, 3)) {
	    qy = "get_user_account_by_uid";
	    argv[0] = stringval(form, 3);
	    argc = 1;
	} else if (*stringval(form, 4)) {
	    qy = "get_user_account_by_class";
	    argv[0] = stringval(form, 4);
	    argc = 1;
	} else if (*stringval(form, 1) == 0 &&
		   *stringval(form, 2) == 0) {
	    display_error("Must fill in at least one blank.");
	    return;
	} else {
	    if (*stringval(form, 1) == 0)
	      StoreField(form, 1, "*");
	    if (*stringval(form, 2) == 0)
	      StoreField(form, 2, "*");
	    qy = "get_user_account_by_name";
	    argv[0] = stringval(form, 1);
	    argv[1] = stringval(form, 2);
	    argc = 2;
	}
	break;
    case MM_ADD_USER:
	argv[U_STATE][1] = 0;
	argv[U_SECURE] = argv[U_SIGNATURE];
	SignUser(argv, 0);
	argc = U_MODTIME;
	break;
    case MM_MOD_USER:
	if (!strcmp(form->formname, "mod_user")) {
	    qy = "update_user_account";
	    for (i = 0; i < U_SIGNATURE; i++)
	      argv[i + 1] = StringValue(form, i);
	    argv[0] = form->extrastuff;
	    argv[U_STATE + 1][1] = 0;
	    argv[U_SIGNATURE + 1] = form->inputlines[U_SIGNATURE]->keyword_name;
	    argv[U_SECURE + 1] = StringValue(form, U_SIGNATURE);
	    if (*argv[U_SECURE + 1] == '1')
	      if (atoi(form->inputlines[U_COMMENT]->keyword_name))
		argv[U_SECURE + 1] = form->inputlines[U_COMMENT]->keyword_name;
	      else {
		  struct timeval tv;
		  char buf[32];
		  gettimeofday(&tv, NULL);
		  printf("Got %ld, %ld\n", tv.tv_sec, tv.tv_usec);
		  sprintf(buf, "%ld", tv.tv_sec);
		  argv[U_SECURE + 1] = strsave(buf);
	      }
	    SignUser(argv, 1);
	    argc = U_MODTIME + 1;
	    break;
	}
	form->extrastuff = (caddr_t) "mod_user";
	retfunc = ModifyCallback;
	break;
    case MM_REGISTER:
	if (*stringval(form, 2)) {
	    argv[0] = stringval(form, 2);
	} else {
	    argv[0] = stringval(form, 0);
	    argv[1] = stringval(form, 1);
	    form->extrastuff = NULL;
	    i = MoiraQuery("get_user_account_by_name", 2, argv,
			   ModifyCallback, (char *)form);
	    if (i) {
		com_err(program_name, i, " looking up user by name");
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
    case MM_DEACTIVATE:
	argv[1] = "3";
	break;
    case MM_EXPUNGE:
	display_error("Expunge is not yet implemented");
	return;
	break;
    case MM_MOD_FINGER:
	if (!strcmp(form->formname, "mod_finger")) {
	    qy = "update_finger_by_login";
	    for (i = 0; i < F_MODTIME - 1; i++)
	      argv[i + 1] = StringValue(form, i);
	    argv[0] = form->extrastuff;
	    argc = F_MODTIME;
	    break;
	}
	form->extrastuff = (caddr_t) "mod_finger";
	retfunc = ModifyCallback;
	break;
    case MM_SHOW_KRBMAP:
	if (!*stringval(form, 0))
	  argv[0] = "*";
	if (!*stringval(form, 1))
	  argv[1] = "*";
	break;
    case MM_SET_POBOX:
	if (!strcmp(argv[1], "POP"))
	  argv[2] = stringval(form, 3);
	break;
    case MM_SHOW_ACE_USE:
	if (boolval(form, 2)) {
	    sprintf(buf, "R%s", stringval(form, 0));
	    argv[0] = buf;
	}
	break;
    case MM_MOD_LIST:
	if (!strcmp(form->formname, "mod_list")) {
	    qy = "update_list";
	    for (i = 0; i < L_MODTIME; i++)
	      argv[i + 1] = StringValue(form, i);
	    argv[0] = form->extrastuff;
	    argc = L_MODTIME + 1;
	    break;
	}
	form->extrastuff = (caddr_t) "mod_list";
	retfunc = ModifyCallback;
	break;
    case MM_SHOW_MEMBERS:
	if (!*stringval(form, 0)) {
	    qy = "get_lists_of_member";
	    argv[0] = stringval(form, 1);
	    sprintf(buf, "Lists of %s %s:\n", stringval(form, 1), argv[2]);
	    AppendToLog(buf);
	    if (boolval(form, 3)) {
		sprintf(buf, "R%s", stringval(form, 1));
		argv[0] = buf;
	    }
	    argv[1] = stringval(form, 2);
	    argc = 2;
	} else {
	    sprintf(buf, "Members of list: %s\n", argv[0]);
	    AppendToLog(buf);
	}
	break;
    case MM_DEL_ALL_MEMBER:
	form->extrastuff = (caddr_t) sq_create();
	retfunc = CollectData;
	break;
    case MM_SHOW_FILSYS:
	if (*stringval(form, 0)) {
	    qy = "get_filesys_by_label";
	    argv[0] = stringval(form, 0);
	    argc = 1;
	} else if (*stringval(form, 1)) {
	    StoreHost(form, 1, &argv[0]);
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
	/* fall through to */
    case MM_SHOW_FSGROUP:
	form->extrastuff = (caddr_t) sq_create();
	break;
    case MM_ADD_FILSYS:
	StoreHost(form, FS_MACHINE, &argv[FS_MACHINE]);
	if (!strcmp(stringval(form, FS_TYPE), "AFS") ||
	    !strcmp(stringval(form, FS_TYPE), "FSGROUP") ||
	    !strcmp(stringval(form, FS_TYPE), "MUL"))
	  argv[FS_MACHINE] = "\\[NONE\\]";
	break;
    case MM_MOD_FILSYS:
	if (!strcmp(form->formname, "mod_filsys")) {
	    qy = "update_filesys";
	    for (i = 0; i < FS_MODTIME; i++)
	      argv[i + 1] = StringValue(form, i);
	    StoreHost(form, FS_MACHINE, &argv[FS_MACHINE + 1]);
	    argv[0] = form->extrastuff;
	    argc = FS_MODTIME + 1;
	    break;
	}
	form->extrastuff = (caddr_t) "mod_filsys";
	retfunc = ModifyCallback;
	break;
    case MM_MOV_FSGROUP:
	argv[1] = strsave(stringval(form, 1));
	s = index(argv[1], ' ');
	if (s) *s = 0;
	i = MoiraQuery("remove_filesys_from_fsgroup", 2, argv, ModifyCallback,
		       (char *)form);
	if (i) {
	    com_err(program_name, i, " removing filesystem from FS group");
	    return;
	}
	argc = 3;
	/* fall through to: */
    case MM_ADD_FSGROUP:
	/* find the two keys to sort between */
	argv[2] = strsave(argv[2]);
	s = index(argv[2], ' ');
	if (s) {
	    argv[2] = s+2;
	    s = index(argv[2], ')');
	    if (s) *s = 0;
	} else
	  argv[2] = "";
	if (*argv[2] == 0) argv[2] = "A";
	/* Finding the after key is gross.  We look through the widgets
	 * in the radiobox to find the one selected and the one after
	 * it.  The name of the widget is also the member name.
	 */
	{ 
	    Widget w, kid;
	    CompositeRec *cr;

	    argv[3] = "";
	    cr = (CompositeRec *)form->inputlines[2]->mywidget;
	    for (i = 0; i < cr->composite.num_children; i++) {
		kid = cr->composite.children[i];
		if (!strcmp(XtName(kid), stringval(form, 2))) {
		    i++;
		    if (i < cr->composite.num_children) {
			argv[3] = strsave(XtName(cr->composite.children[i]));
			s = index(argv[3], ' ');
			if (s) {
			    argv[3] = s+2;
			    s = index(argv[3], ')');
			    if (s) *s = 0;
			} else
			  argv[3] = "";
		    }
		    break;
		}
	    }
	}
	if (*argv[3] == 0) argv[3] = "Z";
#ifdef DEBUG
	printf("Got before key of \"%s\" and after key of \"%s\"\n",
	       argv[2], argv[3]);
#endif /* DEBUG */
	/* copy the matching chars */
	for  (s = buf; *argv[2] && *argv[2] == *argv[3]; argv[3]++)
	  *s++ = *argv[2]++;
	/* and set the last char */
	if (*argv[2] == 0)
	  *argv[2] = i = 'A';
	else
	  i = argv[2][1];
	if (i == 0) i = 'A';
	if (*argv[3] == 0) *argv[3] = 'Z';
	if (*argv[3] - *argv[2] > 1) {
	    *s++ = (*argv[3] + *argv[2])/2;
	} else {
	    *s++ = *argv[2];
	    *s++ = (i + 'Z')/2;
	}
	*s = 0;
	argv[2] = strsave(buf);
	break;
    case MM_DEL_FSGROUP:
	argv[1] = strsave(stringval(form, 1));
	s = index(argv[1], ' ');
	if (s) *s = 0;
	break;
    case MM_SHOW_FS_ALIAS:
    case MM_ADD_FS_ALIAS:
    case MM_DEL_FS_ALIAS:
	argv[1] = "FILESYS";
	argv[2] = stringval(form, 1);
	break;
    case MM_SHOW_NFS:
	StoreHost(form, NFS_NAME, &argv[NFS_NAME]);
	if (!*stringval(form, 1))
	  argv[1] = "*";
	break;
    case MM_ADD_NFS:
	StoreHost(form, NFS_NAME, &argv[NFS_NAME]);
	sprintf(buf, "%d", atoi(stringval(form, NFS_STATUS)) +
		(boolval(form, 4) ? MR_FS_GROUPQUOTA : 0));
	argv[NFS_STATUS] = buf;
	argv[NFS_ALLOC] = stringval(form, 5);
	argv[NFS_SIZE] = stringval(form, 6);
	break;
    case MM_MOD_NFS:
	StoreHost(form, NFS_NAME, &argv[NFS_NAME]);
	if (!strcmp(form->formname, "mod_nfs")) {
	    qy = "update_nfsphys";
	    argc = NFS_MODTIME;
	    StoreHost(form, NFS_NAME, &argv[NFS_NAME]);
	    sprintf(buf, "%d", atoi(stringval(form, NFS_STATUS)) +
		    (boolval(form, 4) ? MR_FS_GROUPQUOTA : 0));
	    argv[NFS_STATUS] = buf;
	    argv[NFS_ALLOC] = stringval(form, 5);
	    argv[NFS_SIZE] = stringval(form, 6);
	    break;
	}
	form->extrastuff = (caddr_t) "mod_nfs";
	retfunc = ModifyCallback;
	break;
    case MM_DEL_NFS:
	StoreHost(form, NFS_NAME, &argv[NFS_NAME]);
	break;
    case MM_SHOW_QUOTA:
	if (!*stringval(form, 0))
	  argv[0] = "*";
	if (*stringval(form, 1)) {
	    argv[1] = "USER";
	    argv[2] = stringval(form, 1);
	} else if (*stringval(form, 2)) {
	    argv[1] = "GROUP";
	    argv[2] = stringval(form, 2);
	} else if (!*stringval(form, 0)) {
	    display_error("Must specify something.");
	    return;
	} else {
	    qy = "get_quota_by_filesys";
	    argc = 1;
	}
	break;
    case MM_SHOW_MACH:
    case MM_ADD_MACH:
    case MM_DEL_MACH:
	StoreHost(form, 0, &argv[0]);
	break;
    case MM_MOD_MACH:
	if (!strcmp(form->formname, "mod_machine")) {
	    qy = "update_machine";
	    for (i = 0; i < M_MODTIME; i++)
	      argv[i + 1] = StringValue(form, i);
	    argv[0] = form->extrastuff;
	    argc = M_MODTIME + 1;
	    break;
	}
	StoreHost(form, 0, &argv[0]);
	form->extrastuff = (caddr_t) "mod_machine";
	retfunc = ModifyCallback;
	break;
    case MM_MOD_CLUSTER:
	if (!strcmp(form->formname, "mod_cluster")) {
	    qy = "update_cluster";
	    for (i = 0; i < C_MODTIME; i++)
	      argv[i + 1] = StringValue(form, i);
	    argv[0] = form->extrastuff;
	    argc = C_MODTIME + 1;
	    break;
	}
	form->extrastuff = (caddr_t) "mod_cluster";
	retfunc = ModifyCallback;
	break;
    case MM_SHOW_MCMAP:
	if (!*stringval(form, 0))
	  argv[0] = "*";
	else
	  StoreHost(form, 0, &argv[0]);
	if (!*stringval(form, 1))
	  argv[1] = "*";
	AppendToLog("Cluster mappings:\n");
	break;
    case MM_ADD_MCMAP:
    case MM_DEL_MCMAP:
	StoreHost(form, 0, &argv[0]);
	break;
    case MM_SHOW_CLDATA:
	if (!*stringval(form, 1))
	  argv[1] = "*";
	break;
    case MM_ADD_PCAP:
	StoreHost(form, PCAP_SPOOL_HOST, &argv[PCAP_SPOOL_HOST]);
	StoreHost(form, PCAP_QSERVER, &argv[PCAP_QSERVER]);
	break;
    case MM_MOD_PCAP:
	if (!strcmp(form->formname, "mod_printer")) {
	    qy = "update_printcap_entry";
	    argc = PCAP_MODTIME;
	    break;
	}
	form->extrastuff = (caddr_t) "mod_printer";
	retfunc = ModifyCallback;
	break;
    case MM_MOD_SERVICE:
	if (!strcmp(form->formname, "mod_service")) {
	    qy = "update_server_host_info";
	    argc = SH_MODTIME;
	    break;
	}
	form->extrastuff = (caddr_t) "mod_service";
	retfunc = ModifyCallback;
	break;
    case MM_SAVE_LOG:
	if (!write_log_to_file(stringval(form, 0)) && remove)
	  XtUnmanageChild(form->formpointer);
	return;
    case MM_NEW_VALUE:
	argv[0] = form->extrastuff;
	argv[1] = "TYPE";
	argv[2] = StringValue(form, 0);
	for (s = argv[2]; *s; s++)
	  if (islower(*s)) *s = toupper(*s);
	break;
    }

    if (argc == -1) {
	display_error("Unknown function in form callback.\n");
	return;
    }
    i = MoiraQuery(qy, argc, argv, retfunc, (char *)form);
    if (i) {
	com_err(program_name, i, " executing database query");
	return;
    }
    
    f = NULL;
    if (form->extrastuff && (f = GetForm((char *)(form->extrastuff)))) {
	if (form->formpointer)
	  XtUnmanageChild(form->formpointer);
	f->extrastuff = (caddr_t) strsave(stringval(form, 0));
	f->menu = form->menu;
    }

    switch (form->menu->operation) {
    case MM_MOD_USER:
	if (f) {
	    f->inputlines[U_STATE]->keywords = user_states;
	    StoreField(f, U_STATE, user_states[atoi(stringval(f, U_STATE))]);
	    GetKeywords(f, U_CLASS, "class");
	} else
	  AppendToLog("Done.\n");
	break;
    case MM_MOD_FINGER:
    case MM_MOD_CLUSTER:
	if (!f)
	  AppendToLog("Done.\n");	  
	break;
    case MM_MOD_LIST:
	if (f) {
	    GetKeywords(f, L_ACE_TYPE, "ace_type");
	    f->inputlines[L_GROUP]->valuechanged = MoiraValueChanged;
	    f->inputlines[L_ACE_TYPE]->valuechanged = MoiraValueChanged;
	} else
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
	    f->inputlines[FS_TYPE]->valuechanged = MoiraValueChanged;
	} else
	  AppendToLog("Done.\n");	  
	break;
    case MM_MOD_MACH:
	if (f)
	  GetKeywords(f, 1, "mac_type");
	else
	  AppendToLog("Done.\n");	  
	break;
    case MM_MOD_NFS:
	if (f)
	  f->inputlines[3]->keywords = nfs_states;
	else
	  AppendToLog("Done.\n");	  
	break;
    case MM_DEL_ALL_MEMBER:
	argv[1] = StringValue(form, 0);
	argv[2] = StringValue(form, 1);
	while (sq_get_data(form->extrastuff, &(argv[0]))) {
	    sprintf(buf, "Delete %s %s from list %s?", StringValue(form, 0),
		    StringValue(form, 1), argv[0]);
	    if (!boolval(form, 2) ||
		AskQuestion(buf, "confirm_del_all")) {
		i = MoiraQuery("delete_member_from_list", 3, argv,
			       DisplayCallback, NULL);
		if (i)
		  com_err(program_name, i, " while removing member from list");
		else {
		    sprintf(buf, "Member %s %s removed from list %s.\n",
			    argv[1], argv[2], argv[0]);
		    AppendToLog(buf);
		}
	    }
	    free(argv[0]);
	}
	AppendToLog("Done.\n");
	break;
    case MM_SHOW_FILSYS:
    case MM_SHOW_FSGROUP:
	while (sq_get_data(form->extrastuff, &aargv)) {
	    ShowFilsys(aargv);
	}
	sq_destroy(form->extrastuff);
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
    case MM_RESET_POBOX:
	AppendToLog("Done.\n");
	break;
    case MM_NEW_VALUE:
	CacheNewValue(GetForm(form->menu->form), (int) form->menu->accel,
		      form->extrastuff, StringValue(form, 0));
    }

    if (remove && form->formpointer)
      XtUnmanageChild(form->formpointer);

    if (f)
      DisplayForm(f);
    else
      AppendToLog("\n");
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
    qy = m->query;
    argc = m->argc;

    switch (m->operation) {
    case MM_SHOW_MAILLIST:
	argv[0] = argv[1] = argv[3] = "TRUE";
	argv[2] = "FALSE";
	argv[4] = "DONTCARE";
	AppendToLog("Public Mailinglists:\n");
	break;
    case MM_SHOW_DQUOTA:
	argv[0] = "def_quota";
	break;
    case MM_SHOW_DCM:
	AppendToLog("Services and Hosts with failed updates:\n");
	argv[0] = argv[2] = "DONTCARE";
	argv[1] = "TRUE";
	i = MoiraQuery("qualified_get_server", 3, argv, retfunc, NULL);
	if (i && i != MR_NO_MATCH)
	  com_err(program_name, i, " executing database query");
	argv[0] = "*";
	argv[1] = argv[2] = argv[3] = argv[5] = "DONTCARE";
	argv[4] = "TRUE";
	i = MoiraQuery("qualified_get_server_host", 6, argv, retfunc, NULL);
	if (i && i != MR_NO_MATCH)
	  com_err(program_name, i, " executing database query");
	AppendToLog("\n");
	return;
    case MM_HELP_MOIRA:
    case MM_HELP_WILDCARDS:
    case MM_HELP_AUTHORS:
    case MM_HELP_BUGS:
    case MM_HELP_MOUSE:
    case MM_HELP_KEYBOARD:
	help(m->query);
	return;
    case MM_QUIT:
	mr_disconnect();
	exit(0);
    }
    if (argc == -1) {
	display_error("Unknown function in menu callback.\n");
	return;
    }
    i = MoiraQuery(qy, argc, argv, retfunc, (char *)&dummy);
    if (i)
      com_err(program_name, i, " executing database query");
    AppendToLog("\n");
}
