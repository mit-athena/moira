/* $Id: globals.c 3956 2010-01-05 20:56:56Z zacheiss $
 *
 *	This is the file globals.c for the Moira Client, which allows users
 *      to quickly and easily maintain most parts of the Moira database.
 *	It Contains:  globals variable definitions.
 *
 *	Created: 	4/12/88
 *	By:		Chris D. Peterson
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include "defs.h"

RCSID("$HeadURL: svn+ssh://svn.mit.edu/moira/trunk/moira/clients/moira/globals.c $ $Id: globals.c 3956 2010-01-05 20:56:56Z zacheiss $");

Bool verbose;			/* TRUE if verbose mode is active (default). */
Bool found_some;		/* used by lists.c for determining if there
				   are any members of a given type. */
char *user;			/* The name of the user executing this
				   program. */
char *program_name;		/* The name of this instance of the program. */

