/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/gdb_trans2.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/gdb_trans2.c,v 1.3 1993-10-22 14:38:37 mar Exp $
 */

#ifndef lint
static char *rcsid_gdb_trans2_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/gdb_trans2.c,v 1.3 1993-10-22 14:38:37 mar Exp $";
#endif	lint


/************************************************************************
 *	
 *			   gdb_trans2.c
 *	
 *	      GDB - Data Transport Services Routines (Part 2)
 *	
 *	Author: Noah Mendelsohn
 *	Copyright: 1986 MIT Project Athena 
 *		For copying and distribution information, please see
 *	  	the file <mit-copyright.h>.
 *	
 *	
 *	These routines implement layer 6 of the Client Library
 *	Specification of the GDB system, as well as the facilities
 *	outlined in the GDB Protocol Specification.  Part 2 of 2.
 *
 *	Some of the routines specified are actually implemented as
 *	macros defined in gdb.h.
 *	
 ************************************************************************/

#include <mit-copyright.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include "gdb.h"
#include <sys/uio.h>
#include <sys/socket.h>
#include <string.h>
extern int errno;				/* Unix error slot */

/*
 * The following values are returned by g_con_progress
 */
#define NOPROGRESS 0				/* nothing happened on this */
						/* connection--must be 0*/
#define PROGRESS 1				/* connection has progressed */
#define COMPLETE 2				/* an operation has */
						/* completed on this con */

/************************************************************************/
/*	
/*		             queue_operation(queue_operation)
/*	
/*	Add an operation to the queue for a given connection, and
/*	then allows all connections to progress.  Returns the last
/*	known status of the operation.
/*	
/************************************************************************/

int
queue_operation(con, direction, op)
CONNECTION con;
int     direction;
OPERATION op;
{
	register HALF_CONNECTION hcon = (direction==CON_INPUT)?(&(con->in)):
					                       (&(con->out));
	GDB_CHECK_CON(con, "queue_operation")
       /*
        * Write message to debugging log
        */
	if (gdb_Debug & GDB_LOG)
		fprintf(gdb_log, "op queued: con=0x%x dir=%s op=0x%x Q was %s empty\n",
			con, (direction == CON_INPUT)?"INPUT":"OUTPUT",
			op, (hcon->op_q_first == (OPERATION)hcon)?"":"not");
       /*
        * Make sure connection is up
        */
	if (con->status != CON_UP) {
		op->status = OP_CANCELLED;
		if (gdb_Debug & GDB_LOG)
			fprintf(gdb_log, "\nop NOT queued\n");
		return OP_CANCELLED;
	}

       /*
        * Put the new operation at the end of the queue
        */
	op->prev = hcon->op_q_last;
	op->next = (OPERATION)hcon;
	hcon->op_q_last->next = op;
	hcon->op_q_last = op;
       /*
        * Mark it as queued
        */
	op->status = OP_QUEUED;
	op->halfcon = hcon;

       /*
        * Force progress on this connection
        */
	(void) g_con_progress(con - gdb_cons);
       /*
        * Con_select with notime is used here as a kind of fudge for
        * doing a fastprogress with a select built in before it.
        */
	(void) con_select(0, (fd_set *)0, (fd_set *)0, (fd_set *)0,
		   &gdb_notime);/* XXX */
       /*
        * Return the last known status of the operation
        */
	return op->status;
}
/************************************************************************/
/*	
/*		             requeue_operation
/*	
/*	This routine may be called from an init or continuation routine
/*	to cause the current operation to be requeued on a new connection.
/*	The init routine field ofthe operation should be properly set to 
/*	indicate the routine to receive control when the operation actually
/*	runs on the new connection.  The caller of this routine is 
/*	responsible for returning the status OP_REQUEUED to its caller.
/*	
/*	This routine returns the status of the newly queued operation.
/*	Note, however, that even if this operation returns the status
/*	CANCELLED, the operation itself may not continue to execute
/*	on the old connection and it should return the status OP_REQUEUED,
/*	NOT OP_CANCELLED (at least in this implementation.)
/*	
/************************************************************************/

int
requeue_operation(con, direction, op)
CONNECTION con;
int     direction;
OPERATION op;
{
       /*
        * Make sure the connection supplied is a legal one
        */
	GDB_CHECK_CON(con, "requeue_operation")
       /*
        * Write message to debugging log
        */
	if (gdb_Debug & GDB_LOG)
		fprintf(gdb_log, "op requeued: new con=0x%x dir=%s op=0x%x\n",
			con, (direction == CON_INPUT)?"INPUT":"OUTPUT",
			op);
       /*
        * Dequeue the operation from its old half connection
        */
	(void) g_op_newhead(op->halfcon);

       /*
        * Now queue it on the new one
        */
	return queue_operation(con, direction, op);
}

