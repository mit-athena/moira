/* $Id: genwords.c,v 1.1 1998-08-06 18:13:42 danw Exp $
 *
 * Utility program to generate magic words for a given ID
 *
 * Copyright (C) 1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 */

#include <mit-copyright.h>
#include <moira.h>
#include "reg_svr.h"

#include <stdio.h>
#include <stdlib.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/genwords.c,v 1.1 1998-08-06 18:13:42 danw Exp $");

int main(int argc, char **argv)
{
  char *words[6];

  if (argc != 2)
    {
      fprintf(stderr, "Usage: genwords id\n");
      exit(1);
    }

  if (!read_hmac_key())
    {
      fprintf(stderr, "Couldn't read hmac key\n");
      exit(1);
    }

  getwordlist(argv[1], words);
  printf("%s %s %s %s %s %s\n", words[0], words[1], words[2],
	 words[3], words[4], words[5]);
}

/* used by words.c */
void *xmalloc(size_t size)
{
  /* we won't run out of memory here */
  return malloc(size);
}
