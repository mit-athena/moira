/* $Id: get_file.c,v 1.16 1998-02-05 22:52:00 danw Exp $
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include "update_server.h"
#include "update.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <des.h>
#include <gdb.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/get_file.c,v 1.16 1998-02-05 22:52:00 danw Exp $");

#ifndef MIN
#define MIN(a, b)    (((a) < (b)) ? (a) : (b))
#endif /* MIN */

extern CONNECTION conn;
extern char buf[BUFSIZ];

extern int code, uid;
extern char *whoami;

extern int have_authorization, have_file, done;
extern C_Block session;
static des_key_schedule sched;
static des_cblock ivec;

static int get_block(int fd, int max_size, int encrypt);

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

int get_file(char *pathname, int file_size, int checksum,
	     int mode, int encrypt)
{
  int fd, n_written;
  int found_checksum;

  if (!have_authorization)
    {
      reject_call(MR_PERM);
      return 1;
    }
  if (done)			/* re-initialize data */
    initialize();
  if (setuid(uid) < 0)
    {
      com_err(whoami, errno, "Unable to setuid to %d\n", uid);
      exit(1);
    }
  /* unlink old file */
  if (!config_lookup("noclobber"))
    unlink(pathname);
  /* open file descriptor */
  fd = open(pathname, O_CREAT|O_EXCL|O_WRONLY, mode);
  if (fd == -1)
    {
      code = errno;
      sprintf(buf, "%s: creating file %s (get_file)",
	      error_message(code), pathname);
      mr_log_error(buf);
      report_error("reporting file creation error (get_file)");
      if (setuid(0) < 0)
	{
	  com_err(whoami, errno, "Unable to setuid back to %d\n", 0);
	  exit(1);
	}
      return 1;
    }
  /* check to see if we've got the disk space */
  n_written = 0;
  while (n_written < file_size)
    {
      int n_wrote;
      n_wrote = write(fd, buf, sizeof(buf));
      if (n_wrote == -1)
	{
	  code = errno;
	  sprintf(buf, "%s: verifying free disk space for %s (get_file)",
		  error_message(code), pathname);
	  mr_log_error(buf);
	  /* do all we can to free the space */
	  unlink(pathname);
	  ftruncate(fd, 0);
	  close(fd);
	  report_error("reporting test-write error (get_file)");
	  if (setuid(0) < 0)
	    {
	      com_err(whoami, errno, "Unable to setuid back to %d\n", 0);
	      exit(1);
	    }
	  return 1;
	}
      n_written += n_wrote;
    }
  lseek(fd, 0, SEEK_SET);
  if (send_ok())
    lose("sending okay for file transfer (get_file)");
  if (encrypt)
    {
      des_key_sched(session, sched);
      memcpy(ivec, session, sizeof(ivec));
    }
  n_written = 0;
  while (n_written < file_size && code == 0)
    {
      int n_got = get_block(fd, file_size - n_written, encrypt);
      if (n_got == -1)
	{
	  /* get_block has already printed a message */
	  unlink(pathname);
	  if (setuid(0) < 0)
	    {
	      com_err(whoami, errno, "Unable to setuid back to %d\n", 0);
	      exit(1);
	    }
	  return 1;
	}
      n_written += n_got;
      if (n_written != file_size)
	{
	  if (send_ok())
	    lose("receiving data");
	}
    }
  if (code)
    {
      code = connection_errno(conn);
      report_error("reading file (get_file)");
      if (setuid(0) < 0)
	{
	  com_err(whoami, errno, "Unable to setuid back to %d\n", 0);
	  exit(1);
	}
      return 1;
    }
  fsync(fd);
  ftruncate(fd, file_size);
  fsync(fd);
  close(fd);
  if (setuid(0) < 0)
    {
      com_err(whoami, errno, "Unable to setuid back to %d\n", 0);
      exit(1);
    }
  /* validate checksum */
  found_checksum = checksum_file(pathname);
  if (checksum != found_checksum)
    {
      code = MR_MISSINGFILE;
      com_err(whoami, code, ": expected = %d, found = %d",
	      checksum, found_checksum);
      report_error("checksum error");
      return 1;
    }
  /* send ack or nack */
  have_file = 1;
  if (send_ok())
    {
      code = connection_errno(conn);
      unlink(pathname);
      lose("sending ok after file transfer (get_file)");
      return 1;
    }
  return 0;
}

static int get_block(int fd, int max_size, int encrypt)
{
  STRING data;
  unsigned char dst[UPDATE_BUFSIZ + 8], *src;
  int n_read, n, i;

  code = receive_object(conn, (char *)&data, STRING_T);
  if (code)
    {
      code = connection_errno(conn);
      lose("receiving data file (get_file)");
    }

  if (encrypt)
    {
      src = (unsigned char *)STRING_DATA(data);
      n = MAX_STRING_SIZE(data);
      des_pcbc_encrypt(src, dst, n, sched, ivec, 1);
      for (i = 0; i < 8; i++)
	ivec[i] = src[n - 8 + i] ^ dst[n - 8 + i];
      memcpy(STRING_DATA(data), dst, n);
    }

  n_read = MIN(MAX_STRING_SIZE(data), max_size);
  n = 0;
  while (n < n_read)
    {
      int n_wrote;
      n_wrote = write(fd, STRING_DATA(data) + n, n_read - n);
      if (n_wrote == -1)
	{
	  code = errno;
	  sprintf(buf, "%s: writing file (get_file)", error_message(code));
	  mr_log_error(buf);
	  string_free(&data);
	  report_error("reporting write error (get_file)");
	  close(fd);
	  return -1;
	}
      n += n_wrote;
    }
  string_free(&data);
  return n;
}
