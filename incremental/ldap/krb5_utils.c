/*--
    krb5_utils.c

Abstract:

    ASN.1 encoder for the
    Kerberos Change Password Protocol (I-D) variant for Windows 2000

--*/

#include <krb5.h>
#include <ldap.h>
#ifdef _WIN32
#include "asn1_make.h"
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#define NEED_SOCKETS
#ifndef _WIN32
#include <netdb.h>
#include <sys/socket.h>
#endif
#include <stdio.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include "kpasswd.h"

#ifndef KRB5_USE_INET
#ifdef HAVE_NETINET_IN_H
#define KRB5_USE_INET 1
#endif
#endif

#ifndef _WIN32
typedef krb5_octet asn1_octet;
typedef krb5_error_code asn1_error_code;
typedef struct code_buffer_rep {
  char *base, *bound, *next;
} asn1buf;
typedef enum { UNIVERSAL = 0x00, APPLICATION = 0x40,
               CONTEXT_SPECIFIC = 0x80, PRIVATE = 0xC0 } asn1_class;
#endif

static const char rcsid[] = "$Id: krb5_utils.c,v 1.1 2009-02-25 22:29:51 zacheiss Exp $";

asn1_error_code asn1_encode_realm(asn1buf *buf, const krb5_principal val, 
                                  int *retlen);
asn1_error_code asn1_encode_principal_name(asn1buf *buf, 
                                           const krb5_principal val, 
                                           int *retlen);
asn1_error_code asn1_encode_octetstring(asn1buf *buf, const int len, 
                                        const asn1_octet *val, int *retlen);

/* From src/lib/krb5/asn.1/krb5_encode.c */

/* setup() -- create and initialize bookkeeping variables
     retval: stores error codes returned from subroutines
     buf: the coding buffer
     length: length of the most-recently produced encoding
     sum: cumulative length of the entire encoding */
#define krb5_setup()\
  asn1_error_code retval;\
  asn1buf *buf=NULL;\
  int length, sum=0;\
\
  if(rep == NULL) return ASN1_MISSING_FIELD;\
\
  retval = asn1buf_create(&buf);\
  if(retval) return retval


/* krb5_addfield -- add a field, or component, to the encoding */
#define krb5_addfield(value,tag,encoder)\
{ retval = encoder(buf,value,&length);\
  if(retval){\
    asn1buf_destroy(&buf);\
    return retval; }\
  sum += length;\
  retval = asn1_make_etag(buf,CONTEXT_SPECIFIC,tag,length,&length);\
  if(retval){\
    asn1buf_destroy(&buf);\
    return retval; }\
  sum += length; }

/* krb5_addlenfield -- add a field whose length must be separately specified */
#define krb5_addlenfield(len,value,tag,encoder)\
{ retval = encoder(buf,len,value,&length);\
  if(retval){\
    asn1buf_destroy(&buf);\
    return retval; }\
  sum += length;\
  retval = asn1_make_etag(buf,CONTEXT_SPECIFIC,tag,length,&length);\
  if(retval){\
    asn1buf_destroy(&buf);\
    return retval; }\
  sum += length; }

/* form a sequence (by adding a sequence header to the current encoding) */
#define krb5_makeseq()\
  retval = asn1_make_sequence(buf,sum,&length);\
  if(retval){\
    asn1buf_destroy(&buf);\
    return retval; }\
  sum += length

/* produce the final output and clean up the workspace */
#define krb5_cleanup()\
  retval = asn12krb5_buf(buf,code);\
  if(retval){\
    asn1buf_destroy(&buf);\
    return retval; }\
  retval = asn1buf_destroy(&buf);\
  if(retval){\
    return retval; }\
\
  return(0)

krb5_error_code encode_krb5_setpw(const krb5_setpw *rep,
                                  krb5_data ** code)
{
  krb5_setup();

  if (rep->targprinc != NULL)
    {  /* target principal name is OPTIONAL */
      krb5_addfield(rep->targprinc,2,asn1_encode_realm);
      krb5_addfield(rep->targprinc,1,asn1_encode_principal_name);
    }
  krb5_addlenfield(rep->newpasswd.length, rep->newpasswd.data, 
                   0, asn1_encode_octetstring);
  krb5_makeseq();
  krb5_cleanup();
}

