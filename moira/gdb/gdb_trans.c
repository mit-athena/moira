/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/gdb_trans.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/gdb_trans.c,v 1.4 1997-01-29 23:16:53 danw Exp $
 */

#ifndef lint
static char *rcsid_gdb_trans_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/gdb_trans.c,v 1.4 1997-01-29 23:16:53 danw Exp $";
#endif

/************************************************************************/
/*	
/*			   gdb_trans.c
/*	
/*	      GDB - Data Transport Services Routines (Part 1)
/*	
/*	Author: Noah Mendelsohn
/*	Copyright: 1986 MIT Project Athena 
/*		For copying and distribution information, please see
/*	  	the file <mit-copyright.h>.
/*	
/*	These routines implement layer 6 of the Client Library
/*	Specification of the GDB system, as well as the facilities
/*	outlined in the GDB Protocol Specification. (Part 1 of 2)
/*
/*	Some of the routines specified are actually implemented as
/*	macros defined in gdb.h.
/*	
/************************************************************************/

#include <mit-copyright.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <varargs.h>
#include <errno.h>
extern int errno;
#include "gdb.h"


/************************************************************************/
/*	
/*			OPERATION Manipulation
/*	
/*	The routines in this section provide services for creating
/*	and manipulating GDB queueable operations.  
/*	
/************************************************************************/

	/*----------------------------------------------------------*/
	/*	
	/*		   create_operation (create_operation)
	/*	
	/*	Allocates space for an operation descriptor and 
	/*	returns a pointer to it.  Initialize the eyecatcher
	/*	so that in the future we can prove by inspection that
	/*	this really is an operation descriptor.
	/*	
	/*----------------------------------------------------------*/

OPERATION
create_operation()
{
	register OPERATION op;
	GDB_INIT_CHECK
	op = (OPERATION)db_alloc(sizeof(OPERATION_DATA));
	op->status = OP_NOT_STARTED;		/* things like */
						/* reset_operation */
						/* want valid status */
						/* at all times*/
	op->id = GDB_OP_ID;
	return op;
}

	/*----------------------------------------------------------*/
	/*	
	/*		   reset_operation (reset_operation)
	/*	
	/*	Sets the status of an operation to OP_NOT_STARTED
	/*	
	/*----------------------------------------------------------*/

int
reset_operation(op)
OPERATION op;
{
	register int status;

	GDB_CHECK_OP(op, "reset_operation")

	status = OP_STATUS(op);
	if (status == OP_QUEUED || 
	    status == OP_RUNNING)
		GDB_GIVEUP("gdb:reset_operation invalid on running operation")
	op->status = OP_NOT_STARTED;
}

	/*----------------------------------------------------------*/
	/*	
	/*		  delete_operation(delete_operation)
	/*	
	/*	Frees an operation descriptor.
	/*	
	/*----------------------------------------------------------*/

int
delete_operation(op)
OPERATION op;
{
	GDB_CHECK_OP(op, "delete_operation");
	op->id = -1;
	db_free((char *)op, sizeof(OPERATION_DATA));
}

	/*----------------------------------------------------------*/
	/*	
	/*		  initialize_operation(initialize_operation)
	/*	
	/*	Initialize the data in an operation descriptor
	/*	based on the supplied parameters.
	/*	
	/*----------------------------------------------------------*/

int
initialize_operation(operation, init_function, arg, cancel_function)
OPERATION operation;				/* initialize this */
int (*init_function)();				/* function to call when */
						/* starting operation */
char *arg;					/* arg to pass to init and */
						/* continue routines */
int (*cancel_function)();			/* call this function */
						/* when cancelling the op. */
						/* may be NULL, may not */
						/* be elided */
{
	register OPERATION op = operation;	/* just for speed */

	GDB_CHECK_OP(op, "initialize_operation")


       /*
        * Fill in boilerplate - same for all newly initialized ops.
        */
	op->next = op->prev = NULL;		/* it's not in a queue yet */
	op->tag = (-1);				/* not tagged until queued */
	op->status = OP_NOT_STARTED;
	op->flags = 0;
	op->result = (-1);			/* result is not valid until */
						/* status is OP_COMPLETE. */
						/* All users of result field */
						/* are expected to treat -1 */
						/* invalid */
	op->halfcon = NULL;			/* not queued yet */

       /*
        * Fill in supplied parameters
        */
	op->arg = arg;
	op->fcn.init = init_function;
	op->cancel = cancel_function;

	return;
}

	/*----------------------------------------------------------*/
	/*	
	/*	    create_list_of_operations (create_list_of_operations)
	/*	
	/*	Allocates and fills in a data structure which 
	/*	contains a list of operations.  This list is
	/*	suitable for passing to op_select.
	/*	
	/*	The facilities of <varargs.h> are used to parse 
	/*	the variable length parameter list to this function.
	/*	See "man 3 varargs" for details.
	/*	
	/*----------------------------------------------------------*/

