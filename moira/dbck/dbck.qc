/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dbck/dbck.qc,v 1.1 1988-09-25 21:48:56 mar Exp $
 *
 * SMS database consistency checker
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <stdio.h>
#include <strings.h>
#include <signal.h>
#include "dbck.h"


int debug = 1;
int mode = MODE_ASK;
int fast = 0;
struct hash *users, *machines, *clusters, *lists, *filesys, *nfsphys, *strings;
##int dcmenable;
int cleanup();


main(argc, argv)
int argc;
char **argv;
{
    char **arg = argv;
##  char *database;
    int ingerr();
    int countonly = 0;

    database = "sms";

    while (++arg - argv < argc) {
	if  (**arg == '-')
	  switch ((*arg)[1]) {
	  case 'd':
	      debug = atoi((*arg)[2] ? *arg+2 : *++arg);
	      break;
	  case 'n':
	      mode = MODE_NO;
	      break;
	  case 'y':
	      mode = MODE_YES;
	      break;
	  case 'p':
	      mode = MODE_PREEN;
	      break;
	  case 'a':
	      mode = MODE_ASK;
	      break;
	  case 'c':
	      countonly++;
	      break;
	  case 'f':
	      fast++;
	      break;
	  }
	else
	  database = *arg;
    }
    if (countonly)
      printf("Only doing counts\n");
    else if (fast)
      printf("Doing fast version (skipping some checks)\n");
    if (mode == MODE_NO)
      printf("Will NOT modify the database\n");
    else if (mode == MODE_PREEN)
      printf("Will fix simple things without asking\n");
    else if (mode == MODE_YES)
      printf("Will fix everything without asking\n");
    if (debug)
      printf("Debug level is %d\n", debug);

    signal(SIGHUP, cleanup);
    signal(SIGQUIT, cleanup);
    signal(SIGINT, cleanup);

    IIseterr(ingerr);
    printf("Opening database %s...", database);
    fflush(stdout);
##  ingres database
    printf("done\n");
##  retrieve (dcmenable = values.value) where values.name = "dcm_enable"
    dprintf("DCM disabled (was %d)\n", dcmenable);
##  replace values (value = 0) where values.name = "dcm_enable"

##  begin transaction

    if (!countonly) {
	phase1();
	phase2();
	phase3();
    } else {
	printf("Skipping to counts\n");
	count_only_setup();
    }
    phase4();

##  end transaction
##  replace values (value = dcmenable) where values.name = "dcm_enable"
    printf("Done.\n");
##  exit
    exit(0);
}

ingerr(num)
int	*num;
{
    printf("An ingres error occuurred, code %d\n", *num);
    printf("Aborting...\n");
##  abort
    exit(1);
}


int cleanup()
##{
    printf("Signal caught, aborting transaction\n");
##  abort
##  replace values (value = dcmenable) where values.name = "dcm_enable"
##  exit
    exit(0);
##}