/************************************************************************/
/*	
/*			  g_preempt_me
/*	
/*	Sticks a new operation in ahead of the current one and runs it
/*	on the current connection.  May be called only from an init or
/*	continuation routine.  The old operation must have completely
/*	prepared the descriptor for the new operation, i.e. it should
/*	be in the same state as it would be for a call to queue_operation.
/*	g_preempt_me makes it possible for operations to be built by
/*	composition of other smaller operations, since newop runs, in
/*	a sense, as a subroutine of oldop.  opdop must (1) reset its
/*	initialization routine to be a routine to be called when newop
/*	completes or cancels and (2) return the status OP_PREEMPTED to
/*	its caller.
/*	
/************************************************************************/

int
g_preempt_me(oldop, newop)
OPERATION oldop;
OPERATION newop;
{
	register OPERATION old=oldop, new=newop;
	register HALF_CONNECTION hc = old->halfcon;

       /*
        * Write message to debugging log
        */
	if (gdb_Debug & GDB_LOG)
		fprintf(gdb_log, "op preempted: halfcon=0x%x oldop=0x%x newop=0x%x\n",
			oldop,newop);
       /*
        * link in the new operation
        */
	old->prev = new;
	hc->op_q_first = new;
	new->prev = (OPERATION)hc;
	new->next = old;
       /*
        * Set the status of the new operation
        */
	new->status = OP_QUEUED;
	new->halfcon = hc;
       /*
        * Change the status of the old operation (one could argue that
        * this should be done in gdb_hcon_progress after the return code
        * is detected.)
        */
	old->status = OP_QUEUED;
	return OP_QUEUED;
}



/************************************************************************/
/*	
/*			   gdb_progress
/*	
/*	This routine should be called whenever it is suspected that
/*	progress can be made on any connection.  This routine will
/*	cause all connections to proceed as far as they can without 
/*	blocking, and will make a best effort to avoid long blocks.
/*	This routine MAY retain control for long periods when sustained
/*	progress is possible, but it will not knowingly hang.  
/*	
/*	Returns: number of connections on which OPERATIONS have 
/*	COMPLETED (not just progressed).
/*	
/************************************************************************/

int

gdb_progress()
{
	register int i;				/* index to available */
						/* connections */
	register int return_value = 0;		/* the value we return */
	int rc;					/* short term storage for */
						/* a return code */
	int progress_made;			/* true when some con */
						/* made progress during */
						/* latest pass through list */
	int complete_map[GDB_MAX_CONNECTIONS];	/* indicates whether a */
						/* transmission operation */
						/* is newly complete on */
						/* corresponding connection */
						/* 1 if yes else 0  */
	int maxcon = gdb_mcons;			/* gdb_mcons may change */
						/* out from under us if */
						/* connections break.  This */
						/* is the initial value. */

       /*
        * Zero out the completion map for all connections.
        */
	for (i=0; i<maxcon; i++)
		complete_map[i]=0;

	 /*
        * Make repeated passes through all the fd's until a pass is made
        * in which none makes any progress.  This logic is important, 
        * because it catches the case where A is blocked, B makes progress,
        * and A unblocks during the period where B is progressing.
        */

	do {
		progress_made = FALSE;
		for (i=0; i<gdb_mcons; i++) {
			if (rc = g_con_progress(i)) { /* note: NOPROGRESS==0 */
				progress_made = TRUE;
				if (rc == COMPLETE)
					complete_map[i] = 1;
			}
		}
	} while (progress_made);

	 /*
        * We've gone as far as we can, now find out how many connections
        * have had operations complete.
        */
	for (i=0; i<maxcon; i++) 
		return_value += complete_map[i];

	return return_value;
}


/************************************************************************/
/*	
/*			gdb_fastprogress
/*	
/*	Similar to gdb_progress, but this routine attempts progress
/*	only on those connections which have already shown themselves
/*	to be ready for activity by a prior select.  This is safe to do
/*	when (1) the only activity we are interested in is that related
/*	to ongoing I/O and (2) a select was recently done to set the
/*	last_c.fds flags.  Condition (1) is violated in the case where
/*	an operation may be newly at the head of a queue and its init 
/*	routine may not have had a chance to run. Condition (2) is violated 
/*	when we are entering after having done significant computation.
/*	
/*	This routine was introduced by Bill Sommerfeld after profiling 
/*	revealed that unnecessary attempts to progress on quiescent
/*	sockets were causing excessive overhead in the system.  I am
/*	still suspicious that this routine may be getting called in
/*	places where a full gdb_progress is needed.  e.g. I'm not
/*	sure its use in op_select is entirely safe.
/*	
/************************************************************************/

