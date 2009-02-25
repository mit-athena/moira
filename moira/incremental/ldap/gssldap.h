/*
 * Copyright (C) 1998-2000 Luke Howard. All rights reserved.
 * CONFIDENTIAL
 * $Id: gssldap.h,v 1.1 2009-02-25 22:29:51 zacheiss Exp $
 *
 * Public client API for GSS-API LDAP SASL mechanism.
 */

#ifndef _GSSLDAP_H_
#define _GSSLDAP_H_

#include <lber.h>
#include <ldap.h>

LDAP_API(int)
LDAP_CALL ldap_adgssapi_bind(LDAP * ld, const char *who, int layer, 
			     char *ldap_domain_name, char *ldap_realm_name,
			     char *server);
LDAP_API(int)
LDAP_CALL ldap_gssapi_debug(int on);

#endif				/* _GSSLDAP_H_ */
