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

/* This code was derived from Mark Shand's PRL Montgomery code. */

#include <stdio.h>
#include <time.h>
#include <sys/types.h>

#include "BigZ.h"
#include "BigRSA.h"
#include "bigrsacode.h"
#include "bigkeygen.h"

#ifdef mips
#undef BnnSetToZero
#endif

#define KEY_LINE 64
#define KEY_RADIX 16

/* 
#define DEBUG_1
*/

/* Global storage indicating whether to print prime progress reports */

int bigkeygenPrintStatistics = 0;

#ifdef DEBUG
#undef DEBUG
#endif

/*
 * One-half the difference between the lengths of p and q, in bits
 */
#define PRIME_BIT_DIFF 3

/*
 * Local modular product routines
 */

/* put in-line RSA routines here */

#include "RSAcrypto.h"

static void BnnGCD_1();
static void modpwr2_1();
static BigNum muB_1();
static int bigprime();

/*
 * newRSAKey generates a new RSA key instance.
 *
 *      bitlen  length of primes, in bits.  Modulus will be
 *              approximately 2*bitlen
 *      keys    RSAKeyStorage structure for new key
 */
 
int newRSAKey ( bitlen , keys )
int bitlen ;
RSAKeyStorage *keys ;
{
    BigNum  p = keys->p, q = keys->q ;
    unsigned lowbits = 1, confidence = 20, pl, ql;
#ifdef DEBUG
    clock_t mark, tenths;
    struct timeb start_time , end_time ; 
#endif

    if ((bitlen<128)||(bitlen>MaxPrimeBits)) return (0);

    memset(keys,0,sizeof(RSAKeyStorage));
    BnnInit();

#ifdef DEBUG
    mark = clock();
    ftime (&start_time);
#endif

    bigprime(p, &pl, bitlen+PRIME_BIT_DIFF, lowbits, confidence, 0);
    do {
	bigprime(q, &ql, bitlen-PRIME_BIT_DIFF, lowbits, confidence, 0);
    } while (BnnCompare(p, pl, q, ql) == 0);

#ifdef DEBUG
    mark = clock() - mark ;
    ftime (&end_time);

#ifdef ultrix
#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC  1000000
#endif
    tenths = (100*(mark%CLOCKS_PER_SEC))/CLOCKS_PER_SEC ;
    mark /= CLOCKS_PER_SEC ;
#else
    tenths = (100*(mark%CLK_TCK))/CLK_TCK ;
    mark /= CLK_TCK ;
#endif

    printf ("\n/* Primes found in %d.%02d seconds of process time*/\n", mark, tenths );
    mark = end_time.time - start_time.time ;
    { long xt = (unsigned long) end_time.millitm - (unsigned long) start_time.millitm ;
      if (xt<0) { mark--; xt = 1000 - xt;}
      printf ("\n/* Primes found in %d.%03d elapsed seconds */\n",
		 end_time.time-start_time.time, xt );
    }
#endif
 
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

    /*
     * Set exponent to F16 and compute modulus.
     */
    BnnSetDigit(keys->e, (BigNumDigit) ((1<<16)+1));
    keys->el = 1;

    BnnMultiply(keys->n, pl+ql, p, pl, q, ql),
    keys->nl = pl+ql;

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


/* 
 * Prime-finding routines 
 */


static BigNumDigit smallprime[]=
{3,5,7,11,13,17,19,23,29,31,37,41,43,53,59,
61,67,71,73,79,83,89,97,101,103,107,109,113,127,131,137,139,149,151,157,
163,167,173,179,181,191,193,197,199,211,223,227,229,233,239,241,251,
257,263,269,271,277,281,283,293,307,311,313,317,331,337,347,349,353,
359,367,373,379,383,389,397,401,409,419,421,431,433,439,443,449,457,
461,463,467,479,487,491,499 };


/*
 * Test for primality.
 */

static int IsPrime(n,nl,confidence)
BigNum n;
int nl;
{
        int i;
        static BigNumDigit x[DigitLim], 
                           y[2*DigitLim], 
                           beta2n[2*DigitLim+1],
                           nINV [2*DigitLim] ;

        for(i=0;i<=(sizeof(smallprime)/sizeof(BigNumDigit));i++) 
          if(BnnDivideDigit(y,n,nl+1,smallprime[i])==0) goto exit ;

        muB_1(nINV, n, nl*BN_DIGIT_SIZE);
        modpwr2_1(beta2n, 2, n, nl);
        BnnSetToZero (x, nl);

        for(i=0;i<=confidence;i++) {
                x[0]+= smallprime[i] ;
                BnnDyRaise_1(y, x, n, nl, n, beta2n, nINV, nl);
                BnnDyCanon_1(y, n, nl);
                if (BnnCompare(y, nl, x, nl)) goto exit;
                }
        return(1);

exit:
        return(0);
}

/*
 * Generate a big prime.
 */
static bigprime (n,nlen,bitlen,lowbits,confidence,stats)
BigNum n;        
unsigned *nlen;
int bitlen;
BigNumDigit lowbits;
int confidence;
unsigned *stats;
{ 
  char * z ;
  static char sieve [1000];
  int i,j;
  BigNumDigit r, s, q[DigitLim] ;
  unsigned nl =(bitlen+(BN_DIGIT_SIZE-1))/BN_DIGIT_SIZE ;
  unsigned ntop = nl;

  if((((bitlen-1)%BN_DIGIT_SIZE)+1)>=(BN_DIGIT_SIZE-1)) nl++;
  *nlen = nl;
  BnnSetToZero(n,nl+1);
  while(1) {
    random_BigNum(n,ntop);
    n[nl-1]&=(1<<((BN_DIGIT_SIZE)-(nl*(BN_DIGIT_SIZE)-bitlen))) -1 ;
    n[(bitlen-1)/BN_DIGIT_SIZE] |= 1<<((bitlen-1)%BN_DIGIT_SIZE);
    n[(bitlen-2)/BN_DIGIT_SIZE] |= 1<<((bitlen-2)%BN_DIGIT_SIZE);
    n[0]&=~1;

    if(bigkeygenPrintStatistics) 
        printf("\nCandidate Random Number: %s\n",BzToString(BzFromBigNum(n,nl),16));

    z = &sieve[0] ;
    for (i=0;i<1000;i=i+2){*z++=1;*z++=0;};
    r = BnnDivideDigit(q,n,nl+1,(BigNumDigit)3);
    if (r==0) r = (BigNumDigit) 3;
    r--;
    z = &sieve[3-r] ;
    for (j=3-r;j<1000;j+=3) {*z=1;z+=3;}

    for (s=3;s<10000;s+=2) {
      for (j=0; j<5; j++)if ((s>smallprime[j])&&(0==(s%smallprime[j]))) goto nexts;
      r = BnnDivideDigit(q,n,nl+1,s);
      if (r==0) r = s;
      z = &sieve[s-r] ;
      for (j=s-r;j<1000;j+=s) {*z=1;z+=s;}
    nexts: ;
    }
    for(i=0;i<1000;i++) {
      if (!sieve[i]) {

    if(bigkeygenPrintStatistics) printf("|"),fflush(stdout);

         if (IsPrime(n,nl,5)) goto exit;
      }
      BnnAddCarry(n,nl,1);  /* try next one */
    } 
  }
exit:

if(bigkeygenPrintStatistics) printf(""), fflush(stdout);

}


/*
 * This routine expects modulus n and exponent e to be initialized
 * in the RSAKeyStorage structure supplied.  It computes the rest of
 * the key components based on what is provided.  If p and q are there
 * it also makes sure a complete private key is initialized.
 */
int FinishKey (keys)
RSAKeyStorage *keys;
{
    static BigNumDigit g[4*DigitLim], wgcd[12*DigitLim], 
	r[4*DigitLim], pq[4*DigitLim], d[4*DigitLim], z[4*DigitLim];

    BigNum n = keys->n, dp = keys->dp, dq = keys->dq, cr = keys->cr,
	   e = keys->e, p = keys->p, q = keys->q ;

    unsigned nl=keys->nl, pl=keys->pl, ql=keys->ql, tmp,
             dpl = keys->dpl, dql = keys->dql ;

    /*
     * If primes are supplied, generate dp, dq, and cr if needed.
     * Assume cr there if dp and dq are.
     */

    if (pl && ql && ((dpl == 0)||(dql ==0)) ) {
        BnnAssign(pq, n, nl);
        BnnSubtract(pq, nl, p, pl, 1);
        BnnSubtract(pq, nl, q, ql, 1);
        BnnAddCarry(pq, nl, 1);
        BnnGCD_1(g, d, r, e, pq, wgcd, nl);
        if (BnnCompare(g,nl,one,nl)!=0) {
#ifdef DEBUG
printf("GCD failed to generate private key exponent.\n");
#endif
        	return(0);
        }
        BnnAssign(z, d, nl);
        z[nl] = 0;
        BnnAssign(pq, p, pl);
        BnnSubtractBorrow(pq, pl, 0);
        BnnDivide(z, nl+1, pq, tmp=BnnNumDigits(pq, pl));
        BnnSetToZero(z+tmp, nl+1-tmp);
        BnnAssign(dp, z, (keys->dpl)=BnnNumDigits(z,nl));

        BnnAssign(z, d, nl);
        z[nl] = 0;
        BnnAssign(pq, q, ql);
        BnnSubtractBorrow(pq, ql, 0);
        BnnDivide(z, nl+1, pq, tmp=BnnNumDigits(pq, ql));
        BnnSetToZero(z+tmp, nl+1-tmp);
        BnnAssign(dq, z, (keys->dql)=BnnNumDigits(z,nl));
    
        BnnGCD_1(g,r,cr,p,q,wgcd,pl);
        if ((BnnGetDigit(cr+(pl-1))&TopBitInWord)!=0) BnnAdd(cr,pl,p,pl,0);
    }

    modpwr2_1(z, 2, n, nl);
    BnnAssign(keys->beta2n, z, nl);
    muB_1(keys->nINV, n, nl*BN_DIGIT_SIZE);

    if (pl&&ql)  {
        modpwr2_1(z, 3, p, pl);
        BnnAssign(keys->beta3p, z, pl);
        modpwr2_1(z, 2, p, pl);
        BnnAssign(keys->beta2p, z, pl);
        modpwr2_1(z, 3, q, ql);
        BnnAssign(keys->beta3q, z, ql);
        muB_1(keys->pINV, p, pl*BN_DIGIT_SIZE);
        muB_1(keys->qINV, q, ql*BN_DIGIT_SIZE);
    }

#ifdef DEBUG_1
    printf("\n");
    printf("p = %s\n", BzToString(BzFromBigNum(p, pl), 16));
    printf("q = %s\n", BzToString(BzFromBigNum(q, ql), 16));
    printf("n = %s\n", BzToString(BzFromBigNum(n, nl), 16));
    printf("e = %s\n", BzToString(BzFromBigNum(e, nl), 16));
    printf("d = %s\n", BzToString(BzFromBigNum(d, nl), 16));
    printf("dp = %s\n", BzToString(BzFromBigNum(dp, dpl), 16));
    printf("dq = %s\n", BzToString(BzFromBigNum(dq, dql), 16));
    printf("cr = %s\n", BzToString(BzFromBigNum(cr, pl), 16));
    fflush(stdout);
#endif


    return(1);
}


static BigNum muB_1(res, zz, bits)
BigNum res, zz;
int bits;
{
    /* Compute -z^-1 mod 2^bits */
#define muBLIM 40
    BigNumDigit n[muBLIM];
    BigNumDigit msk[muBLIM], z[muBLIM];
    unsigned nl = (bits+BN_DIGIT_SIZE-1)/BN_DIGIT_SIZE;
    int i;
    
    if (nl > muBLIM)
    {
        fprintf(stderr, "Limit check in %s:muB_1 line %d\n", __FILE__, __LINE__);
        abort();
    }
    
    BnnSetToZero(n, nl);
    BnnComplement(n,nl);
    BnnSetToZero(res, nl);
    BnnSetToZero(msk, nl);
    BnnSetDigit(msk, 1);
    BnnAssign(z, zz, nl);
    BnnShiftRight(z,nl,1);
    while (bits--)
    {
        if (BnnIsDigitOdd(*n))
        {
            for (i = 0; i < nl; i++)
                BnnOrDigits(res+i, msk[i]);
            BnnShiftRight(n,nl,1);
            BnnSubtract(n, nl, z, nl, 1);
        }
        else
           BnnShiftRight(n,nl,1);
        BnnShiftLeft(msk,nl,1);
    }
    return res;
}

static void modpwr2_1(result, pwr, m, ml)
BigNum result, m;
unsigned pwr, ml;
{	unsigned tmp;
	BnnSetToZero(result, pwr*ml);
	BnnSetDigit(result+pwr*ml, (BigNumDigit) 1);
	BnnDivide(result, pwr*ml+1, m, tmp=BnnNumDigits(m, ml));
	BnnSetToZero(result+tmp, pwr*ml+1-tmp);
}


static void BnnGCD_1(gcd, A, B, U, V, work, len)
BigNum gcd, A, B, U, V, work;
int len;
{
    /* Extended binary GCD
       code based on Knuth Vol.2 Second Edition
       section 4.5.2 Answers to Exercises page 599
    */
    /* Note: len must be large enough to hold 2*U or 2*V */
    /* "work" is a BigNum of length 3*len */
    BigNumDigit k;
    BigNum u1,u2,u3, v1,v2,v3;
    BigNum t1,t2,t3;
    u1 = A; u2 = B; u3 = gcd;
    v1 = work; v2 = work + len; v3 = work + 2*len;

    k = 0;
    while (!BnnIsDigitOdd(*U) && !BnnIsDigitOdd(*V))
    {
        BnnShiftRight(U, len, 1);
        BnnShiftRight(V, len, 1);
        k++;
    }
    BnnSetToZero(u1, len); BnnSetDigit(u1, (BigNumDigit) 1);
    BnnSetToZero(u2, len);
    BnnAssign(u3, U, len);
    if (BnnIsDigitOdd(*U))
    {
        t1 = v1; t2 = v2; t3 = v3;
        BnnSetToZero(v1, len);
        BnnSetToZero(v2, len); BnnComplement(v2, len);
        BnnSetToZero(v3, len); BnnSubtract(v3, len, V, len, 1);
    }
    else
    {
        t1 = u1; t2 = u2; t3 = u3;
        BnnAssign(v1, V, len);
        BnnSetToZero(v2, len); BnnSetDigit(v2, (BigNumDigit) 1);
            BnnSubtract(v2, len, U, len, 1);
        BnnAssign(v3, V, len);
    }
#ifdef PRINT_LEVEL2
    printf("#[%d %d %d ] [ %d %d %d ]\n", *u1,*u2,*u3, *v1,*v2,*v3);
#endif
    while (!BnnIsZero(t3, len))
    {
        while (!BnnIsDigitOdd(*t3))
        {
            BigNumDigit sign;
            if (!BnnIsDigitOdd(*t1) && !BnnIsDigitOdd(*t2))
            {
                sign = BnnGetDigit(t1+(len-1)) & TopBitInWord;
                BnnShiftRight(t1, len, 1);
                BnnOrDigits(t1+(len-1), sign);
                sign = BnnGetDigit(t2+(len-1)) & TopBitInWord;
                BnnShiftRight(t2, len, 1);
                BnnOrDigits(t2+(len-1), sign);
            }
            else
            {
                BnnAdd(t1, len, V, len, 0);
                sign = BnnGetDigit(t1+(len-1)) & TopBitInWord;
                BnnShiftRight(t1, len, 1);
                BnnOrDigits(t1+(len-1), sign);
                BnnSubtract(t2, len, U, len, 1);
                sign = BnnGetDigit(t2+(len-1)) & TopBitInWord;
                BnnShiftRight(t2, len, 1);
                BnnOrDigits(t2+(len-1), sign);
            }
            sign = BnnGetDigit(t3+(len-1)) & TopBitInWord;
            BnnShiftRight(t3, len, 1);
            BnnOrDigits(t3+(len-1), sign);
        }
        if (t1 == v1) /* a cheap way to recall what state we are in */ 
        {
            BnnComplement(v1, len);
            BnnAdd(v1, len, V, len, 1);
            BnnComplement(v2, len);
            BnnAddCarry(v2, len, 1);
            BnnSubtract(v2, len, U, len, 1);
            BnnComplement(v3, len);
            BnnAddCarry(v3, len, 1);
        }
        if (BnnCompare(u3, len, v3, len) > 0)
        {
            BnnSubtract(u1, len, v1, len, 1);
            BnnSubtract(u2, len, v2, len, 1);
            BnnSubtract(u3, len, v3, len, 1);
            t1 = u1; t2 = u2; t3 = u3;
        }
        else
        {
            BnnComplement(v1, len);
            BnnAdd(v1, len, u1, len, 1);
            BnnComplement(v2, len);
            BnnAdd(v2, len, u2, len, 1);
            BnnComplement(v3, len);
            BnnAdd(v3, len, u3, len, 1);
            t1 = v1; t2 = v2; t3 = v3;
        }

        if (BnnGetDigit(t1+(len-1)) & TopBitInWord)
        {
            BnnAdd(t1, len, V, len, 0);
            BnnSubtract(t2, len, U, len, 1);
        }
#ifdef PRINT_LEVEL2
    printf(">[%d %d %d ] [ %d %d %d ]\n", *u1,*u2,*u3, *v1,*v2,*v3);
#endif
    }
    while (k-- > 0)
        BnnShiftLeft(u3, len, 1);
}


/**********************************************************************/

static char *TestData = "Now is the time for all good men to come to the aid of their" ;
static BigNumDigit test_in [2*DigitLim], test1 [2*DigitLim], test2 [2*DigitLim] ;

int TestRSAKeys(key)
RSAKeyStorage *key ; 
{
        int nl=key->nl;
        int ll = BnnNumDigits(key->n,nl);

        memset(test_in,0,sizeof(test_in));
        memset(test1,0,sizeof(test1));
        memset(test2,0,sizeof(test2));
        strcpy((char *)test_in,TestData);
        BnnSetToZero(test_in+ll-2,(nl-ll)+3);
#ifdef DEBUG
printf("\nPrivate Key Encrypt/Decrypt Test.");
printf("\nInput(hex): %s", BzToString(BzFromBigNum(test_in, nl), 16));
printf("\nInput(string): %s", test_in);
#endif
        rsaencrypt_1(test_in,test1,key);
#ifdef DEBUG
printf("\nEncrypted: %s", BzToString(BzFromBigNum(test1, nl), 16));
#endif
        rsadecrypt_1(test1,test2,key);
#ifdef DEBUG
printf("\nDecrypted: %s", BzToString(BzFromBigNum(test2, nl), 16));
printf("\n");
#endif
        if (BnnCompare(test_in,nl,test2,nl) != 0) { 
#ifdef DEBUG
printf("\nKey Test Failed.\n");
#endif
                return(0);
        }
#ifdef DEBUG
printf("\nKey Test Passed.\n");
#endif
        return(1);
}  

static int Test2RSAPrivate_1(key)
RSAKeyStorage *key ; 
{
        int nl=key->nl;
        int ll = BnnNumDigits(key->n,nl);

        memset(test_in,0,sizeof(test_in));
        memset(test1,0,sizeof(test1));
        memset(test2,0,sizeof(test2));
        strcpy((char *)test_in,TestData);
        BnnSetToZero(test_in+ll-2,(nl-ll)+3);
#ifdef DEBUG
        printf("\nPrivate Key Decrypt/Encrypt Test.");
        printf("\nInput(hex): %s", BzToString(BzFromBigNum(test_in, nl), 16));
        printf("\nInput(string): %s", test_in);
#endif
        rsadecrypt_1(test_in,test1,key);
#ifdef DEBUG
printf("\nSigned: %s", BzToString(BzFromBigNum(test1, nl), 16));
#endif
        rsaencrypt_1(test1,test2,key);
#ifdef DEBUG
printf("\nVerified: %s", BzToString(BzFromBigNum(test2, nl), 16));
printf("\n");
#endif
        if (BnnCompare(test_in,nl,test2,nl) != 0) { 
#ifdef DEBUG
printf("\nKey Test Failed.\n");
#endif
                return(0);
        }
        return(1);
}  


PrintBigNum(n,nl,radix)
BigNum n;
int radix, nl;
{  
     static char oneline [KEY_LINE+1];
     char *p = BzToString(BzFromBigNum(n,nl), radix);

     oneline[KEY_LINE]='\0';
     while(*p != '\0'){
        strncpy(oneline,p,KEY_LINE); 
        printf("\n     %s",oneline);
        p+=((strlen(p)>KEY_LINE)?KEY_LINE:strlen(p));
     }
}

int PrintTestKey (Keys)
RSAKeyStorage *Keys;
{
    int i;
  
    printf("\ne: (%d BigNumDigit", Keys->el);
    if(Keys->el > 1) printf("s) "); else printf(")");
    PrintBigNum(Keys->e,Keys->el,KEY_RADIX);
    printf("\nn: (%d BigNumDigits) ", Keys->nl);
    PrintBigNum(Keys->n,Keys->nl,KEY_RADIX);

    if (Keys->pl) { printf("\np: (%d BigNumDigits) ", Keys->pl);
              PrintBigNum(Keys->p,Keys->pl,KEY_RADIX);
              printf("\nq: (%d BigNumDigits) ", Keys->ql);
              PrintBigNum(Keys->q,Keys->ql,KEY_RADIX);
              printf("\ndp: (%d BigNumDigits) ", Keys->dpl);
              PrintBigNum(Keys->dp, Keys->dpl,KEY_RADIX);
              printf("\ndq: (%d BigNumDigits) ", Keys->dql);
              PrintBigNum(Keys->dq, Keys->dql,KEY_RADIX);
              printf("\ncr: ");
              PrintBigNum(Keys->cr, Keys->pl,KEY_RADIX);
    }
    
    i=BnnNumDigits(Keys->nINV,sizeof(*Keys->nINV)*sizeof(int));
    printf("\nnINV: (%d BigNumDigits) ", i);
    PrintBigNum(Keys->nINV, i, KEY_RADIX);

    i=BnnNumDigits(Keys->beta2n,sizeof(*Keys->beta2n)*sizeof(int));
    printf("\nbeta2n: (%d BigNumDigits) ", i);
    PrintBigNum(Keys->beta2n, i,KEY_RADIX);

    if(Keys->pl) {
        i=BnnNumDigits(Keys->pINV,sizeof(*Keys->pINV)*sizeof(int));
        printf("\npINV: (%d BigNumDigits) ", i);
        PrintBigNum(Keys->pINV, i,KEY_RADIX);

        i=BnnNumDigits(Keys->qINV,sizeof(*Keys->qINV)*sizeof(int));
        printf("\nqINV: (%d BigNumDigits) ", i);
        PrintBigNum(Keys->qINV, i,KEY_RADIX);


        i=BnnNumDigits(Keys->beta2p,sizeof(*Keys->beta2p)*sizeof(int));
        printf("\nbeta2p: (%d BigNumDigits) ", i);
        PrintBigNum(Keys->beta2p, i,KEY_RADIX);

        i=BnnNumDigits(Keys->beta3p,sizeof(*Keys->beta3p)*sizeof(int));
        printf("\nbeta3p: (%d BigNumDigits) ", i);
        PrintBigNum(Keys->beta3p, i,KEY_RADIX);

        i=BnnNumDigits(Keys->beta3q,sizeof(*Keys->beta3q)*sizeof(int));
        printf("\nbeta3q: (%d BigNumDigits) ", i);
        PrintBigNum(Keys->beta3q, i,KEY_RADIX);
    }
    
    printf("\n");
    fflush(stdout);
}


int PrintPubKey (Keys)
RSAKeyStorage *Keys;
{
    int i=BnnNumDigits(Keys->n,Keys->nl);
    i = i*BN_DIGIT_SIZE - BnnNumLeadingZeroBitsInDigit(Keys->n[i-1]);
  
    printf("\ne: (%d BigNumDigit", Keys->el);
    if(Keys->el > 1) printf("s) "); else printf(")");
    PrintBigNum(Keys->e,Keys->el,KEY_RADIX);
    printf("\nn: (%d BigNumDigits, %d bits) ", Keys->nl, i);
    PrintBigNum(Keys->n,Keys->nl,KEY_RADIX);

    printf("\n");
    fflush(stdout);
}

int PrintPrivKey (Keys)
RSAKeyStorage *Keys;
{
    int i=BnnNumDigits(Keys->n,Keys->nl);
    i = i*BN_DIGIT_SIZE - BnnNumLeadingZeroBitsInDigit(Keys->n[i-1]);
  
    printf("\ne: (%d BigNumDigit", Keys->el);
    if(Keys->el > 1) printf("s) "); else printf(")");
    PrintBigNum(Keys->e,Keys->el,KEY_RADIX);
    printf("\nn: (%d BigNumDigits, %d bits) ", Keys->nl, i);
    PrintBigNum(Keys->n,Keys->nl,KEY_RADIX);

    if (Keys->pl) { printf("\np: (%d BigNumDigits) ", Keys->pl);
              PrintBigNum(Keys->p,Keys->pl,KEY_RADIX);
              printf("\nq: (%d BigNumDigits) ", Keys->ql);
              PrintBigNum(Keys->q,Keys->ql,KEY_RADIX);
              printf("\ndp: (%d BigNumDigits) ", Keys->dpl);
              PrintBigNum(Keys->dp, Keys->dpl,KEY_RADIX);
              printf("\ndq: (%d BigNumDigits) ", Keys->dql);
              PrintBigNum(Keys->dq, Keys->dql,KEY_RADIX);
              printf("\ncr: ");
              PrintBigNum(Keys->cr, Keys->pl,KEY_RADIX);
    }
    else printf("\n no private key parameters.");

    printf("\n");
    fflush(stdout);
    }
    

/*********************************************************************/

/* In-line DES routines here. */

#include "DEScrypto.h"

static KEYschedule local_key_schedule;
static RSAKeyStorage tempkey;

/*
 * Decrypts an RSA private key.
 */
int recover_private(deskey, encrypted_key, encrypted_key_len, key)
DESblock *deskey;               /* derived from password */
char *encrypted_key;            /* input to be decrypted */
unsigned encrypted_key_len;     /* must be multiple of 8 bytes */
RSAKeyStorage *key;             /* output with new key in it */
{
   unsigned char *outbuf;
   int rt=0;

   if((outbuf=(unsigned char *)calloc(encrypted_key_len,1))==0)return(0);

   DES_load_key_local( deskey, &local_key_schedule);
   if (DES_CBC_decrypt_local(0, encrypted_key, encrypted_key_len, 
                                outbuf, &local_key_schedule)==0) 
                                                        goto clean;
   memset(&tempkey,0,sizeof(tempkey));
   if (key->nl) tempkey.nl=key->nl, BnnAssign(tempkey.n,key->n,key->nl);
#ifdef DEBUG
printf("\nDecrypted private key:\n");
dumphex(outbuf,encrypted_key_len-sizeof(long));
#endif
   if (DecodePrivate(outbuf, &tempkey) != NULL) {
#ifdef DEBUG
printf("\nRecovered Key:\n");
PrintTestKey(&tempkey);
#endif
        if (rt=TestRSAKeys(&tempkey)) memcpy(key,&tempkey,sizeof(RSAKeyStorage));
   }
#ifdef DEBUG
   else printf("\nDecode Failed.\n");
#endif

   memset(outbuf,0,encrypted_key_len);
   memset(&tempkey,0,sizeof(tempkey));
clean:
   free(outbuf);
   return(rt);
}

/*
 * Decrypts an RSA private key.
 */
int recover_privateP(deskey, encrypted_key, encrypted_key_len, key)
DESblock *deskey;               /* derived from password */
char *encrypted_key;            /* input to be decrypted */
unsigned encrypted_key_len;     /* must be multiple of 8 bytes */
RSAKeyStorage *key;             /* output with new key in it */
{
   unsigned char *outbuf;
   int rt=0;

   if((outbuf=(unsigned char *)calloc(encrypted_key_len,1))==0)return(0);

   DES_load_key_local( deskey, &local_key_schedule);
   if (DES_CBC_decrypt_local(0, encrypted_key, encrypted_key_len, 
                                outbuf, &local_key_schedule)==0) 
                                                        goto clean;
   memset(&tempkey,0,sizeof(tempkey));
   if (key->nl) tempkey.nl=key->nl, BnnAssign(tempkey.n,key->n,key->nl);
#ifdef DEBUG
printf("\nDecrypted private key:\n");
dumphex(outbuf,encrypted_key_len-sizeof(long));
#endif
   /*  make sure that we decode the private key with only the prime p  */
   if (DecodePrivateP(outbuf, &tempkey) != NULL) {
#ifdef DEBUG
printf("\nRecovered Key:\n");
PrintTestKey(&tempkey);
#endif
        if (rt=TestRSAKeys(&tempkey)) memcpy(key,&tempkey,sizeof(RSAKeyStorage));
   }
#ifdef DEBUG
   else printf("\nDecode Failed.\n");
#endif

   memset(outbuf,0,encrypted_key_len);
   memset(&tempkey,0,sizeof(tempkey));
clean:
   free(outbuf);
   return(rt);
}

/*
 * Encrypts an RSA private key.
 *
 * NOTE: Contains components to foil use as a general encrypt/decrypt
 *       facility.
 *
 */
int hide_private(deskey, encrypted_key, encrypted_key_len, key)
DESblock *deskey;               /* derived from password */
char **encrypted_key;           /* storage will be allocated */
unsigned *encrypted_key_len;    /* will be written */
RSAKeyStorage *key;             /* input must be a valid private key */
{
   unsigned char *outbuf, *encoded_key;
   int len, klen;

   if (TestRSAKeys(key)!=1) return(0); 
   if ((encoded_key=EncodePrivate(key))==0) return(0) ;
   len = (((klen=DecodeTotalLength(encoded_key))+7)/8)*8; /* pad */
   if ((outbuf = (unsigned char *)calloc(len,1)) ==0) 
                        {FreePrivate(encoded_key); return(0);};
   memcpy(outbuf,encoded_key,klen);
   FreePrivate(encoded_key);
   DES_load_key_local(deskey, &local_key_schedule);
   DES_CBC_encrypt_local (0, outbuf, len, outbuf, &local_key_schedule);
   *encrypted_key = (char *)outbuf;
   *encrypted_key_len = len;
   return(1);
}


/*
 * Same as above, but only encrypts prime P
 */
int hide_privateP(deskey, encrypted_key, encrypted_key_len, key)
DESblock *deskey;               /* derived from password */
char **encrypted_key;           /* storage will be allocated */
unsigned *encrypted_key_len;    /* will be written */
RSAKeyStorage *key;             /* input must be a valid private key */
{
   unsigned char *outbuf, *encoded_key;
   int len, klen;

   if (TestRSAKeys(key)!=1) return(0); 
   if ((encoded_key=EncodePrivateP(key))==0) return(0) ;
   len = (((klen=DecodeTotalLength(encoded_key))+7)/8)*8;
   if ((outbuf = (unsigned char *)calloc(len,1)) ==0) 
                        {FreePrivate(encoded_key); return(0);};
   memcpy(outbuf,encoded_key,klen);
   FreePrivate(encoded_key);
   DES_load_key_local(deskey, &local_key_schedule);
   DES_CBC_encrypt_local (0, outbuf, len, outbuf, &local_key_schedule);
   *encrypted_key = (char *)outbuf;
   *encrypted_key_len = len;
   return(1);
}
