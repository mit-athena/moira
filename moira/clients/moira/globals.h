/*	This is the file globals.h for the SMS Client, which allows a nieve
 *      user to quickly and easily maintain most parts of the SMS database.
 *	It Contains:  globals variable definitions.
 *	
 *	Created: 	4/12/88
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/globals.h,v $
 *      $Author: mar $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/globals.h,v 1.5 1991-01-04 16:57:06 mar Exp $
 *	
 *  	Copyright 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include "mit-copyright.h"
#include "defs.h"

#ifndef _sms_client_globals_
#define _sms_client_globals_

extern Bool verbose;		/* TRUE if verbose mode is active (default). */
extern Bool found_some;		/* used by lists.c for determining if there 
				 are any members of a given type. */
extern char *user;		/* The name of the user executing this
				   program. */
extern char *program_name;	/* The name of this instance of the program. */

#endif /* _sms_client_globals_ */ /* Do Not Add Anything After This Line */
