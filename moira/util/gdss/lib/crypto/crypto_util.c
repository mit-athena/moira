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
#ifndef ultrix
#include <sys/time.h>
#else
#include <time.h>
#endif
#include <sys/types.h>
#include "random.h"
#include "BigNum.h"
#include "BigRSA.h"
#include "bigkeygen.h"

/*
 * InitDelegationKey.  Generate a new rsa key of desired bit length.
 *                     If random number generator is not yet seeded,
 *                     initialize with uncertainty from the principal
 *                     prime.
 */
int InitDelegationKey(principal, delegation, bitlen)
RSAKeyStorage *principal, *delegation;
int bitlen;
{
    RNGState rng;
    read_rng_state (&rng);
    if ((rng.count)==0) initialize_rng_state (principal->p,32);
    return(newRSAKey (delegation, bitlen));
}

/*
 * Private2Public.  Remove private key stuff in a key storage block.
 */
int Private2Public (key)
RSAKeyStorage *key;
{
       memset(key->p,0,PRIVATE_KEY_SIZE-PUBLIC_KEY_SIZE);
       key->pl=key->ql=key->dpl=key->dql=0;
}


/* 
 * initrandom - interactive random number initializer
 *
 * The following routine attempts to initialize the random number generator
 * with a reasonable amount of uncertainty.  This is provided mostly by
 * the user, with some additional time inputs from the system.
 */
int initrandom()
{ 
unsigned char bar[16];
int seed ;
char buffer[256], sphinx_ans[80];
char *ptr=buffer, *env=NULL;
int i;
long tick = clock();
struct timeval tv ;
struct timezone tz ;
int thischar , lastchar ;

        memset(ptr,0,256);

        gettimeofday(&tv, &tz);
        memcpy(ptr,&tv,sizeof(struct timeval));
        ptr += sizeof(struct timeval);
/*
        printf("\nRiddle of the Sphinx :\n\n");
        printf("What creature is it, that in the morning goes on four feet,\n");
        printf("at noon on two, and at night on three?\n\n");
        fflush(stdout);
        scanf("%s", sphinx_ans); gets(ptr);
        if ((strcasecmp(sphinx_ans, "man") == 0) || (strcasecmp(sphinx_ans, "woman") == 0)) {
          printf("\nCorrect!\n\n");
        } else {
          printf("\nWrong!  Please brush up on your Egyptian mythology.\n\n");
        }
*/
        tick = clock();
        memcpy(ptr,&tick,sizeof(long));
        ptr += sizeof(long);
        memcpy(ptr,&ptr,sizeof(char *));
        ptr+=sizeof(char *);
        gettimeofday(&tv, &tz);
        memcpy(ptr,&tv,sizeof(struct timeval));
        ptr += sizeof(struct timeval);
 
        seed = sizeof(buffer) - (ptr - buffer) - sizeof(struct timeval);

        printf("\nSome 'uncertainty' is needed to initialize  the  random");
        printf("\nnumber generator to generate your long term key. Please");
        printf("\nenter up to %d characters of text.  The quality of your key", seed);
        printf("\ndepends upon how 'uncertain' this input is.  When you");
        printf("\nthink you have entered enough text, enter two successive");
        printf("\ncarriage returns.");
        printf("\n\n");
        fflush(stdout);

        for( i = thischar = lastchar = 0;
          (thischar=getchar())!= EOF,((thischar != '\n')||(lastchar != '\n')); i++) {
              lastchar = thischar;
              ptr[ i % seed ] += (unsigned char) thischar ;
        } 

        printf("\nThank you very much.\n");

        gettimeofday(&tv, &tz);
        memcpy(ptr,&tv,sizeof(struct timeval));

        initialize_rng_state (buffer, sizeof(buffer));

}