gdb_fastprogress()   
{
	int i;
	int retval=0, rc;
	
	for (i=0; i<gdb_mcons; i++) {
		register CONNECTION con = &gdb_cons[i];
		register int infd = gdb_cons[i].in.fd;
		register int outfd = gdb_cons[i].out.fd;

		if(connection_status(con) != CON_UP)
			continue;

		gdb_conok = TRUE;
		if ((!(con->in.flags&HCON_UNUSED))&&
		    ((con->in.stream_buffer_remaining > 0)
		    || FD_ISSET(infd, &last_crfds))) {
			rc = gdb_hcon_progress(CON_INPUT, &con->in);
			if (!gdb_conok) {
				g_stop_with_errno(con);
				rc = COMPLETE;
			}
			if (rc == COMPLETE)
				retval++;
		}
		if ((!(con->out.flags&HCON_UNUSED))&&
		    (FD_ISSET(outfd, &last_cwfds))) {
			rc = gdb_hcon_progress(CON_OUTPUT, &con->out);
			if (!gdb_conok) {
				g_stop_with_errno(con);
				rc = COMPLETE;
			}
			if (rc == COMPLETE)
				retval++;
		}
	}
       /*
        * We've gone as far as we can, now find out how many connections
        * have had operations complete.
        */

	return retval;
}

/************************************************************************/
/*	
/*			   g_con_progress
/*	
/*	Make as much progress as possible on the specified connection.
/*	Returns NOPROGRESS if no bytes moved on either half connection,
/*	PROGRESS, if some moved and no operations completed, or COMPLETE if
/*	any of the operations completed.  Note that each connection
/*	consists of two half connections, and we must make each of them
/*	progress as far as possible.
/*	
/*	The nest here starts getting so deep that it's hard to pass state
/*	around efficiently.  We  use a single global variable, gdb_conok,
/*	to indicate whether the connection we're working on now has died.
/*	The move data routines set this to FALSE whenever there is a 
/*	fatal error on a connection.  We check it, and do a proper 
/*	sever on the connection if it seems to be in trouble.
/*	
/************************************************************************/


int
g_con_progress(con_id)
int	con_id;					/* index of this connection */
						/* in the connection desc. */
						/* arrays*/
{
	register CONNECTION con= (&gdb_cons[con_id]);
						/* pointer to the connection */
						/* data structure */
	register int progress = NOPROGRESS;
	register int live = TRUE;		/* true when we've seen */
						/* enough to make sure we */
						/* want to go around again*/
	int rc;
       /*
        * Check status of connection-if it's not running, then just return.
        */
	if (con->status != CON_UP)
		return NOPROGRESS;
       /*
        * Repeatedly make progress on each half connection until both
        * are idle.  Important to keep trying as one may become active
        * while the other is progressing.  
        */

	gdb_conok = TRUE;			/* this gets set to FALSE */
						/* for fatal I/O errors */
						/* there may be a timing */
						/* window here in use of */
						/* HCON_BUSY.  Also: it is */
						/* essential that errno */
						/* remain valid after conok */
						/* goes bad */
	while (live) {
		live = FALSE;			/* until proven otherwise */
               /*
                * make progress on the input connection note that following
                * logic depends on NOPROGRESS being 0
                */
		if (rc = gdb_hcon_progress(CON_INPUT, &con->in)) {
			live = TRUE;
			progress = max(rc, progress);
		}
               /*
                * See if connection has died
                */
		if (!gdb_conok) {
			g_stop_with_errno(con);
			return COMPLETE;	/* dying connection always */
						/* implies that the */
						/* operation at the head */
						/* of the queue completed */
		}
               /*
                * make progress on the output connection
                */
		if (rc = gdb_hcon_progress(CON_OUTPUT, &con->out)) {
			live = TRUE;
			progress = max(rc, progress);
		}
               /*
                * See if connection has died
                */
		if (!gdb_conok) {
			g_stop_with_errno(con);
			return COMPLETE;
		}
	}

	return progress;
}


/************************************************************************/
/*	
/*			gdb_hcon_progress
/*	
/*	Allows a specified half-connection to progress as much as possible,
/*	and returns true iff at least one operation is newly completed.
/*
/************************************************************************/

