/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/client.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/client.c,v 1.1 1987-08-22 17:53:46 wesommer Exp $
 */

#ifndef lint
static char *rcsid_client2_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/client.c,v 1.1 1987-08-22 17:53:46 wesommer Exp $";
#endif	lint

/*
 * MODULE IDENTIFICATION:
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/client.c,v 1.1 1987-08-22 17:53:46 wesommer Exp $
 *	Copyright 1987 MIT Project Athena.
 * DESCRIPTION:
 *	This code handles the actual distribution of data files
 *	to servers in the SMS server-update program.
 * AUTHOR:
 *	Ken Raeburn (spook@athena.MIT.EDU),
 *		MIT Project Athena/MIT Information Systems.
 * DEFINED VALUES:
 *	conn
 *	update_info
 *	sms_update_server
 * VERSION HISTORY:
 *	$Log: not supported by cvs2svn $
 */

#include <stdio.h>
#include <strings.h>
#include "gdb.h"
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/param.h>
#include "update.h"
#include <errno.h>
#include "sms_update_int.h"
#include "smsu_int.h"
#include <krb.h>

extern char *malloc(), *error_message();
extern int errno;

/* XXX */
#include "kludge.h"
/* XXX */

static char buf[BUFSIZ];

CONNECTION conn;
struct update_desc *info;

static int code;

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
 *	Initializes GDB library and SMSU error table.
 * PROBLEMS:
 *
 */
static
void
initialize()
{
    static int initialized = 0;
    if (!initialized) {
	gdb_init();
	init_smsU_err_tbl();
	initialized++;
    }
}

