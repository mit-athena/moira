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

#include "BigNum.h"
#include "BigRSA.h"
#include "random.h"
#include "hashes.h"
#include "read_password.h"
#include "bigkeygen.h"
#include "bigrsacode.h"

#define MAX_NAME 80
#define MAX_UID 80
#define MAX_KEY 2048
#define MAX_HASH 16

#ifdef DEBUG
#undef DEBUG
#endif

static RSAKeyStorage keys ;
static RSAKeyStorage public_key ;
static RSAKeyStorage private_key ;

main(argc,argv)
int     argc;
char    **argv;
{
    unsigned char *encodedP ;
    char usernameBuf [50], uidBuf [50], x500NameBuf[50], hashkey[50];
    int uid_len;
    FILE *publ, *priv;

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
        printf("\nPrivate key read failed.\n");
        exit(0);
        }
        
    if (read_pubkey (usernameBuf, x500NameBuf, uidBuf, &uid_len, &public_key)){
        printf("\nPublic key read.");
        PrintTestKey(&public_key);
    }

    if ((encodedP=EncodePrivateP(&private_key))==0) {
        printf("\nEncode private key failed.\n");
        exit(0);
        }

    printf("\nEncoded private key (prime P only):\n");
    dumphex(encodedP,DecodeTotalLength(encodedP));

    printf("\nDecoding...\n");
    
    if ((DecodePrivate(encodedP, &public_key))==0) {
        printf("\nDecode of private key failed.\n");
        exit(0);
        }
    else {
        printf("\nRecovered Private Key:\n");
        PrintTestKey (&public_key);
        }

BnnClose();
exit(0);
}




int read_privkey (filename,name,uid,uid_len,hashkey,key)
char *filename, *name, *hashkey;
unsigned char *uid;
int *uid_len;
RSAKeyStorage *key;
{
        static unsigned char buffer [MAX_KEY];
        DESblock pwkey ;
        char *ptr;
        unsigned char *uptr;
        int i,j,c;
        FILE *fp;
        char tempname[80];

        strcpy(tempname,filename);
        strcat(tempname,"_privkey");

        if((fp=fopen(tempname,"r"))==NULL) {
        	printf("\nCan't open file %s.\n", tempname);
        	return(0);
        }

        ptr=name;
        for(i=0,j=0;i<MAX_NAME;i++)
        switch (*ptr++ =getc(fp)){
           case '{': j++;break;
           case '}': j--;if(j==0)goto next;break;
           case EOF : {
		printf("\nUnexpected end of file %s.\n",tempname);
		return(0);
	   }
        }	
        next:
        if(i>=MAX_NAME-1) {
        	printf("\nIssuer name too long.\n");
        	return(0);
        }
        *ptr='\0';

#ifdef DEBUG
printf("\nissuer name= %s",name);
#endif

        uptr=uid;
        for(i=0;i<MAX_UID;i++)
        	if(fscanf(fp,"%2x",&j)==1) *uptr++ =j; else break;
        if(i==MAX_UID)return(0);

        *uid_len = i;
        
#ifdef DEBUG
printf("\nuid=");
dumphex(uid,i);
#endif

        while(getc(fp)!=';');
                for(i=0;i<MAX_HASH;i++)
                	if(fscanf(fp,"%2x",&j)==1) hashkey[i]=j ;
                	else break;
        if(i==MAX_HASH) return(0);
#ifdef DEBUG
printf("\nSize of hash: %d\n", i);
dumphex(hashkey,i);
#endif

        while(getc(fp)!=';');
                for(i=0;i<MAX_KEY;i++)
                	if(fscanf(fp,"%2x",&j)==1) buffer[i]=j ;
                	else break;
        if(i==MAX_KEY) return(0);
#ifdef DEBUG
printf("\nSize of private key read: %d\n", i);
dumphex(buffer,i);
#endif

	if (DES_read_password(&pwkey, "\nEnter Password: ", 0) == 0) {
            printf("\nError entering password.\n");
            return(0);
        }

        memset(key,0,sizeof(*key));
        if (recover_private(&pwkey,buffer,i,key)==0) {
               printf("\nError recovering key.\n");
               return(0);
        }
        
#ifdef DEBUG
printf("\nRecovered Key: \n");
PrintTestKey(key);
#endif


return(1);
}


