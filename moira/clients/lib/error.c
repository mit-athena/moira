/* $Id$
 *
 * mrcl error interface
 *
 * Copyright (C) 1999 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include <mrclient.h>
#include "mrclient-internal.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <com_err.h>

RCSID("$HeadURL$ $Id$");

static char *mrcl_message = NULL;

void mrcl_set_message(char *fmt, ...)
{
  int args, len;
  char *p;
  va_list ap;

  free(mrcl_message);

  /* Count "%s"s */
  for (args = 0, p = strstr(fmt, "%s"); p; p = strstr(p + 2, "%s"))
    args++;

  /* Measure the output string. */
  len = strlen(fmt) + 1;
  va_start(ap, fmt);
  while (args--)
    {
      p = va_arg(ap, char *);
      len += strlen(p);
    }
  va_end(ap);

  /* Malloc and print */
  mrcl_message = malloc(len);
  if (mrcl_message)
    {
      va_start(ap, fmt);
      vsprintf(mrcl_message, fmt, ap);
      va_end(ap);
    }
}

char *mrcl_get_message(void)
{
  return mrcl_message;
}

void mrcl_clear_message(void)
{
  free(mrcl_message);
  mrcl_message = NULL;
}

void mrcl_com_err(char *whoami)
{
  if (mrcl_message)
    com_err(whoami, 0, "%s", mrcl_message);
}
