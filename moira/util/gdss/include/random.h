/* random.h */
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

#ifndef SPHINX_RANDOM
#define SPHINX_RANDOM

typedef unsigned long	INT32;		/* 32-bit unsigned integer */

typedef union 
{					/* data type for the DES blocks */
    unsigned char	bytes[8];	/* for access as bytes */
    INT32		longwords[2];	/* for access as longwords */
} DESblock;

#define DES_BLOCK_SIZE sizeof(DESblock)

typedef struct {
        int count ;
        DESblock seed , key , current;
        }  RNGState ;

#define RNG_STATE_SIZE sizeof(RNGState) 
 
void read_rng_state ();
void restore_rng_state ();
void initialize_rng_state ();
void random_bytes ();
void DES_X9_MAC();      /* DES_X9_MAC(key,input_buffer, length, hash_result) */

#endif

