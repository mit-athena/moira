/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/form_setup.c,v 1.7 1992-10-28 16:14:06 mar Exp $
 */

#include <stdio.h>
#include <ctype.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <moira.h>
#include <moira_site.h>
#include <Xm/Xm.h>
#include "mmoira.h"


extern char *user, *program_name;
extern EntryForm *MoiraForms[];
EntryForm *GetAndClearForm();

char *user_states[] = { "0 - Registerable",
			  "1 - Active",
			  "2 - Half Registered",
			  "3 - Deleted",
			  "4 - Not registerable",
			  "5 - Enrolled/Registerable",
			  "6 - Enrolled/Not Registerable",
			  "7 - Half Enrolled",
		          NULL};
char *nfs_states[] = { "0 - Not Allocated",
			 "1 - Student",
			  "2 - Faculty",
			  "4 - Staff",
			  "8 - Miscellaneous",
		          NULL};


static FixNameCase(form, field)
EntryForm *form;
int field;
{
    char fixname[128], buf[256];

    strcpy(fixname, stringval(form, field));
    FixCase(fixname);
    sprintf(buf, "You entered a name which does not follow the capitalization conventions.\nCorrect it to \"%s\"?", fixname);
    if (strcmp(fixname, stringval(form, field)) &&
	AskQuestion(buf, "fixcase"))
      StoreField(form, field, fixname);
}


get_members(argc, argv, sq)
int argc;
char **argv;
struct save_queue *sq;
{
    char buf[256];

    sprintf(buf, "%s (%s)", argv[0], argv[1]);
    sq_save_data(sq, strsave(buf));
    return(MR_CONT);
}


/* Deal with AFS cell names */

char *canonicalize_cell(c)
char *c;
{
    struct stat stbuf;
    char path[512];
    int count;

    c = strtrim(c);
    sprintf(path, "/afs/%s", c);
    if (lstat(path, &stbuf) || !stbuf.st_mode&S_IFLNK)
      return(c);
    count = readlink(path, path, sizeof(path));
    if (count < 1) return(c);
    path[count] = 0;
    free(c);
    return(strsave(path));
}


int GetAliasValue(argc, argv, retval)
int argc;
char **argv;
char **retval;
{
    *retval = strsave(argv[2]);
    return(MR_CONT);
}



