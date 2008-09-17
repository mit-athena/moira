/*
 * Copyright (C) 1998-2000 Luke Howard. All rights reserved.
 * CONFIDENTIAL
 * $Id$
 *
 * General GSS-API SASL (not LDAP specific) stuff.
 */

#ifndef _GSSSASL_H_
#define _GSSSASL_H_

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef HEIMDAL
#include <gssapi.h>
#else
#include <gssapi/gssapi_generic.h>
#endif
#ifndef _WIN32
#include <netinet/in.h>
#endif

#define OM_uint32 ULONG

#define GSSAPI_SASL_NAME                  "GSSAPI"

/* GSS_Accept_Context */
#define GSSSASL_NEGOTIATE_CONTEXT         0x01
/* Section 7.2.2 of RFC 2222 */
#define GSSSASL_SEND_SECURITY_OPTIONS     0x02
/* Section 7.2.1 of RFC 2222 */
#define GSSSASL_RECEIVE_SECURITY_OPTIONS  0x03
#define GSSSASL_COMPLETED                 0x04

/* Section 7.3 of RFC 2222 */
#define GSSSASL_NO_SECURITY_LAYER         0x01
#define GSSSASL_INTEGRITY_PROTECTION      0x02
#define GSSSASL_PRIVACY_PROTECTION        0x04

/* Token for security negotiation (after GSS-API nego) */
typedef struct gsssasl_security_negotiation_desc {
    /* Section 7.2.3 of RFC 2222 */
    int security_layer:8;
    /* Section 7.2.[12] of RFC 2222 */
    int token_size:24;
    /* Section 7.2.1 of RFC 2222 */
    char identity[1];
} gsssasl_security_negotiation_desc, *gsssasl_security_negotiation_t;

typedef int gsssasl_server_negotiation_desc, *gsssasl_server_negotiation_t;

#endif				/* _GSSSASL_H_ */
