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
#include <sys/types.h>
#include "BigNum.h"
#include "BigRSA.h"

#ifdef DEBUG
#undef DEBUG
#endif

/*
 * These routines convert ASN.1 to RSAKeyStorage and vice-versa.
 */

#define INTEGER_TAG	2
#define BITSTRING_TAG   3
#define SEQUENCE_TAG	48
#define CONTEXT_0_TAG	160
#define CONTEXT_1_TAG	161
#define CONTEXT_2_TAG	162
#define CONTEXT_3_TAG	163
#define CONTEXT_4_TAG	164
#define CONTEXT_5_TAG	165

/*
 * Routines for encoding keys.
 */
 
/* 
 * Compute total size of length field assuming minimal (X.509) encoding.
 */

int NumEncodeLengthOctets (size) 
int size;
{
        int i = 2;
        if (size<128) i=1; else if (size>256) i=3 ;
        return(i);
}

/*
 *  Write out length, advance pointer to next thing to be written.
 */
unsigned char *EncodeLength (input, size)
unsigned char *input;
int size;
{
        int i = NumEncodeLengthOctets(size);

        if (i==1) (*input++)=(unsigned char)size ;
        else {  i--;
                (*input++)= i+0x80;
                for (i--;i>=0;i--) (*input++)=(size>>(i*8));
        }
        return(input);
}

/* 
 * Compute the value of the INTEGER encoded length field for a BigNum 
 * of length nl.
 */
int NumEncodeValueOctets(n,nl)
int nl;
BigNum n;
{
        int i,j;
        
        if (n==0) return (0);
        
        i = BnnNumDigits(n,nl);
        j = BnnNumLeadingZeroBitsInDigit(BnnGetDigit(n+i-1));

        i=i*sizeof(int)-j/8;
        if ((j%8)==0)i++;
        return(i);
}

/* 
 * Write out BigNum as encoded INTEGER, most to least significant octet, and
 * return pointer to next octet in stream.  Expects to be supplied with
 * a sufficiently large buffer.  Always expects the BigNum to be positive.
 */
unsigned char *EncodeBigIntegerValue (stream, input, size)
unsigned char *stream;
BigNum input;
unsigned size; 
{
        int nl = BnnNumDigits(input, size);
        BigNumDigit d = BnnGetDigit(input+nl-1);
        int i = BnnNumLeadingZeroBitsInDigit(d);

        if ((i%8)==0) *stream++ = 0;      /* want positive number, after all */
        for(i=sizeof(int)-(i/8)-1;i>=0;i--) *stream++ = (unsigned char) (d>>(i*8))&0xff;
        for(nl--;nl>0;nl--){
                  d = BnnGetDigit(input+nl-1);
                  for(i=sizeof(int)-1;i>=0;i--) *stream++ = (d>>(i*8))&0xff;
        }
        return(stream);
}

/*
 * Make an encoded public key from RSAKeyStorage.  This includes allocating
 * the stream storage.
 *
 * A public key is defined as SEQUENCE { INTEGER, INTEGER }
 */
unsigned char *EncodePublic(keys)
RSAKeyStorage *keys;
{
        int mod_len,exp_len,sequence_len,public_key_len ;
        unsigned char *buffer, *start;

        if ((keys->nl)==0) return(0);
        
        mod_len = NumEncodeValueOctets(keys->n,keys->nl);
        exp_len = NumEncodeValueOctets(keys->e,keys->el);
        sequence_len = exp_len + mod_len + NumEncodeLengthOctets(exp_len)+
        	NumEncodeLengthOctets(mod_len)+2;
        public_key_len = NumEncodeLengthOctets(sequence_len)+sequence_len+1;

        if (start = buffer = (unsigned char *) calloc(public_key_len,1)) {
        	*buffer++ = SEQUENCE_TAG;
        	buffer = EncodeLength (buffer,sequence_len);
        	*buffer++= INTEGER_TAG;	
        	buffer=EncodeLength (buffer,mod_len);
        	buffer=EncodeBigIntegerValue(buffer,keys->n,keys->nl);
        	*buffer++= INTEGER_TAG;	
        	buffer=EncodeLength (buffer,exp_len);
        	buffer=EncodeBigIntegerValue (buffer,keys->e,keys->el);
        }
        return(start);
}

