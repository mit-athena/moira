/*--

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1999  Microsoft Corporation.  All rights reserved.

Module Name:

    setpw.c

Abstract:

    Set a user's password using the
    Kerberos Change Password Protocol (I-D) variant for Windows 2000

--*/
/*
 * lib/krb5/os/changepw.c
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * Export of this software from the United States of America may 
 *   require a specific license from the United States Government.
 *   It is the responsibility of any person or organization contemplating
 *   export to obtain such a license before exporting.
 * 
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 * 
 */


#define NEED_SOCKETS
#include <krb5.h>
#include <krb.h>
#include <ldap.h>
#ifdef _WIN32
#include <wshelper.h>
#include "k5-int.h"
#include "adm_err.h"
#include "krb5_err.h"
#else
#include <sys/socket.h>
#include <netdb.h>
#include <sys/select.h>
#endif
#include <auth_con.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/timeb.h>
#include <errno.h>
#include "kpasswd.h"
#include "gsssasl.h"
#include "gssldap.h"

#define PW_LENGTH 25
#define KDC_PORT  464
#define ULONG     unsigned long

#ifndef krb5_is_krb_error
#define krb5_is_krb_error(dat)\
            ((dat) && (dat)->length && ((dat)->data[0] == 0x7e ||\
            (dat)->data[0] == 0x5e))
#endif

#ifdef _WIN32
#define sleep(Seconds) Sleep(Seconds * 1000)
#define gethostbyname(Server) rgethostbyname(Server)
#endif

/* Win32 defines. */
#if defined(_WIN32) && !defined(__CYGWIN32__)
#ifndef ECONNABORTED
#define ECONNABORTED WSAECONNABORTED
#endif
#ifndef ECONNREFUSED
#define ECONNREFUSED WSAECONNREFUSED
#endif
#ifndef EHOSTUNREACH
#define EHOSTUNREACH WSAEHOSTUNREACH
#endif
#endif /* _WIN32 && !__CYGWIN32__ */

static const char rcsid[] = "$Id: setpw.c,v 1.4 2001-05-03 22:00:15 zacheiss Exp $";

