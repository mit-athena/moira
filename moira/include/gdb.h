/*
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/gdb.h,v 1.6 1992-12-01 11:39:59 mar Exp $
 */

/************************************************************************
 *	
 *			gdb.h
 *	
 *	Includes for the global database facility (gdb)
 *	
 *	Author: Noah Mendelsohn
 *	Copyright: 1986 MIT Project Athena
 *	
 ************************************************************************/

/*
 * Note: following include may safely be done redundantly, so it doesn't
 * matter if caller does it too.  We need it for fd_set.
 */
#include <sys/types.h>
#include <sys/time.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef NFDBITS
#define NFDBITS 32
#endif
#ifndef howmany
#define	howmany(x, y)	(((x)+((y)-1))/(y))
#endif
#ifndef FD_SETSIZE
#define FD_SETSIZE 256
#endif
#ifndef FD_SET
#define FD_SET(n, p)    ((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define FD_CLR(n, p)    ((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)      bzero(p, sizeof(*(p)))
typedef long fd_mask;
typedef struct fd_set {fd_mask fds_bits[howmany(FD_SETSIZE, NFDBITS)];} fd_set;
#endif 



	/*----------------------------------------------------------*
	 *	
	 *			GDB_GIVEUP
	 *	
	 *	This macro is called with a string argument whenever a
	 *	fatal error is encounterd.  If you re-define this
	 *	macro, you can control the disposition of fatal gdb
	 *	errors.
	 *	
	 *	The gdb library must be recompiled for the change to
	 *	take effect.  That will have to be fixed sometime.
	 *	
	 *----------------------------------------------------------*/

#define GDB_GIVEUP(errormsg) g_givup(errormsg);

	/*----------------------------------------------------------*
	 *	
	 *	                GDB_ROUNDUP
	 *	
	 *	Round a number up to the next specified boundary.
	 *	
	 *----------------------------------------------------------*/

#define GDB_ROUNDUP(n,b) ((((n)+(b)-1)/(b))*(b))

extern int	gdb_Options;			/* Control optional features */
#define GDB_OPT_KEEPALIVE	1
extern int	gdb_Debug;			/* debugging flags are */
						/* stored here */
extern FILE *gdb_log;				/* file descriptor for */
						/* logging gdb output */


/*
 * 			connection failure indicator
 * 
 * This variable is used to communicate between gdb_move_data and
 * g_con_progress without passing an extra parameter through lots
 * of procedure calls.  When set to FALSE, it indicates that the 
 * connection currently being processed has encountered a fatal error
 * and should be severed.
 */
extern int gdb_conok;					
/************************************************************************
 *	
 *			    USER IDENTIFICATION
 *	
 *	gdb_init puts the user's i.d. and hostname as strings here.
 *	
 ************************************************************************/

extern char *gdb_uname;				/* user's string name */
extern char *gdb_host;				/* name of local host */
						/* goes here */



/************************************************************************
 *	
 *	
 *			  TYPE MANAGEMENT
 *	
 *	Declarations used to control the definition and use of 'types'
 *	as supported by the global database system.  Most of the 
 *	declarations for 'System Defined Types' will not be of concern
 *	to typical users, with the exception of the type names like
 *	INTEGER_T which are defined below.
 *	
 *	In this implementation, user defined types are added
 *	dynamically during execution by calling the appropriate
 *	functions.  The define GDB_MAX_TYPES below sets the maximum
 *	total number of types, including both system and user defined,
 *	which the system can support for any one application.  When
 *	GDB_MAX_TYPES is changed, the libary must be re-built.  Space
 *	for a two dimensional array, with one word for each property
 *	of each possible type, is allocated statically in the library.
 *	
 ************************************************************************/


/*
 * Maximum number of types we can support, including both system and
 * user defined.
 */

#define GDB_MAX_TYPES 50

typedef int FIELD_TYPE;				/* data needed to repre- */
						/* sent a system or user */
						/* defined data type */
						/* This is actualy just */
						/* a row index in the type */
						/* definition table */

	/*----------------------------------------------------------
	 *	
	 *		System defined types
	 *	
	 *	WARNING: Any changes to these type definitions must be
	 *	carefully matched with the initializations in the
	 *	gdb_i_stype routine in gdb_stype.c.  Mistakes in these
	 *	tables may be VERY difficult to debug.  Be careful!
	 *	
	 *----------------------------------------------------------*/

/*
 * Primitive types for ingres data
 */

#define INTEGER_T (0)
#define STRING_T (1)
#define REAL_T (2)
#define DATE_T (3)

/*
 * Structured types
 */

#define TUPLE_DESCRIPTOR_T (4)
#define TUPLE_T (5)
#define TUPLE_DATA_T (6)
#define RELATION_T (7)

/*
 * Number of system defined types
 * 
 * This will always be equal to one more than index of last system type
 */

#define SYSTEM_TYPE_COUNT 8

	/*----------------------------------------------------------
	 *	
	 *		Type descriptor tables
	 *	
	 *----------------------------------------------------------*/

/*
 * 			gdb_prop_union
 * 
 * Each entry in the type definition table is a union of this form,
 * which allows us to store a choice of an integer, a function 
 * pointer, or a pointer to a character string.
 */

union gdb_prop_union {
	int i;					/* when we want as an */
						/* integer */
	int (*f)();				/* as a function pointer */
	char *cp;				/* character pointer  */
};

#define TYPE_PROPERTY_COUNT 8			/* number of properties */
						/* for each type*/

