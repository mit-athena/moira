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
#include <time.h>
#include <syslog.h>
#include "bigsignverify.h"

#ifdef DEBUG
#undef DEBUG
#endif

/* in-line crypto routines here */

#include "RSAcrypto.h"
#include "DEScrypto.h"

#define CLOCK_SKEW  5*60

#ifdef mips
#undef BnnSetToZero
#endif

/*
 * Global
 */
 
static BigNumDigit buffer1[2*DigitLim], buffer2[2*DigitLim];

/*
 * RSASign takes a key in local format and a counted character string and
 * writes a signature in "BER INTEGER" encoded format into the supplied buffer.
 * The length in bytes is also written.
 *
 */

int RSASign (toBeSigned, datalen, key, signature, siglen)
RSAKeyStorage *key;
unsigned char *toBeSigned;
unsigned char *signature;       /* output assumed large enough to hold modulus */
int datalen, *siglen;           /* in bytes */
{
        char *temp = (char *)buffer2;
        int i, nl=key->nl, pl=key->pl, ql=key->ql;
        BigNum signed_digest = buffer1, p = key->p, q = key->q;

        if ((pl==0)||(ql==0)) return(0);        /* not a private key */
        RSA_MD2(toBeSigned, datalen, temp);     /* hash to temp */
        BnnSetToZero(signed_digest,nl);
        for (i=0;i<16;i++)                      /* copy in right order */
          signed_digest[i/sizeof(BigNumDigit)]|=
                ((0x0ff&temp[15-i])<<((i%sizeof(BigNumDigit))*8));
	signed_digest[4] |= 0x00000410;
        rsadecrypt_1(signed_digest, signed_digest, key);
        *siglen = NumEncodeValueOctets(signed_digest, nl);
        if(EncodeBigIntegerValue(signature, signed_digest, nl)) return(TRUE);
        else return(FALSE);
}

/*
 * RSAVerify takes an encoded signature, allegedly signed data and key
 * and yields a boolean (TRUE/FALSE).
 */

int RSAVerify (allegedSigned, datalen, key, signature, siglen)
RSAKeyStorage *key;
char *allegedSigned, *signature;
int datalen, siglen;
{
        char *temp=(char *)buffer2;
        BigNum md = buffer1, buffer = buffer2, n = key->n;
        int i, bl, nl=key->nl;

        if ((bl=(siglen+sizeof(BigNumDigit)-1)/sizeof(BigNumDigit)) > nl) {
#ifdef DEBUG        
printf("\nRSAVerify: signature has more BigNumDigits than the modulus\n");
#endif
                return(FALSE);
        }

        BnnSetToZero(md,nl);
        RSA_MD2(allegedSigned, datalen, temp);
        for (i=0;i<16;i++)                      /* copy in right order */
          md[i/sizeof(BigNumDigit)]|=((0x0ff&temp[15-i])<<((i%sizeof(BigNumDigit))*8));

        BnnSetToZero(buffer,nl);
        DecodeBigInteger(signature, buffer, siglen);
        if (BnnCompare(n, nl, buffer, bl)!=1) {
#ifdef DEBUG
printf("\nRSAVerify: signature is larger BigNum value than modulus\n");
#endif
                return(FALSE);
        }

        rsaencrypt_1(buffer, buffer, key);
#ifdef DEBUG
printf("RSAVerify: computed md:\n");
dumphex(md,nl*8);
printf("RSAVerify: recovered md:\n");
dumphex(buffer,nl*8);
#endif
	buffer[4] &= 0xffff0000;
        if (BnnCompare(md,nl,buffer,nl)==0)  return (TRUE);
#ifdef DEBUG
printf("\nRSAVerify: signature verification failed.\n");
#endif
        return(FALSE);

}


/*
 * The initialization routine generates a DES key and encrypts it under
 * the public key of the intended verifier principal.
 *
 * A word about byte ordering.  The "least significant byte" of a BigNum
 * is the byte that has the least significant bit in it.  The "most
 * significant byte" has the most significant bit in it.
 *
 */
#include "endian.h"
#define RandomOffset (sizeof(DESblock)+2*sizeof(time_t))
#define BYTE_IN_BIGNUM(x) (SPHINX_ENDIAN?(sizeof(BigNumDigit)-(x%sizeof(BigNumDigit)-1)):x)

int InitAuthenticationKey (verifier, delegation, new_key, encrypted_key, 
                                encrypted_key_len, when_expires)
