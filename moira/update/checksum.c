/* $Id: checksum.c,v 1.8 1998-02-05 22:51:57 danw Exp $
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include "update_server.h"

#include <stdio.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/checksum.c,v 1.8 1998-02-05 22:51:57 danw Exp $");

/*
 * checksum_fd(fd)
 * returns 24-bit checksum of bytes in file
 */

int checksum_file(char *path)
{
  int sum;
  int ch;
  FILE *f;

  sum = 0;
  f = fopen(path, "r");
  while ((ch = getc(f)) != EOF)
    sum = (sum + ch) & ((1 << 24) - 1);
  fclose(f);
  return sum;
}
