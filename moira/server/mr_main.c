/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_main.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_main.c,v 1.12 1987-08-04 01:50:00 wesommer Exp $
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
 * Revision 1.11  87/07/29  16:04:54  wesommer
 * Add keepalive feature.
 * 
 * Revision 1.10  87/06/30  20:02:26  wesommer
 * Added returned tuple chain to client structure.
 * Added local realm global variable.
 * 
 * Revision 1.9  87/06/21  16:39:54  wesommer
 * Performance work, rearrangement of include files.
 * 
 * Revision 1.8  87/06/09  18:44:45  wesommer
 * modified error handling.
 * 
 * Revision 1.7  87/06/08  02:44:44  wesommer
 * Minor lint fix.
 * 
 * Revision 1.6  87/06/03  17:41:00  wesommer
 * Added startup support.
 * 
 * Revision 1.5  87/06/03  16:07:17  wesommer
 * Fixes for lint.
 * 
 * Revision 1.4  87/06/02  20:05:11  wesommer
 * Bug fixes on memory allocation.
 * 
 * Revision 1.3  87/06/01  04:34:27  wesommer
 * Changed returned error code.
 * 
 * Revision 1.2  87/06/01  03:34:53  wesommer
 * Added shutdown, logging.
 * 
 * Revision 1.1  87/05/31  22:06:56  wesommer
 * Initial revision
 * 
 */

static char *rcsid_sms_main_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_main.c,v 1.12 1987-08-04 01:50:00 wesommer Exp $";

#include <strings.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include "sms_server.h"

extern CONNECTION newconn, listencon;

extern int nclients;
extern client **clients, *cur_client;

extern OPERATION listenop;
extern LIST_OF_OPERATIONS op_list;

extern struct sockaddr_in client_addr;
extern int client_addrlen;
extern TUPLE client_tuple;

extern char *whoami;
extern char buf1[BUFSIZ];
extern char *takedown;
extern int errno;


extern char *malloc();
extern char *inet_ntoa();
extern void sms_com_err();
extern void do_client();

extern int sigshut();
void clist_append();
void oplist_append();
extern u_short ntohs();

extern time_t now;

/*
 * Main SMS server loop.
 *
 * Initialize the world, then start accepting connections and
 * making progress on current connections.
 */

/*ARGSUSED*/
int
main(argc, argv)
	int argc;
	char **argv;
{
	int status;
	time_t tardy;
	
	whoami = argv[0];
	/*
	 * Error handler init.
	 */
	init_sms_err_tbl();
	init_krb_err_tbl();
	set_com_err_hook(sms_com_err);
	setlinebuf(stderr);
	
	if (argc != 1) {
		com_err(whoami, 0, "Usage: smsd");
		exit(1);
	}		
	
	/*
	 * GDB initialization.
	 */
	if(gdb_init() != 0) {
		com_err(whoami, 0, "GDB initialization failed.");
		exit(1);
	}
	gdb_debug(0); /* this can be patched, if necessary, to enable */
		      /* GDB level debugging .. */
	krb_realm = malloc(REALM_SZ);
	get_krbrlm(krb_realm, 1);
	
	/*
	 * Database initialization.
	 */

	if ((status = sms_open_database()) != 0) {
		com_err(whoami, status, "when trying to open database.");
		exit(1);
	}
	
	/*
	 * Set up client array handler.
	 */
	nclients = 0;
	clients = (client **) malloc(0);
	
	/*
	 * Signal handlers
	 *	There should probably be a few more of these.
	 */
	
	if ((((int)signal (SIGTERM, sigshut)) < 0) ||
	    (((int)signal (SIGHUP, sigshut)) < 0)) {
		com_err(whoami, errno, "Unable to establish signal handler.");
		exit(1);
	}
	
	/*
	 * Establish template connection.
	 */
	if ((status = do_listen()) != 0) {
		com_err(whoami, status,
			"while trying to create listening connection");
		exit(1);
	}
	
	op_list = create_list_of_operations(1, listenop);
	
	com_err(whoami, 0, "started (pid %d)", getpid());
	com_err(whoami, 0, rcsid_sms_main_c);

	/*
	 * Run until shut down.
	 */
	while(!takedown) {		
		register int i;
		/*
		 * Block until something happens.
		 */
#ifdef notdef
		com_err(whoami, 0, "tick");
#endif notdef
		errno = 0;
		status = op_select_any(op_list, 0,
				       (fd_set *)NULL, (fd_set *)NULL,
				       (fd_set *)NULL, (struct timeval *)NULL);

		if (status == -1) {
			com_err(whoami, errno, "error from op_select");
			continue;
		} else if (status != -2) {
			com_err(whoami, 0, "wrong return from op_select_any");
			continue;
		}
		if (takedown) break;
		time(&now);
#ifdef notdef
		fprintf(stderr, "    tick\n");
#endif notdef
		/*
		 * Handle any new connections; this comes first so
		 * errno isn't tromped on.
		 */
		if (OP_DONE(listenop)) {
			if (OP_STATUS(listenop) == OP_CANCELLED) {
				if (errno == EWOULDBLOCK) {
					do_reset_listen();
				} else {
					com_err(whoami, errno,
						"error on listen");
					exit(1);
				}
			} else if ((status = new_connection()) != 0) {
				com_err(whoami, errno,
					"Error on listening operation.");
				/*
				 * Sleep here to prevent hosing?
				 */
			}
		}
		/*
		 * Handle any existing connections.
		 */
		tardy = now - 30*60;
		
		for (i=0; i<nclients; i++) {
			cur_client = clients[i];
			if (OP_DONE(clients[i]->pending_op)) {
				cur_client->last_time_used = now;
				do_client(cur_client);
			} else if (clients[i]->last_time_used < tardy) {
				com_err(whoami, 0, "Shutting down connection due to inactivity");
				shutdown(cur_client->con->in.fd, 0);
			}
			cur_client = NULL;
			if (takedown) break;
		}
	}
	com_err(whoami, 0, "%s", takedown);
	sms_close_database();
	return 0;
}

