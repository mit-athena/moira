/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/tcl.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/tcl.c,v 1.1 1987-08-02 22:13:37 wesommer Exp $
 */

#ifndef lint
static char *rcsid_tcl_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/tcl.c,v 1.1 1987-08-02 22:13:37 wesommer Exp $";
#endif	lint

/************************************************************************/
/*	
/*		          tcl (test client)
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
/*	A simple GDB client program.  This program is the client in a
/*	server/client pair.  It does a start server connection and then
/*	sends characters to the server for echoing.
/*	
/*	NOTES
/*	-----
/*	
/*	1) For simplicity, and do demonstrate the interactive response
/*	   of GDB connections, this program echoes single characters,
/*	   represented as ASCII coded integers.  Because of the polymorphism
/*	   inherent in the send_object and receive_object functions,
/*	   similar one line calls are used to transmit entire tuples
/*	   (aggregates of typed fields), relations (aggregates of tuples,
/*	   which are customarily represented by linked lists), etc.
/*	
/*	2) All communication done by this program and any other 
/*	   GDB based program is supported across incompatible
/*	   machine types.  For example, this program may run on an
/*	   RT/PC while the server may be running on a Vax.  Since
/*	   these machines use differing representations for integers,
/*	   GDB must do the necessary byte order conversions.
/*	
/*	4) Though this program is straightforward, it includes all
/*	   necessary error checking of the communication
/*	   path to the server.  For example, this program will 
/*	   terminate gracefully if the server crashes or is terminated
/*	   while this client is running.
/*	
/*	
/************************************************************************/


#include <stdio.h>
#include <sgtty.h>
#include "gdb.h"

struct sgttyb sgtty;

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

	CONNECTION server;			/* this represents the */
						/* server*/

	int data;				/* the input as typed */
	char c;					/* same input as a char */
						/* instead of an int */

	/*----------------------------------------------------------*/
	/*	
	/*	 	    EXECUTION BEGINS HERE
	/*	
	/*	Make sure the command line specifies the name
	/*	of the host on which the server program is running.
	/*	
	/*----------------------------------------------------------*/

	if (argc != 2) {
               /* 
                * Tell 'em the syntax
                */
		fprintf(stderr, "tcl <server-host-name:service-i.d.>\n");
		fprintf(stderr, "\tservice-i.d. is from #port number or name from /etc/services.\n");
		fprintf(stderr, "\tShould match name service i.d. given to tsr.\n");
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
	/*	Try for a connection to the server on the 
	/*	specified host.
	/*	
	/*----------------------------------------------------------*/

	printf("Attempting connection to server: %s\n", argv[1]);

	server = start_server_connection(argv[1], "Dummy parms");

        if (server == NULL) {
		fprintf(stderr,"Could not start connection to server\n");
		exit(8);
	}

	printf("Server connection started successfully.\n");

	/*----------------------------------------------------------*/
	/*	
	/*	Put the terminal into CBREAK mode, so that we'll
	/*	see each character as it's typed.  Also, turn off
	/*	local character echoing, since the whole point of
	/*	this program is to echo through the network.
	/*	
	/*----------------------------------------------------------*/

	ioctl(0, TIOCGETP, &sgtty);		/* go into cbreak */
	sgtty.sg_flags |= CBREAK;
	sgtty.sg_flags &= ~ECHO;
	ioctl(0, TIOCSETP, &sgtty);		/* go into cbreak */

	/*----------------------------------------------------------*/
	/*	
	/*	Start reading from keyboard and writing to server.
	/*	Send each character to the server separately, and
	/*	when it is echoed, present it on the screen.  <CRLF>
	/*	processing is done locally.  Loop until CTRL_D (EOT)
	/*      is entered.
	/*	
	/*----------------------------------------------------------*/

	printf("Start typing data to be uppercased and echoed. ^D Exits.\n\n\n");
	data = getchar();

#define EOT 4

	while (data != EOT) {
		if (send_object(server, (char *)&data, INTEGER_T) ==
		    OP_CANCELLED) {
			    fprintf(stderr,"\n\nSend error.\n");
			    break;
		}
		if (receive_object(server, (char *)&data, INTEGER_T) ==
		    OP_CANCELLED) {
			    fprintf(stderr,"\n\nSend error.\n");
			    break;
		}
		c = data;
		putchar(c);
		if (c == '\r')
			putchar('\n');
		data = getchar();
	}

	/*----------------------------------------------------------*/
	/*	
	/*	Put terminal back into normal line-at-a-time with
	/*	echoing mode.
	/*	
	/*----------------------------------------------------------*/

	sgtty.sg_flags |= ECHO;
	sgtty.sg_flags &= ~CBREAK;
	ioctl(0, TIOCSETP, &sgtty);		/* go into cbreak */

	/*----------------------------------------------------------*/
	/*	
	/*	Terminate the connection to the server.
	/*	
	/*----------------------------------------------------------*/

	printf("\n\nClosing connection\n\n");
	sever_connection(server);
	return;
}
			
