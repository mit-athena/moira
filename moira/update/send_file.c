/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/send_file.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/send_file.c,v 1.3 1988-08-23 11:47:36 mar Exp $
 */

#ifndef lint
static char *rcsid_send_file_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/send_file.c,v 1.3 1988-08-23 11:47:36 mar Exp $";
#endif	lint

#include <stdio.h>
#include <com_err.h>
#include <gdb.h>
#include <dcm.h>
#include <sms.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <update.h>


extern CONNECTION conn;
extern int errno;
char buf[BUFSIZ];

/*
 * syntax:
 * (already sent) pathname file_size checksum
 * <<< (int)code			can we send it?
 * >>> data
 * <<< 0
 * >>> data
 * <<< 0
 * ....
 * >>> data				(last block)
 * <<< 0	(on final write, close, sync, checksum)
 *
 * returns:
 *  0 on success
 *  1 on error (file not found, etc)
 */

int
send_file(pathname, target_path)
char *pathname;
char *target_path;
{
    int n, fd, code, n_to_send;
    STRING data;
    struct stat statb;

    string_alloc(&data, UPDATE_BUFSIZ);

    /* send file over */
    fd = open(pathname, O_RDONLY, 0);
    if (fd < 0) {
	com_err(whoami, errno, "unable to open %s for read", pathname);
	return(SMS_OCONFIG);
    }
    if (fstat(fd, &statb)) {
	com_err(whoami, errno, "unable to stat %s", pathname);
	close(fd);
	return(SMS_OCONFIG);
    }
    n_to_send = statb.st_size;
    
    sprintf(STRING_DATA(data), "XFER_002 %d %d %s",
	    n_to_send, checksum_file(pathname), target_path);
    code = send_object(conn, (char *)&data, STRING_T);
    if (code) {
	com_err(whoami, code, " sending XFER_002 request");
	close(fd);
	return(code);
    }
    code = receive_object(conn, (char *)&n, INTEGER_T);
    if (code) {
	com_err(whoami, code, " getting reply from XFER_002 request");
	close(fd);
	return(code);
    }
    if (n) {
	com_err(whoami, n, " transfer request (XFER_002) rejected");
	close(fd);
	return(n);
    }
    
    code = receive_object(conn, (char *)&n, INTEGER_T);
    if (code) {
	com_err(whoami, connection_errno(conn), ": lost connection");
	close(fd);
	return(code);
    }
    if (n) {
	com_err(whoami, n, " from remote server: can't update %s",
		pathname);
	close(fd);
	return(n);
    }

    while (n_to_send > 0) {
#ifdef	DEBUG
	printf("n_to_send = %d\n", n_to_send);
#endif	/* DEBUG */
	n = read(fd, STRING_DATA(data), UPDATE_BUFSIZ);
	if (n < 0) {
	    com_err(whoami, errno, " reading %s for transmission", pathname);
	    close(fd);
	    return(SMS_ABORTED);
	}
	MAX_STRING_SIZE(data) = n;
	code = send_object(conn, (char *)&data, STRING_T);
	if (code) {
	    com_err(whoami, connection_errno(conn), " transmitting file %s",
		    pathname);
	    close(fd);
	    return(code);
	}
	n_to_send -= n;
	code = receive_object(conn, (char *)&n, INTEGER_T);
	if (code) {
	    com_err(whoami, connection_errno(conn),
		    " awaiting ACK remote server during transmission of %s",
		    pathname);
	    close(fd);
	    return(code);
	}
	if (n) {
	    com_err(whoami, n, " from remote server during transmission of %s",
		    pathname);
	    close(fd);
	    return(n);
	}
    }
    if (statb.st_size == 0) {
	code = receive_object(conn, (char *)&n, INTEGER_T);
	if (code) {
	    com_err(whoami, connection_errno(conn),
		    " awaiting ACK remote server after transmission of %s",
		    pathname);
	    close(fd);
	    return(code);
	}
	if (n) {
	    com_err(whoami, n, " from remote server after transmission of %s",
		    pathname);
	    close(fd);
	    return(n);
	}
    }
    close(fd);
    return(SMS_SUCCESS);
}
