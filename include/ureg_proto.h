/* $Id$
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 *
 */

#define UREG_VERIFY_USER 0
#define UREG_RESERVE_LOGIN 1
#define UREG_SET_PASSWORD 2
#define UREG_SET_IDENT 3
#define UREG_GET_KRB 4
#define UREG_GET_SECURE 5
#define UREG_SET_SECURE 6

typedef unsigned long U_32BIT;

#ifndef REG_SVR
/* (client) prototypes from reg_stubs.c */
int ureg_init(void);
int verify_user(char *first, char *last, char *idnumber,
		char *hashidnumber, char *login);
int grab_login(char *first, char *last, char *idnumber, char *hashidnumber,
	       char *login);
int enroll_login(char *first, char *last, char *idnumber, char *hashidnumber,
		 char *login);
int set_password(char *first, char *last, char *idnumber, char *hashidnumber,
		 char *password);
int get_krb(char *first, char *last, char *idnumber, char *hashidnumber,
	    char *password);
#endif
