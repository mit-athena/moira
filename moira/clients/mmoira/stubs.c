/*
** Stub functions
*/

#include	<X11/Intrinsic.h>
#include	"mmoira.h"
#include        <Xm/Text.h>

static Widget	logwidget = NULL;

Widget
SetupLogWidget(parent)
Widget	parent;
{
	if (logwidget)
		return (logwidget);
	
	logwidget = XtCreateManagedWidget("logwidget", 
						xmTextWidgetClass,
						parent, NULL, 0);
	return (logwidget);
}

/*
** PopupErrorMessage(text)
**
** Given a char* pointing to an error message, possibly with imbedded
** newlines, display the text in a popup window and put two buttons
** at the bottom of the window, labelled "OK" and "Abort."  Pop down
** when one of the buttons is pressed.
**
** Return 0 if "OK" is pressed, 1 for "Abort."
*/

int
PopupErrorMessage(text)
char	*text;
{
	printf ("STUB:  error, %s\n",text);
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
	printf ("STUB:  help message, %s\n",text);
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
	
}

/*
** Given a pointer to a form, update the displayed values of the input 
** fields and their sensitivity.  QUESTION:  Should I also map the form?
*/

void
UpdateForm(formptr)
EntryForm	*formptr;
{
	printf ("STUB:  Updated form %s\n", formptr->formname);
}

void
MakeWatchCursor()
{
}

void
MakeNormalCursor()
{
}
