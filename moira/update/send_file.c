/* $Id: send_file.c,v 1.13 1998-02-05 22:52:01 danw Exp $
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include "update_server.h"

#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <des.h>
#include <gdb.h>
#include <update.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/send_file.c,v 1.13 1998-02-05 22:52:01 danw Exp $");

extern CONNECTION conn;
char buf[BUFSIZ];
extern C_Block session;
extern char *whoami;

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

int send_file(char *pathname, char *target_path, int encrypt)
{
  int n, fd, code, n_to_send, i;
  STRING data;
  unsigned char dst[UPDATE_BUFSIZ + 8], *src;
  struct stat statb;
  des_key_schedule sched;
  des_cblock ivec;

  /* send file over */
  fd = open(pathname, O_RDONLY, 0);
  if (fd < 0)
    {
      com_err(whoami, errno, "unable to open %s for read", pathname);
      return MR_OCONFIG;
    }
  if (fstat(fd, &statb))
    {
      com_err(whoami, errno, "unable to stat %s", pathname);
      close(fd);
      return MR_OCONFIG;
    }
  n_to_send = statb.st_size;

  string_alloc(&data, UPDATE_BUFSIZ);
  sprintf(STRING_DATA(data), "XFER_00%c %d %d %s",
	  (encrypt ? '3' : '2'), n_to_send,
	  checksum_file(pathname), target_path);
  code = send_object(conn, (char *)&data, STRING_T);
  if (code)
    {
      com_err(whoami, connection_errno(conn), " sending XFER request");
      close(fd);
      return connection_errno(conn);
    }
  code = receive_object(conn, (char *)&n, INTEGER_T);
  if (code)
    {
      com_err(whoami, connection_errno(conn),
	      " getting reply from XFER request");
      close(fd);
      return connection_errno(conn);
    }
  if (n)
    {
      com_err(whoami, n, " transfer request (XFER) rejected");
      close(fd);
      return n;
    }

  code = receive_object(conn, (char *)&n, INTEGER_T);
  if (code)
    {
      com_err(whoami, connection_errno(conn), ": lost connection");
      close(fd);
      return connection_errno(conn);
    }
  if (n)
    {
      com_err(whoami, n, " from remote server: can't update %s", pathname);
      close(fd);
      return n;
    }

  if (encrypt)
    {
      des_key_sched(session, sched);
      memmove(ivec, session, sizeof(ivec));
    }

  while (n_to_send > 0)
    {
      n = read(fd, STRING_DATA(data), UPDATE_BUFSIZ);
      if (n < 0)
	{
	  com_err(whoami, errno, " reading %s for transmission", pathname);
	  close(fd);
	  return MR_ABORTED;
	}
      MAX_STRING_SIZE(data) = n;
      if (encrypt)
	{
	  src = (unsigned char *)STRING_DATA(data);
	  memmove(dst, src, n);
	  memset(dst + n, 0, 7);
	  /* encrypt! */
	  des_pcbc_encrypt(dst, src, n, sched, ivec, 0);
	  /* save vector to continue chaining */
	  for (i = 0; i < 8; i++)
	    ivec[i] = dst[n - 8 + i] ^ src[n - 8 + i];
	  /* round up to multiple of 8 */
	  data.length = (data.length + 7) & 0xfffffff8;
	}
      code = send_object(conn, (char *)&data, STRING_T);
      if (code)
	{
	  com_err(whoami, connection_errno(conn), " transmitting file %s",
		  pathname);
	  close(fd);
	  return connection_errno(conn);
	}
      n_to_send -= n;
      code = receive_object(conn, (char *)&n, INTEGER_T);
      if (code)
	{
	  com_err(whoami, connection_errno(conn),
		  " awaiting ACK remote server during transmission of %s",
		  pathname);
	  close(fd);
	  return connection_errno(conn);
	}
      if (n)
	{
	  com_err(whoami, n, " from remote server during transmission of %s",
		  pathname);
	  close(fd);
	  return n;
	}
    }
  if (statb.st_size == 0)
    {
      code = receive_object(conn, (char *)&n, INTEGER_T);
      if (code)
	{
	  com_err(whoami, connection_errno(conn),
		  " awaiting ACK remote server after transmission of %s",
		  pathname);
	  close(fd);
	  return connection_errno(conn);
	}
      if (n)
	{
	  com_err(whoami, n, " from remote server after transmission of %s",
		  pathname);
	  close(fd);
	  return n;
	}
    }
  close(fd);
  return MR_SUCCESS;
}