void FreePublic(buffer)
{       free(buffer);
}

/*
 * Make an encoded private key from RSAKeyStorage.  This includes allocating
 * the stream storage.
 *
 * A private key is defined as 
 *   SEQUENCE { 
 *              primep     INTEGER,           -- always there
 *              primeq [0] INTEGER OPTIONAL,
 *              mod    [1] INTEGER OPTIONAL,  -- never there
 *              exp    [2] INTEGER OPTIONAL,
 *              dp     [3] INTEGER OPTIONAL,
 *              dq     [4] INTEGER OPTIONAL,
 *              cr     [5] INTEGER OPTIONAL }
 */ 
unsigned char *EncodePrivate(keys)
RSAKeyStorage *keys;
{
        int p_len, q_len, exp_len, dp_len, dq_len, cr_len, sequence_len ;
        unsigned char *buffer, *start;

        /* must have two primes to be a private key */
        if ((keys->pl ==0)||(keys->ql ==0)) return(0);

        sequence_len = 6 +
                (p_len = NumEncodeValueOctets(keys->p,keys->pl)) +
                NumEncodeLengthOctets(p_len) +
                (q_len = NumEncodeValueOctets(keys->q,keys->ql)) +
                NumEncodeLengthOctets(q_len) +
                (exp_len = NumEncodeValueOctets(keys->e,keys->el)) +
                NumEncodeLengthOctets(exp_len) + 
                (dp_len = NumEncodeValueOctets(keys->dp,keys->dpl)) +
                NumEncodeLengthOctets(dp_len) +
                (dq_len = NumEncodeValueOctets(keys->dq,keys->dql)) +
                NumEncodeLengthOctets(dq_len) + 
                (cr_len = NumEncodeValueOctets(keys->cr,keys->pl)) +
                NumEncodeLengthOctets(cr_len);            

        if (start=buffer= 
         (unsigned char *)calloc(1+NumEncodeLengthOctets(sequence_len)+sequence_len,1)) {
        	*buffer++ = SEQUENCE_TAG;
        	buffer=EncodeLength (buffer,sequence_len);
        	*buffer++= INTEGER_TAG;	
        	buffer=EncodeLength (buffer,p_len);
        	buffer=EncodeBigIntegerValue (buffer,keys->p,keys->pl);
        	*buffer++= CONTEXT_0_TAG;	
        	buffer=EncodeLength (buffer,q_len);
        	buffer=EncodeBigIntegerValue (buffer,keys->q,keys->ql);
                /*
                 * No optional modulus.
                 */
        	*buffer++= CONTEXT_2_TAG;
        	buffer=EncodeLength (buffer,exp_len);
        	buffer=EncodeBigIntegerValue (buffer,keys->e,keys->el);
        	*buffer++= CONTEXT_3_TAG;
        	buffer=EncodeLength (buffer,dp_len);
        	buffer=EncodeBigIntegerValue (buffer,keys->dp,keys->dpl);
        	*buffer++= CONTEXT_4_TAG;
        	buffer=EncodeLength (buffer,dq_len);
        	buffer=EncodeBigIntegerValue (buffer,keys->dq,keys->dql);
        	*buffer++= CONTEXT_5_TAG;
        	buffer=EncodeLength (buffer,cr_len);
        	buffer=EncodeBigIntegerValue (buffer,keys->cr,keys->pl);
        }
        return(start);
}

/* 
 * The following routine is similar to the above but just puts prime p in 
 * the encoded private key.
 */
