/*
** Stub functions
*/

#include	<ctype.h>
#include	<X11/StringDefs.h>
#include	<X11/Intrinsic.h>
#include	<X11/IntrinsicP.h>
#include	<X11/Core.h>
#include	<X11/CoreP.h>
#include	<X11/CompositeP.h>
#include	<X11/cursorfont.h>
#include        <Xm/Text.h>
#include	<Xm/MessageB.h>
#include	"mmoira.h"
#include	<sys/file.h>

static char rcsid[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/stubs.c,v 1.9 1992-10-28 16:04:04 mar Exp $";

void	extra_help_callback();
extern EntryForm *MoiraForms[];
static DoMoiraSelect(), DoReference();

static Widget	logwidget = NULL;


/* No-op action to mask built-in actions */

void noopACT(w, event, p, n)
Widget w;
XEvent *event;
String *p;
Cardinal *n;
{
}


/* Action to do a moira retrieve on what was clicked at */

void moiraRetrieveACT(w, event, p, n)
Widget w;
XEvent *event;
String *p;
Cardinal *n;
{
    XmTextPosition pos;
    XButtonEvent *be;

    be = (XButtonEvent *)event;

    pos = XmTextXYToPos(w, be->x, be->y);
    DoMoiraSelect(pos, w, 0);
}


/* Action to modify the moira object that was clicked on */

void moiraModifyACT(w, event, p, n)
Widget w;
XEvent *event;
String *p;
Cardinal *n;
{
    XmTextPosition pos;
    XButtonEvent *be;

    be = (XButtonEvent *)event;

    pos = XmTextXYToPos(w, be->x, be->y);
    DoMoiraSelect(pos, w, 1);
}


static XtActionsRec myactions[] = {
    { "moiraRetrieve", moiraRetrieveACT },
    { "moiraModify", moiraModifyACT },
    { "noop", noopACT },
};


/* These are the additional translations added to the Motif text widget
 * for the log window.  The two noop translations are here to avoid
 * nasty interactions with the selection mechanism.  They match the
 * existing translations that involve shifted mouse buttons.
 */

#define newtrans "~Ctrl  Shift ~Meta ~Alt<Btn1Down>: moiraRetrieve()\n\
	~Ctrl ~Meta ~Alt<Btn1Up>: noop()\n\
	~Ctrl Shift ~Meta ~Alt<Btn1Motion>: noop()\n\
	~Ctrl  Shift ~Meta ~Alt<Btn2Down>: moiraModify()"


/* Create the log window, and setup the translation table to taking
 * mouse clicks in it.
 */

Widget
SetupLogWidget(parent)
Widget	parent;
{
	Arg		wargs[10];
	int		n;
	XtTranslations	trans;

	if (logwidget)
		return (logwidget);
	
	n = 0;
	XtSetArg(wargs[n], XmNeditMode, XmMULTI_LINE_EDIT);	n++;
	XtSetArg(wargs[n], XmNeditable, False);			n++;

	logwidget = XmCreateScrolledText(	parent,
						"logwidget", 
						wargs, n);
	XtManageChild(logwidget);

	XtAppAddActions(XtWidgetToApplicationContext(logwidget),
			myactions, XtNumber(myactions));
	trans = XtParseTranslationTable(newtrans);
	XtOverrideTranslations(logwidget, trans);
	return (logwidget);
}


/* This actually does all of the work for handling mouse clicks.  It
 * finds the surrounding text and decides what to do with the click,
 * the actually starts the moira query or pops up the form.
 */

static DoMoiraSelect(pos, w, modify)
XmTextPosition pos;
Widget w;
int modify;    
{
    char *log, *p, *p1, *p2;
    char name[256], type[256], type2[256];

    log = XmTextGetString(w);
    for (p = &log[pos]; p > log; p--)
      if (isspace(*p) || *p == ':') break;

    /* p now points to the beginning of the word on which the mouse was
     * clicked.  Begin gross hacking to find the name and type of object.
     */
    /* get name */
    p1 = p + 1;
    p2 = name;
    while (*p1 && !isspace(*p1)) *p2++ = *p1++;
    *p2 = 0;
    /* backup two words before for type2 */
    while (p >= log && isspace(*p) || *p == ':') p--;
    while (p >= log && !isspace(*p)) p--;
    while (p >= log && isspace(*p)) p--;
    while (p >= log && !isspace(*p)) p--;
    p++;
    p2 = type2;
    /* new get type2 */
    if (p > log) {
	while (!isspace(*p)) *p2++ = *p++;
	*p2++ = *p++;
	/* skip additional whitespace */
	while (isspace(*p)) p++;
    }
    /* get type & rest of type2 */
    p1 = type;
    while (*p && *p != ':' && !isspace(*p)) *p2++ = *p1++ = *p++;
    *p2 = *p1 = 0;
    /* Done! */
#ifdef DEBUG
    printf("name \"%s\", type \"%s\" type2 \"%s\"\n", name, type, type2);
#endif /* DEBUG */
    if (!strcasecmp(type, "USER") ||
	!strcmp(type, "name") ||
	!strcmp(type2, "Modified by") ||
	!strcmp(type2, "User Ownership") ||
	!strcmp(type2, "Login name") ||
	!strcmp(type, "login")) {
	DoReference(name, "select_user", MM_MOD_USER, MM_SHOW_USER,
		    "get_user_account_by_login", modify);
    } else if (!strcasecmp(type, "LIST") ||
	       !strcmp(type2, "Group Ownership")) {
	DoReference(name, "select_list", MM_MOD_LIST, MM_SHOW_LIST,
		    "get_list_info", modify);
    } else if (!strcasecmp(type, "MACHINE") ||
	       !strcmp(type, "host") ||
	       !strcmp(type, "Server") ||
	       !strcmp(type, "on") ||
	       !strcmp(type, "Box")) {
	DoReference(name, "select_machine", MM_MOD_MACH, MM_SHOW_MACH,
		    "get_machine", modify);
    } else if (!strcasecmp(type, "CLUSTER")) {
	DoReference(name, "select_cluster", MM_MOD_CLUSTER, MM_SHOW_CLUSTER,
		    "get_cluster", modify);
    } else if (!strcasecmp(type, "FILESYSTEM") ||
	       !strcmp(type, "FILESYS") ||
	       !strcmp(type2, "syslib Data")) {
	DoReference(name, "select_filsys", MM_MOD_FILSYS, MM_SHOW_FILSYS,
		    "get_filesys_by_label", modify);
    } else if (!strcmp(type, "Printer") ||
	       !strcmp(type2, "lpr Data")) {
	DoReference(name, "select_printer", MM_MOD_PCAP, MM_SHOW_PCAP,
		    "get_printcap_entry", modify);
    } else if (!strcmp(type, "Service")) {
	DoReference(name, "select_service", MM_MOD_SERVICE, MM_SHOW_SERVICE,
		    "get_server_info", modify);
    } else {
	XBell(XtDisplay(w), 100);
    }
    XtFree(log);
}


/* This is the heart of handling the reference once we have found out
 * what type of reference it is.
 */

static DoReference(name, formname, modop, showop, query, modify)
char *name;
char *formname;
int modop;
int showop;
char *query;
int modify;
{
    EntryForm *form, f;
    MenuItem m;
    char *argv[2], **aargv;
    int status;

    if (modify) {
	form = GetAndClearForm(formname);
	StoreField(form, 0, name);
	form->menu = &m;
	m.operation = modop;
	m.query = query;
	m.argc = 1;
	MoiraFormApply(0, form);
    } else {
	f.menu = &m;
	if (showop == MM_SHOW_FILSYS) f.extrastuff = (caddr_t) sq_create();
	argv[0] = name;
	m.operation = showop;
	status = MoiraQuery(query, 1, argv, DisplayCallback, &f);
	if (showop == MM_SHOW_FILSYS) {
	    while (sq_get_data(f.extrastuff, &aargv)) {
		ShowFilsys(aargv);
	    }
	    sq_destroy(f.extrastuff);
	}
	if (status)
	  com_err(program_name, status, " while looking up data");
	else
	  AppendToLog("\n");
    }
}


/*
** PopupErrorMessage(text)
**
** Given a char* pointing to an error message, possibly with imbedded
** newlines, display the text in a popup window and put two buttons
** at the bottom of the window, labelled "OK" and "Cancel."  Pop down
** when one of the buttons is pressed.
**
** Return 0 if "OK" is pressed, 1 for "Cancel."
*/

Boolean
PopupErrorMessage(text, extrahelp)
char	*text;
char	*extrahelp;
{
	static Widget		child;
	Arg		wargs[10];
	int		n;
	static XmString        label;

	if (label) {
		XtFree(label);
		XtDestroyWidget(child);
	}


	label = XmStringCreateLtoR( text, XmSTRING_DEFAULT_CHARSET);

	n = 0;
	XtSetArg(wargs[n], XmNmessageString, label);		n++;

	child = (Widget) XmCreateErrorDialog(logwidget, "errormessage", wargs, n);
	if (extrahelp) 
		XtAddCallback (child, XmNhelpCallback, extra_help_callback, extrahelp);
	else
		XtUnmanageChild(XmMessageBoxGetChild (child, XmDIALOG_HELP_BUTTON));
	XtUnmanageChild(XmMessageBoxGetChild (child, XmDIALOG_CANCEL_BUTTON));
	XtManageChild(child);
}

/*
** PopupHelpWindow(text)
**
** Given a char* pointing to an help message, possibly with imbedded
** newlines, display the text in a popup window and put a single button
** at the bottom of the window, labelled "OK."  Pop down when the
** the buttons is pressed.
*/

void
PopupHelpWindow(text)
char	*text;
{
	static Widget		child;
	Arg		wargs[10];
	int		n;

	static XmString        label;
	if (label) {
		XtFree(label);
		XtDestroyWidget(child);
	}

	label = XmStringCreateLtoR( text, XmSTRING_DEFAULT_CHARSET);

	n = 0;
	XtSetArg(wargs[n], XmNmessageString, label);		n++;

	child = (Widget) XmCreateMessageDialog(logwidget, "helpmessage", wargs, n);
	XtUnmanageChild(XmMessageBoxGetChild (child, XmDIALOG_CANCEL_BUTTON));
	XtUnmanageChild(XmMessageBoxGetChild (child, XmDIALOG_HELP_BUTTON));

	XtManageChild(child);
}

/*
** Given a char* to a single line of text, append this line at the bottom
** of the log window.  Return 0 of the append was sucessful, non-zero
** for an error condition.
*/

int
AppendToLog(text)
char	*text;
{
	XmTextPosition	pos;
	char		*string, *p;

	string = XmTextGetString(logwidget);
	pos = strlen(string);
	XtFree(string);

	XmTextReplace(logwidget, pos, pos, text);
	XmTextSetCursorPosition(logwidget, pos + strlen(text));
	if (pos > MAXLOGSIZE) {
	    for (p = &string[pos - MAXLOGSIZE]; *p && *p != '\n'; p++);
	    if (*p)
	      pos = p - string;
	    XmTextReplace(logwidget, 0, pos, "");
	}
}

void
MakeWatchCursor(topW)
Widget	topW;
{
    static Cursor mycursor = NULL;
    EntryForm **fp;

    if (!topW)
      return;

    if (!mycursor)
      mycursor = XCreateFontCursor (XtDisplay(topW), XC_watch);

    XDefineCursor(XtDisplay(topW), XtWindow(topW), mycursor);
    for (fp = MoiraForms; *fp; fp++)
      if ((*fp)->formpointer && XtIsManaged((*fp)->formpointer))
	XDefineCursor(XtDisplay(topW), XtWindow((*fp)->formpointer), mycursor);
}

void
MakeNormalCursor(topW)
Widget	topW;
{
    EntryForm **fp;

    if (!topW)
      return;

    XUndefineCursor(XtDisplay(topW), XtWindow(topW));
    for (fp = MoiraForms; *fp; fp++)
      if ((*fp)->formpointer && XtIsManaged((*fp)->formpointer))
	XUndefineCursor(XtDisplay(topW), XtWindow((*fp)->formpointer));
}

/*
** Move through the fields of the spec and make certain that the
** form's widgets actually reflect the current values.
*/

void
UpdateForm(spec)
EntryForm	*spec;
{
	UserPrompt      **myinputlines = spec->inputlines;
	UserPrompt	*current;
	Arg		wargs[10];
	int		n, kidcount;
	Widget		kid;

	if (spec->formpointer == NULL) return;

	for (	current = (*myinputlines);
		current; 
		myinputlines++, current = (*myinputlines)) {

		if (current->changed) {
		    n = 0;
		    XtSetArg(wargs[n], XmNsensitive,
			     current->insensitive ? False : True); n++;
		    XtSetValues(current->mywidget, wargs, n);
		    current->changed = False;
		}

		switch (current->type) {
		case FT_STRING:
			if (current->returnvalue.stringvalue) {
				XmTextSetString (current->mywidget, current->returnvalue.stringvalue);
			}
			break;

		case FT_BOOLEAN:
			n = 0;
			XtSetArg(wargs[n], XmNset,
				 current->returnvalue.booleanvalue ? True : False);	n++;
			XtSetValues (current->mywidget, wargs, n);
			break;

		case FT_KEYWORD:
			kidcount = ((CompositeRec *)(current->mywidget))->
					composite.num_children;

			while(kidcount--) {
				n = 0;
				kid = ((CompositeRec *)(current->mywidget))->
					composite.children[kidcount];
				if (current->returnvalue.stringvalue &&
					(!strcmp (XtName(kid), current->returnvalue.stringvalue))) {
					XtSetArg(wargs[n], XmNset, True);
					n++;
				}
				else {
					XtSetArg(wargs[n], XmNset, False);
					n++;
				}
				XtSetValues (kid, wargs, n);
			}
			break;

		case FT_NUMBER:
			break;
		}
		n = 0;
		XtSetArg(wargs[n], XtNsensitive, !(current->insensitive));		n++;
		XtSetValues (current->mywidget, wargs, n);
	}
}

void
extra_help_callback(w, client_data, call_data)
Widget	w;
char	*client_data;
XmAnyCallbackStruct	*call_data;
{
    help(client_data);
}


int write_log_to_file(fn)
char *fn;
{
    char *string, buf[256];
    int fd, i;
    extern int errno;

    string = XmTextGetString(logwidget);
    fd = open(fn, O_WRONLY|O_CREAT, 0666);
    if (fd < 0) {
	sprintf(buf, "opening output file \"%s\"", fn);
	com_err(program_name, errno, buf);
	return(1);
    }
    if ((i = write(fd, string, strlen(string))) < 0) {
	sprintf(buf, "writing output file \"%s\"", fn);
	com_err(program_name, errno, buf);
	return(1);
    }
    if ((i = close(fd)) < 0) {
	sprintf(buf, "closing output file \"%s\"", fn);
	com_err(program_name, errno, buf);
	return(1);
    }
    return(0);
}


void yesCallback(w, ret, dummy)
Widget w;
XtPointer ret;
XtPointer dummy;
{
    int *ip = (int *)ret;
    *ip = 1;
}

void noCallback(w, ret, dummy)
Widget w;
XtPointer ret;
XtPointer dummy;
{
    int *ip = (int *)ret;
    *ip = -1;
}


static int value;
static XtCallbackRec yescb[] = { { yesCallback, (XtPointer) &value },
				 { NULL, NULL} };
static XtCallbackRec nocb[]  = { { noCallback, (XtPointer) &value },
				 {NULL, NULL} };

Boolean AskQuestion(text, helpname)
char *text, *helpname;
{
	static Widget		child;
	Arg		wargs[10];
	int		n;
	static XmString        label, yes = NULL, no;
	XEvent	event;
	XtAppContext _XtDefaultAppContext();

	if (!yes) {
	    yes = XmStringCreate("Yes", XmSTRING_DEFAULT_CHARSET);
	    no = XmStringCreate("No", XmSTRING_DEFAULT_CHARSET);
	}
	if (label) {
		XtFree(label);
		XtDestroyWidget(child);
	}

	label = XmStringCreateLtoR( text, XmSTRING_DEFAULT_CHARSET);

	n = 0;
	XtSetArg(wargs[n], XmNmessageString, label);		n++;
	XtSetArg(wargs[n], XmNokLabelString, yes);		n++;
	XtSetArg(wargs[n], XmNcancelLabelString, no);		n++;
	XtSetArg(wargs[n], XmNokCallback, yescb);		n++;
	XtSetArg(wargs[n], XmNcancelCallback, nocb);		n++;

	child = (Widget) XmCreateQuestionDialog(logwidget, "question", wargs, n);
	if (helpname) 
		XtAddCallback (child, XmNhelpCallback, extra_help_callback, helpname);
	else
		XtUnmanageChild(XmMessageBoxGetChild (child, XmDIALOG_HELP_BUTTON));

	XtManageChild(child);
	value = 0;
	while (value == 0) {
	    XtAppNextEvent(_XtDefaultAppContext(), &event);
	    XtDispatchEvent(&event);
	}
	if (value > 0)
	  return(1);
	else
	  return(0);
}



/******* temporary ********/
display_error(msg)
char *msg;
{
    PopupErrorMessage(msg, "no_more_help");
}
