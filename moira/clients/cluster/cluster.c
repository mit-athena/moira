/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/cluster/cluster.c,v 1.3 1997-01-29 23:00:08 danw Exp $ */

#include <stdio.h>
#include <Wc/WcCreate.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Text.h>
#include <netdb.h>
#include "MList.h"
#include <moira.h>
#include <moira_site.h>
#include <string.h>

extern void AriRegisterAthena();
extern int errno;

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

char *substr();
void localErrorHandler(), mr_x_input(), inputDone(), Help(), popup_error_hook();
void getClusters(), getClusterMachine();
void selectClusters(), selectClusterMachine(), selectClusterData();
void deselectClusters(), clearClusters();
void getMachines(), getMachineCluster();
void selectMachines(), selectMachineCluster();
void deselectMachines(), clearMachines(), getMachineTypes();
void selectMachineType(), selectMachineSubnet(), getClusterServer();
void addMachineCluster(), removeMachineCluster(), removeMachineAllCluster();
void selectAllCluster(), selectAllMachine();
void saveCluster(), saveMachine();

XtActionsRec actions[] = {
    { "inputDone", inputDone },
};

XtActionsRec callbacks[] = {
    { "HelpCB", Help },
    { "getClusters", getClusters },
    { "getClusterMachine", getClusterMachine },
    { "selectClusters", selectClusters },
    { "selectClusterData", selectClusterData },
    { "selectAllCluster", selectAllCluster },
    { "selectClusterMachine", selectClusterMachine },
    { "deselectClusters", deselectClusters },
    { "clearClusters", clearClusters },
    { "getClusterServer", getClusterServer },
    { "getMachines", getMachines },
    { "getMachineCluster", getMachineCluster },
    { "selectMachines", selectMachines },
    { "selectAllMachine", selectAllMachine },
    { "selectMachineCluster", selectMachineCluster },
    { "deselectMachines", deselectMachines },
    { "clearMachines", clearMachines },
    { "getMachineTypes", getMachineTypes },
    { "selectMachineType", selectMachineType },
    { "selectMachineSubnet", selectMachineSubnet },
    { "addMachineCluster", addMachineCluster },
    { "removeMachineCluster", removeMachineCluster },
    { "removeMachineAllCluster", removeMachineAllCluster },
    { "saveCluster", saveCluster },
    { "saveMachine", saveMachine },
};

char *moira_help = "Moira is the Athena configuration management system.";

char *program_help = 
"This program allows you to manipulate machine/cluster mappings.  To\n\
create or delete machines or clusters, use one of the other moira\n\
clients.\n\
\n\
You can fetch from the database clusters and machines, which will be\n\
listed in the two scrolling windows.  They may be fetched by name\n\
(wildcards are allowed), machine<->cluster mappings, or fileservers\n\
used.\n\
\n\
The machines and clusters you are going to change must be selected.\n\
You can do this by mousing on them or using operations similar ton\n\
those that fetched them from the database.  Then you can use the\n\
commands in the Mappings menu.";
char *author_help = "Written by Mark Rosenstein <mar@MIT.EDU>, MIT Project Athena.";
char *unknown_help = "Sorry, help is not available on that topic.";

#define MACHLEN 32
#define CLULEN 16

typedef struct _mattr {
	char	type[9];
	long	addr;
} mattr;

Widget appShell, clist, mlist;
ClusterResources resources;
Display *dpy;
char *program_name;
char *clusters[256], *machines[1000];
mattr machattr[1000];
int nclusters = 0, nmachines = 0;
int suppress_updates = 0;


void
main(argc, argv)
int argc;
char* argv[];
{   
    int status, i;
    char *motd;
    XtAppContext app;

/*    setenv("XFILESEARCHPATH", "/afs/athena/system/moira/lib/%N", 1); */
    setenv("XFILESEARCHPATH", "/afs/athena/astaff/project/moiradev/src/clients/cluster/%N", 1);
    if ((program_name = strrchr(argv[0], '/')) == NULL)
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
    for (i = 0; i < sizeof(callbacks)/sizeof(XtActionsRec); i++)
      WcRegisterCallback(app, callbacks[i].string, callbacks[i].proc, NULL);
    /* Register all Motif widget classes */
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
    clist = WcFullNameToWidget(appShell, "*clusterlist");
    XawMListChange(clist, clusters, 0, 0, False);
    mlist = WcFullNameToWidget(appShell, "*machinelist");
    XawMListChange(mlist, machines, 0, 0, False);
    /*
     *  Realize the widget tree, finish up initializing,
     *  and enter the main application loop
     */
    XtRealizeWidget(appShell);
    mr_set_alternate_input(ConnectionNumber(dpy), mr_x_input);
    set_com_err_hook(popup_error_hook);
    XtMainLoop();
}


