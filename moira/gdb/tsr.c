/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/tsr.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/tsr.c,v 1.1 1987-08-02 22:14:47 wesommer Exp $
 */

#ifndef lint
static char *rcsid_tsr_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/tsr.c,v 1.1 1987-08-02 22:14:47 wesommer Exp $";
#endif	lint

/************************************************************************/
/*	
/*		          tsr (test server)
/*			  -----------------
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
/*	communicating with an arbitrary number of clients from a single
/*	Unix server process.  This server accepts GDB connections from
/*	clients as requests come in (up to the arbitrary maximum
/*	MAXCLIENTS.)  On each connection, it receives a stream of integers,
/*	which it interprets as ASCII characters.  The characters are
/*	converted to uppercase, and then sent back to the client from
/*	which they came.  
/*	
/*	All of this is done completely asynchronously.  No client is 
/*	locked out while characters are being echoed to another, and
/*	new connections are accepted at any time.
/*	
/*	NOTES
/*	-----
/*	
/*	1) The complete state of each client is kept in the array
/*	   named client.  The client[i].state variable indicates whether
/*	   client i is active, and if so, the client[i].action variable
/*	   indicates what kind of asynchronous activity the client is 
/*	   engaged in.  Note that these are local conventions, having
/*	   nothing to do with GDB or its interfaces.  
/*	
/*	2) Communication to each client is done over its connection,
/*	   named client[i].con.
/*	
/*	3) There is at most one asynchronous activity pending to 
/*	   each client at any given time, and its state is tracked
/*	   in the variable named client[i].pending_op.  The operation
/*	   may be a send, a receive, or an accept, depending on 
/*	   the contents of client[i].action.  These operations are
/*	   allocated when the server starts up, and then re-used
/*	   repeatedly.  They are the GDB analog of a lightweight process,
/*	   which is activated when queued on a connection.
/*	
/*	4) A special form of connection and a special listening operation
/*	   are used for asynchronously listening for new connection
/*	   requests.  These are 'listencon' and 'listenop' respectively.
/*	
/*	5) GDB includes a special form of select which waits for 
/*	   completion of operations as well as for activity on user
/*	   specified file descriptors.  The list of operations to be
/*	   monitored is stored in the variable
/*	   named op_list.  The call to op_select_any hangs until one
/*	   or more of these operations complete, then terminates.
/*	
/*	6) The main server loop acts on any new connection requests,
/*	   processes any newly completed activity on the active
/*	   clients, then drops into op_select_any to allow asynchronous
/*	   activities to progress.  
/*	
/*	
/************************************************************************/

#include <stdio.h>
#include "gdb.h"

extern int errno;


/************************************************************************/
/*	
/*			     DECLARATIONS
/*	
/************************************************************************/

#define MAXCLIENTS 10

	/*----------------------------------------------------------*/
	/*	
	/*	State of each possible client
	/*	
	/*----------------------------------------------------------*/

struct client {
	int state;				/* state of this client */
						/* descriptor */
#define CL_DEAD 1				/* client not started */
#define CL_STARTING 2				/* accepted, reply ongoing */
#define CL_UP 3					/* ready to go */
	int action;				/* what are we doing now */
#define CL_RECEIVE 4				/* waiting for a packet */
#define CL_SEND    5				/* sending a packet */
#define CL_ACCEPT  6				/* sending a reply */
	CONNECTION con;				/* connection to this */
						/* client, if any */
	OPERATION pending_op;			/* pending operation */
						/* on this connection, */
						/* if any */
	int data;				/* the character to echo */
						/* goes here, expressed as */
						/* an int  */
};

struct client client[MAXCLIENTS];

	/*----------------------------------------------------------*/
	/*	
	/*	Connections and operations for listening for
	/*	new clients.
	/*	
	/*----------------------------------------------------------*/

CONNECTION listencon;				/* listen on this */
						/* connection */
OPERATION  listenop;				/* this operation is used */
						/* repeatedly for listening */
						/* for new clients */

int	nextcl = 0;				/* index of the next client */
						/* we'll accept */

	/*----------------------------------------------------------*/
	/*	
	/*	Miscellaneous variables used in acquiring connections.
	/*	These are ignored in a simple server like this;  a 
	/*	more sophisticated server might want to validate the
	/*	names of its clients before accepting connections.
	/*	
	/*----------------------------------------------------------*/

TUPLE client_tuple;				/* client request goes */
						/* here */
char otherside[100];
int othersize;



/************************************************************************/
/*	
/*				MAIN
/*	
/************************************************************************/

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

	register int i;				/* loop index */
	LIST_OF_OPERATIONS op_list;		/* for op_select_any */

	/*----------------------------------------------------------*/
	/*	
	/*	Check parameters
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
	init_clients();				/* null the client states */
	do_listen(argv[1]);			/* start the listening */
						/* connection and queue */
						/* a listening operation */
	make_oplist(&op_list);			/* create wait list */

	/*----------------------------------------------------------*/
	/*	
	/*	     Loop forever taking care of business.
	/*	
	/*	1) If any new connection requests have come in,
	/*	   accept them.
	/*	
	/*	2) For each client on which some activity is newly
	/*	   completed, take care of it.
	/*	
	/*----------------------------------------------------------*/

	while (TRUE) {
		if (OP_DONE(listenop))
			new_connection();
		for (i=0; i<MAXCLIENTS; i++) {
			if (OP_DONE(client[i].pending_op))
				do_client(i);
		}
		if(op_select_any(op_list, 0, NULL, NULL, NULL, NULL)==(-1)) {
			perror("op_select_any returned error");
			exit(32);
		}
	}
}