/*
 * Uses of the type properties.  Each type has a set of properties.
 * Some are integers, some are functions.  The defines below descrive
 * respectively the 0'th,1'st, 2'nd, etc. properties of EACH type.
 * 
 * Note:  TYPE_PROPERTY_COUNT (above) must be changed when new properties
 * are added.  For system defined types, bindings for the properties
 * are done in gdb_i_stype in the gdb_stype.c source file.
 */

#define LENGTH_PROPERTY 0			/* integer */
#define ALIGNMENT_PROPERTY 1			/* integer */
#define NULL_PROPERTY 2				/* function */
#define CODED_LENGTH_PROPERTY 3			/* function */
#define ENCODE_PROPERTY 4			/* function */
#define DECODE_PROPERTY 5			/* function */
#define FORMAT_PROPERTY 6			/* function */
#define NAME_PROPERTY 7				/* char pointer */

/*
 * 			gdb_type_def
 * 
 * Information to describe a single type
 */

typedef union gdb_prop_union gdb_type_def[TYPE_PROPERTY_COUNT];


/*
 * 			g_type_table
 * 
 * This is the table where the actual definitions for the types are
 * kept.
 */

extern gdb_type_def g_type_table[GDB_MAX_TYPES];
extern int gdb_n_types;				/* number of entries in */
						/* table */

	/*----------------------------------------------------------
	 *	
	 *	    Macros for accessing properties
	 *	
	 *----------------------------------------------------------*/

#define INT_PROPERTY(type, prop) (g_type_table[type][prop].i) 
#define STR_PROPERTY(type, prop) (g_type_table[type][prop].cp) 
#define FCN_PROPERTY(type, prop) (*g_type_table[type][prop].f) 


/************************************************************************
 *	
 *			   STRUCTURED DATA
 *	
 *	Stuff needed to declare and manage TUPLES, TUPLE_DESCRIPTORS
 *	and RELATIONS.
 *	
 ************************************************************************/

	/*----------------------------------------------------------
	 *	
	 *		   TUPLE_DESCRIPTOR
	 *	
	 *----------------------------------------------------------*/

#define GDB_DESC_ID 0x54504400			/* "TPD" */

struct tupld_var {				/* the variable length */
						/* stuff in a tuple */
						/* descriptor*/
	char *name;				/* string name of field */
	FIELD_TYPE type;			/* type of this field */
	int  offset;				/* byte offset of this field */
						/* relative to first byte of */
						/* data (not start of whole */
						/* tuple) */
	int length;				/* Length of the actual data */
						/* for this field. Note that */
						/* alignment requirements of */
						/* following field are NOT */
						/* included in this length */
};
struct tupl_desc {
	long id;				/* this should say TPD\0 */
	int ref_count;				/* when this goes to zero, */
						/* the descriptor may really */
						/* be reclaimed */
	int field_count;			/* number of fields in */
						/* the tuple*/
	int data_len;				/* length of the data in */
						/* the actual tuple */
	int str_len;				/* length of the strings */
						/* stored off the end of */
						/* this descriptor*/
	struct tupld_var var[1];		/* one of these for each */
						/* field, but the C compiler */
						/* won't accept the[] decl, */
						/* because it's afraid of */
						/* uncertain length*/
};

typedef struct tupl_desc *TUPLE_DESCRIPTOR;	/* use this to declare a */
						/* tuple descriptor anchor */

#define gdb_descriptor_length(num_flds) (sizeof(struct tupl_desc) + ((num_flds)-1) * sizeof(struct tupld_var))

	/*----------------------------------------------------------
	 *	
	 *			TUPLE
	 *	
	 *	tuple_dat is allocated by the create_tuple routine.
	 *	
	 *	TUPLE may be used in user code to declare a handle
	 *	on a tuple.
	 *	
	 *----------------------------------------------------------*/

#define GDB_TUP_ID 0x54555000

typedef struct tuple_dat *TUPLE;		/* handle on a tuple */

struct tuple_dat {
	TUPLE next, prev;			/* chain pointers when */
						/* tuples are linked, as in */
						/* a relation.  Convention is*/
						/* double linked, circular.*/
	long id;				/* should say TUP\0  */
	TUPLE_DESCRIPTOR desc;			/* pointer to the descriptor */
	char data[1];				/* data goes here, word */
						/* aligned.  Should be [] */
						/* if compiler would allow */
};


	/*----------------------------------------------------------
	 *	
	 *		      RELATION
	 *	
	 *	rel_dat is allocated by the create_relation
	 *	routine.
	 *	
	 *	RELATION may be used in user code to declare a handle
	 *	on a relation.
	 *	
	 *----------------------------------------------------------*/

#define GDB_REL_ID 0x52454c00

struct rel_dat {
	TUPLE first, last;			/* chain pointers to tuples */
						/* note that first->prev and */
						/* last->next are both == */
						/* &rel-dat.  Maintenance is */
						/* simplified by keeping */
						/* as the first fields in */
						/* both rel_dat and tuple_dat*/
						/* a minor non-portability */
	long id;				/* should contain REL\0  */
	TUPLE_DESCRIPTOR desc;			/* descriptor for the tuples */
						/* in this relation. Should */
						/* == that in each tuple */

};

typedef struct rel_dat *RELATION;		/* handle on a relation */

/************************************************************************
 *	
 *		      transport LAYER DECLARATIONS
 *	
 *	Declares the state maintenance structures for full duplex 
 *	connections with asynchronous transmission capability.  Most
 *	users need only know that the type CONNECTION is defined, and
 *	that it may be treated as a pointer for most purposes (i.e. it
 *	is compact, and copying it does not actually copy the connection
 *	state.)
 *	
 ************************************************************************/