RSAKeyStorage *verifier, *delegation;
DESblock *new_key;
unsigned char *encrypted_key;
int *encrypted_key_len;
time_t when_expires;
{
        time_t when_signed;
        BigNum vn=verifier->n, p = buffer1, temp = buffer1;
        int i, leading_zero_bytes, vnl=verifier->nl, tl;
        RNGState rng;
        
        /*
         * This is all we need the delegation key for.
         */
        read_rng_state(&rng);
        if(((rng.count)==0)&&delegation) initialize_rng_state(delegation->p,16);

        /*
         * Fill temporary storage with a random number.
         */
        BnnSetToZero(temp,vnl+1);
        tl = BnnNumDigits(vn,vnl);
        random_BigNum(temp,tl);

        /* 
         * There must always be one more significant byte in the modulus than 
         * the one that has the "64" in the encrypted key.
         */
#ifdef DEBUG
printf("\nleading zero bits: %d, vn[tl-1]=%08x\n",
                BnnNumLeadingZeroBitsInDigit(vn[tl-1]), vn[tl-1]);
#endif
        leading_zero_bytes=BnnNumLeadingZeroBitsInDigit(BnnGetDigit(vn+tl-1))/8;
        if (leading_zero_bytes == (sizeof(BigNumDigit)-1)) { 
        /* 
         * There is only one significant byte in the most significant BigNumDigit,
         * so zero out top digit and put "64" into the most significant byte of 
         * the next-to-top digit 
         */
                temp[tl-1] = 0;
                temp[tl-2] &= ~(0x00ff << 8*leading_zero_bytes); 
                temp[tl-2] |= 64 << 8*leading_zero_bytes ;
                }
        else { 
        /* 
         * Zero out bytes with significant stuff in them, and put "64"
         * in the next-most-significant byte.
         */
                for(i=0;i<=leading_zero_bytes+1;i++)
                        temp[tl-1] &= ~(0x00ff << (8*(sizeof(BigNumDigit)-1-i)));
                temp[tl-1] |= 64<<((sizeof(BigNumDigit)-2-leading_zero_bytes)*8);
        }
#ifdef DEBUG
printf("\nleading_zero_bytes=%d\n",leading_zero_bytes);
printf("\ntl=%d, vnl=%d, temp[tl-1]=%08x, temp[tl-2]=%08x\n", tl, vnl, temp[tl-1], temp[tl-2]);
#endif

        BnnSetToZero(temp, (RandomOffset+4)/sizeof(BigNumDigit));
        /*
         * Generate 8 bytes of DES key, copy into buffer in right order
         */
        random_bytes(temp, sizeof(DESblock));
        for (i=0;i<sizeof(DESblock);i++) new_key->bytes[sizeof(DESblock)-i-1] =
           (temp[i/sizeof(BigNumDigit)]>>((i%sizeof(BigNumDigit)*8)));

        /*
         * Assume sizeof(time_t) == sizeof(BigNumDigit).
         * Everyone has to swap these bytes, I guess.
         */
        time(&when_signed);
        p = &temp[sizeof(DESblock)/sizeof(BigNumDigit)];
        p[0]=when_signed;
        p[1]=when_expires;
/*
        for(i=0;i<sizeof(time_t);i++) {
             p[0] |= ((when_signed>>(i*8))&(BigNumDigit)0xff)<<((sizeof(time_t)-1-i)*8);
             p[1] |= ((when_expires>>(i*8))&(BigNumDigit)0xff)<<((sizeof(time_t)-1-i)*8);
             }
*/
#ifdef DEBUG
printf("\nBlock to encrypt: (low to high order bytes) \n");
dumphex(temp,tl*sizeof(BigNumDigit));
printf("\nModulus:\n");
dumphex(vn,vnl*sizeof(BigNumDigit));
#endif
        rsaencrypt_1(temp,temp,verifier);
        *encrypted_key_len = NumEncodeValueOctets(temp,tl);
        if (EncodeBigIntegerValue(encrypted_key,temp,tl)) return(TRUE);
        else return(FALSE);
}

/*
 * The accept routine takes in encoded elements and yields the transferred
 * DES key along with an expiration date.
 *
 */
int AcceptAuthenticationKey (verifier, new_key, encrypted_key, 
                                encrypted_key_len, when_expires)