static int frequency[26][26] =
{ {4, 20, 28, 52, 2, 11, 28, 4, 32, 4, 6, 62, 23, 167, 2, 14, 0, 83, 76, 
127, 7, 25, 8, 1, 9, 1}, /* aa - az */
   {13, 0, 0, 0, 55, 0, 0, 0, 8, 2, 0, 22, 0, 0, 11, 0, 0, 15, 4, 2, 13, 0, 
0, 0, 15, 0}, /* ba - bz */
   {32, 0, 7, 1, 69, 0, 0, 33, 17, 0, 10, 9, 1, 0, 50, 3, 0, 10, 0, 28, 11, 
0, 0, 0, 3, 0}, /* ca - cz */
   {40, 16, 9, 5, 65, 18, 3, 9, 56, 0, 1, 4, 15, 6, 16, 4, 0, 21, 18, 53, 
19, 5, 15, 0, 3, 0}, /* da - dz */
   {84, 20, 55, 125, 51, 40, 19, 16, 50, 1, 4, 55, 54, 146, 35, 37, 6, 191, 
149, 65, 9, 26, 21, 12, 5, 0}, /* ea - ez */
   {19, 3, 5, 1, 19, 21, 1, 3, 30, 2, 0, 11, 1, 0, 51, 0, 0, 26, 8, 47, 6, 
3, 3, 0, 2, 0}, /* fa - fz */
   {20, 4, 3, 2, 35, 1, 3, 15, 18, 0, 0, 5, 1, 4, 21, 1, 1, 20, 9, 21, 9, 
0, 5, 0, 1, 0}, /* ga - gz */
   {101, 1, 3, 0, 270, 5, 1, 6, 57, 0, 0, 0, 3, 2, 44, 1, 0, 3, 10, 18, 6, 
0, 5, 0, 3, 0}, /* ha - hz */
   {40, 7, 51, 23, 25, 9, 11, 3, 0, 0, 2, 38, 25, 202, 56, 12, 1, 46, 79, 
117, 1, 22, 0, 4, 0, 3}, /* ia - iz */
   {3, 0, 0, 0, 5, 0, 0, 0, 1, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 3, 0, 0, 0, 
0, 0}, /* ja - jz */
   {1, 0, 0, 0, 11, 0, 0, 0, 13, 0, 0, 0, 0, 2, 0, 0, 0, 0, 6, 2, 1, 0, 2, 
0, 1, 0}, /* ka - kz */
   {44, 2, 5, 12, 62, 7, 5, 2, 42, 1, 1, 53, 2, 2, 25, 1, 1, 2, 16, 23, 9, 
0, 1, 0, 33, 0}, /* la - lz */
   {52, 14, 1, 0, 64, 0, 0, 3, 37, 0, 0, 0, 7, 1, 17, 18, 1, 2, 12, 3, 8, 
0, 1, 0, 2, 0}, /* ma - mz */
   {42, 10, 47, 122, 63, 19, 106, 12, 30, 1, 6, 6, 9, 7, 54, 7, 1, 7, 44, 
124, 6, 1, 15, 0, 12, 0}, /* na - nz */
   {7, 12, 14, 17, 5, 95, 3, 5, 14, 0, 0, 19, 41, 134, 13, 23, 0, 91, 23, 
42, 55, 16, 28, 0, 4, 1}, /* oa - oz */
   {19, 1, 0, 0, 37, 0, 0, 4, 8, 0, 0, 15, 1, 0, 27, 9, 0, 33, 14, 7, 6, 0, 
0, 0, 0, 0}, /* pa - pz */
   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 17, 0, 0, 
0, 0, 0}, /* qa - qz */
   {83, 8, 16, 23, 169, 4, 8, 8, 77, 1, 10, 5, 26, 16, 60, 4, 0, 24, 37, 
55, 6, 11, 4, 0, 28, 0}, /* ra - rz */
   {65, 9, 17, 9, 73, 13, 1, 47, 75, 3, 0, 7, 11, 12, 56, 17, 6, 9, 48, 
116, 35, 1, 28, 0, 4, 0}, /* sa - sz */
   {57, 22, 3, 1, 76, 5, 2, 330, 126, 1, 0, 14, 10, 6, 79, 7, 0, 49, 50, 
56, 21, 2, 27, 0, 24, 0}, /* ta - tz */
   {11, 5, 9, 6, 9, 1, 6, 0, 9, 0, 1, 19, 5, 31, 1, 15, 0, 47, 39, 31, 0, 
3, 0, 0, 0, 0}, /* ua - uz */
   {7, 0, 0, 0, 72, 0, 0, 0, 28, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 3, 0}, /* va - vz */
   {36, 1, 1, 0, 38, 0, 0, 33, 36, 0, 0, 4, 1, 8, 15, 0, 0, 0, 4, 2, 0, 0, 
1, 0, 0, 0}, /* wa - wz */
   {1, 0, 2, 0, 0, 1, 0, 0, 3, 0, 0, 0, 0, 0, 1, 5, 0, 0, 0, 3, 0, 0, 1, 0, 
0, 0}, /* xa - xz */
   {14, 5, 4, 2, 7, 12, 12, 6, 10, 0, 0, 3, 7, 5, 17, 3, 0, 4, 16, 30, 0, 
0, 5, 0, 0, 0}, /* ya - yz */
   {1, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}}; /* za - zz */

/*
 * This MUST be equal to the sum of the equivalent rows above.
 */

static int row_sums[26] =
{796,160,284,401,1276,262,199,539,777,	
  16,39,351,243,751,662,181,17,683,	
  662,968,248,115,180,17,162,5};

/*
 * Frequencies of starting characters
 */

static int start_freq [26] =
{1299,425,725,271,375,470,93,223,1009,
 24,20,355,379,319,823,618,21,317,
 962,1991,271,104,516,6,16,14};

