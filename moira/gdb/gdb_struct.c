/*
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/gdb_struct.c,v 1.5 1994-09-16 16:25:08 jweiss Exp $
 */

#ifndef lint
static char *rcsid_gdb_struct_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/gdb_struct.c,v 1.5 1994-09-16 16:25:08 jweiss Exp $";
#endif	lint


/************************************************************************/
/*	
/*			   gdb_struct.c
/*	
/*	      GDB - Structured Data Maintenance Routines
/*	
/*	Author: Noah Mendelsohn
/*	Copyright: 1986 MIT Project Athena 
/*		For copying and distribution information, please see
/*	  	the file <mit-copyright.h>.
/*	
/*	These routines implement the following layers of the 
/*	Client Library Specification of the GDB system:
/*	
/*		Layer		Function
/*		-----		--------
/*		  2		Structured Data Management at a 
/*				Single Site
/*		  
/*		  4		Memory Management
/*	
/*		  5		String Management
/*	
/*	Some of the routines specified are actually implemented as
/*	macros defined in gdb.h.
/*	
/************************************************************************/

#include <mit-copyright.h>
#include <stdio.h>
#include <string.h>
#include "gdb.h"

extern char *malloc();
#ifndef lint
extern void   free();
#endif

/************************************************************************/
/*	
/*			MEMORY MANAGEMENT
/*	
/*	In anticipation of the day when we may want to do something 
/*	fancy with memory management, all of the gdb routines which
/*	require dynamic allocation of memory call the routines named
/*	db_alloc and db_free.  For the moment, these are implemented
/*	as calls to malloc and free.  
/*	
/************************************************************************/

	/*----------------------------------------------------------*/
	/*	
	/*			gdb_am
	/*	
	/*	Allocate memory for use by gdb.  Current implementation
	/*	just does a malloc.
	/*	
	/*----------------------------------------------------------*/

char *
gdb_am(bytes)
int bytes;
{
	return(malloc((unsigned)bytes));
}

	/*----------------------------------------------------------*/
	/*	
	/*			gdb_fm
	/*	
	/*	Return allocated memory.  Note: the current 
	/*	implementation ignores the byte count supplied,
	/*	but future implementations may require that it 
	/*	be correct.
	/*	
	/*----------------------------------------------------------*/

/*ARGSUSED*/
int
gdb_fm(ptr, bytes)
char *ptr;
int bytes;
{
	free(ptr);
	return;
}

/************************************************************************/
/*	
/*			STRING MANAGEMENT
/*	
/*	To allow dynamic manipulation of strings in gdb without
/*	excessive memory re-allocation, we define a string as a
/*	counted byte space.  Though this space will frequently be used
/*	to store a standard null terminated string, that is not
/*	required.
/*	
/*	Current representation for a string is a pointer followed by
/*	an integer length.  A null pointer indicates a null string, in
/*	which case the length is arbitrary.  Any other pointer is to
/*	memory which was allocated by db_alloc in which must be free'd
/*	eventually with db_free.
/*	
/************************************************************************/

	/*----------------------------------------------------------*/
	/*	
	/*		string_alloc (string_alloc)
	/*	
	/*	Fills in supplied string descriptor and returns 
	/*	pointer to the newly allocated data.
	/*	
	/*----------------------------------------------------------*/

char *
string_alloc(stringp, bytes)
STRING *stringp;				/* pointer to string */
						/* descriptor to be */
						/* filled in */
int	bytes;					/* number of bytes to alloc */
{
	GDB_INIT_CHECK

	MAX_STRING_SIZE(*stringp) = bytes;	/* put length in returned */
						/* string descriptor-- */
						/* will be irrelavent if */
						/* alloc fails */

	STRING_DATA(*stringp) = db_alloc(bytes); /* try to get the data */
	return (STRING_DATA(*stringp));		/* return ptr to new string */
						/* if any */
}

	/*----------------------------------------------------------*/
	/*	
	/*		  string_free (string_free)
	/*	
	/*	Releases the data space for a gdb string.  Must have
	/*	been allocated with string_alloc.  Remember to pass
	/*	in the address of the string descriptor, not the 
	/*	descriptor itself!
	/*	
	/*----------------------------------------------------------*/
int
string_free(stringp)
STRING *stringp;
{
	GDB_INIT_CHECK

	if (stringp->ptr == NULL)
		return;	 
	db_free(stringp->ptr, stringp->length);
	stringp->ptr = NULL;
	stringp->length = 0;
	return;
}

