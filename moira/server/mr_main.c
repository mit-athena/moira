/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_main.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_main.c,v 1.2 1987-06-01 03:34:53 wesommer Exp $
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
 * Revision 1.1  87/05/31  22:06:56  wesommer
 * Initial revision
 * 
 */

#ifndef lint
static char *rcsid_sms_main_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_main.c,v 1.2 1987-06-01 03:34:53 wesommer Exp $";
#endif lint

#include <strings.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <krb.h>
#include "sms_private.h"
#include "sms_server.h"

extern char *malloc();

CONNECTION newconn, listencon;
int nclients;

client **clients, *cur_client;

OPERATION listenop;

LIST_OF_OPERATIONS op_list;

/*
 * What is this??
 */
char otherside[100];
int othersize;
TUPLE client_tuple;				/* client request goes */
						/* here */
void sms_com_err();
void sigshut();

char *whoami;

char buf1[BUFSIZ];

char *takedown;

main(argc, argv)
	int argc;
	char **argv;
{
	int i;
	gdb_init();
	nclients = 0;
	clients = (client **) malloc(0);
	init_sms_err_tbl();
	init_krb_err_tbl();
	whoami = argv[0];
	
	set_com_err_hook(sms_com_err);
	
	signal (SIGTERM, sigshut);
	signal (SIGHUP, sigshut);
	
	do_listen();

	op_list = create_list_of_operations(1, listenop);

	sprintf(buf1, "started (pid %d)", getpid());
	com_err(whoami, 0, buf1);
	takedown = 0;
	
	while(!takedown) {		
		op_select_any(op_list, 0, NULL, NULL, NULL, NULL);
		if (takedown) break;
#ifdef notdef
		fprintf(stderr, "    tick\n");
#endif notdef
		if (OP_DONE(listenop)) {
			new_connection();
		}
		for (i=0; i<nclients; i++) {
			if (OP_DONE(clients[i]->pending_op)) {
				cur_client = clients[i];
				do_client(cur_client);
				cur_client = NULL;
				if (takedown) break;
			}
		}
	}
	com_err(whoami, 0, takedown);
	exit(0);
}

void sigshut()
{
	takedown = "Shut down by signal.";
}

