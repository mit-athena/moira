/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/regtape/uidfix.qc,v 1.1 1993-01-04 16:38:29 mar Exp $
 *
 * This program will change UIDs on users who are not active so that
 * no UIDs are > 32000.
 */

#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <sys/time.h>
#include <moira.h>
#include <moira_site.h>

#define MAX_ID_VALUE	31999
#define MIN_ID_VALUE	101


char *program;

main(argc, argv)
int argc;
char **argv;
##{
     char buf[BUFSIZ], *p;
     struct save_queue *sq;
     int found, status, wait, i;
##   int id, uid;

     program = "uidfix";
     init_krb_err_tbl();
     init_sms_err_tbl();

     for (i = 1; i < argc; i++) {
	if (!strcmp(argv[i], "-w"))
	  wait++;
	else if (!strcmp(argv[i], "-D"))
	  setenv("ING_SET", "set printqry");
	else
	  fprintf(stderr, "Usage: %s [-w] [-D]\n", argv[0]);
    }

##   ingres sms
##   range of u is users

     found = 0;
     sq = sq_create();

##   retrieve (id = u.users_id)
##	where u.#uid >= 32000 and (u.status = 0 or u.status = 4) {
	sq_save_data(sq, id);
	found++;
##   }

     printf("Found %d users to change.\n", found);

     while (sq_get_data(sq, &id)) {
	 uid = set_next_object_id("uid", 1);
##	 replace u (#uid = uid) where u.users_id = id
	 if (wait) {
	     printf("Next");
	     fflush(stdout);
	     gets(buf);
	 }
     }
##   exit
     exit(0);
##}


set_next_object_id(object, limit)
    char *object;
    int limit;
##{
##  char *name;
##  int rowcount, exists, value;

    name = object;
##  begin transaction
##  repeat retrieve (value = values.#value) where values.#name = @name
##  inquire_equel(rowcount = "rowcount")
    if (rowcount != 1) {
##	abort
	return(0);
    }

##  retrieve (exists = any(users.name where users.name = value))
##  inquire_equel(rowcount = "rowcount")
    if (rowcount != 1) {
##	abort
	return(0);
    }
    while (exists) {
	value++;
	if (limit && value > MAX_ID_VALUE)
	    value = MIN_ID_VALUE;
##	retrieve (exists = any(users.name where users.name = value))
    }

##  repeat replace values (#value = @value) where values.#name = @name
##  end transaction
    return(value);
##}