/************************************************************************/
/*	
/*				do_client
/*	
/*	An operation has completed on the specified client.
/*	
/************************************************************************/

int
do_client(id)
int id;
{
	register struct client *cp = &(client[id]);

       /*
        * If there has been an error, shutdown the client.
        */
	connection_perror(cp->con, "Unix error on send or receive"); 
						/* print error if any */
	if (connection_status(cp->con) != CON_UP ||
	    OP_STATUS(cp->pending_op) == OP_CANCELLED) {
		sever_connection(cp->con);
		reset_operation(cp->pending_op);
		cp->state = CL_DEAD;
		cp->action = 0;
		return;
	}
       /*
        * The operation completed successfully.  Figure out what it was
        * and do the right thing.
        */
	switch (cp->action) {
	      case CL_ACCEPT:
	      case CL_SEND:
		start_receiving_object(cp->pending_op, cp->con, 
				       (char *)&cp->data,
				       INTEGER_T);
		cp->action = CL_RECEIVE;
		break;
	      case CL_RECEIVE:
		if (cp->data >= 'a' && cp->data <= 'z')
			cp->data += 'A'-'a';	/* upcase the response */
		start_sending_object(cp->pending_op, cp->con, 
				       (char *)&cp->data,
				       INTEGER_T);
		cp->action = CL_SEND;
	}
}

/************************************************************************/
/*	
/*			  init_clients
/*	
/************************************************************************/

int
init_clients()
{
	register struct client *c;

	for (c=client; c<client+MAXCLIENTS; c++){
		c->state = CL_DEAD;
		c->action = 0;
		c->con = NULL;
		c->pending_op = create_operation();
		reset_operation(c->pending_op);		
	}
}



/************************************************************************/
/*	
/*			make_oplist
/*	
/************************************************************************/

int
make_oplist(oplp)
LIST_OF_OPERATIONS *oplp;
{
       /*
        * ugh! we've got to fix create_list_of_operations to be
        * more flexible!!
        */

	*oplp = create_list_of_operations(MAXCLIENTS+1, listenop,
					  client[0].pending_op,
					  client[1].pending_op,
					  client[2].pending_op,
					  client[3].pending_op,
					  client[4].pending_op,
					  client[5].pending_op,
					  client[6].pending_op,
					  client[7].pending_op,
					  client[8].pending_op,
					  client[9].pending_op);
}
/************************************************************************/
/*	
/*				do_listen
/*	
/*	Do the one time setup for listening for clients, and
/*	also start a listen for an actual client.
/*	
/************************************************************************/

int
do_listen(service)
char *service;
{

	/*----------------------------------------------------------*/
	/*	
	/*	Make a listening connection
	/*	
	/*----------------------------------------------------------*/

	fprintf(stderr, "Server creating listening connection\n");
	listencon = create_listening_connection(service);

	if (listencon == NULL  || connection_status(listencon) != CON_UP) {
		if(connection_status(listencon) == CON_STOPPING) {
			connection_perror(listencon, 
					  "Unix error creating listening connection");
		}
		fprintf(stderr,"tsr: could not create listening connection\n");
		exit (4);
	}

	/*----------------------------------------------------------*/
	/*	
	/*	On that connection, put up an operation to listen
	/*	for our first client.
	/*	
	/*----------------------------------------------------------*/

	listenop = create_operation();

	othersize = sizeof(otherside);

	start_accepting_client(listencon, listenop, &(client[nextcl].con),
			       (char *)otherside,
			       &othersize, &client_tuple);

}

/************************************************************************/
/*	
/*			new_connection
/*	
/*	We have just gotten a connection for client nextcl.  
/*	
/************************************************************************/

int
new_connection()
{
	register struct client *cp = &client[nextcl];
       /*
        * Make sure there's been no error
        */
	if(connection_status(listencon) != CON_UP) {
		connection_perror(listencon, "Unix error on listening connection");
		fprintf(gdb_log, "Listening connection has died.\n");
		exit(8);
	}
	if(OP_STATUS(listenop) != OP_COMPLETE ||
	   cp->con == NULL || connection_status(cp->con) != CON_UP) {
		fprintf(stderr,"Error on listening operation\n");
		if (cp->con != NULL && 
		    connection_status(cp->con)==CON_STOPPING) {
			 connection_perror(cp->con, 
					   "Error on newly started client connection.");
			 sever_connection(cp->con);
			 cp->con = NULL;
		} else
			exit(8);
	} else {
	       /*
        	* Set up the new connection and reply to the client
	        */
		cp->state = CL_STARTING;
		cp->action = CL_ACCEPT;
		start_replying_to_client(cp->pending_op, cp->con, GDB_ACCEPTED,
					 "", "");
	       /*
        	* Find a new free connection descriptor.  Blow up if
                * we've used the last one
        	*/
		for (nextcl=0; nextcl<MAXCLIENTS; nextcl++)
			if (client[nextcl].state == CL_DEAD)
				break;

		if (nextcl == MAXCLIENTS) {
			fprintf(stderr,"Too many clients, giving up\n");
			exit(8);
		}
	}
       /*
        * Start listening again
        */
	reset_operation(listenop);
	othersize = sizeof(otherside);

	start_accepting_client(listencon, listenop, &(client[nextcl].con),
			       (char *)otherside,
			       &othersize, &client_tuple);


}
