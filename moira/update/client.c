/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/client.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/client.c,v 1.8 1989-08-21 21:45:36 mar Exp $
 */

#ifndef lint
static char *rcsid_client2_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/client.c,v 1.8 1989-08-21 21:45:36 mar Exp $";
#endif	lint

/*
 * MODULE IDENTIFICATION:
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/client.c,v 1.8 1989-08-21 21:45:36 mar Exp $
 *	Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 * DESCRIPTION:
 *	This code handles the actual distribution of data files
 *	to servers in the SMS server-update program.
 * AUTHOR:
 *	Ken Raeburn (spook@athena.MIT.EDU),
 *		MIT Project Athena/MIT Information Systems.
 * DEFINED VALUES:
 *	conn
 *	sms_update_server
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
#include <sms.h>
#include <krb.h>

extern char *malloc();
extern int errno, dbg;

static char buf[BUFSIZ];
static int code;

CONNECTION conn;


/*
 * FUNCTION:
 *	initialize()
 * DESCRIPTION:
 *	Insures that various libraries have a chance to get
 *	initialized.
 * INPUT:
 * OUTPUT:
 * RETURN VALUE:
 *	void
 * SIDE EFFECTS:
 *	Initializes GDB library.
 * PROBLEMS:
 *
 */
static void
initialize()
{
    static int initialized = 0;

    if (!initialized) {
	gdb_init();
	initialized++;
    }
}


/*
 * FUNCTION:
 *	sms_update_server(service, machine, target_path)
 * DESCRIPTION:
 *	Attempts to perform an update to the named machine
 *	of the named service.  The file SMS_DIR/dcm/service.out
 *	will be sent, then the file SMS_DIR/bin/service.sh,
 *	the the shell script will be executed.
 * INPUT:
 *	service
 *		Name of service to be updated; used to find
 *		the source data file in the SMS data directory.
 *	machine
 *	target_path
 *		Location to install the file.
 * RETURN VALUE:
 *	int:
 *		Error code, or zero if no error was detected
 *		in the data supplied to this routine.
 * SIDE EFFECTS:
 *	May write information to logs.
 * PROBLEMS:
 *
 */

int
sms_update_server(service, machine, target_path, instructions)
char *service;
char *machine;
char *target_path;
char *instructions;
{
#define ASSERT(condition,stat,amsg) \
    if (!(condition)) { com_err(whoami, stat, amsg); return(stat); }
#define ASSERT2(condition,stat,amsg,arg1) \
    if (!(condition)) { com_err(whoami, stat, amsg, arg1); return(stat); }
#define NONNULL(str) \
    (((str) != (char *)NULL) && (strlen(str) != 0))

    char *service_address, *service_updated, *pathname;
    int on;
    
    /* some sanity checking of arguments while we build data */
    ASSERT(NONNULL(machine), SMS_INTERNAL, " null host name");
    ASSERT(NONNULL(service), SMS_INTERNAL, " null service name");
    ASSERT((strlen(machine) + strlen(service) + 2 < BUFSIZ), SMS_ARG_TOO_LONG,
	   " machine and service names");
    sprintf(buf, "%s:%s", machine, service);
    service_updated = strsave(buf);
    ASSERT(NONNULL(service_updated), SMS_NO_MEM, " for service name");
    ASSERT((strlen(machine)+strlen(SERVICE_NAME)+2 < BUFSIZ), SMS_ARG_TOO_LONG,
	   " machine and update service name");
    sprintf(buf, "%s:%s", machine, SERVICE_NAME);
    service_address = strsave(buf);
    ASSERT(NONNULL(service_address), SMS_NO_MEM, " for service address");
    ASSERT(NONNULL(target_path), SMS_INTERNAL, " null target pathname");
    ASSERT((strlen(target_path) < MAXPATHLEN), SMS_ARG_TOO_LONG,
	   " target pathname");
    ASSERT2(target_path[0] == '/', SMS_NOT_UNIQUE,
	   " non-absolute pathname supplied \"%s\"", target_path);
    sprintf(buf, "%s/dcm/%s.out", SMS_DIR, service);
    pathname = strsave(buf);
    ASSERT(NONNULL(pathname), SMS_NO_MEM, " for pathname");
    ASSERT(NONNULL(instructions), SMS_NO_MEM, " for instructions");
    ASSERT((strlen(instructions) < MAXPATHLEN), SMS_ARG_TOO_LONG,
	   " instruction pathname");
    
    initialize();
    com_err(whoami, 0, "starting update for %s", service_updated);
    
    /* open connection */
    conn = start_server_connection(service_address, 0);
    if (!conn || (connection_status(conn) == CON_STOPPED)) {
	com_err(whoami, connection_errno(conn),
		" can't connect to update %s", service_address);
	return(SMS_CANT_CONNECT);
    }
    on = 1;
    setsockopt(conn->in.fd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on));
    
    /* send authenticators */
    code = send_auth(machine);
    if (code) {
	com_err(whoami, code, " authorization attempt to %s failed",
		service_updated);
	goto update_failed;
    }
    
    code = send_file(pathname, target_path);
    if (code)
      goto update_failed;

    /* send instructions for installation */
    strcpy(buf, "/tmp/sms-update.XXXXXX");
    mktemp(buf);
    code = send_file(instructions, buf);
    if (code)
      goto update_failed;

    /* perform installation */
    code = execute(buf);
    if (code) {
	com_err(whoami, code, " installation of %s failed, code = %d",
		service_updated, code);
	goto update_failed;
    }
    
    /* finished updates */
    code = SMS_SUCCESS;

 update_failed:
    send_quit();
    conn = sever_connection(conn);
    return(code);

#undef NONNULL
#undef ASSERT
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
    
    code = get_sms_update_ticket(host_name, ticket);
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
	return(response);
    }
    return(SMS_SUCCESS);
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
    if (dbg & DBG_TRACE)
      com_err(whoami, response, "execute returned %d", response);
    if (response)
      return(response);
    return(SMS_SUCCESS);
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