RSAKeyStorage *verifier;
DESblock *new_key;
unsigned char *encrypted_key;
int encrypted_key_len;
time_t *when_expires;
{
        time_t now, when_signed, when_expires_temp ;
        int i, tl, vnl=verifier->nl, leading_zero_bytes;
        BigNum vn=verifier->n, p = buffer1, temp = buffer1;
        char *ptr;

        BnnSetToZero(temp,vnl+1);
        if ((tl=(encrypted_key_len+sizeof(BigNumDigit)-1)/sizeof(BigNumDigit)) > vnl) {
#ifdef DEBUG
printf("\nAcceptAuthenticationKey: Signature has more BigNumDigits than modulus.\n");
#endif
syslog(LOG_INFO, "AAK : Signature has more BigNumDigits than modulus");
                goto error;
        }
        
        DecodeBigInteger(encrypted_key,temp,encrypted_key_len);
        if (BnnCompare(vn,vnl,temp,tl) != 1) {
#ifdef DEBUG
printf("\nAcceptAuthenticationKey: Signature is larger than modulus.\n");
#endif
syslog(LOG_INFO, "AAK : Signature is larger than modulus");
                goto error;
        }
       
        rsadecrypt_1(temp,temp,verifier);
#ifdef DEBUG
printf("\nAcceptAuthenticationKey: Decrypted key:\n");
dumphex(temp,(vnl+1)*sizeof(BigNumDigit));
fflush(stdout);
#endif
        /* 
         * Check for a "64" in the right place.
         */
        tl = BnnNumDigits(vn,vnl);
        leading_zero_bytes=BnnNumLeadingZeroBitsInDigit(BnnGetDigit(vn+tl-1))/8;
        if (leading_zero_bytes == sizeof(BigNumDigit)-1) { 
                if ((temp[tl-1] != 0) || 
                        (((temp[tl-2]>>(8*leading_zero_bytes))&0x0ff) != 64)) {
#ifdef DEBUG
printf("\nTop digit not zero, or 64 byte test failed.\n");
#endif
syslog(LOG_INFO, "AAK : Top digit not zero, or 64 byte test failed");
                                goto error ;
                        }
                }
        else {  for(i=0;i<leading_zero_bytes;i++)
                 if(((temp[tl-1]>>((sizeof(BigNumDigit)-leading_zero_bytes-1+i)*8))&0x0ff)
                                != 0) {
#ifdef DEBUG
printf("\nTop byte zero test failed.\n");
#endif
syslog(LOG_INFO, "AAK : Top byte zero test failed");
                                goto error ;
                }
                if (((temp[tl-1]>>((sizeof(BigNumDigit)-2-leading_zero_bytes)*8))&0xff)
                                != 64){
#ifdef DEBUG
printf("\nTop byte 64 test failed.\n");
#endif
syslog(LOG_INFO, "AAK : Top byte 64 test failed");
                                goto error ;
                }
        }

        ptr = (char *) &temp[RandomOffset/sizeof(BigNumDigit)];
        for (i=0;i<4;i++) if (*ptr++ !=0) {
#ifdef DEBUG
printf("\nAcceptAuthenticationKey: Zero mid-buffer error.\n");
#endif
syslog(LOG_INFO, "AAK : Zero mid-buffer error");
                goto error;
        }

        when_signed=when_expires_temp=0;

        p = &temp[sizeof(DESblock)/sizeof(BigNumDigit)];
        when_signed = p[0];
        when_expires_temp = p[1];
/*
        for(i=0;i<sizeof(time_t);i++) {
             when_signed |= ((p[0]>>(i*8))&(BigNumDigit)0x0ff)<<((sizeof(time_t)-1-i)*8);
             when_expires_temp |= ((p[1]>>(i*8))&(BigNumDigit)0x0ff)<<((sizeof(time_t)-1-i)*8);
             }
*/
        time(&now);
#ifdef DEBUG
printf("\n Signed:  %s", ctime(&when_signed));
printf("\n Expires: %s\n", ctime(&when_expires_temp));
#endif

        if((when_signed > now + CLOCK_SKEW) || (when_expires_temp < now - CLOCK_SKEW)) {
#ifdef DEBUG
printf("\nAcceptAuthenticationKey: Key Has Expired.\n");
#endif
syslog(LOG_INFO, "AAK : signed on (%d) %s", when_signed, ctime(&when_signed));
syslog(LOG_INFO, "AAK : expire on (%d) %s", when_expires_temp, ctime(&when_expires_temp));
syslog(LOG_INFO, "AAK : key has expired - now is %d", now);
               goto error;
        }
        *when_expires = when_expires_temp;

        for (i=0;i<sizeof(DESblock);i++) new_key->bytes[sizeof(DESblock)-i-1] =
           (temp[i/sizeof(BigNumDigit)]>>((i%sizeof(BigNumDigit)*8)));
        return(TRUE);

error:
        return(FALSE);
        
}