/*VARARGS1*/

LIST_OF_OPERATIONS
create_list_of_operations(count, va_alist)
int	count;
va_dcl
{
	va_list ap;				/* the control structure */
						/* for varargs parsing */

	register int i;
	register LIST_OF_OPERATIONS newlist;	/* newly allocated list */

       /*
        * Allocate the new list of operations, initialize its count.
        */
	newlist = (LIST_OF_OPERATIONS)
	           db_alloc(size_of_list_of_operations(count));
	newlist->count = count;

       /*
        * Using the facilities of varargs.h, pick up the operations
        * and put them in the list.
        */
	va_start(ap);
	for (i=0; i<count; i++) {
		newlist->op[i] = va_arg(ap, OPERATION);
		GDB_CHECK_OP((newlist->op[i]), "create_list_of_operations")
	}
	va_end(ap);

	return newlist;
}

	/*----------------------------------------------------------*/
	/*	
	/*	    delete_list_of_operations(delete_list_of_operations)
	/*	
	/*	Deallocate the space for a list of operations.
	/*	
	/*----------------------------------------------------------*/

int
delete_list_of_operations(op_list)
LIST_OF_OPERATIONS op_list;
{
	db_free((char *)op_list, size_of_list_of_operations(op_list->count));
}

/************************************************************************/
/*	
/*			OPERATION QUEUE MANIPULATION
/*	
/*	Routines to queue (initiate) and track completion of GDB
/*	operations.
/*	
/************************************************************************/

	/*----------------------------------------------------------*/
	/*	
	/*			op_select(op_select)
	/*	
	/*	This function is an analog of the standard Berkeley
	/*	select system call.  It provides all the functions
	/*	of select, but in addition, it awaits completion
	/*	of a specified list of queued gdb operations.  
	/*	
	/*	This function returns when any combination of the
	/*	following are found to be true:
	/*	
	/*	1) Any of the designated operations has completed
	/*	   or terminated prematurely.
	/*	
	/*	2) Activity is detected on any of the explictly supplied
	/*	   file descriptors or the supplied timer has expired.
	/*	
	/*	The count returned is only for file descriptors 
	/*	explicitly listed.  Completed operatons may be detected
	/*	by checking OP_STATUS for each of the operations in
	/*	the list.  File descriptors controlled by CONNECTIONS
	/*	should never be passed explictly in the bit maps to
	/*	this routine.  Only user controlled file descriptors 
	/*	may be explictly selected.
	/*	
	/*	Return code summary:
	/*		
	/*		-2	One or more listed operations completed.
	/*			Timer did not ring (as far as we could
	/*			tell), other fd's did not complete, or
	/*			we didn't get far enough to bother 
	/*			looking. (see fairness below.)
	/*	
	/*		-1      An error was returned on select of a 
	/*			user supplied socket.  errno has the
	/*			error code.
	/*	
	/*		0	The timer rang.  Some operations may
	/*			be complete, but it's unlikely.
	/*	
	/*		>0	This many user supplied fd's were
	/*			satisfied--same as for select.
	/*			Operations in the list may also have
	/*			completed.
	/*	
	/*	Fairness is not guaranteed.  This routine tends to
	/*	favor activity on CONNECTIONs.  In particular, if
	/*	some operation(s) in the list can be completed 
	/*	synchronously, a real select is never done to 
	/*	check on the file descriptors.
	/*	
	/*----------------------------------------------------------*/

int
op_select(op_list, nfds, readfds, writefds, exceptfds, timeout)
LIST_OF_OPERATIONS op_list;
int	nfds;
fd_set  *readfds, *writefds, *exceptfds;
struct timeval *timeout;
{
	int rc;					/* return code holder */
	fd_set rfds, wfds, efds;		/* local copys of read */
						/* write, and exception */
						/* fd's */

       /*
        * Make sure that any activity which can be done immediately is
        * indeed done now--we may not have to wait at all.
        */
	(void) gdb_fastprogress();   /*<==FIX (check to make sure this is OK)*/