unsigned char *EncodePrivateP(keys)
RSAKeyStorage *keys;
{
        int p_len = NumEncodeValueOctets(keys->p,keys->pl);
        int sequence_len = p_len + 1 + NumEncodeLengthOctets(p_len) ;
        unsigned char *start, *buffer ;

        if (start=buffer= (unsigned char *)
             calloc(1+NumEncodeLengthOctets(sequence_len)+sequence_len, 1)) {
        	*buffer++ = SEQUENCE_TAG;
        	buffer=EncodeLength (buffer,sequence_len);
        	*buffer++= INTEGER_TAG;	
        	buffer=EncodeLength (buffer,p_len);
        	buffer=EncodeBigIntegerValue (buffer,keys->p,keys->pl);
        }
        return(start);
}

void FreePrivate(buffer)
{       free(buffer);
}


/*
 * Decoding routines.
 */
 
/*
 * Input is an encoded value field, tells how many octets in the value.
 * Does not handle indefinite length forms.
 */
int DecodeValueLength (input) /* handles non-minimal size case */
unsigned char *input ;
{
        int len , i ;

        if ((*input&0x80)==0) return(*input);
        if (*input==0x80) return(0);
        for (i=(*input++ & 0x7f),len=0;i>0;i--) len=(len << 8)+(unsigned int)*input++;

        return(len);
}

/*
 * Input is undecoded string.  Returns offset to start of value field.
 */
int DecodeHeaderLength (input)
unsigned char *input ;
{
        int tlen = DecodeTypeLength(input);
        input+=tlen;
        tlen++;
        if (*input&0x80) tlen += (*input&0x7f);
        return(tlen);
}

/*
 * Compute number of type bytes.
 */
int DecodeTypeLength (input)
unsigned char *input;
{       int temp=1;
        if ((*input&31)==31) do {input++;temp++;} while (*input&0x80) ;
        return(temp);
}

/*
 * Compute offset to next element to decode.
 * Assumes definite length encoding.
 */
int DecodeTotalLength (input)
unsigned char *input ;
{
 return (DecodeHeaderLength(input)+DecodeValueLength(input+DecodeTypeLength(input)));
}

/* 
 *  DecodeBigInteger Returns pointer to one byte beyond end of encoded value.
 *  Reads in least to most significant.
 */
unsigned char *DecodeBigInteger (stream, value, size)
unsigned char *stream;
BigNum value ;
int size;
{
        int nl = (size+sizeof(int)-1)/sizeof(int);
        BigNumDigit d;
        int i,j,k=size-1;

        for(i=0;i<(nl-1);i++){
            d = 0;
            for(j=0;j<sizeof(int);j++)
              d |= (BigNumDigit)(stream[k-j]&0xff)<<(j*8);
            BnnSetDigit(value+i,d);
            k-=sizeof(int);
          }
/* assert: i = nl-1 */
        d=0;
        BnnSetToZero(value+i,1);
        nl=(size+sizeof(int)-1)%sizeof(int);
        for(j=0;j<=nl;j++)
              d |= (BigNumDigit)(stream[k-j]&0xff)<<(j*8);
        BnnSetDigit(value+i,d);

        return(stream+size);
}

/* 
 * DecodePublic.  Initializes RSAKeyStorage. 
 * Returns pointer to next element if no syntax problems, else zero.
 */
unsigned char *DecodePublic (input, keys)
unsigned char *input;
RSAKeyStorage *keys;
{
        register len;
        unsigned char *next=input;

        memset(keys,0,sizeof(RSAKeyStorage));

        if ((*input != SEQUENCE_TAG)|| /* must be definite form */
            (DecodeValueLength(input+1)<=0))  goto syntax_error;
        input += DecodeHeaderLength(input);

        if (*input != INTEGER_TAG) goto syntax_error;
        len = DecodeValueLength(input+1);
        if ((len<=0)||(len >= sizeof(int)*2*DigitLim)) goto syntax_error;
        input+= DecodeHeaderLength(input);
        input = DecodeBigInteger (input, keys->n, len);
        keys->nl = 1 + BnnNumDigits(keys->n, (len+sizeof(int)-1)/sizeof(int));

        if (*input != INTEGER_TAG) goto syntax_error;
        len = DecodeValueLength(input+1);
        if ((len<=0)||(len >= sizeof(int)*2*DigitLim)) goto syntax_error;
        input+= DecodeHeaderLength(input);
        input = DecodeBigInteger (input, keys->e, len);
        keys->el = BnnNumDigits(keys->e, (len+sizeof(int)-1)/sizeof(int));

        /* 
         * should be at the end of the encoded substring
         */
        if (input != next+DecodeTotalLength(next)) goto syntax_error ;
        
        FinishKey(keys);

        return(input);

syntax_error:
#ifdef DEBUG
printf("\nDecodePublic: Syntax Error.\n");
#endif
        return(0);
}

