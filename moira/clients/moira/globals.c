#if (!defined(lint) && !defined(SABER))
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/globals.c,v 1.6 1990-03-17 17:10:31 mar Exp $";
#endif lint

/*	This is the file globals.c for the Moira Client, which allows a nieve
 *      user to quickly and easily maintain most parts of the Moira database.
 *	It Contains:  globals variable definitions.
 *	
 *	Created: 	4/12/88
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/globals.c,v $
 *      $Author: mar $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/globals.c,v 1.6 1990-03-17 17:10:31 mar Exp $
 *	
 *  	Copyright 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include "mit-copyright.h"
#include "defs.h"

Bool verbose;			/* TRUE if verbose mode is active (default). */
Bool found_some;		/* used by lists.c for determining if there 
				   are any members of a given type. */
char *user;			/* The name of the user executing this
				   program. */
char *program_name;		/* The name of this instance of the program. */

