/*
 * $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/gdb_serv.c,v $
 * $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/gdb_serv.c,v 1.6 1994-09-16 16:24:50 jweiss Exp $
 */

#ifndef lint
static char *rcsid_gdb_serv_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/gdb_serv.c,v 1.6 1994-09-16 16:24:50 jweiss Exp $";
#endif	lint


/************************************************************************
 *	
 *			   gdb_serv.c
 *	
 *	      GDB - Routines to implement the server/client model
 *		    of connections.
 *	
 *	Author: Noah Mendelsohn
 *	Copyright: 1986 MIT Project Athena 
 *		For copying and distribution information, please see
 *	  	the file <mit-copyright.h>.
 *	
 ************************************************************************/

#include <mit-copyright.h>
#include <stdio.h>
#include <string.h>
#include "gdb.h"
#include <sys/ioctl.h>
#ifdef SOLARIS
#include <sys/filio.h>
#endif

	/*----------------------------------------------------------
	 *	
	 *	The following field names and types describe the
	 *	tuple sent from clients to servers during negotiation.
	 *	
	 *----------------------------------------------------------*/

char *g_tsv_field_names[] = {"server_id",
			    "parms",
		            "host",
		            "user"};
FIELD_TYPE g_tsv_ftypes[] = {STRING_T,
			    STRING_T,
		            STRING_T,
		            STRING_T};

#define TSV_FLDCOUNT 4

#define TSV_SERVER_ID 0
#define TSV_PARMS     1
#define TSV_HOST      2
#define TSV_USER      3


	/*----------------------------------------------------------*/
	/*	
	/*	The following field names and types describe the
	/*	tuple returned from the server to the client during
	/*	negotiation.
	/*	
	/*----------------------------------------------------------*/

char *g_fsv_field_names[] = {"disposition",
			     "server_id",
			     "parms"};
FIELD_TYPE g_fsv_ftypes[] = {INTEGER_T,
			     STRING_T,
			     STRING_T};

#define FSV_FLDCOUNT 3

#define FSV_DISPOSITION 0
#define FSV_SERVER_ID   1
#define FSV_PARMS       2


/************************************************************************/
/*	
/*			  gdb_i_srv
/*	
/*	Initialize the server client layer.
/*	
/*	This routine is called during gdb_init to do the processing
/*	common to all server/client handing.
/*	
/*	In particular, we build the tuple descriptors for the 
/*	messages used in negotiating the server/client startup.
/*	
/************************************************************************/

int
gdb_i_srv()
{
	gdb_tosrv = create_tuple_descriptor(TSV_FLDCOUNT, g_tsv_field_names, 
					    g_tsv_ftypes);
	gdb_fmsrv = create_tuple_descriptor(FSV_FLDCOUNT, g_fsv_field_names, 
					    g_fsv_ftypes);
	gdb_client_tuple = NULL;
	gdb_socklen = sizeof(gdb_sockaddr_of_client);
}


/************************************************************************/
/*	
/*		   start_server_connection (start_server_connection)
/*	
/*	This routine is called from a client that wishes to make a
/*	connection to a server.  In the current implementation, the
/*	string argument supplied is just the internet name of the 
/*	host on which the server runs.  This will later be generalized
/*	to a more flexible naming scheme.
/*	
/*	This routine builds a connection to the requested server, 
/*	sends the server i.d. and parms to the server (as strings), 
/*	and waits for a response indicating whether the server has
/*	agreed to the connection.  The server responds in one of three
/*	ways (1) connection accepted (2) connection declined (3) redirect.
/*	In this last case, the server returns a forwarding address to
/*	be substituted for the server_id, and the whole process is tried
/*	again repeatedly until a connection is established or a
/*	retry limit is exceeded.
/*	
/************************************************************************/

