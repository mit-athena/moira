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
 * CONTENTS:
 *
 *  This file contains RSA hash routines.
 */
 
#include <stdio.h>
#include "endian.h"
#include "hashes.h"


/*
 * This conditionally includes the BSAFE MD and MAC routines.  These
 * are not used by Sphinx so usually this definition is commented out.
 */
 
/*
#define INCLUDE_RSA_BSAFE_STUFF  1 
*/

#define TEMP_BUFSIZ 256

/*
 * Externally Callable Routines. All arguments are character pointers
 * or integer lengths.
 */

void RSA_MD2();         /* RSA_MD2(input_buffer, length, hash_result) */
void RSA_MD4();         /* RSA_MD4(input_buffer, length, hash_result) */
#ifdef INCLUDE_RSA_BSAFE_STUFF  
void RSA_MAC();         /* RSA_MAC(input_buffer, length, mac, output_length) */
void RSA_MD();          /* RSA_MD(input_buffer, length, hash_result) */
#endif


/* 
** **************************************************************************
** md4.h -- Header file for implementation of MD4 Message Digest Algorithm **
** Updated: 2/13/90 by Ronald L. Rivest                                    **
** (C) 1990 RSA Data Security, Inc.                                        **
** **************************************************************************
*/
 
/* MDstruct is the data structure for a message digest computation.
*/
typedef struct {
  unsigned int buffer[4];    /* Holds 4-word result of MD computation */
  unsigned char count[8];    /* Number of bits processed so far */
  unsigned int done;         /* Nonzero means MD computation finished */
} MDstruct, *MDptr;
 
/* 
** **************************************************************************
** md4.c -- Implementation of MD4 Message Digest Algorithm                 **
** Updated: 2/16/90 by Ronald L. Rivest                                    **
** (C) 1990 RSA Data Security, Inc.                                        **
** **************************************************************************
*/
 
/* 
** To use MD4:
**   -- Include md4.h in your program
**   -- Declare an MDstruct MD to hold the state of the digest computation.
**   -- Initialize MD using MDbegin(&MD)
**   -- For each full block (64 bytes) X you wish to process, call
**          MDupdate(&MD,X,512)
**      (512 is the number of bits in a full block.)
**   -- For the last block (less than 64 bytes) you wish to process,
**          MDupdate(&MD,X,n)
**      where n is the number of bits in the partial block. A partial
**      block terminates the computation, so every MD computation should
**      terminate by processing a partial block, even if it has n = 0.
**   -- The message digest is available in MD.buffer[0] ... MD.buffer[3].
**      (Least-significant byte of each word should be output first.)
**   -- You can print out the digest using MDprint(&MD)
*/
 
/* Implementation notes:
** This implementation assumes that ints are 32-bit quantities.
** If the machine stores the least-significant byte of an int in the
** least-addressed byte (eg., VAX and 8086), then LOWBYTEFIRST should be
** set to TRUE.  Otherwise (eg., SUNS), LOWBYTEFIRST should be set to
** FALSE.  Note that on machines with LOWBYTEFIRST FALSE the routine
** MDupdate modifies has a side-effect on its input array (the order of bytes
** in each word are reversed).  If this is undesired a call to MDreverse(X) can
** reverse the bytes of X back into order after each call to MDupdate.
*/
#ifdef TRUE
#undef TRUE
#endif

#define TRUE  1

#ifdef FALSE
#undef FALSE
#endif

#define FALSE 0

#define LOWBYTEFIRST SPHINX_ENDIAN

