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

#define	HPADDING		8
#define	WPADDING		20
#define	MAX(a,b)	((a > b) ? a : b)

extern Widget toplevel;

void	map_menu_widget();
void	manage_widget();
Widget	CreateForm();
Widget	CreateMenu();
Widget	BuildMenuTree();
int	button_callback();
int	radio_callback();
void	menu_callback();
void	post_menu_handler();


/*
** Read the specification and put up a menu to match...
**
** Something in here is making a bad button grab...It either gets the
** error, 
**        "BadAccess (attempt to access private resource denied)
**         Major opcode of failed request:  28 (X_GrabButton)
**
** or puts up the menu and permanently grabs the pointer!
**
** All Motif pulldown menus are replaced with homebrews until I get an
** updated widget set to try out.
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
	Widget		shellparent;

	label = XmStringCreate(	"Complete junk", XmSTRING_DEFAULT_CHARSET);

	n = 0;

/* MOTIF menus are broken.
	XtSetArg(wargs[n], XmNlabelString, label);	n++;
	menuparent = XmCreatePopupMenu(parent, "randommenu", wargs, n);
*/

	XtSetArg(wargs[n], XtNmappedWhenManaged, False);	n++;
	shellparent = XtCreateApplicationShell(	"shellparent", 
						transientShellWidgetClass,
						wargs, n);

	n = 0;
	if (orientation)
		XtSetArg(wargs[n], XtNorientation, orientation);	n++;
	menuparent = XtCreateManagedWidget (	"row", 
					xmRowColumnWidgetClass,
					shellparent, wargs, n);
						

/*
	XtAddEventHandler (	parent, ButtonPressMask, FALSE,
				post_menu_handler, menuparent);
*/
	for (	curmenuitem = (*spec);
		curmenuitem;
		spec++, curmenuitem = (*spec)) {

#ifdef	DEBUG
		printf ("Making entry:  %s\n", curmenuitem->label);
#endif
		label = XmStringCreate(	curmenuitem->label,
					XmSTRING_DEFAULT_CHARSET);
		if (curmenuitem->submenu) {
			label = XmStringConcat(label,
					XmStringCreate(	"...",
					XmSTRING_DEFAULT_CHARSET));
		}
		n = 0;
		XtSetArg(wargs[n], XmNlabelString, label);	n++;

		childbutton = XtCreateManagedWidget(	"child",
				xmPushButtonGadgetClass,
				menuparent, wargs, n);

		if (curmenuitem->submenu) {
#ifdef	DEBUG
			printf ("It has a submenu, which I'm recursing on...\n");
#endif
/* MOTIF menus don't work...
			childmenu = XmCreatePulldownMenu(menuparent,
					curmenuitem->label,
					NULL, 0);

			n = 0;
			XtSetArg(wargs[n], XmNsubMenuId, childmenu);	n++;

			childbutton = XtCreateManagedWidget(	"child",
					xmCascadeButtonWidgetClass,
					menuparent, wargs, n);
*/

			childmenu = CreateMenu(	childbutton, 
						curmenuitem->submenu,
						XmVERTICAL);
			XtAddCallback(	childbutton, XmNactivateCallback,
					map_menu_widget, childmenu);

		}

		else {
			XtAddCallback(	childbutton, 
					XmNactivateCallback, 
					menu_callback, curmenuitem);
		}
	}
	XtRealizeWidget(shellparent);

	return (shellparent);
}

/*	We don't need this, since I'm using callbacks rather than
**	event handlers.

void
post_menu_handler(w, menu, event)
Widget	w;
Widget	menu;
XEvent	*event;
{
	Arg	wargs[10];
	int	n;
	Widget	button;

	n = 0;
	XtSetArg(wargs[n], XmNwhichButton, &button);      n++;
	XtGetValues(menu, wargs, n);

	if ((Widget) (event->xbutton.button) == button) {
		XmMenuPosition (menu, event);
		XtManageChild(menu);
	}

	else {
		printf ("Ignoring hit from 'wrong' button\n");
	}
}
*/


DisplayForm(spec)
EntryForm	*spec;
{
	Widget	w;

	w = CreateForm(toplevel, spec);
	XtManageChild(w);
}


/*
** Read the specification and put up a form to match...
*/

