/* hashes.h */
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


/*
 * Externally Callable Routines. All arguments are character pointers
 * or integer lengths.
 */

#ifndef SPHINX_HASHES
#define SPHINX_HASHES

void RSA_MD2();         /* RSA_MD2(input_buffer, length, hash_result) */
void RSA_MD4();         /* RSA_MD4(input_buffer, length, hash_result) */
void RSA_MAC();         /* RSA_MAC(input_buffer, length, mac, output_length) */
void RSA_MD();          /* RSA_MD(input_buffer, length, hash_result) */

int H1();               /* H1(username, password, hash_result) */
int H2();               /* H2(username, password, hash_result) */

/*
 * Common data structures
 */
#define MAC_BLOCK_SIZE	8
#define MD_BLOCK_SIZE	16


#endif