/* Compile-time declarations of MD4 ``magic constants''.
*/
#define I0  0x67452301       /* Initial values for MD buffer */
#define I1  0xefcdab89
#define I2  0x98badcfe
#define I3  0x10325476
#define C2  013240474631     /* round 2 constant = sqrt(2) in octal */
#define C3  015666365641     /* round 3 constant = sqrt(3) in octal */
/* C2 and C3 are from Knuth, The Art of Programming, Volume 2
** (Seminumerical Algorithms), Second Edition (1981), Addison-Wesley.
** Table 2, page 660.
*/
#define fs1  3               /* round 1 shift amounts */
#define fs2  7   
#define fs3 11  
#define fs4 19  
#define gs1  3               /* round 2 shift amounts */
#define gs2  5   
#define gs3  9   
#define gs4 13  
#define hs1  3               /* round 3 shift amounts */
#define hs2  9 
#define hs3 11 
#define hs4 15
 
 
/* Compile-time macro declarations for MD4.
** Note: The ``rot'' operator uses the variable ``tmp''.
** It assumes tmp is declared as unsigned int, so that the >>
** operator will shift in zeros rather than extending the sign bit.
*/
#define	f(X,Y,Z)             ((X&Y) | ((~X)&Z))
#define	g(X,Y,Z)             ((X&Y) | (X&Z) | (Y&Z))
#define h(X,Y,Z)             (X^Y^Z)
#define rot(X,S)             (tmp=X,(tmp<<S) | (tmp>>(32-S)))
#define ff(A,B,C,D,i,s)      A = rot((A + f(B,C,D) + X[i]),s)
#define gg(A,B,C,D,i,s)      A = rot((A + g(B,C,D) + X[i] + C2),s)
#define hh(A,B,C,D,i,s)      A = rot((A + h(B,C,D) + X[i] + C3),s)
 

/* MDprint(MDp)
** Print message digest buffer MDp as 32 hexadecimal digits.
** Order is from low-order byte of buffer[0] to high-order byte of buffer[3].
** Each byte is printed with high-order hexadecimal digit first.
** This is a user-callable routine.
*/
static void MDprint(MDp)
MDptr MDp;
{ int i,j;
  for (i=0;i<4;i++)
    for (j=0;j<32;j=j+8)
      printf("%02x",(MDp->buffer[i]>>j) & 0xFF);
}
 
/* MDbegin(MDp)
** Initialize message digest buffer MDp. 
** This is a user-callable routine.
*/
static void MDbegin(MDp)
MDptr MDp;
{ int i;
  MDp->buffer[0] = I0;  
  MDp->buffer[1] = I1;  
  MDp->buffer[2] = I2;  
  MDp->buffer[3] = I3; 
  for (i=0;i<8;i++) MDp->count[i] = 0;
  MDp->done = 0;
}
 
/* MDreverse(X)
** Reverse the byte-ordering of every int in X.
** Assumes X is an array of 16 ints.
** The macro revx reverses the byte-ordering of the next word of X.
*/
#define revx { t = (*X << 16) | (*X >> 16); \
	       *X++ = ((t & 0xFF00FF00) >> 8) | ((t & 0x00FF00FF) << 8); }
static void MDreverse(X)
unsigned int *X;
{ register unsigned int t;
  revx; revx; revx; revx; revx; revx; revx; revx;
  revx; revx; revx; revx; revx; revx; revx; revx;
}

