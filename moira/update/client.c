/* $Id: client.c,v 1.26 2001-01-08 19:28:11 zacheiss Exp $
 *
 * This code handles the actual distribution of data files
 * to servers in the Moira server-update program.
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include "update.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <des.h>
#include <krb.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/client.c,v 1.26 2001-01-08 19:28:11 zacheiss Exp $");

extern des_cblock session;
extern char *whoami;

int mr_send_auth(int conn, char *host_name)
{
  KTEXT_ST ticket_st;
  int code, auth_version = 2;
  long response;

  code = get_mr_update_ticket(host_name, &ticket_st);
  if (code)
    return code;
  code = send_string(conn, "AUTH_002", 9);
  if (code)
    return code;
  code = recv_int(conn, &response);
  if (code)
    return code;
  if (response)
    {
      code = send_string(conn, "AUTH_001", 9);
      if (code)
	return code;
      code = recv_int(conn, &response);
      if (code)
	return code;
      if (response)
	return response;
      auth_version = 1;
    }
  code = send_string(conn, (char *)ticket_st.dat, ticket_st.length);
  if (code)
    return code;
  code = recv_int(conn, &response);
  if (code)
    return code;
  if (response)
    return response;

  if (auth_version == 2)
    {
      des_key_schedule sched;
      C_Block enonce;
      char *data;
      size_t size;

      code = recv_string(conn, &data, &size);
      if (code)
	return code;
      des_key_sched(session, sched);
      des_ecb_encrypt(data, enonce, sched, 1);
      free(data);
      code = send_string(conn, (char *)enonce, sizeof(enonce));
      if (code)
	return code;
      code = recv_int(conn, &response);
      if (code)
	return code;
      if (response)
	return response;
    }

  return MR_SUCCESS;
}

int mr_execute(int conn, char *path)
{
  long response;
  char *data;
  int code;

  data = malloc(10 + strlen(path));
  if (!data)
    return ENOMEM;
  sprintf(data, "EXEC_002 %s", path);
  code = send_string(conn, data, strlen(data) + 1);
  free(data);
  if (code)
    return code;
  code = recv_int(conn, &response);
  if (code)
    return code;
  if (response)
    return response;

  return MR_SUCCESS;
}

void mr_send_quit(int conn)
{
  send_string(conn, "quit", 5);
}

void fail(int conn, int err, char *msg)
{
  com_err(whoami, err, msg);
  return;
}
