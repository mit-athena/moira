/*--

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1999  Microsoft Corporation.  All rights reserved.

Module Name:

    kpasswd.h

Abstract:

    Definitions for the Kerberos change password functions

--*/

/* changepw.h */
krb5_error_code krb5_set_password
	KRB5_PROTOTYPE((krb5_context, krb5_ccache, char *, char *, char *, int *));

/*krb5_error_code 
(krb5_context, const krb5_data *, struct sockaddr **, int *);
*/

/* password change constants */

typedef struct _krb5_setpw {
    	krb5_magic	magic;
    	krb5_data 	newpasswd;
    	krb5_principal	targprinc;
} krb5_setpw;

#ifndef KRB5_KPASSWD_SUCCESS
#define KRB5_KPASSWD_SUCCESS		0
#define KRB5_KPASSWD_MALFORMED		1
#define KRB5_KPASSWD_HARDERROR		2
#define KRB5_KPASSWD_AUTHERROR		3
#define KRB5_KPASSWD_SOFTERROR		4
#endif
#define KRB5_KPASSWD_ACCESSDENIED	5
#define KRB5_KPASSWD_BAD_VERSION	6
#define KRB5_KPASSWD_INITIAL_FLAG_NEEDED 7

#define KRB5_KPASSWD_VERS_CHANGEPW	1
#define KRB5_KPASSWD_VERS_SETPW		0xff80

#ifndef DEFAULT_KPASSWD_PORT
#define DEFAULT_KPASSWD_PORT	464
#endif