/* MDblock(MDp,X)
** Update message digest buffer MDp->buffer using 16-word data block X.
** Assumes all 16 words of X are full of data.
** Does not update MDp->count.
** This routine is not user-callable. 
*/
static void MDblock(MDp,X)
MDptr MDp;
unsigned int *X;
{ 
  register unsigned int tmp, A, B, C, D;
#if LOWBYTEFIRST == FALSE
  MDreverse(X);
#endif
  A = MDp->buffer[0];
  B = MDp->buffer[1];
  C = MDp->buffer[2];
  D = MDp->buffer[3];
  /* Update the message digest buffer */
  ff(A , B , C , D ,  0 , fs1); /* Round 1 */
  ff(D , A , B , C ,  1 , fs2); 
  ff(C , D , A , B ,  2 , fs3); 
  ff(B , C , D , A ,  3 , fs4); 
  ff(A , B , C , D ,  4 , fs1); 
  ff(D , A , B , C ,  5 , fs2); 
  ff(C , D , A , B ,  6 , fs3); 
  ff(B , C , D , A ,  7 , fs4); 
  ff(A , B , C , D ,  8 , fs1); 
  ff(D , A , B , C ,  9 , fs2); 
  ff(C , D , A , B , 10 , fs3); 
  ff(B , C , D , A , 11 , fs4); 
  ff(A , B , C , D , 12 , fs1); 
  ff(D , A , B , C , 13 , fs2); 
  ff(C , D , A , B , 14 , fs3); 
  ff(B , C , D , A , 15 , fs4); 
  gg(A , B , C , D ,  0 , gs1); /* Round 2 */
  gg(D , A , B , C ,  4 , gs2); 
  gg(C , D , A , B ,  8 , gs3); 
  gg(B , C , D , A , 12 , gs4); 
  gg(A , B , C , D ,  1 , gs1); 
  gg(D , A , B , C ,  5 , gs2); 
  gg(C , D , A , B ,  9 , gs3); 
  gg(B , C , D , A , 13 , gs4); 
  gg(A , B , C , D ,  2 , gs1); 
  gg(D , A , B , C ,  6 , gs2); 
  gg(C , D , A , B , 10 , gs3); 
  gg(B , C , D , A , 14 , gs4); 
  gg(A , B , C , D ,  3 , gs1); 
  gg(D , A , B , C ,  7 , gs2); 
  gg(C , D , A , B , 11 , gs3); 
  gg(B , C , D , A , 15 , gs4);  
  hh(A , B , C , D ,  0 , hs1); /* Round 3 */
  hh(D , A , B , C ,  8 , hs2); 
  hh(C , D , A , B ,  4 , hs3); 
  hh(B , C , D , A , 12 , hs4); 
  hh(A , B , C , D ,  2 , hs1); 
  hh(D , A , B , C , 10 , hs2); 
  hh(C , D , A , B ,  6 , hs3); 
  hh(B , C , D , A , 14 , hs4); 
  hh(A , B , C , D ,  1 , hs1); 
  hh(D , A , B , C ,  9 , hs2); 
  hh(C , D , A , B ,  5 , hs3); 
  hh(B , C , D , A , 13 , hs4); 
  hh(A , B , C , D ,  3 , hs1); 
  hh(D , A , B , C , 11 , hs2); 
  hh(C , D , A , B ,  7 , hs3); 
  hh(B , C , D , A , 15 , hs4);
  MDp->buffer[0] += A; 
  MDp->buffer[1] += B;
  MDp->buffer[2] += C;
  MDp->buffer[3] += D; 
}
 

