/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/blanche/blanche.c,v 1.4 1988-12-07 18:45:55 mar Exp $
 *
 * Command line oriented SMS List tool.
 *
 * by Mark Rosenstein, September 1988.
 *
 * Copyright 1989 by the Massachusetts Institute of Technology.
 *
 * (c) Copyright 1988 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

/* ### Aren't there a lot of sq abstraction barrier violations here?
   Do we need to improve the support for queue operations? */

#include <mit-copyright.h>
#include <stdio.h>
#include <ctype.h>
#include <sms.h>
#include <sms_app.h>

#ifndef LINT
static char smslist_rcsid[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/blanche/blanche.c,v 1.4 1988-12-07 18:45:55 mar Exp $";
#endif


struct member {
    int type;
    char *name;
};

/* It is important to membercmp that M_USER < M_LIST < M_STRING */
#define M_ANY		0
#define M_USER		1
#define M_LIST		2
#define M_STRING	3

/* argument parsing macro */
#define argis(a,b) ((strcmp(*arg+1, a) == 0) || (strcmp(*arg+1, b) == 0))

/* flags from command line */
int infoflg, verbose, syncflg, memberflg, recursflg, debugflg;

/* various member lists */
struct save_queue *addlist, *dellist, *memberlist, *synclist;

char *listname, *whoami;

extern char *index();
extern int errno;

int show_list_info(), show_list_count(), get_list_members(), scream();
int show_list_members();
struct member *parse_member();



main(argc, argv)
int argc;
char **argv;
{
    int status;
    char **arg = argv;
    char *membervec[3];
    struct member *memberstruct;

    /* clear all flags & lists */
    infoflg = verbose = syncflg = memberflg = debugflg = recursflg = 0;
    listname = NULL;
    addlist = sq_create();
    dellist = sq_create();
    memberlist = sq_create();
    synclist = sq_create();
    whoami = argv[0];

    /* parse args, building addlist, dellist, & synclist */
    while (++arg - argv < argc) {
	if  (**arg == '-')
	{
	    if (argis("m", "members"))
		memberflg++;
	    else if (argis("D", "debug"))
		debugflg++;
	    else if (argis("i","information"))
	      infoflg++;
	    else if (argis("v","verbose"))
	      verbose++;
	    else if (argis("r","recursive"))
	      recursflg++;
	    else if (argis("a","add"))
		if (arg - argv < argc - 1) {
		    ++arg;
		    if (memberstruct = parse_member(*arg))
			sq_save_data(addlist, memberstruct);
		} else
		    usage(argv);
	    else if (argis("d","delete"))
		if (arg - argv < argc - 1) {
		    ++arg;
		    if (memberstruct = parse_member(*arg))
			sq_save_data(dellist, memberstruct);
		} else
		    usage(argv);
	    else if (argis("f","file"))
		if (arg - argv < argc - 1) {
		    FILE *in;
		    char buf[BUFSIZ];

		    syncflg++;
		    ++arg;
		    if (!strcmp(*arg, "-"))
		      in = stdin;
		    else {
			in = fopen(*arg, "r");
			if (!in) {
			    com_err(whoami, errno, 
				    " while opening %s for input", *arg);
			    exit(2);
			}
		    }
		    while (fgets(buf, BUFSIZ, in))
		      if (memberstruct = parse_member(buf))
			sq_save_data(synclist, memberstruct);
		    if (!feof(in))
		      com_err(whoami, errno, " while reading from %s", *arg);
		} else
		  usage(argv);
	    else
		usage(argv);
	}
	else if (listname == NULL)
	  listname = *arg;
	else
	  usage(argv);
    }
    if (listname == NULL)
      usage(argv);

    /* if no other options specified, turn on list members flag */
    if (!(infoflg || syncflg ||
	  addlist->q_next != addlist || dellist->q_next != dellist))
      memberflg++;

    /* fire up SMS */
    if (status = sms_connect(SMS_SERVER)) {
	com_err(whoami, status, " unable to connect to SMS");
	exit(2);
    }
    if (status = sms_auth("blanche")) {
	com_err(whoami, status, " unable to authenticate to SMS");
	exit(2);
    }

    /* display list info if requested to */
    if (infoflg) {
	status = sms_query("get_list_info", 1, &listname, show_list_info,NULL);
	if (status)
	  com_err(whoami, status, " while getting list information");
	if (verbose && !memberflg) {
	    status = sms_query("count_members_of_list", 1, &listname,
			       show_list_count, NULL);
	    if (status)
	      com_err(whoami, status, " while getting list count");
	}
    }

    /* if we're synchronizing to a file, we need to:
     *  get the current members of the list
     *	for each member of the sync file
     *	   if they are on the list, remove them from the in-memory copy
     *	   if they're not on the list, add them to add-list
     *	if anyone is left on the in-memory copy, put them on the delete-list
     * lastly, reset memberlist so we can use it again later
     */
    if (syncflg) {
	status = sms_query("get_members_of_list", 1, &listname,
			   get_list_members, (char *)memberlist);
	if (status)
	  com_err(whoami, status, " getting members of list %s", listname);
	while (sq_get_data(synclist, &memberstruct)) {
	    struct save_queue *q;
	    int removed = 0;

	    for (q = memberlist->q_next; q != memberlist; q = q->q_next) {
		if (membercmp(q->q_data, memberstruct) == 0) {
		    q->q_prev->q_next = q->q_next;
		    q->q_next->q_prev = q->q_prev;
		    removed++;
		    break;
		}
	    }
	    if (!removed)
	      sq_save_data(addlist, memberstruct);
	}
	while (sq_get_data(memberlist, &memberstruct))
	  sq_save_data(dellist, memberstruct);
	sq_destroy(memberlist);
	memberlist = sq_create();
    }

    /* Process the add list */
    while (sq_get_data(addlist, &memberstruct)) {
	membervec[0] = listname;
	membervec[2] = memberstruct->name;
	if (verbose && syncflg) {
	    printf("Adding member ");
	    show_list_member(memberstruct);
	}
	switch (memberstruct->type) {
	case M_ANY:
	case M_USER:
	    membervec[1] = "USER";
	    status = sms_query("add_member_to_list", 3, membervec, scream, NULL);
	    if (status == SMS_SUCCESS)
	      break;
	    else if (status != SMS_USER || memberstruct->type != M_ANY) {
		com_err(whoami, status, " while adding member %s to %s",
			memberstruct->name, listname);
		break;
	    }
	case M_LIST:
	    membervec[1] = "LIST";
	    status = sms_query("add_member_to_list", 3, membervec,
			       scream, NULL);
	    if (status == SMS_SUCCESS)
	      break;
	    else if (status != SMS_LIST || memberstruct->type != M_ANY) {
		com_err(whoami, status, " while adding member %s to %s",
			memberstruct->name, listname);
		break;
	    }
	case M_STRING:
	    membervec[1] = "STRING";
	    status = sms_query("add_member_to_list", 3, membervec,
			       scream, NULL);
	    if (status != SMS_SUCCESS)
	      com_err(whoami, status, " while adding member %s to %s",
		      memberstruct->name, listname);
	}
    }

    /* Process the delete list */
    while (sq_get_data(dellist, &memberstruct)) {
	membervec[0] = listname;
	membervec[2] = memberstruct->name;
	if (verbose && syncflg) {
	    printf("Deleting member ");
	    show_list_member(memberstruct);
	}
	switch (memberstruct->type) {
	case M_ANY:
	case M_USER:
	    membervec[1] = "USER";
	    status = sms_query("delete_member_from_list", 3, membervec,
			       scream, NULL);
	    if (status == SMS_SUCCESS)
	      break;
	    else if ((status != SMS_USER && status != SMS_NO_MATCH) ||
		     memberstruct->type != M_ANY) {
		com_err(whoami, status, " while deleteing member %s from %s",
			memberstruct->name, listname);
		break;
	    }
	case M_LIST:
	    membervec[1] = "LIST";
	    status = sms_query("delete_member_from_list", 3, membervec,
			       scream, NULL);
	    if (status == SMS_SUCCESS)
	      break;
	    else if ((status != SMS_LIST && status != SMS_NO_MATCH) ||
		     memberstruct->type != M_ANY) {
		com_err(whoami, status, " while deleteing member %s from %s",
			memberstruct->name, listname);
		break;
	    }
	case M_STRING:
	    membervec[1] = "STRING";
	    status = sms_query("delete_member_from_list", 3, membervec,
			       scream, NULL);
	    if (status == SMS_STRING && memberstruct->type == M_ANY)
	      com_err(whoami, 0, "Unable to find member %s to delete from %s",
		      memberstruct->name, listname);
	    else if (status != SMS_SUCCESS)
	      com_err(whoami, status, " while deleteing member %s from %s",
		      memberstruct->name, listname);
	}
    }

    /* Display the members of the list now, if requested */
    if (memberflg) {
	if (recursflg)
	  recursive_display_list_members();
	else {
	    status = sms_query("get_members_of_list", 1, &listname,
			       get_list_members, (char *)memberlist);
	    if (status)
	      com_err(whoami, status, " while getting members of list %s",
		      listname);
	    while (sq_get_data(memberlist, &memberstruct))
	      show_list_member(memberstruct);
	}
    }

    /* We're done! */
    sms_disconnect();
    exit(0);
}

usage(argv)
char **argv;
{
    fprintf(stderr, "Usage: %s [options] listname [options]\n",argv[0]);
    fprintf(stderr, "Options are\n");
    fprintf(stderr, "   -v | -verbose\n");
    fprintf(stderr, "   -m | -members\n");
    fprintf(stderr, "   -i | -info\n");
    fprintf(stderr, "   -r | -recursive\n");
    fprintf(stderr, "   -a | -add member\n");
    fprintf(stderr, "   -d | -delete member\n");
    fprintf(stderr, "   -f | -file filename\n");
    fprintf(stderr, "   -D | -debug\n");
    exit(1);
}


/* Display the members stored in the queue */

show_list_member(memberstruct)
struct member *memberstruct;
{
    if (verbose) {
	char *s;
	switch (memberstruct->type) {
	case M_USER:
	    s = "USER";
	    break;
	case M_LIST:
	    s = "LIST";
	    break;
	case M_STRING:
	    s = "STRING";
	    break;
	case M_ANY:
	    printf("%s\n", memberstruct->name);
	    return;
	}
	printf("%s:%s\n", s, memberstruct->name);
    } else {
	if (memberstruct->type == M_LIST)
	  printf("LIST:%s\n", memberstruct->name);
	else if (memberstruct->type == M_STRING &&
		 !index(memberstruct->name, '@'))
	  printf("STRING:%s\n", memberstruct->name);
	else
	  printf("%s\n", memberstruct->name);
    }
}


/* Show the retrieved information about a list */

show_list_info(argc, argv, hint)
int argc;
char **argv;
int hint;
{
    printf("List: %s\n", argv[0]);
    printf("Description: %s\n", argv[9]);
    printf("Flags: %s, %s, and %s\n",
	   atoi(argv[1]) ? "active" : "inactive",
	   atoi(argv[2]) ? "public" : "private",
	   atoi(argv[3]) ? "hidden" : "visible");
    printf("%s is %sa maillist and is %sa group", argv[0],
	   atoi(argv[4]) ? "" : "not ",
	   atoi(argv[5]) ? "" : "not ");
    if (atoi(argv[5]))
      printf(" with GID %d\n", atoi(argv[6]));
    else
      printf("\n");
    printf("Owner: %s %s\n", argv[7], argv[8]);
    printf("Last modified by %s with %s on %s\n", argv[11], argv[12], argv[10]);
    return(SMS_CONT);
}


/* Show the retrieve list member count */

show_list_count(argc, argv, hint)
int argc;
char **argv;
int hint;
{
    printf("Members: %s\n", argv[0]);
}


/* Recursively find all of the members of listname, and then display them */

recursive_display_list_members()
{
    int status, count, savecount;
    struct save_queue *lists, *members;
    struct member *m, *m1, *data;

    lists = sq_create();
    members = sq_create();
    m = (struct member *) malloc(sizeof(struct member));
    m->type = M_LIST;
    m->name = listname;
    sq_save_data(lists, m);

    while (sq_get_data(lists, &m)) {
	sq_destroy(memberlist);
	memberlist = sq_create();
	if (debugflg)
	  fprintf(stderr, "Fetching members of %s\n", m->name);
	status = sms_query("get_members_of_list", 1, &(m->name),
			   get_list_members, (char *)memberlist);
	if (status)
	  com_err(whoami, status, " while getting members of list %s", m->name);
	while (sq_get_data(memberlist, &m1)) {
	    if (m1->type == M_LIST)
	      unique_add_member(lists, m1);
	    else
	      unique_add_member(members, m1);
	}
    }
    savecount = count = sq_count_elts(members);
    data = (struct member *) malloc(count * sizeof(struct member));
    count = 0;
    while (sq_get_data(members, &m))
      bcopy(m, &data[count++], sizeof(struct member));
    qsort(data, count, sizeof(struct member), membercmp);
    for (count = 0; count < savecount; count++) {
	show_list_member(&data[count]);
    }
}


/* add a struct member to a queue if that member isn't already there. */

unique_add_member(q, m)
struct save_queue  *q;
struct member *m;
{
    struct save_queue *qp;

    for (qp = q->q_next; qp != q; qp = qp->q_next) {
	if (!membercmp(qp->q_data, m))
	  return;
    }
    sq_save_data(q, m);
}


/* Collect the retrieved members of the list */

get_list_members(argc, argv, q)
int argc;
char **argv;
struct save_queue *q;
{
    struct member *m;

    m = (struct member *) malloc(sizeof(struct member));
    switch (argv[0][0]) {
    case 'U':
	m->type = M_USER;
	break;
    case 'L':
	m->type = M_LIST;
	break;
    case 'S':
	m->type = M_STRING;
	break;
    }
    m->name = strsave(argv[1]);
    sq_save_data(q, m);
    return(SMS_CONT);
}


/* Called only if a query returns a value that we weren't expecting */

scream()
{
    fprintf(stderr, "Programmer botch\n");
    exit(3);
}


/* Parse a line of input, fetching a member.  NULL is returned if a member
 * is not found.  Only the first token on the line is parsed.  ';' is a
 * comment character.
 */

struct member *parse_member(s)
register char *s;
{
    register struct member *m;
    char *p;

    while (*s && isspace(*s))
      s++;
    p = s;
    while (*p && isprint(*p) && !isspace(*p) && *p != ';')
      p++;
    *p = 0;
    if (p == s || strlen(s) == 0)
      return(NULL);

    if ((m = (struct member *) malloc(sizeof(struct member))) == NULL)
      return(NULL);

    if (p = index(s, ':')) {
	*p = 0;
	m->name = ++p;
	if (!strcasecmp("user", s))
	  m->type = M_USER;
	else if (!strcasecmp("list", s))
	  m->type = M_LIST;
	else if (!strcasecmp("string", s))
	  m->type = M_STRING;
	else {
	    m->type = M_STRING;
	    *(--p) = ':';
	    m->name = s;
	}
	m->name = strsave(m->name);
	return(m);
    }
    m->name = strsave(s);
    if (index(s, '@') || index(s, '!') || index(s, '%'))
      m->type = M_STRING;
    else
      m->type = M_ANY;
    return(m);
}


  /* 
   * This routine two compares members by the following rules:
   * 1.  A USER is less than a LIST
   * 2.  A LIST is less than a STRING
   * 3.  If two members are of the same type, the one alphabetically first
   *     is less than the other
   * It returs < 0 if the first member is less, 0 if they are identical, and
   * > 0 if the second member is less (the first member is greater).
   */

int membercmp(m1, m2)
  struct member *m1, *m2;
{
    if (m1->type == M_ANY || m2->type  == M_ANY || (m1->type == m2->type))
	return(strcmp(m1->name, m2->name));
    else
	return(m1->type - m2->type);
}


sq_count_elts(q)
struct save_queue *q;
{
    char  *foo;
    int count;

    count = 0;
    while (sq_get_data(q, &foo))
      count++;
    return(count);
}