int collect(argc, argv, sq)
int argc;
char **argv;
struct save_queue *sq;
{
    char **argv_copy;
    int i, n;

    argv_copy = (char **)malloc((argc + 1) * sizeof (char *));
    for (i = 0; i < argc; i++)
      argv_copy[i] = strsave(argv[i]);
    argv_copy[i] = NULL;

    sq_save_data(sq, (char *)argv_copy);
    return(MR_CONT);
}


void deselectClusters()
{
    XawMListReturnStruct *ret;
    int count;

    ret = XawMListShowCurrent(clist, &count);
    for (count--; count >= 0; count--)
      XawMListUnhighlight(clist, ret[count].list_index);
    free(ret);
}


void clearClusters()
{
    nclusters = 0;
    deselectClusters();
    clusters[0] = NULL;
    XawMListChange(clist, clusters, 0, 0, False);
}


void addCluster(clu)
char *clu;
{
    XawMListReturnStruct *ret;
    int count, i, j, new, status;
    char *argv[3], **nargv, buf[BUFSIZ], buf1[BUFSIZ];
    struct save_queue *sq;

    /* Get cluster data and assemble line */
    sq = sq_create();
    argv[0] = clu;
    argv[1] = "*";
    status = MoiraQuery("get_cluster_data", 2, argv, collect, sq);
    if (status)
      com_err(program_name, status, " while fetching cluster data");
    if (status == MR_SUCCESS) {
	sq_get_data(sq, &nargv);
	sprintf(buf, "%-16s %s %s", clu, nargv[1], nargv[2]);
	free_argv(nargv);
	while (sq_get_data(sq, &nargv)) {
	    sprintf(buf1, "%s; %s %s", buf, nargv[1], nargv[2]);
	    strcpy(buf, buf1);
	    free_argv(nargv);
	}
    } else
      strcpy(buf, clu);
    sq_destroy(sq);

    /* find out where in sorted list new item will go */
    for (new = 0; new < nclusters; new++) {
	if ((i = strmcmp(clusters[new], clu)) > 0)
	  break;
	if (i == 0) {
	    free(clusters[new]);
	    clusters[new] = strsave(buf);
	    return;
	}
    }

    /* unselect everything in list */
    ret = XawMListShowCurrent(clist, &count);
    for (i = 0; i < count; i++)
      XawMListUnhighlight(clist, ret[i].list_index);

    /* insert new item */
    for (i = nclusters; i > new; i--)
      clusters[i + 1] = clusters[i];

    clusters[new] = strsave(buf);
    nclusters++;
    if (suppress_updates)
      suppress_updates++;
    else
      XawMListChange(clist, clusters, nclusters, 0, True);

    /* re-highlight (possibly moved) items */
    for (i = 0; i < count; i++)
      for (j = 0; j < nclusters; j++)
	if (!strmcmp(ret[i].string, clusters[j]))
	  XawMListHighlight(clist, j);
    free(ret);
}


void selectCluster(clu)
char *clu;
{
    int i;

    addCluster(clu);
    for (i = 0; i < nclusters; i++)
      if (!strmcmp(clusters[i], clu)) {
	  XawMListHighlight(clist, i);
	  return;
      }
}


void getClusters()
{
    char clu[64], **argv, *p;
    struct save_queue *sq;
    int status;

    sq = sq_create();
    PromptUser("Cluster name:", clu, sizeof(clu));
    p = clu;
    status = MoiraQuery("get_cluster", 1, &p, collect, sq);
    if (status)
      com_err(program_name, status, " while fetching clusters");

    suppress_updates = 1;
    while (sq_get_data(sq, &argv)) {
	addCluster(strsave(argv[0]));
	free_argv(argv);
    }
    sq_destroy(sq);
    if (suppress_updates > 1)
      XawMListChange(clist, clusters, nclusters, 0, True);
    suppress_updates = 0;
}

