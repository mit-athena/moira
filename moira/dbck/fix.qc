/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dbck/fix.qc,v 1.1 1988-09-25 21:54:45 mar Exp $
 *
 * User interface routines for dbck (SMS database consistency checker)
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <sms.h>
#include <stdio.h>
#include "dbck.h"


##char *_table;
##char *_idfield;

##generic_ffunc(id)
##int id;
##{
##  int rowcount;

##  delete _table where _table._idfield = id
##  inquire_equel(rowcount = "rowcount")
    if (rowcount > 0)
      printf("%d entr%s deleted\n", rowcount, rowcount==1?"y":"ies");
    else
      printf("Not deleted\n");
##}


generic_delete(sq, pfunc, table, idfield, preen)
struct save_queue *sq;
void (*pfunc)();
char *table, *idfield;
int preen;
{
    _table = table;
    _idfield = idfield;
    generic_fix(sq, pfunc, "Delete", generic_ffunc, preen);
}


single_delete(table, idfield, id)
char *table, *idfield;
int id;
{
    _table = table;
    _idfield = idfield;
    generic_ffunc(id);
}


##zero_fix(table, zrfield, idfield, id)
##char *table, *zrfield, *idfield;
##int id;
##{
##  int rowcount;

##  replace table (zrfield = 0) where table.idfield = id
##  inquire_equel(rowcount = "rowcount")
    if (rowcount > 0)
      printf("%d entr%s fixed\n", rowcount, rowcount==1?"y":"ies");
    else
      printf("Not fixed\n");
##}


generic_fix(sq, pfunc, msg, ffunc, preen)
struct save_queue *sq;
char *msg;
int (*pfunc)(), (*ffunc)();
int preen;
{
    int id;

    while (sq_get_data(sq, &id)) {
	if ((*pfunc)(id) == 0 && singe_fix(msg, preen))
	  (*ffunc)(id);
    }
    sq_destroy(sq);
}


int single_fix(msg, preen)
char *msg;
int preen;
{
    int lmode;

    lmode = mode;
    if (mode == MODE_PREEN) {
	if (preen)
	  lmode = MODE_YES;
	else
	  lmode = MODE_NO;
    }

    switch (lmode) {
    case MODE_ASK:
	if (!prompt(msg))
	  break;
    case MODE_YES:
	return(1);
	break;
    case MODE_NO:
	;
    }
    return(0);
}


int prompt(msg)
char *msg;
{
    char buf[BUFSIZ];
##  extern int dcmenable;

    while (1) {
	printf("%s (Y/N/Q)? ", msg);
	fflush(stdout);
	gets(buf);
	if (buf[0] == 'Y' || buf[0] == 'y')
	  return(1);
	if (buf[0] == 'N' || buf[0] == 'n')
	  return(0);
	if (buf[0] == 'Q' || buf[0] == 'q') {
	    if (prompt("Are you sure you want to quit")) {
		if (prompt("Save database changes")) {
##		    end transaction
##		    replace values (value = dcmenable)
##			where values.name = "dcm_enable"
##		    exit
		    exit(0);
		} else {
		    cleanup();
		}
	    }
	}
    }
}


/**
 ** set_next_object_id - set next object id in values table
 **
 ** Inputs: object - object name in values table and in objects
 **	    table - name of table objects are found in
 **
 ** - called before an APPEND operation to set the next object id to
 **   be used for the new record to the next free value
 **
 **/

int set_next_object_id(object, table)
    char *object;
    char *table;
##{
##  char *name, *tbl;
##  int rowcount, exists, value;

    name = object;
    tbl = table;
##  range of v is values
##  repeat retrieve (value = v.#value) where v.#name = @name
##  inquire_equel(rowcount = "rowcount")
    if (rowcount != 1)
	return(SMS_NO_ID);

##  retrieve (exists = any(tbl.name where tbl.name = value))
##  inquire_equel(rowcount = "rowcount")
    if (rowcount != 1)
	return(SMS_NO_ID);
    while (exists) {
	value++;
	if (value > MAX_ID_VALUE)
	    value = MIN_ID_VALUE;
##	retrieve (exists = any(tbl.name where tbl.name = value))
    }

    printf("setting ID %s to %d", name, value);
##  repeat replace v (#value = @value) where v.#name = @name
    return(SMS_SUCCESS);
##}


##generic_fix_id(table, idfield, txtfield, oldid, name)
##char *table;
##char *idfield;
##char *txtfield;
##int oldid;
##char *name;
##{
##  int rowcount, id;

    set_next_object_id(table, idfield);
##  retrieve (id = values.value) where values.#name = idfield
##  replace table (idfield = values.value) where values.#name = idfield and
##	table.idfield = oldid and table.txtfield = name
##  inquire_equel(rowcount = "rowcount")
    if (rowcount == 1)
      printf("Fixed\n");
    else
      printf("Not fixed, rowcount = %d\n", rowcount);
    return(id);
##}
