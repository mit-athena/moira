/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/update_test.c,v 1.7 1998-01-05 19:53:58 danw Exp $
 *
 * Test client for update_server protocol.
 *
 * Reads commands from the command line:
 * test host [commands...]
 *	-s file file	sends file to host
 *	-S file file	sends encrypted file to host
 *	-i file		sends instruction file to host
 *	-x file		executes instructions
 *	-n		nop
 */

#include <mit-copyright.h>
#include <stdio.h>
#include <string.h>
#include <gdb.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <update.h>
#include <errno.h>
#include <moira.h>
#include <moira_site.h>
#include <krb.h>

CONNECTION conn;
char *whoami;

int main(int argc, char **argv)
{
  char *host, service_address[256], *file, *rfile, buf[256];
  int code, i, count = 0;

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

usage(void)
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
