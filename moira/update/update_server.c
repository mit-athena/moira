/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/update_server.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/update_server.c,v 1.1 1987-08-22 17:53:54 wesommer Exp $
 */

#ifndef lint
static char *rcsid_dispatch_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/update_server.c,v 1.1 1987-08-22 17:53:54 wesommer Exp $";
#endif	lint

#include <stdio.h>
#include "gdb.h"
#include <errno.h>
#include <strings.h>
#include "update.h"
#include "sms_update_int.h"
#include "smsu_int.h"

/* XXX */
#include "kludge.h"
/* XXX */

extern int auth_001(), exec_001(), inst_001(), xfer_001();
extern int xfer_002(), exec_002();

extern int sync_proc(), quit();

extern void gdb_debug();
extern int exit(), abort(), errno;

CONNECTION conn;
int code;
char *whoami;

#define send_int(n) \
     (_send_int=(n),send_object(conn,(char *)&_send_int,INTEGER_T))
int _send_int;

struct _dt {
     char *str;
     int (*proc)();
} dispatch_table[] = {
     { "INST_001", inst_001 },
     { "AUTH_001", auth_001 },
     { "XFER_001", xfer_001 },
     { "EXEC_001", exec_001 },
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

     umask(0077);
     sms_update_initialize();
     init_smsU_err_tbl();

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
	  code = send_int(SMSU_UNKNOWN_PROC);
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
