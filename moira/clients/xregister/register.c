/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/xregister/register.c,v 1.1 1990-07-31 18:43:01 mar Exp $
 */

#include <stdio.h>
#include <errno.h>
#include <X11/Intrinsic.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <X11/cursorfont.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>


int quit(), help(), again(), go();
void advance_focus(), retreat_focus();
char fname[128], mname[128], lname[128], iname[128], uname[128], pname[128];
Widget TopWidget;

XtActionsRec field_actions[] = {
    {"AdvanceFocus", advance_focus},
    {"RetreatFocus", retreat_focus},
};

#define NFIELDS	5
struct field {
    char *s;
    Widget w;
} fields[NFIELDS] = {
    { fname }, { mname }, { lname }, { iname }, { uname }
};
int active_field = 0;


main(argc, argv)
int argc;
char **argv;
{
    Widget table, w, box;
    Arg args[10];
    Cardinal arg_cnt;
    Cursor cursor;

    setenv("XAPPLRESDIR", "/mit/moiradev/src/clients/xregister/");
    TopWidget = XtInitialize("Register", "Register", NULL, 0,
			     (unsigned int *)&argc, argv);

    XtGetApplicationResources(TopWidget, NULL, NULL, 0, NULL);
    box = XtCreateManagedWidget("box", formWidgetClass, TopWidget, args, 0);
    XtCreateManagedWidget("title", labelWidgetClass, box, args, 0);
    XtCreateManagedWidget("logo", labelWidgetClass, box, args, 0);
    XtCreateManagedWidget("info", asciiTextWidgetClass, box, args, 0);
    XtCreateManagedWidget("fnamel", labelWidgetClass, box, args, 0);
    XtCreateManagedWidget("mnamel", labelWidgetClass, box, args, 0);
    XtCreateManagedWidget("lnamel", labelWidgetClass, box, args, 0);
    XtCreateManagedWidget("idl", labelWidgetClass, box, args, 0);
    XtCreateManagedWidget("loginl", labelWidgetClass, box, args, 0);

    XtAddActions(field_actions, 2);
    fname[0] = 0;
    XtSetArg(args[0], XtNstring, fname);
    fields[0].w = XtCreateManagedWidget("fnameb", asciiTextWidgetClass,
					box, args, 1);
    mname[0] = 0;
    XtSetArg(args[0], XtNstring, mname);
    fields[1].w = XtCreateManagedWidget("mnameb", asciiTextWidgetClass,
					box, args, 1);
    lname[0] = 0;
    XtSetArg(args[0], XtNstring, lname);
    fields[2].w = XtCreateManagedWidget("lnameb", asciiTextWidgetClass,
					box, args, 1);
    iname[0] = 0;
    XtSetArg(args[0], XtNstring, iname);
    fields[3].w = XtCreateManagedWidget("idb", asciiTextWidgetClass,
					box, args, 1);
    uname[0] = 0;
    XtSetArg(args[0], XtNstring, uname);
    fields[4].w = XtCreateManagedWidget("loginb", asciiTextWidgetClass,
					box, args, 1);

    w = XtCreateManagedWidget("go", commandWidgetClass, box, args, 0);
    XtAddCallback(w, XtNcallback, go, NULL);
    w = XtCreateManagedWidget("again", commandWidgetClass, box, args, 0);
    XtAddCallback(w, XtNcallback, again, NULL);
    w = XtCreateManagedWidget("quit", commandWidgetClass, box, args, 0);
    XtAddCallback(w, XtNcallback, quit, NULL);

    XtRealizeWidget(TopWidget);
    XtMainLoop();
}


quit()
{
    exit(0);
}

remove_popup(ignored, w, ignored1)
Widget ignored;
caddr_t w;
caddr_t ignored1;
{
    XtPopdown((Widget) w);
}

help(button)
Widget button;
{
    static Widget top;
    Widget box, last;
    Cardinal num_args;
    Arg args[12];
    Position x, y;
    static int inited = 0;

    if (!inited++) {
	XtTranslateCoords(TopWidget, (Position) 0, (Position) 120, &x, &y);
	num_args = 0;
	XtSetArg(args[num_args], XtNx, x); num_args++;
	XtSetArg(args[num_args], XtNy, y); num_args++;
	top = XtCreatePopupShell("helpshell", transientShellWidgetClass, 
				 TopWidget, args, num_args);

	box = XtCreateManagedWidget("box", boxWidgetClass, top, NULL, 0);
	last = XtCreateManagedWidget("helptext", asciiTextWidgetClass,
				 box, args, 0);
	last = XtCreateManagedWidget("helpcmd", commandWidgetClass,
				     box, NULL, 0);
	XtAddCallback(last, XtNcallback, remove_popup, top);
    }
    XtPopup(top, XtGrabNone);
}

again()
{
    fname[0] = mname[0] = lname[0] = iname[0] = uname[0] = pname[0] = 0;
    activate_field(0);
}

activate_field(f)
int f;
{
    Arg args[10];
    int i;

    XtSetArg(args[0], XtNborderWidth, 0);
    XtSetArg(args[1], XtNdisplayCaret, False);
    for (i = 0; i < NFIELDS; i++) {
	XtSetValues(fields[i].w, args, 2);
    }
    XtSetArg(args[0], XtNborderWidth, 2);
    XtSetArg(args[1], XtNdisplayCaret, True);
    XtSetValues(fields[f].w, args, 2);
}

void advance_focus()
{
    if (active_field++ >= NFIELDS)
      active_field = 0;
    activate_field(active_field);

}

void retreat_focus()
{
    if (active_field-- < 0)
      active_field = NFIELDS;
    activate_field(active_field);

}

go()
{
}
