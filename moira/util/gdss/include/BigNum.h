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

/* BigN.h - Types and structures for clients of BigNum */



		/******** representation of a bignum ******/
/*
**  <--------------------------- nl ---------------------------->
**  |   Least                                           Most    |
**  |Significant|           |           |           |Significant|
**  |BigNumDigit|           |           |           |BigNumDigit|
**  |___________|___________|___________|___________|___________|
**        ^                                          (sometimes
**        |                                            is zero)
**       nn
*/

/* signals BigNum.h already included */
#define BIGNUM

		/*************** sizes ********************/

#define BN_BYTE_SIZE			8
#define BN_WORD_SIZE			(sizeof (int) * BN_BYTE_SIZE)
#define BN_DIGIT_SIZE			(sizeof (BigNumDigit) * BN_BYTE_SIZE)

/* notes: */
/* BN_BYTE_SIZE: number of bits in a byte */
/* BN_WORD_SIZE: number of bits in an "int" in the target language */
/* BN_DIGIT_SIZE: number of bits in a digit of a BigNum */


		/****** results of compare functions ******/

 /* Note: we don't use "enum" to interface with Modula2+, Lisp, ... */
#define BN_LT				-1
#define BN_EQ				0
#define BN_GT				1


		/*************** boolean ******************/
#ifndef TRUE
#define 	TRUE			1
#endif
#ifndef FALSE
#define 	FALSE			0
#endif

typedef short				Boolean;


		/* if DIGITon16BITS is defined, a single digit is on 16 bits */
		/* otherwise (by default) a single digit is on 32 bits *****/

#ifdef DIGITon16BITS
typedef unsigned short			BigNumDigit;
#else
typedef unsigned int 			BigNumDigit;
#endif


		/* bignum types: digits, big numbers, carries ... */

typedef BigNumDigit * 	BigNum;		/* A big number is a digit pointer */
typedef unsigned short	BigNumCarry;	/* Either 0 or 1 */
typedef unsigned long 	BigNumProduct;	/* The product of two digits */
typedef unsigned long	BigNumLength;	/* The length of a bignum */
typedef short		BigNumCmp;	/* result of comparison */

/**/


		/************ functions of bn.c ***********/

extern void             BnnInit 			();
extern void             BnnClose 			();

extern Boolean		BnnIsZero 			();
extern BigNumCarry 	BnnMultiply			();
extern void		BnnDivide			();
extern BigNumCmp	BnnCompare			();


		/*********** functions of KerN.c **********/

extern void 		BnnSetToZero			();
extern void 		BnnAssign			();
extern void 		BnnSetDigit			();
extern BigNumDigit 	BnnGetDigit			();
extern BigNumLength	BnnNumDigits			();
extern BigNumDigit	BnnNumLeadingZeroBitsInDigit	();
extern Boolean 		BnnDoesDigitFitInWord 		();
extern Boolean		BnnIsDigitZero 			();
extern Boolean		BnnIsDigitNormalized 		();
extern Boolean 		BnnIsDigitOdd			();
extern BigNumCmp		BnnCompareDigits		();
extern void 		BnnComplement			();
extern void 		BnnAndDigits			();
extern void		BnnOrDigits			();
extern void		BnnXorDigits			();
extern BigNumDigit	BnnShiftLeft			();
extern BigNumDigit	BnnShiftRight			();
extern BigNumCarry 	BnnAddCarry			();
extern BigNumCarry 	BnnAdd				();
extern BigNumCarry 	BnnSubtractBorrow		();
extern BigNumCarry 	BnnSubtract			();
extern BigNumCarry 	BnnMultiplyDigit		();
extern BigNumDigit	BnnDivideDigit			();

/**/

		/* some functions can be written with macro-procedures */


#ifndef BNNMACROS_OFF
/* the functions BnnIsZero and BnnCompareDigits are not macro procedures
since they use parameters twice, and that can produce some bugs if
you pass a parameter like x++, the increment will be executed twice ! */
#define BnnSetToZero(nn,nl)             memset (nn, 0, (nl)*BN_DIGIT_SIZE/BN_BYTE_SIZE)
#define BnnSetDigit(nn,d) 		(*(nn) = (d))
#define BnnGetDigit(nn)			((unsigned)(*(nn)))
#define BnnDoesDigitFitInWord(d)	(BN_DIGIT_SIZE > BN_WORD_SIZE ? ((d) >= 1 << BN_WORD_SIZE ? FALSE : TRUE) : TRUE)
#define BnnIsDigitZero(d)		((d) == 0)
#define BnnIsDigitNormalized(d)		((d) & (1 << (BN_DIGIT_SIZE - 1)) ? TRUE : FALSE)
#define BnnIsDigitOdd(d) 		((d) & 1 ? TRUE : FALSE)
#define BnnAndDigits(nn, d)		(*(nn) &= (d))
#define BnnOrDigits(nn, d)		(*(nn) |= (d))
#define BnnXorDigits(nn, d)		(*(nn) ^= (d))

#endif


