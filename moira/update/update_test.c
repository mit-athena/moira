/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/update_test.c,v 1.1 1992-08-25 14:47:13 mar Exp $
 *
 * Test client for update_server protocol.
 * 
 * Reads commands from the command line:
 * test host [commands...]
 *	-s file file	sends file to host
 *	-i file		sends instruction file to host
 *	-x file		executes instructions
 *	-n		nop
 */

#include <mit-copyright.h>
#include <stdio.h>
#include <strings.h>
#include <gdb.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <update.h>
#include <errno.h>
#include <dcm.h>
#include <moira.h>
#include <moira_site.h>
#include <krb.h>

CONNECTION conn;
char *whoami;


main(argc, argv)
int argc;
char **argv;
{
    char *host, service_address[256], *file, *rfile, buf[256];
    int code, i, count=0;

    whoami = argv[0];
    initialize_sms_error_table();
    initialize_krb_error_table();
    gdb_init();

    if (argc < 2) usage();
    host = argv[1];

    sprintf(service_address, "%s:%s", host, SERVICE_NAME);
    conn = start_server_connection(service_address, "");
    if (!conn || (connection_status(conn) == CON_STOPPED)) {
	com_err(whoami, connection_errno(conn),
		" can't connect to update %s", service_address);
	return(MR_CANT_CONNECT);
    }
    code = send_auth(host);
    if (code) {
	com_err(whoami, code, " authorization attempt failed");
    }

    for (i = 2; i < argc; i++) {
	if (argv[i][0] != '-') usage();
	switch (argv[i][1]) {
	case 's':
	    if (i+2 >= argc) usage();
	    file = argv[++i];
	    rfile = argv[++i];
	    fprintf(stderr, "Sending file %s to %s as %s\n", file, host, rfile);
	    send_file(file, rfile);
	    break;
	case 'i':
	    if (i+1 >= argc) usage();
	    file = argv[++i];
	    strcpy(buf, "/tmp/moira-updateXXXXX");
	    mktemp(buf);
	    fprintf(stderr, "Sending instructions %s to %s as %s\n",
		    file, host, buf);
	    send_file(file, buf);
	    break;
	case 'I':
	    if (i+2 >= argc) usage();
	    file = argv[++i];
	    rfile = argv[++i];
	    strcpy(buf, rfile);
	    fprintf(stderr, "Sending instructions %s to %s as %s\n",
		    file, host, buf);
	    send_file(file, buf);
	    break;
	case 'x':
	    fprintf(stderr, "Executing instructions %s on %s\n", buf, host);
	    code = execute(buf);
	    if (code) com_err(whoami, code, "executing");
	    break;
	case 'X':
	    if (i+1 >= argc) usage();
	    file = argv[++i];
	    fprintf(stderr, "Executing instructions %s on %s\n", file, host);
	    code = execute(file);
	    if (code) com_err(whoami, code, "executing");
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

usage()
{
    fprintf(stderr, "Usage: test host [commands...]\n");
    fprintf(stderr, "  Commands are:\n");
    fprintf(stderr, "\t-s srcfile dstfile\tsends file\n");
    fprintf(stderr, "\t-i srcfile\t\tsends instructions\n");
    fprintf(stderr, "\t-I srcfile dstfile\tsends instructions\n");
    fprintf(stderr, "\t-x\t\texecutes last instructions\n");
    fprintf(stderr, "\t-X file\t\texecutes file\n");
    exit(1);
}

static
send_auth(host_name)
char *host_name;
{
    KTEXT_ST ticket_st;
    KTEXT ticket = &ticket_st;
    STRING data;
    register int code;
    int response;
    
    code = get_mr_update_ticket(host_name, ticket);
    if (code) {
	return(code);
    }
    STRING_DATA(data) = "AUTH_001";
    MAX_STRING_SIZE(data) = 9;
    code = send_object(conn, (char *)&data, STRING_T);
    if (code) {
	return(connection_errno(conn));
    }
    code = receive_object(conn, (char *)&response, INTEGER_T);
    if (code) {
	return(connection_errno(conn));
    }
    if (response) {
	return(response);
    }
    STRING_DATA(data) = (char *)ticket->dat;
    MAX_STRING_SIZE(data) = ticket->length;
    code = send_object(conn, (char *)&data, STRING_T);
    if (code) {
	return(connection_errno(conn));
    }
    code = receive_object(conn, (char *)&response, INTEGER_T);
    if (code) {
	return(connection_errno(conn));
    }
    if (response) {
	com_err(whoami, response, "Permission to connect denied");
	return(response);
    }
    return(MR_SUCCESS);
}

static
execute(path)
    char *path;
{
    int response;
    STRING data;
    register int code;
    
    string_alloc(&data, BUFSIZ);
    sprintf(STRING_DATA(data), "EXEC_002 %s", path);
    code = send_object(conn, (char *)&data, STRING_T);
    if (code)
	return(connection_errno(conn));
    code = receive_object(conn, (char *)&response, INTEGER_T);
    if (code)
	return(connection_errno(conn));
    if (response)
      return(response);
    return(MR_SUCCESS);
}

send_quit()
{
    STRING str;
    if (!conn)
	return;
    string_alloc(&str, 5);
    (void) strcpy(STRING_DATA(str), "quit");
    (void) send_object(conn, (char *)&str, STRING_T);
    string_free(&str);
}
