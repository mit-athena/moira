/* $Id: mr_init.c,v 1.10 1998-02-08 19:31:20 danw Exp $
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

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_init.c,v 1.10 1998-02-08 19:31:20 danw Exp $");

int mr_inited = 0;

/* the reference to link_against_the_moira_version_of_gdb is to make
 * sure that this is built with the proper libraries.
 */
void mr_init(void)
{
  extern int link_against_the_moira_version_of_gdb;
  if (mr_inited)
    return;

  gdb_init();
  initialize_sms_error_table();
  initialize_krb_error_table();
  link_against_the_moira_version_of_gdb = 0;
  mr_inited = 1;
}