#define GDB_PROTOCOL_VERSION 0x01		/* version of the gdb */
						/* protocols that we're */
						/* observing */
#define GDB_STREAM_BUFFER_SIZE 4096		/* amount to read in */
						/* one chunk from tcp stream*/
#define GDB_PORT htons(9425)			/* temporary until we use */
						/* services properly */
#define GDB_BIND_RETRY_COUNT 5			/* number of times to */
						/* retry a bind before */
						/* giving up.  Used when */
						/* accepting connections */
#define GDB_BIND_RETRY_INTERVAL 10		/* Number of seconds to wait */
						/* between attempts to bind */
						/* the listening socket */
#define GDB_MAX_CONNECTIONS 25			/* maximum number of */
						/* connections that */
						/* any one process can */
						/* control simultaneously */
/*
 * Circumvent bug in ACIS 4.2 socket support
 */
#ifdef ibm032
#define GDB_MAX_SOCK_WRITE 2047			/* rt can't do socket */
						/* writes longer than this */
						/* gives errno 40*/
#else
#define GDB_MAX_SOCK_WRITE 0x00ffffff
#endif

	/*----------------------------------------------------------
	 *	
	 *	     Declarations for HALF_CONNECTIONS
	 *	
	 *	Each full duplex connection has associated with it 
	 *	two simplex half-connections, each of which 
	 *	has its own queue of pending operations.  The
	 *	following describes the state of a half-connection.
	 *	
	 *----------------------------------------------------------*/

struct half_con_data {
       /*
        * these two must be first to match position in OPERATION
        */
	struct oper_data *op_q_first;		/* first item in q of pending*/
						/* operations for this */
						/* half-connection. (chained */
						/* circularly to half con, */
						/* NOT the con. */
	struct oper_data *op_q_last;		/* last item in q of pending*/
						/* operations for this */
						/* half-connection*/
	int status;				/* values are of type */
						/* OPSTATUS. tells whether */
						/* transmit/receive is */
						/* pending.*/
	long flags;				/* bit flags */
#define HCON_PROGRESS 0x00000001		/* used by selection */
						/* routines to detect */
						/* progress */
#define HCON_LISTEN   0x00000002		/* indicates that this is */
						/* a special half connection */
						/* used only for listenting */
						/* to incoming connection */
						/* requests */
#define HCON_UNUSED   0x00000004		/* on a one-way connection, */
						/* this flag marks an unused */
						/* half */
#define HCON_PENDING_LISTEN 0x00000008		/* a queued op on this half */
						/* connection is actually */
						/* trying to listen */
#define HCON_BUSY     0x00000010		/* we are currently making */
						/* progress on this half */
						/* connection.  Used to */
						/* detect re-entrance of */
						/* hcon_progress */
	int fd;					/* main half duplex file */
						/* descriptor for this h_conn*/
	int oob_fd;				/* file descriptor for */
						/* out of band signals*/
	char *next_byte;			/* next byte to send/recv */
	int remaining;				/* number of bytes remaining */
						/* to send/receive */
	char *stream_buffer;			/* points to a buffer */
						/* used to pre-read/write */
						/* the stream to avoid */
						/* window thrashing */
	int stream_buffer_length;		/* length of the stream */
						/* buffer */
	char *stream_buffer_next;		/* next byte to read in */
						/* stream buffer */
	int stream_buffer_remaining;		/* number of bytes currently */
						/* in use in stream buffer*/
	int *accepted_fdp;			/* used only for listening */
						/* connections.  latest */
						/* accepted fd is put where*/
						/* this points */
	int *accepted_len;			/* ptr to length of 'from' */
						/* data on accept  */

};

typedef struct half_con_data *HALF_CONNECTION;


	/*----------------------------------------------------------
	 *	
	 *	     Declarations for CONNECTIONS
	 *	
	 *----------------------------------------------------------*/

#define GDB_CON_ID 0x434f4e00 /*"CON"*/

struct con_data {
	long	id;				/* should contain CON\0  */
	int status;				/* See definitions below. */
						/* Do not confuse with */
						/* the status sub-fields of */
						/* in and out half-cons. */
	int version;				/* the version of the */
						/* protocol being observed */
						/* on this connection */
	int errno;				/* the real errno gets */
						/* copied here if it causes */
						/* the connection to die */
	int (*oob_fcn)();			/* pointer to function to */
						/* call when something */
						/* arrives on the out of */
						/* band channel */
	struct half_con_data in, out;		/* states of the inbound */
						/* and outbound half */
						/* sessions.*/
};

typedef struct con_data *CONNECTION;		/* the only externally */
						/* visible handle on a */
						/* connection*/

/*
 * Definitions of status fields.
 * 
 * WARNING: If you change any of the following, there are coordinated
 * changes to be made in gdb_debug.c
 */

#define CON_STOPPED 1				/* never started, terminated */
#define CON_UP 2				/* ready to use */
#define CON_STARTING 3				/* trying to start */
#define CON_STOPPING 4				/* trying to stop */

/*
 * The following are used as keywords when distinguishing input from output
 * half connections.
 */
#define CON_INPUT 1				/* this is an input half */
						/* session*/
