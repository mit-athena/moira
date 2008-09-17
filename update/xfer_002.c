/* $Id$
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

RCSID("$Header$");

/*
 *
 * syntax:
 * >>> (STRING)"xfer_002" filesize checksum pathname
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
 *
 * still to be done: file locking; perform transfers in pieces instead
 * of all at once; use checksums
 */

void xfer_002(int conn, char *str)
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
  code = get_file(conn, pathname, file_size, checksum, 0700, 0);
  if (!code)
    com_err(whoami, 0, "Transferred file %s", pathname);

  return;
}
