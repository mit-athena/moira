/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/get_file.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/get_file.c,v 1.5 1989-08-16 21:00:02 mar Exp $
 */
/*  (c) Copyright 1988 by the Massachusetts Institute of Technology. */
/*  For copying and distribution information, please see the file */
/*  <mit-copyright.h>. */

#ifndef lint
static char *rcsid_get_file_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/get_file.c,v 1.5 1989-08-16 21:00:02 mar Exp $";
#endif	lint

#include <mit-copyright.h>
#include <stdio.h>
#include <gdb.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sms.h>
#include "update.h"

extern CONNECTION conn;
char buf[BUFSIZ];

extern int code, errno;

extern int have_authorization, have_file, done;

int get_block();

/*
 * get_file()
 *
 * arguments:
 *	char *pathname
 *		file to receive
 *	int file_size
 *		number of bytes
 *	int checksum
 *		linear checksum of bytes
 *
 * syntax:
 * (initial protocol already done)
 * <<< (int)code	(can we accept the file?)
 * >>> (STRING)data
 * <<< (int)code
 * >>> (STRING)data
 * <<< (int)code
 * ...
 * >>> (STRING)data	(last data block)
 * <<< (int)code	(from read, write, checksum verify)
 *
 * returns:
 * 	int
 * 		0 for success, 1 for failure
 *
 * function:
 *	perform initial preparations and receive file as
 * a single string, storing it into <pathname>
 *
 */

int
get_file(pathname, file_size, checksum)
    char *pathname;
    int file_size;
    int checksum;
{
    int fd, n_written;
    int found_checksum;
    
    if (!have_authorization) {
	reject_call(SMS_PERM);
	return(1);
    }
    if (done)			/* re-initialize data */
	initialize();
    /* unlink old file */
    (void) unlink(pathname);
    /* open file descriptor */
    fd = open(pathname, O_CREAT|O_EXCL|O_WRONLY, 0700);
    if (fd == -1) {
	code = errno;
	sprintf(buf, "%s: creating file %s (get_file)",
		error_message(code), pathname);
	sms_log_error(buf);
	report_error("reporting file creation error (get_file)");
	return(1);
    }
    /* check to see if we've got the disk space */
    n_written = 0;
    while (n_written < file_size) {
	register int n_wrote;
	n_wrote = write(fd, buf, sizeof(buf));
	if (n_wrote == -1) {
	    code = errno;
	    sprintf(buf, "%s: verifying free disk space for %s (get_file)",
		    error_message(code), pathname);
	    sms_log_error(buf);
	    /* do all we can to free the space */
	    (void) unlink(pathname);
	    (void) ftruncate(fd, 0);
	    (void) close(fd);
	    report_error("reporting test-write error (get_file)");
	    return(1);
	}
	n_written += n_wrote;
    }
    lseek(fd, 0, L_SET);
    if (send_ok())
	lose("sending okay for file transfer (get_file)");
    n_written = 0;
    while (n_written < file_size && code == 0) {
	int n_got = get_block(fd, file_size - n_written);
	if (n_got == -1) {
	    /* get_block has already printed a message */
	    unlink(pathname);
	    return(1);
	}
	n_written += n_got;
	if (n_written != file_size)
	    if (send_ok())
		lose("receiving data");
    }
    if (code) {
	code = connection_errno(conn);
	report_error("reading file (get_file)");
	return(1);
    }
    fsync(fd);
    ftruncate(fd, file_size);
    fsync(fd);
    close(fd);
    /* validate checksum */
    found_checksum = checksum_file(pathname);
    if (checksum != found_checksum) {
	code = SMS_MISSINGFILE;
	com_err(whoami, code, ": expected = %d, found = %d",
		checksum, found_checksum);
	report_error("checksum error");
	return(1);
    }
    /* send ack or nack */
    have_file = 1;
    if (send_ok()) {
	code = connection_errno(conn);
	(void) unlink(pathname);
	lose("sending ok after file transfer (get_file)");
	return(1);
    }
    return(0);
}

static int
get_block(fd, max_size)
    int fd;
    int max_size;
{
    STRING data;
    int n_read, n;

    code = receive_object(conn, (char *)&data, STRING_T);
    if (code) {
	code = connection_errno(conn);
	lose("receiving data file (get_file)");
    }
    n_read = MIN(MAX_STRING_SIZE(data), max_size);
    n = 0;
    while (n < n_read) {
	register int n_wrote;
	n_wrote = write(fd, STRING_DATA(data)+n,
			n_read-n);
	if (n_wrote == -1) {
	    code = errno;
	    sprintf(buf, "%s: writing file (get_file)", error_message(code));
	    sms_log_error(buf);
	    string_free(&data);
	    report_error("reporting write error (get_file)");
	    close(fd);
	    return(-1);
	}
	n += n_wrote;
    }
    string_free(&data);
    return(n);
}
