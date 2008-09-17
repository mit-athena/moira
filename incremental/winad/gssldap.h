/*
 * Copyright (C) 1998-2000 Luke Howard. All rights reserved.
 * CONFIDENTIAL
 * $Id$
 *
 * Public client API for GSS-API LDAP SASL mechanism.
 */

#ifndef _GSSLDAP_H_
#define _GSSLDAP_H_

#include <lber.h>
#include <ldap.h>

LDAP_API(int)
LDAP_CALL ldap_adgssapi_bind(LDAP * ld, const char *who, int layer);
LDAP_API(int)
LDAP_CALL ldap_gssapi_debug(int on);

#endif				/* _GSSLDAP_H_ */
