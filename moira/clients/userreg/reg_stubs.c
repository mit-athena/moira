/* $Id $
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include <moira_site.h>
#include "ureg_proto.h"

#include <sys/types.h>
#include <sys/socket.h>

#include <netdb.h>
#include <netinet/in.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <des.h>
#include <hesiod.h>
#include <kadm_err.h>
#include <krb.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/reg_stubs.c,v 1.29 1998-02-05 22:50:58 danw Exp $");

int do_operation(char *first, char *last, char *idnumber, char *hashidnumber,
		 char *data, u_long opcode);
int do_secure_operation(char *login, char *idnumber, char *passwd,
			char *newpasswd, u_long opcode);
int do_call(char *buf, int len, int seq_no, char *login);

char *krb_realmofhost(char *);

static int reg_sock = -1;
static int seq_no = 0;
static char *host;
#define UNKNOWN_HOST -1
#define UNKNOWN_SERVICE -2

#ifndef FD_SET
#define FD_ZERO(p)  ((p)->fds_bits[0] = 0)
#define FD_SET(n, p)   ((p)->fds_bits[0] |= (1 << (n)))
#define FD_ISSET(n, p)   ((p)->fds_bits[0] & (1 << (n)))
#endif /* FD_SET */

int ureg_init(void)
{
  struct servent *sp;
  char **p, *s;
  struct hostent *hp;
  struct sockaddr_in s_in;

  initialize_ureg_error_table();
  initialize_krb_error_table();
  initialize_sms_error_table();
  initialize_kadm_error_table();

  seq_no = getpid();

  host = NULL;
  host = getenv("REGSERVER");
#ifdef HESIOD
  if (!host || (strlen(host) == 0))
    {
      p = hes_resolve("registration", "sloc");
      if (p)
	host = *p;
    }
#endif
  if (!host || (strlen(host) == 0))
    {
      host = strdup(MOIRA_SERVER);
      s = strchr(host, ':');
      if (s)
	*s = '\0';
    }
  hp = gethostbyname(host);
  host = strdup(hp->h_name);
  if (hp == NULL)
    return UNKNOWN_HOST;

  sp = getservbyname("moira_ureg", "udp");

  if (sp == NULL)
    return UNKNOWN_SERVICE;

  close(reg_sock);
  reg_sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (reg_sock < 0)
    return errno;

  memset(&s_in, 0, sizeof(s_in));
  s_in.sin_port = sp->s_port;
  memcpy(&s_in.sin_addr, hp->h_addr, sizeof(struct in_addr));
  s_in.sin_family = AF_INET;

  if (connect(reg_sock, (struct sockaddr *)&s_in, sizeof(s_in)) < 0)
    return errno;
  return 0;
}

int verify_user(char *first, char *last, char *idnumber,
		char *hashidnumber, char *login)
{
  char buf[1024];
  int version = ntohl((u_long)1);
  int call = ntohl((u_long)UREG_VERIFY_USER);
  des_cblock key;
  des_key_schedule ks;
  char *bp = buf;
  int len;
  char crypt_src[1024];

  memcpy(bp, &version, sizeof(int));
  bp += sizeof(int);
  seq_no++;
  memcpy(bp, &seq_no, sizeof(int));

  bp += sizeof(int);

  memcpy(bp, &call, sizeof(int));

  bp += sizeof(int);

  strcpy(bp, first);
  bp += strlen(bp) + 1;

  strcpy(bp, last);
  bp += strlen(bp) + 1;

  len = strlen(idnumber) + 1;
  memcpy(crypt_src, idnumber, len);

  memcpy(crypt_src + len, hashidnumber, 13);

  des_string_to_key(hashidnumber, key);
  des_key_sched(key, ks);
  des_pcbc_encrypt(crypt_src, bp, len + 13, ks, key, DES_ENCRYPT);
  bp += len + 14 + 8;
  len = bp - buf;
  return do_call(buf, len, seq_no, login);
}

int do_operation(char *first, char *last, char *idnumber, char *hashidnumber,
		 char *data, u_long opcode)
{
  char buf[1024];
  int version = ntohl((u_long)1);
  int call = ntohl(opcode);
  des_cblock key;
  des_key_schedule ks;
  char *bp = buf;
  int len;

  char crypt_src[1024];
  char *cbp;

  memcpy(bp, &version, sizeof(int));
  bp += sizeof(int);
  seq_no++;
  memcpy(bp, &seq_no, sizeof(int));

  bp += sizeof(int);

  memcpy(bp, &call, sizeof(int));

  bp += sizeof(int);

  strcpy(bp, first);
  bp += strlen(bp) + 1;

  strcpy(bp, last);
  bp += strlen(bp) + 1;

  len = strlen(idnumber) + 1;
  cbp = crypt_src;

  memcpy(crypt_src, idnumber, len);
  cbp += len;

  memcpy(cbp, hashidnumber, 14);
  cbp += 14;

  len = strlen(data) + 1;
  memcpy(cbp, data, len);
  cbp += len;

  len = cbp - crypt_src;
  des_string_to_key(hashidnumber, key);
  des_key_sched(key, ks);
  des_pcbc_encrypt(crypt_src, bp, len, ks, key, 1);
  len = ((len + 7) >> 3) << 3;
  bp += len;

  len = bp - buf;
  return do_call(buf, len, seq_no, 0);
}

int grab_login(char *first, char *last, char *idnumber, char *hashidnumber,
	       char *login)
{
  return do_operation(first, last, idnumber, hashidnumber, login,
		      UREG_RESERVE_LOGIN);
}