CONNECTION
start_server_connection(server_id, parms)
char *server_id;
char *parms;
{
	CONNECTION con;				/* the connection we're */
						/* creating */
	TUPLE response = NULL;			/* each time we try a server */
						/* it sends back its */
						/* response here */
	int retries = GDB_MAX_SERVER_RETRIES;	/* number of servers we'll */
						/* try before giving up in */
						/* fear of a loop */

	char serv_id[GDB_MAX_SERVER_ID_SIZE];	/* a place to store server */
						/* id's.  New ones go here */
						/* when our request is */
						/* forwarded */
	char latest_parms[GDB_MAX_SERVER_PARMS_SIZE];
						/* likewise for parms */

	GDB_INIT_CHECK

       /*
        * Try to allocate a connection and fill it in with null values.
        */

	con = g_make_con();

       /*
        * Loop asking different servers to accept our connection
        * until one does or we are flatly refused.
        */

	/*
	 * Allocate a buffer, if necessary, and reset buffer pointers
	 * so first request will result in a long read into the buffer
	 */
	g_allocate_connection_buffers(con);


	g_try_server(&con, server_id, parms, &response);

        while ((retries--) &&
	       con != NULL && 
	       response != NULL &&
	       *(int *)FIELD_FROM_TUPLE(response,FSV_DISPOSITION)==GDB_FORWARDED) {

		(void) sever_connection(con);
		con = g_make_con();
		(void) strcpy(serv_id, 
		       STRING_DATA(*(STRING *) 
				   (FIELD_FROM_TUPLE(response, 
						     FSV_SERVER_ID))));
		(void) strcpy(latest_parms, 
		       STRING_DATA(*(STRING *) 
				   (FIELD_FROM_TUPLE(response, 
						     FSV_PARMS))));
		null_tuple_strings(response);
		delete_tuple(response);
		g_try_server(&con, serv_id, latest_parms, &response);
	}

	/*
	 * At this point, we are done trying servers, now find out
         * whether we get to keep the connnection or whether it
         * didn't work.  First, see whether the connection is even up.
	 */
	if (con == NULL ||
	    connection_status(con) != CON_UP) {
		return con;
	}

       /*
        * We have at least some active connection, now see whether we 
        * are going to get to keep it
        */
	if (response != NULL &&
	    *(int *)FIELD_FROM_TUPLE(response,FSV_DISPOSITION) == GDB_ACCEPTED) {
		null_tuple_strings(response);
		delete_tuple(response);
		return con;
	} else {
		if (response != NULL) {
			null_tuple_strings(response);
			delete_tuple(response);
		}
		(void) sever_connection(con);
		return NULL;
	}
}

/************************************************************************/
/*	
/*	  		     g_try_server
/*	
/*	Builds a single connection to a server and returns status
/*	to indicate whether the connection has been accepted, declined,
/*	or is to be retried.  This status is conveyed in a tuple received
/*	back from the server.
/*	
/************************************************************************/

int
g_try_server(conp, server_id, parms, responsep)
CONNECTION *conp;
char *server_id;
char *parms;
TUPLE *responsep;
{

	register CONNECTION con = *conp;
	int flag = 1;
	
       /* 
        * In this implementation, we use a single fd for both inbound and
        * outbound traffic.  Try to connect to other side.  Current
        * implementation of this is synchronous--may be a problem?  If the
        * connections come up, then verify the level of protocol being
        * observed on the connections.  If incompatible, then turn off the
        * connection.
        */

 	if(!g_try_connecting(con,server_id) || 
	   con->status != CON_STARTING) {
		return;				/* If there we an error, */
						/* connection will have been */
						/* left CON_STOPPING with */
						/* possible errno set */
	}
	g_ver_oprotocol(con);
	if (con->status != CON_UP) {
		return;
	}

       /*
        * We've successfully started the connection, now mark
        * it for non-blocking I/O.  Also, update the high water
        * mark of fd's controlled by our system.
        */
	if(ioctl(con->in.fd, FIONBIO, (char *)&flag)== (-1)) {
			g_stop_with_errno(con);
			gdb_perror("gdb: ioctl for non-block failed");
			return;
	}
	if (con->in.fd +1 > gdb_mfd) 
			gdb_mfd = con->in.fd + 1;

	g_ask_server(conp, server_id, parms, responsep);	

	return;
}


/************************************************************************/
/*	
/*			g_ask_server
/*	
/*	Called once we are in touch with the server and our physical
/*	transmission protocol is comprehensible.  This routine
/*	sends out a tuple containing the server i.d. and parameter
/*	strings and it returns a tuple received back from the server
/*	containing the server's response.
/*	
/************************************************************************/

