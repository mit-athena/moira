/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/gdss_convert.c,v 1.1 1992-06-24 17:08:42 mar Exp $
 *
 * Convert from GDSS error codes to com_err error codes
 */

#include <gdss_et.h>

gdss2et(code)
int code;
{
    if (code == 0)
      return(code);
    return(ERROR_TABLE_BASE_gdss - code - 1);
}