/*
 * FUNCTION:
 *	sms_update_server(info)
 * DESCRIPTION:
 *	Attempts to perform an update based on the information
 *	contained in the data structure pointed to by "info".
 *	Errors in performing the update are logged with the
 *	sms_log* routines; the 'override' field may be updated
 *	to cause updates to occur other than on the regular
 *	schedule (generally to force an earlier retry).
 * INPUT:
 *	info->service_name
 *		Name of service to be updated; used to find
 *		the source data file in the SMS data directory.
 *	info->host_name
 *	info->target_path
 *		Location to install the file.
 *	info->enable
 *		Must be non-zero.
 *	info->instructions
 *		Sequence of commands to execute on remote
 *		machine to effect the installation.
 * OUTPUT:
 *	info->last_time
 *		Set to the current time if the update was
 *		attempted.
 *	info->success
 *		Set to non-zero if the update succeeded, set
 *		to zero otherwise.
 *	info->override
 *		Set to -1 if the update succeeds, to (possibly)
 *		some other value otherwise.
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
sms_update_server(update_info, pathname)
    struct update_desc *update_info;
    char *pathname;
{
#define ASSERT(condition,amsg) \
    if (!(condition)) { msg = amsg; code = 0; goto local_error; }
#define NONNULL(str) \
    (((str) != (char *)NULL) && ((size = strlen(str)) != 0))
#define	IO_ERROR(msg) \
    {log_priority=log_ERROR;com_err(whoami,connection_errno(conn),msg);goto io_error;}

    char *service_address;
    char *service_updated;
    char *msg;
    register char *cp;
    STRING string_data, string2;
    int size, num;
    int fd = -1;
    struct stat statb;
    
    /* variable initializations */
    code = 0;
    info = update_info;
    service_updated = (char *)NULL;
    STRING_DATA(string_data) = (char *)NULL;

    /* pessimism */
    info->success = 0;

    /* some sanity checking of arguments while we build data */
    strcpy(buf, "???:???");
    ASSERT(NONNULL(info->host_name), "null host name");
    /* XXX -- check length here */
    strcpy(buf, info->host_name);
    for (cp = buf; *cp; cp++)
	;
    strcpy(cp, ":???");
    ASSERT(NONNULL(info->service_name), "null service name");
    /* XXX -- check length here */
    strcpy(++cp, info->service_name);
    service_updated = malloc(strlen(buf)+1);
    /* XXX -- check allocation succeeded */
    strcpy(service_updated, buf);
    service_address = malloc(strlen(SERVICE_NAME)+strlen(info->host_name)+1);
    if (!service_address) {
	code = errno;
	info->override = -1;
	return(code);
    }
    strcpy(service_address, info->host_name);
    strcat(service_address, ":");
    strcat(service_address, SERVICE_NAME);
    ASSERT(info->enable, "server is disabled");
    ASSERT(NONNULL(info->target_path), "null target pathname");
    ASSERT(size < MAXPATHLEN, "target pathname too long");
    ASSERT(info->target_path[0] == '/', "non-absolute pathname supplied");
    
    initialize();
    
    string_alloc(&string2, BUFSIZ);
    
    sprintf(buf, "starting update for %s", service_updated);
    sms_log_info(buf);
    
    fd = open(pathname, O_RDONLY, 0);
    if (fd < 0) {
	code = errno;
	msg = pathname;
	goto local_error;
    }
    if (fstat(fd, &statb)) {
	code = errno;
	close(fd); fd = -1;
	strcat(buf, ": can't fstat:");
	strcat(buf, error_message(code));
	sms_log_error(buf);
	goto error_exit;
    }
    size = statb.st_size;
    
    /* open connection */
    conn = start_server_connection(service_address, 0);
    if (!conn) {
	com_err(whoami, 0, "can't connect to update %s", service_address);
    connect_failed:
	if (info->override<0 || info->override>INTERVAL_connection_failed)
	    info->override = INTERVAL_connection_failed;
	return(0);
    }
    else if (connection_status(conn) == CON_STOPPED) {
	com_err(whoami, connection_errno(conn), ": can't connect to update %s",
		service_address);
	goto connect_failed;
    }
    
    
    /* send authenticators */
    code = send_auth();
    if (code) {
	sprintf(buf, "authorization attempt to %s failed: %s\n",
		service_updated, error_message(code));
	goto update_failed;
    }
    
    /* send file over */
    fd = open(pathname, O_RDONLY, 0);
    if (fd < 0) {
	code = errno;
	msg = pathname;
	goto local_error;
    }
    sprintf(STRING_DATA(string2), "XFER_002 %d %d %s",
	    size, checksum_fd(fd), info->target_path);
    code = send_object(conn, (char *)&string2, STRING_T);
    if (code)
	IO_ERROR("%s: sending XFER_002 request");
    code = receive_object(conn, (char *)&num, INTEGER_T);
    if (code)
	IO_ERROR("%s: getting reply from XFER_002 request");
    if (num) {
	sprintf(buf, "transfer request to %s (XFER_002) rejected: %s",
		service_updated, error_message(num));
    update_failed:
	sms_log_error(buf);
	/*
	 * 	   * if the update fails, something is probably wrong on
	 * 	   * the remote side; we'll have to let a maintainer
	 * 	   * take care of it.  don't bother trying again any sooner
	 * 	   * than INTERVAL_update_failed minutes
	 * 	   */
    update_failed_1:
	if (info->override < INTERVAL_update_failed && info->override != -1)
	    info->override = INTERVAL_update_failed;
	goto do_quit;
    }
    /* send actual data */
    code = send_file(pathname, size);
    if (code)
	goto update_failed_1;
    string_free(&string_data);
    close(fd);
    
    /* send instructions for installation */
    strcpy(buf, "/tmp/sms-update.XXXXXX");
    mktemp(buf);
    fd = open(info->instructions, O_RDONLY, 0);
    if (fd < 0) {
	code = errno;
	log_priority = log_ERROR;
	com_err(whoami, code, ": can't open %s", info->instructions);
	send_quit();
	info->override = INTERVAL_local_error;
	goto error_exit;
    }
    if (fstat(fd, &statb)) {
	code = errno;
	close(fd);
	fd = -1;
	log_priority = log_ERROR;
	com_err(whoami, code, ": can't fstat %s", info->instructions);
	goto error_exit;
    }
    sprintf(STRING_DATA(string2), "XFER_002 %d %d %s",
	    statb.st_size, checksum_fd(fd), buf);
    code = send_object(conn, (char *)&string2, STRING_T);
    if (code)
	IO_ERROR("%s: sending request for transfer of instructions");
    code = receive_object(conn, (char *)&num, INTEGER_T);
    if (code)
	IO_ERROR("%s: lost reply from installation script request");
    if (num) {
	com_err(whoami, num, ": transfer request rejected for %s", buf);
	goto update_failed_1;
    }
    code = send_file(info->instructions, statb.st_size);
    if (code)
	goto update_failed_1;
    
    /* perform installation */
    code = execute(buf);
    if (code) {
	sprintf(buf, "installation of %s failed: %s", service_updated,
		error_message(code));
	sms_log_error(buf);
    }
    
    /* clear override timer and indicate success */
    info->override = -1;
    info->success = 1;

    /* finished updates */
do_quit:
    send_quit();
    
    /* fall through */
EGRESS:
    code = 0;
error_exit:
    info->last_time = time((long *)0);
    if (STRING_DATA(string2))
	string_free(&string2);
    if (STRING_DATA(string_data))
	string_free(&string_data);
    conn = sever_connection(conn);
    return(code);
    
local_error:
    log_priority = log_ERROR;
    com_err(whoami, code, code ? ": %s" : "%s", msg);
    return(SMSU_INTERNAL_ERROR);
    
io_error:
    sms_log_error(buf);
    if (info->override== -1 || info->override > INTERVAL_connection_lost)
	info->override = INTERVAL_connection_lost;
    goto EGRESS;
    
#undef IO_ERROR
#undef NONNULL
#undef ASSERT
}

static
send_auth()
{
    KTEXT_ST ticket_st;
    KTEXT ticket = &ticket_st;
    STRING data;
    register int code;
    int response;
    
    code = get_sms_update_ticket(info->host_name, ticket);
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
    return(0);
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
    return(0);
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
