/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/inst_001.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/inst_001.c,v 1.1 1987-08-22 17:54:32 wesommer Exp $
 */

#ifndef lint
static char *rcsid_inst_001_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/inst_001.c,v 1.1 1987-08-22 17:54:32 wesommer Exp $";
#endif	lint

#include <stdio.h>
#include "gdb.h"
#include "kludge.h"
#include "sms_update_int.h"

STRING instructions = { 0, 0 };
extern CONNECTION conn;
extern int have_instructions;
extern int code;

/*
 * instruction sequence transmission:
 * syntax:
 *	>>> (STRING) "inst_001"
 *	<<< (int)0
 *	>>> (STRING) instructions
 *	<<< (int)0
 */

inst_001(str)
     char *str;
{
     if (send_ok())
	  lose("sending okay for inst_001");
     code = receive_object(conn, (char *)&instructions, STRING_T);
     if (code) {
	  report_error("reporting failure to receive instructions");
	  return;
     }
     have_instructions = 1;
     if (send_ok())
	  lose("sending okay after instructions");
     dprintf(("got instructions: %s\n", STRING_DATA(instructions)));
}
