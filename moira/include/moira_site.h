/*	This is the file infodefs.h for allmaint, the SMS client that allows
 *      a user to maintaint most important parts of the SMS database.
 *	It Contains: Definitions
 *	
 *	Created: 	6/8/88
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/moira_site.h,v $
 *      $Author: kit $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/moira_site.h,v 1.3 1988-06-29 20:12:16 kit Exp $
 *	
 *  	Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

/*
 *     This file contains all the definitions that allow easy access to 
 * elements returned by most of the SMS queries that return information.
 *
 *     If order of the arguments in the SMS queries change (again) then all
 * that need to be changed is the values of these items and all
 * should be well.
 *
 *                                 Chris D. Peterson - kit@athena
 *                                   6/8/88
 */

#ifndef _infodefs_
#define _infodefs_

#include "mit-copyright.h"

#define NAME 0

/* get_acl_use */

#define ACL_TYPE 0
#define ACL_NAME 1
#define ACL_END  2

/* alias queries. */

#define ALIAS_NAME  NAME
#define ALIAS_TYPE  1
#define ALIAS_TRANS 2
#define ALIAS_END   3

/* Cluster information queries */

#define C_NAME       NAME
#define C_DESCRIPT   1
#define C_LOCATION   2
#define C_MODTIME    3
#define C_MODBY      4
#define C_MODWITH    5
#define C_END        6

/* Cluster Data information queries */

#define CD_NAME      NAME
#define CD_LABEL     1
#define CD_DATA      2
#define CD_END       3

/* Delete Member from list queries. */

#define DM_LIST      0
#define DM_TYPE      1
#define DM_MEMBER    2
#define DM_END       3

/* Filesys queries (attachmaint) */

#define FS_NAME         NAME
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

#define GLOM_NAME     NAME
#define GLOM_ACTIVE   1
#define GLOM_PUBLIC   2
#define GLOM_HIDDEN   3
#define GLOM_MAILLIST 4
#define GLOM_GROUP    5
#define GLOM_END      6

/* General List information Queries. */

#define L_NAME     NAME
#define L_ACTIVE   1
#define L_PUBLIC   2
#define L_HIDDEN   3
#define L_MAILLIST 4
#define L_GROUP    5
#define L_GID      6
#define L_ACL_TYPE 7
#define L_ACL_NAME 8
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

#define M_NAME       NAME
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

#define NFS_NAME    NAME
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

#define PO_NAME    NAME
#define PO_TYPE    1
#define PO_MACHINE 2
#define PO_BOX     3
#define PO_END     4

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

/* User Information queries */

#define U_NAME    NAME
#define U_NEWNAME 1
#define U_UID     2
#define U_SHELL   3
#define U_LAST    4
#define U_FIRST   5
#define U_MIDDLE  6
#define U_STATE   7  
#define U_MITID   8
#define U_CLASS   9
#define U_MODTIME 10
#define U_MODBY   11
#define U_MODWITH 12
#define U_END     13

#endif _infodefs_		/* Do not add anything after this line. */

