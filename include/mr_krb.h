/* $Id: mr_krb.h,v 1.1 2009-05-04 20:49:11 zacheiss Exp $
 *
 * Copyright (C) 2009 by the Massachusetts Institute of Technology
 *
 * Define some useful constants that used to be provided by the krb4
 * libraries.
 *
 */

#define ANAME_SZ      40
#define INST_SZ       40
#define REALM_SZ      40
/* include space for '.' and '@' */
#define MAX_K_NAME_SZ (ANAME_SZ + INST_SZ + REALM_SZ + 2)

#define KRB_REALM "ATHENA.MIT.EDU"

#define kname_parse mr_kname_parse

#define ERROR_TABLE_BASE_krb                     (39525376L)