#define CON_OUTPUT 2				/* this is an output half */
						/* session*/


	/*----------------------------------------------------------
	 *	
	 *			gdb_cons
	 *	
	 *	This is the array of connection control data 
	 *	structures for gdb.  Every connection has its 
	 *	structure stored here, but they are in no 
	 *	particular order.  Because the connection data
	 *	itself cannot be moved (due to possible dangling
	 *	pointers), there may be some unused connections 
	 *	in the middle of this array.  gdb_mcons is the
	 *	1 based number of the highest connection which is
	 *	actually in use at this time.  This is a considerable
	 *	optimization for the typical case where very few
	 *	are in use, and turnover is low.
	 *	
	 *	These are externs for globals defined in gdb_lib.h
	 *	and included by gdb.c.
	 *	
	 *----------------------------------------------------------*/

extern int  gdb_mcons;				/* one based number of the */
						/* highest connection */
						/* descriptor we're using */
						/* at the moment */

extern int  gdb_mfd;				/* number of the highest */
						/* file descriptor in use */
						/* for a connection */
extern struct con_data gdb_cons[GDB_MAX_CONNECTIONS];
						/* actual connection data */
						/* is stored here */

extern fd_set gdb_crfds, gdb_cwfds, gdb_cefds;	/* connection related file */
						/* descriptor maps to be */
						/* used in select */
extern fd_set last_crfds, last_cwfds, last_cefds;/* these file desc. bit */
						/* masks are set up */
						/* for each select call */
						/* to include the user */
						/* supplied and the */
						/* connection related */
						/* fd's */

	/*----------------------------------------------------------
	 *	
	 *			OPERATIONS
	 *	
	 *----------------------------------------------------------*/

#define GDB_OP_ID 0x4f505200

struct oper_data {
	struct oper_data *next, *prev;		/* forward and back chain */
	long id;				/* should contain OPR\0  */
	int tag;				/* unique identifier for */
						/* this operation */
	int status;				/* current state of this */
						/* oaperation*/
	int flags;
#define OPF_MARKED_COMPLETE 0x00000001		/* operation was already */
						/* complete when opsel was */
						/* called*/
#define OPF_MARKED_CANCELLED 0x00000002		/* operation was already */
						/* cancelled when opsel was */
						/* called*/
	int result;				/* when the operation */
						/* completes, this field is */
						/* set to reflect its dispos-*/
						/* ition. Contents of this */
						/* field will depend on the */
						/* operation being performed */
	HALF_CONNECTION halfcon;		/* the half connection on */
						/* which this operation is */
						/* queued */
						/* May be meaningless if not */
						/* queued..*/
	char *arg;				/* pointer to user data */
	union fcn {
		int (*init)();			/* pointer to routine to */
						/* call to start this */
					
	/* operation*/
		int (*cont)();			/* pointer to function to */
						/* be called when this */
						/* logical operation */
						/* continues*/
	} fcn;

	int (*cancel)();			/* this routine gets called */
						/* to handle a cancel request*/
						/* this field is normally */
						/* set to NULL when the */
						/* operation is queued and */
						/* updated as required by */
						/* the init and cont funcs. */
};

typedef struct oper_data OPERATION_DATA;	/* an actual operation */
						/* descritor, creator is */
						/* responsible for making */
						/* sure that memory is not */
						/* freed while in use */
typedef OPERATION_DATA *OPERATION;		/* a handle on an operation */
						/* this is what most */
						/* applications will use */

	/*----------------------------------------------------------
	 *	
	 *	          STATES OF AN OPERATION
	 *	
	 *	These represent the state of an asynchronous, queued
	 *	operation.  For convenience of the application programmer,
	 *	some of these are folded together when queried through the
	 *	operation_status routine.  In particular, operation status
	 *	returns only one of:
	 *	
	 *	OP_NOT_RUNNING, OP_RUNNING, OP_COMPLETE,  or 
	 *	OP_CANCELLED.
	 *	
	 *	Any other status is reported as OP_RUNNING.  This is
	 *	done on the assumption that it makes correct coding
	 *	of applications less error-prone, as there are fewer
	 *	cases to check, and most of them would not be of 
	 *	interest anyway.
	 *	
	 *	Note that OP_CANCELLED may be generated by the system
	 *	even when no explicit request for cancellation has been
	 *	issued.  For example, this may occur when a connection
	 *	is severed unexpectedly.
	 *	
	 *	WARNING:  If you change any of the following, be sure
	 *	to make the equivalent changes to gdb_debug.c.
	 *	
	 *	We also define here certain standard values of OP_RESULT,
	 *	since some return conventions presume that op_status 
	 *	and op_result values are orthogonal.
	 *	
	 *----------------------------------------------------------*/

#define OP_SUCCESS     0			/* this value is actually */
						/* used only in result */
						/* fields, but it is */
						/* sometimes convenient to */
						/* have status and result */
						/* have orthogonal values */
#define OP_NOT_STARTED 1			/* this operation has been */
						/* initialized but is not on */
						/* any connection's queue */
#define OP_QUEUED 2				/* this operation is on */
						/* some connection's queue */
						/* but it has not yet */
						/* reached the head of the q */
#define OP_RUNNING 3				/* op is at head of q trying */
						/* to progress */
#define OP_COMPLETE 4				/* operation has run to */
						/* completion.  result field */
						/* is now valid */
#define OP_CANCELLING 5				/* we are in the process of */
						/* (trying to) cancel this */
						/* operation */
#define OP_CANCELLED 6				/* operation was prematurely */
						/* terminated.  result field */
						/* is NOT valid. */
#define OP_MARKED 7				/* used by op_select_all */
#define OP_REQUEUED 8				/* returned by an init or */
						/* continuation routine to */
						/* indicate that the */
						/* operation has requeued */
						/* itself */
