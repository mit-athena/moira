/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/checksum.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/checksum.c,v 1.6 1998-01-05 19:53:51 danw Exp $
 */
/*  (c) Copyright 1988 by the Massachusetts Institute of Technology. */
/*  For copying and distribution information, please see the file */
/*  <mit-copyright.h>. */

#ifndef lint
static char *rcsid_checksum_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/checksum.c,v 1.6 1998-01-05 19:53:51 danw Exp $";
#endif

#include <mit-copyright.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>

/*
 * checksum_fd(fd)
 * returns 24-bit checksum of bytes in file
 */

int checksum_file(char *path)
{
  register int sum;
  register int ch;
  register FILE *f;

  sum = 0;
  f = fopen(path, "r");
  while ((ch = getc(f)) != EOF)
    sum = (sum + ch) & ((1 << 24) - 1);
  fclose(f);
  return sum;
}
