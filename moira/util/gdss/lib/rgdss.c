/*
 * $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/util/gdss/lib/rgdss.c,v $
 * $Author: danw $
 * $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/util/gdss/lib/rgdss.c,v 1.3 1998-08-10 17:54:58 danw Exp $
 */
/*
 * GDSS The Generic Digital Signature Service
 *
 * rgdss.c: Raw signature signing and verification routines.
 */

#include <BigNum.h>
#include <BigRSA.h>
#include <krb.h>
#include <gdss.h>
#include <stdio.h>
#include <time.h>

int gdss_rsign(signature, hash, name, instance, realm, key)
unsigned char *signature;
unsigned char *hash;
char *name;
char *instance;
char *realm;
RSAKeyStorage *key;
{
  unsigned char *cp, *ip;
  time_t the_time;
  register int i;
  register int status;
  int loopcnt;
  int siglen;

  for (loopcnt = 0; loopcnt < 10; loopcnt++) {
    cp = signature;
    for (i = 0; i < 16; i++)
      *cp++ = hash[i];
    *cp++ = 0x44;   /* Version Number */
    ip = (unsigned char *) name;
    while (*cp++ = *ip++);
    ip = (unsigned char *) instance;
    while (*cp++ = *ip++);
    ip = (unsigned char *) realm;
    while (*cp++ = *ip++);
    time(&the_time);
    *cp++ = ((the_time) >> 24) & 0xff;
    *cp++ = ((the_time) >> 16) & 0xff;
    *cp++ = ((the_time) >> 8) & 0xff;
    *cp++ = the_time & 0xff;
    if(!RSASign(signature, cp - signature, key, &signature[cp - signature],
		&siglen)) return (-1);
    status = gdss_rpadout(&signature[16], cp - signature + siglen - 16);
    if ((status == GDSS_SUCCESS) || (status != GDSS_E_PADTOOMANY)) {
      ip = &signature[16];
      cp = signature;
      while (*cp++ = *ip++);	/* shuffle over hash */
      return(GDSS_SUCCESS);
    }
    sleep(1);			/* Allow time to change */
  }
  return (GDSS_E_PADTOOMANY);
}

/* gdss_rpadout: Remove null bytes from signature by replacing them with
   the sequence GDSS_ESCAPE, GDSS_NULL. Keep track of how much bigger
   the signature block is getting and abort if too many bytes (more than
   GDSS_PAD) would be required.
*/

int gdss_rpadout(signature, siglen)
unsigned char *signature;
int siglen;
{
  register unsigned char *cp;
  register unsigned char *bp;
  unsigned char *buf;
  register int i;
  register int c;
  buf = (unsigned char *)malloc(siglen + GDSS_PAD + 1); /* 1 for the null! */
  if (buf == NULL) return (GDSS_E_ALLOC);
  memset(buf, 0, siglen + GDSS_PAD + 1); /* Just to be safe */
  bp = buf;
  cp = signature;
  c = 0;
  for (i = 0; i < siglen; i++) {
    if ((*cp != '\0') && (*cp != GDSS_ESCAPE)) {
      *bp++ = *cp++;
      continue;
    }
    if (c++ > GDSS_PAD) {
      free(buf);		/* Don't have to zeroize, nothing
				   confidential */
      return (GDSS_E_PADTOOMANY);
    }
    *bp++ = GDSS_ESCAPE;
    *bp++ = (*cp == '\0') ? GDSS_NULL : GDSS_ESCAPE;
    cp++;
  }
  *bp++ = '\0';			/* Null Terminate */
  memcpy(signature, buf, bp - buf);
  free(buf);
  return (GDSS_SUCCESS);
}
  
