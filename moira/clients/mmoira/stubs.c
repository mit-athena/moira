/*
** Stub functions
*/

#include	<X11/StringDefs.h>
#include	<X11/Intrinsic.h>
#include	<X11/IntrinsicP.h>
#include	<X11/Core.h>
#include	<X11/CoreP.h>
#include	<X11/CompositeP.h>
#include	<X11/cursorfont.h>
#include        <Xm/Text.h>
#include	"mmoira.h"
#include	<sys/file.h>

static char rcsid[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/stubs.c,v 1.5 1992-10-13 11:11:37 mar Exp $";

void	extra_help_callback();

static Widget	logwidget = NULL;

Widget
SetupLogWidget(parent)
Widget	parent;
{
	Arg		wargs[10];
	int		n;

	if (logwidget)
		return (logwidget);
	
	n = 0;
	XtSetArg(wargs[n], XmNeditMode, XmMULTI_LINE_EDIT);	n++;
	XtSetArg(wargs[n], XmNeditable, False);			n++;

	logwidget = XmCreateScrolledText(	parent,
						"logwidget", 
						wargs, n);
	XtManageChild(logwidget);

	return (logwidget);
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
	Cursor	mycursor;

	if (!topW)
		return;

	mycursor = XCreateFontCursor (XtDisplay(topW), XC_watch);
	XDefineCursor(XtDisplay(topW), XtWindow(topW), mycursor);
}

void
MakeNormalCursor(topW)
Widget	topW;
{
	if (!topW)
		return;

	XUndefineCursor(XtDisplay(topW), XtWindow(topW));
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
	PopupHelpWindow(client_data);
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


yesCallback(w, ret, dummy)
Widget w;
int *ret;
{
    *ret = 1;
}

noCallback(w, ret, dummy)
Widget w;
int *ret;
{
    *ret = -1;
}


static int value;
static XtCallbackRec yescb[] = { { yesCallback, &value }, {NULL, NULL} };
static XtCallbackRec nocb[]  = { { noCallback, &value }, {NULL, NULL} };

Boolean AskQuestion(text, helpname)
char *text, helpname;
{
	static Widget		child;
	Arg		wargs[10];
	int		n;
	static XmString        label, yes = NULL, no;
	XEvent	event;

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
    PopupErrorMessage(msg, "Sorry, no further help is available");
}
