/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/gdb_conn.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/gdb_conn.c,v 1.3 1991-08-21 10:43:53 mar Exp $
 */

#ifndef lint
static char *rcsid_gdb_conn_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/gdb_conn.c,v 1.3 1991-08-21 10:43:53 mar Exp $";
#endif	lint
























/************************************************************************/
/*	
/*			   gdb_conn.c
/*	
/*	      GDB - Connection Management Services
/*	
/*	Author: Noah Mendelsohn
/*	Copyright: 1986 MIT Project Athena 
/*		For copying and distribution information, please see
/*	  	the file <mit-copyright.h>.
/*	
/*	Routines used in the creation and maintenance of CONNECTIONS.
/*	Note: these are closely related to the services provided
/*	by gdb_trans.c and gdb_trans2.c.
/*	
/*	
/************************************************************************/

#include <mit-copyright.h>
#include <stdio.h>
#include <strings.h>
#include "gdb.h"
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

extern int errno;
#ifdef vax
extern u_short htons();			/* ?? All versions?  */
#endif vax

CONNECTION gdb_allocate_connection();

/************************************************************************/
/*	
/*			start_peer_connection (start_peer_connection)
/*	
/*	Starts a connection to another process which itself will be 
/*	issuing a start_peer_connection to us.  Current implementation
/*	builds at most one stream, with the risk of a hang if 
/*	the attempts to connect cross in the night.  This is a bug,
/*	but this level of support is acceptable for casual debugging
/*	of applications, and perhaps for some production use in
/*	controlled settings.  I think the only other way to do it 
/*	is to risk building two streams in parallel, possibly tearing
/*	one down when the duplication is discovered.  Seems complicated
/*	and messy.
/*	
/************************************************************************/

CONNECTION
start_peer_connection(id)
char *id;					/* null terminated string */
{
	register CONNECTION con;		/* the connection we're */
						/* creating */

	GDB_INIT_CHECK

       /*
        * Try to allocate a connection and fill it in with null values.
        */

	con = g_make_con();

       /*
        * In this implementation, we use a single fd for both inbound and
        * outbound traffic.  Try to connect to other side.  If that 
        * doesn't work, wait to accept a connection from the other side.
        * Current implementation of this is synchronous--may be a problem?
        * Also note timing window bug in the following.  If the two peers
        * are started at just about the same time, the race may not be handled
        * propoerly.  If the connections come up, then verify the level of
        * protocol being observed on the connections.  If incompatible,
        * then turn off the connection.
        */

 	if(!g_try_connecting(con,id)) {
		g_try_accepting(con,id);
		if(con->status == CON_STARTING)
			g_ver_iprotocol(con);
	} else
		if(con->status == CON_STARTING)
			g_ver_oprotocol(con);


	if (con->status == CON_UP) {
               /*
                * We've successfully started the connection, now mark
                * it for non-blocking I/O.  Also, update the high water
                * mark of fd's controlled by our system.
                */
		int nb = 1;
		if(ioctl(con->in.fd, FIONBIO, (char *)&nb)== (-1)) { 
			g_stop_with_errno(con);
			return con;
		}
		if (con->in.fd +1 > gdb_mfd) 
			gdb_mfd = con->in.fd + 1;
               /*
                * Allocate a buffer, if necessary, and reset buffer pointers
                * so first request will result in a long read into the buffer
                */
		g_allocate_connection_buffers(con);

		return con;
	} else
		return NULL;
}

/************************************************************************/
/*	
/*				g_make_con
/*	
/*	Internal routine to allocate a new connection structure and
/*	initialize all its fields to logical null values.
/*	
/************************************************************************/

CONNECTION
g_make_con()
{
	register CONNECTION con;

       /*
        * Try to allocate a connection, fatal error if none available
        */
	con = gdb_allocate_connection();
	if (con == NULL)
		GDB_GIVEUP("start_peer_connection: Tried to allocate too many connections") /* <==RECOVERABLE */
	
       /*
        * Give the fields their initial values
        */
        g_null_con(con);

	return con;

}

