/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/gdb_lib.h,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/gdb_lib.h,v 1.1 1987-08-02 22:17:09 wesommer Exp $
 */

/************************************************************************/
/*	
/*			gdb_lib.h
/*	
/*	Includes used within the Global Database Facilities library.
/*	Should NOT be included by typical users of gdb.
/*	
/*	Author: Noah Mendelsohn
/*	Copyright: 1986 MIT Project Athena
/*	
/************************************************************************/

/************************************************************************/
/*	
/*			    USER IDENTIFICATION
/*	
/*	gdb_init puts the user's i.d. and hostname as strings here.
/*	
/************************************************************************/

char *gdb_uname;				/* user's string name */
char *gdb_host;					/* name of local host */
						/* goes here */


/************************************************************************/
/*	
/*			    MEMORY MANAGEMENT
/*	
/*	These vectors point to the memory allocation and free routines.
/*	The standard routines supplied with the system are gdb_am and
/*	gdb_fm, but users may supply their own by clobbering the vectors.
/*	
/************************************************************************/

char *((*gdb_amv)()) = gdb_am;
int  (*gdb_fmv)() = gdb_fm;



/************************************************************************/
/*	
/*	
/*			  SYSTEM TYPE DEFINITIONS
/*	
/*	Declarations used to control the definition and use of 'types'
/*	as supported by the global database system.
/*	
/*	
/************************************************************************/

int	gdb_Debug=0;				/* debugging flags are */
						/* stored here */
FILE *gdb_log = stderr;				/* all debugging */
						/* output goes on stderr*/

char g_errstr[150];				/* build emsgs here */

/*
 * 			g_type_table
 * 
 * This is the table where the actual definitions for the types are
 * kept.
 */

gdb_type_def g_type_table[GDB_MAX_TYPES];
int gdb_n_types;				/* number of entries in */
						/* table */
/*
 * 			connection failure indicator
 * 
 * This variable is used to communicate between gdb_move_data and
 * g_con_progress without passing an extra parameter through lots
 * of procedure calls.  When set to FALSE, it indicates that the 
 * connection currently being processed has encountered a fatal error
 * and should be severed.
 */
int gdb_conok;					

/************************************************************************/
/*	
/*	     CONNECTION AND DATA TRANSMISSION SERVICES
/*	
/*	These are the global data structures used by the routines
/*	which maintain connections and do asynchronous data transfer
/*	on them.
/*	
/************************************************************************/

	/*----------------------------------------------------------*/
	/*	
	/*			gdb_cons
	/*	
	/*	This is the array of connection control data 
	/*	structures for gdb.  Every connection has its 
	/*	structure stored here, but they are in no 
	/*	particular order.  Because the connection data
	/*	itself cannot be moved (due to possible dangling
	/*	pointers), there may be some unused connections 
	/*	in the middle of this array.  gdb_mcons is the
	/*	1 based number of the highest connection which is
	/*	actually in use at this time.  This is a considerable
	/*	optimization for the typical case where very few
	/*	are in use, and turnover is low.
	/*	
	/*----------------------------------------------------------*/

int	gdb_mcons;				/* 0 based index of */
						/* last connection */
						/* currently in use */

int  gdb_mfd;					/* number of the highest */
						/* file descriptor in use */
						/* for a connection */
struct con_data gdb_cons[GDB_MAX_CONNECTIONS];	/* actual connection data */
						/* is stored here */

	/*----------------------------------------------------------*/
	/*	
	/*	Bit maps of the file descriptors involved in connections.
	/*	Technically, this is redundant with the information in
	/*	the connection descriptors above, but it makes select
	/*	preparation much faster.
	/*	
	/*----------------------------------------------------------*/

fd_set gdb_crfds, gdb_cwfds, gdb_cefds;		/* connection related file */
						/* descriptor maps to be */
						/* used in select */
	/*----------------------------------------------------------*/
	/*	
	/*			gdb_notime
	/*	
	/*	Pass this to select when doing a poll.
	/*	
	/*----------------------------------------------------------*/

struct timeval gdb_notime = {0,0};

/************************************************************************/
/*	
/*		       SERVER/CLIENT MANAGEMENT
/*	
/*	Definitions used in starting and maintaining communication 
/*	between servers and clients (as opposed to peers.)
/*	
/************************************************************************/

TUPLE_DESCRIPTOR gdb_tosrv;			/* descriptor for request */
						/* tuples sent to the */
						/* server during negotiation*/

TUPLE_DESCRIPTOR gdb_fmsrv;			/* descriptor for request */
						/* tuples sent from the */
						/* server during negotiation*/

	/*----------------------------------------------------------*/
	/*	
	/*	Global variables inherited by a child from a server
	/*	parent.
	/*	
	/*----------------------------------------------------------*/

TUPLE gdb_client_tuple;				/* request tuple sent from */
						/* the client */

char gdb_sockaddr_of_client[100];		/* this should really be */
						/* sockaddr_in, but I don't */
						/* want everyone to have */
						/* to include all those */
						/* big .h files */
int gdb_socklen;				/* length of above */