int
gdb_hcon_progress(direction, hc)
int	direction;				/* CON_INPUT or CON_OUTPUT */
struct  half_con_data *hc;			/* pointer to control struct */
						/* for this half connection */
{
        HALF_CONNECTION half_con = hc;
						/* half connection pointer */
						/* fast copy in register */
	register OPERATION op;			/* current operation on this */
						/* half connection */
	int progress = NOPROGRESS;		/* can indicate any progress */
						/* on the half con or */
						/* whether any operations */
						/* completed */
	int done;				/* true when no more progress*/
						/* can be made */
	int fcn_result;				/* result of latest init or */
						/* continue function */

       /*
        * Write message to debugging log
        */
	if (gdb_Debug & GDB_LOG)
		fprintf(gdb_log, "hcon_progress: halfcon=0x%x dir=%s ",
			half_con, (direction==CON_INPUT)?"INPUT":"OUTPUT");

	/*----------------------------------------------------------*/
	/*	
	/*	See if we are being re-entered and are already working
	/*	on this half_con.  If so, return right now.  
	/*	
	/*----------------------------------------------------------*/

	if (half_con->flags & HCON_BUSY) {
		/*
		 * Write message to debugging log
		 */
		if (gdb_Debug & GDB_LOG)
			fprintf(gdb_log, "BUSY, returning\n");
		return NOPROGRESS;
	}

	/*----------------------------------------------------------*/
	/*	
	/*	See if there is an operation on this half connection.
	/*	If not, return.
	/*	
	/*----------------------------------------------------------*/


	op = half_con->op_q_first;		/* pick up first operation */
						/* in queue */
	if (op == (OPERATION)half_con) {	/* see if end of circular */
						/* list */
		/*
		 * Write message to debugging log
		 */
		if (gdb_Debug & GDB_LOG)
			fprintf(gdb_log, "Q EMPTY, returning\n");
		return NOPROGRESS;		/* nothing to do on */
						/* this half session */
	}


	/*----------------------------------------------------------*/
	/*	
	/*	Loop until all operations are complete, or until no further
	/*	progress can be made on this one.
	/*	
	/*	Loop invariants:
	/*	
	/*	1) Op contains the operation at the head of the q, or
	/*	   else is == half_con, indicating no more operationos
	/*	   to be processed.
	/*	
	/*	2) The operation at the head of the queue is either running
	/*	   or continuing.  As soon as one completes, it is dequeued.
	/*	
	/*	Progress is declared whenever an operation newly
	/*	returns OP_COMPLETE, i.e. whenever there has been 
	/*	an operation which went from running to complete.
	/*	
	/*	Done is declared whenever an operation returns anything
	/*	other than complete, indicating that it cannot progress
	/*	further at this time.  Loop ends.  
	/*	
	/*	While we're here, mark us busy so we won't try the
	/*	same half_con on reentry.
	/*	
	/*----------------------------------------------------------*/

	done = FALSE;				/* this one may be able to */
						/* progress */

	half_con->flags |= HCON_BUSY;		/* don't try this hcon */
						/* while we already doing */
						/* it.  Could happen if */
						/* we queue new ops */
	half_con->flags &= ~HCON_PROGRESS;	/* gdb_move_data will */
						/* indicate progress here*/
	if (gdb_Debug & GDB_LOG)
		fprintf(gdb_log, "LOOPING\n");

	/*----------------------------------------------------------*/
	/*	
	/*	Loop through the operations queued on this half con
	/*	trying to make progress on them, in order.
	/*	
	/*----------------------------------------------------------*/

	while (!done &&
	       op != (OPERATION)half_con) {

	        if (gdb_Debug & GDB_LOG)
			fprintf(gdb_log, "\top=0x%x status%d...",
				op, OP_STATUS(op));

		switch (op->status) {
            /*
             * Operation is at head of queue for first time and has
             * never been started.  Try to start it up.
             */
	    case OP_QUEUED:
                       /*
                        * Call the initialization routine for this operation
                        */
			fcn_result = (*op->fcn.init)(op,half_con,op->arg);
			if (gdb_Debug & GDB_LOG)
				fprintf(gdb_log, "init result=%d\n",
					fcn_result);

			switch (fcn_result) {
			      case OP_COMPLETE:
			      case OP_CANCELLED:
				op->status = fcn_result; 
				op = g_op_newhead(half_con);
				progress = COMPLETE;
				break;
			      case OP_PREEMPTED:
				op->status = OP_QUEUED;
				/* fall thru */
			      case OP_REQUEUED:
				/* important: don't set status on re-queued */
				/* op as it may already have completed in */
				/* its second life ! */
				op = half_con->op_q_first;
				progress = max(progress, PROGRESS);
				break;
			      default:
				op->status = fcn_result; 
				done = TRUE;	/* could not get done */
			}
			break;
            /*
             * Operation is at head of queue and has already 
             * started trying to run.  The only reason we could be in this
             * state is that the last time we tried to do the requested input
             * or output, all the data could not be moved synchronously.  
             * We therefore try to move some more, and if it all goes now,
             * we call the continuation routine.
             */
	    case OP_RUNNING:
                       /*
                        * Try to move some more data.  If it won't all 
                        * go now, we're done with this half connection.
			*
			* If this is a special listening connection which
			* has an operation queued trying to do a listen,
			* then do the listen.  Otherwise do an ordinary
			* data move.
                        */
			if (half_con->flags & HCON_PENDING_LISTEN) {
				if (gdb_listen(half_con)==FALSE) {
					if (gdb_Debug & GDB_LOG)
						fprintf(gdb_log, "NO LISTEN\n");
					done = TRUE;
					break;
				}
			} else
				if (gdb_move_data(direction, half_con)==FALSE) {
					done = TRUE;
					if (gdb_Debug & GDB_LOG)
						fprintf(gdb_log, "NO DATA\n");
					break;
				}
                       /* 
                        * The pending data transmission has now completed.
                        * Call the continuation routine for this operation
                        */
			fcn_result = (*op->fcn.cont)(op,half_con,op->arg);
			if (gdb_Debug & GDB_LOG)
				fprintf(gdb_log, "cont result=%d\n",
					fcn_result);

			switch (fcn_result) {
			      case OP_COMPLETE:
			      case OP_CANCELLED:
				op->status = fcn_result; 
				op = g_op_newhead(half_con);
				progress = COMPLETE;
				break;
			      case OP_PREEMPTED:
				op->status = OP_QUEUED;
				/* fall thru */
			      case OP_REQUEUED:
				/* important: don't set status on re-queued */
				/* op as it may already have completed in */
				/* its second life ! */
				op = half_con->op_q_first;
				progress = max(progress, PROGRESS);
				break;
			      default:
				op->status = fcn_result; 
				done = TRUE;	/* could not get done */
			}
			break;
            /*
             * Following cases are all unexpected, at least for the
             * moment.  (See explanation of loop invariants for this while
             * loop.  Give up if they turn up.
             */
	    case OP_COMPLETE:
			GDB_GIVEUP("gdb_hcon_progress: found OP_COMPLETE on q")
	    case OP_CANCELLED:
			GDB_GIVEUP("gdb_hcon_progress: found OP_CANCELLED on q")
	    case OP_CANCELLING:
			GDB_GIVEUP("gdb_hcon_progress: OP_CANCELLING")
	    default:
			GDB_GIVEUP("gdb_hcon_progress: Operation is queued, but is not runnable")
		  }
	}

	if (progress == NOPROGRESS && (half_con->flags & HCON_PROGRESS))
		progress = PROGRESS;

	half_con->flags &= ~HCON_BUSY;

	if (gdb_Debug & GDB_LOG)
		fprintf(gdb_log, "hcon_progress: returns %d\n",progress);

	return progress;			/* NOPROGRESS, PROGRESS */
						/* or COMPLETE */
}

