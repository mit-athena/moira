/*
 * $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/util/gdss/lib/gdss.c,v $
 * $Author: danw $
 * $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/util/gdss/lib/gdss.c,v 1.1 1997-07-10 23:56:18 danw Exp $
 */
/*
 * GDSS The Generic Digital Signature Service
 *
 * gdss.c: Main interface routines
 */

#include <BigNum.h>
#include <BigRSA.h>
#include <krb.h>
#include <gdss.h>
#include <stdio.h>

static RSAKeyStorage gdss_pubkey;
static int gdss_have_key;
static int pfetchkey();

#ifdef notdef
/* This function is obsolete */
int GDSS_Sig_Info(Signature, aSigInfo)
unsigned char *Signature;
SigInfo *aSigInfo;
{
  int status;
  static int pfetchkey();
  unsigned char hash[16];
  unsigned char *cp;

  cp = aSigInfo->rawsig;
  memset(aSigInfo, 0, sizeof(SigInfo));
  aSigInfo->rawsig = cp;
  aSigInfo->SigInfoVersion = 0;
  do {
    status = pfetchkey();
    if (status) break;
    status = gdss_rverify(Signature, hash, &(*aSigInfo).pname,
			  &(*aSigInfo).pinst, &(*aSigInfo).prealm,
			  &gdss_pubkey, &(*aSigInfo).timestamp,
			  aSigInfo->rawsig);
  } while(0);
  return (status);
}
#endif

int GDSS_Verify(Data, DataLen, Signature, aSigInfo)
unsigned char *Data;
unsigned int DataLen;
unsigned char *Signature;
SigInfo *aSigInfo;
{
  unsigned char hash[16];
  SigInfo bSigInfo, *iSigInfo;
  int status;
  unsigned char *cp;

  if (aSigInfo == NULL) {
    iSigInfo = &bSigInfo;
  } else {
    iSigInfo = aSigInfo;
  }
  status = pfetchkey();
  if (status) return (status);

  memset(&bSigInfo, 0, sizeof(bSigInfo));
  cp = iSigInfo->rawsig;
  memset(iSigInfo, 0, sizeof(bSigInfo));
  iSigInfo->rawsig = cp;

  RSA_MD2(Data, DataLen, hash);

  status = gdss_rverify(Signature, hash, iSigInfo->pname,
			iSigInfo->pinst, iSigInfo->prealm,
			&gdss_pubkey, &iSigInfo->timestamp, iSigInfo->rawsig);

  if (status) return (status);
  return (GDSS_SUCCESS);
}

int GDSS_Sig_Size()
{
  int status;
  int retval;
  status = pfetchkey();
  if (status) retval = 512;	/* No provision for errors, so default value */
  retval = sizeof(SigInfo) + (gdss_pubkey.nl)*4 + GDSS_PAD + 5;
  return (retval);
}

static int pfetchkey()
{
  FILE *keyf;
  unsigned char buffer[512];

  if (gdss_have_key) return (0);
  keyf = fopen("/etc/athena/gdss_public_key", "r");
  if (keyf == NULL) {
      keyf = fopen("/afs/net.mit.edu/system/config/gdss_public_key", "r");
      if (keyf == NULL) return (GDSS_E_NOPUBKEY);
  }
  fread(buffer, 1, 512, keyf);
  fclose(keyf);
  DecodePublic(buffer, &gdss_pubkey);
  gdss_have_key++;
  return (GDSS_SUCCESS);
}

GDSS_Recompose(aSigInfo, signature)
SigInfo *aSigInfo;
unsigned char *signature;
{
  if (aSigInfo->rawsig == NULL) return (GDSS_E_BADINPUT);
  return(gdss_recompose(aSigInfo, signature));
}


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <hesiod.h>

static struct timeval timeout = { CLIENT_KRB_TIMEOUT, 0 };
GDSS_Sign(Data, DataLen, Signature)
unsigned char *Data;
unsigned int DataLen;
unsigned char *Signature;
{
  KTEXT_ST authent;
  char lrealm[REALM_SZ];
  char linst[INST_SZ];
  char sinst[INST_SZ];
  char *cp;
  unsigned char hash[16];
  unsigned char dhash[16];	/* Second level hash */
  int s;			/* Socket to do i/o on */
  int status;
  register int i;
  unsigned int cksum;
  unsigned char packet[2048];
  int plen;
  unsigned char ipacket[2048];
  int iplen;
  struct hostent *hp;
  struct sockaddr_in sin, lsin;
  fd_set readfds;
  char **hostname;
  int trys;
  char *krb_get_phost();

  memset(packet, 0, sizeof(packet)); /* Zeroize Memory */
  memset(ipacket, 0, sizeof(ipacket));
  krb_get_lrealm(lrealm, 1);	/* Get our Kerberos realm */

  RSA_MD2(Data, DataLen, hash);
  RSA_MD2(hash, 16, dhash);	/* For use of Kerberos */
  memcpy(packet, hash, 16);

  cksum = 0;
  for (i = 0; i < 4; i++)	/* High order 32 bits of dhash is the
				   Kerberos checksum, I wish we could do
				   better, but this is all kerberos allows
				   us */
    cksum = (cksum << 8) + dhash[i];

  /* Use Hesiod to find service location of GDSS Server Here */

  hostname = hes_resolve("gdss", "sloc");
  if (hostname == NULL) return(-1); /* No hesiod available */

  cp = krb_get_phost(*hostname);
  if (cp == NULL) return (-1);	/* Should use a better error code */

  strcpy(sinst, cp);

  hp = gethostbyname(*hostname);

  if(hp == NULL) return (-1);	/* Could not find host, you lose */

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = hp->h_addrtype;
  memcpy(&sin.sin_addr, hp->h_addr, sizeof(hp->h_addr));
  sin.sin_port = htons(7201);	/* Should get this from services or Hesiod */

  strcpy(linst, "gdss");	/* Grrr... krb_mk_req bashes its input
				   So we better copy it first! */
  status = krb_mk_req(&authent, linst, sinst, lrealm, cksum);
  if (status != KSUCCESS) return (GDSS_E_KRBFAIL);
  packet[0] = 0;		/* Version 0 of protocol */
  memcpy(&packet[1], hash, 16);
  memcpy(&packet[17], &authent, sizeof(authent));
  plen = sizeof(authent) + 16 + 1;	/* KTEXT_ST plus the hash + version */

  s = -1;			/* "NULL" Value for socket */
  do {
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
      status = GDSS_E_NOSOCKET;
      break;
    }
    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
      status = GDSS_E_NOCONNECT;
      break;
    }
    trys = 3;
    status = GDSS_E_TIMEDOUT;
    while (trys > 0) {
      if(send(s, packet, plen, 0) < 0) break;
      FD_ZERO(&readfds);
      FD_SET(s, &readfds);
      if ((select(s+1, &readfds, (fd_set *)0, (fd_set *)0, &timeout) < 1)
	  || !FD_ISSET(s, &readfds)) {
	trys--;
	continue;
      }
      if((iplen = recv(s, (char *)ipacket, 2048, 0)) < 0) break;
      status = GDSS_SUCCESS;
      break;
    }
  } while (0);
  shutdown(s, 0);
  close(s);
  if (status != GDSS_SUCCESS) return (status);
  memcpy(Signature, ipacket, iplen);
  return (GDSS_SUCCESS);
}
