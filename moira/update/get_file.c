/* $Id: get_file.c,v 1.17 1998-02-15 17:49:28 danw Exp $
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
#include <stdlib.h>
#include <unistd.h>

#include <des.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/get_file.c,v 1.17 1998-02-15 17:49:28 danw Exp $");

#ifndef MIN
#define MIN(a, b)    (((a) < (b)) ? (a) : (b))
#endif /* MIN */

static des_key_schedule sched;
static des_cblock ivec;
extern des_cblock session;

static int get_block(int conn, int fd, int max_size, int encrypt);

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

int get_file(int conn, char *pathname, int file_size, int checksum,
	     int mode, int encrypt)
{
  int fd, n_written, code;
  int found_checksum;
  char buf[BUFSIZ];

  if (!have_authorization)
    {
      send_int(conn, MR_PERM);
      return 1;
    }
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
      com_err(whoami, errno, "creating file %s (get_file)", pathname);
      send_int(conn, code);
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
	  com_err(whoami, code, "verifying free disk space for %s (get_file)",
		  pathname);
	  send_int(conn, code);

	  /* do all we can to free the space */
	  unlink(pathname);
	  ftruncate(fd, 0);
	  close(fd);

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
  send_ok(conn);

  if (encrypt)
    {
      des_key_sched(session, sched);
      memcpy(ivec, session, sizeof(ivec));
    }

  n_written = 0;
  while (n_written < file_size)
    {
      int n_got = get_block(conn, fd, file_size - n_written, encrypt);

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
	send_ok(conn);
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
      send_int(conn, code);
      return 1;
    }

  send_ok(conn);
  return 0;
}

static int get_block(int conn, int fd, int max_size, int encrypt)
{
  char *data;
  size_t len;
  int n_read, n, i, code;

  recv_string(conn, &data, &len);

  if (encrypt)
    {
      char *unenc = malloc(len);

      if (!unenc)
	{
	  send_int(conn, ENOMEM);
	  return -1;
	}

      des_pcbc_encrypt(data, unenc, len, sched, ivec, 1);
      for (i = 0; i < 8; i++)
	ivec[i] = data[len - 8 + i] ^ unenc[len - 8 + i];
      free(data);
      data = unenc;
    }

  n_read = MIN(len, max_size);
  n = 0;
  while (n < n_read)
    {
      int n_wrote;
      n_wrote = write(fd, data + n, n_read - n);
      if (n_wrote == -1)
	{
	  code = errno;
	  com_err(whoami, errno, "writing file (get_file)");
	  send_int(conn, code);
	  free(data);
	  close(fd);
	  return -1;
	}
      n += n_wrote;
    }
  free(data);
  return n;
}