/************************************************************************/
/*	
/*				g_null_con
/*	
/*	Sets a connection descriptor to have all null values in 
/*	its fields.  This routine does NOT do any of the cleanup
/*	which is necessary after the connection has really been used.
/*	
/************************************************************************/

int
g_null_con(con)
CONNECTION con;
{
       /*
        * Initialize the connection control data structure.
        */
	con->id = GDB_CON_ID;
        con->status = CON_STARTING;
	con->oob_fcn = NULL;			/* out of band signalling */
						/* is not currently */
						/* implemented */
	con->errno = 0;				/* system errno gets */
						/* copied here iff it */
						/* causes this con to die */
       /*
        * Initialize input half connection to null state before trying
        * to bring it up.
        */
	con->in.status = OP_NOT_STARTED;
	con->in.fd = -1;
	con->in.oob_fd = -1;
	con->in.op_q_first = (struct oper_data *)&con->in;
	con->in.op_q_last = (struct oper_data *)&con->in;
	con->in.next_byte = NULL;
	con->in.remaining = 0;
	con->in.flags = 0;

       /*
        * Initialize output half connection to null state before trying
        * to bring it up.
        */
	con->out.status = OP_NOT_STARTED;
	con->out.fd = -1;
	con->out.oob_fd = -1;
	con->out.op_q_first = (struct oper_data *)&con->out;
	con->out.op_q_last = (struct oper_data *)&con->out;
	con->out.next_byte = NULL;
	con->out.remaining = 0;
	con->out.flags = 0;

	return;

}


/************************************************************************/
/*	
/*			gdb_allocate_connection
/*	
/*	Return an unused entry in the connection array.  Unused entries
/*	are recognized by being marked as CON_STOPPED.
/*	
/*	Note that gdb_mcons is the number of descriptors which have 
/*	ever been used (i.e. a high water mark), so status fields
/*	are invalid above that.
/*	
/************************************************************************/

CONNECTION
gdb_allocate_connection()
{
	register int i;				/* index of next one */
						/* to check */

       /*
        * First look for one below the high water mark
        */
	for(i=0; i<gdb_mcons; i++) {
		if (gdb_cons[i].status == CON_STOPPED)
			return &gdb_cons[i];
	}

       /*
        * Allocate one which has never been used, if possible
        */

	if (i>=GDB_MAX_CONNECTIONS)
		GDB_GIVEUP("gdb: tried to allocate too many simulataneous connections.\n, See GDB_MAX_CONNECTIONS in gdb.h.") /* <==RECOVERABLE */

	gdb_mcons++;				/* bump the high water mark */
	gdb_cons[i].status = CON_STOPPED;	/* initialize status of the */
						/* new connection */
	return &gdb_cons[i];			/* return new highest con */
						/* ever used*/       
}

/************************************************************************/
/*	
/*			g_try_connecting
/*	
/*	Try to start a connection to the designated site, filling 
/*	in the appropriate information in the connection descriptor
/*	if successful.  Return TRUE if connection succeeded or if
/*	error was fatal enough that we shouldn't try accepting.  Returns
/*	FALSE if we should try accepting.
/*	
/************************************************************************/

int

