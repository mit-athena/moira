/* $Id$
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include "update_server.h"

#include <sys/utsname.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>

RCSID("$HeadURL$ $Id$");

static char service[] = "rcmd";
static char master[] = "sms";
static char qmark[] = "???";

/*
 * authentication request auth_002:
 *
 * >>> (STRING) "auth_002"
 * <<< (int) 0
 * >>> (STRING) ticket
 * <<< (int) code
 * <<< (STRING) nonce
 * >>> (STRING) encrypted nonce
 * <<< (int) code
 *
 */

void auth_002(int conn, char *str)
{
  send_int(conn, MR_NO_KRB4);
}
