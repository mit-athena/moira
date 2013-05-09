/* $Id$
 *
 * This file contains all definitions that allow easy access to
 * elements returned by most of the @i[Moira] queries.  It also defines
 * the default server location and the directories used on the server.
 *
 * If the order of the arguments in the @i[Moira] queries change (again)
 * then all that needs to be changed are the values of these items
 * and all should be well, (hopefully :-).
 *
 *                                 Chris D. Peterson - kit@athena
 *                                   7/27/88
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
 * For further information on copyright and distribution
 * see the file <mit-copyright.h>.
 */

#ifndef _moira_site_
#define _moira_site_

#include "mit-copyright.h"

/* Default Moira server to connect to */
#define MOIRA_SERVER	"MOIRA.MIT.EDU:moira_db"


#define NAME 0

/* get_ace_use */

#define ACE_TYPE 0
#define ACE_NAME 1
#define ACE_END  2

/* acl queries */

#define ACL_HOST    0
#define ACL_TARGET  1
#define ACL_KIND    2
#define ACL_LIST    3
#define ACL_MODTIME 4
#define ACL_MODBY   5
#define ACL_MODWITH 6
#define ACL_END     7

/* alias queries */

#define ALIAS_NAME  0
#define ALIAS_TYPE  1
#define ALIAS_TRANS 2
#define ALIAS_END   3

/* Cluster information queries */

#define C_NAME       0
#define C_DESCRIPT   1
#define C_LOCATION   2
#define C_MODTIME    3
#define C_MODBY      4
#define C_MODWITH    5
#define C_END        6

/* Cluster Data information queries */

#define CD_NAME      0
#define CD_LABEL     1
#define CD_DATA      2
#define CD_END       3

/* Delete Member from list queries. */

#define DM_LIST      0
#define DM_TYPE      1
#define DM_MEMBER    2
#define DM_END       3

/* Finger queries */

#define F_NAME          0
#define F_FULLNAME      1
#define F_NICKNAME      2
#define F_HOME_ADDR     3
#define F_HOME_PHONE    4
#define F_OFFICE_ADDR   5
#define F_OFFICE_PHONE  6
#define F_MIT_DEPT      7
#define F_MIT_AFFIL     8
#define F_MODTIME       9
#define F_MODBY         10
#define F_MODWITH       11
#define F_END           12

/* Filesys queries */

#define FS_NAME         0
#define FS_TYPE         1
#define FS_MACHINE      2
#define FS_PACK         3
#define FS_M_POINT      4
#define FS_ACCESS       5
#define FS_COMMENTS     6
#define FS_OWNER        7
#define FS_OWNERS       8
#define FS_CREATE       9
#define FS_L_TYPE       10
#define FS_MODTIME      11
#define FS_MODBY        12
#define FS_MODWITH      13
#define FS_END          14

/* Get List Of Member queries. */

#define GLOM_NAME     0
#define GLOM_ACTIVE   1
#define GLOM_PUBLIC   2
#define GLOM_HIDDEN   3
#define GLOM_MAILLIST 4
#define GLOM_GROUP    5
#define GLOM_END      6

/* Kerberos/User Map */

#define KMAP_USER	0
#define KMAP_PRINCIPAL	1
#define KMAP_END	2

/* General List information Queries, v4. */

#define L_NAME            0
#define L_ACTIVE          1
#define L_PUBLIC          2
#define L_HIDDEN          3
#define L_MAILLIST        4
#define L_GROUP           5
#define L_GID             6
#define L_NFSGROUP        7
#define L_MAILMAN         8
#define L_MAILMAN_SERVER  9
#define L_ACE_TYPE        10
#define L_ACE_NAME        11
#define L_MEMACE_TYPE     12
#define L_MEMACE_NAME     13
#define L_DESC            14
#define L_MODTIME         15
#define L_MODBY           16
#define L_MODWITH         17
#define L_END             18

/* List Member information queries. */

#define LM_LIST   0
#define LM_TYPE   1
#define LM_MEMBER 2
#define LM_END    3

#define LM_TAG     3
#define LM_TAG_END 4

/* Machine information queries */

#define M_NAME		0
#define M_VENDOR	1
#define M_TYPE		1
#define M_MODEL		2
#define M_OS		3
#define M_LOC		4
#define M_CONTACT	5
#define M_BILL_CONTACT  6
#define M_ACCT_NUMBER   7
#define M_USE		8
#define M_STAT		9
#define M_STAT_CHNG	10
#define M_SUBNET	11
#define M_ADDR		12
#define M_OWNER_TYPE	13
#define M_OWNER_NAME	14
#define M_ACOMMENT	15
#define M_OCOMMENT	16
#define M_CREATED	17
#define M_CREATOR	18
#define M_INUSE		19
#define M_MODTIME	20
#define M_MODBY		21
#define M_MODWITH	22
#define M_END		23

