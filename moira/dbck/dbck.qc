/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dbck/dbck.qc,v 1.6 1990-03-19 18:43:57 mar Exp $
 *
 * Moira database consistency checker
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


static char dbck_qc_rcsid[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dbck/dbck.qc,v 1.6 1990-03-19 18:43:57 mar Exp $";


int debug = 0;
int mode = MODE_ASK;
int fast = 0;
int warn = 1;
int abort = 0;
struct hash *users, *machines, *clusters, *lists, *filesys, *nfsphys;
struct hash *strings, *members;
##int dcmenable;
struct save_queue *modtables, *sq_create();
int interrupt();


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
	  case 'w':
	      warn = 0;
	      break;
	  default:
	      printf("Usage: %s [-d level] [-n] [-y] [-p] [-a] [-c] [-f] [-w] [database]\n",
		     argv[0]);
	      exit(1);
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

    signal(SIGHUP, interrupt);
    signal(SIGQUIT, interrupt);
    signal(SIGINT, interrupt);
    modtables = sq_create();

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
	count_only_setup();
    }
    phase4();

##  end transaction
    cleanup();
    printf("Done.\n");
    exit(0);
}

ingerr(num)
int	*num;
{
    printf("An ingres error occuurred, code %d\n", *num);
    printf("Aborting...\n");
    if (!abort) {
	abort++;
##	abort
    }
    exit(1);
}


int interrupt()
##{
    printf("Signal caught\n");
    if (prompt("Save database changes")) {
	/* break out of a retrieve loop */
	IIbreak();
##	end transaction
	cleanup();
	exit(0);
    }
    printf("Aborting transaction\n");
    if (!abort) {
	abort++;
	/* break out of a retrieve loop */
	IIbreak();
##  	abort
    }
##  replace values (value = dcmenable) where values.name = "dcm_enable"
##  exit
    exit(0);
##}


modified(table)
char *table;
{
    sq_save_unique_string(modtables, table);
}

cleanup()
##{
##  char *tab;

    while (sq_get_data(modtables, &tab)) {
##	replace tblstats (modtime = "now") where tblstats.table = tab
    }
##  replace values (value = dcmenable) where values.name = "dcm_enable"
##  exit
##}


out_of_mem(msg)
char *msg;
{
    fprintf(stderr, "Out of memory while %s\n", msg);
##  end transaction    
    cleanup();
    exit(1);
}