g_try_connecting(con,id)
CONNECTION con;
char *id;
{
	int peer;				/* socket for talking to
						   peer */
	struct sockaddr_in target;		/* build the peer address */
						/* here */
	struct hostent *peer_host; 		/* host where peer is */

	/*----------------------------------------------------------*/
	/*	
	/*	Make sure connection is marked stopped until we 
	/*	get it going.
	/*	
	/*----------------------------------------------------------*/

	con->status = CON_STOPPED;

	/*----------------------------------------------------------*/
	/*	
	/*	Find out host where peer is, and validate it.  Take
	/*	care of port at the same time.
	/*	
	/*----------------------------------------------------------*/

	bzero((char *)&target, sizeof(target));
	g_parse_target(id, &peer_host, &target.sin_port);
	if (peer_host == NULL) {
		fprintf(gdb_log,"gdb: g_try_connecting...  '%s' is not a valid host:server\n",
			id);
		return TRUE;			/* so we won't try accepting */
	}
	
	/*----------------------------------------------------------*/
	/*	
	/*	Create a socket
	/*	
	/*----------------------------------------------------------*/

	peer = socket(AF_INET, SOCK_STREAM, 0);
	if (peer < 0) {
		g_stop_with_errno(con);
		return TRUE;			/* fatal error */
	}

	/*----------------------------------------------------------*/
	/*	
	/*	Get information and bind socket using well known 
	/*	port (BUG: this restricts us to one pair of peers
	/*	per host pair, as well as being bad practice on 
	/*	the network.  It will do for debugging.
	/*	
	/*----------------------------------------------------------*/


	bcopy(peer_host->h_addr, (char *)&target.sin_addr, peer_host->h_length);
	target.sin_family = peer_host->h_addrtype;

	/*----------------------------------------------------------*/
	/*	
	/*	Make the connection
	/*	
	/*----------------------------------------------------------*/

	if(connect(peer, (struct sockaddr *)&target, sizeof(target)) < 0) {
		if (errno == ECONNREFUSED)
		  	return FALSE;		/* other side not yet */
						/* up, but no other fatal */
						/* errors*/
		else {
			gdb_perror("gdb: unexpected error connecting");
			g_stop_with_errno(con);
			return TRUE;
		}
	}

	/*----------------------------------------------------------*/
	/*	
	/*	The connection has been made, fill in the connection
	/*	control data structure.
	/*	
	/*----------------------------------------------------------*/

	con->in.fd = peer;
	con->out.fd = peer;
	con->status = CON_STARTING;

	return TRUE;

}

/************************************************************************/
/*	
/*			g_parse_target
/*	
/*	For a given server or peer i.d., figure out the host and the
/*	port.  Arguments are:  
/*	
/*		string i.d. of the server, which is	
/*		in one of two forms:
/*	
/*			host:servicename (where service name must not begin
/*				  with #)
/*	
/*			host:#portnumber  (where portnumber is the actual
/*				   number of the port to be used)
/*	
/*			(actually, a 3rd form, with no port number supplied,
/*			will use a default GDB_PORT, but this is unsafe
/*			and it will be disabled in production versions
/*			of the gdb system.)
/*	
/*		**hostent: returned to indicate host to be used.  Null
/*			if host could not be found
/*	
/*		*port   pointer to an integer where the port number will
/*			be put.  We return the port number in network
/*			byte order.
/*	
/************************************************************************/

int
g_parse_target(id, host, port)
char *id;
struct hostent **host;
u_short *port;
{
	char buffer[256];			/* longest host name */
	register char *ip, *bp;			/* for copying name */
	struct servent *serv;			/* returned from */
						/* get service by name */

	/*----------------------------------------------------------*/
	/*	
	/*	copy the host name part only to local buffer
	/*	
	/*----------------------------------------------------------*/

	ip = id;
	bp = buffer;

	while (*ip != '\0' && *ip != ':')
		*bp++ = *ip++;
	*bp = '\0';

	/*----------------------------------------------------------*/
	/*	
	/*	Look up the host name, return if bad.
	/*	
	/*----------------------------------------------------------*/

	*host = gethostbyname(buffer);

	if (*host == NULL)
		return;

	/*----------------------------------------------------------*/
	/*	
	/*	Set up the port address
	/*	
	/*----------------------------------------------------------*/

	if (*ip++ != ':') {
		*port = GDB_PORT;
		return;
	}
	
	if (*ip == '\0') {
		*host = NULL;
		return;
	}

	if (*ip == '#') {
               /*
                * port number supplied explictly
                */
		ip++;
		if (*ip < '0' || *ip>'9') {
			*host = NULL;
			return;
		}
		*port = htons((u_short)atoi(ip));
	} else {
               /*
                * service identified by name
                */
		serv = getservbyname(ip, "tcp");
		if (serv == NULL) {
			*host = NULL;
			return;
		}
	        *port = serv->s_port;
	}
}

/************************************************************************/
/*	
/*			g_try_accepting
/*	
/*	Try to accept a connection to the designated site, filling 
/*	in the appropriate information in the connection descriptor
/*	if successful.
/*	
/************************************************************************/

