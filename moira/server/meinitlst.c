/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/meinitlst.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/meinitlst.c,v 1.3 1990-04-04 18:28:50 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 * 	This code is a reverse-engineered version of
 * 	~rtingres/lib/compatlib(meinitlst.o) with a better malloc and
 * 	free.  It only works with Ingres version 5 on the vax.
 */

#ifndef lint
static char *rcsid_meinitlst_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/meinitlst.c,v 1.3 1990-04-04 18:28:50 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <moira_site.h>

#if INGRESVER == 5 && defined(vax)

/*
 * entry points:
 * 	MEinitLists()
 * 	calloc()
 * 	free()
 * 	malloc()
 * 	memalign()
 * 	realloc()
 * 	valloc()
 */

struct cons {
	char *car;
	char *cdr;
};
extern int Status;
extern char MEsetup;
extern struct cons MElist, MEfreelist;
extern short ME_pid;
extern char *MElistHead, *MEfreeHead;

MEinitLists()
{
	Status = 0;
	MEsetup = 1;
	MElist.car = 0;
	MElist.cdr = MElistHead;
	MEfreelist.car = 0;
	MEfreelist.cdr = MEfreeHead;
	ME_pid = getpid() & 0x7fff;
	return 0;
}
#ifdef notdef
free(addr)
{
	return MEfree(addr);
}

caddr_t malloc(size)
	u_int size;
{
	char *temp;
	if(MEalloc(1, size, &temp)) return 0;
	else return temp;
}

caddr_t calloc(a1, a2)
	int a1, a2;
{
	char *temp;
	if (MEcalloc(a1, a2, &temp)) return 0;
	else return temp;
}

caddr_t realloc(adr, nsize)
	char *adr;
	u_int nsize;
{
	char *new_addr;
	u_int cur_size;
	if (MEsize(adr, &cur_size)) return 0;
	if (cur_size >= nsize) return adr;
	if ( (new_addr = malloc(nsize)) == 0) return 0;
	(void) MEcopy(adr, cur_size, new_addr);
	(void) free(adr);
	return new_addr;
}

valloc(size)
	int size;
{
	static int pagesize;
	if (pagesize == 0) pagesize = getpagesize();
	return memalign(pagesize, size);
}

memalign(alignment, size)
{
	int temp1;
	int temp2;
	int temp3;
	int temp4;
	if (!size || alignment & 1) {
		errno = EINVAL;
		return 0;
	}
	if (alignment < 4) alignment = 4;

/*
_memalign:      fc0
_memalign+2:    subl2   $10,sp
_memalign+5:    tstl    8(ap)
_memalign+8:    beql    _memalign+e
_memalign+a:    blbc    4(ap),_memalign+18
>_memalign+e:    movl    $16,_errno
>_memalign+15:   clrf    r0
_memalign+17:   ret
>_memalign+18:   cmpl    4(ap),$4
_memalign+1c:   bgequ   _memalign+22
_memalign+1e:   movl    $4,4(ap)
>_memalign+22:   pushl   $4
_memalign+24:   addl3   $3,8(ap),-(sp)
_memalign+29:   calls   $2,udiv
_memalign+30:   ashl    $2,r0,8(ap)
_memalign+35:   addl3   4(ap),8(ap),r0
_memalign+3b:   addl3   $20,r0,-4(fp)
_memalign+40:   pushl   -4(fp)
_memalign+43:   calls   $1,_malloc
_memalign+4a:   movl    r0,r11
_memalign+4d:   beql    _memalign+15
_memalign+4f:   subl3   $10,r11,r10
_memalign+53:   ashl    $0,4(r10),r8
_memalign+58:   pushl   4(ap)
_memalign+5b:   subl3   $1,4(ap),r0
_memalign+60:   addl3   r11,r0,-(sp)
_memalign+64:   calls   $2,udiv
_memalign+6b:   mull3   4(ap),r0,-c(fp)
_memalign+71:   movl    -c(fp),r7
_memalign+75:   subl3   $10,r7,r6
_memalign+79:   subl3   r10,r6,-8(fp)
_memalign+7e:   subl2   -8(fp),r8
_memalign+82:   cmpl    -8(fp),$10
_memalign+86:   bgequ   _memalign+a7
_memalign+88:   pushl   8(ap)
_memalign+8b:   pushl   4(ap)
_memalign+8e:   calls   $2,_memalign
_memalign+95:   movl    r0,-10(fp)
_memalign+99:   pushl   r11
_memalign+9b:   calls   $1,_free
_memalign+a2:   movl    -10(fp),r0
_memalign+a6:   ret
>_memalign+a7:   movl    -8(fp),r0
_memalign+ab:   extzv   $0,$20,r0,4(r10)
_memalign+b1:   extzv   $0,$20,r8,4(r6)
_memalign+b7:   movl    (r10),(r6)
_memalign+ba:   movw    8(r10),8(r6)
_memalign+bf:   movl    r6,_MElist
_memalign+c6:   cmpl    _MElist+4,r10
_memalign+cd:   bneq    _memalign+d6
_memalign+cf:   movl    r6,_MElist+4
>_memalign+d6:   pushl   r10
_memalign+d8:   calls   $1,_MEf_add
_memalign+df:   addl3   $10,r6,r0
_memalign+e3:   ret
*/
#endif notdef
#endif /* INGRESVER == 5 && defined(vax) */
