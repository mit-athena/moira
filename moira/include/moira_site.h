/*
 * This file contains all definitions that allow easy access to 
 * elements returned by most of the @i[SMS] queries.
 *
 * If the order of the arguments in the @i[SMS] queries change (again)
 * then all that needs to be changed are the values of these items 
 * and all should be well, (hopefully :-).
 *
 *                                 Chris D. Peterson - kit@athena
 *                                   7/27/88
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/moira_site.h,v $
 *      $Author: qjb $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/moira_site.h,v 1.11 1988-12-19 15:15:03 qjb Exp $
 *	
 *  	Copyright 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#ifndef _sms_app_
#define _sms_app_

#include "mit-copyright.h"

/* Default SMS server to connect to */
#define SMS_SERVER	"SMS.MIT.EDU:sms_db"

#define NAME 0

/* get_ace_use */

#define ACE_TYPE 0
#define ACE_NAME 1
#define ACE_END  2

/* alias queries. */

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

/* General List information Queries. */

#define L_NAME     0
#define L_ACTIVE   1
#define L_PUBLIC   2
#define L_HIDDEN   3
#define L_MAILLIST 4
#define L_GROUP    5
#define L_GID      6
#define L_ACE_TYPE 7
#define L_ACE_NAME 8
#define L_DESC     9
#define L_MODTIME  10
#define L_MODBY    11
#define L_MODWITH  12
#define L_END      13

/* List Member information queries. */

#define LM_LIST   0
#define LM_TYPE   1
#define LM_MEMBER 2
#define LM_END    3

/* Machine information queries */

#define M_NAME       0
#define M_TYPE       1
#define M_MODTIME    2
#define M_MODBY      3
#define M_MODWITH    4
#define M_END        5

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

/* PO box infomarion queries */

#define PO_NAME    0
#define PO_TYPE    1
#define PO_BOX     2
#define PO_END     3

/* Quota queries */

#define Q_FILESYS   0
#define Q_LOGIN     1
#define Q_QUOTA     2
#define Q_DIRECTORY 3  
#define Q_MACHINE   4
#define Q_MODTIME   5
#define Q_MODBY     6
#define Q_MODWITH   7
#define Q_END       8

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

/* User Information queries */

#define U_NAME    0
#define U_UID     1
#define U_SHELL   2
#define U_LAST    3
#define U_FIRST   4
#define U_MIDDLE  5
#define U_STATE   6  
#define U_MITID   7
#define U_CLASS   8
#define U_MODTIME 9
#define U_MODBY   10
#define U_MODWITH 11
#define U_END     12

/* User states (the value of argv[U_STATE] from a user query) */

#define US_NO_LOGIN_YET 0
#define US_REGISTERED   1
#define US_NO_PASSWD    2
#define US_DELETED      3
#define US_NOT_ALLOWED  4
#define US_END          5

/* User shell queries */

#define USH_NAME        0
#define USH_SHELL       1
#define USH_END         2

#endif _sms_app_		/* Do not add anything after this line. */
