/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/form_setup.c,v 1.3 1991-06-05 11:22:47 mar Exp $
 */

#include <stdio.h>
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
	break;
    case MM_ADD_USER:
	StoreField(f, U_NAME, UNIQUE_LOGIN);
	StoreField(f, U_UID, UNIQUE_UID);
	StoreField(f, U_SHELL, "/bin/csh");
	StoreField(f, U_STATE, user_states[US_NO_LOGIN_YET]);
	f->inputlines[U_STATE]->keywords = user_states;
	GetKeywords(f, U_CLASS, "class");
	break;
    case MM_SET_POBOX:
	GetKeywords(f, 1, "pobox");
	GetKeywords(f, 3, "poserver");
	break;
    case MM_ADD_FILSYS:
	StoreField(f, FS_TYPE, "NFS");
	StoreField(f, FS_M_POINT, "/mit/");
	StoreField(f, FS_ACCESS, "w");
	StoreField(f, FS_OWNER, user);
	StoreField(f, FS_OWNERS, user);
	boolval(f, FS_CREATE) = TRUE;
	GetKeywords(f, FS_TYPE, "filesys");
	GetKeywords(f, FS_ACCESS, "fs_access_NFS");
	GetKeywords(f, FS_L_TYPE, "lockertype");
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
    int MoiraFormComplete(), button_callback(), help_form_callback();

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
	buttons = (BottomButton **)malloc(4 * sizeof(BottomButton *));
	buttons[0] = (BottomButton *)malloc(sizeof(BottomButton));
	buttons[0]->label = "OK";
	buttons[0]->returnfunction = MoiraFormComplete;
	buttons[1] = (BottomButton *)malloc(sizeof(BottomButton));
	buttons[1]->label = "Cancel";
	buttons[1]->returnfunction = button_callback;
	buttons[2] = (BottomButton *)malloc(sizeof(BottomButton));
	buttons[2]->label = "Help";
	buttons[2]->returnfunction = help_form_callback;
	buttons[3] = NULL;
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

    s = strsave(stringval(form, 0));
    s = canonicalize_hostname(s);
    StoreField(form, 0, s);
    free(s);
    *dest = stringval(form, 0);
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

int GetKeywords(form, field, name)
EntryForm *form;
int field;
char *name;
{
    static struct save_queue *cache = NULL;
    struct save_queue *sq, *s;
    struct cache_elem { char *name; char **values; } *ce;
    int i, size, stat;
    char *argv[4];

    /* init cache */
    if (cache == NULL)
      cache = sq_create();
    cache->q_lastget = NULL;

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
