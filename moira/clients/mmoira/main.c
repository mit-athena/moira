/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/main.c,v 1.1 1991-05-30 21:59:27 mar Exp $
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

void	map_menu_widget();
Widget	CreateMenu();
Widget	BuildMenuTree();

Widget  toplevel;
char *user, *program_name, *moira_server;

main(argc, argv)
int argc;
char *argv[];
{
	Widget	button, bboard;
	Widget	menuwidget;
	int	n;

	user = "mar";
	program_name = argv[0];
	moira_server = "";

	mr_connect(moira_server);
	mr_auth("mmoira");

	toplevel = XtInitialize("toplevel", "Moira", NULL, 0, 
				&argc, argv);

	bboard = XtCreateManagedWidget(	"bboard",
				       xmBulletinBoardWidgetClass,
				       toplevel, NULL, 0);
	button = XtCreateManagedWidget("button",
				       xmPushButtonWidgetClass,
				       bboard, NULL, 0);

	menuwidget = BuildMenuTree(bboard, &MenuRoot);

	XtRealizeWidget(toplevel);

	map_menu_widget(bboard, menuwidget, NULL);
	XtMainLoop();
}

Widget
BuildMenuTree(topW, spec)
Widget		topW;
MenuItem	*spec;
{
	return (CreateMenu(topW, spec->submenu, XmHORIZONTAL));
}


int MoiraQuery(query, argc, argv, callback, data)
char *query;
int argc;
char **argv;
int (*callback)();
caddr_t data;
{
    int status;

    status = mr_query(query, argc, argv, callback, data);
    if (status != MR_ABORTED && status != MR_NOT_CONNECTED)
      return(status);
    status = mr_connect(moira_server);
    if (status) {
	com_err(program_name, status, " while re-connecting to server %s",
		moira_server);
	return(MR_ABORTED);
    }
    status = mr_auth("mmoira");
    if (status) {
	com_err(program_name, status, " while re-authenticating to server %s",
		moira_server);
	mr_disconnect();
	return(MR_ABORTED);
    }
    status = mr_query(query, argc, argv, callback, data);
    return(status);

}
