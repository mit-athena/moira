/* $Id: send_file.c,v 1.16 2001-09-04 19:46:21 zacheiss Exp $
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
#include <update.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/send_file.c,v 1.16 2001-09-04 19:46:21 zacheiss Exp $");

extern des_cblock session;

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

int mr_send_file(int conn, char *pathname, char *target_path, int encrypt)
{
  int n, fd, code, n_to_send, i;
  char data[UPDATE_BUFSIZ], enc[UPDATE_BUFSIZ];
  long response;
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

  sprintf(data, "XFER_00%c %d %d %s", (encrypt ? '3' : '2'), n_to_send,
	  checksum_file(pathname), target_path);
  code = send_string(conn, data, strlen(data) + 1);
  if (code)
    {
      com_err(whoami, code, "sending XFER request");
      close(fd);
      return code;
    }
  code = recv_int(conn, &response);
  if (code)
    {
      com_err(whoami, code, "getting reply from XFER request");
      close(fd);
      return code;
    }
  if (response)
    {
      com_err(whoami, response, "transfer request (XFER) rejected");
      close(fd);
      return response;
    }

  code = recv_int(conn, &response);
  if (code)
    {
      com_err(whoami, code, ": lost connection");
      close(fd);
      return code;
    }
  if (response)
    {
      com_err(whoami, response, " from remote server: can't update %s",
	      pathname);
      close(fd);
      return response;
    }

  if (encrypt)
    {
      des_key_sched(session, sched);
      memmove(ivec, session, sizeof(ivec));
    }

  while (n_to_send > 0)
    {
      n = read(fd, data, sizeof(data));
      if (n < 0)
	{
	  com_err(whoami, errno, "reading %s for transmission", pathname);
	  close(fd);
	  return MR_ABORTED;
	}
      if (encrypt)
	{
	  memset(data + n, 0, sizeof(data) -n);
	  des_pcbc_encrypt(data, enc, (n + 7) & ~7, sched, ivec, 0);
	  /* save vector to continue chaining */
	  for (i = 0; i < 8; i++)
	    ivec[i] = data[n - 8 + i] ^ enc[n - 8 + i];
	  /* round up to multiple of 8 */
	  n = (n + 7) & ~7;
	  code = send_string(conn, enc, n);
	}
      else
	code = send_string(conn, data, n);
      if (code)
	{
	  com_err(whoami, code, "transmitting file %s", pathname);
	  close(fd);
	  return code;
	}

      n_to_send -= n;
      code = recv_int(conn, &response);
      if (code)
	{
	  com_err(whoami, code, "awaiting ACK during transmission of %s",
		  pathname);
	  close(fd);
	  return code;
	}
      if (response)
	{
	  com_err(whoami, response,
		  "from remote server during transmission of %s",
		  pathname);
	  close(fd);
	  return response;
	}
    }

  if (statb.st_size == 0)
    {
      code = recv_int(conn, &response);
      if (code)
	{
	  com_err(whoami, code, "awaiting ACK after transmission of %s",
		  pathname);
	  close(fd);
	  return code;
	}
      if (response)
	{
	  com_err(whoami, response,
		  "from remote server after transmission of %s",
		  pathname);
	  close(fd);
	  return response;
	}
    }
  close(fd);
  return MR_SUCCESS;
}
