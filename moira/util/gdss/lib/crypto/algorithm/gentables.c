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

#ifndef SPHINX_ITAR
#define SPHINX_ITAR
static char copyright[] = "\n Copyright, 1989, 1990, Digital Equipment Corporation \n";
static char warning[]= "\n  This software is subject to export restrictions under \
\n  the U.S. Department of State's International Traffic in Arms \
\n  Regulations (ITAR).  This software must not be transmitted \
\n  in source form outside the United States or to a foreign \
\n  national in the United States without a valid U.S. State \
\n  Department export license. ";
#endif

/*
 * Generate the permutation tables for a software implementation of DES.
 */
#include <stdio.h>

#define FILENAME	"tables.h"
#define EFILE           "endian.h"

typedef unsigned long	INT32;	 /* generic type for 32-bit integer */

/*
 * The following tables are based on those in the Data Encryption
 * Standard document (FIPS PUB 46).
 */

/*
 * Initial permutation IP
 */
static char ip[] = { 2, 4, 6, 8, 1, 3, 5, 7 };

/*
 * Final permutation IP^-1
 */
static char fp[] = { 8, 7, 6, 5, 4, 3, 2, 1 };

/*
 * The (in)famous S-boxes
 */
static char si[8][64] = {
	/*
	 * S1
	 */
	14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
	 0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
	 4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
	15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13,

	/*
	 * S2
	 */
	15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
	 3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
	 0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
	13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9,

	/*
	 * S3
	 */
	10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
	13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
	13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
	 1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12,

	/*
	 * S4
	 */
	 7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
	13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
	10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
	 3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14,

	/*
	 * S5
	 */
	 2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
	14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
	 4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
	11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3,

	/*
	 * S6
	 */
	12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
	10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
	 9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
	 4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13,

	/*
	 * S7
	 */
	 4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
	13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
	 1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
	 6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12,

	/*
	 * S8
	 */
	13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
	 1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
	 7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
	 2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
};

/*
 * 32-bit permutation function P used on the output of the S-boxes
 */
static char p32i[] = {	
	16,  7, 20, 21,
	29, 12, 28, 17,
	 1, 15, 23, 26,
	 5, 18, 31, 10,
	 2,  8, 24, 14,
	32, 27,  3,  9,
	19, 13, 30,  6,
	22, 11,  4, 25
};

static char ei[] = {
	32,  1,  2,  3,  4,  5,
	 4,  5,  6,  7,  8,  9,
	 8,  9, 10, 11, 12, 13,
	12, 13, 14, 15, 16, 17,
	16, 17, 18, 19, 20, 21,
	20, 21, 22, 23, 24, 25,
	24, 25, 26, 27, 28, 29,
	28, 29, 30, 31, 32,  1 
};

/*
 * Permuted choice table (key)
 */
static char pc1[] = {
	57, 49, 41, 33, 25, 17,  9,
	 1, 58, 50, 42, 34, 26, 18,
	10,  2, 59, 51, 43, 35, 27,
	19, 11,  3, 60, 52, 44, 36,

	63, 55, 47, 39, 31, 23, 15,
	 7, 62, 54, 46, 38, 30, 22,
	14,  6, 61, 53, 45, 37, 29,
	21, 13,  5, 28, 20, 12,  4
};

/*
 * Permuted choice key (table)
 */
static char pc2[] = {
	14, 17, 11, 24,  1,  5,
	 3, 28, 15,  6, 21, 10,
	23, 19, 12,  4, 26,  8,
	16,  7, 27, 20, 13,  2,
	41, 52, 31, 37, 47, 55,
	30, 40, 51, 45, 33, 48,
	44, 49, 39, 56, 34, 53,
	46, 42, 50, 36, 29, 32
};

/*
 * Note that the Standard is written in Big Endian terms so bit 1 is the
 * high bit of the byte.
 */
