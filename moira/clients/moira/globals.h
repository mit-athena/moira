/*	This is the file globals.h for the Moira Client, which allows users
 *      to quickly and easily maintain most parts of the Moira database.
 *	It Contains:  globals variable definitions.
 *
 *	Created: 	4/12/88
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/globals.h,v $
 *      $Author: danw $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/globals.h,v 1.7 1998-01-07 17:12:58 danw Exp $
 *
 *  	Copyright 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution
 *	see the file mit-copyright.h
 */

#include "mit-copyright.h"
#include "defs.h"

#ifndef _moira_client_globals_
#define _moira_client_globals_

extern Bool verbose;		/* TRUE if verbose mode is active (default). */
extern Bool found_some;		/* used by lists.c for determining if there
				 are any members of a given type. */
extern char *user;		/* The name of the user executing this
				   program. */
extern char *program_name;	/* The name of this instance of the program. */

#endif /* _moira_client_globals_ */ /* Do Not Add Anything After This Line */
