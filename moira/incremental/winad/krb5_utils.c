/*--
    krb5_utils.c

Abstract:

    ASN.1 encoder for the
    Kerberos Change Password Protocol (I-D) variant for Windows 2000

--*/

#include <krb5.h>
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

#ifndef T_SRV
#define T_SRV 33
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

static const char rcsid[] = "$Id: krb5_utils.c,v 1.1 2000-11-11 11:05:31 zacheiss Exp $";

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

krb5_error_code
krb5_locate_dns_srv(krb5_context context, const krb5_data *realm,
                    const char *service, const char *protocol,
                    struct sockaddr **addr_pp, int *naddrs)
{
  int             len;
  int             out;
  int             j;
  int             count;
  unsigned char   reply[1024];
  struct hostent  *hp;
  unsigned char   *p;
  char            host[128];
  int             status;
  int             priority;
  int             weight;
  u_short         port;
  struct sockaddr *addr_p = NULL;
  struct sockaddr_in *sin_p;
    
  out = 0;
  addr_p = (struct sockaddr *)malloc (sizeof (struct sockaddr));
  if (addr_p == NULL)
	  return(ENOMEM);
  count = 1;

#ifdef HAVE_SNPRINTF
  snprintf(host, sizeof(host), "%s.%s.%*s.",
	         service, protocol, realm->length, realm->data);
#else
  sprintf(host, "%s.%s.%*s.",
	        service, protocol, realm->length, realm->data);
#endif
  len = res_search(host, C_IN, T_SRV, reply, sizeof(reply));
  if (len >=0)
    {
  	  p = reply;
	    p += sizeof(HEADER);
	    status = dn_expand(reply, reply + len, p, host, sizeof(host));
  	  if (status < 0)
	      goto out;
	    p += status;
  	  p += 4;
  	  while (p < reply + len)
        {
	        int type, class, ttl, size;
	        status = dn_expand(reply, reply + len, p, host, sizeof(host));
	        if (status < 0)
		        goto out;
  	      p += status;
	        type = (p[0] << 8) | p[1];
	        p += 2;
	        class = (p[0] << 8) | p[1];
	        p += 2;
  	      ttl = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
	        p += 4;
	        size = (p[0] << 8) | p[1];
	        p += 2;
	        if (type == T_SRV)
            {
		          status = dn_expand(reply, reply + len, p + 6, host, sizeof(host));
  		        if (status < 0)
	  	          goto out;
		          priority = (p[0] << 8) | p[1];
		          weight = (p[2] << 8) | p[3];
		          port = (p[4] << 8) | p[5];
  		        hp = (struct hostent *)gethostbyname(host);
	  	        if (hp != 0)
                {
		              switch (hp->h_addrtype)
                    {
#ifdef KRB5_USE_INET
		                  case AF_INET:
			                  for (j=0; hp->h_addr_list[j]; j++)
                          {
			                      sin_p = (struct sockaddr_in *) &addr_p[out++];
			                      memset ((char *)sin_p, 0, sizeof(struct sockaddr));
			                      sin_p->sin_family = hp->h_addrtype;
			                      sin_p->sin_port = htons(port);
			                      memcpy((char *)&sin_p->sin_addr,
				                    (char *)hp->h_addr_list[j],
  				                  sizeof(struct in_addr));
	  		                    if (out+1 >= count)
                              {
			  	                      count += 5;
				                        addr_p = (struct sockaddr *)
				                        realloc ((char *)addr_p,
					                      sizeof(struct sockaddr) * count);
				                        if (!addr_p)
				                          goto out;
			                        }
			                    }
			                  break;
#endif
  		                default:
	  		                break;
		                }
		            }
		          p += size;
	          }
	      }
    }
    
out:
  if (out == 0)
    {
     free(addr_p);
     return(KRB5_REALM_CANT_RESOLVE);
    }

  *addr_pp = addr_p;
  *naddrs = out;
  return(0);
}

krb5_error_code 
krb5_locate_kpasswd(krb5_context context, const krb5_data *realm,
                    struct sockaddr **addr_pp, int *naddrs)
{
  krb5_error_code code;
	code = krb5_locate_dns_srv(context, realm, "_kpasswd", "_udp",
	                           addr_pp, naddrs);
  if (code)
    code = krb5_locate_dns_srv(context, realm, "_kpasswd", "_tcp",
                               addr_pp, naddrs);

  return(code);
}