static int bytebit[] = {
    0200, 0100, 040, 020, 010, 04, 02, 01
};

/*
 * Generated tables
 */
static INT32 iperma[256],ipermb[256];
static INT32 fperma[256],fpermb[256];
static INT32 sp[8][64];
static char pc1bit[56],pc1byte[56];

static FILE *tables;
static FILE *endian;

/*
 * Initialize a permutation array.
 */
static perminit( perma,permb,p )
INT32 perma[256],permb[256];
char p[8];
{
    register int i,j;
    
    /*
     * Clear out the permutation array.
     */
    memset(perma, 0, sizeof(perma));
    memset(permb, 0, sizeof(permb));
    
    /*
     * Now produce the table.
     */
    for (i = 0; i < 4; i++)
	for (j = 0; j < 256; j++)
	{
	    if ((j & bytebit[p[i] - 1]) != 0)
		perma[j] |= 1L << (8 * (3 - i));
	    if ((j & bytebit[p[i+4] - 1]) != 0)
		permb[j] |= 1L << (8 * (3 - i));
	}
}

outperm( perma,permb,name,comment )
INT32 perma[256],permb[256];
char *name,*comment;
{
    register int i, j;
    
    fprintf(tables, "/*\n * %s\n */\n", comment);
    fprintf(tables, "static INT32 %sa[256] = {", name);
    for (i = 0; i < 64; i++)
    {
	fprintf(tables, "\n    ");
	for (j = 0; j < 4; j++)
	    fprintf(tables, "0x%lx, ", (long)perma[(i * 4) + j]);
    }

    fprintf(tables, "\n};\n\nstatic INT32 %sb[256] = {", name);
    for (i = 0; i < 64; i++)
    {
	fprintf(tables, "\n    ");
	for (j = 0; j < 4; j++)
	    fprintf(tables, "0x%lx, ", (long)permb[(i * 4) + j]);
    }
    fprintf(tables, "\n};\n\n");
}

/*
 * Intialize the combined S and P boxes
 */
static spinit()
{
    register int i,j,k,l;
    INT32 perm;
    unsigned char inv[32];
    
    /*
     * Invert the pbox array.
     */
    for (i = 0; i < 32; i++)
	inv[p32i[i] - 1] = i;

    /*
     * Now build a combination of the p and s boxes.
     */
    for (i = 0; i < 8; i++)
	for (j = 0; j < 64; j++)
	{
	    perm = 0;
	    
	    /*
	     * Compute the index into the sbox table. The row number is
	     * formed from bits 0 and 5 (VAX terminology) and the column
	     * number from the middle 4 bits.
	     */
	    l = (j & 0x20) | ((j & 1) ? 0x10 : 0) | ((j >> 1) & 0xF);
	    
	    /*
	     * Each sbox entry gives us 4 bits which must be merged into
	     * the final table.
	     */
	    for (k = 0; k < 4; k++)
	    {
		if ((si[i][l] & (8 >> k)) != 0)
		    perm |= 1L << (31 - inv[(i * 4) + k]);
	    }
	    sp[i][j] = perm;
	}
}

outsp()
{
    register int i,j,k;
    
    fprintf(tables, "/*\n * Combined s and p boxes\n */\n");
    fprintf(tables, "static INT32 sp[8][64] = {");
    
    for (i = 0; i < 8; i++)
    {
	fprintf(tables, "\n    /*\n     * Permuted S box %d\n     */", i);
	for (j = 0; j < 16; j++)
	{
	    fprintf(tables, "\n    ");
	    for (k = 0; k < 4; k++)
		fprintf(tables, "0x%lx, ", (long)(sp[i][(j * 4) + k]));
	}
    }
    fprintf(tables, "\n};\n\n");
}

