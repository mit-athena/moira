#if (!defined(lint) && !defined(SABER))
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/globals.c,v 1.3 1988-06-29 20:12:07 kit Exp $";
#endif lint

/*	This is the file globals.h for allmaint, the SMS client that allows
 *      a user to maintaint most important parts of the SMS database.
 *	It Contains: globals variable definitions
 *	
 *	Created: 	4/12/88
 *	By:		4/12/88
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/globals.c,v $
 *      $Author: kit $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/globals.c,v 1.3 1988-06-29 20:12:07 kit Exp $
 *	
 *  	Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include "mit-copyright.h"
#include "allmaint.h"

Bool verbose;			/* TRUE if verbose mode is active (default). */
Bool found_some;		/* used by lists.c for determining if there 
				   are any members of a given type. */
char *user;			/* The name of the user executing allmaint. */
char *program_name;		/* The name of this instance of the program. */