MoiraValueChanged(f, prompt)
EntryForm *f;
UserPrompt *prompt;
{
    char buf[1024];
    char *argv[5], *p;
    int i, size, field;
    struct save_queue *sq, *s;
#define maybechange(f, n, v)	{if (f->inputlines[n]->insensitive != v) { f->inputlines[n]->insensitive=v; f->inputlines[n]->changed = True; }}

    for (field = 0; f->inputlines[field]; field++)
      if (prompt == f->inputlines[field]) break;

    switch (f->menu->operation) {
    case MM_ADD_LIST:
    case MM_MOD_LIST:
	maybechange(f, L_GID, !boolval(f, L_GROUP));
	if (!strcmp(stringval(f, L_ACE_TYPE), "NONE"))
	  maybechange(f, L_ACE_NAME, True)
	else
	  maybechange(f, L_ACE_NAME, False)
	break;
    case MM_ADD_FILSYS:
    case MM_MOD_FILSYS:
	if (field == FS_TYPE &&
	    (!strcmp(stringval(f, FS_TYPE), "FSGROUP") ||
	     !strcmp(stringval(f, FS_TYPE), "MUL") ||
	     !strcmp(stringval(f, FS_TYPE), "ERR"))) {
	    maybechange(f, FS_MACHINE, True);
	    maybechange(f, FS_PACK, True);
	    maybechange(f, FS_M_POINT, True);
	    maybechange(f, FS_ACCESS, True);
	    maybechange(f, FS_CREATE, True);
	} else if (field == FS_TYPE &&
		   (!strcmp(stringval(f, FS_TYPE), "NFS") ||	
		    !strcmp(stringval(f, FS_TYPE), "AFS") ||
		    !strcmp(stringval(f, FS_TYPE), "RVD"))) {
	    maybechange(f, FS_MACHINE, False);
	    maybechange(f, FS_PACK, False);
	    maybechange(f, FS_M_POINT, False);
	    maybechange(f, FS_ACCESS, False);
	    maybechange(f, FS_CREATE, False);
	}
	if (field == FS_NAME && !strcmp(stringval(f, FS_M_POINT), "/mit/")) {
	    sprintf(buf, "/mit/%s", stringval(f, FS_NAME));
	    StoreField(f, FS_M_POINT, buf);
	}
	if (field == FS_MACHINE && !strcmp(stringval(f, FS_TYPE), "AFS")) {
	    p = strsave(stringval(f, FS_MACHINE));
	    p = canonicalize_cell(p);
	    lowercase(p);
	    StoreField(f, FS_MACHINE, p);
	    free(p);
	}
	if (field == FS_MACHINE && (!strcmp(stringval(f, FS_TYPE), "NFS") ||
				    !strcmp(stringval(f, FS_TYPE), "RVD"))) {
	    StoreHost(f, FS_MACHINE, &p);
	}
	if (!strcmp(stringval(f, FS_TYPE), "AFS") &&
	    *stringval(f, FS_NAME) &&
	    *stringval(f, FS_MACHINE) &&
	    *stringval(f, FS_L_TYPE)) {
	    char *path;
	    int depth;

	    sprintf(buf, "%s:%s", stringval(f, FS_MACHINE),
		    stringval(f, FS_L_TYPE));
	    argv[0] = buf;
	    argv[1] = "AFSPATH";
	    argv[2] = "*";
	    path = "???";
	    i = MoiraQuery("get_alias", 3, argv, GetAliasValue, &path);
	    if (i == MR_SUCCESS) {
		p = index(path, ':');
		if (p) {
		    *p = 0;
		    depth = atoi(++p);
		} else depth = 0;
		sprintf(buf, "/afs/%s/%s", stringval(f, FS_MACHINE), path);
		if (depth >= 0) {
		    for (p=stringval(f, FS_NAME);
			 *p && (p - stringval(f, FS_NAME)) < depth;
			 p++) {
			if (islower(*p)) {
			    strcat(buf, "/x");
			    buf[strlen(buf)-1] = *p;
			} else {
			    sprintf(buf, "/afs/%s/%s/other",
				    stringval(f, FS_MACHINE), path);
			    break;
			}
		    }
		} else if (depth = -1) {
		    if (isdigit(stringval(f, FS_NAME)[0])) {
			strcat(buf, "/");
			depth = strlen(buf);
			for (p = stringval(f, FS_NAME);
			     *p && isdigit(*p);
			     p++) {
			    buf[depth++] = *p;
			    buf[depth] = 0;
			}
		    } else
		      strcat(buf, "/other");
		} else {
		    /* no default */
		}
		strcat(buf, "/");
		strcat(buf, stringval(f, FS_NAME));
		free(path);
	    } else {
		p = strsave(stringval(f, FS_L_TYPE));
		sprintf(buf, "/afs/%s/%s/%s", stringval(f, FS_MACHINE),
			lowercase(p), stringval(f, FS_NAME));
		free(p);
	    }
	    StoreField(f, FS_PACK, buf);
	}
	break;
    case MM_SET_POBOX:
	if (!strcmp(stringval(f, PO_TYPE), "POP")) {
	    maybechange(f, 2, True);
	    maybechange(f, 3, False);
	} else if (!strcmp(stringval(f, PO_TYPE), "SMTP")) {
	    maybechange(f, 2, False);
	    maybechange(f, 3, True);
	} else if (!strcmp(stringval(f, PO_TYPE), "NONE")) {
	    maybechange(f, 2, True);
	    maybechange(f, 3, True);
	}
	break;
    case MM_ADD_QUOTA:
    case MM_MOD_QUOTA:
	if (!strcmp(stringval(f, Q_TYPE), "ANY"))
	  maybechange(f, Q_NAME, True)
	else
	  maybechange(f, Q_NAME, False)
	break;
    case MM_ADD_ZEPHYR:
    case MM_MOD_ZEPHYR:
	if (!strcmp(stringval(f, ZA_XMT_TYPE), "NONE"))
	  maybechange(f, ZA_XMT_ID, True)
	else
	  maybechange(f, ZA_XMT_ID, False)
	if (!strcmp(stringval(f, ZA_SUB_TYPE), "NONE"))
	  maybechange(f, ZA_SUB_ID, True)
	else
	  maybechange(f, ZA_SUB_ID, False)
	if (!strcmp(stringval(f, ZA_IWS_TYPE), "NONE"))
	  maybechange(f, ZA_IWS_ID, True)
	else
	  maybechange(f, ZA_IWS_ID, False)
	if (!strcmp(stringval(f, ZA_IUI_TYPE), "NONE"))
	  maybechange(f, ZA_IUI_ID, True)
	else
	  maybechange(f, ZA_IUI_ID, False)
	break;
    case MM_ADD_USER:
	FixNameCase(f, U_LAST);
	FixNameCase(f, U_FIRST);
	FixNameCase(f, U_MIDDLE);
	break;
    case MM_SHOW_USER:
	FixNameCase(f, 1);
	FixNameCase(f, 2);
	break;
    case MM_ADD_FSGROUP:
	argv[0] = stringval(f, 0);
	sq = sq_create();
	i = MoiraQuery("get_fsgroup_members", 1, argv, get_members, (char *)sq);
	if (i) {
	    com_err(program_name, i, " retrieving filesystem group members");
	    break;
	}
	size = 1;
	for (s = sq->q_next; s->q_next != sq; s = s->q_next)
	  size++;
	if (f->inputlines[2]->keywords)
	  free(f->inputlines[2]->keywords);
	f->inputlines[2]->keywords = (char **)malloc(sizeof(char *)*(size+2));
	if (f->inputlines[2]->keywords == NULL) {
	    display_error("Out of memory while fetching members");
	    return;
	}
	f->inputlines[2]->keywords[0] = "[First]";
	for (i = 0; i < size; i++)
	  sq_get_data(sq, &f->inputlines[2]->keywords[i+1]);
	f->inputlines[2]->keywords[i+1] = NULL;
	f->inputlines[2]->changed = 1;
	sq_destroy(sq);
	RemakeRadioField(f, 2);
	break;
    case MM_DEL_FSGROUP:
	argv[0] = stringval(f, 0);
	sq = sq_create();
	i = MoiraQuery("get_fsgroup_members", 1, argv, get_members, (char *)sq);
	if (i) {
	    com_err(program_name, i, " retrieving filesystem group members");
	    break;
	}
	size = 1;
	for (s = sq->q_next; s->q_next != sq; s = s->q_next)
	  size++;
	if (f->inputlines[1]->keywords)
	  free(f->inputlines[1]->keywords);
	f->inputlines[1]->keywords = (char **)malloc(sizeof(char *)*(size+1));
	if (f->inputlines[1]->keywords == NULL) {
	    display_error("Out of memory while fetching members");
	    return;
	}
	for (i = 0; i < size; i++)
	  sq_get_data(sq, &f->inputlines[1]->keywords[i]);
	f->inputlines[1]->keywords[i] = NULL;
	f->inputlines[1]->changed = 1;
	sq_destroy(sq);
	RemakeRadioField(f, 1);
	break;
    case MM_MOV_FSGROUP:
	argv[0] = stringval(f, 0);
	sq = sq_create();
	i = MoiraQuery("get_fsgroup_members", 1, argv, get_members, (char *)sq);
	if (i) {
	    com_err(program_name, i, " retrieving filesystem group members");
	    break;
	}
	size = 1;
	for (s = sq->q_next; s->q_next != sq; s = s->q_next)
	  size++;
	if (f->inputlines[1]->keywords)
	  free(f->inputlines[1]->keywords);
	f->inputlines[1]->keywords = (char **)malloc(sizeof(char *)*(size+1));
	if (f->inputlines[1]->keywords == NULL) {
	    display_error("Out of memory while fetching members");
	    return;
	}
	if (f->inputlines[2]->keywords)
	  free(f->inputlines[2]->keywords);
	f->inputlines[2]->keywords = (char **)malloc(sizeof(char *)*(size+2));
	if (f->inputlines[2]->keywords == NULL) {
	    display_error("Out of memory while fetching members");
	    return;
	}
	f->inputlines[2]->keywords[0] = "[First]";
	for (i = 0; i < size; i++) {
	    sq_get_data(sq, &f->inputlines[1]->keywords[i]);
	    f->inputlines[2]->keywords[i+1] = f->inputlines[1]->keywords[i];
	}
	f->inputlines[1]->keywords[i] = NULL;
	f->inputlines[2]->keywords[i+1] = NULL;
	f->inputlines[1]->changed = 1;
	f->inputlines[2]->changed = 1;
	sq_destroy(sq);
	RemakeRadioField(f, 1);
	RemakeRadioField(f, 2);
	break;
    default:
	return;
    }
    UpdateForm(f);
}