void selectClusters()
{
    char clu[64], **argv, *p;
    struct save_queue *sq;
    int status;

    sq = sq_create();
    PromptUser("Cluster name:", clu, sizeof(clu));
    p = clu;
    status = MoiraQuery("get_cluster", 1, &p, collect, sq);
    if (status)
      com_err(program_name, status, " while fetching clusters");

    suppress_updates = 1;
    while (sq_get_data(sq, &argv)) {
	selectCluster(strsave(argv[0]));
	free_argv(argv);
    }
    sq_destroy(sq);
    if (suppress_updates > 1)
      XawMListChange(clist, clusters, nclusters, 0, True);
    suppress_updates = 0;
}

void getClusterMachine()
{
    char mach[64], **argv, *qargv[2];
    struct save_queue *sq;
    int status;

    sq = sq_create();
    PromptUser("Machine name:", mach, sizeof(mach));
    qargv[0] = canonicalize_hostname(strsave(mach));
    qargv[1] = "*";
    status = MoiraQuery("get_machine_to_cluster_map", 2, qargv, collect, sq);
    if (status)
      com_err(program_name, status, " while fetching clusters");
    free(qargv[0]);

    suppress_updates = 1;
    while (sq_get_data(sq, &argv)) {
	addCluster(strsave(argv[1]));
	free_argv(argv);
    }
    sq_destroy(sq);
    if (suppress_updates > 1)
      XawMListChange(clist, clusters, nclusters, 0, True);
    suppress_updates = 0;
}


void selectClusterMachine()
{
    char mach[64], **argv, *qargv[2];
    struct save_queue *sq;
    int status;

    sq = sq_create();
    PromptUser("Machine name:", mach, sizeof(mach));
    qargv[0] = canonicalize_hostname(strsave(mach));
    qargv[1] = "*";
    status = MoiraQuery("get_machine_to_cluster_map", 2, qargv, collect, sq);
    if (status)
      com_err(program_name, status, " while fetching clusters");
    free(qargv[0]);

    suppress_updates = 1;
    while (sq_get_data(sq, &argv)) {
	selectCluster(strsave(argv[1]));
	free_argv(argv);
    }
    sq_destroy(sq);
    if (suppress_updates > 1)
      XawMListChange(clist, clusters, nclusters, 0, True);
    suppress_updates = 0;
}


void getClusterServer()
{
    char buf[64], *mach, **argv, *qargs[3];
    struct save_queue *fsq, *cluq, *sq;
    int status;

    fsq = sq_create();
    PromptUser("Server name:", buf, sizeof(buf));
    mach = canonicalize_hostname(strsave(buf));
    status = MoiraQuery("get_filesys_by_machine", 1, &mach, collect, fsq);
    if (status) {
	com_err(program_name, status, " while fetching filesystems");
	return;
    }
    free(mach);
    cluq = sq_create();
    qargs[0] = "*";
    qargs[1] = "syslib";
    status = MoiraQuery("get_cluster_data", 2, qargs, collect, cluq);
    if (status) {
	com_err(program_name, status, " while fetching cluster data");
	return;
    }
    suppress_updates = 1;
    while (sq_get_data(cluq, &argv)) {
	mach = strchr(argv[2], ' ');
	if (mach) *mach = 0;
	printf("Searching for filsys %s\n", argv[2]);
	for (sq = fsq->q_next; sq != fsq; sq = sq->q_next)
	  if (!strcmp(argv[2], ((char **)sq->q_data)[0]))
	    addCluster(strsave(argv[0]));
	free_argv(argv);
    }
    if (suppress_updates > 1)
      XawMListChange(clist, clusters, nclusters, 0, True);
    suppress_updates = 0;
    while (sq_get_data(fsq, &argv)) {
	printf("filsys %s\n", argv[0]);
	free_argv(argv);
    }
    sq_destroy(fsq);
    sq_destroy(cluq);
}


void selectClusterData()
{
    char buf[64];
    int i;

    PromptUser("Data:", buf, sizeof(buf));

    for (i = 0; i < nclusters; i++)
      if (substr(buf, &clusters[i][CLULEN]))
	XawMListHighlight(clist, i);
}

