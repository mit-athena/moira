/*
 * GDB operations to send and recieve RPC requests and replies.
 */

sms_start_call(op, hcon, arg)
	OPERATION op;
	HALF_CONNECTION hcon;
	register struct param *arg;
{
	/*
	 * This should probably be split into several routines.
	 * It could also probably be made more efficient (punting most
	 * of the argument marshalling stuff by doing I/O directly
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

	argl = (int *)malloc(sizeof(int) * arg->argc);
	
	/*
	 * For each argument, figure out how much space is needed.
	 */
	
	for (i = arg->argc; i; --i) {
		argl[i] = len = strlen(arg->argv[i]) + 1;
		sms_size += sizeof(long) + len;
		sms_size = sizeof(long) * howmany(sms_size, sizeof(long));
	}
	
	arg->flattened = buf = db_alloc(sms_size);

	bzero(arg->flattened, sms_size);
	
	arg->size = sms_size;
	
	/*
	 * This is gross.  Any better suggestions, anyone?
	 * It should work on the RT's, since malloc is guaranteed to
	 * return a pointer which is aligned correctly for any data.
	 */

	((long *)buf)[0] = htonl(SMS_VERSION_1);
	((long *)buf)[1] = htonl(sms_size);
	((long *)buf)[2] = htonl(arg->procno);
	((long *)buf)[3] = htonl(arg->argc);

	/*
	 * bp is a pointer into the point in the buffer to put
	 * the next argument.
	 */
	
	bp = (char *)(&(long *)buf[4])
	for (i = arg->argc; i; --i) {
		len = argl[i];
		*((long *)bp) = htonl(len);
		bp += sizeof(long);
		bcopy(arg->argv[i], bp, len);
		bp += sizeof(long) * howmany(len, sizeof(long));
	}
	op->fcn.cont = sms_cont_call;
	arg->size = sms_size;
	
	if (gdb_send_data(hcon, arg->flattened, sms_size) == OP_COMPLETE)
		return sms_cont_call(op, hcon, arg);
	else return OP_RUNNING;
}	
	
/*
 * This doesn't get called until after the actual buffered write completes.
 * In a non-preflattening version of this, this would then queue the
 * write of the next bunch of data.
 */

sms_cont_call(op, hcon, arg)
	OPERATION op;
	HALF_CONNECTION hcon;
	struct params *arg;
{
	op->result = OP_SUCCESS;
	db_free(arg->flattened, arg->size);
	db_free(arg, sizeof(struct params));
	return OP_COMPLETE;
}


/* Need routines to decode all that... */