/************************************************************************/
/*	
/*	    STRUCTURED DATA MANAGEMENT AT A SINGLE SITE
/*	
/*	These routines provide the abstraction of typed, structured
/*	data at a single site.  Tuples are collections of typed fields,
/*	and they are each described by a tuple descriptor.  Relations
/*	are circularly linked lists of tuples.  For completeness, a
/*	relation also carries a tuple descriptor, which should match
/*	the descriptor for each of its constituent tuples.  This allows
/*	a null relation to be typed.
/*	
/*	Some of the facilities of structured data management are 
/*	defined as macros in gdb.h.  In many cases, the routines
/*	declared below are known by defines of more descriptive
/*	names, also in gdb.h.
/*	
/************************************************************************/
/************************************************************************/
/*	
/*	             TUPLE_DESCRIPTOR MANAGEMENT
/*	
/************************************************************************/


	/*----------------------------------------------------------*/
	/*	
	/*	    create_tuple_descriptor (create_tuple_descriptor)
	/*	
	/*	Allocates space for a tuple descriptor and fills
	/*	it in.  Gives up if space is not available.  
	/*	Should be passed a list of integer coded types
	/*	and a list of string field names.  
	/*	
	/*	Tuple descriptors are reference counted, and they are
	/*	not really deleted until the reference count goes
	/*	to zero.   It is presumed that all callers use the
	/*	create and delete routines, or otherwise maintain
	/*	the reference count appropriately.
	/*	
	/*----------------------------------------------------------*/


TUPLE_DESCRIPTOR
create_tuple_descriptor(number_of_fields, name_list, type_list)
char *name_list[];
FIELD_TYPE type_list[];
int  number_of_fields;
{
	register TUPLE_DESCRIPTOR tpd;		/* pointer to new descriptor */
	register int i;
	register int data_len;			/* length of the actual */
	int field_len;				/* length of current field */
	int align;				/* code describing alignment */
						/* requirement for this field*/
						/* (4 for fullword, 1 for */
						/* char, etc.) */
	int next_offset;			/* byte offset to next field */
	int descriptor_length;			/* length of the part of */
						/* the allocated storage */
						/* actually used for the */
						/* descriptor.*/
	int str_len;				/* we also have to retain */
						/* the string names for the */
						/* fields.  These are stored */
						/* immediately off the end */
						/* of the descriptor, and */
						/* all are allocated */
						/* together.  This is the */
						/* length of the string data */
	char *next_name;			/* place to put the next */
						/* copied fieldname*/


	GDB_INIT_CHECK

       /*
        * Calculate size and allocate descriptor
        */

	descriptor_length = sizeof(struct tupl_desc) +
	                    (number_of_fields-1) * sizeof(struct tupld_var);

	str_len = 0;

	for (i=0; i<number_of_fields; i++)
		str_len += strlen(name_list[i]) +1;

	tpd = (TUPLE_DESCRIPTOR)db_alloc(descriptor_length+str_len);	
						/* try to allocate it */

	if (tpd == NULL)
		GDB_GIVEUP("create_tuple_descriptor (gbd_ctd) could not get enough memory.\n") 

         /*
          * Fill in the descriptor fields:
          * 
          * Each field is aligned according to its own alignment code.
          * Going in to the top of the for loop, next_offset is set to
          * the offset of the first possible byte for storing the next field.
          * During the loop, that number is rounded up, if necessary, to
          * achieve the alignment actually required for the field.  Finally,
          * the length of the new field is added, which yields the first
          * possible byte of any field to follow.
          */

	tpd->id = GDB_DESC_ID;

	tpd->ref_count = 1;			/* whoever asked for creation*/
						/* is expected to delete*/

	tpd->field_count = number_of_fields;
	tpd->str_len = str_len;

	data_len = sizeof(struct tuple_dat) - 1; /*  tuple_dat includes the */
						 /*  first byte of data */
	next_offset = 0;
	next_name = ((char *)tpd) + descriptor_length;
						/* place to put first */
						/* field name*/
	for (i=0; i<number_of_fields; i++) {
               /*
                * Calculate lengths and alignments for the field data.
                */
		field_len = INT_PROPERTY(type_list[i],LENGTH_PROPERTY);
		align = INT_PROPERTY(type_list[i],ALIGNMENT_PROPERTY);
               /*
                * Copy the string field name into the newly allocated
                * space just after the descriptor itself.
                */
		tpd->var[i].name = strcpy(next_name, name_list[i]);
		next_name += strlen(next_name) + 1;
               /*
                * Put in the type and the length for the field data
                */
		tpd->var[i].type = type_list[i];
		tpd->var[i].length = field_len;
               /*
                * Now store the actual offset of this field, and 
                * compute the first byte address we could conceivably
                * use for the next field.
                */
		next_offset = GDB_ROUNDUP(next_offset, align);
		tpd->var[i].offset = next_offset;
		next_offset += field_len;
	}
	
	data_len += next_offset;
	tpd->data_len = data_len;

	return tpd;				/* return the new descriptor */
}

	/*----------------------------------------------------------*/
	/*	
	/*	    delete_tuple_descriptor (delete_tuple_descriptor)
	/*	
	/*	Return the space for a tuple descriptor
	/*	
	/*----------------------------------------------------------*/