void selectAllCluster()
{
    int i;

    for (i = 0; i < nclusters; i++)
      XawMListHighlight(clist, i);
}


void deselectMachines()
{
    XawMListReturnStruct *ret;
    int count;

    ret = XawMListShowCurrent(mlist, &count);
    for (count--; count >= 0; count--)
      XawMListUnhighlight(mlist, ret[count].list_index);
    free(ret);
}


void clearMachines()
{
    nmachines = 0;
    deselectMachines();
    machines[0] = NULL;
    XawMListChange(mlist, machines, 0, 0, False);
}


void addMachine(mach, type)
char *mach, *type;
{
    XawMListReturnStruct *ret;
    int count, i, j, new, addr;
    char buf[BUFSIZ];
    struct hostent *he;

    he = gethostbyname(mach);
    if (he)
      addr = *(int *)he->h_addr_list[0];
    else
      addr = 0L;
    sprintf(buf, "%-32s %-8s %s", mach, type, inet_ntoa(addr));

    /* find out where in sorted list new item will go */
    for (new = 0; new < nmachines; new++) {
	if ((i = strmcmp(machines[new], mach)) > 0)
	  break;
	if (i == 0) {
	    if (strlen(mach) > strlen(machines[new])) {
		free(machines[new]);
		machines[new] = strsave(buf);
		strcpy(machattr[new].type, type);
		machattr[new].addr = addr;
	    }
	    if (suppress_updates)
	      suppress_updates++;
	    else
	      XawMListChange(mlist, machines, nmachines, 0, True);
	    return;
	}
    }

    /* unselect everything in list */
    ret = XawMListShowCurrent(mlist, &count);
    for (i = 0; i < count; i++)
      XawMListUnhighlight(mlist, ret[i].list_index);

    /* insert new item */
    for (i = nmachines; i > new; i--) {
	machines[i] = machines[i - 1];
	machattr[i] = machattr[i - 1];
    }

    machines[new] = strsave(buf);
    strcpy(machattr[new].type, type);
    machattr[new].addr = addr;
    nmachines++;
    if (suppress_updates)
      suppress_updates++;
    else
      XawMListChange(mlist, machines, nmachines, 0, True);

    /* re-highlight (possibly moved) items */
    for (i = 0; i < count; i++)
      for (j = 0; j < nmachines; j++)
	if (!strmcmp(ret[i].string, machines[j]))
	  XawMListHighlight(mlist, j);
    free(ret);
}


void selectMachine(mach, type)
char *mach, *type;
{
    int i;

    addMachine(mach, type);
    for (i = 0; i < nmachines; i++)
      if (!strmcmp(machines[i], mach)) {
	  XawMListHighlight(mlist, i);
	  return;
      }
}

void getMachines()
{
    char *mach, **argv, buf[BUFSIZ];
    struct save_queue *sq;
    int status;

    sq = sq_create();
    PromptUser("Machine name:", buf, sizeof(buf));
    mach = canonicalize_hostname(strsave(buf));
    status = MoiraQuery("get_machine", 1, &mach, collect, sq);
    if (status)
      com_err(program_name, status, " while fetching machines");
    free(mach);

    suppress_updates = 1;
    while (sq_get_data(sq, &argv)) {
	addMachine(argv[0], argv[1]);
	free_argv(argv);
    }
    sq_destroy(sq);
    if (suppress_updates > 1)
      XawMListChange(mlist, machines, nmachines, 0, True);
    suppress_updates = 0;
}


void selectMachines()
{
    char *mach, **argv, buf[BUFSIZ];
    struct save_queue *sq;
    int status;

    sq = sq_create();
    PromptUser("Machine name:", buf, sizeof(buf));
    mach = canonicalize_hostname(strsave(buf));
    status = MoiraQuery("get_machine", 1, &mach, collect, sq);
    if (status)
      com_err(program_name, status, " while fetching machines");
    free(mach);

    suppress_updates = 1;
    while (sq_get_data(sq, &argv)) {
	selectMachine(argv[0], argv[1]);
	free_argv(argv);
    }
    sq_destroy(sq);
    if (suppress_updates > 1)
      XawMListChange(mlist, machines, nmachines, 0, True);
    suppress_updates = 0;

}


