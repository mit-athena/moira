/*	This is the file globals.h for allmaint, the SMS client that allows
 *      a user to maintaint most important parts of the SMS database.
 *	It Contains: globals variable definitions
 *	
 *	Created: 	4/12/88
 *	By:		4/12/88
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/globals.h,v $
 *      $Author: kit $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/globals.h,v 1.2 1988-06-10 18:36:44 kit Exp $
 *	
 *  	Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include "mit-copyright.h"
#include "allmaint.h"

#ifndef _allmaint_globals_
#define _allmaint_globals_

extern Bool verbose;		/* TRUE if verbose mode is active (default). */
extern Bool found_some;		/* used by lists.c for determining if there 
				 are any members of a given type. */
extern char *user;		/* The name of the user executing allmaint. */
extern char *program_name;	/* The name of this instance of the program. */

#endif _allmaint_globals_	/* Do Not Add Anything After This Line */
