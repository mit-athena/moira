/* $Id$
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include "mr_server.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

RCSID("$HeadURL$ $Id$");

extern char *whoami;

char *requote(char *cp)
{
  int len = 0;
  char *out = xmalloc(4 * strlen(cp) + 3), *op = out;

  *op++ = '"';
  len++;
  while (*cp)
    {
      if (*cp == '\\' || *cp == '"')
	{
	  *op++ = '\\';
	  len++;
	}
      if (isprint(*cp))
	{
	  *op++ = *cp++;
	  len++;
	}
      else
	{
	  sprintf(op, "\\%03o", (unsigned char)*cp++);
	  op += 4;
	  len += 4;
	}
    }

  strcpy(op, "\"");
  len += 2;

  out = realloc(out, len); /* shrinking, so can't fail */
  return out;
}

void log_args(char *tag, int version, int argc, char **argv)
{
  char *buf, **qargv;
  int i, len;
  char *bp;

  qargv = xmalloc(argc * sizeof(char *));

  for (i = len = 0; i < argc; i++)
    {
      qargv[i] = requote(argv[i]);
      len += strlen(qargv[i]) + 2;
    }

  buf = xmalloc(len + 1);

  for (i = 0, *buf = '\0'; i < argc; i++)
    {
      if (i)
	strcat(buf, ", ");
      strcat(buf, qargv[i]);
      free(qargv[i]);
    }
  free(qargv);

  com_err(whoami, 0, "%s[%d]: %s", tag, version, buf);
  free(buf);
}

void mr_com_err(const char *whoami, long code, const char *fmt, va_list pvar)
{
  extern client *cur_client;

  if (whoami)
    {
      fputs(whoami, stderr);
      if (cur_client)
	fprintf(stderr, "[#%d]", cur_client->id);
      fputs(": ", stderr);
    }
  if (code) {
    fputs(error_message(code), stderr);
    fputs(" ", stderr);
  }
  if (fmt)
    vfprintf(stderr, fmt, pvar);
  putc('\n', stderr);
}


/* mr_trim_args: passed an argument vector, it will trim any trailing
 * spaces on the args by writing a null into the string.
 */

int mr_trim_args(int argc, char **argv)
{
  char **arg;
  unsigned char *p, *lastch;

  for (arg = argv; argc--; arg++)
    {
      for (lastch = p = (unsigned char *) *arg; *p; p++)
	{
	  if (!isspace(*p))
	    lastch = p;
	}
      if (p != lastch)
	{
	  if (isspace(*lastch))
	    *lastch = '\0';
	  else
	    if (*(++lastch))
	      *lastch = '\0';
	}
    }
  return 0;
}


/* returns a copy of the argv and all of it's strings */

char **mr_copy_args(char **argv, int argc)
{
  char **a;
  int i;

  a = xmalloc(argc * sizeof(char *));
  for (i = 0; i < argc; i++)
    a[i] = xstrdup(argv[i]);
  return a;
}


/* malloc or die! */
void *xmalloc(size_t bytes)
{
  void *buf = malloc(bytes);

  if (buf)
    return buf;

  critical_alert(whoami, "moirad", "Out of memory");
  exit(1);
}

void *xrealloc(void *ptr, size_t bytes)
{
  void *buf = realloc(ptr, bytes);

  if (buf)
    return buf;

  critical_alert(whoami, "moirad", "Out of memory");
  exit(1);
}

char *xstrdup(char *str)
{
  char *buf = strdup(str);

  if (buf)
    return buf;

  critical_alert(whoami, "moirad", "Out of memory");
  exit(1);
}

int mask_to_masksize(char *addr_type, char *mask)
{

  if (!strcmp(addr_type, "IPV4"))
    {
      struct in_addr in;

      if (inet_pton(AF_INET, mask, &in) == -1)
	return -1;
      return bit_count(ntohl(in.s_addr));
    }
  else if (!strcmp(addr_type, "IPV6"))
    {
      struct in6_addr in6;
      int i, bits = 0;

      if (inet_pton(AF_INET6, mask, &in6) == -1)
        return -1;

      for (i = 0; i < 16; i++) {
	bits += bit_count(ntohl(in6.s6_addr[i]));
      }

      return bits;
    }
  else
    return -1;
}

int bit_count(uint32_t i)
{
  int c = 0;
  unsigned int seen_one = 0;

  while (i > 0) {
    if (i & 1) {
      seen_one = 1;
      c++;
    } else {
      if (seen_one) {
        return -1;
      }
    }
    i >>= 1;
  }

  return c;
}

char *masksize_to_mask(char *addr_type, unsigned int prefix)
{
  int i, j;

  if (!strcmp(addr_type, "IPV4"))
    {
      struct in_addr in;
      char buf[INET_ADDRSTRLEN];

      if (prefix > 32)
        return NULL;

      memset(&in, 0, sizeof(in));

      if (prefix)
        in.s_addr = htonl(~((1 << (32 - prefix)) - 1));

      if (inet_ntop(AF_INET, &in, buf, INET_ADDRSTRLEN) == NULL)
        return NULL;
      else
        return strdup(buf);
    }
  else if (!strcmp(addr_type, "IPV6"))
    {
      struct in6_addr in6;
      char buf[INET6_ADDRSTRLEN];

      if (prefix > 128)
        return NULL;

      memset(&in6, 0, sizeof(in6));
      for (i = prefix, j = 0; i > 0; i -= 8, j++) {
        if (i >= 8) {
          in6.s6_addr[j] = 0xff;
        } else {
          in6.s6_addr[j] = (unsigned long)(0xffU << (8 - i));
        }
      }

      if (inet_ntop(AF_INET6, &in6, buf, INET6_ADDRSTRLEN) == NULL)
        return NULL;
      else
        return strdup(buf);
    }
  else
    return NULL;
}