       /* 
        * Loop checking for termination conditions, and if none arise,
        * use con_select to make wait for one or more of the
        * connections to wake up and do the appropriate processing.
        */
	while (TRUE) {
               /*
                * If any of the queue operations have completed,
                * then just return now.
                */
		if (gdb_list_complete(op_list)) 
			return (-2);		/* no fd's satisfied here */

               /*
                * Use con_select to await all the appropriate events
                */
		g_fd_copy(nfds, readfds, &rfds);
		g_fd_copy(nfds, writefds, &wfds);
		g_fd_copy(nfds, exceptfds, &efds);

		rc = con_select(nfds, &rfds, &wfds, &efds, timeout);

               /*
                * At this point, either some of the supplied fd's have
                * been satisfied(rc>0), or the timer has rung (rc=0),
                * an error was returned from select on a user specified socket
                * (-1) or none of these (-2).  In any case, there may
                * have been progress on one of the connections, and
                * con_select will already have progressed as far as 
                * possible before returning.  Now, decide what to
                * do, given the return code.
                */
		if (rc>= (-1)) {
                       /*
                        * Return updated bit-maps to caller.
                        */
			g_fd_copy(nfds, &rfds, readfds);
			g_fd_copy(nfds, &wfds, writefds);
			g_fd_copy(nfds, &efds, exceptfds);
			return rc;
		}	
	}	
}

	/*----------------------------------------------------------*/
	/*	
	/*			op_select_all(op_select_all)
	/*	
	/*	Similar to op_select_any, but returns (-1) only
	/*	in the case that all of the designated descriptors
	/*	are OP_COMPLETE or OP_CANCELLED.
	/*	
	/*----------------------------------------------------------*/

int
op_select_all(op_list, nfds, readfds, writefds, exceptfds, timeout)
LIST_OF_OPERATIONS op_list;
int	nfds;
fd_set  *readfds, *writefds, *exceptfds;
struct timeval *timeout;
{
	register int i;
        int rc = -1;
	register int left = op_list->count;

       /*
        * take care of those which are already complete by decrementing
        * and reseting them
        */

	for (i=0; i<op_list->count; i++) {
		if (op_list->op[i]->status==OP_COMPLETE) {
			op_list->op[i]->flags |= OPF_MARKED_COMPLETE;
			op_list->op[i]->status = OP_MARKED;
			                        /* so op_select_any won't */
						/* fall right through*/
			
			left--;
			continue;
		}
		if (op_list->op[i]->status==OP_CANCELLED) {
			op_list->op[i]->flags |= OPF_MARKED_CANCELLED;
			op_list->op[i]->status = OP_MARKED;
			                        /* so op_select_any won't */
						/* fall right through*/
			
			left--;
		}
	}

	
       /*
        * As long as there are incomplete operations left in the list,
        * keep calling op_select_any
        */
	while (left) {
		rc = op_select(op_list, nfds, readfds, writefds, exceptfds, 
			       timeout);
		if (rc>=0)
			break;
		for (i=0; i<op_list->count; i++) {
			if (op_list->op[i]->status==OP_COMPLETE) {
				op_list->op[i]->flags |= OPF_MARKED_COMPLETE;
				op_list->op[i]->status = OP_MARKED;
						/* so op_select_any won't */
						/* fall right through*/
				
				left--;
				continue;
			}
			if (op_list->op[i]->status==OP_CANCELLED) {
				op_list->op[i]->flags |= OPF_MARKED_CANCELLED;
				op_list->op[i]->status = OP_MARKED;
						/* so op_select_any won't */
						/* fall right through*/
				
				left--;
			}
		}
	}

       /*
        * Clean up the marked operations and return
        */
	for (i=0; i<op_list->count; i++) {
		if (op_list->op[i]->status==OP_MARKED) {
			op_list->op[i]->status = (op_list->op[i]->flags & 
						  OPF_MARKED_COMPLETE) ?
						    OP_COMPLETE : 
						    OP_CANCELLED;
			op_list->op[i]->flags &= ~(OPF_MARKED_COMPLETE |
						  OPF_MARKED_CANCELLED);
		}
	}

	return rc;

}

/************************************************************************/
/*	
/*	Internal transport layer routines, not called directly
/*	from client library.
/*	
/************************************************************************/

	/*----------------------------------------------------------*/
	/*	
	/*		     gdb_list_complete
	/*	
	/*	Given a list of gdb operations, return TRUE if any
	/*	of them are complete, otherwise return FALSE.
	/*	
	/*----------------------------------------------------------*/

