/* $Id$
 *
 * socket layer for update_server
 *
 * Copyright (C) 1997-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>

#include <sys/types.h>
#include <sys/uio.h>

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

RCSID("$HeadURL$ $Id$");

#define putlong(cp, l) { ((unsigned char *)cp)[0] = l >> 24; ((unsigned char *)cp)[1] = l >> 16; ((unsigned char *)cp)[2] = l >> 8; ((unsigned char *)cp)[3] = l; }
#define getlong(cp, l) l = ((((unsigned char *)cp)[0] * 256 + ((unsigned char *)cp)[1]) * 256 + ((unsigned char *)cp)[2]) * 256 + ((unsigned char *)cp)[3]

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
  int len;

  len = read(conn, buf, 8);
  if (len == -1)
    {
      fail(conn, errno, "reading integer");
      return errno;
    }
  else if (len < 8)
    {
      fail(conn, 0, "remote connection closed while reading integer");
      return EIO;
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
  int size, more;

  size = read(conn, tmp, 4);
  if (size == -1)
    {
      fail(conn, errno, "reading string");
      return errno;
    }
  else if (size < 4)
    {
      fail(conn, 0, "remote connection closed while reading string");
      return EIO;
    }

  size = read(conn, tmp, 4);
  if (size == -1)
    {
      fail(conn, errno, "reading string");
      return errno;
    }
  else if (size < 4)
    {
      fail(conn, 0, "remote connection closed while reading string");
      return EIO;
    }
  getlong(tmp, *len);

  *buf = malloc(*len);
  if (!*buf)
    {
      fail(conn, ENOMEM, "reading string");
      return ENOMEM;
    }
  for (size = 0; size < *len; size += more)
    {
      more = read(conn, *buf + size, *len - size);
      if (!more)
	break;
    }
  
  if (size != *len)
    {
      free(buf);
      fail(conn, errno, "reading string");
      return errno;
    }

  return 0;
}