Widget
CreateForm(parent, spec)
Widget		parent;
EntryForm	*spec;
{
	XmBulletinBoardWidget	bb;
	Arg		wargs[10];
	int		i, n;
	XmString	label;		/* compound string required */
	Dimension	height_so_far = 0, width_so_far = 0;
	Dimension	height, width;
	Widget		titleW, instructionW;
	Position	x, y;
	Widget		shellparent;

/*
	n = 0;
	XtSetArg(wargs[n], XtNmappedWhenManaged, False);	n++;
	shellparent = XtCreateApplicationShell(	"shellparent", 
						topLevelShellWidgetClass,
						wargs, n);
*/

#define GETSIZE(foo)	n = 0; \
			XtSetArg(wargs[n], XtNwidth, &width);	n++; \
			XtSetArg(wargs[n], XtNheight, &height);	n++; \
			XtGetValues (foo, wargs, n); \

#define STORESIZE	if (width > width_so_far) width_so_far = width;\
			height_so_far += height + HPADDING;


	n = 0;
	XtSetArg(wargs[n], XmNautoUnmanage, FALSE);		n++;
	bb = (XmBulletinBoardWidget)
		XmCreateBulletinBoardDialog(parent, "board", wargs, n);

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
	MakeInputLines((Widget) bb, &height, &width, spec->inputlines);
	STORESIZE;

	height = height_so_far;
	width = width_so_far;
	MakeButtons((Widget) bb, &height, &width, spec->buttons, (char *)spec);
	STORESIZE;

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

	spec->formpointer = (Widget) bb;

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

MakeInputLines(parent, pheight, pwidth, inputlines)
Widget		parent;
Dimension	*pheight;
Dimension	*pwidth;
UserPrompt      **inputlines;
{
	UserPrompt	*current;
	XmString	label;		/* compound string required */
	Arg		wargs[10];
	int		i, n;
	Widget		child;
	Dimension	width, height, maxleftwidth = 0, maxrightwidth = 0;
	Dimension	localy, leftheight = 0, rightheight = 0;
	UserPrompt      **myinputlines = inputlines;
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
		XtSetArg(wargs[n], XtNsensitive, 
			!(current->insensitive));		n++;
		switch (current->type) {
		case FT_STRING:
			children[i] = XtCreateManagedWidget(	"child",
						xmTextWidgetClass,
						parent, wargs, n);
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
/*
** EEEuch!  Not only to I have to use a blank (not NULL!) string to
** override the ToggleButton's insistance on labelling itself, it has
** to be a _compound_ blank string!
*/
			label = XmStringCreate(	" ", XmSTRING_DEFAULT_CHARSET);
			XtSetArg(wargs[n], XmNlabelString, label);	n++;

			children[i] = XtCreateManagedWidget(	"ignore this",
						xmToggleButtonWidgetClass,
						parent, wargs, n);

			XtAddCallback(	children[i], XmNvalueChangedCallback,
				radio_callback, NULL);

			GETSIZE (children[i]);
			rightheight = height;
			if (width > maxrightwidth)
				maxrightwidth = width;
			break;

		case FT_KEYWORD:
			children[i] = XmCreateRadioBox(parent, "radio", wargs, n);
			XtManageChild(children[i]);	
			AddRadioButtons(	children[i], 
						current,
						&rightheight);
			GETSIZE (children[i]);
			if (width > maxrightwidth)
				maxrightwidth = width;
			break;

		default:
			printf ("Sorry, don't recognize that type\n");
			break;
		}

		localy += MAX(rightheight, leftheight) + HPADDING;
	}

/*
** Now slide the input widgets right as far as the widest prompt.
*/
	n = 0;
	XtSetArg(wargs[n], XtNx, maxleftwidth + WPADDING);	n++;
	for (; i; i--)
		XtSetValues (children[i - 1], wargs, n);

	*pheight = localy - HPADDING;
	*pwidth = maxleftwidth + maxrightwidth + WPADDING;
}

/*
** All the junk about keeping track of the sum of the children's heights
** is because the !#$% RowColumn widget doesn't sum them for us, NOR
** does it accept SetValues on its XtNHeight!
*/

AddRadioButtons(parent, prompt, pheight)
Widget		parent;
UserPrompt	*prompt;
Dimension	*pheight;
{
	Widget	child;
	char	*current;
	Arg	wargs[10];
	int	i, n;
	XmString	label;		/* compound string required */
	Dimension	height, width;
	Dimension	height_so_far = 0;
	char	**keywords = prompt->keywords;
	char	*defvalue = prompt->returnvalue.stringvalue;

	if (!keywords) {
		fprintf (stderr, "Warning:  No list of keywords for widget\n");
		return;
	}
	for (current=(*keywords); current; keywords++, current=(*keywords)) {
		n = 0;
		label = XmStringCreate(current, XmSTRING_DEFAULT_CHARSET);
		XtSetArg(wargs[n], XmNlabelString, label);	n++;
		if (!strcmp (current, defvalue)) {
			XtSetArg(wargs[n], XmNset, True);	n++;
		}
		else {
			XtSetArg(wargs[n], XmNset, False);	n++;
		}
		child = XtCreateManagedWidget(	current,
						xmToggleButtonGadgetClass,
						parent, wargs, n);

		XtAddCallback(	child, XmNvalueChangedCallback,
				radio_callback, prompt);

		GETSIZE (child);
		height_so_far += height;
	}
/*
	GETSIZE (parent);
	printf ("height of radio parent was %d\n", height);
	n = 0;
	XtSetArg(wargs[n], XtNheight, height_so_far);	n++;
	XtSetValues (parent, wargs, n);
	GETSIZE (parent);
	printf ("height of radio parent is now %d\n", height);
*/
	*pheight = height_so_far;
}