/*
 * This MUST be equal to the sum of all elements in the above array.
 */

struct sockaddr_in  kdc_server;
SOCKET              kdc_socket;
krb5_context        context;
krb5_ccache         ccache;
krb5_auth_context   auth_context = NULL;
krb5_data           ap_req;
krb5_creds          *credsp = NULL;
krb5_creds          creds;
char                connected_server[128];

static int total_sum = 11646;

int get_krb5_error(krb5_error_code rc, char *in, char *out);
int ad_connect(LDAP **ldap_handle, char *ldap_domain, char *dn_path, 
               char *Win2kPassword, char *Win2kUser, char *default_server, 
               int connect_to_kdc);
int ad_kdc_connect(char *connectedServer);
int ad_server_connect(char *connectedServer, char *domain);
void ad_kdc_disconnect();
int compare_elements(const void *arg1, const void *arg2);
int convert_domain_to_dn(char *domain, char *dnp);
int set_password(char *user, char *password, char *domain);

int locate_ldap_server(char *domain, char **server_name);

long myrandom();
void generate_password(char *password);
krb5_error_code encode_krb5_setpw
        PROTOTYPE((const krb5_setpw *rep, krb5_data ** code));

krb5_error_code make_setpw_req(krb5_context context,  krb5_auth_context auth_context,
                                  krb5_data *ap_req, krb5_principal targprinc,
                                  char *passwd, krb5_data *packet)
{
  krb5_error_code  ret;
  krb5_setpw       setpw;
  krb5_data        cipherpw;
  krb5_data        *encoded_setpw;
  krb5_replay_data replay;
  char             *ptr;
  register int     count = 2;

  memset(&setpw, 0, sizeof(krb5_setpw));
  if (ret = krb5_auth_con_setflags(context, auth_context,
                                   KRB5_AUTH_CONTEXT_DO_SEQUENCE))
    return(ret);
  setpw.targprinc = targprinc;
  setpw.newpasswd.length = strlen(passwd);
  setpw.newpasswd.data = passwd;
  if ((ret = encode_krb5_setpw(&setpw, &encoded_setpw)))
    return( ret );
  if (ret = krb5_mk_priv(context, auth_context,
                         encoded_setpw, &cipherpw, &replay))
    return(ret);
  packet->length = 6 + ap_req->length + cipherpw.length;
  packet->data = (char *) malloc(packet->length);
  ptr = packet->data;
  /* Length */
  *ptr++ = (packet->length>>8) & 0xff;
  *ptr++ = packet->length & 0xff;
  /* version */
  *ptr++ = (char)0xff;
  *ptr++ = (char)0x80;
  /* ap_req length, big-endian */
  *ptr++ = (ap_req->length>>8) & 0xff;
  *ptr++ = ap_req->length & 0xff;
  /* ap-req data */
  memcpy(ptr, ap_req->data, ap_req->length);
  ptr += ap_req->length;
  /* krb-priv of password */
  memcpy(ptr, cipherpw.data, cipherpw.length);
  free(cipherpw.data);
/*  krb5_free_data_contents(context, &cipherpw);*/
  krb5_free_data(context, encoded_setpw);
  return(0);
}

