/* $Id: xfer_003.c 3956 2010-01-05 20:56:56Z zacheiss $
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include "update_server.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

RCSID("$HeadURL: svn+ssh://svn.mit.edu/moira/trunk/moira/update/xfer_003.c $ $Id: xfer_003.c 3956 2010-01-05 20:56:56Z zacheiss $");

/*
 *
 * syntax:
 * >>> (STRING)"xfer_003" filesize checksum pathname
 * <<< (int)0
 * >>> (STRING)data
 * <<< (int)code
 * >>> (STRING)data
 * <<< (int)code
 * ...
 * >>> (STRING)data	(last data block)
 * <<< (int)code	(from read, write, checksum verify)
 *
 * function:
 *	perform initial preparations and receive file as
 * a single string, storing it into <pathname>.moira_update.
 * this version of transfer encrypts the file being transferred.
 */

void xfer_003(int conn, char *str)
{
  int file_size, checksum, code;
  char *pathname, *p;

  str += 8;
  while (*str == ' ')
    str++;
  if (!*str)
    {
    failure:
      send_int(conn, MR_ARGS);
      return;
    }

  file_size = strtol(str, &p, 10);
  if (p == str)
    {
      send_int(conn, MR_ARGS);
      return;
    }
  else
    str = p;
  while (*str == ' ')
    str++;

  checksum = strtol(str, &p, 10);
  if (p == str)
    {
      send_int(conn, MR_ARGS);
      return;
    }
  else
    str = p;
  while (*str == ' ')
    str++;

  if (*str != '/')
    goto failure;
  pathname = str;

  if (!have_authorization)
    {
      send_int(conn, MR_PERM);
      return;
    }

  send_ok(conn);
  code = get_file(conn, pathname, file_size, checksum, 0444, 1);
  if (!code)
    com_err(whoami, 0, "Transferred file %s", pathname);

  return;
}
