#include	<stdio.h>
#include	<X11/StringDefs.h>
#include	<X11/IntrinsicP.h>
#include	<X11/Shell.h>
#include	<X11/Core.h>
#include	<X11/CoreP.h>
#include	<Xm/Xm.h>
#include	<Xm/BulletinB.h>
#include	<Xm/Label.h>
#include	<Xm/Text.h>
#include	<Xm/PushB.h>
#include	<Xm/PushBG.h>
#include	<Xm/CascadeB.h>
#include	<Xm/ToggleB.h>
#include	<Xm/ToggleBG.h>
#include	<Xm/RowColumn.h>
#include	<Xm/Separator.h>
#include	"mmoira.h"

static char rcsid[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/formup.c,v 1.4 1991-06-04 14:33:08 mar Exp $";

#define	MAX(a,b)	((a > b) ? a : b)
#define	MIN(a,b)	((a < b) ? a : b)

int	hpad = 10;
int	vpad = 5;

void	manage_widget();
Widget	CreateForm();
Widget	CreateMenu();
Widget	BuildMenuTree();
Widget	MakeRadioField();
int	button_callback();
void	radio_callback();
void	string_callback();
void	boolean_callback();
void	menu_callback();

extern void	UpdateForm();
extern int	PopupErrorMessage();
extern void	PopupHelpWindow();
extern int	AppendToLog();
extern void	MakeWatchCursor();
extern void	MakeNormalCursor();
extern Widget	SetupLogWidget();


void
manage_widget(w, widget, call_data)
Widget	w, widget;
XmAnyCallbackStruct	*call_data;
{
	XtManageChild(widget);	
}

int
button_callback(w, f, call_data)
Widget	w;
EntryForm *f;
XmAnyCallbackStruct	*call_data;
{
	char	output[100];
	static int	mode = 0;

	XtUnmanageChild(f->formpointer);	
}


Widget
BuildMenuTree(topW, spec)
Widget		topW;
MenuItem	*spec;
{
	return (CreateMenu(topW, spec->submenu, XmHORIZONTAL));
}

/*
** Read the specification and put up a menu to match...
*/

Widget
CreateMenu(parent, spec, orientation)
Widget		parent;
MenuItem	**spec;
int		orientation;
{
	Widget		menuparent;
	MenuItem	*curmenuitem;
	Widget		childbutton, childmenu;
	Arg		wargs[10];
	int		n;
	XmString	label;		/* !@#$%^ compound string required */

	label = XmStringCreate(	"Complete junk", XmSTRING_DEFAULT_CHARSET);

	n = 0;
	XtSetArg(wargs[n], XmNlabelString, label);	n++;

	if (orientation == XmHORIZONTAL) {
		XtSetArg(wargs[n], XmNspacing, 5);	n++;
		menuparent = XmCreateMenuBar(	parent, "randommenu", 
					wargs, n);
		XtManageChild(menuparent);
	}
	else
		menuparent = XmCreatePulldownMenu(parent, "randommenu", 
						wargs, n);

	for (	curmenuitem = (*spec);
		curmenuitem;
		spec++, curmenuitem = (*spec)) {

#ifdef	DEBUG
		printf ("Making entry:  %s\n", curmenuitem->label);
#endif
		label = XmStringCreate(	curmenuitem->label,
					XmSTRING_DEFAULT_CHARSET);
		n = 0;
		XtSetArg(wargs[n], XmNlabelString, label);	n++;

		if (curmenuitem->submenu) {
#ifdef	DEBUG
			printf ("It has a submenu, which I'm recursing on...\n");
#endif
			childmenu = CreateMenu(	menuparent, 
						curmenuitem->submenu,
						XmVERTICAL);
			XtSetArg(wargs[n], XmNsubMenuId, childmenu);	n++;

			childbutton = XtCreateManagedWidget(	"child",
					xmCascadeButtonWidgetClass,
					menuparent, wargs, n);

		}

		else {
			childbutton = XtCreateManagedWidget(	"child",
					xmPushButtonGadgetClass,
					menuparent, wargs, n);

			XtAddCallback(	childbutton, 
					XmNactivateCallback, 
					menu_callback, curmenuitem);
		}
	}

	return (menuparent);
}

/*
** Read the specification and put up a form to match...
*/

Widget
CreateForm(parent, spec)
Widget		parent;
EntryForm	*spec;
{
	Widget		bb;
	Arg		wargs[10];
	int		n;
	XmString	label;		/* compound string required */
	Dimension	height_so_far = 0, width_so_far = 0;
	Dimension	height, width;
	Widget		titleW, instructionW;
	Position	x, y;
	Widget		shellparent;

	if (spec->formpointer) {
		printf ("Form %s already exists\n", spec->formname);
		UpdateForm(spec);
		return(spec->formpointer);
	}

#define GETSIZE(foo)	n = 0; \
			XtSetArg(wargs[n], XtNwidth, &width);	n++; \
			XtSetArg(wargs[n], XtNheight, &height);	n++; \
			XtGetValues (foo, wargs, n); \

#define STORESIZE	if (width > width_so_far) width_so_far = width;\
			height_so_far += height + vpad;


	n = 0;
	XtSetArg(wargs[n], XmNautoUnmanage, False);		n++;
	bb = XmCreateBulletinBoardDialog(parent, "board", wargs, n);

	spec->formpointer = bb;

#ifdef FORMTITLES
	label = XmStringCreate(spec->formname, XmSTRING_DEFAULT_CHARSET);
	n = 0;
	XtSetArg(wargs[n], XmNlabelString, label);		n++;
	XtSetArg(wargs[n], XtNx, 0);				n++;
	XtSetArg(wargs[n], XtNy, 0);				n++;
	titleW = XtCreateManagedWidget(	"title",
				xmLabelWidgetClass,
				bb, wargs, n);
	GETSIZE(titleW);
	STORESIZE;
#endif

	label = XmStringCreate(spec->instructions, XmSTRING_DEFAULT_CHARSET);
	n = 0;
	XtSetArg(wargs[n], XmNlabelString, label);		n++;
	XtSetArg(wargs[n], XtNx, 0);				n++;
	XtSetArg(wargs[n], XtNy, height_so_far);		n++;
	instructionW = XtCreateManagedWidget(	"title",
				xmLabelWidgetClass,
				bb, wargs, n);
	GETSIZE(instructionW);
	STORESIZE;

	height = height_so_far;
	width = width_so_far;
	MakeInputLines(bb, &height, &width, spec);
	STORESIZE;

	height = height_so_far;
	width = width_so_far;
	MakeButtons(bb, &height, &width, spec);
	STORESIZE;

#ifdef FORMTITLES
/*
** Center the title of the form
*/
	n = 0;
	XtSetArg(wargs[n], XtNwidth, &width);			n++;
	XtGetValues (titleW, wargs, n);

	x = (width_so_far - width) / 2;

	n = 0;
	XtSetArg(wargs[n], XtNx, x);				n++;
	XtSetValues (titleW, wargs, n);
#endif
	return((Widget) bb);
}

/*
** Pheight and pwidth start with the values-to-date of the bboard so far.
** Return your height and width in them when you're done.
**
** Positioning the widgets happens in two phases: 
**	First, we set their y-positions as we create them.
**	After they're created, we go back and adjust the x-positions
**	according to the widest left side noted.
*/

MakeInputLines(parent, pheight, pwidth, spec)
Widget		parent;
Dimension	*pheight;
Dimension	*pwidth;
EntryForm	*spec;
{
	UserPrompt	*current;
	XmString	label;		/* compound string required */
	Arg		wargs[10];
	int		i, n;
	Widget		child;
	Dimension	width, height, maxleftwidth = 0, maxrightwidth = 0;
	Dimension	localy, leftheight = 0, rightheight = 0;
	UserPrompt      **myinputlines = spec->inputlines;
	int		foo = 30;
	Widget		children[20];

	for (	current = (*myinputlines), localy = 0,  i = 0;
		current; 
		myinputlines++, current = (*myinputlines), i++) {


#ifdef	DEBUG
		printf ("Making entry %d: %s of type %d\n", 
				i, current->prompt, current->type);
#endif
/*
** First, make the prompt
*/
		label = XmStringCreate(	current->prompt, 
					XmSTRING_DEFAULT_CHARSET);
		n = 0;
		XtSetArg(wargs[n], XmNlabelString, label);	n++;
		XtSetArg(wargs[n], XtNy, localy + *pheight);	n++;
		child = XtCreateManagedWidget(	"prompt",
				xmLabelWidgetClass,
				parent, wargs, n);

		GETSIZE(child);
		leftheight = height;
		if (width > maxleftwidth)
			maxleftwidth = width;

/*
** Second, make the input widget
*/
		n = 0;
		XtSetArg(wargs[n], XtNy, localy + *pheight);	n++;
		XtSetArg(wargs[n], XmNtraversalOn, True);	n++;
		XtSetArg(wargs[n], XtNsensitive, 
			!(current->insensitive));		n++;
		switch (current->type) {
		case FT_STRING:
			children[i] = XtCreateManagedWidget(	"child",
						xmTextWidgetClass,
						parent, wargs, n);
			XtAddCallback(	children[i], XmNlosingFocusCallback,
				string_callback, current);
			if (current->returnvalue.stringvalue) {
				XmTextSetString (children[i], current->returnvalue.stringvalue);
			}
			GETSIZE (children[i]);
			rightheight = height;
			if (width > maxrightwidth)
				maxrightwidth = width;
			break;

		case FT_BOOLEAN:
			XtSetArg(wargs[n], XmNset, current->returnvalue.booleanvalue);	n++;

			if (current->returnvalue.booleanvalue)
				label = XmStringCreate(	"(True)", XmSTRING_DEFAULT_CHARSET);
			else
				label = XmStringCreate(	"(False)", XmSTRING_DEFAULT_CHARSET);
			XtSetArg(wargs[n], XmNlabelString, label);	n++;

			children[i] = XtCreateManagedWidget(	"ignore this",
						xmToggleButtonWidgetClass,
						parent, wargs, n);

			XtAddCallback(	children[i], XmNvalueChangedCallback,
				boolean_callback, current);

			GETSIZE (children[i]);
			rightheight = height;
			if (width > maxrightwidth)
				maxrightwidth = width;
			break;

		case FT_KEYWORD:
			children[i] = 
				MakeRadioField(parent, current, &rightheight);
			XtManageChild(children[i]);
			XtSetValues(children[i], wargs, n);
			GETSIZE (children[i]);
			if (width > maxrightwidth)
				maxrightwidth = width;
			break;

		default:
			printf ("Sorry, don't recognize that type\n");
			break;
		}
		XmAddTabGroup(children[i]);
		current->mywidget = children[i];

		localy += MAX(rightheight, leftheight) + vpad;
	}

/*
** Now slide the input widgets right as far as the widest prompt.
*/
	n = 0;
	XtSetArg(wargs[n], XtNx, maxleftwidth + hpad);	n++;
	for (; i; i--)
		XtSetValues (children[i - 1], wargs, n);

	*pheight = localy - vpad;
	*pwidth = maxleftwidth + maxrightwidth + hpad;
}

/*
** All the junk about keeping track of the sum of the children's heights
** is because the !#$% RowColumn widget doesn't sum them for us, NOR
** does it accept SetValues on its XtNHeight!  Thanks, Motif!
*/

Widget
MakeRadioField(parent, prompt, pheight)
Widget		parent;
UserPrompt	*prompt;
Dimension	*pheight;
{
	Widget	radioparent, child;
	char	*current;
	Arg	wargs[10];
	int	count, n;
	XmString	label;	/* accursed compound string required */
	Dimension	height, width;
	char	**keywords;


	if (!prompt->keywords) {
		fprintf (stderr, "Warning:  No list of keywords for widget\n");
		return;
	}
	for (	count = 0, keywords = prompt->keywords;
		*keywords; 
		keywords++, count++);

/*
** Although the XmNnumColumns resource is documented as actually
** representing the number of _rows_ when XmNorientation is set to XmVERTICAL,
** it doesn't.  So I need to count the items myself and manually set the
** number of columns to get a maximum of five rows.  There's no XmNnumRows
** resource.  Thanks, Motif!
*/

	n = 0;
	XtSetArg(wargs[n], XmNspacing, 0);	n++;

	if (count > 5) {
		printf ("Special case:  Asking for %d columns\n",1 + count/5);
		XtSetArg(wargs[n], XmNnumColumns, 1 + count / 5);		n++;
		XtSetArg(wargs[n], XmNorientation, XmVERTICAL);	n++;
		XtSetArg(wargs[n], XmNpacking, XmPACK_COLUMN);	n++;
	}

	radioparent = XmCreateRadioBox(parent, "radio", wargs, n);

	keywords = prompt->keywords;
	for (current=(*keywords); current; keywords++, current=(*keywords)) {
		n = 0;
		label = XmStringCreate(current, XmSTRING_DEFAULT_CHARSET);
		XtSetArg(wargs[n], XmNlabelString, label);	n++;
		if (!strcmp (current, prompt->returnvalue.stringvalue)) {
			XtSetArg(wargs[n], XmNset, True);	n++;
		}
		else {
			XtSetArg(wargs[n], XmNset, False);	n++;
		}
		child = XtCreateManagedWidget(	current,
						xmToggleButtonWidgetClass,
						radioparent, wargs, n);

		XtAddCallback(	child, XmNvalueChangedCallback,
				radio_callback, prompt);

	}
/*
** Assume all child widgets are the same height.  Increase height by
** five times this, or the actual number of children, whichever is lesser.
*/

	GETSIZE (child);
	*pheight = (height * MIN(5, count));

	return(radioparent);
}

MakeButtons(parent, pheight, pwidth, spec)
Widget		parent;
Dimension	*pheight;
Dimension	*pwidth;
EntryForm	*spec;
{
	BottomButton	*current;
	XmString	label;		/* compound string required */
	Arg		wargs[10];
	int		i, n;
	Dimension	newwidth, width = 25;
	Widget		newbutton;
	BottomButton	**buttons = spec->buttons;

	*pheight += vpad;

	n = 0;
	XtSetArg(wargs[n], XtNy, *pheight);			n++;
	XtSetArg(wargs[n], XtNx, 0);				n++;
	XtSetArg(wargs[n], XtNwidth, *pwidth);			n++;
	XtCreateManagedWidget(	"separator",
				xmSeparatorWidgetClass,
				parent, wargs, n);
	*pheight += vpad;

	for (	current=(*buttons); 
		current; 
		buttons++, current=(*buttons)) {

#ifdef	DEBUG
		printf ("Making a button labeled %s\n", current->label);
#endif
		label = XmStringCreate(	current->label, 
					XmSTRING_DEFAULT_CHARSET);
		n = 0;
		XtSetArg(wargs[n], XtNy, (*pheight));			n++;
		XtSetArg(wargs[n], XtNx, width);			n++;
		XtSetArg(wargs[n], XmNlabelString, label);		n++;

		newbutton = XtCreateManagedWidget(	current->label,
						xmPushButtonWidgetClass,
						parent, wargs, n);

		XtAddCallback(	newbutton, XmNactivateCallback,
				current->returnfunction, spec);
		n = 0;
		XtSetArg(wargs[n], XtNwidth, &newwidth);		n++;
		XtGetValues (newbutton, wargs, n);

		width += (newwidth + hpad);
	}

	(*pheight) += 100;
}

void
radio_callback(w, client_data, call_data)
Widget	w;
XmAnyCallbackStruct	*client_data;
XmAnyCallbackStruct	*call_data;
{
	Arg		wargs[10];
	int		n;
	Boolean		is_set;

	UserPrompt	*prompt = (UserPrompt *) client_data;

	n = 0;
	XtSetArg(wargs[n], XmNset, &is_set);	n++;
	XtGetValues (w, wargs, n);

	if (!is_set)
		return;

/*
** Since Motif insists on using !@#$% Compound Strings as the text for
** its label widgets, but doesn't provide a way of getting a char* back
** from a !@#$% Compound String, I can't retrieve the label of the button 
** that was hit. 
**
** Fortunately, I was smart enough to use the button label as the name 
** of the widget, and I can extract it via XtName().  Thanks, Motif!
*/
	if (strcmp(prompt->returnvalue.stringvalue, XtName(w))) {
		printf ("Replacing old value of selection, '%s', with '%s'\n",
				prompt->returnvalue.stringvalue,
				XtName(w));
		strcpy(prompt->returnvalue.stringvalue, XtName(w));
	}

}

void
boolean_callback(w, client_data, call_data)
Widget	w;
XmAnyCallbackStruct	*client_data;
XmAnyCallbackStruct	*call_data;
{
	Arg		wargs[10];
	int		n;
	Boolean		is_set;
	UserPrompt	*current = (UserPrompt *)client_data;
	XmString	label;

	n = 0;
	XtSetArg(wargs[n], XmNset, &is_set);			n++;
	XtGetValues (w, wargs, n);

	current->returnvalue.booleanvalue = is_set;

	if (is_set)
		label = XmStringCreate(	"(True)", XmSTRING_DEFAULT_CHARSET);
	else
		label = XmStringCreate(	"(False)", XmSTRING_DEFAULT_CHARSET);
	n = 0;
	XtSetArg(wargs[n], XmNlabelString, label);		n++;
	XtSetValues (w, wargs, n);

#if DEBUG
	printf ("boolean_callback:  button %x is %s\n", 
			w, (is_set ? "True" : "False"));
#endif
}

void
menu_callback(w, client_data, call_data)
Widget	w;
XmAnyCallbackStruct	*client_data;
XmAnyCallbackStruct	*call_data;
{
	MenuItem	*itemhit = (MenuItem *) client_data;

	MoiraMenuRequest(itemhit);
}


void
string_callback(w, client_data, call_data)
Widget	w;
XmAnyCallbackStruct	*client_data;
XmAnyCallbackStruct	*call_data;
{
	UserPrompt	*current = (UserPrompt *)client_data;
	char		*newvalue;

	newvalue = XmTextGetString(w);

	if (strcmp(current->returnvalue.stringvalue, newvalue)) {
		printf ("Replacing old value of selection, '%s', with '%s'\n",
				current->returnvalue.stringvalue,
				newvalue);
		strcpy(current->returnvalue.stringvalue, newvalue);
	}
	XtFree(newvalue);
}