/* Called when a menu item is selected. */

MoiraMenuRequest(menu)
MenuItem	*menu;
{
    EntryForm *f;

    f = GetAndClearForm(menu->form);
    if (f == NULL) {
	if (!strcmp(menu->form, "0"))
	  MoiraMenuComplete(menu);
	else
	  display_error("Sorry, unable to process that request (no form registered).");
	return;
    }

    switch (menu->operation) {
    case MM_SHOW_USER:
	GetKeywords(f, 4, "class");
	f->inputlines[1]->valuechanged = MoiraValueChanged;
	f->inputlines[2]->valuechanged = MoiraValueChanged;
	break;
    case MM_ADD_USER:
	StoreField(f, U_NAME, UNIQUE_LOGIN);
	StoreField(f, U_UID, UNIQUE_UID);
	StoreField(f, U_SHELL, "/bin/csh");
	StoreField(f, U_STATE, user_states[US_NO_LOGIN_YET]);
	f->inputlines[U_STATE]->keywords = user_states;
 	f->inputlines[U_LAST]->valuechanged = MoiraValueChanged;
	f->inputlines[U_FIRST]->valuechanged = MoiraValueChanged;
	f->inputlines[U_MIDDLE]->valuechanged = MoiraValueChanged;
	GetKeywords(f, U_CLASS, "class");
	break;
    case MM_SET_POBOX:
	GetKeywords(f, 1, "pobox");
	GetKeywords(f, 3, "poserver");
	f->inputlines[PO_TYPE]->valuechanged = MoiraValueChanged;
	break;
    case MM_ADD_FILSYS:
	StoreField(f, FS_TYPE, "AFS");
	StoreField(f, FS_M_POINT, "/mit/");
	StoreField(f, FS_MACHINE, "athena.mit.edu");
	StoreField(f, FS_ACCESS, "w");
	StoreField(f, FS_OWNER, user);
	StoreField(f, FS_OWNERS, user);
	boolval(f, FS_CREATE) = TRUE;
	GetKeywords(f, FS_TYPE, "filesys");
	GetKeywords(f, FS_ACCESS, "fs_access_AFS");
	GetKeywords(f, FS_L_TYPE, "lockertype");
	f->inputlines[FS_TYPE]->valuechanged = MoiraValueChanged;
	f->inputlines[FS_L_TYPE]->valuechanged = MoiraValueChanged;
	f->inputlines[FS_NAME]->valuechanged = MoiraValueChanged;
	f->inputlines[FS_MACHINE]->valuechanged = MoiraValueChanged;
	break;
    case MM_ADD_FSGROUP:
	if (f->inputlines[2]->keywords)
	  free(f->inputlines[2]->keywords);
	f->inputlines[2]->keywords = (char **)malloc(sizeof(char*)*2);
	f->inputlines[2]->keywords[0] = "[First]";
	f->inputlines[2]->keywords[1] = NULL;
	f->inputlines[0]->valuechanged = MoiraValueChanged;
	break;
    case MM_DEL_FSGROUP:
	if (f->inputlines[1]->keywords)
	  free(f->inputlines[1]->keywords);
	f->inputlines[1]->keywords = (char **)malloc(sizeof(char*)*2);
	f->inputlines[1]->keywords[0] = "[Placeholder]";
	f->inputlines[1]->keywords[1] = NULL;
	f->inputlines[0]->valuechanged = MoiraValueChanged;
	break;
    case MM_MOV_FSGROUP:
	if (f->inputlines[1]->keywords)
	  free(f->inputlines[1]->keywords);
	f->inputlines[1]->keywords = (char **)malloc(sizeof(char*)*2);
	f->inputlines[1]->keywords[0] = "[Placeholder]";
	f->inputlines[1]->keywords[1] = NULL;
	if (f->inputlines[2]->keywords)
	  free(f->inputlines[2]->keywords);
	f->inputlines[2]->keywords = (char **)malloc(sizeof(char*)*2);
	f->inputlines[2]->keywords[0] = "[First]";
	f->inputlines[2]->keywords[1] = NULL;
	f->inputlines[0]->valuechanged = MoiraValueChanged;
	break;
    case MM_ADD_NFS:
	StoreField(f, 1, "/u1/lockers");
	StoreField(f, 2, "/dev/");
	StoreField(f, 5, "0");
	StoreField(f, 6, "0");
	StoreField(f, 3, nfs_states[1]);
	f->inputlines[3]->keywords = nfs_states;
	break;
    case MM_ADD_QUOTA:
    case MM_DEL_QUOTA:
    case MM_MOD_QUOTA:
	GetKeywords(f, 1, "quota_type");
	f->inputlines[Q_TYPE]->valuechanged = MoiraValueChanged;
	break;
    case MM_SHOW_ACE_USE:
	StoreField(f, 0, "USER");
	GetKeywords(f, 0, "ace_type");
	break;
    case MM_ADD_LIST:
	boolval(f, L_ACTIVE) = TRUE;
	boolval(f, L_MAILLIST) = TRUE;
	StoreField(f, L_GID, UNIQUE_GID);
	StoreField(f, L_ACE_TYPE, "USER");
	GetKeywords(f, L_ACE_TYPE, "ace_type");
	f->inputlines[L_GID]->insensitive = True;
	f->inputlines[L_GROUP]->valuechanged = MoiraValueChanged;
	f->inputlines[L_ACE_TYPE]->valuechanged = MoiraValueChanged;
	break;
    case MM_SHOW_MEMBERS:
	GetKeywords(f, 1, "member");
	break;
    case MM_ADD_MEMBER:
    case MM_DEL_MEMBER:
	GetKeywords(f, 1, "member");
	break;
    case MM_DEL_ALL_MEMBER:
	boolval(f, 2) = TRUE;
	GetKeywords(f, 0, "member");
	break;
    case MM_ADD_MACH:
	GetKeywords(f, 1, "mac_type");
	break;
    case MM_ADD_PCAP:
	StoreField(f, PCAP_SPOOL_DIR, "/usr/spool/printer/");
	boolval(f, PCAP_AUTH) = TRUE;
	StoreField(f, PCAP_PRICE, "10");
	break;
    case MM_ADD_SERVICE:
	StoreField(f, SVC_INTERVAL, "1430");
	StoreField(f, SVC_TARGET, "/tmp/");
	StoreField(f, SVC_SCRIPT, "/moira/bin/");
	StoreField(f, 4, "UNIQUE");
	StoreField(f, 6, "NONE");
	GetKeywords(f, 4, "service");
	GetKeywords(f, 6, "ace_type");
	break;
    case MM_SHOW_ALIAS:
	StoreField(f, 2, "*");
	GetKeywords(f, 1, "alias");
	break;
    case MM_ADD_ZEPHYR:
	GetKeywords(f, 1, "ace_type");
	GetKeywords(f, 3, "ace_type");
	GetKeywords(f, 5, "ace_type");
	GetKeywords(f, 7, "ace_type");
	f->inputlines[ZA_XMT_TYPE]->valuechanged = MoiraValueChanged;
	f->inputlines[ZA_SUB_TYPE]->valuechanged = MoiraValueChanged;
	f->inputlines[ZA_IWS_TYPE]->valuechanged = MoiraValueChanged;
	f->inputlines[ZA_IUI_TYPE]->valuechanged = MoiraValueChanged;
	break;
    }

    f->menu = menu;
    DisplayForm(f);
}


