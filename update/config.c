/* $Id: config.c,v 1.7 1998-02-15 17:49:27 danw Exp $
 *
 * Routines to handle configuration file for Moira's update_server.
 * These routines must load the file into memory rather than parse
 * it each time as one of the things the server may do is chroot()
 * itself.
 *
 * Copyright (C) 1992-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include "update_server.h"

#include <sys/stat.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

RCSID("$Header: /afs/athena.mit.edu/astaff/project/moiradev/repository/moira/update/config.c,v 1.7 1998-02-15 17:49:27 danw Exp $");

#define CONFIG_FILE	"/etc/athena/moira.conf"

/* Variables currently supported:
 * chroot directory	daemon will run chrooted to this directory
 * user username	daemon will run with this user's uid
 * port portname	daemon will listen on this port number
 * nofork		server stays in foreground & logs to stdout
 * auth krbname		this user is authorized to connect
 * noclobber		will not overwrite existing files
 * noexec		will not execute instructions received
 */

static char *config_buf = NULL;
static char **config_keys, **config_values;

static int init(void)
{
  int fd, count = 0;
  struct stat st;
  char *p, *start;

  /* Only execute once */
  if (config_buf)
    return MR_SUCCESS;

  fd = open(CONFIG_FILE, O_RDONLY, 0);
  if (fd < 0)
    {
      config_buf = "";
      config_keys = malloc(sizeof(char *) * 2);
      if (!config_keys)
	return ENOMEM;
      config_keys[0] = config_keys[1] = NULL;
      return MR_SUCCESS;
    }

  if (fstat(fd, &st) < 0)
    return MR_INTERNAL;

  config_buf = malloc(st.st_size + 2);
  if (!config_buf)
    return ENOMEM;

  if (read(fd, config_buf, st.st_size) < st.st_size)
    {
      free(config_buf);
      config_buf = NULL;
      return MR_INTERNAL;
    }
  config_buf[st.st_size] = '\0';

  for (p = config_buf; *p; p++)
    {
      if (*p == '\n')
	count++;
    }
  count++;
  config_keys = malloc(count * sizeof(char *));
  config_values = malloc(count * sizeof(char *));
  if (!config_keys || !config_values)
    {
      free(config_buf);
      free(config_keys);
      free(config_values);
      config_buf = NULL;
      return ENOMEM;
    }

  count = 0;
  for (p = strtok(config_buf, "\n"); p; p = strtok(NULL, "\n"))
    config_keys[count++] = p;
  config_keys[count] = NULL;

  for (count = 0; config_keys[count]; count++)
    {
      config_values[count] = "";
      for (p = config_keys[count]; *p; p++)
	{
	  if (isspace(*p))
	    {
	      *p++ = '\0';
	      while (*p && isspace(*p))
		p++;
	      config_values[count] = p;
	    }
	}
    }
  return MR_SUCCESS;
}


/* Given a key, lookup the associated value.
 * Returns "" on a key without a value, NULL on a non-existant key.
 * If a key appears multiple times, successive calls will cycle through
 * the possible values.
 */

char *config_lookup(char *key)
{
  static int i = 0;
  int start;

  if (init() != MR_SUCCESS)
    return NULL;

  start = i++;
  if (!config_keys[i])
    i = 0;
  if (!config_keys[i])
    return NULL;

  do
    {
      if (!strcasecmp(key, config_keys[i]))
	return config_values[i];
      if (!config_keys[++i])
	i = 0;
    }
  while (i != start);

  if (!strcasecmp(key, config_keys[i]))
    return config_values[i];

  return NULL;
}