krb5_error_code get_setpw_rep(krb5_context context, krb5_auth_context auth_context,
                              krb5_data *packet, int *result_code,
                              krb5_data *result_data)
{
  char             *ptr;
  int              plen;
  int              vno;
  krb5_data        ap_rep;
  krb5_error_code  ret;
  krb5_data        cipherresult;
  krb5_data        clearresult;
  krb5_error       *krberror;
  krb5_replay_data replay;
  krb5_keyblock    *tmp;
  krb5_ap_rep_enc_part *ap_rep_enc;

  if (packet->length < 4)
    return(KRB5KRB_AP_ERR_MODIFIED);
  ptr = packet->data;
  if (krb5_is_krb_error(packet))
    {
      ret = decode_krb5_error(packet, &krberror);
      if (ret)
        return(ret);
      ret = krberror->error;
      krb5_free_error(context, krberror);
      return(ret);
    }
  /* verify length */
  plen = (*ptr++ & 0xff);
  plen = (plen<<8) | (*ptr++ & 0xff);
  if (plen != packet->length)
    return(KRB5KRB_AP_ERR_MODIFIED);
  vno = (*ptr++ & 0xff);
  vno = (vno<<8) | (*ptr++ & 0xff);
  if (vno != KRB5_KPASSWD_VERS_SETPW && vno != KRB5_KPASSWD_VERS_CHANGEPW)
    return(KRB5KDC_ERR_BAD_PVNO);
  /* read, check ap-rep length */
  ap_rep.length = (*ptr++ & 0xff);
  ap_rep.length = (ap_rep.length<<8) | (*ptr++ & 0xff);
  if (ptr + ap_rep.length >= packet->data + packet->length)
    return(KRB5KRB_AP_ERR_MODIFIED);
  if (ap_rep.length)
    {
      /* verify ap_rep */
      ap_rep.data = ptr;
      ptr += ap_rep.length;
      if (ret = krb5_rd_rep(context, auth_context, &ap_rep, &ap_rep_enc))
        return(ret);
      krb5_free_ap_rep_enc_part(context, ap_rep_enc);
      /* extract and decrypt the result */
      cipherresult.data = ptr;
      cipherresult.length = (packet->data + packet->length) - ptr;
      /* XXX there's no api to do this right. The problem is that
         if there's a remote subkey, it will be used.  This is
         not what the spec requires */
      tmp = auth_context->remote_subkey;
      auth_context->remote_subkey = NULL;
      ret = krb5_rd_priv(context, auth_context, &cipherresult, &clearresult,
                         &replay);
      auth_context->remote_subkey = tmp;
      if (ret)
        return(ret);
    }
  else
    {
      cipherresult.data = ptr;
      cipherresult.length = (packet->data + packet->length) - ptr;

      if (ret = krb5_rd_error(context, &cipherresult, &krberror))
        return(ret);

      clearresult = krberror->e_data;
    }
  if (clearresult.length < 2)
    {
      ret = KRB5KRB_AP_ERR_MODIFIED;
      goto cleanup;
    }
  ptr = clearresult.data;
  *result_code = (*ptr++ & 0xff);
  *result_code = (*result_code<<8) | (*ptr++ & 0xff);
  if ((*result_code < KRB5_KPASSWD_SUCCESS) ||
      (*result_code > KRB5_KPASSWD_ACCESSDENIED))
    {
      ret = KRB5KRB_AP_ERR_MODIFIED;
        goto cleanup;
    }
  /* all success replies should be authenticated/encrypted */
  if ((ap_rep.length == 0) && (*result_code == KRB5_KPASSWD_SUCCESS))
    {
      ret = KRB5KRB_AP_ERR_MODIFIED;
        goto cleanup;
    }
  result_data->length = (clearresult.data + clearresult.length) - ptr;
  if (result_data->length)
    {
      result_data->data = (char *) malloc(result_data->length);
      memcpy(result_data->data, ptr, result_data->length);
    }
  else
      result_data->data = NULL;
  ret = 0;
cleanup:
  if (ap_rep.length)
    free(clearresult.data);
  else
      krb5_free_error(context, krberror);
  return(ret);
}

