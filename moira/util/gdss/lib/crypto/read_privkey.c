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

#define MAX_NAME 80
#define MAX_UID 80
#define MAX_KEY 2048
#define MAX_HASH 8

#ifdef DEBUG
#undef DEBUG
#endif

int read_privkey_messages =0;
RSAKeyStorage RSAKeys ;

int read_privkey_verbose(filename,name,uid,uid_len,hashkey,key)
char *filename, *name, *hashkey;
unsigned char *uid;
int *uid_len;
RSAKeyStorage *key;
{
int save = read_privkey_messages ,x;
  read_privkey_messages = 1;
  x=read_privkey (filename,name,uid,uid_len,hashkey,key);
  read_privkey_messages = save;
  return(x);
}

int read_privkey(filename,name,uid,uid_len,hashkey,key)
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
        char tempname[80], prompt[80];

        strcpy(tempname,filename);
        strcat(tempname,"_privkey");

        if((fp=fopen(tempname,"r"))==NULL) {
                if (read_privkey_messages) 
                        printf("\n%s: Can't open file %s.\n", __FILE__,tempname);
        	return(0);
        }

        ptr=name;
        for(i=0,j=0;i<MAX_NAME;i++) {
        if((c=getc(fp))==EOF) {
                if (read_privkey_messages) 
                        printf("\n%s: Unexpected end of file %s.\n",__FILE__,tempname);
		return(0);
            }
        switch (*ptr++ = (char) c){
           case '{': j++; 
                     break;
           case '}': j--;
                     if(j==0) goto next;
                     break;
           case '\n': if(j==0) {ptr--; goto next;}
                      break;
	   }
        }	
        next:
        if(i>=MAX_NAME-1) {
                if (read_privkey_messages) 
                        printf("\n%s: Issuer name too long.\n", __FILE__);
        	return(0);
        }
        *ptr='\0';

#ifdef DEBUG
printf("\n%s: issuer name= %s",__FILE__,name);
#endif

        uptr=uid;
        for(i=0;i<MAX_UID;i++)
        	if(fscanf(fp,"%2x",&j)==1) *uptr++ =j; else break;
        if(i==MAX_UID)return(0);

        *uid_len = i;
        
#ifdef DEBUG
printf("\n%s: uid is ",__FILE__);
dumphex(uid,i);
#endif

        while(getc(fp)!=';');
        for(i=0;i<MAX_HASH;i++)
              	if(fscanf(fp,"%2x",&j)==1) hashkey[i]=j ;
               	else break;
#ifdef DEBUG
printf("\n%s: Size of hash: %d\n", __FILE__,i);
dumphex(hashkey,i);
#endif
        
        /* next non-white must be a ';' */
        do {j=getc(fp);}while(isspace(j));
        if(j != ';'){
                if(read_privkey_messages) printf("\n%s: Hash Overflow.\n",__FILE__);
                return(0);
        }

        for(i=0;i<MAX_KEY;i++)
              	if(fscanf(fp,"%2x",&j)==1) buffer[i]=j ;
               	else break;
        if(i==MAX_KEY) return(0);
#ifdef DEBUG
printf("\n%s: Size of private key read: %d\n", __FILE__,i);
dumphex(buffer,i);
#endif
	sprintf(prompt,"\nEnter %s's password: ", filename);
	if (DES_read_password(&pwkey, prompt, 0) == 0) {
            if (read_privkey_messages) printf("\n%s, Error entering password.\n",__FILE__);
            return(0);
        }

        memset(key,0,sizeof(*key));
        if (recover_private(&pwkey,buffer,i,key)==0) {
               if (read_privkey_messages) printf("\nError recovering key.\n");
               return(0);
        }
        
#ifdef DEBUG
printf("\n%s: Recovered Key: \n",__FILE__);
PrintTestKey(key);
#endif

return(1);
}