int
g_try_accepting(con,id)
CONNECTION con;
char *id;
{
	int slisten;				/* socket on which
						   we listen for connections */

	int peer;				/* socket for talking to
						   peer */
	int fromlen;
	struct sockaddr_in self, from;
	int retries = GDB_BIND_RETRY_COUNT;
	int onoff = 1;				/* used as argument to */
						/* setsockopt */

	struct hostent *peer_host; 		/* host where peer is */

	/*----------------------------------------------------------*/
	/*	
	/*	Make sure connection is marked stopped until we 
	/*	get it going.
	/*	
	/*----------------------------------------------------------*/

	con->status = CON_STOPPED;

	/*----------------------------------------------------------*/
	/*	
	/*	Create a socket on which to listen.  Tell it that
	/*	it's OK to re-use the port address, which may still
	/*	appear busy if connections are taking awhile to go
	/*	away.
	/*	
	/*----------------------------------------------------------*/

	slisten = socket(AF_INET, SOCK_STREAM, 0);
	if (slisten < 0) {
		gdb_perror("g_try_accepting: error creating listen socket");
		g_stop_with_errno(con);
	}
	/* Try 4.2 method */
	if(setsockopt(slisten, SOL_SOCKET, SO_REUSEADDR, (char *)0, 0)<0)
	/* that didn't work, try 4.3 */
		if(setsockopt(slisten, SOL_SOCKET, SO_REUSEADDR,
			      (char *)&onoff, sizeof(int)) <0)
			GDB_GIVEUP("g_try_accepting: could not set SO_REUSEADDR");
	
	/*----------------------------------------------------------*/
	/*	
	/*	Find out host where peer is, and validate it.  Take
	/*	care of port at the same time.  This is redundant
	/*	given that g_try_connecting is always called first.
	/*	
	/*----------------------------------------------------------*/

	bzero((char *)&self, sizeof(self));
	g_parse_target(id, &peer_host, &self.sin_port);
	if (peer_host == NULL) {
		GDB_GIVEUP("gdb_try_accepting: bad port not caught by try connecting")
	}
	
	/*----------------------------------------------------------*/
	/*	
	/*	Bind the socket to ourselves, using the well known
	/*	port (See bug note in g_try_connecting.
	/*	
	/*	This code should really go in initialization, I think.
	/*	
	/*----------------------------------------------------------*/

	while (bind(slisten,(struct sockaddr *)&self,sizeof(self)) < 0) {
		if (errno == EADDRINUSE && retries--) {
			fprintf(gdb_log,"gdb: port address in use, will retry %d more time(s)\n",retries+1);
		        sleep(GDB_BIND_RETRY_INTERVAL);
			continue;
		} else {
			gdb_perror("gdb: error binding listen socket");
			g_stop_with_errno(con);
			(void) close(slisten);			
			return;
		}
	}

	/*----------------------------------------------------------*/
	/*	
	/*	Listen for connections.
	/*	
	/*----------------------------------------------------------*/

	(void) listen (slisten, 5);		/* does not block, just */
						/* sets the maximum backlog */
						/* of pending non-accepted */
						/* cons.*/
	fromlen = sizeof(from);
	peer = accept(slisten, &from, &fromlen);
	if (peer < 0) {
		g_stop_with_errno(con);
		gdb_perror("gdb_try_accepting: error accepting connection");
		(void) close(slisten);
		return;
	}

	(void) close (slisten);			/* we're not using the */
						/* listening socket */
						/* anymore, only the */
						/* connection to the peer */

	/*----------------------------------------------------------*/
	/*	
	/*	The connection has been made, fill in the connection
	/*	control data structure.
	/*	
	/*----------------------------------------------------------*/

	con->in.fd = peer;
	con->out.fd = peer;
	con->status = CON_STARTING;
}

/************************************************************************/
/*	
/*			g_ver_oprotocol
/*	
/*	Called when an outbound connection is started to verify 
/*	the version of the protocol being observed.
/*	
/************************************************************************/