krb5_error_code kdc_set_password(krb5_context context, krb5_ccache ccache,
                                  char *newpw, char *user, char *domain,
                                  int *result_code)
{
  krb5_data         chpw_snd;
  krb5_data         chpw_rcv;
  krb5_data         result_string;
  krb5_address      local_kaddr;
  krb5_address      remote_kaddr;
  char              userrealm[256];
  char              temp[256];
  krb5_error_code   code;
  struct sockaddr   local_addr;
  struct sockaddr   remote_addr;
  int               i;
  int               addrlen;
  int               cc;
  int               local_result_code;
  int               nfds;
  krb5_principal    targprinc;
  struct timeval    TimeVal;
  fd_set            readfds;

  memset(&local_addr, 0, sizeof(local_addr));
  memset(&local_kaddr, 0, sizeof(local_kaddr));
  memset(&result_string, 0, sizeof(result_string));
  memset(&remote_kaddr, 0, sizeof(remote_kaddr));
  memset(&chpw_snd, 0, sizeof(krb5_data));
  memset(&chpw_rcv, 0, sizeof(krb5_data));
  memset(userrealm, '\0', sizeof(userrealm));
  targprinc = NULL;

  chpw_rcv.length = 1500;
  chpw_rcv.data = (char *) calloc(1, chpw_rcv.length);

  for (i = 0; i < (int)strlen(domain); i++)
    userrealm[i] = toupper(domain[i]);

  sprintf(temp, "%s@%s", user, userrealm);
  krb5_parse_name(context, temp, &targprinc);

  if (credsp == NULL)
    {
      memset(&creds, 0, sizeof(creds));
      memset(&ap_req, 0, sizeof(krb5_data));
      sprintf(temp, "%s@%s", "kadmin/changepw", userrealm);
      if (code = krb5_parse_name(context, temp, &creds.server))
        goto cleanup;
      if (code = krb5_cc_get_principal(context, ccache, &creds.client))
        goto cleanup;
      if (code = krb5_get_credentials(context, 0, ccache, &creds, &credsp))
        goto cleanup;
      if (code = krb5_mk_req_extended(context, &auth_context, AP_OPTS_USE_SUBKEY,
                                      NULL, credsp, &ap_req))
        goto cleanup;
    }

  addrlen = sizeof(local_addr);
  if (getsockname(kdc_socket, &local_addr, &addrlen) < 0)
    {
      code = KDC_GETSOCKNAME_ERROR;
      goto cleanup;
    }
  if (((struct sockaddr_in *)&local_addr)->sin_addr.s_addr != 0)
    {
      local_kaddr.addrtype = ADDRTYPE_INET;
      local_kaddr.length =
        sizeof(((struct sockaddr_in *) &local_addr)->sin_addr);
      local_kaddr.contents = 
        (char *) &(((struct sockaddr_in *) &local_addr)->sin_addr);
    }
  else
    {
      krb5_address **addrs;
      krb5_os_localaddr(context, &addrs);
      local_kaddr.magic = addrs[0]->magic;
      local_kaddr.addrtype = addrs[0]->addrtype;
      local_kaddr.length = addrs[0]->length;
      local_kaddr.contents = calloc(1, addrs[0]->length);
      memcpy(local_kaddr.contents, addrs[0]->contents, addrs[0]->length);
      krb5_free_addresses(context, addrs);
    }

  addrlen = sizeof(remote_addr);
  if (getpeername(kdc_socket, &remote_addr, &addrlen) < 0)
    {
      code = KDC_GETPEERNAME_ERROR;
      goto cleanup;
    }
  remote_kaddr.addrtype = ADDRTYPE_INET;
  remote_kaddr.length = sizeof(((struct sockaddr_in *) &remote_addr)->sin_addr);
  remote_kaddr.contents = (char *) &(((struct sockaddr_in *) &remote_addr)->sin_addr);

  if (code = krb5_auth_con_setaddrs(context, auth_context, &local_kaddr, NULL))
    goto cleanup;
  if (code = make_setpw_req(context, auth_context, &ap_req,
                               targprinc, newpw, &chpw_snd))
    goto cleanup;

  for (i = 0; i < 3; i++)
    {
      if ((cc = sendto(kdc_socket, chpw_snd.data, chpw_snd.length, 0,
                       NULL,
                       0)) != chpw_snd.length)
        {
          code = KDC_SEND_ERROR;
          sleep(1);
          continue;
        }

      TimeVal.tv_sec = 3;
      TimeVal.tv_usec = 0;
      FD_ZERO(&readfds);
      FD_SET(kdc_socket, &readfds);
      nfds = kdc_socket + 1;
      code = select(nfds, &readfds, NULL, NULL, &TimeVal);
      if ((code == 0) || (code == SOCKET_ERROR))
        {
          code = KDC_RECEIVE_TIMEOUT;
          sleep(1);
          continue;
        }

      if ((cc = recvfrom(kdc_socket, chpw_rcv.data, chpw_rcv.length, 0, 
                         NULL, NULL)) < 0)
        {
          code = KDC_RECEIVE_TIMEOUT;
          sleep(1);
          continue;
        }
      chpw_rcv.length = cc;
      if (code = krb5_auth_con_setaddrs(context, auth_context, NULL, &remote_kaddr))
        {
          sleep(1);
          continue;
        }
      local_result_code = 0;
      code = get_setpw_rep(context, auth_context, &chpw_rcv,
                           &local_result_code, &result_string);

      if (local_result_code)
        {
          if (local_result_code == KRB5_KPASSWD_SOFTERROR)
            local_result_code = KRB5_KPASSWD_SUCCESS;
          *result_code = local_result_code;
        }
      if ((code == 0) && (local_result_code == 0))
        break;
      sleep(1);
    }

cleanup:
  if (chpw_snd.data != NULL)
    free(chpw_snd.data);
  if (chpw_rcv.data != NULL)
    free(chpw_rcv.data);
  if (targprinc != NULL)
    krb5_free_principal(context, targprinc);
  return(code);
}