#define OP_PREEMPTED 9				/* returned by an init or */
						/* continuation routine to */
						/* indicate that the op has */
						/* preempted itself by */
						/* queueing a new operation */
						/* ahead of itself */

	/*----------------------------------------------------------
	 *	
	 *		     LIST_OF_OPERATIONS
	 *	
	 *----------------------------------------------------------*/

struct oper_list {
	int	count;				/* number of operations */
						/* in the list */
	OPERATION op[1];			/* really op[n], but */
						/* structs must have a */
						/* definite length */
};

typedef struct oper_list *LIST_OF_OPERATIONS;	/* handle on a list */

#define size_of_list_of_operations(n)					\
		(sizeof(struct oper_list) + (n-1)*sizeof(OPERATION))

	/*----------------------------------------------------------
	 *	
	 *			gdb_notime
	 *	
	 *	Pass this to select when doing a poll.
	 *	
	 *----------------------------------------------------------*/

extern struct timeval gdb_notime;


/************************************************************************
 *	
 *		CHECKING ROUTINES IMPLEMENTED AS MACROS
 *	
 ************************************************************************/

extern char g_errstr[150];			/* build emsgs here */

#define GDB_INIT_CHECK g_chk_init();		/* make sure gdb_init */
						/* was called */

#define GDB_CHECK_CON(con, where) if ((con)->id != GDB_CON_ID) \
	{ (void) sprintf(g_errstr, "Invalid connection descriptor passed to \"%s\"\n", where); \
	GDB_GIVEUP(g_errstr) }


#define GDB_CHECK_TUP(tup, where) if ((tup)->id != GDB_TUP_ID) \
	{ (void) sprintf(g_errstr, "Invalid tuple passed to \"%s\"\n", where); \
	GDB_GIVEUP(g_errstr) }


#define GDB_CHECK_TPD(tpd, where) if ((tpd)->id != GDB_DESC_ID) \
	{ (void) sprintf(g_errstr, "Invalid tuple descriptor passed to \"%s\"\n", where); \
	GDB_GIVEUP(g_errstr) }


#define GDB_CHECK_REL(rel, where) if ((rel)->id != GDB_REL_ID) \
	{ (void) sprintf(g_errstr, "Invalid relation passed to \"%s\"\n", where); \
	GDB_GIVEUP(g_errstr) }

#define GDB_CHECK_OP(op, where) if ((op)->id != GDB_OP_ID) \
	{ (void) sprintf(g_errstr, "Invalid operation passed to \"%s\"\n", where); \
	GDB_GIVEUP(g_errstr) }

#define GDB_CHECK_DB(db, where) if (db->id != GDB_DB_ID) \
	{ (void) sprintf(g_errstr, "Invalid database handle passed to \"%s\"\n", where); \
	GDB_GIVEUP(g_errstr) }




/************************************************************************
 *	
 *		TRANSPORT ROUTINES IMPLEMENTED AS MACROS
 *	
 ************************************************************************/

	/*----------------------------------------------------------
	 *	
	 *		   connection_status
	 *	
	 *	Returns the status of the indicated connection.
	 *	Possible return values are:
	 *	
	 *		CON_STOPPED	never started or terminated
	 *		CON_UP		currently usable
	 *		CON_STARTING    transient state on way up
	 *		CON_STOPPING    transient state on way down
	 *	
	 *----------------------------------------------------------*/

#define connection_status(con) ((con)->status)

	/*----------------------------------------------------------
	 *	
	 *		   connection_errno
	 *	
	 *	When a connection dies due to an error on a system
	 *	call, the corresponding errno is recorded in the 
	 *	connection descriptor.  This macro returns that value.
	 *	
	 *----------------------------------------------------------*/

#define connection_errno(con) ((con)->errno)



/************************************************************************
 *	
 *		       SERVER/CLIENT MANAGEMENT
 *	
 *	Definitions used in starting and maintaining communication 
 *	between servers and clients (as opposed to peers.)
 *	
 ************************************************************************/

#define GDB_MAX_SERVER_RETRIES 3		/* maximum number of times */
						/* clients will accept */
						/* forwarding requests from */
						/* a given server */


extern TUPLE_DESCRIPTOR gdb_tosrv;		/* descriptor for request */
						/* tuples sent to the */
						/* server during negotiation*/

extern TUPLE_DESCRIPTOR gdb_fmsrv;		/* descriptor for request */
						/* tuples sent from the */
						/* server during negotiation*/

#define GDB_MAX_SERVER_ID_SIZE 255		/* longest name of a server */
						/* that we can handle */
#define GDB_MAX_SERVER_PARMS_SIZE 1023		/* longest parm string we */
						/* can exchange between */
						/* server and client*/

	/*----------------------------------------------------------
	 *	
	 *	The following are values returned in the disposition
	 *	field of the response tuple to indicate what the
	 *	server has decided to do about the connection 
	 *	request.
	 *	
	 *----------------------------------------------------------*/

#define GDB_ACCEPTED 1
#define GDB_REFUSED  2
#define GDB_FORWARDED 3

	/*----------------------------------------------------------
	 *	
	 *	Global variables inherited by a child from a server
	 *	parent.
	 *	
	 *----------------------------------------------------------*/

extern TUPLE gdb_client_tuple;			/* request tuple sent from */
						/* the client */

extern char gdb_sockaddr_of_client[100];	/* this should really be */
						/* sockaddr_in, but I don't */
						/* want everyone to have */
						/* to include all those */
						/* big .h files */
extern int gdb_socklen;				/* length of above */