/************************************************************************/
/*	
/*				g_op_newhead
/*	
/*	Dequeues the operation at the head of the queue for the
/*	given half connection and returns the pointer to the 
/*	new head of the queue.  If the queue is null, then a pointer
/*	to the half_con itself is returned.  (The lists are
/*	linked circularly.)
/*	
/************************************************************************/

OPERATION
g_op_newhead(hcp)
HALF_CONNECTION hcp;
{
	register OPERATION newhead, oldhead;

       /*
        * Get old and new heads of chain
        */
	oldhead = hcp->op_q_first;
	newhead = oldhead->next;
       /*
        * Make sure nobody chained a bad one on us
        */
	if (newhead == NULL) {
		if (gdb_Debug & GDB_LOG) {
			fprintf(gdb_log,"\t\tg_op_newhead: found null link, oldhead = 0x%x newhead=0x%x halfcon=0x%x\n\t\t\t hc->first=0x%x hc->last=0x%x\n",
				oldhead, newhead, hcp, hcp->op_q_first,
				hcp->op_q_last);
		}
		GDB_GIVEUP("g_op_newhead: found NULL chain link")
	}
       /*
        * Remove oldhead from chain, fixing up chain pointers
        */
	newhead->prev = oldhead->prev;
	hcp->op_q_first = newhead;

       /*
        * Clean up pointers in the newly dequeued operation.  This is
        * just for cleanliness and ease of debugging.
        */
	oldhead->next = oldhead->prev = NULL;
	oldhead->halfcon = NULL;

	return newhead;
}

/************************************************************************/
/*	
/*	                        gdb_move_data
/*	
/*	This routine attempts to make further progress on the pending
/*	level transmission operation pending on this half connection.
/*	(Presumes that such an operation is pending.)  Returns TRUE
/*	if all the requested data has been moved, else FALSE.
/*	
/*	We assume here that all fd's are set to non-blocking I/O, so
/*	we can safely try reading and writing until they return 0 bytes.
/*	
/************************************************************************/

#define FIX_BUFFER_POINTERS(hc, count) if (count>0) {hc->next_byte += count; \
						     hc->remaining -= count;}
