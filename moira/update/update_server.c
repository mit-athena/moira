/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/update_server.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/update_server.c,v 1.2 1988-08-23 11:51:47 mar Exp $
 */

#ifndef lint
static char *rcsid_dispatch_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/update_server.c,v 1.2 1988-08-23 11:51:47 mar Exp $";
#endif	lint

#include <stdio.h>
#include <gdb.h>
#include <errno.h>
#include <strings.h>
#include <sms.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include "update.h"

/* XXX */
#include "kludge.h"
/* XXX */

extern int auth_001(), inst_001();
extern int xfer_002(), exec_002();

extern int sync_proc(), quit();

extern void gdb_debug();
extern int exit(), abort(), errno;
extern STRING instructions;

CONNECTION conn;
int code;
char *whoami;

int have_authorization = 0;
int have_file = 0;
int have_instructions = 0;
int done = 0;


#define send_int(n) \
     (_send_int=(n),send_object(conn,(char *)&_send_int,INTEGER_T))
int _send_int;

struct _dt {
     char *str;
     int (*proc)();
} dispatch_table[] = {
     { "INST_001", inst_001 },
     { "AUTH_001", auth_001 },
     { "XFER_002", xfer_002 },
     { "EXEC_002", exec_002 },
     { "quit", quit },
     { (char *)NULL, abort }
};

/*
 * general scratch space -- useful for building
 * error messages et al...
 */
char buf[BUFSIZ];
err(code, fmt)
     int code;
     char *fmt;
{
     sprintf(buf, fmt, error_message(code));
     sms_log_error(buf);
}

main(argc, argv)
     int argc;
     char **argv;
{
     STRING str;
     struct _dt *d;
     int n;

#ifdef DEBUG
     gdb_debug(GDB_NOFORK);
#endif /* DEBUG */

     whoami = rindex(argv[0], '/');
     if (whoami)
	 whoami++;
     else
	 whoami = argv[0];

     /* interpret arguments here */
     if (argc != 1) {
	  fprintf(stderr, "Usage:  %s\n", whoami);
	  exit(1);
     }
     /* well, sort of... */

#ifndef DEBUG
     if (fork())
       exit(0);
     n = open("/dev/tty", O_RDWR|O_NDELAY);
     if (n > 0) {
	 (void) ioctl(n, TIOCNOTTY, (char *)NULL);
	 (void) close(n);
     }
#endif

     umask(0022);
     init_sms_err_tbl();
     init_krb_err_tbl();
     sms_update_initialize();

     /* wait for connection */
     gdb_init();
     conn = create_forking_server(SERVICE_NAME, 0);
     if (!conn) {
	  err(errno, "%s: can't get connection");
	  exit(1);
     }
     if (connection_status(conn) == CON_STOPPED) {
	 com_err(whoami, connection_errno(conn), ": can't get connection");
	 exit(1);
     }

     sms_log_info("got connection");
     /* got a connection; loop forever */
     while (1) {
	  register char *cp;
	  code = receive_object(conn, (char *)&str, STRING_T);
	  if (code) {
	       err(connection_errno(conn), "%s: receiving command");
	       sever_connection(conn);
	       exit(1);
	  }
	  cp = index(STRING_DATA(str), ' ');
	  if (cp)
	       *cp = '\0';
	  for (d = dispatch_table; d->str; d++) {
	      if (!strcmp(d->str, STRING_DATA(str))) {
		  if (cp)
		      *cp = ' ';
#ifdef	DEBUG
		  printf("got request: %s\n", STRING_DATA(str));
#endif	/* DEBUG */
		  (void)(d->proc)(STRING_DATA(str));
		  goto ok;
	      }
	  }
	  sprintf(buf, "unknown request received: %s\n", STRING_DATA(str));
	  sms_log_error(buf);
	  code = send_int(SMS_UNKNOWN_PROC);
	  if (code) {
	      err(connection_errno(conn), "%s: sending UNKNOWN_PROC");
	  }
     ok:
	  string_free(&str);
     }
}

int
send_ok()
{
     static int zero = 0;
     return((code = send_object(conn, (char *)&zero, INTEGER_T)));
}


initialize()
{
     /* keep have_authorization around */
     have_file = 0;
     have_instructions = 0;
     done = 0;
     if (STRING_DATA(instructions) != (char *)NULL)
	  string_free(&instructions);
}


/*
 * quit request:
 *
 * syntax:
 * >>> quit
 * <<< (int)0
 * any arguments are ignored
 *
 * function:
 *	closes connection from SMS
 */
int
quit(str)
     char *str;
{
#ifdef lint
     str = (char *)NULL;
#endif /* lint */
     (void) send_ok();
     sever_connection(conn);
     sms_log_info("Closing connection.");
     exit(0);
}


/*
 * lose(msg)
 *
 * put <msg> to log as error, break connection, and exit
 */

lose(msg)
    char *msg;
{
    sprintf(buf, "%s: %s", error_message(code), msg);
    sms_log_error(buf);
    if (conn)
	sever_connection(conn);
    exit(1);
}

/*
 * report_error(msg)
 *
 * send back (external) <code>; if error, punt big with <lose(msg)>
 */

report_error(msg)
    char *msg;
{
    code = send_object(conn, (char *)&code, INTEGER_T);
    if (code) {
	code = connection_errno(conn);
	lose(msg);
    }
}

/*
 * reject_call(c)
 *
 * set (external) <code> to <c> and call <report_error>
 */

reject_call(c)
    int c;
{
    code = c;
    report_error("call rejected");
}
