/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/gdb_fserv.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/gdb_fserv.c,v 1.1 1987-08-02 22:11:03 wesommer Exp $
 */

#ifndef lint
static char *rcsid_gdb_fserv_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/gdb_fserv.c,v 1.1 1987-08-02 22:11:03 wesommer Exp $";
#endif	lint





















/************************************************************************/
/*	
/*			   gdb_fserv.c
/*	
/*	      GDB - Routines to implement forking servers.
/*	
/*	Author: Noah Mendelsohn
/*	Copyright: 1986 MIT Project Athena 
/*	
/************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include "gdb.h"
#include <sys/resource.h>

/************************************************************************/
/*	
/*			create_forking_server (create_forking_server)
/*	
/*	Called by an application to turn itself into a forking model
/*	server.  Returns from this routine occur only in the forked
/*	children.  The parent lives in this routine forever, waiting
/*	for incoming connection requests and doing the appropriate
/*	forking.
/*	
/*	Children are expected to do their own cleanup, but this routine
/*	does do the work of reaping the resulting zombie processes.
/*	
/*	ARGUMENTS:
/*	----------
/*	
/*		service-id	identifies the port to be used for
/*				listening.  Same rules as for
/*				create_listening_connection.
/*	
/*		validate-rtn	pointer to a function to be called to
/*				validate the incoming client.  Should
/*				return TRUE if client is acceptable,
/*				else false.  If this is NULL, all clients
/*				are accepted.
/*	
/*	GLOBAL VARIABLES
/*	----------------
/*	
/*	Children created by this routine inherit the global variables
/*	gdb_sockaddr_of_client, which is of type sockaddr_in and 
/*	gdb_socklen, which is the returned length of the sockaddr.
/*	These are the Berkeley identifiers of the clients as accepted.
/*	Use of this interface is non-portable to other than Berkeley 
/*	systems.
/*	
/*	The client's request tuple may be found in gdb_client_tuple.
/*	
/************************************************************************/


CONNECTION
create_forking_server(service, validate)
char *service;
int (*validate)();
{

	CONNECTION incoming;			/* listen for incoming */
						/* children here */
	CONNECTION client = NULL;		/* connection to client */
						/* is created here */
	OPERATION listenop;			/* used to asynchronously */
						/* listen for a child */
						/* connection request */
	GDB_INIT_CHECK

	/*----------------------------------------------------------*/
	/*	
	/*	Set up parent execution environment
	/*	
	/*----------------------------------------------------------*/

	g_do_signals();				/* set up signal handling */

	incoming = create_listening_connection(service);
	if (incoming == NULL)
		GDB_GIVEUP("create_forking_server: can't create listening connection")

	/*----------------------------------------------------------*/
	/*	
	/*	Repeatedly listen for incoming
	/*	
	/*----------------------------------------------------------*/

	listenop = create_operation();
	while (TRUE) {
		gdb_socklen = sizeof(gdb_sockaddr_of_client);
		client = NULL;
		(void) start_accepting_client(incoming, listenop, &client, 
				       gdb_sockaddr_of_client, &gdb_socklen,
				       &gdb_client_tuple);
		if(complete_operation(listenop) != OP_COMPLETE ||
		   client == NULL) 
			GDB_GIVEUP("create_forking_server: failed to accept client")
		

                /*
                 * Call the validate routine, if it fails, 
                 * refuse the client.
                 */
		if (validate != NULL && !(*validate)()) {
			reset_operation(listenop);
			start_replying_to_client(listenop, client, 
						 GDB_REFUSED, "","");
			(void) complete_operation(listenop); 
			reset_operation(listenop);
			(void) sever_connection(client);
			continue;			
		}
                /*
                 * Create  the child for this client
                 */
		if ((gdb_Debug & GDB_NOFORK) || fork() == 0) {
                       /*
                        * This is the child, or we're in noforking
                        * debug mode.
                        */
			reset_operation(listenop);
			start_replying_to_client(listenop, client, 
						 GDB_ACCEPTED, "","");
			if (complete_operation(listenop) != OP_COMPLETE) 
				exit(8);
			return client;		/* return to application */
						/* program */
		}
               /*
                * Still in the parent
                */
		(void) sever_connection(client);
		reset_operation(listenop);
	}
}

/************************************************************************/
/*	
/*				gdb_reaper
/*	
/*	Called on SIGCHILD to reap all dead children.
/*	
/************************************************************************/
int
gdb_reaper()
{
	union wait status;
	extern char *sys_siglist[];
       
	while (wait3(&status, WNOHANG, (struct rusage *)0) >0) {
#ifdef notdef
		if (WIFEXITED(status)) {
			if (status.w_retcode)
				printf("exited with code %d\n",
					status.w_retcode);
		}
		if (WIFSIGNALED(status)) {
			printf("exited on %s signal%s\n",
			       sys_siglist[status.w_termsig],
			       (status.w_coredump?"; core dumped":0));
		}
#endif notdef
	}
}

/************************************************************************/
/*	
/*				g_do_signals
/*	
/*	Set up signal handling for a forking server.
/*	
/************************************************************************/

int
g_do_signals()
{
	(void) signal(SIGCHLD, gdb_reaper);
}