void getMachineCluster()
{
    char **argv, **argv1, *qargv[2], buf[256];
    struct save_queue *sq, *sq1;
    int status;

    sq = sq_create();
    qargv[0] = "*";
    PromptUser("Cluster name:", buf, sizeof(buf));
    qargv[1] = buf;
    status = MoiraQuery("get_machine_to_cluster_map", 2, qargv, collect, sq);
    if (status)
      com_err(program_name, status, " while fetching clusters");

    suppress_updates = 1;
    while (sq_get_data(sq, &argv)) {
	addMachine(strsave(argv[0]), "");
#ifdef MACHTYPE
	sq1 = sq_create();
	status = MoiraQuery("get_machine", 1, argv, collect, sq1);
	if (status)
	  com_err(program_name, status, " while fetching machine type");
	sq_get_data(sq1, &argv1);
	addMachine(argv1[0], argv1[1]);
	sq_destroy(sq1);
	free_argv(argv1);
#endif
	free_argv(argv);
    }
    sq_destroy(sq);
    if (suppress_updates > 1)
      XawMListChange(mlist, machines, nmachines, 0, True);
    suppress_updates = 0;
}


void selectMachineCluster()
{
    char **argv, **argv1, *qargv[2], buf[256];
    struct save_queue *sq, *sq1;
    int status;

    sq = sq_create();
    qargv[0] = "*";
    PromptUser("Cluster name:", buf, sizeof(buf));
    qargv[1] = buf;
    status = MoiraQuery("get_machine_to_cluster_map", 2, qargv, collect, sq);
    if (status)
      com_err(program_name, status, " while fetching clusters");

    suppress_updates = 1;
    while (sq_get_data(sq, &argv)) {
	selectMachine(strsave(argv[0]), "");
#ifdef MACHTYPE
	sq1 = sq_create();
	status = MoiraQuery("get_machine", 1, argv, collect, sq1);
	if (status)
	  com_err(program_name, status, " while fetching machine type");
	sq_get_data(sq1, &argv1);
	selectMachine(argv1[0], argv1[1]);
	free_argv(argv1);
	sq_destroy(sq1);
#endif
	free_argv(argv);
    }
    sq_destroy(sq);
    if (suppress_updates > 1)
      XawMListChange(mlist, machines, nmachines, 0, True);
    suppress_updates = 0;
}


void getMachineTypes()
{
    char *mach, **argv, buf[256], *p;
    int i, changed = 0, status, addr;
    struct save_queue *sq;
    struct hostent *he;
    sq = sq_create();

    for (i = 0; i < nmachines; i++) {
	if (strlen(machattr[i].type) == 0) {
	    mach = machines[i];
	    p = strchr(mach, ' ');
	    if (p) *p = 0;
	    status = MoiraQuery("get_machine", 1, &mach, collect, sq);
	    if (status)
	      com_err(program_name, status, " while fetching machine type");
	    sq_get_data(sq, &argv);
	    strcpy(machattr[i].type, argv[1]);
	    he = gethostbyname(mach);
	    if (he)
	      addr = *(int *)he->h_addr_list[0];
	    else
	      addr = 0L;
	    sprintf(buf, "%-32s %-8s %s", mach, argv[1], inet_ntoa(addr));
	    machines[i] = strsave(buf);
	    machattr[i].addr = addr;
	    free_argv(argv);
	    changed++;
	}
    }
    if (changed)
      XawMListChange(mlist, machines, nmachines, 0, True);
    sq_destroy(sq);
}


void selectMachineType()
{
    char type[16];
    int i;

    PromptUser("Machine type:", type, sizeof(type));
    getMachineTypes();

    for (i = 0; i < nmachines; i++)
      if (!strcasecmp(type, machattr[i].type))
	XawMListHighlight(mlist, i);
}


void selectMachineSubnet()
{
    char buf[BUFSIZ];
    int i, subnet;
    struct hostent *he;

    PromptUser("Machine's subnet:", buf, sizeof(buf));
    subnet = atoi(buf);

    for (i = 0; i < nmachines; i++) {
	/* Here we go hardcoding the subnet mask.... */
	if (subnet == ((ntohl(machattr[i].addr) >> 16) & 0xff))
	  XawMListHighlight(mlist, i);
    }
}


void selectAllMachine()
{
    int i;

    for (i = 0; i < nmachines; i++)
      XawMListHighlight(mlist, i);
}


