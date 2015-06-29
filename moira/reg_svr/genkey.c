/* $Id: genkey.c 3956 2010-01-05 20:56:56Z zacheiss $
 *
 * Utility program to generate a public/private key pair
 *
 * Copyright (C) 1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 */

#include <mit-copyright.h>
#include <moira.h>
#include "reg_svr.h"

#include <sys/param.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com_err.h>

/* RSARef includes */
#include "global.h"
#include "rsaref.h"

RCSID("$HeadURL: svn+ssh://svn.mit.edu/moira/trunk/moira/reg_svr/genkey.c $ $Id: genkey.c 3956 2010-01-05 20:56:56Z zacheiss $");

void printhex(FILE *out, unsigned char *buf, int len);

int main(int argc, char **argv)
{
  R_RSA_PRIVATE_KEY private;
  R_RSA_PUBLIC_KEY public;
  R_RSA_PROTO_KEY proto;
  R_RANDOM_STRUCT random;
  FILE *in, *out;
  int needed;
  unsigned char *buf;
  char pubname[MAXPATHLEN], oldname[MAXPATHLEN];
  char *whoami = argv[0], *inname = argv[1], *outname = argv[2];

  if (strchr(whoami, '/'))
    whoami = strrchr(whoami, '/');

  if (argc != 3)
    {
      com_err(NULL, 0, "Usage: %s infile outfile\n"
	      "where infile is a file of random data", whoami);
      exit(1);
    }
  in = fopen(inname, "r");
  if (!in)
    {
      com_err(whoami, errno, "trying to open %s", inname);
      exit(1);
    }

  R_RandomInit(&random);
  R_GetRandomBytesNeeded(&needed, &random);
  buf = malloc(needed);
  if (fread(buf, needed, 1, in) != 1)
    {
      com_err(whoami, 0, "Not enough random input data: need %d bytes\n",
	      needed);
      exit(1);
    }
  R_RandomUpdate(&random, buf, needed);

  proto.bits = 1024;
  proto.useFermat4 = 1;
  
  if (R_GeneratePEMKeys(&public, &private, &proto, &random))
    {
      com_err(whoami, 0, "Couldn't generate key");
      exit(1);
    }

  sprintf(oldname, "%s.old", outname);
  rename(outname, oldname);
  out = fopen(outname, "w");
  if (!out)
    {
      com_err(whoami, errno, "opening %s", outname);
      exit(1);
    }
  if (fwrite(&private, sizeof(private), 1, out) != 1)
    {
      com_err(whoami, errno, "writing %s", outname);
      exit(1);
    }
  fclose(out);

  sprintf(pubname, "%s.pub", outname);
  sprintf(oldname, "%s.old", pubname);
  rename(pubname, oldname);
  out = fopen(pubname, "w");
  if (!out)
    {
      com_err(whoami, errno, "opening %s", pubname);
      exit(1);
    }
  if (fwrite(&public, sizeof(public), 1, out) != 1)
    {
      com_err(whoami, errno, "writing %s", pubname);
      exit(1);
    }
  fclose(out);

  sprintf(pubname, "%s.pub.txt", outname);
  out = fopen(pubname, "w");
  if (!out)
    {
      com_err(whoami, errno, "opening %s", pubname);
      exit(1);
    }
  printhex(out, public.modulus, MAX_RSA_MODULUS_LEN);
  fclose(out);

  exit(0);
}

char hexd[] = { '0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

void printhex(FILE *out, unsigned char *buf, int len)
{
  while (len--)
    {
      fprintf(out, "%c%c", hexd[*buf>>4], hexd[*buf%0x10]);
      buf++;
    }
}
