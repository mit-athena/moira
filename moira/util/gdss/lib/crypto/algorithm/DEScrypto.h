/* DEScrypto.h */
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

/* In-line include file for DES routines */

#ifndef SPHINX_DESCRYPTO
#define SPHINX_DESCRYPTO

/*
 *********************************************************************
 *********************************************************************
 **                                                                 **
 **                       W A R N I N G                             **
 **                                                                 **
 **  This software is subject to export restrictions under the      **
 **  U.S. Department of State's International Traffic in Arms       **
 **  Regulations (ITAR).  This software must not be transmitted     **
 **  in source form outside the United States or to a foreign       **
 **  national in the United States without a valid U.S. State       **
 **  Department export license.                                     **
 **                                                                 **
 *********************************************************************
 *********************************************************************
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

#include "endian.h"
#include "hashes.h"
#include "random.h"
#include "tables.h"

static DESdecrypt_local();
static DESencrypt_local();
static void DES_load_key_local();
static int DES_CBC_encrypt_local ();
static int DES_CBC_decrypt_local ();

typedef struct 
{					/* DES key schedule */
    unsigned char	subkey[16][8];	/* 8 6-bit subkeys per round */
} KEYschedule;

/*
 * The following macros perform the initial and final permutations on a
 * DES block. The output is accessed as 32-bit units and the input as 8-bit
 * units.
 *
 * Note: These macros reference 2 permutation arrays (iperm and fperm) which
 *	 are machine generated and must be included with any program using
 *	 these macros.
 */

#define INITIAL(out, in) \
    out[0] = iperma[in[0]] \
	     | (iperma[in[1]] << 1) \
	     | (iperma[in[2]] << 2) \
	     | (iperma[in[3]] << 3) \
	     | (iperma[in[4]] << 4) \
	     | (iperma[in[5]] << 5) \
	     | (iperma[in[6]] << 6) \
	     | (iperma[in[7]] << 7); \
    out[1] = ipermb[in[0]] \
	     | (ipermb[in[1]] << 1) \
	     | (ipermb[in[2]] << 2) \
	     | (ipermb[in[3]] << 3) \
	     | (ipermb[in[4]] << 4) \
	     | (ipermb[in[5]] << 5) \
	     | (ipermb[in[6]] << 6) \
	     | (ipermb[in[7]] << 7)

#if SPHINX_ENDIAN
#define FINAL(out, in) \
    out[0] = fperma[in[0]] \
	     | (fperma[in[4]] << 1) \
	     | (fperma[in[1]] << 2) \
	     | (fperma[in[5]] << 3) \
	     | (fperma[in[2]] << 4) \
	     | (fperma[in[6]] << 5) \
	     | (fperma[in[3]] << 6) \
	     | (fperma[in[7]] << 7); \
    out[1] = fpermb[in[0]] \
	     | (fpermb[in[4]] << 1) \
	     | (fpermb[in[1]] << 2) \
	     | (fpermb[in[5]] << 3) \
	     | (fpermb[in[2]] << 4) \
	     | (fpermb[in[6]] << 5) \
	     | (fpermb[in[3]] << 6) \
	     | (fpermb[in[7]] << 7);
#else
#define FINAL(out, in) \
    out[0] = fperma[in[3]] \
	     | (fperma[in[7]] << 1) \
	     | (fperma[in[2]] << 2) \
	     | (fperma[in[6]] << 3) \
	     | (fperma[in[1]] << 4) \
	     | (fperma[in[5]] << 5) \
	     | (fperma[in[0]] << 6) \
	     | (fperma[in[4]] << 7); \
    out[1] = fpermb[in[3]] \
	     | (fpermb[in[7]] << 1) \
	     | (fpermb[in[2]] << 2) \
	     | (fpermb[in[6]] << 3) \
	     | (fpermb[in[1]] << 4) \
	     | (fpermb[in[5]] << 5) \
	     | (fpermb[in[0]] << 6) \
	     | (fpermb[in[4]] << 7);

#endif	/* SPHINX_ENDIAN */

/*
 * The following tables are taken directly from FIPS PUB 46 (Specifications
 * for the Data Encryption Standard).
 */

/*
 * Number of left rotations of pc1
 */
