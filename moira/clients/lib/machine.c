/* $Id$
 *
 * Shared routines for machine-related queries.
 *
 * Copyright (C) 1999 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include <mrclient.h>
#include "mrclient-internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <krb5.h>

RCSID("$HeadURL$ $Id$");

/* Parse a line of input, fetching an identifier.  NULL is returned if an identifier
 * is not found.  ';' is a comment character.
 */

struct mrcl_identifier_type *mrcl_parse_mach_identifier(char *s)
{
  struct mrcl_identifier_type *id;
  char *p, *lastchar;
  
  while (*s && isspace(*s))
    s++;
  lastchar = p = s;
  while (*p && *p != '\n' && *p != ';')
    {
      if (isprint(*p) && !isspace(*p))
        lastchar = p++;
      else
        p++;
    }
  lastchar++;
  *lastchar = '\0';
  if (p == s || strlen(s) == 0)
    return NULL;
  
  if (!(id = malloc(sizeof(struct mrcl_identifier_type))))
    return NULL;

  if ((p = strchr(s, ':')))
    {
      *p = '\0';
      id->value = ++p;
      if (!strcasecmp("hwaddr", s))
	id->type = MRCL_MID_HWADDR;
      else if (!strcasecmp("duid", s))
	id->type = MRCL_MID_DUID;
      else
        {
          id->type = MRCL_MID_ANY;
          *(--p) = ':';
          id->value = s;
        }
      id->value = strdup(id->value);
    }
  else
    {
      id->value = strdup(s);
      id->type = strcasecmp(s, "unknown") ? MRCL_MID_ANY : MRCL_MID_NONE;
    }
  return id;
}

struct mrcl_netaddr_type *mrcl_parse_netaddr(char *s)
{
  struct mrcl_netaddr_type *netaddr;
  char *p, *lastchar;

  while (*s && isspace(*s))
    s++;
  lastchar = p = s;
  while (*p && *p != '\n' && *p != ';')
    {
      if (isprint(*p) && !isspace(*p))
        lastchar = p++;
      else
        p++;
    }
  lastchar++;
  *lastchar = '\0';
  if (p == s || strlen(s) == 0)
    return NULL;

  if (!(netaddr = malloc(sizeof(struct mrcl_netaddr_type))))
    return NULL;

  if ((p = strchr(s, ':')))
    {
      *p = '\0';
      netaddr->network = strdup(s);
      netaddr->address = strdup(++p);

    }
  else
    return NULL;

  return netaddr;
}

struct mrcl_addropt_type *mrcl_parse_addropt(char *s)
{
  struct mrcl_addropt_type *addropt;
  char *p, *lastchar;

  while (*s && isspace(*s))
    s++;
  lastchar = p = s;
  while (*p && *p != '\n' && *p != ';')
    {
      if (isprint(*p) && !isspace(*p))
        lastchar = p++;
      else
        p++;
    }
  lastchar++;
  *lastchar = '\0';
  if (p == s || strlen(s) == 0)
    return NULL;

  if (!(addropt = malloc(sizeof(struct mrcl_addropt_type))))
    return NULL;

  if ((p = strrchr(s, ':')))
    {
      *p = '\0';
      addropt->address = strdup(s);
      addropt->opt = strdup(++p);

    }
  else
    return NULL;

  return addropt;
}