/************************************************************************
 *	
 *			DATABASE MANAGEMENT
 *	
 *	This layer of GDB provides access to the services of a relational
 *	database from anywhere in a GDB network.
 *	
 ************************************************************************/

	/*----------------------------------------------------------
	 *	
	 *			GDB_DB_SERVICE
	 *	
	 *	The name of the service, as found in /etc/services,
	 *	for GDB database servers.
	 *	
	 *----------------------------------------------------------*/

#define GDB_DB_SERVICE "#9420"

	/*----------------------------------------------------------
	 *	
	 *			DATABASE
	 *	
	 *	Describes a client's active connection to a database.
	 *	
	 *----------------------------------------------------------*/

#define GDB_DB_ID 0x44420000			/* eye catcher */

struct db_struct {
	long id;				/* eye catcher */
	CONNECTION connection;			/* the GDB connection */
						/* used to get at this */
						/* database */
	int status;				/* status of this */
						/* database connection */
#define DB_OPEN 1				/* database opened */
						/* successfully */
#define DB_CLOSED 2				/* not open */
	char *name;				/* pointer to string name */
						/* of the database, for */
						/* debugging */
	char *server;				/* pointer to the i.d. */
						/* of the server, for */
						/* debugging */
};

typedef struct db_struct *DATABASE;

	/*----------------------------------------------------------
	 *	
	 *	Return codes from DB operations
	 *	
	 *----------------------------------------------------------*/

#define DB_NO_SUCH_OP 3

	/*----------------------------------------------------------
	 *	
	 *	Parameters which limit sizes of things
	 *	
	 *----------------------------------------------------------*/

#define GDB_MAX_QUERY_SIZE 2048			/* length of the longest */
						/* substituted query */
						/* string we can make */
#define GDB_MAX_QUERY_FIELDS 100		/* maximum number of fields */
						/* we can retrieve in one */
						/* query */
#define GDB_SIZE_OF_INGRES_TEXT 2001		/* number of chars in */
						/* largest ingres text */
						/* field */
#define GDB_MAX_RETRIEVED_TEXT_FIELDS 60	/* maximum number of text */
						/* type fields we can */
						/* retrieve in a single */
						/* query.  we hold this */
						/* down because stack space */
						/* is taken for max size of */
						/* each during query. */

	/*----------------------------------------------------------
	 *	
	 *	Return codes from database operations
	 *	
	 *----------------------------------------------------------*/

#define DB_PARSE_FAIL (-3)			/* couldn't parse */
						/* the request string*/


/************************************************************************
 *	
 *			DATABASE OPERATIONS IMPLEMENTED
 *			        AS MACROS
 *	
 ************************************************************************/

	/*----------------------------------------------------------
	 *	
	 *			DB_STATUS
	 *	
	 *----------------------------------------------------------*/

#define DB_STATUS(dbhandle) ((dbhandle)->status)




/************************************************************************
 *	
 *			STRING MANAGEMENT
 *	
 *	To allow dynamic manipulation of strings in gdb without
 *	excessive memory re-allocation, we define a string as a
 *	counted byte space.  Though this space will frequently be used
 *	to store a standard null terminated string, that is not
 *	required.
 *	
 *	Current representation for a string is a pointer followed by
 *	an integer length.  A null pointer indicates a null string, in
 *	which case the length is arbitrary.  Any other pointer is to
 *	memory which was allocated by db_alloc in which must be free'd
 *	eventually with db_free.
 *	
 ************************************************************************/

typedef struct str_dat {
	char *ptr;				/* pointer to the data */
	int length;				/* length of the allocated */
						/* memory (not necessarily */
						/* length of null-term'd */
						/* string stored there) */
} STRING;

	/*----------------------------------------------------------
	 *	
	 *	Macros for manipulating strings.  These return
	 *	the actual data from the string and the size of
	 *	the data space respectively.  To get the length of
	 *	the null terminated string which might be stored 
	 *	there, use strlen(STRING_DATA(string)).
	 *	
	 *----------------------------------------------------------*/


#define STRING_DATA(str) ((str).ptr)
#define MAX_STRING_SIZE(str) ((str).length)


/************************************************************************
 *	
 *			MEMORY ALLOCATION
 *	
 *	db_alloc and db_free are the externally visible names of
 *	the memory allocation services.  These actually call the 
 *	routines pointed to by the vectors gdb_amv and gdb_fmv, which
 *	default to the supplied routines gdb_am and gdb_fm.  Users
 *	may supply their own memory allocation by storing over the
 *	vectors.  This may be done prior to calling gdb_init to 
 *	insure that all dynamic memory is controlled by the user.
 *	
 ************************************************************************/

#define db_alloc (*gdb_amv)
#define db_free  (*gdb_fmv)

extern char *gdb_am();
extern int  gdb_fm();

extern char *((*gdb_amv)());
extern int  (*gdb_fmv)();


/************************************************************************
 *	
 *	      STRUCTURED DATA ROUTINES IMPLEMENTED AS MACROS
 *	
 ************************************************************************/

	/*----------------------------------------------------------
	 *	
	 *	             ADD_TUPLE_TO_RELATION
	 *	
	 *----------------------------------------------------------*/

#define ADD_TUPLE_TO_RELATION(relation, tuple) 			\
	{							\
                (tuple)->prev    = (relation)->last;            \
		(tuple)->next    = (TUPLE)(relation);      	\
		(relation)->last = tuple;			\
		(tuple)->prev->next = tuple;                    \
        }

	/*----------------------------------------------------------
	 *	
	 *	             ADD_TUPLE_AFTER_TUPLE
	 *	
	 *----------------------------------------------------------*/

