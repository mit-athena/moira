/*	This is the file defs.h for the Moira Client, which allows users
 *      to quickly and easily maintain most parts of the Moira database.
 *	It Contains: useful definitions.
 *
 *	Created: 	4/12/88
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/defs.h,v $
 *      $Author: danw $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/defs.h,v 1.13 1998-01-07 17:12:53 danw Exp $
 *
 *  	Copyright 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution
 *	see the file mit-copyright.h
 */

#include "mit-copyright.h"

#ifndef _defs_header_
#define _defs_header_

#ifndef TRUE
#define TRUE 1
#endif /* TRUE */

#ifndef FALSE
#define FALSE 0
#endif /* FALSE */


/* Modification information is always printed in this format. */

#define MOD_FORMAT ("Last mod by %s at %s with %s.")

/* Some common default values. */

#define DEFAULT_NONE    ("")
#define DEFAULT_YES     ("1")
#define DEFAULT_NO      ("0")
#define DEFAULT_COMMENT ("none")

#define SUB_NORMAL 0
#define SUB_ERROR 1

#define MAX_ARGS_SIZE 100

typedef void (*FVoid)();	/* function pointer typedef. */
typedef char *(*FCharStar)();	/* function pointer typedef. */
typedef int (*FInt)();		/* function pointer typedef. */

typedef int Bool;

#define IS_EMPTY(s)  (strlen((s)) == 0)

#define WILDCARD     ('*')

/* This is unimplemented in the menu stuff, but would be nice. */

#define FORMFEED     Put_message(" ")

/* for use with insqueue and remque. */

struct qelem {
  struct qelem *q_forw;
  struct qelem *q_back;
  char *q_data;
};

#endif /* _defs_header_ */	/* Do Not Add Anything After This Line */
