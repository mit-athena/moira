/* $Id: protocol.c,v 1.3 2002-02-25 16:41:17 zacheiss Exp $
 *
 * Reg_svr protocol and encryption/decryption routines
 *
 * Copyright (C) 1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 */

#include <mit-copyright.h>
#include <moira.h>
#include "reg_svr.h"

#include <sys/stat.h>

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <com_err.h>
#include <des.h>

/* RSARef includes */
#include "global.h"
#include "rsaref.h"

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/protocol.c,v 1.3 2002-02-25 16:41:17 zacheiss Exp $");

R_RSA_PRIVATE_KEY *rsa_key;
char *emsg[NUM_REG_ERRORS], *ename[NUM_REG_ERRORS];
extern char *whoami;

struct _handler {
  char *name;
  void (*handler)(reg_client *rc, int argc, char **argv);
} handlers[] = {
  { "RIFO", RIFO },
  { "SWRD", SWRD },
  { "LOGN", LOGN },
  { "PSWD", PSWD },
  { "QUIT", QUIT },
  { "SPIN", SPIN },
  { NULL, NULL }
};

void parse_pdu(reg_client *rc, long len, char *buf);
void printhex(unsigned char *buf, int len);

int read_rsa_key(void)
{
  struct stat statbuf;
  int fd;

  if (stat(REG_SVR_RSA_KEY, &statbuf))
    return 0;

  fd = open(REG_SVR_RSA_KEY, O_RDONLY);
  if (!fd)
    return 0;

  rsa_key = malloc(statbuf.st_size);
  if (!rsa_key)
    return 0;

  if (read(fd, rsa_key, statbuf.st_size) != statbuf.st_size)
    return 0;

  close(fd);
  return 1;
}

int read_errors(void)
{
  int i;
  char errbuf[100], *p;
  FILE *errs;

  errs = fopen(REG_SVR_ERROR_MESSAGES, "r");
  if (!errs)
    return 0;
  for (i = 0; i < NUM_REG_ERRORS && !feof(errs); i++)
    {
      if (errbuf[0] != '#' || errbuf[1] != ' ')
	sprintf(errbuf, "# %d", i);
      ename[i] = strdup(errbuf + 2);
      if (ename[i][strlen(ename[i]) - 1] == '\n')
	ename[i][strlen(ename[i]) - 1] = '\0';
      emsg[i] = strdup("");
      if (!ename[i] || !emsg[i])
	return 0;
      while (1) {
	if (!fgets(errbuf, sizeof(errbuf) - 1, errs))
	  break;
	if (*errbuf == '#')
	  break;

	if ((p = strchr(errbuf, '\n')) > errbuf)
	  {
	    *p = ' ';
	    *(p + 1) = '\0';
	  }
	emsg[i] = realloc(emsg[i], strlen(emsg[i]) + strlen(errbuf) + 1);
	if (!emsg[i])
	  return 0;
	strcat(emsg[i], errbuf);
      }
    }
  fclose(errs);

  if (i < NUM_REG_ERRORS)
    {
      com_err(whoami, 0, "Not enough error messages in %s",
	      REG_SVR_ERROR_MESSAGES);
      exit(1);
    }
  return 1;
}

