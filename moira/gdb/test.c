/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/test.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/test.c,v 1.1 1987-08-02 22:14:13 wesommer Exp $
 */

#ifndef lint
static char *rcsid_test_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/test.c,v 1.1 1987-08-02 22:14:13 wesommer Exp $";
#endif	lint





















/************************************************************************/
/*	
/*			   gdb_trans2.c
/*	
/*	      GDB - Data Transport Services Routines (Part 2)
/*	
/*	Author: Noah Mendelsohn
/*	Copyright: 1986 MIT Project Athena 
/*	
/*	These routines implement layer 6 of the Client Library
/*	Specification of the GDB system, as well as the facilities
/*	outlined in the GDB Protocol Specification.  Part 2 of 2.
/*
/*	Some of the routines specified are actually implemented as
/*	macros defined in gdb.h.
/*	
/************************************************************************/

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include "gdb.h"

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
        * Force progress on all connections
        */
	gdb_progress();

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
	g_op_newhead(op->halfcon);

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
						/* HCON_BUSY */
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
			g_stop_connection(con);
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
			g_stop_connection(con);
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

int
gdb_move_data(direction, hc)
int	direction;				/* CON_INPUT or CON_OUTPUT */
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
        * For safety, in case we're called when nothing is pending.
        */
	if (half_con->remaining == 0)
		return TRUE;
       /*
        * Loop until either (1) the connection reported that it could
        * not progress any further or (2) the full count has been 
        * satisfied.  Some versions of Unix observe the rule that 
        * a closed connection, especially when reading, is indicated
        * by select claiming that there is data when read says there
        * isn't.  Also, some other versions return errors from the
        * select on that FD.  We test for both situations here.
        */
	FD_ZERO(&tst_bits);

	while(half_con->remaining>0) {
		FD_SET(half_con->fd,&tst_bits);
		if (direction == CON_INPUT) {
			selected = select(gdb_mfd,&tst_bits, NULL, NULL, 
					  &gdb_notime);
                       /*
                        * If selected==(-1), then we know there's something
                        * wrong with the socket
                        */
			if (selected == (-1)) {
				gdb_conok = FALSE;
				break;
			}
                       /*
                        * if selected==0, then we know read won't do
                        * anything, so save the extraneous system call
                        */
			if (selected == 0) {
				count =0;
				break;
			}
                       /*
                        * Selected is >0.  Either the read is going to 
                        * return 0, or this is one of those versions of
                        * Unix that tells us the connection has died by
                        * indicating select=1, read=0.
                        */
			count = read(half_con->fd, half_con->next_byte,
				     half_con->remaining);
		} else {
			selected = select(gdb_mfd, NULL, &tst_bits, NULL, 
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
				     min(half_con->remaining, 
					 GDB_MAX_SOCK_WRITE));
		}
               /*
                * We moved some data
                */
		if (count >0)
			half_con->flags |= HCON_PROGRESS;
               /*
                * rc==0 means we didn't mvoe any data, but if accompanied
                * by select claiming there was data, it really means the 
                * connection is dead!
                */
		if (count==0) {
			if (selected == 1)
				gdb_conok = FALSE;
			break;		/* no more data available now*/
		}
		if (count<0) {
			count = 0;
			if (errno != EWOULDBLOCK) {
				gdb_conok  = FALSE; /* tell callers that */
						    /* con has died */
			}
			break;

		}
		half_con->remaining -=count;
		half_con->next_byte +=count;
	}

       /*
        * The file descriptor masks used for doing selects must be activated
        * when and only when there is a pending operation trying to use
        * the connection.  Update the masks for this half connection.
        */
	fdbits = (direction == CON_INPUT)? &gdb_crfds : &gdb_cwfds;
	if (half_con->remaining >0)
		FD_SET(half_con->fd, fdbits);
	else					
		FD_CLR(half_con->fd, fdbits);

	return (half_con->remaining == 0);
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
/*			gdb_send_data (gdb_sndat)
/*	
/*	This routine is called by an init or continuation routine to
/*	request that a specified amount of data be written, without 
/*	blocking, on the supplied connection.  This routine returns
/*	OP_COMPLETE if the entire write completed synchronously,
/*	or OP_RUNNING if the output remains ongoing or there was an error.
/*	
/************************************************************************/

int
gdb_sndat(half_con, ptr, len)
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

	fd_set tst_bits;			/* these are used for */
						/* the select we do prior */
						/* to reading which tells */
						/* us whether 0 byte read */
						/* means empty or closed  */
        int selected;				/* TRUE iff select says */
						/* we should be able to */
						/* progress */


	GDB_INIT_CHECK

	half_con->flags &= ~HCON_PENDING_LISTEN;/* in case we succeed */

	FD_ZERO(&tst_bits);
	FD_SET(half_con->fd,&tst_bits);
	selected = select(gdb_mfd,&tst_bits, NULL, NULL, &gdb_notime);
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
	 *(half_con->accepted_fdp) = accept(half_con->fd, half_con->next_byte,
				           half_con->accepted_len);
	/*
	 * See whether the accept succeeded
	 */
	 if (*(half_con->accepted_fdp) < 0) {
	 	perror("gdb: start_listening: error on listen");
		GDB_GIVEUP("Unexpected listen error");
	 }

	 FD_CLR(half_con->fd, &gdb_crfds);	/* don't select on this */
	 return TRUE;
}
