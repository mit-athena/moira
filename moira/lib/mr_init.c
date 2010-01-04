/* $Id: mr_init.c,v 1.12 2000-01-28 00:03:30 danw Exp $
 *
 * Initialize libmoira
 *
 * Copyright (C) 1987-1990 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include "mr_private.h"

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_init.c,v 1.12 2000-01-28 00:03:30 danw Exp $");

int mr_inited = 0;

void mr_init(void)
{
  if (mr_inited)
    return;

#if defined(__APPLE__) && defined(__MACH__)
  add_error_table(&et_sms_error_table);
  add_error_table(&et_krb_error_table);
  add_error_table(&et_ureg_error_table);
#else
  initialize_sms_error_table();
  initialize_krb_error_table();
  initialize_ureg_error_table();
#endif

  mr_inited = 1;
}
