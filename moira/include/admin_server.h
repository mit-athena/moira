
#define PW_SRV_VERSION	2		/* version number */
#ifndef KERB_HOST
#define	KERB_HOST	"kerberos"
#endif
#define INSTALL_NEW_PW	(1<<0)		/* ver, cmd, name, password,
					   old_pass, crypt_pass, uid */
#define ADMIN_NEW_PW	(2<<1)		/* ver, cmd, name, passwd,
					   old_pass (grot), crypt_pass (grot),
					   */
#define ADMIN_SET_KDC_PASSWORD (3<<1)   /* ditto */
#define ADMIN_ADD_NEW_KEY (4<<1)        /* ditto */
#define INSTALL_REPLY	(1<<1)		/* ver, cmd, name, password */
#define	RETRY_LIMIT	1
#define	TIME_OUT	30
#define USER_TIMEOUT	90

#define MAX_KPW_LEN	40		/* hey, seems like a good number */