int enroll_login(char *first, char *last, char *idnumber, char *hashidnumber,
		 char *login)
{
  return do_operation(first, last, idnumber, hashidnumber, login,
		      UREG_SET_IDENT);
}

int set_password(char *first, char *last, char *idnumber, char *hashidnumber,
		 char *password)
{
  return do_operation(first, last, idnumber, hashidnumber, password,
		      UREG_SET_PASSWORD);
}

int get_krb(char *first, char *last, char *idnumber, char *hashidnumber,
	    char *password)
{
  return do_operation(first, last, idnumber, hashidnumber, password,
		      UREG_GET_KRB);
}


/* The handles the operations for secure passwords.
 * To find out if a user has a secure instance, the newpasswd
 * field is ignored (but must be a valid char *)
 * and the opcode = UREG_GET_SECURE need to be specified (but the
 * other strings must be valid char*'s).  This will return
 * UREG_ALREADY_REGISTERED if it is set, or SUCCESS if not.
 * To set the password, fill in the rest of the fields, and
 * use opcode = UREG_SET_SECURE.  This returns SUCCESS or any number
 * of failure codes.
 */

int do_secure_operation(char *login, char *idnumber, char *passwd,
			char *newpasswd, u_long opcode)
{
  char buf[1500], data[128], tktstring[128];
  int version = ntohl((u_long)1);
  int call = ntohl(opcode);
  char inst[INST_SZ], hosti[INST_SZ];
  char *bp = buf, *src, *dst, *realm;
  int len, status;
  KTEXT_ST cred;
  CREDENTIALS creds;
  Key_schedule keys;

  memmove(bp, &version, sizeof(int));
  bp += sizeof(int);
  seq_no++;
  memmove(bp, &seq_no, sizeof(int));

  bp += sizeof(int);
  memmove(bp, &call,  sizeof(int));

  bp += sizeof(int);

  /* put the login name in the firstname field */
  strcpy(bp, login);
  bp += strlen(bp) + 1;

  /* the old lastname field */
  strcpy(bp, "");
  bp += strlen(bp) + 1;

  /* don't overwrite existing ticket file */
  sprintf(tktstring, "/tmp/tkt_cpw_%ld", (long)getpid());
  krb_set_tkt_string(tktstring);

  /* get realm and canonizalized hostname of server */
  realm = krb_realmofhost(host);
  for (src = host, dst = hosti; *src && *src != '.'; src++)
    {
      if (isupper(*src))
	*dst++ = tolower(*src);
      else
      	*dst++ = *src;
    }
  *dst = '\0';
  inst[0] = '\0';

  /* get changepw tickets.  We use this service because it's the
   * only one that guarantees we used the password rather than a
   * ticket granting ticket.
   */
  status = krb_get_pw_in_tkt(login, inst, realm, "changepw", hosti, 5, passwd);
  if (status)
    return status + krb_err_base;

  status = krb_mk_req(&cred, "changepw", hosti, realm, 0);
  if (status)
    return status + krb_err_base;

  /* round up to word boundry */
  bp = (char *)((((u_long)bp) + 3) & 0xfffffffc);

  /* put the ticket in the packet */
  len = cred.length;
  cred.length = htonl(cred.length);
  memmove(bp, &(cred), sizeof(int) + len);
  bp += sizeof(int) + len;

  /* encrypt the data in the session key */
  sprintf(data, "%s,%s", idnumber, newpasswd);
  len = strlen(data);
  len = ((len + 7) >> 3) << 3;

  status = krb_get_cred("changepw", hosti, realm, &creds);
  if (status)
    {
      memset(data, 0, strlen(data));
      return status + krb_err_base;
    }
  dest_tkt();

  des_key_sched(creds.session, keys);
  des_pcbc_encrypt(data, bp + sizeof(int), len, keys, creds.session, 1);
  *((int *)bp) = htonl(len);
  memset(data, 0, strlen(data));

  bp += len + sizeof(int);

  len = bp - buf;
  return do_call(buf, len, seq_no, 0);
}

int do_call(char *buf, int len, int seq_no, char *login)
{
  struct timeval timeout;
  char ibuf[1024];
  fd_set set;

  int retry = 0;

  do
    {
      if (write(reg_sock, buf, len) != len)
	return errno;

      FD_ZERO(&set);
      FD_SET(reg_sock, &set);
      timeout.tv_sec = 30;
      timeout.tv_usec = 0;
      do
	{
	  int rtn;
	  struct sockaddr_in s_in;
	  int addrlen = sizeof(s_in);
	  int vno;
	  int sno;
	  int stat;

	  rtn = select(reg_sock + 1, &set, NULL, NULL, &timeout);
	  if (rtn == 0)
	    break;
	  else if (rtn < 0)
	    return errno;

	  len = recvfrom(reg_sock, ibuf, BUFSIZ, 0,
			 (struct sockaddr *)&s_in, &addrlen);
	  if (len < 0)
	    return errno;
	  if (len < 12)
	    return UREG_BROKEN_PACKET;
	  memcpy(&vno, ibuf, sizeof(long));
	  vno = ntohl((u_long)vno);
	  if (vno != 1)
	    continue;
	  memcpy(&sno, ibuf + 4, sizeof(long));

	  if (sno != seq_no)
	    continue;

	  memcpy(&stat, ibuf + 8, sizeof(long));
	  stat = ntohl((u_long)stat);
	  if (login && len > 12)
	    {
	      memcpy(login, ibuf + 12, len - 12);
	      login[len - 12] = '\0';
	    }
	  else if (login)
	    *login = '\0';
	  return stat;
	}
      while (1);
    }
  while (++retry < 10);
  return ETIMEDOUT;
}
