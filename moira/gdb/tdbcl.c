/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/tdbcl.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/tdbcl.c,v 1.1 1987-08-02 22:13:51 wesommer Exp $
 */

#ifndef lint
static char *rcsid_tdbcl_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/tdbcl.c,v 1.1 1987-08-02 22:13:51 wesommer Exp $";
#endif	lint

/************************************************************************/
/*	
/*		          tdbcl (test database client)
/*			  ----------------------------
/*	
/*	Author: Noah Mendelsohn (IBM T.J. Watson Research and MIT Project
/*				 Athena)
/*	
/*	Copyright: 1986 MIT Project Athena
/*	
/************************************************************************/
/*	
/*	PURPOSE
/*	-------
/*	
/*	A GDB client program which accesses a relational database
/*	using the services of GDB.
/*	
/************************************************************************/


#include <stdio.h>
#include "gdb.h"



int ftypes[] = {STRING_T, INTEGER_T, INTEGER_T};
char *fnames[] = {"name", "rank", "serial"};
int
main(argc, argv)
int argc;
char *argv[];
{

	/*----------------------------------------------------------*/
	/*	
	/*			DECLARATIONS
	/*	
	/*----------------------------------------------------------*/

	DATABASE db;				/* this is the database */
						/* we access */
	int rc;					/* return code from */
						/* access_db */
	TUPLE_DESCRIPTOR tpd;
	RELATION rel;
#define MAX_OPS 50
	OPERATION ops[MAX_OPS];
	int next_op=0;				/* next op to use */
	int i;

	/*----------------------------------------------------------*/
	/*	
	/*	 	    EXECUTION BEGINS HERE
	/*	
	/*	Make sure the command line specifies the name
	/*	of the host on which the server program is running.
	/*	
	/*----------------------------------------------------------*/

	if (argc != 3) {
               /* 
                * Tell 'em the syntax
                */
		fprintf(stderr, "tcl <database-name@host> <query>\n");
		exit (4);
	}

	/*----------------------------------------------------------*/
	/*	
	/*	Initialize the GDB library.
	/*	
	/*----------------------------------------------------------*/

	gdb_init();

	/*----------------------------------------------------------*/
	/*	
	/*	Create all the operations
	/*	
	/*----------------------------------------------------------*/
	for (i=0; i<MAX_OPS; i++)
		ops[i] = create_operation();

	/*----------------------------------------------------------*/
	/*	
	/*	Try for a connection to the server on the 
	/*	specified host.
	/*	
	/*----------------------------------------------------------*/

	printf("Attempting database: %s\n", argv[1]);

	if (rc = access_db(argv[1], &db) != DB_OPEN) {
		fprintf(stderr,"Return code from access_db is %d\n",rc);
		exit(8);
	}

        if (db == NULL) {
		fprintf(stderr,"Got null db after access_db reported success\n");
		exit(8);
	}

	/*----------------------------------------------------------*/
	/*	
	/*	Try destroying and creating a table
	/*	
	/*----------------------------------------------------------*/

	printf("Now queueing operations\n");

	rc = start_performing_db_operation(ops[next_op++],db, "destroy tdbcl");

	rc = start_performing_db_operation(ops[next_op++],db, "create tdbcl (name=c8, rank = i4, serial=i4)");

	/*----------------------------------------------------------*/
	/*	
	/*	Append some data to the table
	/*	
	/*----------------------------------------------------------*/

	rc = start_performing_db_operation(ops[next_op++],db, "append to  tdbcl (name=\"noah\", rank = 1, serial=123)");
	rc = start_performing_db_operation(ops[next_op++],db, "append to  tdbcl (name=\"mike\", rank = 5, serial=456)");
	rc = start_performing_db_operation(ops[next_op++],db, "append to  tdbcl (name=\"mike1\", rank = 5, serial=456)");
	rc = start_performing_db_operation(ops[next_op++],db, "append to  tdbcl (name=\"mike2\", rank = 5, serial=456)");
	rc = start_performing_db_operation(ops[next_op++],db, "append to  tdbcl (name=\"mike3\", rank = 5, serial=456)");
	rc = start_performing_db_operation(ops[next_op++],db, "append to  tdbcl (name=\"mike4\", rank = 5, serial=456)");
	rc = start_performing_db_operation(ops[next_op++],db, "append to  tdbcl (name=\"mike5\", rank = 5, serial=456)");
	rc = start_performing_db_operation(ops[next_op++],db, "append to  tdbcl (name=\"mike6\", rank = 5, serial=456)");
	rc = start_performing_db_operation(ops[next_op++],db, "append to  tdbcl (name=\"mike7\", rank = 5, serial=456)");
	rc = start_performing_db_operation(ops[next_op++],db, "append to  tdbcl (name=\"mike8\", rank = 5, serial=456)");
	rc = start_performing_db_operation(ops[next_op++],db, "append to  tdbcl (name=\"mike9\", rank = 5, serial=456)");
	rc = start_performing_db_operation(ops[next_op++],db, "append to  tdbcl (name=\"mike10\", rank = 5, serial=456)");
	rc = start_performing_db_operation(ops[next_op++],db, "append to  tdbcl (name=\"mike11\", rank = 5, serial=456)");
	rc = start_performing_db_operation(ops[next_op++],db, "append to  tdbcl (name=\"mike12\", rank = 5, serial=456)");
	rc = start_performing_db_operation(ops[next_op++],db, "append to  tdbcl (name=\"mike13\", rank = 5, serial=456)");

	
	/*----------------------------------------------------------*/
	/*	
	/*	Complete the operations and print the return codes 
	/*	from them
	/*	
	/*----------------------------------------------------------*/

	printf("Operations have been queued, waiting for last one to complete\n");
	complete_operation(ops[next_op-1]);
	for (i=0; i<=next_op; i++)
		printf("%d: status=%d, result=%d\n", i, OP_STATUS(ops[i]),
		       OP_RESULT(ops[i]));

	/*----------------------------------------------------------*/
	/*	
	/*	Do a query
	/*	
	/*----------------------------------------------------------*/

	tpd = create_tuple_descriptor(3, fnames, ftypes);

	rel = create_relation(tpd);

	rc = db_query(db, rel, /*argv[2]*/ "(>*name*<=tdbcl.name, >*serial*<=tdbcl.serial) where tdbcl.serial>200");

	printf("return code from query is %d\n",rc);
	if (rc == 0)
		print_relation("Query result", rel);

	delete_relation(rel);
	rel = create_relation(tpd);

	rc = db_query(db, rel, /*argv[2]*/ "(>*name*<=tdbcl.name, >*serial*<=tdbcl.serial) where tdbcl.serial>200");

	printf("return code from query is %d\n",rc);
	if (rc == 0)
		print_relation("Query result", rel);

	delete_relation(rel);
	delete_tuple_descriptor(tpd);

	return;
}
			