int
gdb_list_complete(op_list)
LIST_OF_OPERATIONS op_list;
{
	register int i;
	register int status;
	register LIST_OF_OPERATIONS oplist = op_list; /* for speed */
        int count = oplist -> count;

	for (i=0; i<count; i++) {
		status = OP_STATUS(oplist->op[i]);
		if (status == OP_COMPLETE ||
		    status == OP_CANCELLED)
			return TRUE;
	}

	return FALSE;		
}

	/*----------------------------------------------------------*/
	/*	
	/*			  g_fd_copy
	/*	
	/*	Copy one set of fd masks to another. 
	/*	
	/*----------------------------------------------------------*/

g_fd_copy(nfds, source, targ)
int nfds;
fd_set *source, *targ;
{
	register int i;
	register int n = howmany(nfds, NFDBITS); /* number of words for */
						 /* this many fd's */

	for (i=0; i<n; i++)
		targ->fds_bits[i] = source->fds_bits[i];
}
	/*----------------------------------------------------------*/
	/*	
	/*			  g_fd_or_and_copy
	/*	
	/*	Or two sets of file descriptor masks together and
	/*	copy to a third.  Parameters are:
	/*	
	/*	1) First mask count
	/*	2) Second mask count
	/*      3) pointer to first mask
	/*	4) Pointer to second mask
	/*	5) Pointer to output mask
	/*	
	/*----------------------------------------------------------*/

g_fd_or_and_copy(nfds1, nfds2 , source1, source2, targ)
int nfds1, nfds2;
fd_set *source1, *source2, *targ;
{
	register int i;
	fd_set *longer;				/* points to whichever */
						/* of the two masks is */
						/* longer */
	int tot_words = howmany(max(nfds1,nfds2), NFDBITS);
	int shared_words = howmany(min(nfds1,nfds2), NFDBITS);


       /*
        * For words which exist in both masks, or the bits together
        */
	for (i=0; i<shared_words; i++)
		targ->fds_bits[i] = source1->fds_bits[i] | 
		                    source2->fds_bits[i];
       /*
        * Copy the rest of whichever is longer
        */
	longer = (nfds1>nfds2) ? source1 : source2;
	
	while (i<tot_words) {
		targ->fds_bits[i] = longer->fds_bits[i];
		i++;
	}

	return;
}

	/*----------------------------------------------------------*/
	/*	
	/*			  g_compute_return_fds
	/*	
	/*	When con_select returns, the count it gives back 
	/*	applies only to user supplied fd's, not those 
	/*	relating to connections.  Unfortunately, select
	/*	gives back both in the masks it returns.  Here we
	/*	fix only the count to reflect the fact that we will be
	/*	turning off bits when and if they are returned.
	/*	
	/*	We can assume here that the masks provided are as long
	/*	as merged_nfds, which means they are at least as long
	/*	as gdb_mfd.
	/*	
	/*----------------------------------------------------------*/

int
g_compute_return_fds(select_count, rfds, wfds, efds)
int select_count;				/* count of 1 bits as */
						/* returned from select */
fd_set *rfds, *wfds, *efds;			/* read, write, and except */
						/* maps as returned from */
						/* select */
{
	register int i;
	register int return_count = select_count; /* the value we'll return */
	register int words;			/* number of words in each */
						/* of the connection masks */


       /*
        * Since we can only decrement the count, there's no sense doing
        * any work if it's already 0;
        */
	if (return_count == 0)
		return 0;
       /*
        * Figure out how many words we have to look at to get all the
        * bits covered by connection masks.
        */
	words = howmany(gdb_mfd, NFDBITS);

       /*
        * For words which are involved in the connection masks, check
        * for matches and decrement the count accordingly.  Stop when
        * the count hits 0 or we're out of words.
        */
	for (i=0; i<words && (return_count>0) ; i++) {
		return_count -= g_bitcount((unsigned int)
					   (gdb_crfds.fds_bits[i] & 
					    rfds->fds_bits[i])); 
		return_count -= g_bitcount((unsigned int)
					   (gdb_cwfds.fds_bits[i] & 
					    wfds->fds_bits[i])); 
		return_count -= g_bitcount((unsigned int)
					   (gdb_cefds.fds_bits[i] & 
					    efds->fds_bits[i])); 
	}

	return return_count;
}

	/*----------------------------------------------------------*/
	/*	
	/*			  g_fd_reset_conbits
	/*	
	/*	Given a user supplied fd bit mask and a connection
	/*	related bit mask, turn off any of the connection related
	/*	bits in the user mask, and copy the result to a supplied
	/*	target.
	/*	
	/*	1) User mask count
	/*	2) Connection mask count
	/*      3) Pointer to user mask
	/*	4) Pointer to connection mask
	/*	5) Pointer to output mask
	/*	
	/*	Output is always the same length as the user mask.
	/*	
	/*----------------------------------------------------------*/


