/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/main.c,v 1.3 1991-06-05 12:26:30 mar Exp $
 *
 *  	Copyright 1991 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include	<mit-copyright.h>
#include	<stdio.h>
#include	<moira.h>
#include	<Xm/PushB.h>
#include	<Xm/BulletinB.h>
#include	"mmoira.h"

extern MenuItem MenuRoot;

Widget	CreateMenu(), CreateForm();
Widget	BuildMenuTree();
void popup_error_hook(), mr_x_input();

Widget  toplevel;
char *user, *program_name, *moira_server;

main(argc, argv)
int argc;
char *argv[];
{
	Widget	button, bboard, menuwidget;
	char *motd;
	int	n, status;

	if ((user = getlogin()) == NULL)
	  user = getpwuid((int) getuid())->pw_name;
	user = (user && strlen(user)) ? Strsave(user) : "";

	if ((program_name = rindex(argv[0], '/')) == NULL)
	  program_name = argv[0];
	else
	  program_name++;
	program_name = Strsave(program_name);

	/* Need to parse command line argument here */
	moira_server = "";

	status = mr_connect(moira_server);
	if (status) {
	    com_err(program_name, status, " connecting to server");
	    exit(1);
	}
	status = mr_motd(&motd);
	if (status) {
	    com_err(program_name, status, " connecting to server");
	    exit(1);
	}
	if (motd) {
	    fprintf(stderr, "The Moira server is currently unavailable:\n%s\n",
		    motd);
	    mr_disconnect();
	    exit(1);
	}

      	status = mr_auth("mmoira");
	if (status == MR_USER_AUTH) {
	    char buf[BUFSIZ];
	    com_err(program_name, status, "\nPress [RETURN] to continue");
	    gets(buf);
	} else if (status) {
	    if (status >= ERROR_TABLE_BASE_krb &&
		status <= ERROR_TABLE_BASE_krb + 256)
	      com_err(program_name, status, "; authorization failed - please run kinit");
	    else
	      com_err(program_name, status, "; authorization failed");
	    exit(1);
	}

	toplevel = XtInitialize("toplevel", "Moira", NULL, 0, 
				&argc, argv);

	bboard = XtCreateManagedWidget(	"bboard",
				       xmBulletinBoardWidgetClass,
				       toplevel, NULL, 0);
	menuwidget = BuildMenuTree(bboard, &MenuRoot);
	SetupLogWidget(bboard);

	XtRealizeWidget(toplevel);

	set_com_err_hook(popup_error_hook);
	mr_set_alternate_input(ConnectionNumber(XtDisplay(toplevel)),
			       mr_x_input);
	XtMainLoop();
}


int MoiraQuery(query, argc, argv, callback, data)
char *query;
int argc;
char **argv;
int (*callback)();
caddr_t data;
{
    int status;

    MakeWatchCursor(toplevel);
    status = mr_query(query, argc, argv, callback, data);
    if (status != MR_ABORTED && status != MR_NOT_CONNECTED) {
	MakeNormalCursor(toplevel);
	return(status);
    }
    status = mr_connect(moira_server);
    if (status) {
	com_err(program_name, status, " while re-connecting to server %s",
		moira_server);
	MakeNormalCursor(toplevel);
	return(MR_ABORTED);
    }
    status = mr_auth("mmoira");
    if (status) {
	com_err(program_name, status, " while re-authenticating to server %s",
		moira_server);
	mr_disconnect();
	MakeNormalCursor(toplevel);
	return(MR_ABORTED);
    }
    status = mr_query(query, argc, argv, callback, data);
    MakeNormalCursor(toplevel);
    return(status);

}


DisplayForm(spec)
EntryForm	*spec;
{
	Widget	w;

	w = CreateForm(toplevel, spec);
	XtManageChild(w);
}


void popup_error_hook(who, code, fmt, arg1, arg2, arg3, arg4, arg5)
char *who;
long code;
char *fmt;
caddr_t arg1, arg2, arg3, arg4, arg5;
{
    char buf[BUFSIZ], *cp;

    (void) strcpy(buf, who);
    for (cp = buf; *cp; cp++);
    *cp++ = ':';
    *cp++ = ' ';
    if (code) {
	(void) strcpy(cp, error_message(code));
	while (*cp)
	    cp++;
    }
    sprintf(cp, fmt, arg1, arg2, arg3, arg4, arg5);
    display_error(buf);
}


void mr_x_input()
{
    XEvent event;

    XtAppNextEvent(_XtDefaultAppContext(), &event);
    XtDispatchEvent(&event);
}
