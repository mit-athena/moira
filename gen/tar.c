/* $Id: tar.c 3956 2010-01-05 20:56:56Z zacheiss $
 *
 * Utility routines for writing tar files.
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include <moira_site.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

RCSID("$HeadURL: svn+ssh://svn.mit.edu/moira/trunk/moira/gen/tar.c $ $Id: tar.c 3956 2010-01-05 20:56:56Z zacheiss $");

static char tar_zeros[512];

TARFILE *tarfile_open(char *file)
{
  TARFILE *tf;

  tf = malloc(sizeof(TARFILE));
  if (!tf)
    {
      perror("couldn't open tarfile");
      exit(MR_OCONFIG);
    }

  tf->fp = fopen(file, "w");
  if (!tf->fp)
    {
      perror("couldn't open tarfile");
      exit(MR_OCONFIG);
    }

  return tf;
}

void tarfile_close(TARFILE *tf)
{
  long offset;
  int blocks;

  offset = ftell(tf->fp);
  /* The tar file must have two empty 512-byte blocks at the end, and
   * then must be padded to a multiple of 20 512-byte blocks. This
   * calculates how many blocks to add.
   */
  blocks = (20 - ((offset / 512) + 2) % 20) + 2;

  while (blocks--)
    {
      if (fwrite(tar_zeros, 512, 1, tf->fp) != 1)
	{
	  perror("could not write last tarfile block");
	  exit(MR_OCONFIG);
	}
    }

  fclose(tf->fp);
  free(tf);
}

FILE *tarfile_start(TARFILE *tf, char *name, mode_t mode, uid_t uid, gid_t gid,
		    char *user, char *group, time_t mtime)
{
  memset(&(tf->th), 0, sizeof(tf->th));
  memset(tf->th.chksum, ' ', sizeof(tf->th.chksum));
  strcpy(tf->th.name, (name[0] == '/' ? name + 1 : name));
  sprintf(tf->th.mode, "%07lo", (unsigned long)mode);
  sprintf(tf->th.uid, "%07lo", (unsigned long)uid);
  sprintf(tf->th.gid, "%07lo", (unsigned long)gid);
  sprintf(tf->th.mtime, "%011lo", (unsigned long)mtime);
  tf->th.typeflag[0] = '0';
  sprintf(tf->th.magic, "ustar");
  sprintf(tf->th.version, "00");
  sprintf(tf->th.uname, "%.32s", user);
  sprintf(tf->th.gname, "%.32s", group);

  tf->offset = ftell(tf->fp);

  if (fwrite(&(tf->th), sizeof(tf->th), 1, tf->fp) != 1)
    {
      perror("could not write tarfile header");
      exit(MR_OCONFIG);
    }
  return tf->fp;
}

void tarfile_end(TARFILE *tf)
{
  long offset = ftell(tf->fp);
  unsigned long size;
  int chksum = 0;
  char *p;

  size = offset - tf->offset;
  if (size % 512)
    {
      if (fwrite(tar_zeros, 512 - (size % 512), 1, tf->fp) != 1)
	goto err;
    }
  sprintf(tf->th.size, "%011lo", size - 512);

  for (p = (char *)&(tf->th); p < (char *)(&(tf->th) + 1); p++)
    chksum += (unsigned char)*p;
  sprintf(tf->th.chksum, "%07lo", chksum);

  if (fseek(tf->fp, tf->offset, SEEK_SET))
    goto err;
  if (fwrite(&(tf->th), sizeof(tf->th), 1, tf->fp) != 1)
    goto err;
  if (fseek(tf->fp, 0, SEEK_END))
    goto err;

  return;

err:
  perror("could not finish tarfile entry");
  exit(MR_OCONFIG);
}

void tarfile_mkdir(TARFILE *tf, char *name, mode_t mode, uid_t uid, gid_t gid,
		   char *user, char *group, time_t mtime)
{
  struct tarheader th;
  char *p;
  int chksum = 0;

  memset(&th, 0, sizeof(th));
  memset(th.chksum, ' ', sizeof(th.chksum));
  strcpy(th.name, name);
  sprintf(th.mode, "%07lo", (unsigned long)mode);
  sprintf(th.uid, "%07lo", (unsigned long)uid);
  sprintf(th.gid, "%07lo", (unsigned long)gid);
  sprintf(th.size, "%011lo", (unsigned long)0);
  sprintf(th.mtime, "%011lo", (unsigned long)mtime);
  th.typeflag[0] = '5';
  sprintf(th.magic, "ustar");
  sprintf(th.version, "00");
  sprintf(th.uname, "%.32s", user);
  sprintf(th.gname, "%.32s", group);

  for (p = (char *)&th; p < (char *)(&th + 1); p++)
    chksum += (unsigned char)*p;
  sprintf(th.chksum, "%07lo", chksum);

  if (fwrite(&th, sizeof(th), 1, tf->fp) != 1)
    {
      perror("could not write tarfile header");
      exit(MR_OCONFIG);
    }
}
