/*--
Module Name:

    kpasswd.h

--*/


typedef struct _krb5_setpw {
  krb5_magic      magic;
  krb5_data       newpasswd;
  krb5_principal  targprinc;
} krb5_setpw;


#define MAX_SERVER_NAMES 32
#ifndef T_SRV
#define T_SRV 33
#endif
#define LDAP_SERVICE  "_ldap"
#define TCP_PROTOCOL  "_tcp"

#define KDC_RECEIVE_TIMEOUT       10
#define KDC_RECEIVE_ERROR         11
#define KDC_SEND_ERROR            12
#define KDC_GETSOCKNAME_ERROR     13
#define KDC_GETPEERNAME_ERROR     14

#ifndef KRB5_KPASSWD_SUCCESS
#define KRB5_KPASSWD_SUCCESS              0
#define KRB5_KPASSWD_MALFORMED            1
#define KRB5_KPASSWD_HARDERROR            2
#define KRB5_KPASSWD_AUTHERROR            3
#define KRB5_KPASSWD_SOFTERROR            4
#endif
#define KRB5_KPASSWD_ACCESSDENIED         5
#define KRB5_KPASSWD_BAD_VERSION          6
#define KRB5_KPASSWD_INITIAL_FLAG_NEEDED  7

#define KRB5_KPASSWD_VERS_CHANGEPW        1
#define KRB5_KPASSWD_VERS_SETPW           0xff80

#ifndef DEFAULT_KPASSWD_PORT
#define DEFAULT_KPASSWD_PORT 464
#endif