/*  Machine to Cluster mapping */

#define MAP_MACHINE  0
#define MAP_CLUSTER  1
#define MAP_END      2

/*  NFS phys. queries. */

#define NFS_NAME    0
#define NFS_DIR     1
#define NFS_DEVICE  2
#define NFS_STATUS  3
#define NFS_ALLOC   4
#define NFS_SIZE    5
#define NFS_MODTIME 6
#define NFS_MODBY   7
#define NFS_MODWITH 8
#define NFS_END     9

/* Printer queries */

#define PRN_NAME		0
#define PRN_TYPE		1
#define PRN_HWTYPE		2
#define PRN_DUPLEXNAME		3
#define PRN_DUPLEXDEFAULT 	4
#define PRN_HOLDDEFAULT 	5
#define PRN_STATUS      	6
#define PRN_HOSTNAME		7
#define PRN_LOGHOST		8
#define PRN_RM			9
#define PRN_RP			10
#define PRN_RQ			11
#define PRN_KA			12
#define PRN_PC			13
#define PRN_AC			14
#define PRN_LPC_ACL		15
#define PRN_REPORT_LIST		16
#define PRN_BANNER		17
#define PRN_LOCATION		18
#define PRN_CONTACT		19
#define PRN_MODTIME		20
#define PRN_MODBY		21
#define PRN_MODWITH		22
#define PRN_END			23

#define PRN_BANNER_NONE  0
#define PRN_BANNER_FIRST 1
#define PRN_BANNER_LAST  2

/* Print Server queries */

#define PRINTSERVER_HOST	0
#define PRINTSERVER_KIND	1
#define PRINTSERVER_TYPES	2
#define PRINTSERVER_OWNER_TYPE	3
#define PRINTSERVER_OWNER_NAME	4
#define PRINTSERVER_LPC_ACL	5
#define PRINTSERVER_MODTIME	6
#define PRINTSERVER_MODBY	7
#define PRINTSERVER_MODWITH	8
#define PRINTSERVER_END		9

/* PO box information queries */

#define PO_NAME    0
#define PO_TYPE    1
#define PO_BOX     2
#define PO_ADDR    3
#define PO_END     4

/* Quota queries */

#define Q_FILESYS   0
#define Q_TYPE      1
#define Q_NAME      2
#define Q_QUOTA     3
#define Q_DIRECTORY 4
#define Q_MACHINE   5
#define Q_MODTIME   6
#define Q_MODBY     7
#define Q_MODWITH   8
#define Q_END       9

/* Service info */

#define SVC_SERVICE	0
#define SVC_INTERVAL	1
#define SVC_TARGET	2
#define SVC_SCRIPT	3
#define SVC_DFGEN	4
#define SVC_DFCHECK	5
#define SVC_TYPE	6
#define SVC_ENABLE	7
#define SVC_INPROGRESS	8
#define SVC_HARDERROR	9
#define SVC_ERRMSG	10
#define SVC_ACE_TYPE	11
#define SVC_ACE_NAME	12
#define SVC_MODTIME	13
#define SVC_MODBY	14
#define SVC_MODWITH	15
#define SVC_END		16

/* Service add/update */

#define SC_SERVICE	0
#define SC_INTERVAL	1
#define SC_TARGET	2
#define SC_SCRIPT	3
#define SC_TYPE		4
#define SC_ENABLE	5
#define SC_ACE_TYPE	6
#define SC_ACE_NAME	7
#define SC_END		8

/* Service/host tuples */

#define SH_SERVICE	0
#define SH_MACHINE	1
#define SH_ENABLE	2
#define SH_OVERRIDE	3
#define SH_SUCCESS	4
#define SH_INPROGRESS	5
#define SH_HOSTERROR	6
#define SH_ERRMSG	7
#define SH_LASTTRY	8
#define SH_LASTSUCCESS	9
#define SH_VALUE1	10
#define SH_VALUE2	11
#define SH_VALUE3	12
#define SH_MODTIME	13
#define SH_MODBY	14
#define SH_MODWITH	15
#define SH_END		16

/* Service/host tuple add & updates */

#define SHI_SERVICE	0
#define SHI_MACHINE	1
#define SHI_ENABLE	2
#define SHI_VALUE1	3
#define SHI_VALUE2	4
#define SHI_VALUE3	5
#define SHI_END		6

