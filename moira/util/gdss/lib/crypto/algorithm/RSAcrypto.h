/* RSAcrypto.h */
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

/* This code was derived in large part from Mark Shand's PRL Montgomery code. */

/* In-line code for basic RSA routines */

static void rsaencrypt_1();
static void rsadecrypt_1();
static void BnnModularProduct_1();
static void BnnDyRaise_1 ();
static unsigned BnnDyCanon_1 ();
 
/*
 *********************************************************************
 *********************************************************************
 **                                                                 **
 **                       W A R N I N G                             **
 **                                                                 **
 **  This software is subject to export restrictions under the      **
 **  U.S. Department of State's International Traffic in Arms       **
 **  Regulations (ITAR).  This software must not be transmitted     **
 **  IN SOURCE FORM outside the United States or to a foreign       **
 **  national in the United States without a valid U.S. State       **
 **  Department export license.                                     **
 **                                                                 **
 *********************************************************************
 *********************************************************************
*/

#ifndef SPHINX_RSACRYPTO
#define SPHINX_RSACRYPTO

#ifndef SPHINX_ITAR
#define SPHINX_ITAR
static char copyright[] = "\n Copyright, 1989, 1990, Digital Equipment Corporation ";
static char warning[]= "\n  This software is subject to export restrictions under \
\n  the U.S. Department of State's International Traffic in Arms \
\n  Regulations (ITAR).  This software must not be transmitted \
\n  in source form outside the United States or to a foreign \
\n  national in the United States without a valid U.S. State \
\n  Department export license. ";
#endif

#define assert(x) ((x)?0:dumpcore("x",__FILE__,__LINE__))
#define TopBitInWord ((BigNumDigit)(1<<(BN_DIGIT_SIZE-1)))

static BigNumDigit one[2*2*DigitLim] = { 1 };

static int dumpcore(s,f,n)
char *s,*f;
int n;
{
    fflush(stdout);
    fclose(stdout);
    fprintf(stderr, "\nassertion failed file %s line %d: %s\n",  f, n, s);
    fclose(stderr);
    abort();
} 

static void rsaencrypt_1(inblk,outblk,keys)
BigNum  inblk, outblk;
RSAKeyStorage * keys;
{
    static BigNumDigit x[1] ;
    char *charp;
    register int shift = 0;
    BigNum n = keys->n, e = keys->e;
    int nl = keys->nl, el = keys->el;
    int nTop = BnnNumDigits(n,nl) ;
    int BLOCKLEN = nTop*BN_DIGIT_SIZE -	BnnNumLeadingZeroBitsInDigit(n[nTop-1]) - 1;
    int BLOCKBYTES = BLOCKLEN/8;
    int BLOCKWDS = BLOCKLEN/BN_DIGIT_SIZE + 1;
    BnnInit();
    x[0] = 0;
    charp = (char *) x;
    *charp = 1;
    if (x[0] != (BigNumDigit) 1 && BLOCKBYTES % sizeof(BigNumDigit) != 0) 
    {
        /* This machine is big-endian */
        shift = ((sizeof(BigNumDigit) * BLOCKWDS) - BLOCKBYTES) * 8;
    }
    *charp = 0;
    
    inblk[nl-1] >>= shift;
    BnnDyRaise_1 (outblk, inblk, e, el, n, keys->beta2n, keys->nINV, nl);
    BnnClose();
}

static void rsadecrypt_1(inblk, outblk,keys)
BigNum inblk, outblk;
RSAKeyStorage * keys ;
{
    static BigNumDigit x[3*DigitLim], xp[2*DigitLim], xq[2*DigitLim] ;
    char *charp;
    register int shift = 0;
    BigNum n = keys->n, p = keys->p, q = keys->q , 
	   pINV = keys->pINV, qINV = keys->qINV;
    int nl = keys->nl, pl = keys->pl, ql = keys->ql ;
    int nTop = BnnNumDigits(n,nl) ;
    int BLOCKLEN = nTop*BN_DIGIT_SIZE -	BnnNumLeadingZeroBitsInDigit(n[nTop-1]) - 1;
    int BLOCKBYTES = BLOCKLEN/8;
    int BLOCKWDS = BLOCKLEN/BN_DIGIT_SIZE + 1;

    BnnInit();
    x[nl]=0;
    charp = (char *) (x+nl);
    *charp = 1;
    if (x[nl] != (BigNumDigit) 1)
    {
        /* This machine is big-endian */
        shift = ((sizeof(BigNumDigit) * BLOCKWDS) - BLOCKBYTES) * 8;
    }
    {
	/* xp <- inblk/B^PL [p] */
	BnnSetToZero(xp, 2*pl);
	BnnAssign(xp+pl, inblk+pl, nl-pl);
	BnnModularProduct_1 (xp, pl+pl, inblk, pl, one, pl, p, pl, pINV);
	/* xq <- inblk/B^QL [q] */
	BnnSetToZero(xq, 2*ql);
	BnnAssign(xq+ql, inblk+ql, nl-ql);
	BnnModularProduct_1 (xq, ql+ql, inblk, ql, one, ql, q, ql, qINV);
        /* pass beta3[pq] coeff. to compensate for prediv by B^[PQ]L */
        BnnDyRaise_1 (xp, xp+pl, keys->dp, keys->dpl, p, keys->beta3p, pINV, pl);
        BnnDyRaise_1 (xq, xq+ql, keys->dq, keys->dql, q, keys->beta3q, qINV, ql);
	BnnSetToZero(xp+pl, nl-pl);
	BnnSetToZero(xq+ql, nl-ql);
	BnnSubtract(xp,pl,xq,ql,1);
	while((BnnGetDigit(xp+(pl-1))&TopBitInWord)!=0) BnnAdd(xp,pl,p,pl,0);
	BnnSetToZero(x,3*pl);
	BnnModularProduct_1 (x+pl,pl+pl,xp,pl,keys->cr,pl,p,pl,pINV);
	BnnModularProduct_1 (x,pl+pl,keys->beta2p,pl,x+pl+pl,pl,p,pl,pINV);
	BnnDyCanon_1 (x+pl, p, pl);
	BnnMultiply(xq, nl, x+pl, pl, q , ql);
	xq[2*pl-1] <<= shift;
    }
    BnnClose();
    BnnAssign(outblk, xq, nl);
}