/* 
 * DecodePrivate.  Returns pointer to next element if good, else zero.
 *
 * Fills in RSAKeyStorage with key parameters if succesful, 
 * assuming there is enough to go on.  The modulus can be supplied in the structure
 * already, but other values are assumed cleared as necessary.  The idea
 * in this case is to call with the same structure as the public key is in to
 * make it into a private key structure based on decoding the prime p.
 */
unsigned char *DecodePrivate (input, keys)
unsigned char *input;
RSAKeyStorage *keys;
{
        int len, crl ;
        unsigned char *next=input;
        BigNum x;
        
        if ((*input != SEQUENCE_TAG)|| /* must be definite form */
            (DecodeValueLength(input+1)<=0))  goto syntax_error;
        input += DecodeHeaderLength(input);

        if (*input == INTEGER_TAG) { /* must have a p */
                len = DecodeValueLength(input+1);
                if ((len<=0)||(len >= sizeof(BigNumDigit)*PrimeSize)) goto syntax_error;
                input+= DecodeHeaderLength(input);
                input = DecodeBigInteger (input, keys->p, len);
                len = keys->pl = BnnNumDigits(x=keys->p, (len+sizeof(int)-1)/sizeof(int));
                if ((x[len-1]&(3<<BN_DIGIT_SIZE-2))!=0) 
                        if (len<PrimeSize) keys->pl = len+1;
                        else goto syntax_error ;
        } else goto syntax_error ;

        if (*input == CONTEXT_0_TAG) {  /* may have a q */
                len = DecodeValueLength(input+1);
                if ((len<=0)||(len >= sizeof(BigNumDigit)*PrimeSize)) goto syntax_error;
                input+= DecodeHeaderLength(input);
                input = DecodeBigInteger (input, keys->q, len);
                len= keys->ql = BnnNumDigits(x=keys->q, (len+sizeof(int)-1)/sizeof(int));
                if ((x[len-1]&(3<<BN_DIGIT_SIZE-2))!=0) 
                        if (len<PrimeSize) keys->ql = len+1;
                        else goto syntax_error ;
        }
        else keys->ql=0;
               
        if (*input == CONTEXT_1_TAG) { /* may have a modulus */
                memset(keys->n,0,sizeof(*keys->n));
                len = DecodeValueLength(input+1);
                if ((len<=0)||(len >= sizeof(int)*2*DigitLim)) goto syntax_error;
                input+= DecodeHeaderLength(input);
                input = DecodeBigInteger (input, keys->n, len);
                keys->nl = 1 + BnnNumDigits(keys->n, (len+sizeof(int)-1)/sizeof(int));
        }
        
        memset(keys->e,0,sizeof(*keys->e));
        if (*input == CONTEXT_2_TAG) { /* may have an exponent */
                len = DecodeValueLength(input+1);
                if ((len<=0)||(len >= sizeof(int)*2*DigitLim)) goto syntax_error;
                input+= DecodeHeaderLength(input);
                input = DecodeBigInteger (input, keys->e, len);
        } 
        else  /* use default exponent */
                BnnSetDigit(keys->e, (BigNumDigit) ((1<<16)+1) );  /* 32 bits */
        keys->el = BnnNumDigits(keys->e, (len+sizeof(int)-1)/sizeof(int));

        memset(keys->dp,0,sizeof(*keys->dp));
        if (*input == CONTEXT_3_TAG) { /* may have a dp */
                len = DecodeValueLength(input+1);
                if ((len<=0)||(len >= sizeof(int)*2*DigitLim)) goto syntax_error;
                input+= DecodeHeaderLength(input);
                input = DecodeBigInteger (input, keys->dp, len);
                keys->dpl = BnnNumDigits(keys->dp, (len+sizeof(int)-1)/sizeof(int));
        }
        else    keys->dpl=0;

        memset(keys->dq,0,sizeof(*keys->dq));
        if (*input == CONTEXT_4_TAG) { /* may have a dq */
                len = DecodeValueLength(input+1);
                if ((len<=0)||(len >= sizeof(int)*2*DigitLim)) goto syntax_error;
                input+= DecodeHeaderLength(input);
                input = DecodeBigInteger (input, keys->dq, len);
                keys->dql = BnnNumDigits(keys->dq, (len+sizeof(int)-1)/sizeof(int));
        }
        else    keys->dql=0;
        
        memset(keys->cr,0,sizeof(*keys->cr));
        if (*input == CONTEXT_5_TAG) { /* may have a cr */
                len = DecodeValueLength(input+1);
                if ((len<=0)||(len >= sizeof(int)*2*DigitLim)) goto syntax_error;
                input+= DecodeHeaderLength(input);
                input = DecodeBigInteger (input, keys->cr, len);
                crl = 1;
        }
        else    crl=0;
        
        /* 
         * should be at the end of the encoded substring
         */
        if (input != next+DecodeTotalLength(next)) goto syntax_error ;
        
        if (keys->ql == 0)              /* need to regenerate q */
                if (keys->nl == 0) goto syntax_error; /* must know n */
                else {
                        BigNumDigit xx[4*DigitLim];
                        int ql, pl, nl, i;
                        
                        memset(xx,0,sizeof(xx));
                        BnnAssign(xx,keys->n,nl=keys->nl);
                        BnnDivide(xx,nl,keys->p,pl=keys->pl);
                        /* check that p evenly divided n */
                        for(i=0;i<pl;i++) if(xx[i]) goto syntax_error;
                        ql = keys->ql = BnnNumDigits(xx,nl) - pl;
                        BnnAssign(keys->q,xx+pl,ql);
                        if ((x[ql-1]&(3<<BN_DIGIT_SIZE-2))!=0) 
                                if (ql<PrimeSize) keys->ql = ql+1;
                                else goto syntax_error ;
                }

        if (keys->nl == 0){      /* need to regenerate n */
                 int nl = keys->pl + keys->ql;
                 memset(keys->n,0,sizeof(*keys->n));
                 BnnMultiply(keys->n,nl,keys->p,keys->pl,keys->q,keys->ql);
                 keys->nl=nl;
        }

        if(FinishKey(keys) == 0) goto syntax_error ;

        return(input);
syntax_error:
#ifdef DEBUG
printf("\nDecodePrivate: Syntax Error.\n");
#endif
#ifndef DEBUG
        memset(keys->p,0,sizeof(*keys->p));
        memset(keys->q,0,sizeof(*keys->q));
        memset(keys->dp,0,sizeof(*keys->dp));
        memset(keys->dq,0,sizeof(*keys->dq));
        memset(keys->cr,0,sizeof(*keys->cr));
        keys->pl=keys->ql=keys->dpl=keys->dql=0;
#endif
        return(0);
}

