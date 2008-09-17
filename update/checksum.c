/* $Id$
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include "update_server.h"

#include <stdio.h>

RCSID("$Header$");

/*
 * checksum_fd(fd)
 * returns 24-bit checksum of bytes in file
 */

long checksum_file(char *path)
{
  long sum;
  int ch;
  FILE *f;

  sum = 0;
  f = fopen(path, "r");
  while ((ch = getc(f)) != EOF)
    sum = (sum + ch) & ((1 << 24) - 1);
  fclose(f);
  return sum;
}