int
gdb_move_data(direction, hc)
int	direction;				/* CON_INPUT or CON_OUTPUT */
struct  half_con_data *hc;			/* pointer to control struct */
						/* for this half connection */
{
	register HALF_CONNECTION half_con = hc;
						/* half connection pointer */
						/* fast copy in register */
	register fd_set *fdbits;		/* the mask we should adjust */
						/* for this direction */

       /*
        * For safety, in case we're called when nothing is pending.
        */
	if (half_con->remaining == 0)
		return TRUE;
       /*
        * Move the data into the user's buffer.  In the case of input
        * data may come first from the stream buffer, then from the socket
        * itself.
        */
       if (direction == CON_INPUT) {
	        gdb_transfer_from_buffer(half_con);
	       /*
		* If remaining is greater than 0, then we emptied
		* the stream buffer and still weren't done.  Try
		* to read it from the pipe and re-fill the stream
		* buffer.
		*/
	       if (half_con->remaining) {
		       gdb_read_data_and_buffer(half_con);
	       }
       } else {
	       gdb_write_data(half_con);
       }
       /*
        * The file descriptor masks used for doing selects must be activated
        * when and only when there is a pending operation trying to use
        * the connection.  Update the masks for this half connection.
        */
	fdbits = (direction == CON_INPUT)? &gdb_crfds : &gdb_cwfds;
	if (half_con->remaining >0 && gdb_conok)
		FD_SET(half_con->fd, fdbits);
	else					
		FD_CLR(half_con->fd, fdbits);

	return (half_con->remaining == 0);
}

/************************************************************************/
/*	
/*			gdb_transfer_from_buffer
/*	
/*	Given an inbound half connection, satisfy as much as possible
/*	of desired data from the stream buffer.
/*	
/************************************************************************/

int
gdb_transfer_from_buffer(hc)
register HALF_CONNECTION hc;
{
	register int count;			/* amount to move */

       /*
        * Figure out how much, if any, we'll be able to do here
        */
	count = min(hc->remaining, hc->stream_buffer_remaining);
	if (count <= 0)
		return;				/* could not satisfy */
						/* any from buffered data*/

       /*
        * Copy the data, update both stream and data buffer pointers
        */

	memcpy(hc->next_byte, hc->stream_buffer_next, count);

	hc->stream_buffer_next += count;
	hc->stream_buffer_remaining -= count;
	FIX_BUFFER_POINTERS(hc, count)
	
}

/************************************************************************/
/*	
/*	                        gdb_write_data
/*	
/*	This routine implements gdb_move_data for an outbound half
/*	connection.
/*	
/************************************************************************/

int
gdb_write_data(hc)
struct  half_con_data *hc;			/* pointer to control struct */
						/* for this half connection */
{
	register HALF_CONNECTION half_con = hc;
						/* half connection pointer */
						/* fast copy in register */
	register int count;			/* number of bytes read */
						/* or written in latest */
						/* attempt */
	fd_set *fdbits;				/* the mask we should adjust */
						/* for this direction */
	fd_set tst_bits;			/* these are used for */
						/* the select we do prior */
						/* to reading which tells */
						/* us whether 0 byte read */
						/* means empty or closed  */
        int selected;				/* TRUE iff select says */
						/* we should be able to */
						/* progress */

       /*
        * Loop writing to the socket until it claims that no more 
        * progress can be made.  Note that some versions of Unix report
        * socket failure by select = 1, write count = 0.  To avoid
        * extra selects, we try the write first, and only do the select/write
        * sequence if write seems not to be progressing.
        */
	FD_ZERO(&tst_bits);
	while(half_con->remaining>0) {
		count = write(half_con->fd, half_con->next_byte,
			      (int)min(half_con->remaining, 
				       GDB_MAX_SOCK_WRITE));
		if (count == 0) {
			FD_SET(half_con->fd,&tst_bits);
			selected = select(gdb_mfd,
					  (fd_set *)NULL, &tst_bits,
					  (fd_set *)NULL, 
					  &gdb_notime);
			if (selected == (-1)) {
				gdb_conok = FALSE;
				break;
			}
			if (selected == 0) {
				count =0;
				break;
			}
			count = write(half_con->fd, half_con->next_byte,
				      (int)min(half_con->remaining, 
					       GDB_MAX_SOCK_WRITE));
			if (count==0) {
				if (selected == 1)
				  gdb_conok = FALSE;
				break;		/* no more data available now*/
			}
		}
		/*
		 * Count is != 0
		 */
		if (count<0) {
			count = 0;
			if (errno != EWOULDBLOCK) {
				gdb_conok  = FALSE; /* tell callers */
				/* that con has */
				/* died */
			}
			break;
		}

		half_con->flags |= HCON_PROGRESS;
		FIX_BUFFER_POINTERS(half_con, count)
	}
	/*
	 * The file descriptor masks used for doing selects must be activated
	 * when and only when there is a pending operation trying to use
	 * the connection.  Update the masks for this half connection.
	 */
	fdbits =  &gdb_cwfds;
	if (half_con->remaining >0 && gdb_conok)
		FD_SET(half_con->fd, fdbits);
	else					
		FD_CLR(half_con->fd, fdbits);

	return;
}

