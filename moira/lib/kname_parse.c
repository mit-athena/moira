/* $Id: kname_parse.c 3956 2010-01-05 20:56:56Z zacheiss $
 *
 * Provide a copy of kname_parse() from krb4 for when krb4 is no
 * longer available.
 *
 * Copyright (C) 2009 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>

#include <string.h>

#ifdef HAVE_KRB4
#include <krb.h>
#else
#include <mr_krb.h>

#define KRBET_KNAME_FMT                          (39525457L)
#define KNAME_FMT                                (KRBET_KNAME_FMT - ERROR_TABLE_BASE_krb)
#endif

RCSID("$HeadURL: svn+ssh://svn.mit.edu/moira/trunk/moira/lib/kname_parse.c $ $Id: kname_parse.c 3956 2010-01-05 20:56:56Z zacheiss $");

#define NAME    0               /* which field are we in? */
#define INST    1
#define REALM   2

int mr_kname_parse(char *np, char *ip, char *rp, char *fullname)
{
  char buf[MAX_K_NAME_SZ];
  char *rnext, *wnext;        /* next char to read, write */
  register char c;
  int backslash;
  int field;

  backslash = 0;
  rnext = buf;
  wnext = np;
  field = NAME;

  if (strlen(fullname) > MAX_K_NAME_SZ)
    return KNAME_FMT;
  (void) strcpy(buf, fullname);

  while ((c = *rnext++)) {
    if (backslash) {
      *wnext++ = c;
      backslash = 0;
      continue;
    }
    switch (c) {
    case '\\':
      backslash++;
      break;
    case '.':
      switch (field) {
      case NAME:
        if (wnext == np)
          return KNAME_FMT;
        *wnext = '\0';
        field = INST;
        wnext = ip;
        break;
      case INST:          /* We now allow period in instance */
      case REALM:
        *wnext++ = c;
        break;
      default:
        return KNAME_FMT;
      }
      break;
    case '@':
      switch (field) {
      case NAME:
        if (wnext == np)
          return KNAME_FMT;
        *ip = '\0';
        /* fall through */
      case INST:
        *wnext = '\0';
        field = REALM;
        wnext = rp;
        break;
      case REALM:
        return KNAME_FMT;
      default:
        return KNAME_FMT;
      }
      break;
    default:
      *wnext++ = c;
    }
    /*
     * Paranoia: check length each time through to ensure that we
     * don't overwrite things.
     */
    switch (field) {
    case NAME:
      if (wnext - np >= ANAME_SZ)
        return KNAME_FMT;
      break;
    case INST:
      if (wnext - ip >= INST_SZ)
        return KNAME_FMT;
      break;
    case REALM:
      if (wnext - rp >= REALM_SZ)
        return KNAME_FMT;
      break;
    default:
      return KNAME_FMT;
    }
  }
  *wnext = '\0';
  return 0;
}