static void BnnModularProduct_1 (p,pl,a,al,b,bl,m,ml,mu)
BigNum p,b,a,m,mu;
unsigned pl,al,bl,ml;
{
    /* p is length pl */
    /* a has length al, b has length bl, m has length ml, mu has length >= 2 */
    /* mu is -m[0..1..] mod BN_DIGIT_SIZE*2 */
    /* after computation
        p[0..bl-1] = 0
        (p[bl..pl-1] * beta^bl) mod m = p+(a*b) mod m
    */
    BigNumDigit qt[2*DigitLim];
    assert(pl <= 2*DigitLim);
    
    BnnMultiply(p, pl, a, al, b, bl);
    BnnSetToZero(qt, pl);
    BnnMultiply(qt, pl, p, pl-ml, mu, ml);
    BnnMultiply(p, pl, m, ml, qt, pl-ml);
}

static unsigned BnnDyCanon_1 (x, m, ml)
BigNum x, m;
unsigned ml;
{
    int redcount = 0;
    while (BnnCompare(x, ml, m, ml) > 0)
    {
        BnnSubtract(x, ml, m, ml, 1);
        redcount++;
    }
    return redcount;
}

#define MAX_LN2PWR 4
#define MAXPWR (1<<MAX_LN2PWR)

static void BnnDyRaise_1 (x, a, e, el, m, beta2ml, mu, ml)
BigNum x, a, e, m, beta2ml;
BigNum mu;
unsigned ml, el;
{
    static BigNumDigit sBUF[DigitLim];
    static BigNumDigit tBUF[DigitLim*10];
    register BigNum s = sBUF, t = tBUF;
    static BigNum aCache[MAXPWR];
    BigNumDigit ed;
    BigNumDigit mask;
    int eindex, eoffset, nibble, i;
    static BigNumDigit aCacheBUF[MAXPWR*2*DigitLim];
    int pwr,ln2pwr;

    /* Exponentiate: x = a^e mod m */
    if (BnnNumDigits(e, el) > 1)
	ln2pwr = MAX_LN2PWR;
    else
	ln2pwr = 1;
    pwr = (1<<ln2pwr);
    /* We work high to low in the exponent and make a special case 
        of the topmost bit in e */ 
    /* check that m is within range */
    assert((m[ml-1] & (3<<(BN_DIGIT_SIZE-2))) == 0);

    /* prime the aCache */
    t = aCacheBUF+2*DigitLim-ml;
    aCache[1] = s = aCacheBUF+2*DigitLim;
    BnnSetToZero(t, 2*ml);
    BnnModularProduct_1 (t, ml+ml, a, ml, beta2ml, ml, m, ml, mu);
    for (i = 2; i < pwr; i++)
    {
        t += 2*DigitLim;
        BnnSetToZero(t, 2*ml);
        BnnModularProduct_1 (t, ml+ml, s, ml, aCache[1], ml, m, ml, mu);
        aCache[i] = (s += 2*DigitLim);
    }

    eindex = BnnNumDigits(e, el)-1;
    ed = e[eindex];
    eoffset = ((BN_DIGIT_SIZE-BnnNumLeadingZeroBitsInDigit(ed)-1) & ~(ln2pwr-1));
    mask = (pwr-1) << eoffset;
    nibble = (ed&mask)>>eoffset;
    mask >>= ln2pwr;
    eoffset -= ln2pwr;
    s = aCache[nibble];

    while (eindex >= 0)
    {
        ed = e[eindex--];
        while (mask)
        {
            /* Square ln2pwr times */
            t = tBUF;
            BnnSetToZero(t, 2*(ln2pwr+1)*ml);
            for (i = 0; i < ln2pwr; i++)
            {
                BnnModularProduct_1 (t, ml+ml, s, ml, s, ml, m, ml, mu);
                s = t+ml; t += 2*ml;
            }
            nibble = (ed&mask)>>eoffset;
            if (nibble)
            {
                /* Multiply */
                BnnModularProduct_1 (t, ml+ml, s, ml, aCache[nibble], ml, m, ml, mu);
                BnnAssign(sBUF, t+ml, ml);
            }
            else
                BnnAssign(sBUF, s, ml);
            mask >>= ln2pwr;
            eoffset -= ln2pwr;
            s = sBUF;
        }
        eoffset = BN_DIGIT_SIZE-ln2pwr;
        mask = (pwr-1) << eoffset;
    }
    /* restore representation */
    
    t = tBUF;
    BnnSetToZero(t, 2*ml);
    BnnModularProduct_1 (t, ml+ml, s, ml, one, ml, m, ml, mu);
    BnnAssign(x, t+ml, ml);
}

#endif
