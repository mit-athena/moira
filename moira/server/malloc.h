/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/malloc.h,v $
 *	$Author: tytso $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/malloc.h,v 1.1 1993-12-10 13:51:19 tytso Exp $
 *
 *	Copyright (C) 1993 by the Massachusetts Institute of Technology
 *
 * This is a stub routine that defines the malloc, realloc, free, etc.
 * It also #includes string.h and strings.h, so that the debugging
 * malloc can redefine them if necessary.
 */

extern char *malloc();
extern char *realloc();
extern void free();

#include <string.h>
#include <strings.h>