/* Find a form by name.  Returns a pointer to the form or NULL on error. */

EntryForm *GetForm(name)
char *name;
{
    EntryForm **fp;

    for (fp = MoiraForms; *fp; fp++)
      if (!strcmp((*fp)->formname, name))
	break;
    return(*fp);
}


/* Find a form by name and clear all of its fields.  Returns a pointer
 * to the form or NULL on error.
 */

EntryForm *GetAndClearForm(name)
char *name;
{
    EntryForm **fp;
    UserPrompt *p, **pp;
    static BottomButton **buttons = NULL;
    int MoiraFormComplete(), MoiraFormApply();
    int button_callback(), help_form_callback();

    for (fp = MoiraForms; *fp; fp++)
      if (!strcmp((*fp)->formname, name))
	break;
    if (*fp == (EntryForm *)NULL)
      return(NULL);

    for (pp = (*fp)->inputlines; p = *pp; pp++) {
	switch (p->type) {
	case FT_STRING:
	case FT_KEYWORD:
	    if (p->returnvalue.stringvalue)
	      free(p->returnvalue.stringvalue);
	    p->returnvalue.stringvalue = (char *)malloc(MAXFIELDSIZE);
	    if (p->returnvalue.stringvalue == NULL) {
		fprintf(stderr, "Moira: out of memory\n");
		return(NULL);
	    }
	    *(p->returnvalue.stringvalue) = '\0';
	    break;
	case FT_BOOLEAN:
	    p->returnvalue.booleanvalue = False;
	    break;
	case FT_NUMBER:
	    p->returnvalue.integervalue = 0;
	    break;
	}
    }
    if (buttons == NULL) {
	buttons = (BottomButton **)malloc(5 * sizeof(BottomButton *));
	buttons[0] = (BottomButton *)malloc(sizeof(BottomButton));
	buttons[0]->label = "OK";
	buttons[0]->returnfunction = MoiraFormComplete;
	buttons[1] = (BottomButton *)malloc(sizeof(BottomButton));
	buttons[1]->label = "Apply";
	buttons[1]->returnfunction = MoiraFormApply;
	buttons[2] = (BottomButton *)malloc(sizeof(BottomButton));
	buttons[2]->label = "Cancel";
	buttons[2]->returnfunction = button_callback;
	buttons[3] = (BottomButton *)malloc(sizeof(BottomButton));
	buttons[3]->label = "Help";
	buttons[3]->returnfunction = help_form_callback;
	buttons[4] = NULL;
    }
    (*fp)->buttons = buttons;
    return(*fp);
}


