/*
 * Include file for GDSS The Generic Digital Signature Service.
 *
 */
#ifdef SOLARIS
#include <string.h>
#endif

/* Structures Needed */

typedef struct {
	int	SigInfoVersion;
	char	pname[ANAME_SZ];
	char	pinst[INST_SZ];
	char	prealm[REALM_SZ];
	unsigned int	timestamp;
	unsigned char *rawsig;	/* Raw Signature Bytes */
} SigInfo;

/* GDSS Errors */

#define GDSS_SUCCESS	0	/* No error */
#define GDSS_E_BADSIG	-1	/* Signature failed to verify */
#define GDSS_E_NOPUBKEY -2	/* Couldn't find public key file */
#define GDSS_E_KRBFAIL	-3	/* Generic Kerberos error during sign */
#define GDSS_E_NOSOCKET	-4	/* socket() call failed during sign */
#define GDSS_E_NOCONNECT -5	/* connect() call failed during sign */
#define GDSS_E_TIMEDOUT	-6	/* timed out contacting gdss sign server */
#define GDSS_E_PADTOOMANY -7	/* Padding required to much space */
#define GDSS_E_ALLOC	-8	/* malloc() failed to allocate memory */
#define GDSS_E_BVERSION -9	/* Bad version ID in signature */
#define GDSS_E_BADINPUT -10	/* Bad input value to Recompose */

/* GDSS Constants */

#define GDSS_PAD	10	/* Maximum number of nulls to pad */
#define GDSS_ESCAPE	0x20	/* Escape character for padding */
#define GDSS_NULL	0x21	/* Psuedo Null character */
