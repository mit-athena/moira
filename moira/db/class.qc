/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/db/class.qc,v 1.2 1990-03-19 18:37:06 mar Exp $
 *
 *  Counts how many member of each class exist in the database.
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>


main()
{
##  char class[9], *c;
##  int n;
    struct save_queue *sq, *sq_create();

    sq = sq_create();

##  ingres sms

##  range of u is users
##  retrieve unique (class = trim(u.mit_year)) {
	sq_save_data(sq, strsave(class));
##  }

    while (sq_get_data(sq, &c)) {
##	repeat retrieve (n = count(u.login where u.mit_year = @c))
	printf("%8s: %4d\n", c, n);
    }

##  exit
}