int set_password(char *user, char *password, char *domain)
{
  int             res_code;
  krb5_error_code retval;
  char            pw[PW_LENGTH+1];

  memset(pw, '\0', sizeof(pw));
  if (strlen(password) != 0)
    strcpy(pw, password);
  else
    generate_password(pw);
  res_code = 0;
  retval = kdc_set_password(context, ccache, pw, user, domain, &res_code);

  if (res_code)
    return(res_code);
  return(retval);
}

void generate_password(char *password)
{
  int   i;
  int   j;
  int   row_position;
  int   nchars;
  int   position;
  int   word;
  int   line;
  char  *pwp;

  for (line = 22; line; --line)
    {
      for (word = 7; word; --word)
        {
          position = myrandom()%total_sum;
          for(row_position = 0, j = 0; position >= row_position; row_position += start_freq[j], j++)
            continue;
          *(pwp = password) = j + 'a' - 1;
          for (nchars = PW_LENGTH-1; nchars; --nchars)
            {
              i = *pwp - 'a';
              pwp++;
              position = myrandom()%row_sums[i];
              for (row_position = 0, j = 0; position >= row_position; row_position += frequency[i][j], j++)
                continue;
              *pwp = j + 'a' - 1;
            }
          *(++pwp)='\0';
          return;
        }
      putchar('\n');
    }
}

long myrandom()
{
  static int init = 0;
  int pid;
#ifdef _WIN32
  struct _timeb timebuffer;
#else
  struct timeval tv;
#endif

  if (!init)
    {
      init = 1;
      pid = getpid();
#ifdef _WIN32
      _ftime(&timebuffer);
      srand(timebuffer.time ^ timebuffer.millitm ^ pid);
#else
      gettimeofday(&tv, (struct timezone *) NULL);
      srandom(tv.tv_sec ^ tv.tv_usec ^ pid);
#endif
    }
  return (rand());
}

