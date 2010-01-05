/* $Id$
 *
 * Pass an mr_params off to the Moira server and get a reply
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include "mr_private.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <netinet/in.h>
#endif /* _WIN32 */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

RCSID("$HeadURL$ $Id$");

/* Moira RPC format:

   4-byte total length (including these 4 bytes)
   4-byte version number (MR_VERSION_2 == 2)
   4-byte opcode (from client) or status (from server)
   4-byte argc

   4-byte len, followed by null-terminated string, padded to 4-byte boundary
     (the len doesn't include the padding)
   ...

   (followed by more packets if status was MR_MORE_DATA)

   All numbers are in network byte order.
*/

int mr_do_call(struct mr_params *params, struct mr_params *reply)
{
  int status;

  CHECK_CONNECTED;

  status = mr_send(_mr_conn, params);
  if (status == MR_SUCCESS)
    status = mr_receive(_mr_conn, reply);

  if (status)
    mr_disconnect();

  return status;
}

int mr_send(int fd, struct mr_params *params)
{
  u_long length;
  int written;
  int i, *argl;
  char *buf, *p;

  length = 16; /* length + version + opcode/status + argc */

  if (params->mr_argl)
    {
      argl = params->mr_argl;
      for (i = 0; i < params->mr_argc; i++)
	length += 8 + argl[i];
    }
  else
    {
      argl = malloc(params->mr_argc * sizeof(int));
      if (params->mr_argc && !argl)
	return ENOMEM;
      for (i = 0; i < params->mr_argc; i++)
	{
	  argl[i] = strlen(params->mr_argv[i]) + 1;
	  length += 8 + argl[i];
	}
    }

  buf = malloc(length);
  if (!buf)
    {
      if (!params->mr_argl)
	free(argl);
      return ENOMEM;
    }
  memset(buf, 0, length);

  putlong(buf + 4, MR_VERSION_2);
  putlong(buf + 8, params->u.mr_procno);
  putlong(buf + 12, params->mr_argc);

  for (i = 0, p = buf + 16; i < params->mr_argc; i++)
    {
      putlong(p, argl[i]);
      memcpy(p += 4, params->mr_argv[i], argl[i]);
      p += argl[i] + (4 - argl[i] % 4) % 4;
    }
  length = p - buf;
  putlong(buf, length);

  written = send(fd, buf, length, 0);
  free(buf);
  if (!params->mr_argl)
    free(argl);

  if (written != (int)length)
    return MR_ABORTED;
  else
    return MR_SUCCESS;
}

int mr_receive(int fd, struct mr_params *reply)
{
  int status;

  memset(reply, 0, sizeof(struct mr_params));
  do
    status = mr_cont_receive(fd, reply);
  while (status == -1);

  return status;
}
  
/* Read some or all of a client response, without losing if it turns
 * out to be malformed. Returns MR_SUCCESS on success, an error code
 * on failure, or -1 if the packet hasn't been completely received
 * yet.
 */

int mr_cont_receive(int fd, struct mr_params *reply)
{
  u_long length, data;
  int size, more;
  char *p, *end;
  int i;

  if (!reply->mr_flattened)
    {
      char lbuf[4];

      size = recv(fd, lbuf, 4, 0);
      if (size != 4)
	return size ? MR_ABORTED : MR_NOT_CONNECTED;
      getlong(lbuf, length);
      if (length > 8192)
	return MR_INTERNAL;
      reply->mr_flattened = malloc(length);
      if (!reply->mr_flattened)
	return ENOMEM;
      memcpy(reply->mr_flattened, lbuf, 4);
      reply->mr_filled = 4;

      return -1;
    }
  else
    getlong(reply->mr_flattened, length);

  more = recv(fd, reply->mr_flattened + reply->mr_filled,
	      length - reply->mr_filled, 0);
  if (more == -1)
    {
      mr_destroy_reply(*reply);
      return MR_ABORTED;
    }

  reply->mr_filled += more;

  if (reply->mr_filled != length)
    return -1;

  getlong(reply->mr_flattened + 4, data);
  if (data != MR_VERSION_2)
    {
      mr_destroy_reply(*reply);
      return MR_VERSION_MISMATCH;
    }

  getlong(reply->mr_flattened + 8, reply->u.mr_status);
  getlong(reply->mr_flattened + 12, reply->mr_argc);
  if (reply->mr_argc > ((int)length - 16) / 8)
    {
      mr_destroy_reply(*reply);
      return MR_INTERNAL;
    }
  reply->mr_argv = malloc(reply->mr_argc * sizeof(char *));
  reply->mr_argl = malloc(reply->mr_argc * sizeof(int));
  if (reply->mr_argc && (!reply->mr_argv || !reply->mr_argl))
    {
      mr_destroy_reply(*reply);
      return ENOMEM;
    }

  p = (char *)reply->mr_flattened + 16;
  end = (char *)reply->mr_flattened + length;
  for (i = 0; i < reply->mr_argc && p + 4 <= end; i++)
    {
      getlong(p, reply->mr_argl[i]);
      if (p + 4 + reply->mr_argl[i] > end)
	break;
      reply->mr_argv[i] = p + 4;
      p += 4 + reply->mr_argl[i] + (4 - reply->mr_argl[i] % 4) % 4;
    }

  if (i != reply->mr_argc)
    {
      mr_destroy_reply(*reply);
      return MR_INTERNAL;
    }

  return MR_SUCCESS;
}

void mr_destroy_reply(mr_params reply)
{
  free(reply.mr_argl);
  free(reply.mr_argv);
  free(reply.mr_flattened);
}
