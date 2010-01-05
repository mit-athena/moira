#define LDAP_AUTH_OTHERKIND             0x86L
#define LDAP_AUTH_NEGOTIATE             (LDAP_AUTH_OTHERKIND | 0x0400)
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
#include <stdio.h>
#include <sys/timeb.h>
#include <string.h>
#include <stdlib.h>
#include <ldap.h>

#ifdef _WIN32
#include <wshelper.h>
#include "k5-int.h"
#else
#include <sys/socket.h>
#include <netdb.h>
#include <sys/select.h>
#endif

krb5_context        context;
krb5_ccache         ccache;
char                *admin_server;

#define SOCKET          int
#define INVALID_SOCKET  ((SOCKET)~0)
#define SOCKET_ERROR    (-1)

#define PW_LENGTH 255
#define KDC_PORT  464
#define ULONG     unsigned long

#ifndef krb5_is_krb_error
#define krb5_is_krb_error(dat)\
            ((dat) && (dat)->length && ((dat)->data[0] == 0x7e ||\
            (dat)->data[0] == 0x5e))
#endif

#ifdef _WIN32
extern krb5_error_code decode_krb5_error
	(const krb5_data *output, krb5_error **rep);
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

static const char rcsid[] = "$Id: setpw.c,v 1.10 2009-12-29 17:29:31 zacheiss Exp $";

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

static int total_sum = 11646;

int compare_elements(const void *arg1, const void *arg2);
int set_password(char *user, char *password, char *domain, char *msg);

int locate_ldap_server(char *domain, char **server_name);

long myrandom();
void generate_password(char *password);
extern LDAP* ldap_handle;
char DomainOu[256];

#ifdef WIN32
krb5_error_code encode_krb5_setpw
        PROTOTYPE((const krb5_setpw *rep, krb5_data ** code));
#endif

krb5_error_code kdc_set_password(krb5_context context, krb5_ccache ccache,
                                 char *newpw, char *user, char *domain,
                                 int *result_code, char *admin_server)
{
  krb5_data         chpw_snd;
  krb5_data         chpw_rcv;
  krb5_data         result_string;
  krb5_data         result_code_string;
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
  krb5_creds        *credsp = NULL;
  struct hostent    *hp;
  SOCKET            kdc_socket;
  struct sockaddr_in  kdc_server;
  krb5_auth_context   auth_context = NULL;
  krb5_creds          creds;
  krb5_data           ap_req;
  char                *code_string;

  memset(&local_addr, 0, sizeof(local_addr));
  memset(&local_kaddr, 0, sizeof(local_kaddr));
  memset(&result_string, 0, sizeof(result_string));
  memset(&remote_kaddr, 0, sizeof(remote_kaddr));
  memset(&chpw_snd, 0, sizeof(krb5_data));
  memset(&chpw_rcv, 0, sizeof(krb5_data));
  memset(userrealm, '\0', sizeof(userrealm));
  targprinc = NULL;

  chpw_rcv.length = 1500;
  chpw_rcv.data = (char *) malloc(chpw_rcv.length);

  for (i = 0; i < (int)strlen(domain); i++)
    userrealm[i] = toupper(domain[i]);

  sprintf(temp, "%s@%s", user, userrealm);
  krb5_parse_name(context, temp, &targprinc);
  
  if (credsp == NULL)
    {
      memset(&creds, 0, sizeof(creds));
      memset(&ap_req, 0, sizeof(krb5_data));
      memset(&result_string, 0, sizeof(krb5_data));
      memset(&result_code_string, 0, sizeof(krb5_data));
      sprintf(temp, "%s@%s", "kadmin/changepw", userrealm);

      if(code = krb5_init_context(&context)) 
	goto cleanup;
      
      if(code = krb5_cc_default(context, &ccache)) 
	goto cleanup;

      if (krb5_parse_name(context, temp, &creds.server))
	goto cleanup;

      if (krb5_cc_get_principal(context, ccache, &creds.client))
	goto cleanup;

      if (krb5_get_credentials(context, 0, ccache, &creds, &credsp))
	goto cleanup;
    }

  if ((code = krb5_change_set_password(context, credsp, newpw,
				       targprinc, &result_code,
				       &result_code_string,
				       &result_string))) {

    goto cleanup;
  }


cleanup:
  if (targprinc != NULL)
    krb5_free_principal(context, targprinc);
  return(code);
}

int set_password(char *user, char *password, char *domain, char *msg)
{
  int             res_code;
  krb5_error_code retval;
  char            pw[PW_LENGTH+1];
  int newpasswd = 0;

  memset(pw, '\0', sizeof(pw));
  if (strlen(password) != 0)
    {
      strcpy(pw, password);
      newpasswd = 1;
    }
  else
    {
      generate_password(pw);
      newpasswd = 0;
    }
  res_code = 0;

  retval = kdc_set_password(context, ccache, pw, user, domain, &res_code,
			    admin_server);

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