void addMachineCluster()
{
    XawMListReturnStruct *machs, *clus;
    int mcount, ccount, status, c, m;
    char *argv[2], *p;

    machs = XawMListShowCurrent(mlist, &mcount);
    clus = XawMListShowCurrent(clist, &ccount);
    for (c = 0; c < ccount; c++) {
	for (m = 0; m < mcount; m++) {
	    argv[0] = strsave(machines[machs[m].list_index]);
	    p = strchr(argv[0], ' ');
	    if (p) *p = 0;
	    argv[1] = strsave(clusters[clus[c].list_index]);
	    p = strchr(argv[1], ' ');
	    if (p) *p = 0;
	    status = MoiraQuery("add_machine_to_cluster", 2, argv,
				collect, NULL);
	    if (status)
	      com_err(program_name, status, " while adding machines to clusters");
	    free(argv[0]);
	    free(argv[1]);
	}
    }
    free(machs);
    free(clus);
}


void removeMachineCluster()
{
    XawMListReturnStruct *machs, *clus;
    int mcount, ccount, status, c, m;
    char *argv[2], *p;

    machs = XawMListShowCurrent(mlist, &mcount);
    clus = XawMListShowCurrent(clist, &ccount);
    for (c = 0; c < ccount; c++) {
	for (m = 0; m < mcount; m++) {
	    argv[0] = strsave(machs[m].string);
	    p = strchr(argv[0], ' ');
	    if (p) *p = 0;
	    argv[1] = strsave(clus[c].string);
	    p = strchr(argv[1], ' ');
	    if (p) *p = 0;
	    status = MoiraQuery("delete_machine_from_cluster", 2, argv,
				collect, NULL);
	    if (status)
	      com_err(program_name, status, " while deleting machines from clusters");
	    free(argv[0]);
	    free(argv[1]);
	}
    }
    free(machs);
    free(clus);
}


void removeMachineAllCluster()
{
    XawMListReturnStruct *machs;
    int mcount, ccount, status, c, m;
    char *argv[2], *p, **args;
    struct save_queue *sq;

    machs = XawMListShowCurrent(mlist, &mcount);

    for (m = 0; m < mcount; m++) {
	sq = sq_create();
	argv[0] = strsave(machs[m].string);
	p = strchr(argv[0], ' ');
	if (p) *p = 0;
	argv[1] = "*";
	status = MoiraQuery("get_machine_to_cluster_map", 2, argv,
			    collect, sq);
	if (status)
	  com_err(program_name, status, " while getting cluster mapping");
	free(argv[0]);

	while (sq_get_data(sq, &args)) {
	    status = MoiraQuery("delete_machine_from_cluster", 2, args,
				collect, NULL);
	    if (status)
	      com_err(program_name, status, " while deleting machines from clusters");
	}
	sq_destroy(sq);
    }
}


void saveCluster()
{
    char buf[512];
    FILE *out;
    int i;

    PromptUser("Filename:", buf, sizeof(buf));
    out = fopen(buf, "w");
    if (out == NULL) {
	com_err(program_name, errno, " while opening output file \"%s\"", buf);
	return;
    }
    for (i = 0; i < nclusters; i++)
      fprintf(out, "%s\n", clusters[i]);
    if (fclose(out))
      com_err(program_name, errno, " while closing output file \"%s\"", buf);
}


void saveMachine()
{
    char buf[512];
    FILE *out;
    int i;

    PromptUser("Filename:", buf, sizeof(buf));
    out = fopen(buf, "w");
    if (out == NULL) {
	com_err(program_name, errno, " while opening output file \"%s\"", buf);
	return;
    }
    for (i = 0; i < nmachines; i++)
      fprintf(out, "%s\n", machines[i]);
    if (fclose(out))
      com_err(program_name, errno, " while closing output file \"%s\"", buf);
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

    MakeWatchCursor(appShell);
    status = mr_query(query, argc, argv, callback, data);
    if (status != MR_ABORTED && status != MR_NOT_CONNECTED) {
	MakeNormalCursor(appShell);
	return(status);
    }
    status = mr_connect(resources.server);
    if (status) {
	com_err(program_name, status, " while re-connecting to server %s",
		resources.server);
	MakeNormalCursor(appShell);
	return(MR_ABORTED);
    }
    status = mr_auth("mmoira");
    if (status) {
	com_err(program_name, status, " while re-authenticating to server %s",
		resources.server);
	mr_disconnect();
	MakeNormalCursor(appShell);
	return(MR_ABORTED);
    }
    status = mr_query(query, argc, argv, callback, data);
    MakeNormalCursor(appShell);
    return(status);
}


