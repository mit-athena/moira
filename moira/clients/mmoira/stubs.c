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
#include	"data.h"
#include        <Xm/Text.h>

static char rcsid[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/stubs.c,v 1.3 1991-06-04 14:31:44 mar Exp $";

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
	Widget		child;
	Arg		wargs[10];
	int		n;
	XmString        label;          /* !@#$%^ compound string required */


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
	Widget		child;
	Arg		wargs[10];
	int		n;
	XmString        label;          /* !@#$%^ compound string required */


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
	char		*string;

	string = XmTextGetString(logwidget);
	pos = strlen(string);
	XtFree(string);

	XmTextReplace(logwidget, pos, pos, text);
	XmTextSetCursorPosition(logwidget, pos + strlen(text));
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


		switch (current->type) {
		case FT_STRING:
			if (current->returnvalue.stringvalue) {
				XmTextSetString (current->mywidget, current->returnvalue.stringvalue);
			}
			break;

		case FT_BOOLEAN:
			n = 0;
			XtSetArg(wargs[n], XmNset, current->returnvalue.booleanvalue);	n++;
			XtSetValues (current->mywidget, wargs, n);
			break;

		case FT_KEYWORD:
			kidcount = ((CompositeRec *)(current->mywidget))->
					composite.num_children;
			printf ("Keyword field has %d children\n", kidcount);

			while(kidcount--) {
				n = 0;
				kid = ((CompositeRec *)(current->mywidget))->
					composite.children[kidcount];
				if (!strcmp (XtName(kid), current->returnvalue.stringvalue)) {
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