int
g_ask_server(conp, server_id, parms, responsep)
CONNECTION *conp;
char *server_id;
char *parms;
TUPLE *responsep;
{
	register CONNECTION con = *conp;
	TUPLE out_tuple;
	int rc;
	/*----------------------------------------------------------*/
	/*	
	/*	Create a tuple to be sent out containing the
	/*	server_id and parms.
	/*	
	/*----------------------------------------------------------*/

	out_tuple = create_tuple(gdb_tosrv);	/* descriptor was pre- */
						/* built during */
						/* initialization*/

	(void) string_alloc((STRING *)FIELD_FROM_TUPLE(out_tuple, TSV_SERVER_ID),
		     strlen(server_id)+1);
	(void) strcpy(STRING_DATA(*((STRING *)FIELD_FROM_TUPLE(out_tuple,0))),
		       server_id);

	if (parms == NULL) parms = "";
	(void) string_alloc((STRING *)FIELD_FROM_TUPLE(out_tuple, TSV_PARMS),
		     strlen(parms)+1);
	(void) strcpy(STRING_DATA(*((STRING *)FIELD_FROM_TUPLE(out_tuple,1))),
		       parms);

	(void) string_alloc((STRING *)FIELD_FROM_TUPLE(out_tuple, TSV_HOST),
		     strlen(gdb_host)+1);
	(void) strcpy(STRING_DATA(*((STRING *)FIELD_FROM_TUPLE(out_tuple,
							       TSV_HOST))),
		       gdb_host);
	(void) string_alloc((STRING *)FIELD_FROM_TUPLE(out_tuple, TSV_USER),
		     strlen(gdb_uname)+1);
	(void) strcpy(STRING_DATA(*((STRING *)FIELD_FROM_TUPLE(out_tuple,TSV_USER))),
		       gdb_uname);

	/*----------------------------------------------------------*/
	/*	
	/*	Send the tuple to the server, and make sure that
	/*	we succeeded.
	/*	
	/*----------------------------------------------------------*/

	rc = send_object(con, (char *)&out_tuple, TUPLE_T);

	null_tuple_strings(out_tuple);
	delete_tuple(out_tuple);

	if (rc != OP_SUCCESS) {
		return;				/* cleanup from dying send */
						/* should have made this */
						/* CON_STOPPING with errno */
	}

	/*----------------------------------------------------------*/
	/*	
	/*	OK, we sent it out, now lets read back the response.
	/*	
	/*----------------------------------------------------------*/

	rc = receive_object(con, (char *)responsep, TUPLE_T);

	if (rc != OP_SUCCESS) {
		return;				/* cleanup from dying send */
						/* should have made this */
						/* CON_STOPPING with errno */
	}
}


/************************************************************************/
/*	
/*			start_replying_to_client
/*	
/*	Queue an operation which will send a reply to the specified
/*	client.
/*	
/************************************************************************/

struct rtc_data {
	TUPLE reply_data;
	OPERATION tuple_send;
};

int g_irtc();
int g_i2rtc();

int
start_replying_to_client(op, con, disposition, serverid, parms)
OPERATION op;
CONNECTION con;
int disposition;
char *serverid;					/* null terminated */
char *parms;					/*   "     "       */
{
	register struct rtc_data *arg;
	register TUPLE t;

       /*
        * Make sure the supplied connection is a legal one
        */
	GDB_CHECK_CON(con, "start_replying_to_client")

	arg = (struct rtc_data *)db_alloc(sizeof(struct rtc_data));

       /*
        * create an empty operation and a tuple
        */
	arg->tuple_send = create_operation();
	arg->reply_data = create_tuple(gdb_fmsrv);
       /*
        * Fill in the response tuple
        */
	t = arg->reply_data;			/* quicker and easier here */

	*(int *)FIELD_FROM_TUPLE(t,FSV_DISPOSITION) = disposition;
	(void) string_alloc((STRING *)FIELD_FROM_TUPLE(t, FSV_SERVER_ID),
		     strlen(serverid)+1);
	(void) strcpy(STRING_DATA(*(STRING *)(FIELD_FROM_TUPLE(t, FSV_SERVER_ID))),
				       serverid);

	(void) string_alloc((STRING *)FIELD_FROM_TUPLE(t, FSV_PARMS),
		     strlen(parms)+1);
	(void) strcpy(STRING_DATA(*(STRING *)(FIELD_FROM_TUPLE(t, FSV_PARMS))),
				       parms);

       /*
        * Queue an operation ahead of us which will send the response tuple
        * to the client
        */
	start_sending_object(arg->tuple_send, con, (char *)&arg->reply_data,
			     TUPLE_T);
	
       /*
        * Now queue us behind it.  By the time we run our init routine,
        * a connection should have been acquired.
        */
	initialize_operation(op, g_irtc, (char *)arg, (int (*)())NULL);
	(void) queue_operation(con, CON_INPUT, op);
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_irtc
	/*	
	/*	Init routine for replying to a client.  If all went
	/*	well, (or even if it didn't), then we are done.
	/*	All we have to do is clean up the stuff we've allocated.
	/*	
	/*----------------------------------------------------------*/

/*ARGSUSED*/
int
g_irtc(op, hcon, arg)
OPERATION op;
HALF_CONNECTION hcon;
struct rtc_data *arg;
{
	int rc;

       /*
        * Figure out the return information to our caller
        */
	rc = OP_STATUS(arg->tuple_send);

       /*
        * Release all transient data structures.
        */
	null_tuple_strings(arg->reply_data);
	delete_tuple(arg->reply_data);
	delete_operation(arg->tuple_send);
	db_free((char *)arg, sizeof(struct rtc_data));
	return rc;
}
