/* $Id: update_test.c,v 1.8 1998-02-05 22:52:04 danw Exp $
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

#include <stdio.h>
#include <stdlib.h>

#include <gdb.h>

void usage(void);

CONNECTION conn;
char *whoami;

int main(int argc, char **argv)
{
  char *host, service_address[256], *file, *rfile, buf[256];
  int code, i;

  whoami = argv[0];
  initialize_sms_error_table();
  initialize_krb_error_table();
  gdb_init();

  if (argc < 2)
    usage();
  host = argv[1];

  sprintf(service_address, "%s:%s", host, SERVICE_NAME);
  conn = start_server_connection(service_address, "");
  if (!conn || (connection_status(conn) == CON_STOPPED))
    {
      com_err(whoami, connection_errno(conn),
	      " can't connect to update %s", service_address);
      return MR_CANT_CONNECT;
    }
  code = send_auth(host);
  if (code)
    com_err(whoami, code, " authorization attempt failed");

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
	  send_file(file, rfile, 0);
	  break;
	case 'S':
	  if (i + 2 >= argc)
	    usage();
	  file = argv[++i];
	  rfile = argv[++i];
	  fprintf(stderr, "Sending (encrypted) file %s to %s as %s\n",
		  file, host, rfile);
	  send_file(file, rfile, 1);
	  break;
	case 'i':
	  if (i + 1 >= argc)
	    usage();
	  file = argv[++i];
	  strcpy(buf, "/tmp/moira-updateXXXXX");
	  mktemp(buf);
	  fprintf(stderr, "Sending instructions %s to %s as %s\n",
		  file, host, buf);
	  send_file(file, buf, 0);
	  break;
	case 'I':
	  if (i + 2 >= argc)
	    usage();
	  file = argv[++i];
	  rfile = argv[++i];
	  strcpy(buf, rfile);
	  fprintf(stderr, "Sending instructions %s to %s as %s\n",
		  file, host, buf);
	  send_file(file, buf, 0);
	  break;
	case 'x':
	  fprintf(stderr, "Executing instructions %s on %s\n", buf, host);
	  code = execute(buf);
	  if (code)
	    com_err(whoami, code, "executing");
	  break;
	case 'X':
	  if (i + 1 >= argc)
	    usage();
	  file = argv[++i];
	  fprintf(stderr, "Executing instructions %s on %s\n", file, host);
	  code = execute(file);
	  if (code)
	    com_err(whoami, code, "executing");
	  break;
	case 'n':
	  break;
	default:
	  usage();
	}
    }
  send_quit();
  conn = sever_connection(conn);
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