/* MDupdate(MDp,X,count)
** Input: MDp -- an MDptr
**        X -- a pointer to an array of unsigned characters.
**        count -- the number of bits of X to use.
**                 (if not a multiple of 8, uses high bits of last byte.)
** Update MDp using the number of bits of X given by count.
** This is the basic input routine for an MD4 user.
** The routine completes the MD computation when count < 512, so
** every MD computation should end with one call to MDupdate with a
** count less than 512.  A call with count 0 will be ignored if the
** MD has already been terminated (done != 0), so an extra call with count
** 0 can be given as a ``courtesy close'' to force termination if desired.
*/
static int MDupdate(MDp,X,count)
MDptr MDp;
unsigned char *X;
unsigned int count;
{ unsigned int i, tmp, bit, byte, mask;
  unsigned char XX[64];
  unsigned char *p;
  /* return with no error if this is a courtesy close with count
  ** zero and MDp->done is true.
  */
  if (count == 0 && MDp->done) return(1);
  /* check to see if MD is already done and report error */
  if (MDp->done) { 
#ifdef DEBUG
        printf("\nError: MDupdate MD already done."); 
#endif        
                        return(0); }
  /* Add count to MDp->count */
  tmp = count;
  p = MDp->count;
  while (tmp)
    { tmp += *p;
      *p++ = tmp;
      tmp = tmp >> 8;
    }
  /* Process data */
  if (count == 512) 
    { /* Full block of data to handle */
      MDblock(MDp,(unsigned int *)X);
    }
  else if (count > 512) /* Check for count too large */
    { 
#ifdef DEBUG
printf("\nError: MDupdate called with illegal count value %d.",count);
#endif
      return(0);
    }
  else /* partial block -- must be last block so finish up */
    { /* Find out how many bytes and residual bits there are */
      byte = count >> 3;
      bit =  count & 7;
      /* Copy X into XX since we need to modify it */
      for (i=0;i<=byte;i++)   XX[i] = X[i];
      for (i=byte+1;i<64;i++) XX[i] = 0;
      /* Add padding '1' bit and low-order zeros in last byte */
      mask = 1 << (7 - bit);
      XX[byte] = (XX[byte] | mask) & ~( mask - 1);
      /* If room for bit count, finish up with this block */
      if (byte <= 55)
	{ for (i=0;i<8;i++) XX[56+i] = MDp->count[i];
	  MDblock(MDp,(unsigned int *)XX);
	}
      else /* need to do two blocks to finish up */
	{ MDblock(MDp,(unsigned int *)XX);
	  for (i=0;i<56;i++) XX[i] = 0;
	  for (i=0;i<8;i++)  XX[56+i] = MDp->count[i];
	  MDblock(MDp,(unsigned int *)XX);
	}
      /* Set flag saying we're done with MD computation */
      MDp->done = 1;
    }
  return(1);
}
 
/* 
** End of md4.c
*/



/*
 * MD4
 */
void RSA_MD4 (inbuf, isize, digest)
char * inbuf, * digest ;
int isize;
{ unsigned int i;
  MDstruct MD;
  MDbegin(&MD);
  for (i=0;i+64<=isize;i=i+64) MDupdate(&MD,inbuf+i,512);
  MDupdate(&MD,inbuf+i,(isize-i)*8);
  memcpy(digest,MD.buffer,16);
}



#ifdef INCLUDE_RSA_BSAFE_STUFF
/* SUBSTITUTION TABLE BASED ON DIGITS OF PI -- SEE PISUBST.DOC */
/* obtained from RSA, Inc. */
static unsigned char _pisubst[256] = { 
	189, 86,234,242,162,241,172, 42,176,147,209,156, 27, 51,253,208,
	 48,  4,182,220,125,223, 50, 75,247,203, 69,155, 49,187, 33, 90,
	 65,159,225,217, 74, 77,158,218,160,104, 44,195, 39, 95,128, 54,
	 62,238,251,149, 26,254,206,168, 52,169, 19,240,166, 63,216, 12,
	120, 36,175, 35, 82,193,103, 23,245,102,144,231,232,  7,184, 96,
	 72,230, 30, 83,243,146,164,114,140,  8, 21,110,134,  0,132,250,
	244,127,138, 66, 25,246,219,205, 20,141, 80, 18,186, 60,  6, 78,
	236,179, 53, 17,161,136,142, 43,148,153,183,113,116,211,228,191,
	 58,222,150, 14,188, 10,237,119,252, 55,107,  3,121,137, 98,198,
	215,192,210,124,106,139, 34,163, 91,  5, 93,  2,117,213, 97,227,
	 24,143, 85, 81,173, 31, 11, 94,133,229,194, 87, 99,202, 61,108,
	180,197,204,112,178,145, 89, 13, 71, 32,200, 79, 88,224,  1,226,
	 22, 56,196,111, 59, 15,101, 70,190,126, 45,123,130,249, 64,181,
	 29,115,248,235, 38,199,135,151, 37, 84,177, 40,170,152,157,165,
	100,109,122,212, 16,129, 68,239, 73,214,174, 46,221,118, 92, 47,
	167, 28,201,  9,105,154,131,207, 41, 57,185,233, 76,255, 67,171
 };

