/* $Id: sendrecv.c,v 1.1 1998-02-15 17:49:29 danw Exp $
 *
 * socket layer for update_server
 *
 * Copyright (C) 1997-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>

#include <sys/uio.h>

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/sendrecv.c,v 1.1 1998-02-15 17:49:29 danw Exp $");

#define putlong(cp, l) { cp[0] = l >> 24; cp[1] = l >> 16; cp[2] = l >> 8; cp[3] = l; }
#define getlong(cp, l) l = ((cp[0] * 256 + cp[1]) * 256 + cp[2]) * 256 + cp[3]

extern void fail(int conn, int err, char *msg);

int send_int(int conn, long data)
{
  char buf[8];

  putlong(buf, 4);
  putlong((buf + 4), data);
  if (write(conn, buf, 8) == 8)
    return 0;
  else
    {
      fail(conn, errno, "sending integer");
      return errno;
    }
}

int recv_int(int conn, long *data)
{
  char buf[8];

  if (read(conn, buf, 8) != 8)
    {
      fail(conn, errno, "reading integer");
      return errno;
    }
  getlong((buf + 4), *data);
  return 0;
}


int send_string(int conn, char *buf, size_t len)
{
  char fulllen[4], stringlen[4];
  struct iovec iov[3];

  putlong(fulllen, (len + 4));
  putlong(stringlen, len);
  iov[0].iov_base = fulllen;
  iov[0].iov_len = 4;
  iov[1].iov_base = stringlen;
  iov[1].iov_len = 4;
  iov[2].iov_base = buf;
  iov[2].iov_len = len;

  if (writev(conn, iov, 3) == -1)
    {
      fail(conn, errno, "sending string");
      return errno;
    }
  else
    return 0;
}

int recv_string(int conn, char **buf, size_t *len)
{
  char tmp[4];

  if (read(conn, tmp, 4) != 4)
    {
      fail(conn, errno, "reading string");
      return errno;
    }
  if (read(conn, tmp, 4) != 4)
    {
      fail(conn, errno, "reading string");
      return errno;
    }
  getlong(tmp, *len);

  *buf = malloc(*len);
  if (!*buf)
    {
      fail(conn, ENOMEM, "reading string");
      return ENOMEM;
    }
  if (read(conn, *buf, *len) != *len)
    {
      free(buf);
      fail(conn, errno, "reading string");
      return errno;
    }

  return 0;
}
