/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/samp1a.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/samp1a.c,v 1.1 1987-08-02 22:13:22 wesommer Exp $
 */

#ifndef lint
static char *rcsid_samp1a_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/samp1a.c,v 1.1 1987-08-02 22:13:22 wesommer Exp $";
#endif	lint

#define terminatedb(x) /* nothin doin yet */
/*
 *			sample 1
 *
 *	     A simple database query program.
 *
 *	This sample program illustrates the use of the 'client library'
 *      for remote access to a relational database.  This version of
 *	the sample is about the simplest possible.  It accesses only
 *	a single database, it does the access syncrhonously, and it uses
 *	only system defined types.  Many real applications will fit this
 *	model.
 *
 *	For purposes of illustration, this program accesses a database
 *	of parts.  For each part where the value of current inventory,
 *	defined as number on hand times cost per unit, exceeds $1000,
 *	this routine prints the description of the part, a code number,
 *      the manufacturer, the cost, and the number on hand.
 *
 *	Because this program accesses the database using the client
 * 	library, it may be run from any Berkeley Unix system on the internet.
 *	Necessary data conversions are done automatically if the database
 *      itself happens to live on an incompatible machine.
 *
 *	Author: Noah Mendelsohn
 */

#include <stdio.h>
#include "gdb.h"

char *field_names[] = {"desc",	
		       "code",
		       "man",
		       "cost",
		       "cnt"};
FIELD_TYPE field_types[] = {STRING_T,	    /* desc */
			    INTEGER_T,      /* code */
			    STRING_T,	    /* man */
			    REAL_T,	    /* cost */
			    INTEGER_T};	    /* count */

int
main(argc, argv)
int	argc;
char	*argv[];
{

	/************************************************************
	 *	            DECLARATIONS			    *
	 ************************************************************/

	/*
	 * Declare the names of fields to be retrieved and their types
	 */

	int  field_count = 5;


	/*
	 * The following defines are for convenience in addressing
	 * the fields.
	 */

#define DESC  0
#define CODE  1
#define MAN   2
#define COST  3
#define CNT 4

	/*
	 * Declare the relation and related data structures for
	 * storing the retrieved data.
	 */

	TUPLE_DESCRIPTOR tuple_desc;
	RELATION retrieved_data;

	/*
	 *  Declare a handle to identify our session with the database
	 *  server.
	 */

	DATABASE parts_data;

	/*
	 * Declarations for misc. variables
	 */
	
	TUPLE   t;			/* next tuple to print */
	int	rc;			/* A return code */	

	/************************************************************
	 *	       EXECUTION BEGINS HERE 			    *
	 ************************************************************/
	/*
	 * Initialize gdb
	 */
  	 gdb_init();

	/*
	 * Open a connection to the database - identify session as parts_data
	 */

	if (access_db("noah@menelaus", &parts_data) != DB_OPEN) {
		printf("Cannot connect to parts database--giving up\n");
		return;
	}

	/*
	 * Build the descriptor describing the layout of the tuples
	 * to be retrieved, and create an empty relation into which
	 * the retrieval will be done.
	 */

	tuple_desc = create_tuple_descriptor(field_count, field_names,
					     field_types);
	retrieved_data = create_relation(tuple_desc);

	/*
	 * Do the query for parts with inventory over $1000
	 * Put results in the relation named retrieved_data.
	 */

	rc = db_query(parts_data, retrieved_data,
		      "(>*desc*< = parts.desc,>*code*< = parts.code,>*man*<  = parts.man,>*cost*< = parts.cost,>*cnt*< = parts.cnt) where (parts.cnt * parts.cost > 1000.00)");

	if (rc != OP_SUCCESS) {
		printf("Error during retrieval--giving up\n");
		terminatedb(parts_data);
		return;
	}

	/*
	 * Print out the results
	 */

	for (t = FIRST_TUPLE_IN_RELATION(retrieved_data); t!= NULL;
	     t = NEXT_TUPLE_IN_RELATION(retrieved_data,t))
		print_a_line(t);

	printf("\nEnd of Report.\n\n");

	/*
	 *  Clean up and leave
	 */

	terminatedb(parts_data);
	return;
}

/*
 *		print_a_line
 *
 *	Given a tuple with parts data, print it on standard output.
 *      NOTE: for clarity, we've left out the casts which should be
 *      done on the pointers returned from FIELD_FROM_TUPLE.
 */
int
print_a_line(tup)
TUPLE	tup;
{
	printf("desc=%s  ",	  STRING_DATA(*(STRING *)(FIELD_FROM_TUPLE(tup, DESC))));
	printf("code=%d  ", 	  *(int *)(FIELD_FROM_TUPLE(tup, CODE)));
	printf("manufacturer=%s ",STRING_DATA(*(STRING *)(FIELD_FROM_TUPLE(tup, MAN))));
	printf("cost=%f  ", 	  *(double *)(FIELD_FROM_TUPLE(tup, COST)));
	printf("count=%d\n", 	  *(int *)(FIELD_FROM_TUPLE(tup, CNT)));
}