int get_krb5_error(krb5_error_code rc, char *in, char *out)
{
  int krb5Error;
  int retval;

  retval = 1;

  if (rc < 0)
    {
      krb5Error = ((int)(rc & 255));
      sprintf(out, "%s: %s(%ld)", in, error_message(rc), krb5Error);
    }
  else
    {
      switch (rc)
        {
          case KDC_RECEIVE_TIMEOUT:
            {
              retval = 0;
              sprintf(out, "%s: %s(%d)", in, "Receive timeout", rc);
              break;
            }
          case KDC_RECEIVE_ERROR:
            {
              retval = 0;
              sprintf(out, "%s: %s(%d)", in, "Receive error", rc);
              break;
            }
          case KRB5_KPASSWD_MALFORMED:
            {
              sprintf(out, "%s: %s(%d)", in, "malformed password", rc);
              break;
            }
          case KRB5_KPASSWD_HARDERROR:
            {
              sprintf(out, "%s: %s(%d)", in, "hard error", rc);
              break;
            }
          case KRB5_KPASSWD_AUTHERROR:
            {
              retval = 0;
              sprintf(out, "%s: %s(%d)", in, "authentication error", rc);
              break;
            }
          case KRB5_KPASSWD_SOFTERROR:
            {
              retval = 0;
              sprintf(out, "%s: %s(%d)", in, "soft error", rc);
              break;
            }
          case KRB5_KPASSWD_ACCESSDENIED:
            {
              sprintf(out, "%s: %s(%d)", in, "Access denied", rc);
              break;
            }
          case KDC_SEND_ERROR:
            {
              retval = 0;
              sprintf(out, "%s: %s(%d)", in, "Send error", rc);
              break;
            }
          case KDC_GETSOCKNAME_ERROR:
            {
              retval = 0;
              sprintf(out, "%s: %s(%d)", in, "Socket error - getsockname", rc);
              break;
            }
          case KDC_GETPEERNAME_ERROR:
            {
              retval = 0;
              sprintf(out, "%s: %s(%d)", in, "Socket error - getpeername", rc);
              break;
            }
          default:
            {
              sprintf(out, "%s: %s(%d)", in, "unknown error", rc);
              break;
            }
        }
    }
  return(retval);
}

int ad_connect(LDAP **ldap_handle, char *ldap_domain, char *dn_path, 
               char *Win2kPassword, char *Win2kUser, char *default_server,
               int connect_to_kdc)
{
  int         i;
  int         j;
  char        *server_name[MAX_SERVER_NAMES];
  char        server_array[MAX_SERVER_NAMES][256];
  static char temp[128];
  ULONG       version = LDAP_VERSION3;
  ULONG       rc;
  int         Max_wait_time = 500;
  int         Max_size_limit = LDAP_NO_LIMIT;

  if (ldap_domain == NULL)
    ldap_domain = "win.mit.edu";
  convert_domain_to_dn(ldap_domain, dn_path);
  if (strlen(dn_path) == 0)
    return(1);

  memset(server_name, 0, sizeof(server_name[0]) * MAX_SERVER_NAMES);
  memset(server_array, 0, sizeof(server_array[0]) * MAX_SERVER_NAMES);
  if (strlen(default_server) == 0)
    {
      if (locate_ldap_server(ldap_domain, server_name) == -1)
        return(2);
      j = 0;
      for (i = 0; i < MAX_SERVER_NAMES; i++)
        {
          if (server_name[i] != NULL)
            {
              strcpy(server_array[i], server_name[i]);
              free(server_name[i]);
              j++;
            }
        }
      if (j == 0)
        return(2);
      qsort((void *)server_array, (size_t)j, sizeof(server_array[0]), compare_elements);
    }
  else
    strcpy(server_array[0], default_server);

  for (i = 0; i < MAX_SERVER_NAMES; i++)
    {
      if (strlen(server_array[i]) != 0)
        {
          if (((*ldap_handle) = ldap_open(server_array[i], LDAP_PORT)) != NULL)
            {
              rc = ldap_set_option((*ldap_handle), LDAP_OPT_PROTOCOL_VERSION, &version);
              rc = ldap_set_option((*ldap_handle), LDAP_OPT_TIMELIMIT, 
                                   (void *)&Max_wait_time);
              rc = ldap_set_option((*ldap_handle), LDAP_OPT_SIZELIMIT, 
                                   (void *)&Max_size_limit);
              rc = ldap_set_option((*ldap_handle), LDAP_OPT_REFERRALS, LDAP_OPT_OFF);
              rc = ldap_adgssapi_bind((*ldap_handle), dn_path, GSSSASL_PRIVACY_PROTECTION);
              if (rc == LDAP_SUCCESS)
                {
                  if (connect_to_kdc)
                    {
                      if (!ad_server_connect(server_array[i], ldap_domain))
                        {
                          ldap_unbind_s((*ldap_handle));
                          continue;
                        }
                    }
                  if (strlen(default_server) == 0)
                    strcpy(default_server, server_array[i]);
                  strcpy(connected_server, server_array[i]);
                  break;
                }
            }
        }
    }
  if (i >= MAX_SERVER_NAMES)
    return(3);
  return(0);
}