int gdss_rpadin(signature, outlen)
unsigned char *signature;
int *outlen;
{
  unsigned char *buf;
  register unsigned char *cp;
  register unsigned char *bp;
  buf = (unsigned char *) malloc(strlen(signature));
  if (buf == NULL) return (GDSS_E_ALLOC);
  bp = buf;
  cp = signature;
  while (*cp) {
    if (*cp != GDSS_ESCAPE) {
      *bp++ = *cp++;
      continue;
    }
    if (*(++cp) == GDSS_NULL) {
      *bp++ = '\0';
    } else *bp++ = GDSS_ESCAPE;
    if(!*cp++) break;
  }
  *outlen = bp - buf;
  memcpy(signature, buf, *outlen);
  free (buf);
  return (GDSS_SUCCESS);
}

int gdss_rverify(isignature, hash, name, instance,
		 realm, key, the_time, rawsig)
unsigned char *isignature;
unsigned char *hash;
char *name;
char *instance;
char *realm;
RSAKeyStorage *key;
unsigned int *the_time;
unsigned char *rawsig;
{
  unsigned char *cp;
  char *ip;
  register int i;
  int status;
  int siglen;
  unsigned char *signature;

  if (*isignature != 0x44) return (GDSS_E_BVERSION); /* Bad Version */

  signature = (unsigned char *) malloc (strlen(isignature) + 17);
  	/* Length of input signature + null byte + 16 bytes of hash */
  strcpy(&signature[16], isignature);

  status = gdss_rpadin(&signature[16], &siglen);
  if (status) return (status);
  
  siglen += 16;			/* Account for the hash */
  cp = signature;
  for (i = 0; i < 16; i++)
    *cp++ = hash[i];
  if (*cp++ != 0x44) return (GDSS_E_BVERSION); /* Bad Version */
  ip = name;
  while ((*ip++ = *cp++) && (ip < name + ANAME_SZ));
  ip = instance;
  while ((*ip++ = *cp++) && (ip < instance + INST_SZ));
  ip = realm;
  while ((*ip++ = *cp++) && (ip < realm + REALM_SZ));
  *the_time = 0;
  *the_time |= *cp++ << 24;
  *the_time |= *cp++ << 16;
  *the_time |= *cp++ << 8;
  *the_time |= *cp++;
  if(!RSAVerify(signature, cp - signature, key, &signature[cp - signature],
		siglen - (cp - signature))) {
    free (signature);
    return (GDSS_E_BADSIG);
  }
  if (rawsig == NULL) {
    free (signature);
    return (GDSS_SUCCESS);
  }
  memcpy(rawsig, &signature[cp - signature], siglen - (cp - signature));
  status = gdss_rpadout(rawsig, siglen - (cp - signature));
  free (signature);
  return (status);
}
    
gdss_recompose(aSigInfo, signature)
SigInfo *aSigInfo;
unsigned char *signature;
{
  register unsigned char *ip;
  register unsigned char *cp;
  unsigned char *isignature;
  int siglen;
  int status;

  isignature = (unsigned char *) malloc(strlen(aSigInfo->rawsig) + 1);
  if (isignature == NULL) return (GDSS_E_ALLOC);
  strcpy(isignature, aSigInfo->rawsig);
  status = gdss_rpadin(isignature, &siglen);
  if (status) {
    free(isignature);
    return (status);
  }

  cp = signature;
  *cp++ = 0x44;			/* Version */
  ip = (unsigned char *) aSigInfo->pname;
  while (*cp++ = *ip++);
  ip = (unsigned char *) aSigInfo->pinst;
  while (*cp++ = *ip++);
  ip = (unsigned char *) aSigInfo->prealm;
  while (*cp++ = *ip++);
  *cp++ = ((aSigInfo->timestamp) >> 24) & 0xff;
  *cp++ = ((aSigInfo->timestamp) >> 16) & 0xff;
  *cp++ = ((aSigInfo->timestamp) >> 8) & 0xff;
  *cp++ = aSigInfo->timestamp & 0xff;
  memcpy(cp, isignature, siglen);
  free(isignature);
  return(gdss_rpadout(signature, cp - signature + siglen));
}