void parse_packet(reg_client *rc, int type, long len, char *buf, int sleeping)
{
  switch (type)
    {
    case REG_RSA_ENCRYPTED_KEY:
      {
	unsigned char key[MAX_ENCRYPTED_KEY_LEN];
	unsigned int keylen;

	if (RSAPrivateDecrypt(key, &keylen, buf, len, rsa_key) || keylen != 8)
	  {
	    reply(rc, ENCRYPT_KEY, "INIT", "c", NULL);
	    return;
	  }
	des_key_sched(key, rc->sched);
	rc->encrypted = 1;

	if (sleeping)
	  reply(rc, DATABASE_CLOSED, "INIT", "c", NULL);
	else
	  reply(rc, NO_MESSAGE, "GETN", "c", NULL);
	return;
      }

    case REG_ENCRYPTED:
      {
	char *outbuf, iv[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	if (!rc->encrypted)
	  {
	    reply(rc, INTERNAL_ERROR, "INIT", "c", NULL,
		  "Encrypted packet unexpected");
	    return;
	  }

	outbuf = malloc(len + 7);
	if (!outbuf)
	  {
	    reply(rc, INTERNAL_ERROR, "INIT", "c", NULL, "Out of memory");
	    return;
	  }
	des_cbc_encrypt(buf, outbuf, len, rc->sched, iv, 0);

	/* Undo PKCS#5 padding */
	len -= outbuf[len - 1];

	parse_pdu(rc, len - 8, outbuf + 8);
	free(outbuf);
	return;
      }

#ifdef ALLOW_UNENCRYPTED
    case REG_UNENCRYPTED:
      parse_pdu(rc, len, buf);
      return;
#endif

    default:
      com_err(whoami, 0, "Bad packet (type %d, len %d)", type, len);
      rc->lastmod = 0;
    }
}

void parse_pdu(reg_client *rc, long len, char *buf)
{
  char **argv, *p;
  int argc, i;
  void (*handler)(reg_client *rc, int argc, char **argv) = NULL;

  if (len < 8 || strcmp(buf, "v1"))
    {
      com_err(whoami, 0, "Bad packet version number %s", buf);
      reply(rc, PROTOCOL_ERROR, "INIT", "c", NULL);
      return;
    }
  buf += 3;
  len -= 3;

  for (i = 0; handlers[i].name; i++)
    {
      if (!strcmp(buf, handlers[i].name))
	{
	  handler = handlers[i].handler;
	  break;
	}
    }
  if (!handler)
    {
      com_err(whoami, 0, "Bad packet request %s", buf);
      reply(rc, PROTOCOL_ERROR, "INIT", "c", NULL);
      return;
    }
  buf += 5;
  len -= 5;

  for (argc = 0, p = buf; p < buf + len; p++)
    {
      if (!*p)
	argc++;
    }

  argv = malloc(argc * sizeof(char *));
  if (!argv)
    {
      com_err(whoami, 0, "in parse_pdu");
      reply(rc, INTERNAL_ERROR, "INIT", "c", NULL, "Out of memory");
      return;
    }

  fprintf(stderr, "%s[#%d]: %s", whoami, rc->clientid, handlers[i].name);
  for (argc = 0, p = buf - 1; p < buf + len - 1; p++)
    {
      if (!*p)
	{
	  argv[argc++] = p + 1;
	  if (strcmp(handlers[i].name, "PSWD") != 0)
	    fprintf(stderr, " '%s'", p + 1);
	}
    }
  fprintf(stderr, "\n");
  fflush(stderr);

  for (i = 0; i < argc; i++)
    strtrim(argv[i]);
  handler(rc, argc, argv);
  free(argv);
}

void reply(reg_client *rc, int msg, char *state, char *clean, char *data,
	   ...)
{
  /* reply() can't malloc, since it might be returning an "out of memory"
     error. We'll use a static buffer which is much larger than any
     message we'd be returning, and callers have to make sure that any
     user-generated data is length-limited. */
  static char buf[8192], outbuf[8192];
  char *p;
  int len, pad, pcount;
  va_list ap;
  long junk;
  unsigned short *nrand;

  com_err(whoami, 0, "Reply: %s, go to state %s %s", ename[msg], state, clean);

  seed48(rc->random);
  junk = lrand48();
  memcpy(buf + 3, &junk, 4);
  junk = lrand48();
  memcpy(buf + 7, &junk, 4);
  nrand = seed48(rc->random);
  memcpy(rc->random, nrand, 6);

  memcpy(buf + 11, "v1", 3);
  memcpy(buf + 14, state, len = strlen(state) + 1);
  p = buf + 14 + len;
  va_start(ap, data);
  p += vsprintf(p, emsg[msg], ap);
  va_end(ap);
  *p++ = '\0';
  memcpy(p, clean, len = strlen(clean) + 1);
  p += len;
  if (data)
    {
      memcpy(p, data, len = strlen(data) + 1);
      p += len;
    }

  len = p - (buf + 3);
  pad = 8 - len % 8;
  for (pcount = pad; pcount; pcount--)
    buf[3 + len++] = pad;

  if (rc->encrypted)
    {
      char iv[8] = {0, 0, 0, 0, 0, 0, 0, 0};

      des_cbc_encrypt(buf + 3, outbuf + 3, len, rc->sched, iv, 1);
      p = outbuf;
      *p = REG_ENCRYPTED;
    }
  else
    {
      p = buf;
      *p = REG_UNENCRYPTED;
    }

  p[1] = len / 256;
  p[2] = len % 256;
  write(rc->fd, p, len + 3);

  /* If we're going to INIT, set lastmod to 0 to cause the connection
     to be closed once we return to the main loop */
  if (!strcmp(state, "INIT"))
    rc->lastmod = 0;
}

char hexd[] = { '0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

void printhex(unsigned char *buf, int len)
{
  while (len--)
    {
      printf("%c%c", hexd[*buf>>4], hexd[*buf%0x10]);
      buf++;
    }
  printf("\n");
}