/*
 * Set up the template connection and queue the first accept.
 */

int
do_listen()
{
	char *service = index(SMS_GDB_SERV, ':') + 1;

	listencon = create_listening_connection(service);

	if (listencon == NULL)
		return errno;

	listenop = create_operation();
	client_addrlen = sizeof(client_addr);

	start_accepting_client(listencon, listenop, &newconn,
			       (char *)&client_addr,
			       &client_addrlen, &client_tuple);
	return 0;
}


do_reset_listen()
{
	client_addrlen = sizeof(client_addr);
	start_accepting_client(listencon, listenop, &newconn,
			       (char *)&client_addr,
			       &client_addrlen, &client_tuple);
}

/*
 * This routine is called when a new connection comes in.
 *
 * It sets up a new client and adds it to the list of currently active clients.
 */
int
new_connection()
{
	register client *cp;
	static counter = 0;
	
	/*
	 * Make sure there's been no error
	 */
	if(OP_STATUS(listenop) != OP_COMPLETE) {
		return errno;
	}
	
	if (newconn == NULL) {
		return SMS_NOT_CONNECTED;
	}

	/*
	 * Set up the new connection and reply to the client
	 */
	cp = (client *)malloc(sizeof *cp);
	cp->state = CL_STARTING;
	cp->action = CL_ACCEPT;
	cp->con = newconn;
	cp->id = counter++;
	cp->args = NULL;
	cp->clname = NULL;
	cp->reply.sms_argv = NULL;
	cp->first = NULL;
	cp->last = NULL;
	cp->last_time_used = now;
	newconn = NULL;
	
	cp->pending_op = create_operation();
	reset_operation(cp->pending_op);
	oplist_append(&op_list, cp->pending_op);
	cur_client = cp;
	
	/*
	 * Add a new client to the array..
	 */
	clist_append(cp);
	
	/*
	 * Let him know we heard him.
	 */
	start_replying_to_client(cp->pending_op, cp->con, GDB_ACCEPTED,
				 "", "");

	cp->haddr = client_addr;
	
	/*
	 * Log new connection.
	 */
	
	com_err(whoami, 0, "New connection from %s port %d (now %d client%s)",
		inet_ntoa(cp->haddr.sin_addr),
		(int)ntohs(cp->haddr.sin_port),
		nclients,
		nclients!=1?"s":"");
	
	/*
	 * Get ready to accept the next connection.
	 */
	reset_operation(listenop);
	client_addrlen = sizeof(client_addr);

	start_accepting_client(listencon, listenop, &newconn,
			       (char *)&client_addr,
			       &client_addrlen, &client_tuple);
	return 0;
}

/*
 * Add a new client to the known clients.
 */
void
clist_append(cp)
	client *cp;
{		
	client **clients_n;
	
	nclients++;
	clients_n = (client **)malloc
		((unsigned)(nclients * sizeof(client *)));
	bcopy((char *)clients, (char *)clients_n, (nclients-1)*sizeof(cp));
	clients_n[nclients-1] = cp;
	free((char *)clients);
	clients = clients_n;
	clients_n = NULL;
}

		
void
clist_delete(cp)
	client *cp;
{
	client **clients_n, **scpp, **dcpp; /* source and dest client */
					    /* ptr ptr */
	
	int found_it = 0;
	
	clients_n = (client **)malloc
		((unsigned)((nclients - 1)* sizeof(client *)));
	for (scpp = clients, dcpp = clients_n; scpp < clients+nclients; ) {
		if (*scpp != cp) {
			*dcpp++ = *scpp++;
		} else {
			scpp++;
			if (found_it) abort();
			found_it = 1;
		}			
	}
	--nclients;	
	free((char *)clients);
	clients = clients_n;
	clients_n = NULL;
	oplist_delete(op_list, cp->pending_op);
	reset_operation(cp->pending_op);
	delete_operation(cp->pending_op);
	sever_connection(cp->con);
	free((char *)cp);
}

/*
 * Add a new operation to a list of operations.
 *
 * This should be rewritten to use realloc instead, since in most
 * cases it won't have to copy the array.
 */

void
oplist_append(oplp, op)
	LIST_OF_OPERATIONS *oplp;
	OPERATION op;
{
	int count = (*oplp)->count+1;
	LIST_OF_OPERATIONS newlist = (LIST_OF_OPERATIONS)
		db_alloc(size_of_list_of_operations(count));
	bcopy((char *)(*oplp), (char *)newlist,
	      size_of_list_of_operations((*oplp)->count));
	newlist->count++;
	newlist->op[count-1] = op;
	db_free((char *)(*oplp), size_of_list_of_operations(count-1));
	(*oplp) = newlist;
}


oplist_delete(oplp, op)
	LIST_OF_OPERATIONS oplp;
	register OPERATION op;
{
	register OPERATION *s;
	register int c;
	
	for (s = oplp->op, c=oplp->count; c; --c, ++s) {
		if (*s == op) {
			while (c > 0) {
				*s = *(s+1);
				++s;
				--c;
			}
			oplp->count--;
			return;
		}
	}
	abort();
}