MakeWatchCursor(w) {}
MakeNormalCursor(w) {}

static input_done_flag = 0;

void inputDone()
{
    input_done_flag = 1;
}


PromptUser(msg, buf, bufsiz)
char *msg, *buf;
int bufsiz;
{
    XawTextBlock tb;
    Arg args[2];
    XEvent e;
    char *data;

    tb.firstPos = 0;
    tb.ptr = msg;
    tb.length = 0;
    tb.format = FMT8BIT;

    XtSetArg(args[0], XtNlabel, msg);
    XtSetValues(WcFullNameToWidget(appShell, "*query*prompt"), args, 1);

    XawTextReplace(WcFullNameToWidget(appShell, "*query*input"),
		   0, 65536, &tb);

    XtManageChild(WcFullNameToWidget(appShell, "*query"));
    XRaiseWindow(dpy, XtWindow(WcFullNameToWidget(appShell, "*query")));

    /* repeat main_loop here so we can check status & return */
    input_done_flag = 0;
    while (!input_done_flag) {
	XtAppNextEvent(_XtDefaultAppContext(), &e);
	XtDispatchEvent(&e);
    }

    XtSetArg(args[0], XtNstring, &data);
    XtGetValues(WcFullNameToWidget(appShell, "*query*input"), args, 1);
    strncpy(buf, data, bufsiz);

    XtUnmanageChild(WcFullNameToWidget(appShell, "*query"));
}


void Help(w, s, unused)
Widget w;
char *s;
{
    XawTextBlock tb;

    if (!strcmp(s, "moira"))
      tb.ptr = moira_help;
    else if (!strcmp(s, "program"))
      tb.ptr = program_help;
    else if (!strcmp(s, "author"))
      tb.ptr = author_help;
    else tb.ptr = unknown_help;

    tb.firstPos = 0;
    tb.length = strlen(tb.ptr);
    tb.format = FMT8BIT;

    XawTextReplace(WcFullNameToWidget(appShell, "*msg"),
		   0, 65536, &tb);

    XtPopup(WcFullNameToWidget(appShell, "*help"), XtGrabNone);
}


free_argv(argv)
char **argv;
{
    int i;

    for (i = 0; argv[i]; i++)
      free(argv[i]);
    free(argv);
}


/* compare machine strings.  This is a separate routine instead of strcmp()
 * so that we can deal with the optional machine type after the space
 * padding.  Usage is just like strcmp().
 */

int strmcmp(s1, s2)
char *s1, *s2;
{
    if ((*s1 == ' ' || *s1 == 0) &&
	(*s2 == ' ' || *s2 == 0))
      return(0);
    if (*s1 == *s2)
      return(strmcmp(s1+1, s2+1));
    if (*s1 < *s2)
      return(-1);
    return(1);
}


/* search for a substring.  Will scan string s looking for substring
 * f contained within it.  Will return a pointer to the start of the
 * contained substring, or NULL if not found.
 */

char *substr(f, s)
char *f, *s;
{
    char *p;

    while (p = strchr(s, *f)) {
	if (!strncmp(f, p, strlen(f)))
	  return(p);
	s++;
    }
    return(NULL);
}



void popup_error_hook(who, code, fmt, arg1, arg2, arg3, arg4, arg5)
char *who;
long code;
char *fmt;
caddr_t arg1, arg2, arg3, arg4, arg5;
{
    char buf[BUFSIZ], *cp;
    XawTextBlock tb;
    XEvent e;

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

    tb.firstPos = 0;
    tb.ptr = buf;
    tb.length = strlen(tb.ptr);
    tb.format = FMT8BIT;

    XawTextReplace(WcFullNameToWidget(appShell, "*errmsg"),
		   0, 65536, &tb);
    XtCallActionProc(WcFullNameToWidget(appShell, "*errmsg"),
		     "form-paragraph", &e, NULL, 0);

    XtPopup(WcFullNameToWidget(appShell, "*error"), XtGrabNone);
}