int
delete_tuple_descriptor(t)
TUPLE_DESCRIPTOR t;
{
	int descriptor_length;
	register int ref_count;			/* buffer the reference */
						/* count here */

	if (t == NULL)
		return ;
	
	GDB_CHECK_TPD(t, "delete_tuple_descriptor")


       /*
        * Decrement the reference count.  If it's not zero, then just 
        * return.
        */
	if ((ref_count = --(t->ref_count)) >0) 
		return ;
	if (ref_count <0)
		GDB_GIVEUP("Tuple descriptor reference count is less than zero")
       /*
        * Current representation is to allocate the space for the string
        * right off the end of the descriptor itself.  We therefore have
        * to add their length into the amount we free.
        */
	descriptor_length = gdb_descriptor_length(t->field_count);	
	db_free((char *)t, descriptor_length+t->str_len);

	return ;
}

	/*----------------------------------------------------------*/
	/*	
	/*	           field_index (field_index)
	/*	
	/*----------------------------------------------------------*/

int
field_index(tuple_descriptor, field_name)
TUPLE_DESCRIPTOR tuple_descriptor;
char *field_name;
{
	register int i;
	register int n;
	register TUPLE_DESCRIPTOR tpd = tuple_descriptor;

       /*
        * Make sure supplied descriptor is valid
        */
	if (tpd == NULL)
		GDB_GIVEUP("null tuple descriptor passed to field_index function")
	GDB_CHECK_TPD(tpd, "field_index")

	n = tpd -> field_count;

       /*
        * Loop through each field in descriptor, return index if match
        */

	for(i=0; i<n; i++) 
	        if (strcmp(field_name, tpd->var[i].name) == 0)
			return i;
       /*
        * No match, return -1
        */
	return (-1);	
}


/************************************************************************/
/*	
/*			TUPLE MANAGEMENT
/*	
/************************************************************************/

	/*----------------------------------------------------------*/
	/*	
	/*		  create_tuple (create_tuple)
	/*	
	/*	Allocate space for a new tuple, given its 
	/*	descriptor.  Giveup if out of memory.
	/*	
	/*----------------------------------------------------------*/

TUPLE
create_tuple(descriptor)
TUPLE_DESCRIPTOR descriptor;
{
	register TUPLE t;

	GDB_CHECK_TPD(descriptor, "create_tuple")

	t = (TUPLE)db_alloc(descriptor -> data_len);

	if (t == NULL)
		GDB_GIVEUP("create_tuple (create_tuple) could not allocate enough memory.\n")

	t->id = GDB_TUP_ID;

	t->desc = descriptor;			/* fill in descriptor */
						/* pointer in new tuple */
	REFERENCE_TUPLE_DESCRIPTOR(descriptor);	/* bump the reference count */
#ifdef GDB_CHECK
       /*
        * Only for the sake of  keeping things clean, null out the pointers.
        * Wastes time, but helps debugging.
        */
	t->next = t->prev = NULL;
#endif GDB_CHECK	

	return t;
}

	/*----------------------------------------------------------*/
	/*	
	/*		delete_tuple (delete_tuple)
	/*	
	/*	Release the data space occupied by a tuple. 
	/*	
	/*----------------------------------------------------------*/

int
delete_tuple(t)
TUPLE t;
{
	register TUPLE_DESCRIPTOR tpd;
 	if (t==NULL)
		GDB_GIVEUP("Delete_tuple called with null tuple")
	GDB_CHECK_TUP(t, "delete_tuple")
	tpd = t->desc;
	db_free((char *)t, t->desc->data_len);
	delete_tuple_descriptor(tpd);		/* does a reference counted */
						/* delete*/
}

	/*----------------------------------------------------------*/
	/*	
	/*	             initialize_tuple (initialize_tuple)
	/*	
	/*	Set each field in tuple to its null value.
	/*	
	/*----------------------------------------------------------*/

int
initialize_tuple(t)
TUPLE t;
{
	register char *field_data;		/* pointer to first byte */
						/* of field data in tuple*/
	register TUPLE_DESCRIPTOR tpd;		/* pointer to descriptor */
	register int i;				/* counter of fields */
	int num_fields;				/* total number of fields */

       /*
        * Return if no tuple at all supplied--perhaps this should be
        * an error.  If supplied, make sure it looks like a tuple.
        */

	if (t == NULL)
		return;

	GDB_CHECK_TUP(t, "initialize_tuple")

       /*
        * Set up to loop through fields:  get tuple descriptor, field count
        * and pointer to first data byte in the tuple.
        */

	tpd = t->desc;
	num_fields = tpd->field_count;
	field_data = t->data;			/* address of first byte of */
						/* user data */


       /*
        * For each field in the tuple, loop calling its null value 
        * initialization routine.
        */

	for (i=0; i<num_fields; i++) {
		FCN_PROPERTY(tpd->var[i].type, NULL_PROPERTY)
			     (field_data+tpd->var[i].offset);
	}
}


	/*----------------------------------------------------------*/
	/*	
	/*	      null_tuple_strings (null_tuple_strings)
	/*	
	/*	Reclaim the space for all fields in the tuple
	/*	whose type is 'string.'
	/*	
	/*----------------------------------------------------------*/

