/* $Id: mr_init.c,v 1.11 1998-02-15 17:49:03 danw Exp $
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

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_init.c,v 1.11 1998-02-15 17:49:03 danw Exp $");

int mr_inited = 0;

void mr_init(void)
{
  if (mr_inited)
    return;

  initialize_sms_error_table();
  initialize_krb_error_table();
  initialize_gdss_error_table();
  initialize_ureg_error_table();
  mr_inited = 1;
}