/* Subnet info */

#define SN_NAME		0
#define SN_DESC		1
#define SN_STATUS       2
#define SN_CONTACT      3
#define SN_ACCT_NUMBER  4
#define SN_ADDRESS	5
#define SN_MASK		6
#define SN_LOW		7
#define SN_HIGH		8
#define SN_PREFIX	9
#define SN_ACE_TYPE	10
#define SN_ACE_NAME	11
#define SN_MODTIME	12
#define SN_MODBY	13
#define SN_MODWITH	14
#define SN_END		15

/* Subnet statuses */
#define SNET_STATUS_RESERVED         0
#define SNET_STATUS_BILLABLE         1
#define SNET_STATUS_PRIVATE_10MBPS   2
#define SNET_STATUS_PRIVATE_100MBPS  3
#define SNET_STATUS_PRIVATE_OTHER    4
#define SNET_STATUS_RESNET_DORM      5
#define SNET_STATUS_INFRASTRUCTURE   6
#define SNET_STATUS_PRIVATE_1000MBPS 7
#define SNET_STATUS_RESNET_FSILG     8

/* User Information queries, v11 */

#define U_NAME    0
#define U_UID     1
#define U_SHELL   2
#define U_WINCONSOLESHELL 3
#define U_LAST    4
#define U_FIRST   5
#define U_MIDDLE  6
#define U_STATE   7
#define U_MITID   8
#define U_CLASS   9
#define U_COMMENT 10
#define U_SIGNATURE 11
#define U_SECURE  12
#define U_WINHOMEDIR 13
#define U_WINPROFILEDIR 14
#define U_SPONSOR_TYPE 15
#define U_SPONSOR_NAME 16
#define U_EXPIRATION 17
#define U_ALT_EMAIL 18
#define U_ALT_PHONE 19
#define U_AFF_BASIC 20
#define U_AFF_DETAILED 21
#define U_MODTIME 22
#define U_MODBY   23
#define U_MODWITH 24
#define U_CREATED 25
#define U_CREATOR 26
#define U_END     27

/* User states (the value of argv[U_STATE] from a user query) */

#define US_NO_LOGIN_YET               0
#define US_REGISTERED                 1
#define US_NO_PASSWD                  2
#define US_DELETED                    3
#define US_NOT_ALLOWED                4
#define US_ENROLLED                   5
#define US_ENROLL_NOT_ALLOWED         6
#define US_HALF_ENROLLED              7
#define US_NO_LOGIN_YET_KERBEROS_ONLY 8
#define US_REGISTERED_KERBEROS_ONLY   9
#define US_SUSPENDED                 10
#define US_END                       11

/* User shell queries */

#define USH_NAME        0
#define USH_SHELL       1
#define USH_END         2

/* Zephyr ACL queries */

#define ZA_CLASS	0
#define ZA_XMT_TYPE	1
#define ZA_XMT_ID	2
#define	ZA_SUB_TYPE	3
#define	ZA_SUB_ID	4
#define	ZA_IWS_TYPE	5
#define	ZA_IWS_ID	6
#define	ZA_IUI_TYPE	7
#define	ZA_IUI_ID	8
#define ZA_OWNER_TYPE   9
#define ZA_OWNER_ID     10
#define	ZA_MODTIME 	11
#define	ZA_MODBY	12
#define	ZA_MODWITH	13
#define ZA_END		14

/* Container queries */

#define CON_NAME        0
#define CON_PUBLIC      1
#define CON_DESCRIPT    2
#define CON_LOCATION    3
#define CON_CONTACT     4
#define CON_OWNER_TYPE  5
#define CON_OWNER_NAME  6
#define CON_MEMACE_TYPE 7
#define CON_MEMACE_NAME 8
#define CON_MODTIME     9
#define CON_MODBY       10
#define CON_MODWITH     11
#define CON_END         12

/* Directories & files used on the server machine: */

#define MOIRA_DIR	"/moira"	/* home directory of the system */
#define BIN_DIR		"/moira/bin"	/* where progs live */
#define DCM_DIR		"/moira/dcm"	/* where DCM output files end up */
#define LOCK_DIR	"/moira/dcm/locks" /* where DCM puts lock files */

#define CRITERRLOG	"/moira/critical.log"
#define JOURNAL		"/moira/journal"
#define REGJOURNAL	"/moira/journal.reg"
#define NODCMFILE	"/etc/nodcm"	/* flag to inhibit DCM running */
#define MOIRA_MOTD_FILE	"/etc/smsdown"	/* make the server go to sleep */

#endif /* _moira_site_ */	     /* Do not add anything after this line. */