int
null_tuple_strings(t)
TUPLE t;
{
	register char *field_data;		/* pointer to first byte of */
						/* field data in tuple */
	register TUPLE_DESCRIPTOR tpd;		/* pointer to descriptor */
	register int i;				/* counter of fields */
	int num_fields;				/* total number of fields */

       /*
        * Return if no tuple at all supplied--perhaps this should be
        * an error
        */

	if (t == NULL)
		return;

	GDB_CHECK_TUP(t, "null_tuple_strings")

       /*
        * Set up to loop through fields:  get tuple descriptor, field count
        * and pointer to first data byte in the tuple.
        */

	tpd = t->desc;
	num_fields = tpd->field_count;
	field_data = t->data;			/* address of first byte of */
						/* user data */

       /*
        * For each field in the tuple, loop calling its null value 
        * initialization routine.
        */

	for (i=0; i<num_fields; i++) {
		if(FIELD_TYPE_IN_TUPLE(tpd,i) == STRING_T &&
		   (*(char **)(field_data+FIELD_OFFSET_IN_TUPLE(tpd,i)))!=NULL)
			string_free((STRING *)(field_data+
					       FIELD_OFFSET_IN_TUPLE(tpd,i)));
	}
}

/************************************************************************/
/*	
/*	                RELATION MANAGEMENT
/*	
/************************************************************************/

	/*----------------------------------------------------------*/
	/*	
	/*	       create_relation (create_relation)
	/*	
	/*----------------------------------------------------------*/

RELATION
create_relation(desc)
TUPLE_DESCRIPTOR desc;
{
	register RELATION r;

	GDB_CHECK_TPD(desc, "create_relation")

	r = (RELATION)db_alloc(sizeof(struct rel_dat));

	if (r == NULL) 
		GDB_GIVEUP("create_relation (create_relation) could not get enough space.\n")

       /*
        * Fill in the empty relation.  Create a null circular list
        * of tuples and also hang the description.
        */

	r->id = GDB_REL_ID;

	r->first = (TUPLE)r;
	r->last = (TUPLE)r;
	r->desc = desc;
	REFERENCE_TUPLE_DESCRIPTOR(desc);	/* bump the reference count */

	return r;
}

	/*----------------------------------------------------------*/
	/*	
	/*		delete_relation
	/*	
	/*	Deletes the tuples which comprise a relation.
	/*	For each tuple, it does a null-tuple-strings 
	/*	prior to deleting, but it does not yet handle
	/*	any other non-contiguous data.
	/*	
	/*----------------------------------------------------------*/

int
delete_relation(rel)
RELATION rel;
{
	register TUPLE t, next;
	TUPLE_DESCRIPTOR desc;

       /*
        * Make sure a proper relation is supplied.
        */

	if (rel == NULL)
		GDB_GIVEUP("delete_relation called with null relation")
        GDB_CHECK_REL(rel, "delete_relation")

	t = FIRST_TUPLE_IN_RELATION(rel);

	/*----------------------------------------------------------*/
	/*	
	/*	Free all the tuples
	/*	
	/*----------------------------------------------------------*/

	while (t!=NULL) {
		null_tuple_strings(t);
		next = NEXT_TUPLE_IN_RELATION(rel, t);
		delete_tuple(t);
		t = next;
	}

	/*----------------------------------------------------------*/
	/*	
	/*	Give back the memory for the relation
	/*	
	/*----------------------------------------------------------*/

	desc = DESCRIPTOR_FROM_RELATION(rel);
	db_free((char *)rel, sizeof(struct rel_dat));
	delete_tuple_descriptor(desc);		/* does a reference */
						/* counted delete */
}

	/*----------------------------------------------------------*/
	/*	
	/*		tuples_in_relation
	/*	
	/*	Returns number of tuples in a relation.
	/*	
	/*	
	/*----------------------------------------------------------*/

int
tuples_in_relation(rel)
RELATION rel;
{
	register int count;
	register RELATION r=rel;
	register TUPLE t;

	count = 0;

	for (t=FIRST_TUPLE_IN_RELATION(r);
	     t != NULL;
	     t=NEXT_TUPLE_IN_RELATION(r,t)) {
		  count++;
        }
	return count;
}
	