static char totrot[] = {
	1,2,4,6,8,10,12,14,15,17,19,21,23,25,27,28
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
 * Declare external and forward routines.
 */


/*
 *
 * Functional Description:
 *
 *	Perform a DES decryption of a single 8-byte block producing
 *	another 8-byte block.
 *
 * Inputs:
 *
 *	indata	- Pointer to the input data
 *
 * Outputs:
 *
 *	outdata	- Pointer to the output data
 *
 * Return Value:
 *
 *	None
 *
 */
static DESdecrypt_local( indata,outdata,key_schedule )
KEYschedule *key_schedule ;
DESblock *indata,*outdata;
{
    register int i;
    register INT32 er,out;
    register unsigned char *subkey;
    INT32 index;
    DESblock temp;
    
    /*
     * Perform the initial transformation.
     */
    INITIAL(temp.longwords, indata->bytes);

    /*
     * Now swap halves of the working block.
     */
    out = temp.longwords[0];
    temp.longwords[0] = temp.longwords[1];
    temp.longwords[1] = out;
    
    /*
     * Perform the decryption operation.
     */
    for (i = 15; i >= 0; i--)
    {
	/*
	 * Select the subkey for this round.
	 */
	subkey = key_schedule->subkey[i];

	/*
	 * Compute the index value to be used for the first 7 subkeys.
	 */
	index = temp.longwords[1 - (i & 1)];
	er = (index >> 1) | ((index & 1) ? 0x80000000 : 0);
	
	/*
	 * Perform one round of E(input) ^ K through the combined S and
	 * P boxes.
	 */
	out  = sp[0][((er >> 26) & 0x3F) ^ *subkey++];
	out |= sp[1][((er >> 22) & 0x3F) ^ *subkey++];
	out |= sp[2][((er >> 18) & 0x3F) ^ *subkey++];
	out |= sp[3][((er >> 14) & 0x3F) ^ *subkey++];
	out |= sp[4][((er >> 10) & 0x3F) ^ *subkey++];
	out |= sp[5][((er >> 6) & 0x3F) ^ *subkey++];
	out |= sp[6][((er >> 2) & 0x3F) ^ *subkey++];
	er = (index << 1) | ((index & 0x80000000) ? 1 : 0);
	out |= sp[7][(er & 0x3F) ^ *subkey];

	temp.longwords[i & 1] ^= out;
    }
    
    /*
     * Perform the final transformation.
     */
    FINAL(outdata->longwords, temp.bytes);
}


/*
 *
 * Functional Description:
 *
 *	Perform a DES encryption of a single 8-byte block producing
 *	another 8-byte block.
 *
 * Inputs:
 *
 *	indata	- Pointer to the input data
 *
 * Outputs:
 *
 *	outdata	- Pointer to the output data
 *
 * Return Value:
 *
 *	None
 *
 */
static DESencrypt_local( indata,outdata,key_schedule )
DESblock *indata,*outdata;
KEYschedule *key_schedule ;
{
    register int i;
    register INT32 er,out;
    register unsigned char *subkey;
    INT32 index;
    DESblock temp;
    
    /*
     * Perform the initial transformation.
     */
    INITIAL(temp.longwords, indata->bytes);
    
    /*
     * Perform the encryption operation.
     */
    for (i = 0; i < 16; i++)
    {
	/*
	 * Select the subkey for this round.
	 */
	subkey = key_schedule->subkey[i];

	/*
	 * Compute the index value to be used for the first 7 subkeys.
	 */
	index = temp.longwords[1 - (i & 1)];
	er = (index >> 1) | ((index & 1) ? 0x80000000 : 0);
	
	/*
	 * Perform one round of E(input) ^ K through the combined S and
	 * P boxes.
	 */
	out  = sp[0][((er >> 26) & 0x3F) ^ *subkey++];
	out |= sp[1][((er >> 22) & 0x3F) ^ *subkey++];
	out |= sp[2][((er >> 18) & 0x3F) ^ *subkey++];
	out |= sp[3][((er >> 14) & 0x3F) ^ *subkey++];
	out |= sp[4][((er >> 10) & 0x3F) ^ *subkey++];
	out |= sp[5][((er >> 6) & 0x3F) ^ *subkey++];
	out |= sp[6][((er >> 2) & 0x3F) ^ *subkey++];
	er = (index << 1) | ((index & 0x80000000) ? 1 : 0);
	out |= sp[7][(er & 0x3F) ^ *subkey];

	temp.longwords[i & 1] ^= out;
    }
    
    /*
     * Now swap halves of the working block.
     */
    out = temp.longwords[0];
    temp.longwords[0] = temp.longwords[1];
    temp.longwords[1] = out;

    /*
     * Perform the final transformation.
     */
    FINAL(outdata->longwords, temp.bytes);
}

/*
 *
 * Functional Description:
 *
 *	Expand a DES key from an 8-byte array into a key schedule. This
 *	expansion allows the later encryption routines to operate faster.
 *
 * Inputs:
 *
 *	inkey	- Pointer to an 8-byte input key
 *
 * Outputs:
 *
 * Return Value:
 *
 *	None
 *
 */
static void DES_load_key_local( key, key_schedule )
register unsigned char *key;
KEYschedule *key_schedule ;
{
    register int i,j,k;
    unsigned char k1[56],k2[56];

    /*
     * Clear out the key schedule, in case it isn't already
     */
    memset(key_schedule, 0, sizeof(KEYschedule));
    
    /*
     * Compute the key for the first round as an array of 56 bits (k1).
     */
    for (i = 0; i < 56; i++)
	k1[i] = (key[pc1byte[i]] & pc1bit[i]) ? 1 : 0;

    /*
     * Build the subkeys by rotating and selecting the bits from each
     * iteration.
     */
    for (i = 0; i < 16; i++)
    {
	/*
	 * Rotate the initial key permutation the correct amount for
	 * this subkey, rotating each half independently.
	 */
	for (j = 0; j < 56; j++)
	    k2[j] = k1[(k = j + totrot[i]) < (j < 28 ? 28 : 56) ? k : k - 28];

	/*
	 * Select the final values for each subkey.
	 */
	for (j = 0; j < 48; j++)
	    if (k2[pc2[j] - 1] != 0)
		key_schedule->subkey[i][j/6] |= bytebit[j % 6] >> 2;
    }
}



/*
 *
 *			D E S _ C B C _ e n c r y p t
 *
 *	Encrypt a buffer using DES in Cipher Block Chaining mode.
 *
 * Inputs:
 *	iv	- Pointer to initialization vector or zero
 *	inbuf	- Pointer to the input buffer
 *	isize	- Size of the input buffer
 *	outbuf	- Pointer to the output buffer (can be same as input)
 *
 * Outputs:
 *	outbuf  - contains encrypted data
 *
 * Return Value:
 *	0	- Buffer is not a multiple of 8 bytes
 *	1	- Success
 */
static int DES_CBC_encrypt_local (iv , inbuf, isize, outbuf,key_schedule)
unsigned char * iv, * inbuf, * outbuf ;
int isize;
KEYschedule *key_schedule ;
{
	unsigned long temp1[2] ;
	unsigned long * inptr = (unsigned long *) inbuf;
	unsigned long * outptr = (unsigned long *) outbuf;
    
	/*
	 * Check that we have a good buffer size.
	 */
	if ((isize & 0x7) != 0) return (0);

	if (iv!=0) memcpy(temp1,iv,sizeof(temp1)); /* copy over initialization vector */
	else 	memset(temp1,0,sizeof(temp1));

	while (isize > 0) {

		temp1[0] ^= * inptr ++ ; /* XOR previous ciphertext with next data */
		temp1[1] ^= * inptr ++ ;
		
		DESencrypt_local(temp1,temp1,key_schedule); /* encrypt */
		
		* outptr ++ = temp1 [0]; /* copy out data */
		* outptr ++ = temp1 [1];

		isize -= 8;
	}
	return(1);
}

/*
 *
 *			D E S _ C B C _ d e c r y p t
 *
 *	Decrypt a buffer using DES in Cipher Block Chaining mode.
 *
 * Inputs:
 *	iv	- Pointer to initialization vector or zero
 *	inbuf	- Pointer to the input buffer
 *	isize	- Size of the input buffer
 *	outbuf	- Pointer to the output buffer  (can be same as input)
 *
 * Outputs:
 *	outbuf  - contains encrypted data
 *
 * Return Value:
 *	0	- Buffer is not a multiple of 8 bytes
 *	1	- Success
 */
static int DES_CBC_decrypt_local (iv , inbuf, isize, outbuf,key_schedule)
char * iv, * inbuf, * outbuf ;
int isize;
KEYschedule *key_schedule ;
{
	unsigned long temp1[2], temp2[2];
	unsigned long * inptr = (unsigned long *) inbuf;
	unsigned long * outptr = (unsigned long *) outbuf;
    
	/*
	 * Check that we have a good buffer size.
	 */
	if ((isize & 0x7) != 0) return (0);

	if (iv!=0) memcpy(temp1,iv,sizeof(temp1)); /* copy over initialization vector */
	else	memset(temp1,0,sizeof(temp1));

	while (isize > 0) {
		DESdecrypt_local(inptr,temp2,key_schedule); /* decrypt to temporary */

		temp2[0] ^= temp1 [0]; /* XOR previous ciphertext with data */
		temp2[1] ^= temp1 [1];
		
		temp1[0] = * inptr ++ ; /* copy in ciphertext for next block XOR */
		temp1[1] = * inptr ++ ;

		* outptr ++ = temp2 [0]; /* copy out data */
		* outptr ++ = temp2 [1];

		isize -= 8;
	}
	return(1);
}

#endif
