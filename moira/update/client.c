/* $Id: client.c,v 1.22 1998-02-05 22:51:58 danw Exp $
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

#include <stdio.h>

#include <des.h>
#include <gdb.h>
#include <krb.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/client.c,v 1.22 1998-02-05 22:51:58 danw Exp $");

extern int dbg;
extern C_Block session;

CONNECTION conn;

int send_auth(char *host_name)
{
  KTEXT_ST ticket_st;
  KTEXT ticket = &ticket_st;
  STRING data;
  int code;
  int response;
  int auth_version = 2;

  code = get_mr_update_ticket(host_name, ticket);
  if (code)
    return code;
  STRING_DATA(data) = "AUTH_002";
  MAX_STRING_SIZE(data) = 9;
  code = send_object(conn, (char *)&data, STRING_T);
  if (code)
    return connection_errno(conn);
  code = receive_object(conn, (char *)&response, INTEGER_T);
  if (code)
    return connection_errno(conn);
  if (response)
    {
      STRING_DATA(data) = "AUTH_001";
      MAX_STRING_SIZE(data) = 9;
      code = send_object(conn, (char *)&data, STRING_T);
      if (code)
	return connection_errno(conn);
      code = receive_object(conn, (char *)&response, INTEGER_T);
      if (code)
	return connection_errno(conn);
      if (response)
	return response;
      auth_version = 1;
    }
  STRING_DATA(data) = (char *)ticket->dat;
  MAX_STRING_SIZE(data) = ticket->length;
  code = send_object(conn, (char *)&data, STRING_T);
  if (code)
    return connection_errno(conn);
  code = receive_object(conn, (char *)&response, INTEGER_T);
  if (code)
    return connection_errno(conn);
  if (response)
    return response;

  if (auth_version == 2)
    {
      des_key_schedule sched;
      C_Block enonce;

      code = receive_object(conn, (char *)&data, STRING_T);
      if (code)
	return connection_errno(conn);
      des_key_sched(session, sched);
      des_ecb_encrypt(STRING_DATA(data), enonce, sched, 1);
      STRING_DATA(data) = enonce;
      code = send_object(conn, (char *)&data, STRING_T);
      if (code)
	return connection_errno(conn);
      code = receive_object(conn, (char *)&response, INTEGER_T);
      if (code)
	return connection_errno(conn);
      if (response)
	return response;
    }

  return MR_SUCCESS;
}

int execute(char *path)
{
  int response;
  STRING data;
  int code;

  string_alloc(&data, BUFSIZ);
  sprintf(STRING_DATA(data), "EXEC_002 %s", path);
  code = send_object(conn, (char *)&data, STRING_T);
  if (code)
    return connection_errno(conn);
  code = receive_object(conn, (char *)&response, INTEGER_T);
  if (code)
    return connection_errno(conn);
  if (response)
    return response;
  return MR_SUCCESS;
}

void send_quit(void)
{
  STRING str;
  if (!conn)
    return;
  string_alloc(&str, 5);
  strcpy(STRING_DATA(str), "quit");
  send_object(conn, (char *)&str, STRING_T);
  string_free(&str);
}
