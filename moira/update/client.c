/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/client.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/client.c,v 1.21 1998-01-07 17:13:41 danw Exp $
 */

#ifndef lint
static char *rcsid_client2_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/client.c,v 1.21 1998-01-07 17:13:41 danw Exp $";
#endif	lint

/*
 * MODULE IDENTIFICATION:
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/client.c,v 1.21 1998-01-07 17:13:41 danw Exp $
 *	Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 * DESCRIPTION:
 *	This code handles the actual distribution of data files
 *	to servers in the Moira server-update program.
 * AUTHOR:
 *	Ken Raeburn (spook@athena.MIT.EDU),
 *		MIT Project Athena/MIT Information Systems.
 * DEFINED VALUES:
 *	conn
 *	mr_update_server
 */

#include <mit-copyright.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gdb.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <update.h>
#include <errno.h>
#include <moira.h>
#include <moira_site.h>
#include <krb.h>

extern int errno, dbg;
extern C_Block session;

static char buf[BUFSIZ];
static int code;

CONNECTION conn;


/*
 * FUNCTION:
 *	initialize()
 * DESCRIPTION:
 *	Insures that various libraries have a chance to get
 *	initialized.
 * INPUT:
 * OUTPUT:
 * RETURN VALUE:
 *	void
 * SIDE EFFECTS:
 *	Initializes GDB library.
 * PROBLEMS:
 *
 */
static void initialize()
{
  static int initialized = 0;

  if (!initialized)
    {
      gdb_init();
      initialized++;
    }
}

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

send_quit(void)
{
  STRING str;
  if (!conn)
    return;
  string_alloc(&str, 5);
  strcpy(STRING_DATA(str), "quit");
  send_object(conn, (char *)&str, STRING_T);
  string_free(&str);
}
