/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/cluster/cluster.c,v 1.1 1991-09-08 18:29:34 mar Exp $ */

#include <stdio.h>
#include <WcCreate.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Text.h>
#include <moira.h>
#include <moira_site.h>

extern void AriRegisterAthena();

typedef struct _ClusterResources {
    char *server;
} ClusterResources;

static XrmOptionDescRec options[] = {
    {"-server",	"*moiraServer",		XrmoptionSepArg,	NULL},
};

#define Offset(field) (XtOffset(ClusterResources *, field))

static XtResource my_resources[] = {
    {"moiraServer", XtCString, XtRString, sizeof(String),
       Offset(server), XtRImmediate, "" },
};

#undef Offset

void getClusters(), getMachines(), localErrorHandler(), mr_x_input();

XtActionsRec actions[] = {
    { "getClusters", getClusters },
    { "getMachines", getMachines },
};

Widget appShell;
ClusterResources resources;
Display *dpy;
char *program_name;
char *clusters[256], *machines[100];
int nclusters = 0, nmachines = 0;

void
main(argc, argv)
int argc;
char* argv[];
{   
    int status;
    char *motd;
    XtAppContext app;

    if ((program_name = rindex(argv[0], '/')) == NULL)
      program_name = argv[0];
    else
      program_name++;
    program_name = strsave(program_name);

    /*
     *  Intialize Toolkit creating the application shell, and get
     *  application resources.
     */
    appShell = XtInitialize("cluster", "Cluster",
			    options, XtNumber(options),
			    &argc, argv);
    app = XtWidgetToApplicationContext(appShell);
    XtAppSetErrorHandler(app, localErrorHandler);
    dpy = XtDisplay(appShell);
    XtAppAddActions(app, actions, XtNumber(actions));
    XtGetApplicationResources(appShell, (caddr_t) &resources, 
			      my_resources, XtNumber(my_resources),
			      NULL, (Cardinal) 0);
    WcRegisterCallback(app, "getClusters", getClusters, NULL);
    /* Register all Athena widget classes */
    AriRegisterAthena(app);

    status = mr_connect(resources.server);
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

    status = mr_auth("cluster");
    if (status == MR_USER_AUTH) {
	char buf[BUFSIZ];
	com_err(program_name, status, "\nPress [RETURN] to continue");
	gets(buf);
    } else if (status) {
	com_err(program_name, status, "; authorization failed - may need to run kinit");
	exit(1);
    }

    /* Create widget tree below toplevel shell using Xrm database */
    WcWidgetCreation(appShell);
    /*
     *  Realize the widget tree, finish up initializing,
     *  and enter the main application loop
     */
    XtRealizeWidget(appShell);
    mr_set_alternate_input(ConnectionNumber(dpy), mr_x_input);
    XtMainLoop();
}


int collect(argc, argv, sq)
int argc;
char **argv;
struct save_queue *sq;
{
    sq_save_args(argc, argv, sq);
    return(MR_CONT);
}

void getClusters()
{
    Arg args[2];
    char *clu, **argv;
    struct save_queue *sq;
    int status;

    sq = sq_create();
    XtSetArg(args[0], XtNstring, &clu);
    XtGetValues(WcFullNameToWidget(appShell, "*clufilter"), args, 1);
    status = MoiraQuery("get_cluster", 1, &clu, collect, sq);
    if (status)
      com_err(program_name, status, " while fetching clusters");
    while (sq_get_data(sq, &argv)) {
/*	add_to_widget("*clusterlist", argv[0]); */
	clusters[nclusters++] = argv[0];
    }
    XawListChange(WcFullNameToWidget(appShell, "*clusterlist"),
		  clusters, nclusters, 0, False);
}

void getMachines()
{
}

/* Called from within the toolkit */
void localErrorHandler(s)
String s;
{
    fprintf(stderr, "Moira X error: %s\n", s);
    exit(1);
}


void mr_x_input()
{
    XEvent event;

    XtAppNextEvent(_XtDefaultAppContext(), &event);
    XtDispatchEvent(&event);
}


int MoiraQuery(query, argc, argv, callback, data)
char *query;
int argc;
char **argv;
int (*callback)();
caddr_t data;
{
    int status;

/*     MakeWatchCursor(toplevel); */
    status = mr_query(query, argc, argv, callback, data);
    if (status != MR_ABORTED && status != MR_NOT_CONNECTED) {
/*	MakeNormalCursor(toplevel); */
	return(status);
    }
    status = mr_connect(resources.server);
    if (status) {
	com_err(program_name, status, " while re-connecting to server %s",
		resources.server);
/*	MakeNormalCursor(toplevel); */
	return(MR_ABORTED);
    }
    status = mr_auth("mmoira");
    if (status) {
	com_err(program_name, status, " while re-authenticating to server %s",
		resources.server);
	mr_disconnect();
/*	MakeNormalCursor(toplevel); */
	return(MR_ABORTED);
    }
    status = mr_query(query, argc, argv, callback, data);
/*     MakeNormalCursor(toplevel); */
    return(status);
}


add_to_widget(wn, s)
char *wn;
char *s;
{
    XawTextBlock tb;
    int ret, i;
    XawTextPosition size;
    Widget w;
    Arg args[2];
    char buf[512];

    sprintf(buf, "%s\n", s);
    w = WcFullNameToWidget(appShell, wn);
    if (w == NULL)
      com_err(program_name, 0, " unable to find widget \"%s\"", wn);
    tb.firstPos = 0;
    tb.length = strlen(s);
    tb.ptr = buf;
    tb.format = FMT8BIT;
    XtSetArg(args[0], XtNlength, &size);
    XtGetValues(w, args, 1);
    ret = XawTextReplace(w, size, size, &tb);
    if (ret != XawEditDone)
      fprintf(stderr, "Text Edit failed: %d\n", ret);
}

