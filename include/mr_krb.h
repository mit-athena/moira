/* $Id: mr_krb.h 3956 2010-01-05 20:56:56Z zacheiss $
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
