/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/tst.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/tst.c,v 1.1 1987-08-02 22:15:04 wesommer Exp $
 */

#ifndef lint
static char *rcsid_tst_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/tst.c,v 1.1 1987-08-02 22:15:04 wesommer Exp $";
#endif	lint

/************************************************************************/
/*	
/*				tst.c
/*	
/*	A test program for the client library interface.
/*	
/*	Author: Noah Mendelsohn
/*	
/*	Copyright: 1986 MIT Project Athena
/*	
/************************************************************************/

#include <stdio.h>
#include "gdb.h"

char *field_names[] = {"desc",	
		       "code",
		       "man",
		       "cost",
		       "count"};
FIELD_TYPE field_types[] = {STRING_T,	    /* desc */
			    INTEGER_T,      /* code */
			    STRING_T,	    /* man */
			    REAL_T,	    /* cost */
			    INTEGER_T};	    /* count */

FILE *coded_file;

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
	int i;
	/*
	 * The following defines are for convenience in addressing
	 * the fields.
	 */

#define DESC  0
#define CODE  1
#define MAN   2
#define COST  3
#define COUNT 4

	/*
	 * Declare the relation and related data structures for
	 * storing the retrieved data.
	 */

	TUPLE_DESCRIPTOR tuple_desc;
	RELATION retrieved_data, decoded_rel;

	/*
	 * Declarations for misc. variables
	 */
	
	TUPLE   t;			/* next tuple to print */
	int coded_len;
	char *coded_dat;

	int	rc;			/* A return code */	

	/************************************************************
	 *	       EXECUTION BEGINS HERE 			    *
	 ************************************************************/

	/*
	 * Build the descriptor describing the layout of the tuples
	 * to be retrieved, and create an empty relation into which
	 * the retrieval will be done.
	 */

	gdb_init();				/* initialize the global */
						/* database facility */

	printf("tst.c: attempting to create tuple descriptor\n");	

	tuple_desc = create_tuple_descriptor(field_count, field_names,
					     field_types);

	printf("tst.c: tuple desc created.. attempting to create relation\n");
	retrieved_data = create_relation(tuple_desc);

	printf("tst.c: relation created, formatting descriptor\n");


	print_tuple_descriptor("Test Tuple Descriptor", tuple_desc);

	printf("tst.c: descriptor formatted, formatting relation\n");

	print_relation("Test Relation", retrieved_data);

	printf("Creating tuples\n");

	for (i=0; i<3; i++) {
		t = create_tuple(tuple_desc);
		
		initialize_tuple(t);

		fprintf(stderr, "Following tuple should contain null fields:\n\n");

		print_tuple("A NULL TUPLE", t);

		*(int *)FIELD_FROM_TUPLE(t, CODE) = i+1;

		*(double *)FIELD_FROM_TUPLE(t, COST) = 12.34 * (i+1);
		string_alloc((STRING *)FIELD_FROM_TUPLE(t,MAN), 20);
		strcpy(STRING_DATA(*((STRING *)FIELD_FROM_TUPLE(t,MAN))),
		       "Manager field data");
		ADD_TUPLE_TO_RELATION(retrieved_data, t);		
	}


	printf("tst.c: relation initialized, formatting relation\n");

	print_relation("Test Relation", retrieved_data);


/*
 *  Try to encode the entire relation!!
 */

	printf("Attempting to encode the relation\n");

	coded_len = FCN_PROPERTY(RELATION_T, CODED_LENGTH_PROPERTY)
	      (&retrieved_data, NULL);

	coded_dat = (char *)malloc(coded_len);

	FCN_PROPERTY(RELATION_T, ENCODE_PROPERTY)
	  (&retrieved_data, NULL, coded_dat);

	printf("Relation encoding complete, writing file \n\n");

	coded_file = fopen("coded.dat", "w");

	fwrite(coded_dat, 1, coded_len, coded_file);

	fclose(coded_file);

	printf("File written\n");

	printf("Decoding relation\n\n");

	FCN_PROPERTY(RELATION_T, DECODE_PROPERTY)
	  (&decoded_rel, NULL, coded_dat);

	printf("Relation decoded!!  Printing it\n\n");

	print_relation("Decoded Relation", decoded_rel);


	printf("tst.c: exit\n");

	return 0;
}