/************************************************************************/
/*	
/*
/*		           gdb_read_data_and_buffer
/*	
/*	This routine is called only when the half_connection stream 
/*	buffer is known to be empty and the "next-byte" buffer    
/*	has more to be filled in.  We try in one read to finish
/*	off the user's request and at the same time fill the stream
/*	buffer for later.
/*	
/*     	We assume here that all fd's are set to non-blocking I/O, so
/*	we can safely try reading and writing until they return 0 bytes.
/*	
/************************************************************************/

int
gdb_read_data_and_buffer(hc)
struct  half_con_data *hc;			/* pointer to control struct */
						/* for this half connection */
{
	register HALF_CONNECTION half_con = hc;
						/* half connection pointer */
						/* fast copy in register */
	register int count;			/* number of bytes read */
						/* or written in latest */
						/* attempt */
	fd_set *fdbits;				/* the mask we should adjust */
						/* for this direction */
       	struct iovec iov[2];			/* we use this to hold */
						/* pointers to (1) the */
						/* actual user data buffer */
						/* and (2) the pipe length */
						/* pre-read buffer */
	int fix_amount;				/* amount to adjust */
						/* half_con->remaining*/

	/*----------------------------------------------------------*/
	/*	
	/*	Mark the stream buffer as empty, in case we don't
	/*	get around to filling it.
	/*	
	/*----------------------------------------------------------*/

	half_con -> stream_buffer_next = half_con -> stream_buffer;
	half_con -> stream_buffer_remaining = 0;

	/*----------------------------------------------------------*/
	/*	
	/*	Loop trying to read data from the socket.  We scatter
	/*	first into the user's buffer directly, then into
	/*	the stream buffer (which helps us save system
	/*	calls next time around.)  We stop either when:
        /*      socket reports error/no progress or user's buffer is
	/*	full.
	/*	
	/*----------------------------------------------------------*/

       /*
        * Loop until either (1) the connection reported that it could
        * not progress any further or (2) the full count has been 
        * satisfied.  Some versions of Unix observe the rule that 
        * a closed connection, especially when reading, is indicated
        * by returning a count of 0 on read when select claims that progress
        * can be made.  We used to handle this case.  Bill Sommerfeld
        * has introduced a performance change which leaves that checking
        * out in the latest version.  To add it back, then ONLY in
        * the case where read returned 0, do a select followed by another
        * read (the order is important).  If we ever run on a system that
        * works in this way, we may hang at close time.
        */

	while(half_con->remaining>0) {
               /*
                * First we try a read, and if it works, we believe it
                */
		iov[0].iov_base = half_con -> next_byte;
		iov[0].iov_len =  half_con -> remaining;
		iov[1].iov_base = half_con -> stream_buffer;
		iov[1].iov_len =  half_con -> stream_buffer_length;
		count = readv(half_con->fd, iov, 2);

		if (count<0) {
			count = 0;
			if (errno != EWOULDBLOCK) 
				gdb_conok  = FALSE; /* tell callers that */
						    /* con has died */
			break;

		}
		if (count == 0) {/* We hit EOF */
			gdb_conok = FALSE;
			break;
		}
			
		/*
                * Count is >0, we moved some data.  Note, setting of
                * stream_buffer_remaining can only be non-zero on last
                * time through the loop, because that will be when 
                * half_con->remaining goes to zero.
                */
		half_con->flags |= HCON_PROGRESS;
		half_con->stream_buffer_remaining=max(0, count-iov[0].iov_len);
		fix_amount = min(count,half_con->remaining);
		FIX_BUFFER_POINTERS(half_con, fix_amount);
	}

       /*
        * The file descriptor masks used for doing selects must be activated
        * when and only when there is a pending operation trying to use
        * the connection.  Update the masks for this half connection.
        */
	fdbits =  &gdb_crfds;
	if (half_con->remaining >0)
		FD_SET(half_con->fd, fdbits);
	else					
		FD_CLR(half_con->fd, fdbits);

	return ;
}

/************************************************************************/
/*	
/*			gdb_receive_data (gdb_receive_data)
/*	
/*	This routine is called by an init or continuation routine to
/*	request that a specified amount of data be read, without 
/*	blocking, on the supplied connection.  This routine returns
/*	OP_COMPLETE if the entire read completed synchronously,
/*	or OP_RUNNING if the read remains ongoing or is cancelling
/*	due to error on the socket.
/*	
/************************************************************************/

int
gdb_receive_data(half_con, ptr, len)
HALF_CONNECTION half_con;			/* read on this connection*/
char    *ptr;					/* put first byte here */
int	len;					/* number of bytes to read */
{
       /*
        * Fill in the initial state of the attempted receive 
        */
	half_con->remaining = len;
	half_con->next_byte = ptr;

       /*
        * Now see if we can make some progress on this read, possibly
        * even completing it synchronously.  Return appropriate
        * result to our caller.  Note: errors are reflected as OP_RUNNING
        * with global variable gdb_cnok set to FALSE.
        */
	if(gdb_move_data(CON_INPUT, half_con))
		return OP_COMPLETE;
	else
	 	return OP_RUNNING;
}