choiceinit( pc,bit,byte )
char pc[],bit[],byte[];
{
    register int i;
    
    /*
     * Convert a permuted choice table from bit addressing to 2 tables
     * giving the byte and bit within the byte address.
     */
    for (i = 0; i < 56; i++)
    {
	bit[i] = bytebit[(pc[i] - 1) & 07];
	byte[i] = (pc[i] - 1) >> 3;
    }
}

outchoice( bit,byte,name,comment )
char bit[],byte[],*name,*comment;
{
    register int i,j;
    
    fprintf(tables, "/*\n * %s\n */\n", comment);
    fprintf(tables, "static unsigned char %sbit[56] = {", name);
    for (i = 0; i < 7; i++)
    {
	fprintf(tables, "\n    ");
	for (j = 0; j < 8; j++)
	    fprintf(tables, "0x%x, ", bit[(i * 8) + j] & 0xFF);
    }
    fprintf(tables, "\n};\n\n");

    fprintf(tables, "static unsigned char %sbyte[56] = {", name);
    for (i = 0; i < 7; i++)
    {
	fprintf(tables, "\n    ");
	for (j = 0; j < 8; j++)
	    fprintf(tables, "%2d, ", byte[(i * 8) + j]);
    }
    fprintf(tables, "\n};\n\n");
    
}

INT32 swap( x )
INT32 x;
{
    register char *cp,temp;
    
    cp = (char *)&x;
    temp = cp[3];
    cp[3] = cp[0];
    cp[0] = temp;
    
    temp = cp[2];
    cp[2] = cp[1];
    cp[1] = temp;
    
    return (x);
}

main( argc,argv )
int argc;
char *argv[];
{
    register int i;
    union { char bytes [ sizeof(long) ];
            long longs; } testword ;
    long now;
    
    time(&now);

    testword.longs = 1;

    if ((endian = fopen(EFILE, "w")) == NULL)
    {
	perror("endian");
	exit(2);
    }

    fprintf(endian, "/* endian.h */\n");
    fprintf(endian, "/*\n * Machine generated on %s */\n", ctime(&now));
    fprintf(endian, "\n#ifndef SPHINX_ENDIAN\n\n");
    fprintf(endian, "/*\n");
    fprintf(endian, " * Little endian machines are DEC/Intel like\n");
    fprintf(endian, " * Big endian machines are IBM/SPARC/Motorola like\n *\n");
    fprintf(endian, " * This machine is ");
    if (testword.bytes[0]) fprintf (endian,"little "); else fprintf(endian,"big ");
    fprintf(endian, "endian since the value of SPHINX_ENDIAN is %u\n", testword.bytes[0]);
    fprintf(endian, " *\n */\n\n");
    fprintf(endian, "\n#define SPHINX_ENDIAN %u\n\n", testword.bytes[0]);
    fprintf(endian, "\n#endif\n\n\n");
    
    if ((tables = fopen(FILENAME, "w")) == NULL)
    {
	perror("gentables");
	exit(2);
    }
    
    fprintf(tables, "/*\n * Machine generated tables for DES encryption %s */\n\n",
                                        ctime(&now));
    if (testword.bytes[0]) 
         fprintf(tables, "/*\n * Little Endian (DEC/Intel like)\n */\n\n");
    else fprintf(tables, "/*\n * Big Endian (IBM/SPARC/Motorola like)\n */\n\n");

    perminit(iperma, ipermb, ip);
    perminit(fperma, fpermb, fp);

    if (testword.bytes[0]) /* little endian */
    /*
     * Perform a byte swap on the final permutation table.
     */
    for (i = 0; i < 256; i++)
    {
	fperma[i] = swap(fperma[i]);
	fpermb[i] = swap(fpermb[i]);
    }
    
    outperm(iperma, ipermb, "iperm", "Initial permutation");
    outperm(fperma, fpermb, "fperm", "Final permutation");
    spinit();
    outsp();
    choiceinit(pc1, pc1bit, pc1byte);
    outchoice(pc1bit, pc1byte, "pc1", "Permuted choice 1 table");
    exit(0);
}
