/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/formup.c,v 1.9 1992-10-28 16:07:08 mar Exp $ */

#include	<stdio.h>
#include	<strings.h>
#include	<X11/StringDefs.h>
#include	<X11/IntrinsicP.h>
#include	<X11/Shell.h>
#include	<X11/Core.h>
#include	<X11/CoreP.h>
#include	<Xm/Xm.h>
#include	<Xm/BulletinB.h>
#include	<Xm/BulletinBP.h>
#include	<Xm/Label.h>
#include	<Xm/Text.h>
#include	<Xm/TextP.h>
#include	<Xm/PushB.h>
#include	<Xm/PushBG.h>
#include	<Xm/CascadeB.h>
#include	<Xm/ToggleB.h>
#include	<Xm/ToggleBG.h>
#include	<Xm/RowColumn.h>
#include	<Xm/RowColumnP.h>
#include	<Xm/Separator.h>
#include	<Xm/Traversal.h>
#include	"mmoira.h"

static char rcsid[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/formup.c,v 1.9 1992-10-28 16:07:08 mar Exp $";

#ifndef MAX
#define	MAX(a,b)	((a > b) ? a : b)
#endif
#ifndef MIN
#define	MIN(a,b)	((a < b) ? a : b)
#endif

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
void	newvalue();
void	MoiraFocusOut();
EntryForm *WidgetToForm();

extern void	UpdateForm();
extern int	PopupErrorMessage();
extern void	PopupHelpWindow();
extern int	AppendToLog();
extern void	MakeWatchCursor();
extern void	MakeNormalCursor();
extern Widget	SetupLogWidget();

static XtActionsRec myactions[] = {
    { "MoiraFocusOut", MoiraFocusOut },
};


void
manage_widget(w, widget, call_data)
Widget	w, widget;
XmAnyCallbackStruct	*call_data;
{
	XtManageChild(widget);	
}

int
button_callback(w, client_data, call_data)
Widget		w;
EntryForm	*client_data;
XmAnyCallbackStruct	*call_data;
{
	XtUnmanageChild(client_data->formpointer);
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
		if (curmenuitem->accel) {
		    XtSetArg(wargs[n], XmNmnemonic, *(curmenuitem->accel)); n++;
		}

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
		UpdateForm(spec);
		return(spec->formpointer);
	}

	height_so_far = vpad;

#define GETSIZE(foo)	n = 0; \
			XtSetArg(wargs[n], XtNwidth, &width);	n++; \
			XtSetArg(wargs[n], XtNheight, &height);	n++; \
			XtGetValues (foo, wargs, n); \

#define STORESIZE	if (width > width_so_far) width_so_far = width;\
			height_so_far += height + vpad;


	n = 0;
	XtSetArg(wargs[n], XmNautoUnmanage, False);		n++;
	bb = XmCreateBulletinBoardDialog(parent, spec->formname, wargs, n);
	MapWidgetToForm(bb, spec);

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
	instructionW = XtCreateManagedWidget(	"instructions",
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

/*
** Center the title of the form
*/
#ifdef FORMTITLES
	n = 0;
	XtSetArg(wargs[n], XtNwidth, &width);			n++;
	XtGetValues (titleW, wargs, n);

	x = (width_so_far - width) / 2;

	n = 0;
	XtSetArg(wargs[n], XtNx, x);				n++;
	XtSetValues (titleW, wargs, n);
#endif
	n = 0;
	XtSetArg(wargs[n], XtNwidth, &width);			n++;
	XtGetValues (instructionW, wargs, n);

	x = (width_so_far - width) / 2;

	n = 0;
	XtSetArg(wargs[n], XtNx, x);				n++;
	XtSetValues (instructionW, wargs, n);

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
	static XtTranslations trans = NULL;
#define newtrans "<FocusOut>: focusOut() MoiraFocusOut()"


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
		if (current->type == FT_KEYWORD) {
		    char *p;

		    p = index(current->prompt, '|');
		    if (p) {
			*p++ = 0;
			current->keyword_name = p;
		    }
		}

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

		if (current->type == FT_KEYWORD && current->keyword_name) {
		    label = XmStringCreate("add new value",
					   XmSTRING_DEFAULT_CHARSET);
		    n = 0;
		    XtSetArg(wargs[n], XmNlabelString, label);	n++;
		    XtSetArg(wargs[n], XtNy, localy + *pheight + height); n++;
		    XtSetArg(wargs[n], XtNx, height); n++;
		    child = XtCreateManagedWidget("newvalue",
						  xmPushButtonWidgetClass,
						  parent, wargs, n);
		    XtAddCallback(child, XmNactivateCallback,
				  newvalue, current);

		    GETSIZE(child);
		    leftheight += height;
		    if (width + height > maxleftwidth)
		      maxleftwidth = width + height;
		}
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
			children[i] = XtCreateManagedWidget(	"textwidget",
						xmTextWidgetClass,
						parent, wargs, n);
			XtAddCallback(	children[i], XmNvalueChangedCallback,
				string_callback, current);
			if (trans == NULL) {
			    XtAppAddActions(XtWidgetToApplicationContext(children[i]),
					    myactions, XtNumber(myactions));

			    trans = XtParseTranslationTable(newtrans);
			}
			XtOverrideTranslations(children[i], trans);
			if (current->returnvalue.stringvalue) {
				XmTextSetString (children[i], current->returnvalue.stringvalue);
			}
			GETSIZE (children[i]);
			rightheight = height;
			if (width > maxrightwidth)
				maxrightwidth = width;
			break;

		case FT_BOOLEAN:
			XtSetArg(wargs[n], XmNset,
				 current->returnvalue.booleanvalue ? True : False);	n++;

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
				MakeRadioField(parent, current,
					       &rightheight, spec);
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
		MapWidgetToForm(children[i], spec);
		current->parent = (caddr_t) spec;

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
MakeRadioField(parent, prompt, pheight, spec)
Widget		parent;
UserPrompt	*prompt;
Dimension	*pheight;
EntryForm	*spec;
{
	Widget	radioparent, child = NULL;
	char	*current;
	Arg	wargs[10];
	int	count, n;
	XmString	label;	/* accursed compound string required */
	Dimension	height, width;
	char	**keywords, *null[2];

	if (!prompt->keywords) {
		fprintf (stderr, "Warning:  No list of keywords for widget\n");
		prompt->keywords = null;
		null[0] = NULL;
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
		XtSetArg(wargs[n], XmNnumColumns, 1 + (count-1) / 5);		n++;
		XtSetArg(wargs[n], XmNorientation, XmVERTICAL);	n++;
		XtSetArg(wargs[n], XmNpacking, XmPACK_COLUMN);	n++;
	}
	radioparent = XmCreateRadioBox(parent, "radio", wargs, n);

	keywords = prompt->keywords;
	for (current=(*keywords); current; keywords++, current=(*keywords)) {
		n = 0;
		label = XmStringCreate(current, XmSTRING_DEFAULT_CHARSET);
		XtSetArg(wargs[n], XmNlabelString, label);	n++;
		if ((prompt->returnvalue.stringvalue) &&
			(!strcmp (current, prompt->returnvalue.stringvalue))) {
			XtSetArg(wargs[n], XmNset, True);	n++;
		}
		else {
			XtSetArg(wargs[n], XmNset, False);	n++;
		}
		child = XtCreateManagedWidget(	current,
						xmToggleButtonWidgetClass,
						radioparent, wargs, n);
		MapWidgetToForm(child, spec);

		XtAddCallback(	child, XmNvalueChangedCallback,
				radio_callback, prompt);

	}
/*
** Assume all child widgets are the same height.  Increase height by
** five times this, or the actual number of children, whichever is lesser.
*/

	if (child) {
	    GETSIZE (child);
	} else
	  height = 10;
	*pheight = (height * MIN(5, count)) + vpad; 

	return(radioparent);
}


/* This is called when the list of keywords changes.  The old radio box
 * will be destroyed and a new one created.
 */

RemakeRadioField(form, field)
EntryForm *form;
int field;
{
    Dimension x, y, parent_y, oldheight, newheight;
    Arg wargs[4];
    Widget w;
    XmBulletinBoardWidget bb;
    XmRowColumnWidget rc;
    static XtTranslations trans = NULL;
    extern char form_override_table[];
    int i;

    XtSetArg(wargs[0], XtNx, &x);
    XtSetArg(wargs[1], XtNy, &y);
    XtSetArg(wargs[2], XtNheight, &oldheight);
    XtGetValues(form->inputlines[field]->mywidget, wargs, 3);
    XtUnmanageChild(form->inputlines[field]->mywidget);
    form->inputlines[field]->mywidget = w =
      MakeRadioField(form->formpointer, form->inputlines[field],
		     &newheight, form);
    XtSetArg(wargs[0], XtNx, x);
    XtSetArg(wargs[1], XtNy, y);
    XtSetValues(w, wargs, 2);
    MapWidgetToForm(w, form);
    XmAddTabGroup(w);
    if (newheight > oldheight) {
	bb = (XmBulletinBoardWidget) form->formpointer;
	parent_y = y;
	for (i = 0; i < bb->composite.num_children; i++) {
	    XtSetArg(wargs[0], XtNy, &y);
	    XtGetValues(bb->composite.children[i], wargs, 1);
	    if (y > parent_y) {
		y = (y + newheight) - oldheight;
		XtSetArg(wargs[0], XtNy, y);
		XtSetValues(bb->composite.children[i], wargs, 1);
	    }
	}
    }

    if (trans == NULL)
      trans = XtParseTranslationTable(form_override_table);
    XtOverrideTranslations(w, trans);
    rc = (XmRowColumnWidget) w;
    for (i = 0; i < rc->composite.num_children; i++)
      XtOverrideTranslations(rc->composite.children[i], trans);

    XtManageChild(w);
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

		XtAddCallback(newbutton, XmNactivateCallback,
			      (XtCallbackProc) current->returnfunction,
			      spec);
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
	if (prompt->returnvalue.stringvalue &&
			(strcmp(prompt->returnvalue.stringvalue, XtName(w)))) {
		strcpy(prompt->returnvalue.stringvalue, XtName(w));
		if (prompt->valuechanged)
		  (*prompt->valuechanged)(WidgetToForm(w), prompt);
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

	if (current->valuechanged)
	  (*current->valuechanged)(WidgetToForm(w), current);

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

/*	printf 	("menu_callback: item '%s', op %d and string '%s'\n", 
			itemhit->label, 
			itemhit->operation, 
			itemhit->form);
	XtManageChild(entryformwidget);	
*/
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
/*		printf ("Replacing old value of selection, '%s', with '%s'\n",
				current->returnvalue.stringvalue,
				newvalue);
		strcpy(current->returnvalue.stringvalue, newvalue);
		if (current->valuechanged)
		  (*current->valuechanged)(WidgetToForm(w), current);
*/	}
	XtFree(newvalue);
}


void MoiraFocusOut(w, event, p, n)
Widget w;
XEvent *event;
String *p;
Cardinal *n;
{
    char  *newvalue;
    UserPrompt *current = NULL;
    EntryForm *f;
    XmTextRec *tr = (XmTextRec *)w;
    int i;

    if (tr->core.self != w || tr->core.widget_class != xmTextWidgetClass)
      return;
    newvalue = XmTextGetString(w);
    f = WidgetToForm(w);
    for (i = 0; f->inputlines[i]; i++)
      if (f->inputlines[i]->mywidget == w) 
	current = f->inputlines[i];
    if (current == NULL) {
	fprintf(stderr, "Couldn't find prompt structure!\n");
	return;
    }

    if (strcmp(current->returnvalue.stringvalue, newvalue)) {
	strcpy(current->returnvalue.stringvalue, newvalue);
	if (current->valuechanged)
	  (*current->valuechanged)(f, current);
    }
    XtFree(newvalue);
}


void
newvalue(w, client_data, call_data)
Widget	w;
XmAnyCallbackStruct	*client_data;
XmAnyCallbackStruct	*call_data;
{
    UserPrompt	*current = (UserPrompt *)client_data;
    EntryForm	*form, *f;
    int i;
    static MenuItem mi;

    if (current->keyword_name == NULL) {
	PopupErrorMessage("Sorry, that keyword cannot be changed.", NULL);
	return;
    }
    form = (EntryForm *)current->parent;
    for (i = 0; form->inputlines[i]; i++)
      if (form->inputlines[i] == current)
	break;
    f = GetAndClearForm("add_new_value");
    mi.operation = MM_NEW_VALUE;
    mi.query = "add_alias";
    mi.argc = 3;
    mi.form = form->formname;
    mi.accel = (char *) i;
    f->menu = &mi;
    f->extrastuff = current->keyword_name;
    DisplayForm(f);
}


/* WARNING: This routine uses Motif internal undocumented routines.
 * It was the only way to get carriage return to Do The Right Thing.
 * If you are in a single-item tab group, this routine will call
 * MoiraFormComplete() (same as pressing OK on the bottom of the form).
 * otherwise, it advances the focus the same as pressing TAB.
 */

void EnterPressed(w, event, argv, count)
Widget w;
XEvent *event;
char **argv;
Cardinal *count;
{
    Widget next;
    EntryForm *form;

    next = _XmFindNextTabGroup(w);
    if (next == w) {
	MoiraFocusOut(w, event, argv, count);
	form = WidgetToForm(w);
	MoiraFormComplete(NULL, form);
    } else {
	_XmMgrTraversal(w, XmTRAVERSE_NEXT_TAB_GROUP);
    }
}


void CancelForm(w, event, argv, count)
Widget w;
XEvent *event;
char **argv;
Cardinal *count;
{
    EntryForm *form;

    form = WidgetToForm(w);
    if (form)
      XtUnmanageChild(form->formpointer);    
}


void ExecuteForm(w, event, argv, count)
Widget w;
XEvent *event;
char **argv;
Cardinal *count;
{
    EntryForm *form;

    form = WidgetToForm(w);
    if (form) {
	MoiraFocusOut(w, event, argv, count);
	MoiraFormComplete(NULL, form);
    }
}


void DoHelp(w, event, argv, count)
Widget w;
XEvent *event;
char **argv;
Cardinal *count;
{
    EntryForm *form;

    form = WidgetToForm(w);
    if (form)
      help(form->formname);
}


extern struct hash *create_hash();
static struct hash *WFmap = NULL;

MapWidgetToForm(w, f)
Widget *w;
EntryForm *f;
{
    if (WFmap == NULL) {
	WFmap = create_hash(101);
    }
    hash_store(WFmap, w, f);
}

EntryForm *WidgetToForm(w)
Widget *w;
{
    return((EntryForm *) hash_lookup(WFmap, w));
}
