/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/send_file.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/send_file.c,v 1.1 1987-08-22 17:54:57 wesommer Exp $
 */

#ifndef lint
static char *rcsid_send_file_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/send_file.c,v 1.1 1987-08-22 17:54:57 wesommer Exp $";
#endif	lint

#include <stdio.h>
#include "com_err.h"
#include "gdb.h"
#include "smsu_int.h"
#include <sys/file.h>
#include "update.h"
#include "sms_update_int.h"
#include "kludge.h"

extern CONNECTION conn;
extern int errno;
char buf[BUFSIZ];
extern struct update_desc *info;

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
send_file(pathname, n_to_send)
    char *pathname;
    int n_to_send;
{
    int n, fd, code;
    STRING data;

    fd = open(pathname, O_RDONLY, 0);
    if (fd == -1) {
	sprintf(buf, "%s: can't open %s for transmission",
		error_message(errno), pathname);
	sms_log_error(buf);
	return(1);
    }
    code = receive_object(conn, (char *)&n, INTEGER_T);
    if (code) {
	com_err(whoami, connection_errno(conn), ": lost connection");
	info->override = INTERVAL_connection_lost;
	close(fd);
	return(1);
    }
    if (n) {
	log_priority = log_ERROR;
	com_err(whoami, n, ": from remote server: can't update %s",
		pathname);
	info->override = INTERVAL_update_failed;
	close(fd);
	return(1);
    }
    string_alloc(&data, UPDATE_BUFSIZ);
    while (n_to_send > 0) {
#ifdef	DEBUG
	printf("n_to_send = %d\n", n_to_send);
#endif	/* DEBUG */
	n = read(fd, STRING_DATA(data), UPDATE_BUFSIZ);
	if (n < 0) {
	    sprintf(buf, "%s: reading %s for transmission",
		    error_message(errno), pathname);
	    sms_log_error(buf);
	    close(fd);
	    return(1);
	}
	MAX_STRING_SIZE(data) = n;
	code = send_object(conn, (char *)&data, STRING_T);
	if (code) {
	    sprintf(buf, "%s: transmitting file %s",
		    error_message(connection_errno(conn)), pathname);
	    sms_log_error(buf);
	    close(fd);
	    return(1);
	}
	n_to_send -= n;
	code = receive_object(conn, (char *)&n, INTEGER_T);
	if (code) {
	    sprintf(buf,
		    "%s: awaiting ACK remote server during transmission of %s",
		    error_message(connection_errno(conn)), pathname);
	    sms_log_error(buf);
	    close(fd);
	    return(1);
	}
	if (n) {
	    sprintf(buf, "%s: from remote server during transmission of %s",
		    error_message(n), pathname);
	    sms_log_error(buf);
	    close(fd);
	    return(1);
	}
    }
    close(fd);
    return(0);
}
