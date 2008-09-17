/*
 * Copyright (C) 1998-2000 Luke Howard. All rights reserved.
 * CONFIDENTIAL
 * $Id$
 *
 * Structures for handling GSS-API binds within LDAP client
 * and server libraries. This isn't entirely SASL specific,
 * apart from the negotiation state.
 */

#ifndef _GSSLDAP_INT_H_
#define _GSSLDAP_INT_H_

#define GSSAPI_LDAP_PRINCIPAL_ATTRIBUTE	"krbName"
#define GSSAPI_LDAP_SERVICE_NAME	"ldap"
#define GSSAPI_LDAP_DN_PREFIX		"dn:"
#define GSSAPI_LDAP_DN_PREFIX_LEN	(sizeof(GSSAPI_LDAP_DN_PREFIX) - 1)

#include "gsssasl.h"
#include "gssldap.h"

/*
 * GSS-API SASL negotiation state for the client library
 */
typedef struct gssldap_client_state_desc {
    /* LDAP handle */
    LDAP *ld;

    /* dn passed to ldap_gssapi_bind() */
    const char *binddn;

    /* msgid for bind conversation */
    int msgid;

    /* GSS-API context */
    gss_ctx_id_t context;

    /* result code to return from ldap_gssapi_bind() */
    int rc;
} gssldap_client_state_desc, *gssldap_client_state_t;

/*
 * Plugin (server side) state
 */
typedef struct gssldap_server_state_desc {
    /* The LDAP connection */
    int conn;

    /* Where in the negotiation we are */
    gsssasl_server_negotiation_desc state;

    /* The GSS-API context */
    gss_ctx_id_t context;

    /* The client name */
    gss_buffer_desc client_name;

    /* SASL authorization identity (with dn: prefix chopped) */
    char *identity;

    /* The next entry in the list */
    struct gssldap_server_state_desc *next;
} gssldap_server_state_desc, *gssldap_server_state_t;

#endif				/* _GSSLDAP_INT_H_ */