g_fd_reset_conbits(nfds, con_nfds , source, conbits, targ)
int nfds, con_nfds;
fd_set *source, *conbits, *targ;
{
	register int i;
	register int tot_words = howmany(nfds, NFDBITS); /* this rtn never*/
						/* returns a mask longer */
						/* than nfds */
	register int shared_words = howmany(min(nfds, con_nfds), NFDBITS);


       /*
        * For words which exist in both masks, turn off bits from conmask
        */
	for (i=0; i<shared_words; i++)
		targ->fds_bits[i] = source->fds_bits[i] & 
		                    ~(conbits->fds_bits[i]);
       /*
        * Copy the rest of source, if any
        */
	if (tot_words > shared_words)
		while (i<tot_words) {
			targ->fds_bits[i] = source->fds_bits[i];
			i++;
		}

	return;
}


/************************************************************************/
/*	
/*			CONNECTION MANIPULATION
/*	
/*	Routines to control data transmission on gdb connections.
/*	
/************************************************************************/

	/*----------------------------------------------------------*/
	/*	
	/*		 con_select (con_select)
	/*	
	/*	This operation has exactly the same semantics as the select
	/*	system call, except that (1) it implicitly selects all file
	/*	descriptors controlled by connections, as well as those
	/*	explictly specified and (2) it allows transmission and
	/*	receipt to progress on all connections and (3) it considers
	/*	a connection to be selected iff a transmission operation
	/*	which had been pending becomes complete.  One may consider
	/*	that con_select turns the fd's controlled by sockets into
	/*	packet streams rather than byte streams.  Note also that
	/*	this operation differs from a traditional select and an
	/*	op_select in that it is not robust against waiting for
	/*	connections with pre-completed activity. This could be
	/*	added, but since it's an internal routine anyway, it seems
	/*	not to be worthwhile. Also, this routine presumes that all 
	/*	possible progress has been made before con_select is invoked.
	/*	
	/*	This operation hangs in a select.  If activity is
	/*	discovered on any of the sockets controlled by the database
	/*	library, then the corresponding input is read and
	/*	appropriate processing is done.  If any of the transmission
	/*	operations which had been pending on one of these
	/*	connections completes, then con_select may return.  In
	/*	fact, con_select attempts to complete any further
	/*	connection related activity which can be done without
	/*	blocking, but con_select never blocks AFTER a transmission
	/*	operation has gone complete.
	/*	
	/*	If activity is detected on any of the file descriptors
	/*	supplied by the user, then a count and bit fields are
	/*	returned just as for select. (Activity on database sockets
	/*	is never reflected in either count or bitfields.)  Timeout
	/*	causes a return, as with select.  Upon return, the program
	/*	must check for competion or termination on all of the
	/*	connections in which he/she is interested, for activity on
	/*	the selected file descriptors, and for timeouts, if
	/*	requested, since any or all of these may be reported
	/*	together.
	/*	
	/*	Return values for con_select: >0 same as for select, 0 time
	/*	expired, -1, error in select on user fd, -2, connections
	/*	have progressed but nothing else of interest.
	/*	
	/*	
	/*----------------------------------------------------------*/

int
con_select(nfds, readfds, writefds, exceptfds, timeout)
int	nfds;
fd_set  *readfds, *writefds, *exceptfds;
struct timeval *timeout;
{
	int merged_nfds;			/* the index of the last */
						/* file desc we care about.*/
	int select_fds;				/* number of file */
						/* returned from select */
						/* descriptors */
	int return_fds;				/* fds count to be returned */
						/* to the user */
	int complete_count;			/* number of connections on */
						/* which operations have */

						/* completed */

