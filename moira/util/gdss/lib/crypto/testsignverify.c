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
#include <ctype.h>
#include <sys/types.h>
#include <time.h>

#include "bigsignverify.h"

#define MAX_NAME 80
#define MAX_UID 80
#define MAX_KEY 2048
#define MAX_HASH 16

#ifdef DEBUG
#undef DEBUG
#endif

RSAKeyStorage keys ;
RSAKeyStorage public_key ;
RSAKeyStorage private_key ;
unsigned char bigbuf [2*DigitLim*sizeof(BigNumDigit)];
unsigned char testdata[]="Now is the time for all good men to come to the aid";

main(argc,argv)
int     argc;
char    **argv;
{
    char usernameBuf [50], uidBuf [50], x500NameBuf[50], hashkey[50];
    int uid_len;
    FILE *publ, *priv;
    DESblock newkey1, newkey2;
    int bigbuflen;
    time_t expires1, expires2;

    if (argc < 2)
    {
badargs:
        printf("usage: %s name \n", argv[0]);
        exit(1);
    }

    strcpy(usernameBuf, argv[1]);

    memset(&keys,0,sizeof(keys));
    
    if (read_privkey (usernameBuf, x500NameBuf, uidBuf, &uid_len, hashkey, &private_key)) {
        printf("\nPrivate key read.");
        PrintTestKey(&private_key);
    }
    else {
        printf("\nUnable to read private key for %s.\n", usernameBuf);
        exit(0);
        }
        
    if (read_pubkey (usernameBuf, x500NameBuf, uidBuf, &uid_len, &public_key)){
        printf("\nPublic key read.");
        PrintTestKey(&public_key);
    }
    else {
        printf("\nUnable to read public key for %s.\n", usernameBuf);
        exit(0);
        }

    if(!RSASign(testdata, sizeof(testdata), &private_key, bigbuf, &bigbuflen)) {
        printf("\nError signing test data.\n");
        exit(0);
    }

    printf("\nSigned Data:\n");
    dumphex(bigbuf, bigbuflen);
    
    if(!RSAVerify(testdata, sizeof(testdata), &public_key, bigbuf, bigbuflen)) {
        printf("\nError verifying signature.\n");
        exit(0);
    }
    printf("\nSignature verifies.\n");

    time(&expires1);
    printf("\nCurrent time: %s", ctime(&expires1));
    expires1 += (time_t) 60; /* one minute */
    
    InitAuthenticationKey ( &private_key, &public_key, &newkey1, bigbuf, 
                                &bigbuflen, expires1);
    
    printf("\nNew generated key:\n");
    dumphex(&newkey1, sizeof(DESblock));
    printf("\nExpires %s", ctime(&expires1));
    printf("\nEncrypted key (length %d):\n", bigbuflen);
    dumphex(bigbuf, bigbuflen);

    if (AcceptAuthenticationKey (&private_key, &newkey2, bigbuf, bigbuflen, &expires2)) {
        printf("\nRecovered key:\n");
        dumphex(&newkey1, sizeof(DESblock));
        printf("\nExpires %s", ctime(&expires2));
    }
    else {
        printf("\nRecovery of key failed.\n");
    }

exit(0);
}