/* The table PS given below is a permutation of 0...255 constructed    */
/*  from the digits of pi.  It is a ``random'' nonlinear byte         */
/*  substitution operation.                                           */
static unsigned char PS[256] = {
        41, 46, 67,201,162,216,124,  1, 61, 54, 84,161,236,240,  6, 19,
        98,167,  5,243,192,199,115,140,152,147, 43,217,188, 76,130,202,
        30,155, 87, 60,253,212,224, 22,103, 66,111, 24,138, 23,229, 18,
       190, 78,196,214,218,158,222, 73,160,251,245,142,187, 47,238,122,
       169,104,121,145, 21,178,  7, 63,148,194, 16,137, 11, 34, 95, 33,
       128,127, 93,154, 90,144, 50, 39, 53, 62,204,231,191,247,151,  3,
       255, 25, 48,179, 72,165,181,209,215, 94,146, 42,172, 86,170,198,
        79,184, 56,210,150,164,125,182,118,252,107,226,156,116,  4,241,
        69,157,112, 89,100,113,135, 32,134, 91,207,101,230, 45,168,  2,
        27, 96, 37,173,174,176,185,246, 28, 70, 97,105, 52, 64,126, 15,
        85, 71,163, 35,221, 81,175, 58,195, 92,249,206,186,197,234, 38,
        44, 83, 13,110,133, 40,132,  9,211,223,205,244, 65,129, 77, 82,
       106,220, 55,200,108,193,171,250, 36,225,123,  8, 12,189,177, 74,
       120,136,149,139,227, 99,232,109,233,203,213,254, 59,  0, 29, 57,
       242,239,183, 14,102, 88,208,228,166,119,114,248,235,117, 75, 10,
        49, 68, 80,180,143,237, 31, 26,219,153,141, 51,159, 17,131, 20,
};


/*
 *			R S A _ m a c
 *
 *	Compute a message authentication code (MAC) over the specified
 *	buffer.  This uses the RSADSI MAC algorithm.
 *
 * Inputs:
 *	inbuf	- Pointer to the input buffer
 *	isize	- Size of the input buffer
 *	macsize - Number of bytes desired in MAC
 *
 * Outputs:
 *	mac	- Pointer to the resultant message authentication code buffer
 *
 * Return Value:
 */
void RSA_MAC(inbuf, isize, mac, macsize)
unsigned char * inbuf, * mac ;
int isize, macsize ;
{
	int i;
	unsigned char temp;
	memset(mac, 0, macsize);  /* initialize the mac buffer */
	macsize--; 		  /* change to index */
	/*
	 * Run over the input buffer merging each byte into the MAC.
	 */
	for (i = 0; i < isize; i++)
	{
		temp = _pisubst[mac[0] ^ mac[1]];
		/*
		 * Shift down the MAC one place and merge the new value into
		 * the last place.
		 */
#ifdef VAX
		memcpy(&mac[0],&mac[1], macsize);
#else
		memmove(&mac[0],&mac[1], macsize);
#endif
		mac[macsize] = temp ^ *inbuf++;
	}
}



/*
 * 			      R S A _ M D
 *
 *	Compute a message digest over the specified buffer using the
 *	RSA, Inc., message digest "MD" algorithm.
 *
 * Inputs:
 *	inbuf	- Pointer to the input buffer
 *	isize	- Size of the input buffer 
 *
 * Outputs:
 *	digest	- Pointer to the resultant digest buffer. Assumed to be
 *		  16 bytes.
 *
 */

