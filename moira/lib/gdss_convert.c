/* $Id $
 *
 * Convert from GDSS error codes to com_err error codes
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>

#include <gdss_et.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/gdss_convert.c,v 1.3 1998-02-05 22:51:22 danw Exp $");

int gdss2et(int code)
{
  if (code == 0)
    return code;
  return ERROR_TABLE_BASE_gdss - code - 1;
}