       /*
        * figure out highest number file descriptor to worry about
        */
	merged_nfds = max(nfds, gdb_mfd);	/* the number we control */
						/* or last one the user */
						/* supplied, whichevere is */
						/* higher */
       /*
        * Loop waiting for activity to occur and processing it as appropriate.
        * Note that the order of the tests, calls to select, and gdb_progress
        * in this code is critical.  Think it through before you change it
        * to make sure that progress is always made when possible, and that
        * returns are always made when needed.
        */
	while (TRUE) {
               /*
                * Prepare working copies of the file descriptor maps
                * based on the ones supplied by the caller, as well as
                * all the descriptors used by connections.  The latter
                * are mapped in the gdb_c?fds global variables.
                */
		g_fd_or_and_copy(nfds,gdb_mfd,readfds,&gdb_crfds,&last_crfds);
		g_fd_or_and_copy(nfds,gdb_mfd,writefds,&gdb_cwfds,&last_cwfds);
		g_fd_or_and_copy(nfds,gdb_mfd,exceptfds,&gdb_cefds,&last_cefds);
               /*
                * Use select to wait for something to happen.  Compute
                * number select would have returned if connection related
                * fd's had not been supplied.
                */
		select_fds = select(merged_nfds, &last_crfds, &last_cwfds, &last_cefds, 
				    timeout);
               /*
                * There are some messy things we have to worry about here:
                * 
                * 1) Select could return an error.  In particular, some
                *    versions of Unix will return -1 with EBADF if one
                *    of the sockets has closed.  We should call a 
                *    procedure here to see if this has happened to one
                *    of ours.  
                * 
                * 2) Other versions of Unix will claim that there is activity
                *    on our socket when in fact the other end is closed.  
                *    We will leave it to gdb_move_data to try the select
                *    and make the appropriate decision.
                * 
                * Yes folks, messy but true.  If we got an error (case 1),
                * then let's see if we get the same error when we're only
                * looking at the caller's fd's.
                * 
                */
		return_fds = 0;
		if (select_fds < 0)
			if ( errno != EBADF)
				return -1;
			else {
				g_fd_or_and_copy(nfds,0,readfds,&gdb_crfds,&last_crfds);
				g_fd_or_and_copy(nfds,0,writefds,&gdb_cwfds,&last_cwfds);
				g_fd_or_and_copy(nfds,0,exceptfds,&gdb_cefds,&last_cefds);
				if (select(nfds, &last_crfds, &last_cwfds, &last_cefds, 
					   &gdb_notime)<0) {
					g_fd_copy(nfds, &last_crfds, readfds);
					g_fd_copy(nfds, &last_cwfds, writefds);
					g_fd_copy(nfds, &last_cefds, exceptfds);
					return -1; /* select EBADF   */
				} else {
                                       /*
                                        * We should close the connection here
                                        */
					GDB_GIVEUP("con_select: EBADF on GDB controlled file.")
				}
			}
		else {
			return_fds = g_compute_return_fds(select_fds,
						    &last_crfds, &last_cwfds, &last_cefds);
		}
               /*
                * If some connection related descriptors were selected, then
                * try to make progress on them.  Find out if any new operations
                * could be completed.
                */
		if (select_fds != return_fds)
			complete_count = gdb_fastprogress();
		else
			complete_count = 0;
               /*
                * Now, based on the number of operations complete,
                * the number of user file descriptors satisfied, and
                * the possible return of a timeout from select, decide
                * whether to return to the caller.  Note that timeout
                * is identified by select_fds == 0.
                */
		if (complete_count > 0 ||	/* operations are complete */
		    return_fds >0 ||		/* user files satisfied */
		    select_fds ==0) {		/* timeout in select */
                       /*
                        * Connection related fd bits are never returned 
                        * to the caller.  Reset them.  The count of bits
                        * was already adjusted appropriately above.  I don't
                        * think there's too much wasted effort in this, 
                        * but we should watch it if profiling indicates
                        * lots of time being spent computing in con_select.
                        * Put the updated bit masks in the caller supplied
                        * maps.
                        */
			g_fd_reset_conbits(nfds,gdb_mfd,&last_crfds,&gdb_crfds,
					   readfds);
			g_fd_reset_conbits(nfds,gdb_mfd,&last_cwfds,&gdb_cwfds,
					   writefds);
			g_fd_reset_conbits(nfds,gdb_mfd,&last_cefds,&gdb_cefds,
					   exceptfds);
			if (select_fds ==0)
			        return 0;	/* real timeout */
			if (return_fds ==0)	/* something must have */
						/* completed, but our */
						/* count looks like a */
						/* timeout*/
				return (-2);	/* only connections have */
						/* somethine complete */
			else
				return return_fds;
		}
	}
}