int
g_ver_oprotocol(con)
CONNECTION con;
{
#ifdef VERIFY_PROTOCOL
	char ver = GDB_PROTOCOL_VERSION;
	char theirs;
	int len;

        int onoff = 0;				/* for ioctl to turn off */

       /*
	* Because the connection was accepted on a non-blocking 
	* listening socket, the connection itself may be non-blocking.
	*  We can't tolerate that here.  It will be reset later.
	*/
	if (ioctl(con->in.fd, FIONBIO, (char *)&onoff) < 0) { 
		g_stop_with_errno(con);
		gdb_perror("Can't turn off FIONBIO in g_ver_iprotocol");
		return;
	}

	while (write(con->out.fd, &ver, 1) < 0) {
		g_stop_with_errno(con);
		return;
	}

	do {
		len = read(con->in.fd, &theirs, 1);
		if (len == (-1)) {
			g_stop_with_errno(con);
			return;
		}
	} while (len !=1);

	if (theirs == ver)
		con->status = CON_UP;
	else
		con->status = CON_STOPPED;
#else !VERIFY_PROTOCOL
	con->status = CON_UP;
#endif !VERIFY_PROTOCOL
}

/************************************************************************/
/*	
/*			g_ver_iprotocol
/*	
/*	Called when an inbound connection is started to verify 
/*	the version of the protocol being observed.
/*	
/************************************************************************/

int
g_ver_iprotocol(con)
CONNECTION con;
{
#ifdef VERIFY_PROTOCOL
	char ver = GDB_PROTOCOL_VERSION;
	char theirs;
	int len;
	int old_nbio;
        int onoff = 0;				/* for ioctl to turn off */

       /*
	* Because the connection was accepted on a non-blocking 
	* listening socket, the connection itself may be non-blocking.
	*  We can't tolerate that here.  It will be reset later.
	*/
	if (ioctl(con->in.fd, FIONBIO, (char *)&onoff) < 0) { 
		g_stop_with_errno(con);
		gdb_perror("Can't turn off FIONBIO in g_ver_iprotocol");
		return;
	}

	do {
		len = read(con->in.fd, &theirs, 1);
		if (len == (-1)) {
			g_stop_with_errno(con);
			return;
		}
        } while (len !=1) ;

	if (theirs == ver)
		con->status = CON_UP;
	else
		con->status = CON_STOPPED;

	while (write(con->out.fd, &ver, 1) < 0) {
		g_stop_with_errno(con);
		return;
	}
#else   !VERIFY_PROTOCOL
	con->status = CON_UP;
#endif
}


/************************************************************************/
/*	
/*			sever_connection (sever_connection)
/*	
/*	Unconditionally, but cleanly, terminates a connection.  All
/*	pending operations on the connection are cancelled, and the
/*	file descriptor for the connection is closed.  This routine
/*	should be called directly from applications wishing to shut
/*	down a connection.  No transmissions are attempted
/*	by this routine.  Returns NULL, in the hope that applications
/*	will assign this to their old CONNECTION variable.
/*	
/************************************************************************/

CONNECTION
sever_connection(con)
CONNECTION con;
{
	if (con == NULL)
		return NULL;
	GDB_CHECK_CON(con, "sever_connection")
	if (con->status == CON_UP || con->status == CON_STARTING)
		g_stop_connection(con);
	if (con->status != CON_STOPPED)
		gdb_de_allocate_connection(con);

	return NULL;
}

/************************************************************************/
/*	
/*			g_stop_with_errno
/*	
/*	This connection is stopping because of a problem on a syscall.
/*	We record the errno in the connection descriptor for inspection
/*	by the application, then stop the connection.
/*	
/************************************************************************/


int
g_stop_with_errno(con)
CONNECTION con;
{
	con->errno = errno;
	g_stop_connection(con);
	
}

/************************************************************************/
/*	
/*			g_stop_connection
/*	
/*	Unconditionally, but cleanly, terminates a connection.  All
/*	pending operations on the connection are cancelled, and the
/*	file descriptor for the connection is closed.  This routine is 
/*	for internal use.  Applications call sever_connection, which 
/*	also de_allocates the descriptor.  No transmissions are attempted
/*	by this routine.
/*	
/************************************************************************/