/* Store a string into a given field in a form. */

StoreField(form, field, value)
EntryForm *form;
int field;
char *value;
{
    strncpy(form->inputlines[field]->returnvalue.stringvalue,
	    value, MAXFIELDSIZE);
}


char *StringValue(form, field)
EntryForm *form;
int field;
{
    switch (form->inputlines[field]->type) {
    case FT_STRING:
    case FT_KEYWORD:
	return(stringval(form, field));
    case FT_BOOLEAN:
	return(boolval(form, field) ? "1" : "0");
    }
}


StoreHost(form, field, dest)
EntryForm *form;
int field;
char **dest;
{
    char *s;

    s = strsave(stringval(form, field));
    s = canonicalize_hostname(s);
    StoreField(form, field, s);
    form->inputlines[field]->changed = True;
    UpdateForm(form);
    free(s);
    *dest = stringval(form, field);
}


/* Helper routine for GetKeywords() */

static get_alias(argc, argv, sq)
int argc;
char **argv;
struct save_queue *sq;
{
    sq_save_data(sq, strsave(argv[2]));
#ifdef DEBUG
    printf("found keyword %s\n", argv[2]);
#endif    
    return(MR_CONT);
}

static get_sloc(argc, argv, sq)
int argc;
char **argv;
struct save_queue *sq;
{
    sq_save_data(sq, strsave(argv[1]));
#ifdef DEBUG
    printf("found sloc %s\n", argv[1]);
#endif    
    return(MR_CONT);
}


