/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_main.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_main.c,v 1.1 1987-05-31 22:06:56 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *
 * 	SMS server process.
 *
 * 	Most of this is stolen from ../gdb/tsr.c
 *
 * 	You are in a maze of twisty little finite automata, all different.
 * 	Let the reader beware.
 * 
 *	$Log: not supported by cvs2svn $
 */

#ifndef lint
static char *rcsid_sms_main_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_main.c,v 1.1 1987-05-31 22:06:56 wesommer Exp $";
#endif lint

#include "sms_private.h"
#include "sms_server.h"
#include <strings.h>
extern char *malloc();

CONNECTION newconn, listencon;
int nclients;

client **clients;

OPERATION listenop;

LIST_OF_OPERATIONS op_list;

/*
 * What is this??
 */
char otherside[100];
int othersize;
TUPLE client_tuple;				/* client request goes */
						/* here */


main(argc, argv)
{
	int i;
	
	gdb_init();
	nclients = 0;
	clients = (client **) malloc(0);
	
	do_listen(index(SMS_GDB_SERV, ':') + 1); /* XXX */

	op_list = create_list_of_operations(1, listenop);

	fprintf(stderr, "sms server on the air..\n");
	
	for EVER {
		op_select_any(op_list, 0, NULL, NULL, NULL, NULL);
		fprintf(stderr, "tick\n");
		if (OP_DONE(listenop)) {
			new_connection();
		}
		for (i=0; i<nclients; i++) {
			if (OP_DONE(clients[i]->pending_op)) {
				do_client(clients[i]);
			}
		}

	}
}

new_connection()
{
	register client *cp = (client *)malloc(sizeof *cp);
	
	fprintf(stderr, "New connection coming in\n");
	/*
	 * Make sure there's been no error
	 */
	if(OP_STATUS(listenop) != OP_COMPLETE ||
	   newconn == NULL) {
		fprintf(stderr,"Error on listening operation\n");
		exit(8);
	}

	/* Add a new client to the array.. */
	clist_append(cp);
	
	/*
	 * Set up the new connection and reply to the client
	 */

	cp->state = CL_STARTING;
	cp->action = CL_ACCEPT;
	cp->con = newconn;
	newconn = NULL;
	
	cp->pending_op = create_operation();
	reset_operation(cp->pending_op);
	oplist_append(&op_list, cp->pending_op);

	start_replying_to_client(cp->pending_op, cp->con, GDB_ACCEPTED,
				 "", "");
	
#ifdef notdef
	to do this in a production server is moronic;
		
	if (nextcl == MAXCLIENTS) {
		fprintf(stderr,"Too many clients, giving up\n");
		exit(8);
	}
#endif notdef
	
	/*
	 * Start listening again
	 */
	reset_operation(listenop);
	othersize = sizeof(otherside);

	start_accepting_client(listencon, listenop, &newconn,
			       (char *)otherside,
			       &othersize, &client_tuple);
}

int
do_listen(service)
char *service;
{
	listencon = create_listening_connection(service);

	if (listencon == NULL) {
		perror("sms");
		fprintf(stderr,"sms: could not create listening connection\n");
		exit (4);
	}

	listenop = create_operation();

	othersize = sizeof(otherside);

	start_accepting_client(listencon, listenop, &newconn,
			       (char *)otherside,
			       &othersize, &client_tuple);

}

/*
 * Welcome to the (finite state) machine (highest level).
 */
do_client(cp)
	client *cp;
{
	if (OP_STATUS(cp->pending_op) == OP_CANCELLED) {
		fprintf(stderr,"dropping connection..\n");
		reset_operation(cp->pending_op);
		cp->state=CL_DEAD;
		cp->action=0;
		/* XXX should delete client from array */
		return;
	}
	switch (cp->action) {
	case CL_ACCEPT:
	case CL_SEND:
		/* Start recieving next request */
		gdb_inop(cp->pending_op, sms_start_recv, &cp->args, NULL);
		gdb_qop(cp->con, CON_INPUT, cp->pending_op);
		cp->action = CL_RECEIVE;
		break;
	case CL_RECEIVE:
		/* Data is here. Process it & start it heading back */
		do_call(cp); /* This may block for a while. */
		gdb_inop(cp->pending_op, sms_start_send, &cp->reply, NULL);
		gdb_qop(cp->con, CON_OUTPUT, cp->pending_op);
		cp->action = CL_SEND;
		break;
	}
}		

do_call(cl)
	client *cl;
{
	fprintf(stderr, "Handling call\n");
	/* for now, just echo the stuff back */
	cl->reply=cl->args;
}

/*
 * Add a new client to the known clients.
 */
clist_append(cp)
	client *cp;
{		
	client **clients_n;
	
	nclients++;
	clients_n = (client **)malloc(nclients * sizeof(client *));
	bcopy((char *)clients, (char *)clients_n, (nclients-1)*sizeof(cp));
	clients_n[nclients-1] = cp;
	free((char *)clients);
	clients = clients_n;
	clients_n = NULL;
}

/*
 * Grr.  This isn't nice.
 */

oplist_append(oplp, op)
	LIST_OF_OPERATIONS *oplp;
	OPERATION op;
{
	int count = (*oplp)->count+1;
	LIST_OF_OPERATIONS newlist = (LIST_OF_OPERATIONS)
		db_alloc(size_of_list_of_operations(count));
	bcopy((char *)(*oplp), (char *)newlist,
	      size_of_list_of_operations((*oplp)->count));
	if (newlist->count != count-1) abort(); /*XXX*/
	newlist->count++;
	newlist->op[count-1] = op;
	db_free((*oplp), size_of_list_of_operations(count-1));
	(*oplp) = newlist;
}