/************************************************************************/
/*	
/*			gdb_send_data (gdb_send_data)
/*	
/*	This routine is called by an init or continuation routine to
/*	request that a specified amount of data be written, without 
/*	blocking, on the supplied connection.  This routine returns
/*	OP_COMPLETE if the entire write completed synchronously,
/*	or OP_RUNNING if the output remains ongoing or there was an error.
/*	
/************************************************************************/

int
gdb_send_data(half_con, ptr, len)
HALF_CONNECTION half_con;			/* write on this connection*/
char    *ptr;					/* put first byte here */
int	len;					/* number of bytes to read */
{

       /*
        * Fill in the initial state of the attempted receive 
        */
	half_con->remaining = len;
	half_con->next_byte = ptr;

       /*
        * Now see if we can make some progress on this read, possibly
        * even completing it synchronously.  Return appropriate
        * result to our caller.
        */
	if(gdb_move_data(CON_OUTPUT, half_con))
		return OP_COMPLETE;
	else
	 	return OP_RUNNING;
}

/************************************************************************/
/*	
/*			gdb_start_a_listen
/*	
/*	This routine is called by an init or continuation routine to
/*	request that a connection be done.  This routine returns
/*	OP_COMPLETE if the accept completed synchronously,
/*	or OP_RUNNING if the output remains ongoing or there was an error.
/*	
/************************************************************************/

int
gdb_start_a_listen(half_con, otherside, lenp, fdp)
HALF_CONNECTION half_con;			/* write on this connection*/
char    *otherside;				/* put first byte here */
int	*lenp;					/* number of bytes to read */
int     *fdp;
{

       /*
        * Fill in the initial state of the attempted accept
        */
	half_con->accepted_len = lenp;
	half_con->next_byte = otherside;
	half_con->accepted_fdp = fdp;

       /*
        * Now see if we can make some progress on this read, possibly
        * even completing it synchronously.  Return appropriate
        * result to our caller.
        */
	if(gdb_listen(half_con))
		return OP_COMPLETE;
	else
	 	return OP_RUNNING;
}

/************************************************************************/
/*	
/*			gdb_listen (gdb_listen)
/*	
/*	This routine is called from gdb_start_a_listen or hcon_progress to attempt
/*	to continue making progress in accepting a connection on a
/*	listening connection.
/*	
/************************************************************************/

int
gdb_listen(hc)
struct  half_con_data *hc;			/* pointer to control struct */
						/* for this half connection */
{
	register HALF_CONNECTION half_con = hc;
						/* half connection pointer */
						/* fast copy in register */

	GDB_INIT_CHECK

	half_con->flags &= ~HCON_PENDING_LISTEN;/* in case we succeed */

       /* 
        * The first implementatin of this used to do a select to make sure
        * that the accept would not block.  Bill Sommerfeld has changed this
        * to non-blocking I/O, so the following code is commented out.
        */
#ifdef notdef
	FD_ZERO(&tst_bits);
	FD_SET(half_con->fd,&tst_bits);
	selected = select(gdb_mfd,&tst_bits, (fd_set *)NULL, (fd_set *)NULL,
			  &gdb_notime);
        /*
         * If selected==(-1), then we know there's something
         * wrong with the socket
         */
	 if (selected == (-1)) {
		gdb_conok = FALSE;
		return FALSE;
	 }
	 /*
	  * if selected==0, then we know accept won't do anything, so
	  * don't try.
	  */
  	 if (selected == 0) {
		half_con->flags |= HCON_PENDING_LISTEN;
		FD_SET(half_con->fd, &gdb_crfds); /* we'll be looking for */
						  /* this whenever we select*/
		return FALSE;
	 }
        /*
         * Selected is >0.  The accept SHOULD not hang.
         */
#endif notdef

       /*
        * Here is Bill's non-blocking implementation
        */
	 *(half_con->accepted_fdp) = accept(half_con->fd,
			    (struct sockaddr *)half_con->next_byte,
		           half_con->accepted_len);
	/*
	 * See whether the accept succeeded
	 */
	 if (*(half_con->accepted_fdp) < 0) {
		 if (errno != EWOULDBLOCK) {
			 gdb_conok = FALSE;	/* error will be returned */
						/* in shut-down listening con*/
		 }
		 half_con->flags |= HCON_PENDING_LISTEN;
		 FD_SET(half_con->fd, &gdb_crfds);
		 return FALSE;
	 }

	 FD_CLR(half_con->fd, &gdb_crfds);	/* don't select on this */
	 return TRUE;
}
