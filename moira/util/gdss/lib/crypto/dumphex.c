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

/* dumphex.c */
#include <stdio.h>
#include <ctype.h>

#define CHARS_PER_LINE 16

/*
 * write out half-ASCII hex characters to a file
 */

int fdumphex (x,l,fp)
int l;
unsigned char * x;
FILE *fp ;
{
    int i;
    for(i=0;i<l;i++){
	if ((i%CHARS_PER_LINE)==0)fprintf(fp,"\n");
	fprintf(fp," %02x",*x++);
	}
    fprintf(fp,"\n");	 
}

/*
 * read in half-ASCII text, skipping whites, until EOF or non-hex encoded byte
 * found, returning number of bytes read.
 */
int freadhex (x,l,fp)
int l;
unsigned char * x;
FILE *fp ;
{
    int i,j;
    for(i=0;fscanf(fp," %02x",&j);i++) *x++=j;
    return (i);
}


int dumphex (x,l)
int l;
unsigned char * x;
{
    fdumphex(x,l,stdout);
}