int
g_stop_connection(con)
CONNECTION con;
{
       /*
        * Shutdown activity on the two half connections.
        */
	g_cleanup_half_connection(&(con->in));
	g_cleanup_half_connection(&(con->out));

       /*
        * Remove the file descriptor from the select bit maps
        */
	if (!(con->in.flags & HCON_UNUSED) && con->in.fd >= 0)
		FD_CLR(con->in.fd,  &gdb_crfds);
	if (!(con->out.flags & HCON_UNUSED) && con->out.fd >= 0)
		FD_CLR(con->out.fd, &gdb_cwfds);
       /*
        * Close the file descriptor.  Note, this presumes that in fact
        * 1) in is never the unused half and
        * 2) when the connection is bi-directional, in and out share an
        *    fd.  We could do with a more elaborate scheme to control
        *    this in the future.
        */
	(void) close(con->in.fd);

       /*
        * Mark the connection as stopping.  We can't reclaim the 
        * descriptor until the application does a sever, or else there
        * would be a risk of re-allocating it out from under the application.
        */

	con->status = CON_STOPPING;

	return;
}


/************************************************************************/
/*	
/*			gdb_de_allocate_connection
/*	
/*	Return a connection whose file descriptors have been closed
/*	to the pool.
/*	
/************************************************************************/

int
gdb_de_allocate_connection(con)
CONNECTION con;
{
	register int i;					

	con->status = CON_STOPPED;

	i = gdb_mcons-1;			/* start at last one used */

       /*
        * Reset gdb_mcons to be the number of connections in use
        */
	while (i>=0 && gdb_cons[i].status == CON_STOPPED)
		i--;

	gdb_mcons = i + 1;
}

/************************************************************************/
/*	
/*			g_cleanup_half_conection
/*	
/*	Terminate all pending operations on the supplied half 
/*	connection.  Note that the algorithm used here presumes
/*	that cancel_operation will de-queue the operation descriptor, 
/*	therefore we have to be careful here about when we look at 
/*	chain pointers.
/*	
/************************************************************************/

int
g_cleanup_half_connection(hcon)
HALF_CONNECTION hcon;
{
	OPERATION current, next;

	current = hcon->op_q_first;

       /*
        * Loop through all operations in the queue canceling them.
        * Make sure to pick up pointer to 'next' before the current
        * one is canceled, as cancelling may invalidate the pointer.
        */

	while (current != (OPERATION)hcon) {
		next = current->next;
		(void) cancel_operation(current);
		current = next;
	}
}

/************************************************************************/
/*	
/*		    create_listening_connection (create_listening_connection)
/*	
/*	Starts a special type of connection which is used to listen
/*	for incoming connection requests.  The inbound half-connection
/*	is the only one used for this special kind of connection.
/*	
/*	It is the user's responsibility to insure that only appropriate
/*	types of operation are queued on a connection of this sort.  In
/*	general, these connections are intended for internal use by
/*	GDB, and they are not intended to be visible to servers or
/*	clients directly.
/*	
/*	The id supplied should be in one of two forms.  If just a 
/*	string is supplied then it is presumed to be the name of
/*	a registered tcp service.  If the name begins with a #, then
/*	the rest is interpreted as the integer port number to be used.
/*	
/*	In future implementations, the id may have more structure, which
/*	is why we define it as a string.
/*	
/************************************************************************/