void RSA_MD (inbuf, isize, digest )
char * inbuf, * digest ;
int isize;
{
	register int 		i,
				j,
				k,
				l;
	static int 		padlen;
	static unsigned char 	lastmac ,
				x,
				t,
				buf[48],
				mac[16];
    

	memset(digest, 0, MD_BLOCK_SIZE);  /* initialize return digest */
	memset(buf, 0, sizeof(buf)); /* initialize temporaries */
	memset(mac, 0, sizeof(mac));
    
	for (i = 0, k = 0 , lastmac = 0 ; i < isize ; i++)
	{
		/*
		 * Merge the new character into the buffer and 
		 * update the MAC.
		 */
		buf[k + 16] = *inbuf;
		buf[k + 32] = *inbuf ^ buf[k];
		lastmac = (mac[k] ^= PS[(*inbuf++ ^ lastmac) & 0xFF]);
		
		k = (k + 1) & 15;
		/*
		 * Encrypt at the end of each block.
		 */
		if (k==0){
		  t = 0;
		  for (l = 0; l < 18; l++)
		     for (j = 48; j > 0; j--)
		         t = (buf[48 - j] ^= PS[(t + j) & 0xFF]);
		}
	}

	padlen = MD_BLOCK_SIZE - k;
	x = (unsigned char) padlen ;

	for (i = 0; i < padlen ; i++)
	{
		buf[k + 16] = x ;
		buf[k + 32] = x ^ buf[k];
		lastmac = (mac[k] ^= PS[(x ^ lastmac) & 0xFF]);

		k = (k + 1) & 15;
		/*
		 * Encrypt 
		 */
		if (k==0){
		  t = 0;
		  for (l = 0; l < 18; l++)
		     for (j = 48; j > 0; j--)
		         t = (buf[48 - j] ^= PS[(t + j) & 0xFF]);
		}
	}

	/*
	 * Now merge the MAC computed above into the message digest value.
	 */
	for (i = 0; i < 16; i++)
	{
		buf[i + 16] = mac[i];
		buf[i + 32] = mac[i] ^ buf[i];
	}
	t = 0;
	for (i = 0; i < 18; i++)
	{
		for (j = 48; j > 0; j--)
		{
			t = (buf[48 - j] ^= PS[(t + j) & 0xFF]);
		}
	}
    
	/*
	 * Now copy the final digest value to the output.
	 */
	memcpy(digest, buf, MD_BLOCK_SIZE);

}

#endif /* INCLUDE_RSA_BSAFE_STUFF */




/* RSA-MD2 Message Digest algorithm in C  */
/*  by Ronald L. Rivest 10/1/88  */

/**********************************************************************/
/* Message digest routines:                                           */
/* To form the message digest for a message M                         */
/*    (1) Initialize a context buffer md using MDINIT                 */
/*    (2) Call MDUPDATE on md and each character of M in turn         */
/*    (3) Call MDFINAL on md                                          */
/* The message digest is now in md->D[0...15]                         */
/**********************************************************************/
/* An MDCTX structure is a context buffer for a message digest        */
/*  computation; it holds the current "state" of a message digest     */
/*  computation                                                       */
struct MDCTX
{
   unsigned char  D[48];   /* buffer for forming digest in */
                           /* At the end, D[0...15] form the message */
                           /*  digest */
   unsigned char  C[16];   /* checksum register */
   unsigned char  i;       /* number of bytes handled, modulo 16 */
   unsigned char  L;       /* last checksum char saved */
};
/* The table S given below is a permutation of 0...255 constructed    */
/*  from the digits of pi.  It is a ``random'' nonlinear byte         */
/*  substitution operation.                                           */
int S[256] = {
        41, 46, 67,201,162,216,124,  1, 61, 54, 84,161,236,240,  6, 19,
        98,167,  5,243,192,199,115,140,152,147, 43,217,188, 76,130,202,
        30,155, 87, 60,253,212,224, 22,103, 66,111, 24,138, 23,229, 18,
       190, 78,196,214,218,158,222, 73,160,251,245,142,187, 47,238,122,
       169,104,121,145, 21,178,  7, 63,148,194, 16,137, 11, 34, 95, 33,
       128,127, 93,154, 90,144, 50, 39, 53, 62,204,231,191,247,151,  3,
       255, 25, 48,179, 72,165,181,209,215, 94,146, 42,172, 86,170,198,
        79,184, 56,210,150,164,125,182,118,252,107,226,156,116,  4,241,
        69,157,112, 89,100,113,135, 32,134, 91,207,101,230, 45,168,  2,
        27, 96, 37,173,174,176,185,246, 28, 70, 97,105, 52, 64,126, 15,
        85, 71,163, 35,221, 81,175, 58,195, 92,249,206,186,197,234, 38,
        44, 83, 13,110,133, 40,132,  9,211,223,205,244, 65,129, 77, 82,
       106,220, 55,200,108,193,171,250, 36,225,123,  8, 12,189,177, 74,
       120,136,149,139,227, 99,232,109,233,203,213,254, 59,  0, 29, 57,
       242,239,183, 14,102, 88,208,228,166,119,114,248,235,117, 75, 10,
        49, 68, 80,180,143,237, 31, 26,219,153,141, 51,159, 17,131, 20,
};
/*The routine MDINIT initializes the message digest context buffer md.*/
/* All fields are set to zero.                                        */
void MDINIT(md)
  struct MDCTX *md;
  { int i;
    for (i=0;i<16;i++) md->D[i] = md->C[i] = 0;
    md->i = 0;
    md->L = 0;
  }
