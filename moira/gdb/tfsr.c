/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/tfsr.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/tfsr.c,v 1.1 1987-08-02 22:14:34 wesommer Exp $
 */

#ifndef lint
static char *rcsid_tfsr_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/tfsr.c,v 1.1 1987-08-02 22:14:34 wesommer Exp $";
#endif	lint

/************************************************************************/
/*	
/*		          tfsr (test forking server)
/*			  --------------------------
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
/*	A GDB server program demonstrating techniques for asynchronously
/*	communicating with an arbitrary number of clients by forking 
/*	a new server process for each incoming client.
/*	
/*	Each forked child  receives a stream of integers,
/*	which it interprets as ASCII characters.  The characters are
/*	converted to uppercase, and then sent back to the client from
/*	which they came.  
/*	
/*	NOTE
/*	----
/*	
/*	This program is interface compatible with tsr.c.  Clients
/*	cannot tell which style of server they are using.
/*	
/************************************************************************/

#include <stdio.h>
#include "gdb.h"


int
main(argc, argv)
int argc;
char *argv[];
{
	/*----------------------------------------------------------*/
	/*	
	/*		       LOCAL VARIABLES
	/*	
	/*----------------------------------------------------------*/

	CONNECTION client;			/* talk on this to client */

	int data;				/* receive data here */

	/*----------------------------------------------------------*/
	/*	
	/*			EXECUTION BEGINS HERE
	/*	
	/*			  Check parameters
	/*	
	/*----------------------------------------------------------*/

	if (argc != 2) {
		fprintf(stderr,"Correct form is %s <servicename>\n",
			argv[0]);
		exit(4);
	}

	/*----------------------------------------------------------*/
	/*	
	/*			Initialize
	/*	
	/*----------------------------------------------------------*/

	gdb_init();				/* set up gdb */

	/*----------------------------------------------------------*/
	/*	
	/*	Now, turn ourselves into a forking server.
	/*	
	/*----------------------------------------------------------*/

	client = create_forking_server(argv[1],NULL);
	fprintf(stderr,"forked\n");

	/*----------------------------------------------------------*/
	/*	
	/*	Here we are in the child process for each client.
	/*	Echo the characters.
	/*	
	/*----------------------------------------------------------*/

	while (TRUE) {
		if (receive_object(client, &data, INTEGER_T) ==
		    OP_CANCELLED) {
			fprintf(stderr,"receive error\n");
			exit(4);
		}
		if (data >= 'a' && data <= 'z')
			data += 'A'-'a';	/* upcase the response */
		if (send_object(client, &data, INTEGER_T) ==
		    OP_CANCELLED) {
			fprintf(stderr,"send error\n");
			exit(4);
		}
	}
}