CONNECTION
create_listening_connection(id)
char *id;
{
	register CONNECTION con;		/* the connection we're */
						/* creating */

	register int slisten;			/* socket on which
						   we listen for connections */

	struct sockaddr_in self;
	int retries = GDB_BIND_RETRY_COUNT;
	int onoff = 1;				/* used as argument to */
						/* setsockopt */
	struct servent *serv;

	GDB_INIT_CHECK

       /*
        * Try to allocate a connection and fill it in with null values.
        */

	con = g_make_con();

       /*
        * Try to create a socket for listening
        */
	con->in.fd = socket(AF_INET, SOCK_STREAM, 0);
	slisten = con->in.fd;			/* easier and faster than */
						/* using con->in.fd all the */
						/* time*/
	if (slisten < 0 ) {
		gdb_perror("create_listening_connection: error creating listen socket");
	        (void) g_stop_with_errno(con);
		return con;
	}
       /*
        * Set options so the listening address can be re-used (this
        * has its dangers, but otherwise we can't restart our servers
        * for long periods after they crash because of connections which
        * take a long to time clean up and hold ports in use.)
        */

	/* Try 4.2 method */
	if(setsockopt(slisten, SOL_SOCKET, SO_REUSEADDR, (char *)0,0)<0)
	/* that didn't work, try 4.3 */
		if(setsockopt(slisten, SOL_SOCKET, SO_REUSEADDR,
			      (char *)&onoff, sizeof(int)) <0)
			GDB_GIVEUP("create_listening_connection: could not set SO_REUSEADDR")
			  ;
       /*
        * Make the listening socket non-blocking so we won't have to do
        * selects before polling it (change made by Bill Sommerfeld - wesommer)
        */
	if (ioctl(slisten, FIONBIO, (char *)&onoff) < 0) { /*<==FIX,,,add comment */
		g_stop_with_errno(con);
		gdb_perror("ioctl for listening socket");
		return con;
	}
       	/*----------------------------------------------------------*/
	/*	
	/*	Bind the socket to ourselves, using port derived from
	/*	the supplied id string.
	/*	
	/*----------------------------------------------------------*/

	bzero((char *)&self, sizeof(self));
       /*
        * Determine our port number
        */
	if (*id == '#')
		self.sin_port = htons((u_short)atoi(id+1));
	else {
		serv = getservbyname(id, "tcp");
		if (serv == NULL) {
			fprintf(gdb_log,"gdb create_listening_connection: cannot become service named %s\n",id);
			return NULL;		/* BUG: causes connetion */
						/* descriptor leakage.  Should */
						/* return an error code in */
						/* the connection descriptor*/
		}
		self.sin_port = serv->s_port;

	}
       /*
        * Try and re-try the bind until it works or until retry count
        * is exhausted.
        */
	while (bind(slisten,(struct sockaddr *)&self,sizeof(self)) < 0) {
		if (errno == EADDRINUSE && retries--) {
			fprintf(gdb_log,"gdb create_listening_connection: port address in use, will retry %d more time(s)\n",retries+1);
		        sleep(GDB_BIND_RETRY_INTERVAL);
			continue;
		} else {
			gdb_perror("gdb create_listening_connection: error binding listen socket");
			g_stop_with_errno(con);
			return con;
		}
	}

	/*----------------------------------------------------------*/
	/*	
	/*	Listen for connections.
	/*	
	/*----------------------------------------------------------*/

	(void) listen (slisten, 5);		/* does not block, just */
						/* sets the maximum backlog */
						/* of pending non-accepted */
						/* cons.*/

	con->in.flags |= HCON_LISTEN;
	con->out.flags |= HCON_UNUSED;
	con->status = CON_UP;
	if (con->in.fd +1 > gdb_mfd) 
		gdb_mfd = con->in.fd + 1;
	return con;
}

/************************************************************************/
/*	
/*			g_allocate_connection_buffers
/*	
/*	Create a buffer which can be used to receive large
/*	chunks of data from the socket.  This is currently done only
/*	on the inbound half connection.  Also, the buffers are not freed 
/*	once allocated, even if the connection descriptor is re-used.
/*	
/************************************************************************/

int
g_allocate_connection_buffers(con)
CONNECTION con;
{
	HALF_CONNECTION inbound = &(con->in);

       /*
        * See if there is already one allocated, if not, allocate one.
        */
	if (inbound->stream_buffer == (char *)NULL) {
		inbound->stream_buffer = 
		  db_alloc(inbound->stream_buffer_length);
	}

       /*
        * In any case, make sure that it is effectively empty
        */
	inbound -> stream_buffer_next = inbound -> stream_buffer;
	inbound -> stream_buffer_remaining = 0;
}