/* 
 * DecodePrivateP.  Returns pointer to next element if good, else zero.
 *
 * Fills in RSAKeyStorage with key parameters if succesful, 
 * assuming there is enough to go on.  The modulus can be supplied in the structure
 * already, but other values are assumed cleared as necessary.  The idea
 * in this case is to call with the same structure as the public key is in to
 * make it into a private key structure based on decoding the prime p.
 * Only the prime p must be passed in.
 */
unsigned char *DecodePrivateP (input, keys)
unsigned char *input;
RSAKeyStorage *keys;
{
        int len, crl ;
        unsigned char *next=input;
        BigNum x;
        
        if ((*input != SEQUENCE_TAG)|| /* must be definite form */
            (DecodeValueLength(input+1)<=0))  goto syntax_error;
        input += DecodeHeaderLength(input);

        if (*input == INTEGER_TAG) { /* must have a p */
                len = DecodeValueLength(input+1);
                if ((len<=0)||(len >= sizeof(BigNumDigit)*PrimeSize)) goto syntax_error;
                input+= DecodeHeaderLength(input);
                input = DecodeBigInteger (input, keys->p, len);
                len = keys->pl = BnnNumDigits(x=keys->p, (len+sizeof(int)-1)/sizeof(int));
                if ((x[len-1]&(3<<BN_DIGIT_SIZE-2))!=0) 
                        if (len<PrimeSize) keys->pl = len+1;
                        else goto syntax_error ;
        } else goto syntax_error ;

        if (*input == CONTEXT_0_TAG) goto syntax_error ;
        else keys->ql=0;
               
        if (*input == CONTEXT_1_TAG) goto syntax_error ;
        
        memset(keys->e,0,sizeof(*keys->e));
        if (*input == CONTEXT_2_TAG) goto syntax_error ;
        else  /* use default exponent */
                BnnSetDigit(keys->e, (BigNumDigit) ((1<<16)+1) );  /* 32 bits */
        keys->el = BnnNumDigits(keys->e, (len+sizeof(int)-1)/sizeof(int));

        memset(keys->dp,0,sizeof(*keys->dp));
        if (*input == CONTEXT_3_TAG) goto syntax_error ;
        else    keys->dpl=0;

        memset(keys->dq,0,sizeof(*keys->dq));
        if (*input == CONTEXT_4_TAG) goto syntax_error ;
        else    keys->dql=0;
        
        memset(keys->cr,0,sizeof(*keys->cr));
        if (*input == CONTEXT_5_TAG) goto syntax_error ;
        else    crl=0;
        
        /* 
         * should be at the end of the encoded substring
         */
        if (input != next+DecodeTotalLength(next)) goto syntax_error ;
        
        if (keys->ql == 0)              /* need to regenerate q */
                if (keys->nl == 0) goto syntax_error; /* must know n */
                else {
                        BigNumDigit xx[4*DigitLim];
                        int ql, pl, nl, i;
                        
                        memset(xx,0,sizeof(xx));
                        BnnAssign(xx,keys->n,nl=keys->nl);
                        BnnDivide(xx,nl,keys->p,pl=keys->pl);
                        /* check that p evenly divided n */
                        for(i=0;i<pl;i++) if(xx[i]) goto syntax_error;
                        ql = keys->ql = BnnNumDigits(xx,nl) - pl;
                        BnnAssign(keys->q,xx+pl,ql);
                        if ((x[ql-1]&(3<<BN_DIGIT_SIZE-2))!=0) 
                                if (ql<PrimeSize) keys->ql = ql+1;
                                else goto syntax_error ;
                }

        if (keys->nl == 0){      /* need to regenerate n */
                 int nl = keys->pl + keys->ql;
                 memset(keys->n,0,sizeof(*keys->n));
                 BnnMultiply(keys->n,nl,keys->p,keys->pl,keys->q,keys->ql);
                 keys->nl=nl;
        }

        if(FinishKey(keys) == 0) goto syntax_error ;

        return(input);
syntax_error:
#ifdef DEBUG
printf("\nDecodePrivateP: Syntax Error.\n");
#endif
#ifndef DEBUG
        memset(keys->p,0,sizeof(*keys->p));
        memset(keys->q,0,sizeof(*keys->q));
        memset(keys->dp,0,sizeof(*keys->dp));
        memset(keys->dq,0,sizeof(*keys->dq));
        memset(keys->cr,0,sizeof(*keys->cr));
        keys->pl=keys->ql=keys->dpl=keys->dql=0;
#endif
        return(0);
}
