/*
 * COPYRIGHT (C) 1990 DIGITAL EQUIPMENT CORPORATION
 * ALL RIGHTS RESERVED
 *
 * "Digital Equipment Corporation authorizes the reproduction,
 * distribution and modification of this software subject to the following
 * restrictions:
 * 
 * 1.  Any partial or whole copy of this software, or any modification
 * thereof, must include this copyright notice in its entirety.
 *
 * 2.  This software is supplied "as is" with no warranty of any kind,
 * expressed or implied, for any purpose, including any warranty of fitness 
 * or merchantibility.  DIGITAL assumes no responsibility for the use or
 * reliability of this software, nor promises to provide any form of 
 * support for it on any basis.
 *
 * 3.  Distribution of this software is authorized only if no profit or
 * remuneration of any kind is received in exchange for such distribution.
 * 
 * 4.  This software produces public key authentication certificates
 * bearing an expiration date established by DIGITAL and RSA Data
 * Security, Inc.  It may cease to generate certificates after the expiration
 * date.  Any modification of this software that changes or defeats
 * the expiration date or its effect is unauthorized.
 * 
 * 5.  Software that will renew or extend the expiration date of
 * authentication certificates produced by this software may be obtained
 * from RSA Data Security, Inc., 10 Twin Dolphin Drive, Redwood City, CA
 * 94065, (415)595-8782, or from DIGITAL"
 *
 */

#include <stdio.h>
#include <sys/types.h>

#include "BigNum.h"
#include "BigRSA.h"
#include "bigrsacode.h"
#include "bigkeygen.h"

/* 
#ifdef DEBUG
#undef DEBUG
#endif
*/

/* given two primes in msb-lsg order, initialize the key structure */

int KeyFromPQ ( P, PL, Q, QL, keys )
char *P, *Q;
int PL, QL;
RSAKeyStorage *keys ;
{
    BigNum  p = keys->p, q = keys->q ;
    int i;
    char *x, *y;
    unsigned pl, ql;

    memset(keys,0,sizeof(RSAKeyStorage));
    BnnInit();

    DecodeBigInteger(P, p, PL);
    DecodeBigInteger(Q, q, QL);
 
    pl = BnnNumDigits(p, (PL+sizeof(BigNumDigit))/sizeof(BigNumDigit));
    ql = BnnNumDigits(q, (QL+sizeof(BigNumDigit))/sizeof(BigNumDigit));

    if (BnnCompare(p, pl, q, ql) == -1) {
        unsigned tpl = pl;
        BigNumDigit tp[DigitLim];
        BnnSetToZero(tp,DigitLim);
        BnnAssign(tp,p,pl);
        BnnSetToZero(p,pl);
        BnnAssign(p,q,ql);
        BnnAssign(q,tp,pl);
        pl = ql;
        ql = tpl;
    }

    keys->pl = pl;
    keys->ql = ql;

    BnnSetDigit(keys->e, (BigNumDigit) ((1<<16)+1));
    keys->el = 1;

    BnnMultiply(keys->n, pl+ql, p, pl, q, ql),
    keys->nl = pl+ql;

#ifdef DEBUG
PrintTestKey(keys);
#endif

    if (FinishKey(keys)==0) return (0);

    if (TestRSAKeys(keys)!=1) {
#ifdef DEBUG
printf("\nFailed.\n");
#endif
        return(0);
    }  

    BnnClose();
    return(1);
}