MakeButtons(parent, pheight, pwidth, buttons, data)
Widget		parent;
Dimension	*pheight;
Dimension	*pwidth;
BottomButton	**buttons;
caddr_t		data;
{
	BottomButton	*current;
	XmString	label;		/* compound string required */
	Arg		wargs[10];
	int		i, n;
	Dimension	newwidth, width = 25;
	Widget		newbutton;

	n = 0;
	XtSetArg(wargs[n], XtNy, *pheight);			n++;
	XtSetArg(wargs[n], XtNx, 0);				n++;
	XtSetArg(wargs[n], XtNwidth, *pwidth);			n++;
	XtCreateManagedWidget(	"separator",
				xmSeparatorWidgetClass,
				parent, wargs, n);
	*pheight += HPADDING;

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
				current->returnfunction, data);
		n = 0;
		XtSetArg(wargs[n], XtNwidth, &newwidth);		n++;
		XtGetValues (newbutton, wargs, n);

		width += (newwidth + WPADDING);
	}

	(*pheight) += 100;
}

void
map_menu_widget(w, widget, call_data)
Widget	w, widget;
XmAnyCallbackStruct	*call_data;
{
	Arg		wargs[10];
	int		n;
	Position	x, y;
	Widget		foo;

	
	for (	x = 0, y = 0, foo = w->core.parent;
		foo;
		foo = foo->core.parent) {

		x += foo->core.x;
		y += foo->core.y;
	}

/*
	if (w->core.parent)
		x += w->core.parent->core.width;
*/
	x += w->core.width;
	y += w->core.y;

	n = 0;
	XtSetArg(wargs[n], XtNx, x);		n++;
	XtSetArg(wargs[n], XtNy, y);		n++;
	XtSetValues(widget, wargs, n);

	XtMapWidget(widget);	
}

void
manage_widget(w, widget, call_data)
Widget	w, widget;
XmAnyCallbackStruct	*call_data;
{
	XtManageChild(widget);	
}

int
button_callback(w, client_data, call_data)
Widget	w;
EntryForm	*client_data;
XmAnyCallbackStruct	*call_data;
{
	XtUnmanageChild(client_data->formpointer);
}

int
radio_callback(w, client_data, call_data)
Widget	w;
XmAnyCallbackStruct	*client_data;
XmAnyCallbackStruct	*call_data;
{
	Arg		wargs[10];
	int		i, n;
	XmString	label;		/* !@#$ compound string required! */
	char            *text_value;

	UserPrompt	*prompt = (UserPrompt *) client_data;

	printf ("radio_callback: button %x, data %x\n", w, client_data);

/*
** It should be _easy_ to find the value of a label, right?  _WRONG!_
** Have to disassemble the !@#$ compound text to get a char*, and
** Motif doesn't provide any functions to do so.  So I stash the label
** value as the name of the widget.

	n = 0;
	XtSetArg(wargs[n], XmNlabelString, label);	n++;
	XtGetValues (w, wargs, n);

       XmStringGetLtoR (label,
                        XmSTRING_DEFAULT_CHARSET, &text_value);
*/


/*
** Unfortunately, Xt caches w->core.name into something besides a char*,
** so I can't use it.  And XtName is, somehow, unavailable.

	if (!prompt) {
		printf ("Oops!  radio_callback called from '%s' with no data\n",
				w->core.name);
		return(0);
	}

	printf ("Replacing old value of selection, '%s', with '%s'\n",
			prompt->returnvalue.stringvalue,
			w->core.name);

	strcpy(prompt->returnvalue.stringvalue, w->core.name);
*/

}

void
menu_callback(w, client_data, call_data)
Widget	w;
XmAnyCallbackStruct	*client_data;
XmAnyCallbackStruct	*call_data;
{
	MenuItem	*itemhit = (MenuItem *) client_data;

#ifdef DEBUG
	printf 	("menu_callback: item '%s', op %d and string '%s'\n", 
			itemhit->label, 
			itemhit->operation, 
			itemhit->form);
#endif
	MoiraMenuRequest(itemhit);
/*
** Unmap the shell of this button.  (UGLY HACK until Motif menus work)
*/

	XtUnmapWidget(XtParent(XtParent(w)));
}
