/* $Id: xfer_002.c,v 1.12 1998-02-05 22:52:04 danw Exp $
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

#include <gdb.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/xfer_002.c,v 1.12 1998-02-05 22:52:04 danw Exp $");

extern CONNECTION conn;
extern char buf[BUFSIZ];

extern int code;

extern int have_authorization, have_file, done;

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

int xfer_002(char *str)
{
  int file_size;
  int checksum;
  char *pathname;

  str += 8;
  while (*str == ' ')
    str++;
  if (!*str)
    {
    failure:
      reject_call(MR_ARGS);
      return 0;
    }
  file_size = atoi(str);
  while (isdigit(*str))
    str++;
  while (*str == ' ')
    str++;
  checksum = atoi(str);
  while (isdigit(*str))
    str++;
  while (*str == ' ')
    str++;
  if (*str != '/')
    goto failure;
  pathname = str;
  if (!have_authorization)
    {
      reject_call(MR_PERM);
      return 0;
    }
  if (done)			/* re-initialize data */
    initialize();
  code = send_ok();
  if (code)
    lose("sending ok for file xfer (2)");
  code = get_file(pathname, file_size, checksum, 0700, 0);
  if (!code)
    {
      char buf[BUFSIZ];
      have_file = 1;
      strcpy(buf, "transferred file ");
      strcat(buf, pathname);
      mr_log_info(buf);
    }
  return 0;
}