/* Retrieve possible keyword values from Moira and store them in the specified
 * field of the form.  Returns 0 if successful, -1 if not.
 */

static struct save_queue *cache = NULL;
struct cache_elem { char *name; char **values; };

int GetKeywords(form, field, name)
EntryForm *form;
int field;
char *name;
{
    struct save_queue *sq, *s;
    struct cache_elem *ce;
    int i, size, stat;
    char *argv[4];

    /* init cache */
    if (cache == NULL)
      cache = sq_create();
    cache->q_lastget = NULL;

    if (!strcmp(name, "fsgroup")) {
	form->inputlines[field]->keywords = NULL;
    }

    /* look through cache */
    while (sq_get_data(cache, &ce))
      if (!strcmp(ce->name, name)) {
	  form->inputlines[field]->keywords = ce->values;
	  return(0);
      }

    /* not in cache, do query */
    sq = sq_create();
    if (!strcmp(name, "poserver")) {
	argv[0] = "POP";
	stat = MoiraQuery("get_server_locations", 1, argv, get_sloc,
			  (char *)sq);
	if (stat) {
	    com_err(program_name, stat, "retrieving service locations");
	    return(-1);
	}
    } else {
	argv[0] = name;
	argv[1] = "TYPE";
	argv[2] = "*";
	stat = MoiraQuery("get_alias", 3, argv, get_alias, (char *)sq);
	if (stat) {
	    com_err(program_name, stat, "retrieving keyword values");
	    return(-1);
	}
    }

    size = 1;
    for (s = sq->q_next; s->q_next != sq; s = s->q_next)
      size++;
#ifdef DEBUG
    printf("Found %d keywords of type %s\n", size, name);
#endif

    /* transform query data into proper malloc'ed structure */
    ce = (struct cache_elem *) malloc(sizeof(struct cache_elem));
    if (ce == NULL) {
	display_error("Out of memory while fetching keywords");
	return(-1);
    }
    ce->name = name;
    ce->values = (char **) malloc(sizeof(char *) * (size + 1));
    if (ce->values == NULL) {
	display_error("Out of memory while fetching keywords");
	return(-1);
    }
    for (i = 0; i < size; i++)
      sq_get_data(sq, &(ce->values[i]));
    ce->values[i] = NULL;
    sq_destroy(sq);

    /* put it in the cache */
    sq_save_data(cache, ce);

    /* and return answer */
    form->inputlines[field]->keywords = ce->values;
    return(0);
}

CacheNewValue(form, field, name, value)
EntryForm *form;
int field;
char *name;
char *value;
{
    struct save_queue *sq;
    struct cache_elem *ce;
    int size;

    /* init cache */
    if (cache == NULL)
      cache = sq_create();
    cache->q_lastget = NULL;

    /* find entry in cache */
    while (sq_get_data(cache, &ce))
      if (!strcmp(ce->name, name))
	break;

    /* get its size */
    for (size = 0; ce->values[size]; size++);

    /* add new keyword */
    ce->values = (char **)realloc(ce->values, sizeof(char *) * (size + 2));
    ce->values[size] = strsave(value);
    ce->values[size + 1] = NULL;

#ifdef DEBUG	
    printf("CacheNewValue(%x, %d, %s, %s)\n", form, field, name, value);
    printf("  form is %x (%s)\n", form, form->formname);
    printf("  field is %d (%s)\n", field, form->inputlines[field]->prompt);
    printf("  keywords: ");
    for (size = 0; ce->values[size]; size++) printf("%s ", ce->values[size]);
    printf("\n");
#endif /* DEBUG */

    /* new update form */
    form->inputlines[field]->keywords = ce->values;
    RemakeRadioField(form, field);
}