new_connection()
{
	register client *cp = (client *)malloc(sizeof *cp);
	static counter = 0;
	
	/*
	 * Make sure there's been no error
	 */
	if(OP_STATUS(listenop) != OP_COMPLETE ||
	   newconn == NULL) {
		fprintf(stderr,"Error on listening operation\n");
		exit(8); /* XXX */
	}

	/* Add a new client to the array.. */
	clist_append(cp);
	
	/*
	 * Set up the new connection and reply to the client
	 */

	cp->state = CL_STARTING;
	cp->action = CL_ACCEPT;
	cp->con = newconn;
	cp->id = counter++;
	newconn = NULL;
	
	cp->pending_op = create_operation();
	reset_operation(cp->pending_op);
	oplist_append(&op_list, cp->pending_op);
	cur_client = cp;
	
	start_replying_to_client(cp->pending_op, cp->con, GDB_ACCEPTED,
				 "", "");
	{
		int namelen = sizeof(struct sockaddr_in);
		
		getpeername(cp->con->in.fd, &cp->haddr, &namelen);
		
		sprintf(buf1,
			"New connection from %s port %d (now %d client%s)",
			inet_ntoa(cp->haddr.sin_addr),
			ntohs(cp->haddr.sin_port),
			nclients,
			nclients!=1?"s":"");
		com_err(whoami, 0, buf1);
	}
	
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
do_listen()
{
	char *service = index(SMS_GDB_SERV, ':') + 1;
	extern int errno;
	listencon = create_listening_connection(service);

	if (listencon == NULL) {
		com_err(whoami, errno, "while trying to create listening connection");
		exit (4); /* XXX */
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
		sprintf(buf1, "Closed connection (now %d client%s)",
			nclients-1,
			nclients!=2?"s":"");
		com_err(whoami, 0, buf1);
		clist_delete(cp);
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

char *procnames[] = {
	 "noop",
	 "auth",
	 "shutdown",
	 "retrieve",
	 "append",
	 "delete",
	 "edit",
	 };

do_call(cl)
	client *cl;
{
	int pn, i;
	cl->reply.sms_argc = 0;
	if (((pn = cl->args->sms_procno) < 0) ||
	    (pn > SMS_MAX_PROC)) {
		com_err(whoami, 0, "procno out of range");
		cl->reply.sms_status = SMS_UNKNOWN_PROC;
		return;
	}
#ifdef SMS_DBG
	fprintf(stderr, "[#%d] %s(", cl->id, procnames[pn]);
	for (i=0; i < cl->args->sms_argc; i++) {
		if (i) fputc(',', stderr);
		frequote(stderr,cl->args->sms_argv[i]);
	}
	fprintf(stderr, ")\n");
#endif SMS_DBG

	switch(pn) {
	case SMS_NOOP:
		cl->reply.sms_status = 0;
		com_err(whoami, 0, "noop");
		return;
	case SMS_AUTH:
		do_auth(cl);
		return;
#ifdef notdef
	case SMS_RETRIEVE:
		do_retr(cl);
		return;
#endif notdef

	case SMS_SHUTDOWN:
		do_shutdown(cl);
		return;
	}
}

do_shutdown(cl)
	client *cl;
{
	static char buf[BUFSIZ];

	if (cl->args->sms_argc != 1) {
		cl->reply.sms_status = EINVAL;
		return;
	}
		
	if (!cl->clname) {
		sprintf(buf, "Unauthenticated shutdown request rejected",
			cl->clname);
		com_err(whoami, 0, buf);
		cl->reply.sms_status = EPERM;
		return;
	}
	if (!strcmp(cl->clname, "wesommer@ATHENA.MIT.EDU") ||
	    !strcmp(cl->clname, "mike@ATHENA.MIT.EDU")) {
		sprintf(buf, "Shut down by %s", cl->clname);
		com_err(whoami, 0, buf);
		strcpy(buf, "Reason for shutdown: ");
		strcat(buf, cl->args->sms_argv[0]);
		takedown = buf;
	} else {
		sprintf(buf, "Shutdown request by %s rejected",
			cl->clname);
		com_err(whoami, 0, buf);
		cl->reply.sms_status = EPERM;
	}
}
		
		
do_auth(cl)
	client *cl;
{
	KTEXT_ST auth;
	AUTH_DAT ad;
	int status;
	char buf[REALM_SZ+INST_SZ+ANAME_SZ];
	extern int krb_err_base;
	
	auth.length = cl->args->sms_argl[0];
	bcopy(cl->args->sms_argv[0], auth.dat, auth.length);
	auth.mbz = 0;
	
	if ((status = rd_ap_req (&auth, "sms", "sms", cl->haddr.sin_addr,
				 &ad, "")) != KSUCCESS) {
		status += krb_err_base;
		cl->reply.sms_status = status;
		com_err(whoami, status, "(authentication failed)");
		return;
	}
	strcpy(buf, ad.pname);
	if(ad.pinst[0]) {
		strcat(buf, ".");
		strcat(buf, ad.pinst);
	}
	strcat(buf, "@");
	strcat(buf, ad.prealm);
	if (cl->clname) free((char *)cl->clname);
	
	cl->clname = (char *)malloc(strlen(buf)+1);
	strcpy(cl->clname, buf);
	sprintf(buf1, "Authenticated to %s", cl->clname);
	com_err(whoami, 0, buf1);
}

do_retr(cl)
	client *cl;
{
	
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

		
clist_delete(cp)
	client *cp;
{
	int i;
	
	client **clients_n, **scpp, **dcpp; /* source and dest client */
					    /* ptr ptr */
	
/*	cp->state=CL_DEAD;
	cp->action=0;
*/
	int found_it = 0;
	
	clients_n = (client **)malloc((nclients - 1)* sizeof(client *));
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

	reset_operation(cp->pending_op);
	delete_operation(cp->pending_op);
	free(cp);
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

#include <ctype.h>

frequote(f, cp)
	FILE *f;
	register char *cp;
{
	register char c;
	putc('"', f);
	for( ; c= *cp; *cp++){
		if (c == '\\' || c == '"') putc('\\', f);
		if (isprint(c)) putc(c, f);
		else fprintf(f, "\\%03o", c);
	}
	putc('"', f);
}

void sms_com_err(whoami, code, message)
	char *whoami;
	int code;
	char *message;
{
	extern char *error_message();
	
	struct iovec strings[7];
	char buf[32];
	if (cur_client)
		sprintf(buf, "[#%d]", cur_client->id);
	else buf[0]='\0';
	
	strings[1].iov_base = buf;
	strings[1].iov_len = strlen(buf);
	
	strings[0].iov_base = whoami;
	if (whoami) {
		strings[0].iov_len = strlen(whoami);
		strings[2].iov_base = ": ";
		strings[2].iov_len = 2;
	} else {
		strings[0].iov_len = 0;
		strings[2].iov_base = " ";
		strings[2].iov_len = 1;
	}
	if (code) {
		register char *errmsg = error_message(code);
		strings[3].iov_base = errmsg;
		strings[3].iov_len = strlen(errmsg);
		strings[4].iov_base = " ";
		strings[4].iov_len = 1;
	} else {
		strings[3].iov_len = 0;
		strings[4].iov_len = 0;
	}
	strings[5].iov_base = message;
	strings[5].iov_len = strlen(message);
	strings[6].iov_base = "\n";
	strings[6].iov_len = 1;
	(void) writev(2, strings, 7);
}
