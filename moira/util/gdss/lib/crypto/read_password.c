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

#include "hashes.h"
#include <stdio.h>

int MIN_PASSWORD_LENGTH = 6 ;


#define TEMP_BUFSIZ 256

static unsigned char scramble_key [8] = { 0x01, 0x23, 0x45, 0x67, 
		0x89, 0xab, 0xcd, 0xef };

char *getpassword();


/*
 * Password hashing routine number 1.  This is stored with the encrypted
 * private key in the LEAF database.  Result is an 8 byte quantity.
 */

int H1(username, pw, hash)
char *username, *pw, *hash ;
{
    char temp[TEMP_BUFSIZ];
    char md2_hash [16];

    temp[0] = '\0';
    
    if (2 + (username?strlen(username):0) + strlen(pw) > sizeof(temp)) return(0);
    
    if (username) strcat(temp,username);
    strcat(temp,pw);

    RSA_MD2 (temp, strlen(temp), md2_hash);
    memcpy(hash, md2_hash, 8);

    memset(temp,0,sizeof(temp));    
    memset(md2_hash,0,sizeof(md2_hash));    

    return(1);
}



/*
 * Password hashing routine number 2.  This is the key used to encrypt 
 * the private key.
 */

int H2(username, pw, hash)
char *username, *pw, *hash ;
{
    char temp[TEMP_BUFSIZ];

    if (2 + (username?strlen(username):0) + strlen(pw) > sizeof(temp)) return(0);
    
    temp[0] = '\0';
    if (username) strcat(temp,username);
    strcat(temp,pw);
    
    DES_X9_MAC (scramble_key, temp, strlen(temp), hash);

    memset(temp,0,sizeof(temp));    

    return(1);
}


/*
 * Read password.  Returns a DES key.
 */

int DES_read_password(k,prompt,verify)
char *prompt, *k;
int verify;                             /* non-zero means prompt twice for password */
{
    char *pw = getpassword(prompt);
    char *env = NULL;
    int ret = 0;

    if ((verify) && (strlen(pw) < MIN_PASSWORD_LENGTH)) {
        printf("Length error, (must be at least %d char) please re-enter: ", MIN_PASSWORD_LENGTH);
        fflush(stdout);
        pw = getpassword("");
        if (strlen(pw) < MIN_PASSWORD_LENGTH) {
            printf("Password length error. \n");
            goto cleanup;
        }
    }        

    if (verify) {
        char pwcpy[80];
        strcpy(pwcpy,pw);
        printf("Verifying, please re-enter: ");
        fflush(stdout);
        pw = getpassword("");
        if (verify = strcmp(pwcpy,pw)) {
                printf("\nVerification Error\n");
                memset(pwcpy,0,strlen(pwcpy));
                goto cleanup;
                }
        memset(pwcpy,0,strlen(pwcpy));
    }

done:
    ret = H2(0,pw,k);

cleanup:
    memset(pw,0,strlen(pw));
    return(ret);
}


int DES_read_password_hash(H2hash,H1hash,username,prompt,verify)
char *prompt, *H2hash, *username, *H1hash;
int verify;                             /* non-zero means prompt twice for password */
{

    char *pw = getpassword(prompt);
    char *env = NULL;
    int ret = 0;

    if ((verify) && (strlen(pw) < MIN_PASSWORD_LENGTH)) {
        printf("Length error, (must be at least %d char) please re-enter: ", MIN_PASSWORD_LENGTH);
        fflush(stdout);
        pw = getpassword("");
        if (strlen(pw) < MIN_PASSWORD_LENGTH) {
            printf("Password length error. \n");
            goto cleanup;
        }
    }        

    if (verify) {
        char pwcpy[80];
        strcpy(pwcpy,pw);
        printf("Verifying, please re-enter: ");
        fflush(stdout);
        pw = getpassword("");
        if (verify = strcmp(pwcpy,pw)) {
                printf("\nVerification Error\n");
                memset(pwcpy,0,strlen(pwcpy));
                goto cleanup;
                }
        memset(pwcpy,0,strlen(pwcpy));
    }

done:
    H1(username,pw,H1hash);
    H2(0,pw,H2hash);
    ret = 1;

cleanup:
    memset(pw,0,strlen(pw));
    return(ret);
}

