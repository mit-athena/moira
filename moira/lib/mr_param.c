/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_param.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_param.c,v 1.4 1988-09-13 15:52:30 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 */

#ifndef lint
static char *rcsid_sms_param_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_param.c,v 1.4 1988-09-13 15:52:30 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "sms_private.h"

/*
 * GDB operations to send and recieve RPC requests and replies.
 */

/*
 * This doesn't get called until after the actual buffered write completes.
 * In a non-preflattening version of this, this would then queue the
 * write of the next bunch of data.
 */

sms_cont_send(op, hcon, arg)
    OPERATION op;
    HALF_CONNECTION hcon;
    struct sms_params *arg;
{
    op->result = OP_SUCCESS;
    free(arg->sms_flattened);
    arg->sms_flattened = NULL;
    
    return OP_COMPLETE;
}

sms_start_send(op, hcon, arg)
    OPERATION op;
    HALF_CONNECTION hcon;
    register struct sms_params *arg;
{
    int i, len;
    unsigned int sms_size;
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
     * We are sending the version number,
     * total request size, request number, 
     * argument count, and then each argument.
     * At least for now, each argument is a string, which is
     * sent as a count of bytes followed by the bytes
     * (including the trailing '\0'), padded
     * to a longword boundary.
     */

    sms_size = 4 * sizeof(long);

    argl = (int *)malloc((unsigned)(sizeof(int) * arg->sms_argc));

    /*
     * For each argument, figure out how much space is needed.
     */
	
    for (i = 0; i < arg->sms_argc; ++i) {
	if (arg->sms_argl)
	    argl[i] = len = arg->sms_argl[i];
	else
	    argl[i] = len = strlen(arg->sms_argv[i]) + 1;
	sms_size += sizeof(long) + len;
	/* Round up to next longword boundary.. */
	sms_size = sizeof(long) * howmany(sms_size, sizeof(long));
    }
	
    arg->sms_flattened = buf = malloc(sms_size);

    bzero(arg->sms_flattened, sms_size);
	
    arg->sms_size = sms_size;
	
    /*
     * This is gross.  Any better suggestions, anyone?
     * It should work on the RT's, since malloc is guaranteed to
     * return a pointer which is aligned correctly for any data.
     */

    ((long *)buf)[0] = htonl(sms_size);
    ((long *)buf)[1] = htonl(arg->sms_version_no);
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

    free(argl);
    
    if (gdb_send_data(hcon, arg->sms_flattened, sms_size) == OP_COMPLETE)
	return sms_cont_send(op, hcon, arg);
    else return OP_RUNNING;
}	
	
sms_cont_recv(op, hcon, argp)
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
	    fflush(stdout);
	    /* Should validate that length is reasonable */
	    arg->sms_size = ntohl(arg->sms_size);
	    if (arg->sms_size > 65536) {
		return OP_CANCELLED;
	    }
	    arg->sms_flattened = malloc(arg->sms_size);
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
	    arg->sms_version_no = ip[1];
	    if (arg->sms_version_no != SMS_VERSION_1 &&
		arg->sms_version_no != SMS_VERSION_2)
		arg->sms_status = SMS_VERSION_MISMATCH;
	    else arg->sms_status = ip[2];
	    arg->sms_argc = ip[3];
	    cp += 4 * sizeof(int);
	    arg->sms_argv=(char **)malloc(arg->sms_argc *sizeof(char **));
	    arg->sms_argl=(int *)malloc(arg->sms_argc *sizeof(int *));
			
	    for (i = 0; i<arg->sms_argc; ++i) {
		u_short nlen = ntohl(* (int *) cp);
		cp += sizeof (long);
		if (cp + nlen > arg->sms_flattened + arg->sms_size) {
		    free(arg->sms_flattened);
		    arg->sms_flattened = NULL;
		    return OP_CANCELLED;
		}		    
		arg->sms_argv[i] = (char *)malloc(nlen);
		bcopy(cp, arg->sms_argv[i], nlen);
		arg->sms_argl[i]=nlen;
		cp += sizeof(long) * howmany(nlen, sizeof(long));
	    }
	    free(arg->sms_flattened);
	    arg->sms_flattened = NULL;
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
	*argp = arg = (sms_params *)malloc(sizeof(sms_params));
	arg->sms_argl = NULL;
	arg->sms_argv = NULL;
	arg->sms_flattened = NULL;
    }
    arg->sms_state = S_RECV_START;
    op->fcn.cont = sms_cont_recv;
    return sms_cont_recv(op, hcon, argp);
}

sms_destroy_reply(reply)
    sms_params *reply;
{
    int i;
    if (reply) {
	if (reply->sms_argl)
	    free(reply->sms_argl);
	reply->sms_argl = NULL;
	if (reply->sms_flattened)
	    free(reply->sms_flattened);
	reply->sms_flattened = NULL;
	if (reply->sms_argv) {
	    for (i=0; i<reply->sms_argc; i++) {
		if (reply->sms_argv[i])
		    free (reply->sms_argv[i]);
		reply->sms_argv[i] = NULL;
	    }
	    free(reply->sms_argv);
	}
	reply->sms_argv = NULL;
	free(reply);
    }
}

/*
 * Local Variables:
 * mode: c
 * c-indent-level: 4
 * c-continued-statement-offset: 4
 * c-brace-offset: -4
 * c-argdecl-indent: 4
 * c-label-offset: -4
 * End:
 */
