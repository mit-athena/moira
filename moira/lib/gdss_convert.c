/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/gdss_convert.c,v 1.2 1998-01-05 19:53:03 danw Exp $
 *
 * Convert from GDSS error codes to com_err error codes
 */

#include <gdss_et.h>

int gdss2et(int code)
{
  if (code == 0)
    return code;
  return ERROR_TABLE_BASE_gdss - code - 1;
}