/* The routine MDUPDATE updates the message digest context buffer to  */
/*  account for the presence of the character c in the message whose  */
/*  digest is being computed.  This routine will be called for each   */
/*   message byte in turn.                                            */
void MDUPDATE(md,c)
  struct MDCTX *md;
  unsigned char c;
  { register unsigned char i,j,t,*p;
    /**** Put i in a local register for efficiency ****/
       i = md->i;
    /**** Add new character to buffer ****/
       md->D[16+i] = c;
       md->D[32+i] = c ^ md->D[i];
    /**** Update checksum register C and value L ****/
       md->L = (md->C[i] ^= S[0xFF & (c ^ md->L)]);
    /**** Increment md->i by one modulo 16 ****/
       i = md->i = (i + 1) & 15;
    /**** Transform D if i=0 ****/
       if (i == 0)
         { t = 0;
           for (j=0;j<18;j++)
             {/*The following is a more efficient version of the loop:*/
               /*  for (i=0;i<48;i++) t = md->D[i] = md->D[i] ^ S[t]; */
               p = md->D;
               for (i=0;i<8;i++)
                 { t = (*p++ ^= S[t]);
                   t = (*p++ ^= S[t]);
                   t = (*p++ ^= S[t]);
                   t = (*p++ ^= S[t]);
                   t = (*p++ ^= S[t]);
                   t = (*p++ ^= S[t]);
                 }
               /* End of more efficient loop implementation */
               t = t + j;
             }
         }
  }
/* The routine MDFINAL terminates the message digest computation and  */
/* ends with the desired message digest being in md->D[0...15].       */
void MDFINAL(md)
  struct MDCTX *md;
  { int i,padlen;
    /* pad out to multiple of 16 */
       padlen  = 16 - (md->i);
       for (i=0;i<padlen;i++) MDUPDATE(md,(unsigned char)padlen);
    /* extend with checksum */
    /* Note that although md->C is modified by MDUPDATE, character    */
    /* md->C[i] is modified after it has been passed to MDUPDATE, so  */
    /* the net effect is the same as if md->C were not being modified.*/
    for (i=0;i<16;i++) MDUPDATE(md,md->C[i]);
  }

/**********************************************************************/
/* End of message digest implementation                               */
/**********************************************************************/

void RSA_MD2 (inbuf, isize, digest )
char * inbuf, * digest ;
int isize;
{
	struct MDCTX temp ;
	int i;

	MDINIT(&temp);
	for(i=0;i<isize;i++) MDUPDATE(&temp,*inbuf++);
	MDFINAL(&temp);
	memcpy(digest,temp.D,16);
}
