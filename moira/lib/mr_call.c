/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_call.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_call.c,v 1.2 1987-05-31 22:03:37 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 */

#ifndef lint
static char *rcsid_sms_call_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_call.c,v 1.2 1987-05-31 22:03:37 wesommer Exp $";
#endif lint

#include "sms_private.h"

/*
 * GDB operations to send and recieve RPC requests and replies.
 */

static sms_abort_send() {}
static sms_abort_recv() {}
	
/*
 * This doesn't get called until after the actual buffered write completes.
 * In a non-preflattening version of this, this would then queue the
 * write of the next bunch of data.
 */

static sms_cont_send(op, hcon, arg)
	OPERATION op;
	HALF_CONNECTION hcon;
	struct sms_params *arg;
{
	op->result = OP_SUCCESS;
	db_free(arg->sms_flattened, arg->sms_size);
#ifdef notdef
	db_free(arg, sizeof(struct sms_params));
#endif notdef
	return OP_COMPLETE;
}

sms_start_send(op, hcon, arg)
	OPERATION op;
	HALF_CONNECTION hcon;
	register struct sms_params *arg;
{
	int sms_size, i, len;
	int *argl;
	char *buf, *bp;
	
	/*
	 * This should probably be split into several routines.
	 * It could also probably be made more efficient (punting most
	 * of the argument marshalling stuff) by doing I/O directly
	 * from the strings.  Anyone for a scatter/gather sms_send_data?
	 *
	 * that would look a lot like the uio stuff in the kernel..  hmm.
	 */
	
	/*
	 * Marshall the entire data right now..
	 *      We are sending the version number,
	 * 	total request size, request number, 
	 * 	argument count, and then each argument.
	 * 	At least for now, each argument is a string, which is
	 * 	sent as a count of bytes followed by the bytes
	 * 	(including the trailing '\0'), padded
	 * 	to a longword boundary.
	 */

	sms_size = 4 * sizeof(long);

	argl = (int *)malloc(sizeof(int) * arg->sms_argc);
	
	/*
	 * For each argument, figure out how much space is needed.
	 */
	
	for (i = 0; i < arg->sms_argc; ++i) {
		if (arg->sms_argl)
			argl[i] = len = arg->sms_argl[i];
		else
			argl[i] = len = strlen(arg->sms_argv[i]) + 1;
		sms_size += sizeof(long) + len;
		sms_size += sizeof(long) * howmany(sms_size, sizeof(long));
	}
	
	arg->sms_flattened = buf = db_alloc(sms_size);

	bzero(arg->sms_flattened, sms_size);
	
	arg->sms_size = sms_size;
	
	/*
	 * This is gross.  Any better suggestions, anyone?
	 * It should work on the RT's, since malloc is guaranteed to
	 * return a pointer which is aligned correctly for any data.
	 */

	((long *)buf)[0] = htonl(sms_size);
	((long *)buf)[1] = htonl(SMS_VERSION_1);
	((long *)buf)[2] = htonl(arg->sms_procno);
	((long *)buf)[3] = htonl(arg->sms_argc);

	/*
	 * bp is a pointer into the point in the buffer to put
	 * the next argument.
	 */
	
	bp = (char *)(((long *)buf) + 4);
	
	for (i = 0; i<arg->sms_argc; ++i) {
		len = argl[i];
		*((long *)bp) = htonl(len);
		bp += sizeof(long);
		bcopy(arg->sms_argv[i], bp, len);
		bp += sizeof(long) * howmany(len, sizeof(long));
	}
	op->fcn.cont = sms_cont_send;
	arg->sms_size = sms_size;
	
	if (gdb_send_data(hcon, arg->sms_flattened, sms_size) == OP_COMPLETE)
		return sms_cont_send(op, hcon, arg);
	else return OP_RUNNING;
}	
	
static sms_cont_recv(op, hcon, argp)
	OPERATION op;
	HALF_CONNECTION hcon;
	sms_params **argp;
{
	int done = FALSE;
	char *cp;
	int *ip;
	int i;
	register sms_params *arg = *argp;
						       
	while (!done) {
		switch (arg->sms_state) {
		case S_RECV_START:
			arg->sms_state = S_RECV_DATA;
			if (gdb_receive_data(hcon, (caddr_t)&arg->sms_size,
					     sizeof(long)) == OP_COMPLETE)
				continue;
			done = TRUE;
			break;
		case S_RECV_DATA:
			/* Should validate that length is reasonable */
			arg->sms_size = ntohl(arg->sms_size);
			arg->sms_flattened = db_alloc(arg->sms_size);
			arg->sms_state = S_DECODE_DATA;
			bcopy((caddr_t)&arg->sms_size, arg->sms_flattened, sizeof(long));
			
			if (gdb_receive_data(hcon,
					     arg->sms_flattened + sizeof(long),
					     arg->sms_size - sizeof(long))
			    == OP_COMPLETE)
				continue;
			done = TRUE;
			break;
		case S_DECODE_DATA:
			cp = arg->sms_flattened;
			ip = (int *) cp;
			/* we already got the overall length.. */
			for(i=1; i <4; i++) ip[i] = ntohl(ip[i]);
			if (ip[1] != SMS_VERSION_1)
				arg->sms_status = SMS_VERSION_MISMATCH;
			else arg->sms_status = ip[2];
			arg->sms_argc = ip[3];
			cp += 4 * sizeof(int);
			arg->sms_argv=(char **)db_alloc(arg->sms_argc *sizeof(char **));
			arg->sms_argl=(int *)db_alloc(arg->sms_argc *sizeof(int *));
			
			for (i = 0; i<arg->sms_argc; ++i) {
				int nlen = ntohl(* (int *) cp);
				cp += sizeof (long);
				arg->sms_argv[i] = (char *)db_alloc(nlen);
				bcopy(cp, arg->sms_argv[i], nlen);
				arg->sms_argl[i]=nlen;
				cp += sizeof(long) *
					howmany(nlen, sizeof(long));
			}
			return OP_COMPLETE;
		}
	}
	return OP_RUNNING;
}
			

sms_start_recv(op, hcon, argp)
	OPERATION op;
	HALF_CONNECTION hcon;
	struct sms_params **argp;
{
	register sms_params *arg = *argp;
	if (!arg) {
		*argp = arg = (sms_params *)db_alloc(sizeof(sms_params));
	}
	arg->sms_state = S_RECV_START;
	op->fcn.cont = sms_cont_recv;
	return sms_cont_recv(op, hcon, argp);
}

sms_do_call(parms, reply)
	struct sms_params *parms;
	struct sms_params **reply;
{
	if (!_sms_conn)
		return SMS_NOT_CONNECTED;

	if (!_sms_send_op)
		_sms_send_op = create_operation();

	if (!_sms_recv_op)
		_sms_recv_op = create_operation();

	gdb_inop(_sms_send_op, sms_start_send, parms, sms_abort_send);
	gdb_qop(_sms_conn, CON_OUTPUT, _sms_send_op);

	gdb_inop(_sms_recv_op, sms_start_recv, reply, sms_abort_recv);
	gdb_qop(_sms_conn, CON_INPUT, _sms_recv_op);

	/* Block until operation done. */
	gdb_cmpo(_sms_send_op);
	gdb_cmpo(_sms_recv_op);
	/* Look at results */
	if (OP_STATUS(_sms_recv_op) != OP_COMPLETE) {
		return SMS_ABORTED;
	}
	return 0;
}

sms_destroy_reply() /*XXX not implemented yet*/
{
	
}