#define ADD_TUPLE_AFTER_TUPLE(relation, tuple, prev_tuple)	\
	{								\
                (tuple)->prev    = (prev_tuple)->next->prev;            \
		(tuple)->next    = (prev_tuple)->next; 			\
		(tuple)->next->prev = tuple;				\
		(prev_tuple)->next = tuple;                    		\
        }

	/*----------------------------------------------------------
	 *	
	 *	          REMOVE_TUPLE_FROM_RELATION
	 *	
	 *----------------------------------------------------------*/

#define REMOVE_TUPLE_FROM_RELATION(relation, tuple) 			\
	{								\
		(tuple)->prev->next = (tuple)->next;			\
		(tuple)->next->prev = (tuple)->prev;			\
        }


	/*----------------------------------------------------------
	 *	
	 *	           DESCRIPTOR_FROM_TUPLE
	 *	
	 *----------------------------------------------------------*/

#define DESCRIPTOR_FROM_TUPLE(tuple) ((tuple)->desc)

	/*----------------------------------------------------------
	 *	
	 *	           DESCRIPTOR_FROM_RELATION
	 *	
	 *----------------------------------------------------------*/

#define DESCRIPTOR_FROM_RELATION(relation) ((relation)->desc)

	/*----------------------------------------------------------
	 *	
	 *		   REFERENCE_TUPLE_DESCRIPTOR
	 *	
	 *	Bumps the reference count for a tuple descriptor.
	 *	Intended only for internal use of GDB.
	 *	
	 *----------------------------------------------------------*/

#define REFERENCE_TUPLE_DESCRIPTOR(tpd) (++((tpd)->ref_count))

	/*----------------------------------------------------------
	 *	
	 *		   UNREFERENCE_TUPLE_DESCRIPTOR
	 *	
	 *	Decrements the reference count for a tuple descriptor.
	 *	Intended only for internal use of GDB.  Warning: it
	 *	is the user's responsibility to use delete_tuple_descriptor
	 *	instead of this macro in any situation in which the
	 *	reference count might go to zero.
	 *	
	 *----------------------------------------------------------*/

#define UNREFERENCE_TUPLE_DESCRIPTOR(tpd) (--((tpd)->ref_count))

	/*----------------------------------------------------------
	 *	
	 *	                 FIELD_FROM_TUPLE
	 *	
	 *----------------------------------------------------------*/

#define FIELD_FROM_TUPLE(tuple, field_index)				\
	(((tuple)->data) + (((tuple)->desc)->var[field_index].offset))

	/*----------------------------------------------------------
	 *	
	 *	                FIELD_OFFSET_IN_TUPLE
	 *	
	 *----------------------------------------------------------*/

#define FIELD_OFFSET_IN_TUPLE(tuple_descriptor, field_index)		\
	((tuple_descriptor)->var[field_index].offset)

	/*----------------------------------------------------------
	 *	
	 *	                FIELD_TYPE_IN_TUPLE
	 *	
	 *----------------------------------------------------------*/

#define FIELD_TYPE_IN_TUPLE(tuple_descriptor, field_index)		\
	((tuple_descriptor)->var[field_index].type)

	/*----------------------------------------------------------
	 *	
	 *	              FIRST_TUPLE_IN_RELATION
	 *	
	 *----------------------------------------------------------*/

#define FIRST_TUPLE_IN_RELATION(relation)				\
	(((relation)->first) == (TUPLE)relation ? NULL : (relation)-> first)

	/*----------------------------------------------------------
	 *	
	 *			NEXT_TUPLE_IN_RELATION
	 *	
	 *----------------------------------------------------------*/

#define NEXT_TUPLE_IN_RELATION(relation, prev)				\
	(((prev)->next) == (TUPLE)relation ? NULL : prev->next )

	/*----------------------------------------------------------
	 *	
	 *			PREV_TUPLE_IN_RELATION
	 *	
	 *----------------------------------------------------------*/

#define PREV_TUPLE_IN_RELATION(relation, next)				\
	(((next)->prev) == (TUPLE) relation ? NULL : next->prev)




/************************************************************************
 *	
 *	TRANSPORT and OPERATION SERVICES IMPLEMENTED AS MACROS
 *	
 ************************************************************************/

	/*----------------------------------------------------------
	 *	
	 *	             OPERATION_FROM_DATA
	 *	
	 *	Given OPERATION_DATA, return the corresponding handle
	 *	of type OPERATION.  Currently, OPERATION is just
	 *	implemented as a pointer to OPERATION_DATA.
	 *	
	 *----------------------------------------------------------*/

#define OPERATION_FROM_DATA(op_data)					\
	((OPERATION)&(op_data))

	/*----------------------------------------------------------
	 *	
	 *	                 OP_TAG
	 *	
	 *	Return the tag for a given operation.
	 *	
	 *----------------------------------------------------------*/

#define OP_TAG(operation) ((operation)->tag)

	/*----------------------------------------------------------
	 *	
	 *		       OP_STATUS
	 *	
	 *	Return the status of a given operation.  Note that
	 *	status describes an operations progress through 
	 *	execution.  It has the same values for all operations.
	 *	Result describes the final outcome of an operation.
	 *	It's values depend on the particular operation which
	 *	was attempted.  
	 *	
	 *----------------------------------------------------------*/

