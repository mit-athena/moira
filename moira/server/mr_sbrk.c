/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_sbrk.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_sbrk.c,v 1.2 1988-09-13 17:42:07 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 */

#ifndef lint
static char *rcsid_sms_sbrk_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_sbrk.c,v 1.2 1988-09-13 17:42:07 mar Exp $";
#endif lint

#include <mit-copyright.h>

/*
 * The following routine is a kludge around a deficiency in the Ingres
 * memory allocator; it likes to call sbrk(0) a lot to check on the
 * amount of memory allocated.
 */
#include <sys/types.h>

#ifdef vax
extern char end;
asm("	.data");
asm("  	.globl	curbrk");
asm("	.globl	minbrk");
asm("_curbrk:");
asm("curbrk:	.long	_end");
asm("_minbrk:");
asm("minbrk:	.long	_end");
    
extern caddr_t curbrk;
extern caddr_t minbrk;

caddr_t sbrk(incr)
	register int incr;
{
	register caddr_t oldbrk = curbrk;
	if (incr) {
		if (brk(oldbrk + incr) < 0)
			return (caddr_t)-1;
		curbrk += incr;
	}	
	return oldbrk;
}
#endif vax

/*
 * Local Variables:
 * mode: c
 * c-indent-level: 4
 * c-continued-statement-offset: 4
 * c-brace-offset: -4
 * c-argdecl-indent: 4
 * c-label-offset: -4
 * End:
 */
