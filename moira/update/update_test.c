/* $Id: update_test.c,v 1.10 1998-05-26 18:14:23 danw Exp $
 *
 * Test client for update_server protocol.
 *
 * Copyright 1992-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include <update.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void usage(void);

char *whoami;

int main(int argc, char **argv)
{
  char *host, *file, *rfile, *ibuf = NULL;
  int code, i, count = 0, conn;

  whoami = argv[0];
  mr_init();

  if (argc < 2)
    usage();
  host = argv[1];

  conn = mr_connect_internal(host, SERVICE_NAME);
  if (!conn)
    {
      com_err(whoami, errno, ": can't connect to %s:%s", host, SERVICE_NAME);
      exit(1);
    }

  code = send_auth(conn, host);
  if (code)
    com_err(whoami, code, "attempting authorization");

  for (i = 2; i < argc; i++)
    {
      if (argv[i][0] != '-')
	usage();
      switch (argv[i][1])
	{
	case 's':
	  if (i + 2 >= argc)
	    usage();
	  file = argv[++i];
	  rfile = argv[++i];
	  fprintf(stderr, "Sending file %s to %s as %s\n", file, host, rfile);
	  send_file(conn, file, rfile, 0);
	  break;
	case 'S':
	  if (i + 2 >= argc)
	    usage();
	  file = argv[++i];
	  rfile = argv[++i];
	  fprintf(stderr, "Sending (encrypted) file %s to %s as %s\n",
		  file, host, rfile);
	  send_file(conn, file, rfile, 1);
	  break;
	case 'i':
	  if (i + 1 >= argc)
	    usage();
	  file = argv[++i];
	  ibuf = strdup("/tmp/moira-updateXXXXX");
	  if (!ibuf)
	    {
	      com_err(whoami, ENOMEM, "sending instructions");
	      exit(1);
	    }
	  mktemp(ibuf);
	  fprintf(stderr, "Sending instructions %s to %s as %s\n",
		  file, host, ibuf);
	  send_file(conn, file, ibuf, 0);
	  break;
	case 'I':
	  if (i + 2 >= argc)
	    usage();
	  file = argv[++i];
	  ibuf = argv[++i];
	  strcpy(ibuf, rfile);
	  fprintf(stderr, "Sending instructions %s to %s as %s\n",
		  file, host, ibuf);
	  send_file(conn, file, ibuf, 0);
	  break;
	case 'x':
	  if (!ibuf)
	    {
	      fprintf(stderr, "No instructions sent.");
	      usage();
	    }
	  fprintf(stderr, "Executing instructions %s on %s\n", ibuf, host);
	  code = execute(conn, ibuf);
	  if (code)
	    com_err(whoami, code, "executing");
	  break;
	case 'X':
	  if (i + 1 >= argc)
	    usage();
	  file = argv[++i];
	  fprintf(stderr, "Executing instructions %s on %s\n", file, host);
	  code = execute(conn, file);
	  if (code)
	    com_err(whoami, code, "executing");
	  break;
	case 'n':
	  break;
	default:
	  usage();
	}
    }
  send_quit(conn);
  close(conn);
  exit(code);
}

void usage(void)
{
  fprintf(stderr, "Usage: test host [commands...]\n");
  fprintf(stderr, "  Commands are:\n");
  fprintf(stderr, "\t-s srcfile dstfile\tsends file\n");
  fprintf(stderr, "\t-S srcfile dstfile\tsends encrypted file\n");
  fprintf(stderr, "\t-i srcfile\t\tsends instructions\n");
  fprintf(stderr, "\t-I srcfile dstfile\tsends instructions\n");
  fprintf(stderr, "\t-x\t\texecutes last instructions\n");
  fprintf(stderr, "\t-X file\t\texecutes file\n");
  exit(1);
}