#define OP_STATUS(operation) ((operation)->status)

	/*----------------------------------------------------------
	 *	
	 *			OP_DONE
	 *	
	 *	True iff the operation is either OP_COMPLETE or
	 *	OP_CANCELLED.
	 *	
	 *----------------------------------------------------------*/

#define OP_DONE(op) ((op)->status == OP_COMPLETE || (op)->status == OP_CANCELLED)

	/*----------------------------------------------------------
	 *	
	 *		       OP_RESULT
	 *	
	 *	Return the result of a given operation.  Note that
	 *	status describes an operations progress through 
	 *	execution.  It has the same values for all operations.
	 *	Result describes the final outcome of an operation.
	 *	It's values depend on the particular operation which
	 *	was attempted. The special result value -1 is used 
	 *	to indicate an invalid value for result.  Generally,
	 *	-1 is returned when result is accidently queried at
	 *	a time when OP_STATUS != OPERATION_COMPLETE.
	 *	
	 *----------------------------------------------------------*/

#define OP_RESULT(operation) ((operation)->result)


/************************************************************************
 *	
 *			Debugging Interfaces
 *	
 ************************************************************************/

	/*----------------------------------------------------------
	 *	
	 *	The following operation codes my be passed to 
	 *	gdb_debug to set special debugging modes of operation.
	 *	
	 *	Note that most of these are toggles
	 *	
	 *----------------------------------------------------------*/

#define GDB_LOG 	0x00000001		/* turn on tracing to */
						/* log file */
#define GDB_NOFORK 	0x00000002		/* don't fork forking */
						/* servers */


/************************************************************************
 *	
 *	Things which have to be at the end because they require
 *	the various types to be defined first.
 *	
 ************************************************************************/


#ifdef DUMB_7_CHAR_LOADER
	/*----------------------------------------------------------
	 *	
	 *	          Long Names for Routines
	 *	
	 *	External names in Unix must generally be unique
	 *	within the first 7 characters or so, at least for
	 *	some versions of ld.  To account for this without
	 *	making all our routine names terribly short and
	 *	cryptic, we use the following defines.
	 *	
	 *----------------------------------------------------------*/

#define string_alloc 			gdb_sta
#define string_free 			gdb_stf

#define create_tuple_descriptor 	gdb_ctd
#define delete_tuple_descriptor 	gdb_dtd
#define field_index			gdb_fi
#define create_tuple 			gdb_ctup
#define delete_tuple			gdb_dtup
#define initialize_tuple		gdb_itup
#define null_tuple_strings		gdb_ntps

#define create_relation			gdb_crel
#define delete_relation			gdb_drel
#define tuples_in_relation              gdb_trel


/*
 * Transport layer
 */
#define create_operation		gdb_crop
#define delete_operation		gdb_dop
#define initialize_operation	        gdb_inop
#define reset_operation			gdb_rsop
#define cancel_operation		gdb_cnop

#define create_list_of_operations	gdb_clop
#define delete_list_of_operations	gdb_dlop

#define op_select			gdb_opsel
#define op_select_any			gdb_opsel
#define op_select_all			gdb_aopsel
#define con_select			gdb_cnsel

#define gdb_receive_data 		gdb_rcdat        
#define gdb_send_data 			gdb_sndat        
#define gdb_start_listening		gdb_stl
#define start_accepting_client		gdb_stac


#define gdb_listen			gdb_lis

/*
 * Connection management
 */
#define start_peer_connection		gdb_spconn
#define sever_connection		gdb_svconn
#define start_server_connection		gdb_stsrv
#define create_listening_connection	gdb_clc
#define start_replying_to_client	gdb_strtc
#define create_forking_server		gdb_cfs


/*
 * Asynchronous operations
 */
#define start_sending_object 		gdb_snobj
#define start_receiving_object 		gdb_rcobj
#define preempt_and_start_receiving_object 		gdb_prcobj

#define queue_operation			gdb_qop

#define requeue_operation		g_req_op

#define complete_operation		gdb_cmpo
/*
 * Synchronous operations
 */
#define send_object			gdb_sobj
#define receive_object			gdb_robj
/*
 * Database operations
 */
#define access_db 			gdb_adb
#define start_accessing_db 		gdb_sadb
#define perform_db_operation		gdb_pdbo
#define db_query			gdb_dbq
#define start_performing_db_operation	gdb_spdb
#define   start_db_query			gdb_sdbq
#else
#define op_select_any  op_select
#endif /* DUMB_7_CHAR_LOADER */

extern char *string_alloc();
extern int   string_free();
extern TUPLE_DESCRIPTOR create_tuple_descriptor();
extern int  delete_tuple_descriptor();
extern int  field_index();
extern TUPLE create_tuple();
extern int   delete_tuple();
extern int   initialize_tuple();
extern int   null_tuple_strings();
extern RELATION create_relation();
extern int delete_relation();
extern OPERATION create_operation();
extern LIST_OF_OPERATIONS create_list_of_operations();
extern OPERATION g_op_newhead();
extern CONNECTION start_peer_connection();
extern CONNECTION sever_connection();
extern CONNECTION start_server_connection();
extern CONNECTION create_listening_connection();
extern CONNECTION create_forking_server();
extern int start_sending_object();
extern int start_receiving_object();
extern int preempt_and_start_receiving_object();
extern int queue_operation();
extern int requeue_operation();
extern int complete_operation();
extern int send_object();
extern int receive_object();


	/*----------------------------------------------------------
	 *	
	 *	Other routines requiring extern to avoid forward
	 *	reference to non integer type functions.
	 *	
	 *----------------------------------------------------------*/

extern CONNECTION g_make_con();