int ad_server_connect(char *connectedServer, char *domain)
{
  krb5_error_code   rc;
  krb5_creds        creds;
  krb5_creds        *credsp;
  char              temp[256];
  char              userrealm[256];
  int               i;
  unsigned short    port = KDC_PORT;

  context = NULL;
  credsp = NULL;
  memset(&ccache, 0, sizeof(ccache));
  memset(&creds, 0, sizeof(creds));
  memset(userrealm, '\0', sizeof(userrealm));

  rc = 0;
  if (krb5_init_context(&context))
    goto cleanup;
  if (krb5_cc_default(context, &ccache))
    goto cleanup;

  for (i = 0; i < (int)strlen(domain); i++)
    userrealm[i] = toupper(domain[i]);
  sprintf(temp, "%s@%s", "kadmin/changepw", userrealm);
  if (krb5_parse_name(context, temp, &creds.server))
    goto cleanup;
  if (krb5_cc_get_principal(context, ccache, &creds.client))
    goto cleanup;
  if (krb5_get_credentials(context, 0, ccache, &creds, &credsp))
    goto cleanup;

  rc = ad_kdc_connect(connectedServer);


cleanup:
  if (!rc)
    {
      krb5_cc_close(context, ccache);
      krb5_free_context(context);
    }
  krb5_free_cred_contents(context, &creds);
  if (credsp != NULL)
    krb5_free_creds(context, credsp);
  return(rc);
}


int ad_kdc_connect(char *connectedServer)
{
  struct hostent  *hp;
  int             rc;

  rc = 0;
  hp = gethostbyname(connectedServer);
  if (hp == NULL)
    goto cleanup;
  memset(&kdc_server, 0, sizeof(kdc_server));
  memcpy(&(kdc_server.sin_addr),hp->h_addr_list[0],hp->h_length);
  kdc_server.sin_family = hp->h_addrtype;
  kdc_server.sin_port = htons(KDC_PORT);

  if ((kdc_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    goto cleanup;
  if (connect(kdc_socket, (struct sockaddr*)&kdc_server, sizeof(kdc_server)) == SOCKET_ERROR)
    goto cleanup;
  rc = 1;

cleanup:
  return(rc);
}

void ad_kdc_disconnect()
{

  if (auth_context != NULL)
    {
      krb5_auth_con_free(context, auth_context);
      if (ap_req.data != NULL)
        free(ap_req.data);
      krb5_free_cred_contents(context, &creds);
      if (credsp != NULL)
        krb5_free_creds(context, credsp);
    }
  credsp = NULL;
  auth_context = NULL;
  if (context != NULL)
    {
      krb5_cc_close(context, ccache);
      krb5_free_context(context);
    }
  closesocket(kdc_socket);

}

int convert_domain_to_dn(char *domain, char *dnp)
{
  char    *fp;
  char    *dp;
  char    dn[512];

  memset(dn, '\0', sizeof(dn));    
  strcpy(dn, "dc=");
  dp = dn+3;
  for (fp = domain; *fp; fp++)
    {
      if (*fp == '.') 
        {
          strcpy(dp, ",dc=");
          dp += 4;
        }
      else
        *dp++ = *fp;
    }

  strcpy(dnp, dn);
  return 0;
}

int compare_elements(const void *arg1, const void *arg2)
{
  int rc;

  rc = strcmp((char*)arg1, (char*)arg2);
  if (rc < 0)
    return(1);
  if (rc > 0)
    return(-1);
  return(rc);
}

