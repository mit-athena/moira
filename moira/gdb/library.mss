@device(PostScript)
@make(Manual)
@style(FontFamily "Helvetica", Size 11)
@style(hyphenation on)
@style(indent 0)
@style(leftmargin +4cm)
@style(footnotes "")
@modify(example,Size 10, below 1cm, above 1cm, leftmargin +3, rightmargin +0)
@define(F, FaceCode F, Size 11, TabExport)
@define(FunctionSection, leftmargin +1cm, rightmargin +1cm, size 10)
@define(Function, leftmargin -.5cm, nofill, indent 0, break,  group,
	above .6cm, below .4cm, blanklines kept, spread .7 line)
@modify(HD1, Below .75cm, Above 1cm, indent -1cm)
@modify(HD1A, Below .75cm, Above 1cm)
@modify(HD2, Below .6cm, Above 1cm, indent -1cm)
@modify(HD3, Below .6cm, Above 1cm)
@modify(itemize, Below .5cm, Above .6cm)
@modify(example, Below .5cm, Above .6cm)
@modify(float, Below .5cm, Above .6cm)
@begin(titlepage)
@begin(titlebox)
@majorheading(GDB C Library Reference Manual)
@heading(Noah Mendelsohn)

@end(titlebox)
Document Version: 0.4 (DRAFT)
For use with GDB Release: 0.4
This draft printed on: @value(Date)
@copyrightnotice(MIT Project Athena)
@end(titlepage)
@Counter(ProgramNumber, Numbered <@1>, Referenced <@1>)
@set(ProgramNumber=1)
@PrefaceSection(Preface)

GDB is a set of C library routines and related utilities which
facilitate construction of network services.  This preliminary version
of the Library Reference Manual is based on specifications which were
written before the code.  In some cases, it refers to design decisions
which had yet to be made at the time of writing, or uses terminology
which is slightly inconsistent with the other GDB documentation.  The
tone of some of the descriptions is informal, reflecting debates over
design philosophy which occured during development of GDB.
Nevertheless, almost all of the services described herein are
currently operational in Release 0.1 of GDB, and they conform very
closely to their written specifications.  This guide is intended
primarily for reference.  For a general introduction to programming
with GDB, read the document titled "A Guide to Using GDB."  Please
report any discrepancies you discover in either document to the
author.  A more carefully edited version of this manual will accompany
GDB Release 1.

@PageHeading(Even, Right "@c[GDB Library Reference Manual]",
	     	   Left "@c[@value(Page)]")
@PageHeading(Odd, Right "@c[@value(Page)]",
	     	   Left "@c[@Title(Chapter)]")
		 
@Chapter(Introduction)
@Index[Introduction]

This document is the program interface description for a collection of
Unix@+[TM]@foot(@+[TM]Unix is a trademark of AT&T Bell Laboratories)
library routines which provide transparent distributed support for
network servers, and for relational databases in particular.  The
several libraries work together to implement a relational database
which may be conveniently accessed from anywhere in a "Berkeley" Unix
network, but the lower level libraries may themselves be useful for
building other sorts of network services, or for
transmitting other kinds of structured data.  The database access
and transmission services support communication between incompatible
machine architectures; data conversions are done as necessary by the
library routines.

This document contains a separate section for each of the libraries.
Interdependencies are noted in the sections.

@chapter(Using the Libraries)

The sections below outline the library services provided by GDB.  
Programs using GDB
to access relational databases will use, directly or indirectly, the
services of all of the GDB layers.  Programs using GDB for local management
of structured data or as a means of writing servers, clients, or other
communicating programs may use only some of the layers.    In any case,
Every C main program
that uses GDB should include the following:

@begin(group)
@begin(example)

#include <stdio.h>
#include "gdb.h"

int
main()
{
	gdb_init();
	/*
	 * Once gdb_init has been called, the services of gdb are
	 * available.  gdb_init may be called only once.
	 */
}
@end(example)
@end(group)

Other source files should @f[#include "gdb.h"], but should @i[not] 
@index(gdb.h)
invoke @f[gdb_init].
@index(gdb_init)

If you are using GDB to access an Ingres database, then there are
several other things you must do:

@begin(itemize)
@begin(multiple)
Make sure RTI Ingres is installed at the server site where the data 
@index(RTI Ingres)
@index(Ingres)
is to be stored.
@end(multiple)

@begin(multiple)
Make sure that there is an entry in the file named @f[/etc/services]
@index(/etc/services)
at both server and client sites for the service named gdb_db@index(gdb_db).
The entries at the server and client should be identical.  
Talk to your system administrator if this is not the case.
@i[Note: as of this writing, GDB does not check @f[/etc/services] when
accessing Ingres, but soon it will!]
@end(multiple)

@begin(multiple)
Make sure that the @f[dbserv] program supplied with GDB
@index(dbserv)
is started at the server site.
@end(multiple)

@end(itemize)

Gdb produces two kinds of terminal output.  Serious error messages,
which are always directed to stderr, and other output, which are
directed to the file indicated by the gdb_log global variable.
Included in the latter are the results of functions like
@f[print_relation]@index[print_relation], which are described below.
Users wishing to redirect @f[gdb_log]@index[gdb_log] may open any
stream of their choice and put the resulting FILE * variable into
gdb_log after calling gdb_init.

The remainder of this manual is taken with the definitions of the
functions provided by GDB.  The functions are organized by general
category, but all are listed alphabetically in the index.  Each
section is prefaced by a brief introduction, followed by the function
definitions themselves.  In all the descriptions below, the
pseudo-type @f[string]@Index[string] is used for @f[char *] in cases
where a null terminated string is to be supplied.  This is just a
documentation convention, the type string is not defined in the
libraries.  Here is the description of the @f[gdb_init] function, the
only one which has been introduced so far:

@begin(functionsection)
@begin(function)
void
@f[gdb_init]()
@index[gdb_init]
@end(function)

Gdb_init initializes the data structures used by gdb.  Gdb_init must be called
once and only once prior to use of any other GDB services.  No value is
returned.
@end(functionsection)

@chapter(Database Manipulation)
@Index(Database manipulation)

This library provides most of the services of a shared, network wide,
relational database.  It is built on the structured object
manipulation and transmission libraries described in Chapters
@ref(structdata) and @ref(trans).  A query, for example, returns a
RELATION in the local process space.

These routines have the following important characteristics:

@begin(itemize)
Access to databases stored on incompatible machines (e.g. RT/PC to
Vax) is supported transparently.

Pipelining of requests from client to server is encouraged.  This
maximizes overlap in processing and minimizes server swapouts between
repeated requests, thereby improving performance.

Requests proceed asynchronously with client execution, though 
synchronous interfaces are provided for applications where asynchrony
is an unnecessary complication.  Asynchronous processing is managed
by appropriate interaction with the Berkeley 'select' mechanism;
applications can be created which drive a user's terminal
or other I/O devices while simultaneously retrieving the results
of a database query.  One important use of such overlap is to support
abort operations (e.g. a screen menu selection to abort a long running
query.)

Multiple databases may be accessed simultaneously.  If desired, these
may reside at several sites.  Using the asynchronous processing options,
queries may be executed in parallel on the several databases.  The 
same facilities may be used to open parallel connections to the same
database, allowing parallel execution of independent operations.
@end(itemize)

Unfortunately, the syntax of a few of the actual library calls described
herein is Ingres dependent.  It is hoped that one day an effective
cannonical form will be developed, and these routines replaced with a
suite that might be supported using a variety of database engines.  The
Ingres dependency is isolated to a couple of routines.

@section(Types Used for Database Manipulation) 

The types @f[RELATION, TUPLE, and FIELD_TYPE] are inherited from the
structured data management libary (see Chapter @ref(structdata).)  The
following additional type is also used.

@begin(table)
@caption(Types used in Database Manipulation)
@tabset(.75in, 2.5in)

@\DATABASE@INDEX[DATABASE]@\A handle on an accessible database.
@end(table)

@Section(Routines)

NOTE:  See also the "Communications and Server Management" Section of
this specification.  This section contains only those functions
particular to database manipulation.  General facilities for
starting operations and querying their status are found in the other
section. 

@begin(functionsection)
@begin(function)
DBSTATUS
@f[access_db](db_ident, &db_handle);
@index[access_db]
DATABASE db_handle;
string db_ident;
@end(function)

Establishes a database context for future database calls.  A successful
accessdb call must be executed before any database operations can be done.
The return code DB_OPEN indicates that the database was successfully accessed.
A successful call to accessdb returns a local handle which must be
supplied to subsequent library routines to indicate the database to be
accessed.  Most implementations support simultaneous access to several
databases, but that is not required.  The DB_TOO_MANY_ACCESSED error is
raised when and if implementation defined limits are exceeded.  Each
database should be closed with a 'terminatedb' call when no longer needed.
Failure to explicitly terminate databases may waste resources at the
server, though most implementations will detect improper termination and
do the necessary reclamation.  

The form of the db_ident string depends on the implementation.  For
example, in a network with nameservers, a single string such as
'partsdata' may suffice to identify a database on the network.  When
name resolution services are not available, a string of the form
'databasename@@site' may be required.  There is a question of access
rights and authorization for this operation.  I'm hoping that we can
eventually build on Kerberos, but there may be problems.  Worst of all,
we may have to re-invent Ingres' protection mechanism, which would
involve a very detailed understanding of their operations.  I hope not.

@begin(function)
int
@f[start_accessing_db](op, db_ident, &db_handle);
@index[start_accessing_db]
OPERATION
DATABASE db_handle;
string db_ident;
@end(function)

Same as accessdb, but the program continues to execute (if possible)
while the operation is sent to the server and performed.  See
'complete_operation' and 'op_select.'  All operations and queries are
processed in fifo order.  The value of db_handle is undefined until the
operation_status(pending_operation) is OP_COMPLETE and the
operation_result(pending_operation) is DB_OPEN.

@begin(function)
DBSTATUS
@f[terminate_db](&db_handle);
@index[terminate_db]
DATABASE db_handle;
@end(function)

This routine reclaims server and client resources used to access the
database and severs any connections which may no longer be needed.  Data
which has been retrieved and stored locally remains available.

@begin(function)
OPERATION
@f[start_terminatedb](db_ident);
@index[start_terminatedb]
string db_ident;
@end(function)

Same as terminatedb, but the program continues to execute (if
possible) while the operation is sent to the server and performed.
See 'complete_operation' and 'op_select.'  All operations and queries
are processed in fifo order.

@begin(function)
OPERATION
@f[DB_STATUS](db_handle);
@index[DB_STATUS]
DATABASE db_handle;
@end(function)

Returns the status of the specified database.  Values returned are
DB_OPEN@index[DB_OPEN], indicating that the database is available for use,
or DB_CLOSED@index[DB_CLOSED], indicating that it is not.

@begin(function)
int
@f[perform_db_operation](db_handle,request);
@index[perform_db_operation]
DATABASE db_handle;
string request;
@end(function)

This routine may be used to perform any database operation which does
not require structured data as an argument and which only returns a
return code.  In the case of Ingres, this covers all operations except
retrieve.  Requests are represented as null terminated strings.  In
other words, this library routine sends a simple string command to the
server for execution and returns an integer return code.  Any pending
asynchronous operations on this database are completed before the
supplied operation is attempted.

@begin(function)
int
@f[db_query](db_handle, relation, query_string)
@index[db_query]
DATABASE db_handle;
RELATION relation;
string   query_string;
@end(function)

Executes the query specified by query string,
appending its result to the supplied relation.
An integer return code is returned.  OP_SUCCESS is returned for a
successful query.  The relation is unchanged unless the query
succeeds.  Note that it is possible to have a successful query (i.e.
one which is well formed with respect to the database) which returns
no data;  such a query will return OP_SUCCESS leaving the relation unchanged.

Query_string has the same form as an EQUEL query, except that the
names of returned fields are enclosed in >* ....*< brackets (we can
change the brackets to anything that won't conflict with Ingres
syntax.)  Note that the relation must be 'created' before it may be
given to query_db.  This is so we know typing information for
constructing the actual database query.  The caller may use sprintf or
a similar function to prepare a query string containing variable data.
The selected fields MUST match the names of fields in the relation.
If not, a DB_BAD_FIELD_NAME error is raised before the query is
executed.  Null values are provided for tuple fields not retrieved by
the query.

The types of the referenced fields in the supplied relation must be
drawn from a limited set of INTEGER_T, REAL_T, STRING_T and DATE_T types
(the exact list will be provided in a later version of this
specification document.)  If not, a DB_BAD_FIELD_TYPE error is raised
before the query is executed.  The query will be constructed using the
supplied types.  If Ingres is incapable of doing the appropriate
conversions upon retrieval, then a DB_CONVERSION_ERROR is returned.
If the field type in the relation is STRING_T and Ingres succesfully
converts the retrieved data to character form, then a null is appended
iff there is room at the end.  If the supplied field is too short,
data is quietly dropped on the right end.  The string is allocated by
string_alloc.  The
string may be free'd with string_free, by the routine
'null_tuple_strings', or by the delete_relation routine.  Normally,
space is reclaimed automatically when the corresponding tuple or
relation is freed.  Fields not assigned by the query are set to null
values.

As specified, this routine is incapable of retrieving data of
indeterminate structure, e.g. it cannot achieve the effect of an
e.all retrieve in Ingres.  This sort of function could be layered
upon the base support suggested here, by first querying the Ingres
structure database and then issuing the appropriate second query for
the fields actually in the relation.  This implementation is presumably
slow, but I am reluctant to greatly complicate the design at this
point.  Another approach would be to add a special library routine
db_query_relation which would create a relation, making assumptions
about field representations for the fields actually retrieved.  I'm
inclined to put this off until we see the core of the library up and
running.  It would (probably) not involve a major restructure.

@begin(function)
int
@f[start_performing_db_operation](operation, db_handle,
@index[start_performing_db_operation]
request);
OPERATION operation;
DATABASE db_handle;
string request;
@end(function)

Same as perform_db_operation, but the program continues to execute (if
possible) while the operation is sent to the server and performed.  See
'operation_status, operation_result, complete_operation and
op_select.'  All operations and queries are processed in fifo order.
Callers must insure that the request string
remains valid until the operation completes (be careful about using local
variables.)


@begin(function)
int
@f[start_db_query](operation, db_handle, relation, 
@index[start_db_query]
query_string)
OPERATION operation;
DATABASE db_handle;
RELATION relation;
string   query_string;
@end(function)

Same as query_db, but the program continues to execute (if possible)
while the operation is sent to the server and performed.  See
'operation_status, operation_result, complete_operation and
op_select.'  The relation is undefined during the execution of this
operation.  It may not be referenced or free'd until the operation
completes.  All operations and queries are processed in fifo order.
The value of the relation is indeterminate in the case of failure,
but it is guaranteed to be of legal structure and self-consistent.
Callers must insure that the query string
remains valid until the operation completes (be careful about using local
variables.)
@end(functionsection)

@chapter(Structured Data Management at a Single Site)
@Index(Structured data management)
@label(structdata)

These routines provide a simple, limited facility for managing
structured data on behalf of a single process.  A basic datum is called
a field, and it may be of arbitrary type (including user defined types.)
Named fields may be collected into tuples, and lists of tuples may
form relations.  These constructs are used to represent the data
retrieved from a relational database.  A separate library (Chapter
@ref(trans))
supports transmission of these data structures through a
network.  The typing scheme provided is sufficient to drive the
transformations and re-alignment required when transmitting across
incompatible machine architectures.  It is NOT intended as a general 
purpose extension to the C language.

@section(Type Definition and Manipulation)

The information in this section is relavent to those users wishing to
define their own types.  User defined types are NOT necessary for
typical database retrieval and update activities.  Users desiring just
those services need only know the names of built in system types,
such as @f[INTEGER_T] and @f[STRING_T] which have direct counterparts in an
Ingres database.  @i[In the likely event that you have no need to
define your own types, skip ahead now to section @ref(structypes).]

Each datum or field managed by the system must be of some specified
type.  For each type, the system must have access to certain
information (e.g. alignment restrictions) and to certain routines
(initialize, encapsulate for transmission, etc.)  Limited facilities to
support user defined types are also helpful, since they allow such
services as transmission across heterogeneous machines to be supported
for arbitrary user data (this notion is stolen directly from ARGUS.)
On the other hand, we don't want to tackle the many complicated
problems inherent in a general solution to the dynamic typing problem.
Instead, we employ the following simple, ad hoc mechanism, which seems
to be sufficient for our needs.  

The system supports a set of types identified signed integers (e.g.
type 1 might be STRING_T, type 2 might be DATE_T, etc.)  Out of this
name space, the first few starting from 0 are reserved for the system
defined types, and the rest, up to GDB_MAX_TYPES, are available for
dynamic assignment of user defined types.  Appropriate pre-processor
symbols, such as INTEGER_T, are provided in an include file for system
types.

For each type, certain information must be available to the system at
execution time.  This information is coded as a two dimensional array,
each element of which holds either an integer, a pointer to a string,
or a function pointer.  One index selects the type, the other the
property.  For example, property number 6 of each type might be a code
for its alignment requirement (4 for fullword, etc.).  Using the
example above, entry [2,6] in the type definition array would give the
alignment rule for a field of type DATE_T.  In certain cases, the
information to be coded is a pointer to a routine.  For example, a
NULL value routine is provided for each type which can be used to initialize
fields of that type.

In order to create a user defined type, it is necessary to code
functions for each of the necesary properties (e.g. the function which
computes null values) and then use the "create_type" function to
register the new type in the type tables.  "create_type" returns the
integer code assigned to the new type.  WARNING:  create_type is not
yet implemented.

@subsection(Information Required for Each Type)

The simple mechanism described above is sufficient to encode the
properties of each type.  The information currently required is:

@begin(itemize)
LENGTH_PROPERTY: the length of the field 

ALIGNMENT_PROPERTY: an integer code for the alignment rule for the
type (8 for doubleword, etc., may vary by machine type.)

NULL_PROPERTY: a routine to initialize a null value of the type

CODED_LENGTH_PROPERTY: a routine to calculate the length of the
field when encoded for transmission.


ENCODE_PROPERTY: a routine to encode the field for transmission

DECODE_PROPERTY: a routine to decode tranmitted data into local 
representation

FORMAT_PROPERTY: a routine to format the field (optional, for debugging)

NAME_PROPERTY: a string name for the type.
@end(itemize)

For system defined types, this information is provided automatically by
include files and libraries which initialize the system type definition
array.  Interface specifications for the various routines will be
provided in a later version of this specification.  For the moment, 
the code in source file @f[gdb_stype.c]@Index[gdb_stype.c] provides a 
useful model. 

@newpage
@section(C Typedefs Relating to Data Structuring)
@label(structypes)

The following C types are defined in include files for use with the
routines listed below.  These should not be confused with types
declared using the mechanisms above, which the C compiler knows nothing
about.

@begin(table)
@caption(Typedefs for Data Structuring)
@tabset(.75in, 2.5in)
@\TUPLE_DESCRIPTOR@index[TUPLE_DESCRIPTOR]@\information needed to describe the structure
@\@\of a given tuple.

@\TUPLE@index[TUPLE]@\a handle on an actual tuple.  This is always
@\@\a pointer, and it may be manipulated
@\@\accordingly (e.g. its length may be presumed
@\@\small, and NULL is used for non-existent
@\@\TUPLES.)

@\RELATION@index[RELATION]@\a handle on an actual relation.  This is always
@\@\a pointer, and it may be manipulated
@\@\accordingly.

@\FIELD_TYPE@index[FIELD_TYPE]@\a type code for a field...generally
@\@\represented as a signed integer (see above)
@end(table)
	
@section(Routines)

(Note some of these may be implemented as macros.  Where such
implementation is presumed, the name is put in UPPERCASE.  This
allusion to Unix convention is meant to suggest that the associated
operations are extremely fast, and that they are likely to suffer/gain
from the semantic differences which distinguish macros from C
functions.  Note also that some of the macros contain define statements
in C of the form @f[{}], these must be used without semicolon's or
a syntax error will occur.)

@begin(functionsection)
@begin(function)
TUPLE_DESCRIPTOR
@f[create_tuple_descriptor](number_of_fields, name_list,
@index(create_tuple_descriptor)
					 type_list)
string name_list[];
FIELD_TYPE type_list[];
int    number_of_fields;
@end(function)

Allocates and fills in a tuple descriptor data structure.  The
descriptor records the number of fields in the tuple, the length in
bytes of each field, and a pointer to the type descriptor of each
field.  Each field's offset is automatically assigned as the sum of
the offsets of the fields preceeding it, adjusted according to the
alignment condition found in the type descriptor.  Note that the type
named STRING_T has a reserved meaning in some contexts (see below).
Specifically, string data is generally stored discontiguously from the
tuple itself; the tuple holds only control information and a pointer
to the string.  Space for tuple descriptors is obtained using
db_alloc.  The tuple_descriptor reference count is set to 1.

@begin(function)
void
@f[delete_tuple_descriptor](tuple_descriptor)
@index(delete_tuple_descriptor)
@end(function)

Decrements the reference count, and if it goes to zero,
releases the memory occupied by the tuple descriptor.

@begin(function)
TUPLE
@f[create_tuple](tuple_descriptor)
@index(create_tuple)
@end(function)

Allocates space for a new tuple, and initializes its description.  The
values of fields in the tuple are not initialized.  String types are
not distinguished in this operation; no space is allocated for the
target of a string pointer.  The reference count of the tuple
descriptor is incremented.

@begin(function)
void
@f[delete_tuple](tuple)
@index[delete_tuple]
TUPLE tuple;
@end(function)

Deallocates the space for the specified tuple.  Neither the tuple descriptor
nor any of the string data which may be referenced by the tuple is
deleted.  The reference count of the tuple descriptor is decremented,
and if necessary, the descriptor is deleted too.

@begin(function)
void
@f[initialize_tuple](tuple)
@index[initialize_tuple]
TUPLE tuple;
@end(function)

Sets each field in a tuple to its null value.

@begin(function)
void
@f[null_tuple_strings](tuple)
@index(null_tuple_strings)
TUPLE tuple;
@end(function)

Reclaims the space for all fields in the tuple whose type is STRING_T.
The value of the string itself is set to uninitialized.  The sequence
"null_tuple_strings(t); delete_tuple(t);" may be used to reclaim all
the space used for a tuple containing string fields.

@begin(function)
RELATION
@f[create_relation](tuple_descriptor);
@index[create_relation]
TUPLE_DESCRIPTOR tuple_descriptor;
@end(function)

Creates an empty relation.  Tuples subsequently added to the relation
must contain the fields described in the tuple descriptor at the time
the relation is created.  Not sure yet whether this will be checked.
May just create a generalized relation in which each tuple is
self_describing and the consistent case is viewed as a subset.
The tuple descriptor reference count is incremented.


@begin(function)
int
@f[delete_relation](rel);
@index[delete_relation]
RELATION rel;
@end(function)

Reclaims the space for the specified relation, which should not be
used again by the caller.  All tuples are deleted, along with any strings
which have been allocated in the tuples. The caller must insure that the
contents of any string fields are self consistent, i.e. if the string is 
non-null, then it must refer to data which was properly allocated by gdb.
The tuple descriptor reference count is decremented, and if necessary,
the descriptor is deleted.

@begin(function)
void
@f[ADD_TUPLE_TO_RELATION ]@index[ADD_TUPLE_TO_RELATION](relation, tuple)

void
@f[ADD_TUPLE_AFTER_TUPLE] (relation, new_tuple, 
@index[ADD_TUPLE_AFTER_TUPLE]
			     previous_tuple)

RELATION relation;
TUPLE tuple, new_tuple, previous_tuple;
@end(function)

These operations insert a new tuple into a relation.  They do NOT
allocate storage for the tuple, because there may be times when it is
convenient to insert an existing tuple into a relation.  These
routines will probably be implemented as macros, hence the uppercase
names.  ADD_TUPLE_TO_RELATION places the new tuple after all those
already in the relation.  Each tuple may in in only one relation at a
time.  This routine may or may not check for conformity between
the description of the tuple and the description of the relation.  A
given tuple may be in only one relation at a time; this is not
checked.  Note:  to insert a tuple as the first one in the relation, 
provide the relation pointer as the previous tuple, e.g. 
ADD_TUPLE_AFTER_TUPLE(r, t, (TUPLE)r).

@begin(function)
TUPLE
@f[REMOVE_TUPLE_FROM_RELATION](relation, tuple)
@index[REMOVE_TUPLE_FROM_RELATION]
RELATION relation;
TUPLE tuple;
@end(function)

Removes the designated tuple from the relation (this function may 
be implemented to remove the tuple from whichever relation it is in,
or it may check which relation it is in, or it may presume that the
correct relation is specified.  No checking need be done;  caller
is responsible for insuring that the tuple is indeed in the 
specified relation.)  The tuple is not de-allocated, merely removed
from the relation.  It may subsequently be inserted in a different
relation.  


@begin(function)
RELATION
@f[tuples_in_relation](rel);
@index[tuples_in_relation]
RELATION rel;
@end(function)

Returns a count of the tuples in the supplied relation.

@begin(function)
TUPLE_DESCRIPTOR
@f[DESCRIPTOR_FROM_TUPLE](tuple)
@index[DESCRIPTOR_FROM_TUPLE]

TUPLE_DESCRIPTOR 
@f[DESCRIPTOR_FROM_RELATION](relation)

@index[DESCRIPTOR_FROM_RELATION]
TUPLE tuple;
RELATION relation;
@end(function)

Given a tuple or relation, return the descriptor of its entries.

@begin(function)
int
@f[field_index](tuple_descriptor, field_name)
@index[field_index]
TUPLE_DESCRIPTOR tuple_descriptor;
string		 field_name;
@end(function)

Given a tuple descriptor and the string name of a field in the tuple,
this routine returns the 0 relative index of the field in the tuple.
If the named field is not in the tuple, -1 is returned.

@begin(function)
(?? *)
@f[FIELD_FROM_TUPLE](tuple, field_index);
@index[FIELD_FROM_TUPLE]
TUPLE 	tuple;
int   	field_index;
string 	field_name;
@end(function)

Given the relative index of a field in a tuple (0 origin), return a
pointer to the first byte of data in the field.  See also @f[field_index].

@begin(function)
int
@f[FIELD_OFFSET_IN_TUPLE](tuple_descriptor, field_index);
@index[FIELD_OFFSET_IN_TUPLE]

FIELD_TYPE 
@f[FIELD_TYPE_IN_TUPLE](tuple_descriptor, field_index);

@index[FIELD_TYPE_IN_TUPLE]
@end(function)

Given the type descriptor for an entire tuple and the name of a field
in the tuple, these routines return the byte offset, and the
field_type of the named field in the tuple.  Their results are
undefined in the case where the field_index is negative or too large.
Note, the offset returned is from the start of the tuple data, NOT from
the start of the tuple itself.  To address the field, the construction:
(tup->data)+offset gives the true addresss of the first byte of the field.
This may change in the future, since it seems to require an unnecessary
knowledge of the tuple structure on the part of the programmer.

@begin(function)
TUPLE
@f[FIRST_TUPLE_IN_RELATION](relation)
@index[FIRST_TUPLE_IN_RELATION]

TUPLE 
@f[NEXT_TUPLE_IN_RELATION](relation,tuple)
@index[NEXT_TUPLE_IN_RELATION]

TUPLE 
@f[PREV_TUPLE_IN_RELATION](relation,tuple)
@index[PREV_TUPLE_IN_RELATION]

RELATION relation;
TUPLE tuple;
@end(function)

Returns tuples from a relation relative to the order in which they were
inserted.  NULL is returned for the successor of the last tuple or the
predecessor of the first tuple in a relation.

@begin(function)
void
@f[print_relation](name, relation)
@index[print_relation]

void 
@f[print_tuple](name, tuple)

@index[print_tuple]

void 
@f[print_tuple_descriptor](name, desc)
@index[print_tuple_descriptor]

string name;
RELATION relation;
TUPLE    tuple;
TUPLE_DESCRIPTOR desc;
@end(function)

These routines format the contents of the specified relation or tuple
and write the results to the gdb logging file descriptor.  The
supplied name, an ordinary C null terminated string, is used to label
the printed entry in the log.  These routines are intended primarily
for debugging.
@end(functionsection)

@chapter(Structured Data Transmission Services)
@index(Data transmission)
@index(Transmission of data)
@index(Structured data transmission)
@label(trans)

These services are used to move structured data between communicating
programs.  To provide some context for this discussion, we summarize
here the basic techniques used to create servers and clients, and
introduce the use of OPERATIONS for handling asynchronous activities.
This discussion partially duplicates that of Chapter
@ref(serverclient), which is devoted to server/client management, and
general techniques for managing asynchronous operations in GDB.  The
emphasis in this chapter is on transmission of structured data.

@section(Background)

Two styles of communicating program are supported: peer-to-peer, in which
the two sides of the connection are considered to be symmetrical, and
client/server, in which a server is pre-started, awaiting connections from
one or more clients.  The following types are used by the structured data
transmission services:

@begin(itemize)

@begin(multiple)
CONNECTION

represents a communication path to some other program using
GDB.  Usually, this is a network communication path between
programs on different machines.    Connections may
be closed explictly by issueing a "sever_connection", and they
are implicitly closed when the process owning the
connection exits.	 
@end(multiple)

@begin(multiple)
OPERATION

GDB is capable of managing asynchronous activities on its 
CONNECTIONS.  When a program wishes to do asynchronous
communication it creates an OPERATION to represent the state
of the asynchronous activity.  GDB provides functions and
macros for manipulating OPERATIONS.  For example, 
OP_DONE(operation) returns true iff the operation has
completed.
@end(multiple)
@end(itemize)

@section(Service and Network Addresses)

In the current implementation, the address of a GDB program
running on the network has the form:

@begin(example)
		"hostname:servicename"
@end(example)

where hostname is the internet name of the host on which the program
is running, and servicename is the name of the service as registered
in @f[/etc/services]@index[/etc/services].  Services should be of type
tcp.  An alternate form of servicename is #portnumber, where
portnumber is the integer number of the Berkeley port to
be used for the service.  Use of explicit port numbers is
discouraged, except for testing.   Examples:
"myhost:dbserver" or "yourhost:#1234".  All addresses are
represented as null terminated strings in C.
	
	

@section(Establishing Communication)

As noted above, there are two general styles of communicating
programs.  Peer-to-peer communication is started by use of the
following function.  These services and other related routines are
fully documented in Chapter @ref(serverclient).  The discussion here
is intended to introduce the GDB services which transmit structured
data.

@begin(functionsection)

@begin(function)
CONNECTION
@f[start_peer_connection](service_address)
@index[start_peer_connection]
string service_address;
@end(function)

Creates a connection to a peer at the designated address (see service and
network addresses above.)  Either peer may start first.  This routine will
hang indefinitely waiting for its peer, unless a fatal error is
encountered.  There is currently no asynchronous form of this service.
NULL is returned in case of an error.  Messages describing some errors are
written to stderr, but otherwise, there is currently no way for the program
to determine why a failure occurred. 
@end(functionsection)

Clients may request a connection to a server by using the following
function:

@begin(functionsection)
@begin(function)
CONNECTION
@f[start_server_connection](service_address, parms)
@index[start_server_connection]
string service_address;
string parms;
@end(function)

Rules are the same as for start_peer_connection except:  (1) The server must be
started and listening at the time the connection is attempted, or this
request will fail.  (2) The supplied parms are made available to the
server, which may use them in deciding whether to accept the connection or
how to process it.  This request will generally not hang for very long, but
it does set up a socket and do a limited amount of communication
synchronously.  Typical delays on a local network with no gateways are a
fraction of a second.  Note that GDB supports several styles of server
(e.g. forking and non-forking), but clients connect to and communicate with
all of them in the same manner.

@end(functionsection)

The techniques for creating servers are described in Chapter
@ref(serverclient).

@section(Synchronous Communication)

GDB is capable of transmitting data of any of the types described in
Chapter @ref(structdata).  This means that the services below may be
used to send data as simple as a single integer, or as complex as an
entire relation with all of its tuples and fields.  In all cases, data
is automatically converted to the appropriate representation when it
is transmitted between machines of different architecture.

@begin(functionsection)

@begin(function)
int
@f[send_object](con, obj_ptr, type_id)
@index[send_object]
CONNECTION  con;
char 	    *obj_ptr;
FIELD_TYPE  type_id;
@end(function)

Synchronously transmits the specified data on the connection con.  Obj_ptr
must be the address of the object to be sent, and type_id must indicate the
type of the object to be sent.  The final status of the transmit operation
is returned.  In general, OP_SUCCESS indicates success, OP_CANCELLED
indicates an error.   OP_CANCELLED generally implies that the corresponding
connection has broken and should be severed.  Examples:

@begin(example)
rc = send_object(server, &my_int, INTEGER_T);   
			/* send an integer */
rc = send_object(peer, &tuple, TUPLE_T);        
			/* send a tuple */
rc = send_object(peer, &rel, RELATION_T);       
			/* send a relation */
@end(example)

@begin(function)
int
@f[receive_object](con, obj_ptr, type_id)
@index[receive_object]
CONNECTION  con;
char 	    *obj_ptr;
FIELD_TYPE  type_id;
@end(function)

Synchronously receives the specified data on the connection con.  Obj_ptr
must be the address of the object to be received, and type_id must indicate the
type of the object.  The final status of the receive operation
is returned.  In general, OP_SUCCESS indicates success, OP_CANCELLED
indicates an error.   OP_CANCELLED generally implies that the corresponding
connection has broken and should be severed.  Examples:

@begin(example)
rc = receive_object(server, &my_int, INTEGER_T);
			/* receive integer */
rc = receive_object(peer, &tuple, TUPLE_T);     
			/* receive a tuple */
rc = receive_object(peer, &rel, RELATION_T);    
			/* receive relation */
@end(example)

Note that receipt of certain structured objects implies that local memory
is dynamically allocated to hold the newly received objects.  The system
defined types for which this is the case incude: TUPLE_T, RELATION_T,
TUPLE_DESCRIPTOR_T, and STRING_T.  TUPLE_DATA_T implies memory allocation
only if the fields in the tuple require it.
@end(functionsection)

@section(Asynchronous communication)

GDB provides a general architecture for executing asynchronous
communication activities on a connection.  Several asynchronous operations
are provided with the system, and sophisticated users can write their own
(it's a little tricky, but it can be done.)   Most users will find the
supplied operations to be sufficient.

Each asynchronous activity is known as an operation.  Programmers using the
asynchronous capabilities of GDB must declare an OPERATION to keep track of
the state of each asynchronous activity. For example:

@begin(example)
  	OPERATION send_op1, send_op2, receive_op;
@end(example)

declares 3 operations.   Before an operation can be used it must be
"created":

@begin(example)
	send_op1 = create_operation();
@end(example)

The newly allocated 'operation' may then be used to track the state of an
asynchronous activity, and if desired, it may later be reset and re-used
without being re-created.  These techniques lower the cost of doing
repeated asynchronous activities.  Many applications can create all their
operations at start-up, resetting and re-using them as necessary.

Each of the asynchronous routines described below takes an OPERATION as an
argument.  In each case, the supplied operation should be newly created or
reset, and in no case may it be in use by another routine;  each
OPERATION tracks the state of one activity at a time.  The simplest way to
synchronize asynchronous activity is with the 'complete_operation'
function:

@begin(example)
	complete_operation@index[complete_operation](send_op1);
	complete_operation(send_op2);
@end(example)

Note that GDB is coded to maximize throughput and avoid deadlock when
several activities are proceeding simultaneously.  For example, the two
calls shown above can be done in either order, since send_op2 progresses
even while waiting for completion of send_op1.   A simple way to do
parallel communication on several connections is to start each of the
activities and then wait for completion of all of them.  Each progresses as
fast as possible and as nearly in parallel as GDB can manage.  GDB uses
Berkeley 'select' to avoid spinning when work cannot proceed.

In addition to 'complete_operation', several routines are provided which
can wait for any of a list of operations to complete, and which can combine
a GDB wait with the facilities of a Berkeley 'select' on fd's not
controlled by GDB.  These features are documented at the end of this
chapter.

Operations executed on a given half connection are executed in FIFO order,
unless specifically indicated to the contrary.  (A half connection refers
to either the inbound or outbound stream of a given connection; send and
receive requests are queued independently.) A synchronous request as
executed only after the completion of all prior asyncrhonous requests on
the same half connection. 

An operation which is attempted on a failed connection, or an operation
which encounters a connection error during execution returns the status
@f[OP_CANCELLED].   Severing a connection implicitly cancels any operations
which are queued on that connection.  

GDB generally makes as much progress as it can whenever the
communication layer is given control.  For example, whenever a new
operation is queued on a given connection, all connections are allowed
to proceed as far as they can.  When a complete_operation is issued,
all activities proceed while awaiting completion of the specified
operation.  However, gdb only progresses when explictly given control;
it does not use SIGIO@index[SIGIO].  For this reason, GDB programs
should hang in a 'complete_operation' or in one of the forms of
'op_select' when there is no work to be done, as this will allow GDB
activities to proceed.  If the user issues his/her own select, sleep
or a wait, GDB activities may be delayed.

@subsection(Asynchronous Data Transmission)
@label(async)
@index(Asynchronous data transmission)

The following routines are similar to send_object and receive_object, but
they do their work asynchronously.

@begin(functionsection)
@begin(function)
int
@f[start_sending_object](op, con, obj_ptr, type_id)
@index[start_sending_object]
OPERATION op;
CONNECTION  con;
char 	    *obj_ptr;
FIELD_TYPE  type_id;
@end(function)

Asynchronously transmits the specified data on the connection con.
Obj_ptr must be the address of the object to be sent, and type_id must
indicate the type of the object to be sent.  The final status
(OP_STATUS(OP)) of the transmit operation is not available until the
operation 'op' completes.  In general, OP_COMPLETE indicates success,
OP_CANCELLED indicates an error.  OP_CANCELLED generally implies that
the corresponding connection has broken and should be severed.
OP_RESULT of a successful transmission will be OP_SUCCESS.
Example:

@begin(example)
op1 = create_operation();
op2 = create_operation();
op3 = create_operation();
start_sending_object(op1, server, &my_int, INTEGER_T); 
start_sending_object(op2, peer, &tuple, TUPLE_T);      
start_sending_object(op3, peer, &rel, RELATION_T);     
compete_operation(op1);
compete_operation(op2);
compete_operation(op3);
if (OP_STATUS(op1) == OP_CANCELLED ||
    OP_STATUS(op2) == OP_CANCELLED ||
    OP_STATUS(op2) == OP_CANCELLED)
	printf("Couldn't do it\n");
@end(example)

@begin(function)
int
@f[start_receiving_object](op, con, obj_ptr, type_id)
@index[start_receiving_object]
OPERATION op;
CONNECTION  con;
char 	    *obj_ptr;
FIELD_TYPE  type_id;
@end(function)

Asynchronously receives the specified data on the connection con.
Obj_ptr must be the address of the object to be received, and type_id
must indicate the type of the object.  The final status of the receive
operation is not available until the operation 'op' completes.  In
general, an OP_STATUS ofOP_COMPLETE indicates success, OP_CANCELLED
indicates an error.  OP_CANCELLED generally implies that the
corresponding connection has broken and should be severed.  OP_RESULT
of a successful receipt will be OP_SUCCESS.  Example:

@begin(example)
op1 = create_operation();
op2 = create_operation();
op3 = create_operation();
start_receiving_object(op1, server, &my_int, INTEGER_T); 
start_receiving_object(op2, peer, &tuple, TUPLE_T);      
start_receiving_object(op3, peer, &rel, RELATION_T);     
compete_operation(op1);
compete_operation(op2);
compete_operation(op3);
if (OP_STATUS(op1) == OP_CANCELLED ||
    OP_STATUS(op2) == OP_CANCELLED ||
    OP_STATUS(op2) == OP_CANCELLED)
	printf("Couldn't do it\n");
@end(example)
@end(functionsection)

@chapter(Memory Management)
@Index(Memory management)
@Index(Storage allocation)

The following routines are provided to encapsulate calls to memory
management services for all of the libraries defined in this
specification.  Early implementations will just use malloc, but
more sophisticated techniques might be used in the future.

@begin(functionsection)
@begin(function)
(?? *)
@f[db_alloc](bytes)
@index[db_alloc]
int bytes;
@end(function)

Allocates the number of bytes requested and returns the corresponding
pointer.  The allocated space is always word aligned.

@begin(function)
void
@f[db_free](ptr, bytes)
@index[db_free]
(??) *p;
int bytes;
@end(function)

De-allocates the memory pointed to by ptr.  Some implementations by
ignore the supplied size, requiring that ptr be a value obtained from
db_alloc.   Others may support more flexible pool management for 
which the size will be useful.
@end(functionsection)

@section(Overriding the default memory management routines)

Two global function pointers are provided which point to the allocation
and free routines which are actually called when db_alloc and db_free
are used.  A user of gdb may supply his or her own memory management
routines by replacing the supplied pointers with pointers to new routines.
The global variables to be changed are:

@begin(format)
@tabset(.75in, 2.5in)
@\@f[gdb_amv]@index[gdb_amv]@\vector to the allocate memory routine

@\@f[gdb_fmv]@index[gdb_fmv]@\vector to the free memory routine
@end(format)

The pointers should be to routines matching the specification for
@f[db_alloc]@index[db_alloc] and @f[db_free]@index[db_free] above.
The pointers should be changed prior to calling
gdb_init@index[gdb_init] to insure that all memory allocated by gdb is
obtained using the new services.

@chapter(String Management)
@Index(String management)

These routines provide allocation and de-allocation services for
STRING_T data.   Like db_alloc and db_free, their
primary raison d'etre is to centralize creation and deletion of
strings.

The only special TYPEDEF used for string management is @f[STRING].
STRING describes a datum of GDB type STRING_T.  This must contain
sufficient information to record the location of the data
(e.g. a pointer) and the length with which the data
was allocated (which need not be the length of null
terminated data actually contained in the field.)

@section(Routines)

@begin(functionsection)
@begin(function)
int
@f[string_alloc](stringp,bytes);
@index[string_alloc]
STRING *stringp;
int bytes;
@end(function)

Allocates memory to contain a string of the specified length
and returns the associated string handle.  The supplied length should
include space for a terminating null if one is to be used, but these
libraries make no such presumption.  A null is stored in the first byte
of the returned data area.  

@begin(function)
void
@f[string_free](stringp);
@index[string_free]
STRING *stringp;
 
De-allocates the supplied string.  The length given must match that used
to allocate the space with string_alloc (which need not be the same as
the length of the null terminated data currently in the string.)
@end(function)

@begin(function)
(char *)
@f[STRING_DATA](string)
@index[STRING_DATA]
STRING string;
@end(function)

A macro which returns the pointer to the first byte of the referenced
string.  NULL is returned for an uninitialized string.

@begin(function)
int
@f[MAX_STRING_SIZE](string)
@index[MAX_STRING_SIZE]
STRING string;
@end(function)

A macro which returns the total number of bytes available in the string
(which may be greater than the length of any null terminated data which
happens to reside there at the moment.)
@end(functionsection)

@Chapter(Server and Communications Management)
@label(serverclient)
@Index(Server and communication management)

These are routines whose purpose is to facilitate the creation of shared,
centralized, or peer-to-peer network services in a Berkeley Unix system.
At the server, they manage the creation of sub-processes for a given client
or sub-service, and the allocation and use of sockets for communication to
the clients.  A corresponding set of routines is used by clients to request
and maintain sessions with various services.

The following types are used for server and communications management:

@begin(table)
@caption(Types used in Server and Communications Management)
@tabset(.75in, 2.5in)
@\@f[CONNECTION]@Index[CONNECTION]@\Represents an ongoing full duplex
@\@\connection to a central service.

@\@f[CONN_STATUS]@Index[CONN_STATUS]@\a returned value indicating the
@\@\disposition of anattempted 
@\@\library call.

@\@f[OPERATION, OPERATION_DATA]@Index[OPERATION]@Index[OPERATION_DATA]
@\@\Represents an ongoing or completed 
@\@\operation requested of 
@\@\the server or transport system.
@\@\OPERATION is a handle
@\@\on OPERATION_DATA.

@\@f[LIST_OF_OPERATIONS]@Index[LIST_OF_OPERATIONS]
@\@\A collection of OPERATIONs.

@\@f[OP_STATUS]@Index[OP_STATUS]@\A return value describing the
@\@\progress of an asynchronous
@\@\operation.
@end(table)

@section(Host and service names)
@Index(Host names)
@Index(Service names)
@Index(Naming hosts)
@Index(Naming services)
@Index(Naming communicating programs)

In the current implementation, the address of a GDB program running on the
network has the form:

@begin(example)
 	"hostname:servicename"
@end(example)

where hostname is the internet name of the host on which the program
is running, and servicename is the name of the service as registered
in @f[/etc/services]@index[/etc/services].  Services should be of type
tcp.  An alternate form of servicename is #portnumber, where
portnumber is the integer number of the Berkeley port to be used for
the service.  Use of explicit port numbers is discouraged, except for
testing.  Examples: "myhost:dbserver" or "yourhost:#1234".  All
addresses are represented as null terminated strings in C.
	
The form of GDB addresses may evolve over time.  For example, host names
may disappear when better nameservers become available.	

@section(Routines for Use at Client)

@begin(functionsection)
@begin(function)
CONNECTION
@f[start_server_connection](server_id, parms)
@index[start_server_connection]
string 	server_id;
string  parms;
@end(function)

Sets up a connection to a server process at the server site.  server_id
identifies the service; its form is specified above.  This routine
allocates a local socket and negotiates with the server to bind it to a
socket owned by the appropriate server process.  The parms are made
available to the server.

@begin(function)
CONNECTION
@f[start_peer_connection](peer_id)
@index[start_peer_connection]
string 	peer_id;
@end(function)

Sets up a connection to a peer process, which may be at a remote site.
peer_id identifies the process to which the connection is to be made; its
form is specified above.    This routine allocates a
local socket and negotiates to bind to a socket owned by the appropriate
peer process.  Returns NULL if the connection could not be started.
Right now, there is no way to tell why the connection attempt failed.  This
should be fixed in subsequent versions of the spec.

@begin(function)
CONN_STATUS
@f[connection_status](connection)
@index[connection_status]
CONNECTION connection;
@end(function)

Returns the status of the indicated connection.  Possible return values are:
CONN_STOPPED (never started or severed by user), CON_UP (currentfly usable),
CON_STARTING (transient state on way up), CON_STOPPING(transient state on
way down--user is expected to check connection_errno and sever the connection.)

@begin(function)
int
@f[connection_errno](connection)
@index[connection_errno]
CONNECTION connection;
@end(function)

Returns the Unix errno which resulted in this connection being stopped.
While errno may be queried for any connection, it's value is meaningful
only for a connection in the CON_STOPPING state.

@begin(function)
int
@f[connection_perror](connection, msg)
@index[connection_perror]
CONNECTION connection;
char *msg;
@end(function)

Does the equivalent of the Unix perror library routine except (1) the
value of errno is that which caused this connection to stop and (2)
the error message is written to gdb_log, which may or may not be
stderr.  This routine does nothing of connection is NULL or if
connection_status(connection) != CON_STOPPING.

@begin(function)
CONNECTION
@f[sever_connection](connection)
@index[sever_connection]
CONNECTION connection;
@end(function)

Applications should use this routine to terminate a connection.  It may be
called on any active or stopping connection.  The connection is terminated,
all pending operations are completed or (more likely) cancelled, and the
associated descriptor is released.  This routine always returns NULL.  It
is the applcation's responsibility to NULL its own connection variable, which
may conveniently be done with a construction of the form:  

@begin(example)
	peer = sever_connection(peer);
@end(example)

which has the effect of terminating the peer connection and safely nulling
the applications pointer.  This is important because connection 
descriptors are re-used when new connections are started.  The old value
in a connection variable should NEVER be used after a sever.  In particular,
the construction:

@begin(example)
		connection_status(severed_connection)
@end(example)

is safe only when the application can insure that no new connections have
been started since the sever.

Sever_connection is the inverse of start_peer_connection and
start_server_connection.  It informs the foreign process that the client is
done with the connection, and releases all resources associated with the
connection.  
@end(functionsection)

@section(Routines for Use in Forking Servers)
@Index(Forking servers)
@Index(Servers, forking)

@begin(functionsection)
@begin(function)
CONNECTION
@f[create_forking_server](servicename, validate-rtn)
@index[create_forking_server]
string servicename;
int *validate-rtn();
@end(function)

Turns the current process into a forking style server.  The validate
routine, if supplied, is called before a connection is accepted to allow
the server to screen clients.  (Interface to be documented later.)  GDB
handles all management of connections, forking, child reaping, and the
like.  No return is ever made in the parent, which remains under the
control of GDB forever.  GDB will take care of terminating children
whenever the parent is killed.  As new clients are received, GDB does the
necessary creation of connections and returns in the child processes.  The
connection returned by this call is to be used in communicating with the
client.  Because a true fork is done, global data may be inherited from the
master process.  In addition, the global variable gdb_client_tuple is 
initialized to contain four string fields named "server_id", "parms",
"host", and "user".
The first is the string name of the server as passed by the client, and the
second is the parms string supplied by the client.  The last two are the
string names of the client host and user, as best GDB could determine them.
In some cases, GDB may supply "????" for either of these fields if
the true values cannot be determined. The child
may use the full facilities of GDB to communicate with its client, and when
communication is finished, the child should sever its connection (not
stricly required, but it's a good thing to do) and exit.  The server will
appropriately reap the child process.  Note that the servicename supplied
to this routine is NOT a full network address, since the host is implicitly
local.  See service addressing discussion above.

As an aid to debugging, the GDB_NOFORK@index[GDB_NOFORK] flag may be
set to prevent this routine from forking when a client arrives.  See
@f[gdb_debug]@index[gdb_debug].
@end(functionsection)

@section(Routines for Use in Non-Forking Servers)
@begin(functionsection)
@begin(function)
int
@f[create_listening_connection](servicename)
@index[create_listening_connection]
string servicename;
@end(function)

Creates a special 'listening' connection on which start_accepting_client
operations (see below) may be queued.  Using these two facilities together,
a server may asynchronously wait for and acquire connections to new clients
while continuing to service existing clients.  Servicename has the same
forms as for create_forking_server.

@begin(function)
int
@f[start_accepting_client_connection](listencon, op, 
		&clientcon, &otherside
@index[start_accepting_client_connection]
		&othersize, &client_tuple);
CONNECTION listencon;
OPERATION op;
CONNECTION clientcon;
struct sockaddr_in otherside;
int othersize;
TUPLE client_tuple;
@end(function)

Begins the process of asynchronously acquiring a tentative connection from
a client.  When this operation completes successfully, clientcon contains
the connection to the new client, and client_tuple contains the request tuple
(which includes the address requested and the client supplied parms).
The otherside and othersize fields are provided for those programmers who
need access to the Berkeley supplied address of the client.  These are the
same as the values returned by a Berkeley 'accept' call.  Most callers will
not need these, but it is required that appropriate parameters be supplied.
otherside may be a character array of length 100 (less will probably do)
and othersize should be an integer variable set to sizeof(otherside).  The
actual size of the client address is returned, a la Berkeley accept.

@begin(function)
int
@f[start_replying_to_client](op, con, disposition, newaddr, parms)
@index[start_replying_to_client]
OPERATION op;
CONNECTION con;
int	disposition;
string newaddr;
string parms;
@end(function)

The first thing a server must do after successful competion of a
start_accepting_client is to reply to the client, indicating whether its
request is being accepted.  The reply is sent on the new client connection
using start_replying_to_client.  Disposition takes one of three values:
GDB_ACCEPTED, GDB_REFUSED, or GDB_FORWARDED.  In most cases, newaddr and
parms should be null strings ("").  GDB_ACCEPTED completes the process of
creating the connection from client to server.  Once the start_replying
operation is complete, the client connection is available for data
transmission.  GDB_REFUSED indicates that the server declines to talk to
the client, and if desired, it may indicate a reason in the parms string.
The server should wait for completion of the start_replying_to_client, then
sever the client connection.  GDB_FORWARDED is used to tell the client that
the service being requested has moved, and to suggest a new address for the
client to try.  The suggested address should be supplied in newaddr in the
hostname:servicename form shown above.  The 'start_server_connection' which
the client has done takes care of the forwarding request automatically and
transparently.
@end(functionsection)

@section(Routines for Manipulating Operations)

@begin(functionsection)
@begin(function)
OPERATION
@f[create_operation]();
@index[create_operation]
@end(function)

Allocates an operation descriptor and returns a pointer to it.  Note
that the type OPERATION is just a handle, and the system assumes that
operation data structures remain allocated for as long as they are
needed.  Knowledgeable application programmers can allocate their own
structures of type OPERATION_DATA and use the macro OPERATION_FROM_DATA
to create the necessary OPERATIONS efficiently.  create_operation is
appropriate where dynamic allocation is desired or for convenience in
simple applications.

@begin(function)
int
@f[delete_operation](operation)
@index[delete_operation]
OPERATION operation;
@end(function)

Releases space for the specified operation;

@begin(function)
OPERATION
@f[OPERATION_FROM_DATA](op_data)
@index[OPERATION_FROM_DATA]
OPERATION_DATA op_data;
@end(function)

Given the operation data, this macro returns the corresponding OPERATION
handle.

@begin(function)
Bool
@f[OP_DONE](operation)
@index[OP_DONE]
OPERATION operation;
@end(function)

True iff the operation is OP_COMPLETE or OP_CANCELLED.

@begin(function)
int
@f[OP_TAG](operation)
@index[OP_TAG]
@end(function)

Every operation initiated by a given process has a unique integer tag
which identifies it.  This macro returns the tag for a given operation.
Tags are re-assigned each time the queue_op routine is called.
Applications programmers will seldom have need to query or manipulate the
tags directly; they are an internal mechanism used primarily to track
operations which are being cancelled prematurely.  Applications
programmers may occasionally find them useful as a means of generating
operation identifiers which are guaranteed unique over the life of the
process. 

@begin(function)
OP_STATUS
@f[complete_operation](pending_operation)
@index[complete_operation]
OPERATION pending_operation;
@end(function)

Waits unconditionally for completion of the specified operation.  If
several operations are outstanding, then waiting for a given operation
implies a wait for all of its predecessors.  There is a separate queue
of operations for each connection, so the term predecessor is defined
only with respect to a given connection.  Operations pending on other
connections DO proceed while awaiting completion of the specified
operation.  This means that when waiting for several operations to complete,
the calls to complete_operation may be done in any order without affecting
the performance of the system.

@begin(function)
LIST_OF_OPERATIONS
@f[create_list_of_operations](n, op1, op2, ..... opn)
@index[create_list_of_operations]
int n;
OPERATION op1, op2 ... opn)
@end(function)

Creates and returns a list containing the supplied operation
descriptors.  This list may then be passed to the op_select routines.

@begin(function)
void
@f[delete_list_of_operations](op_list)
@index[delete_list_of_operations]
LIST_OF_OPERATIONS op_list;
@end(function)

Reclaims the space for the supplied list of operations.

@begin(function)
int
@f[op_select_any](list_of_pending_operations, nfds, 
@index[op_select_any]
	      &readfds, &writefds, 
	      &exceptfds, timeout)
LIST_OF_OPERATIONS list_of_pending_operations;
int nfds;
fd_set readfds, writefds, exceptfds;
struct timeval *timeout;
@end(function)

This operation has exactly the same semantics as the select system
call, but in addition, it allows pending OPERATIONs to
progress.  This operation hangs in a select.  If activity is discovered
on any of the sockets controlled by the GDB library, then the
corresponding input is read and appropriate processing is done.  If any
of the listed pending_operations completes or terminates due to error,
op_select returns.  op_select also returns immediately if any of the
supplied operations is already complete. 

If activity is detected on any of the file descriptors supplied by the
user, then a count and bit fields are returned just as for select.
(Activity on database sockets is never reflected in either count or
bitfields.)  Timeout causes a return, as with select.  Upon return, the
program must check for competion or termination of any of the listed
operations, for activity on the selected file descriptors, and for
timeouts, if requested, since any or all of these may be reported
together.  

Return values are (>0) same as for regular select, (0) timeout, (-1) only
completion of one of the specified operations was detected.

@begin(function)
int
@f[op_select_all](list_of_pending_operations, nfds, 
@index[op_select_all]
	   &readfds, &writefds, 
	   &exceptfds, timeout)
LIST_OF_OPERATIONS list_of_pending_operations;
int nfds;
fd_set readfds, writefds, exceptfds;
struct timeval *timeout;
@end(function)

Same as op_select_any, except that (-1) is returned only when ALL of the
specified operations in the list have completed.  Values (>=0) are returned
for timeout or activity on FD's specified in the masks.

@begin(function)
int
@f[reset_operation](operation)
@index[reset_operation]
OPERATION operation;
@end(function)

Sets the state of an operation back to OP_NOT_STARTED.  This is
particularly useful for operations in a list passed to op_select_any.
@Index(op_select_any)
Op_select_any drops through immediately if any operation in the list
is marked complete.  Once the operation is reset, it may be left in
the list without detrimental effect.  reset_operation cannot be used
on an operation which is queued or running.

@begin(function)
OP_STATUS
@f[cancel_operation](operation)
@index[cancel_operation]
OPERATION operation;
@end(function)

Attempts to prematurely terminate execution of the specified operation.  If
the operation is queued, then it is removed from the queue of the
connection on which it resides.  If it is at the head of the queue and
running, then an attempt is made to signal the other end of the connection
to terminate execution.  If the operation has already completed,
successfully or otherwise, then CANCEL_OPERATION does nothing.  In general,
the application cannot assume that the cancellation takes effect
immediately, or that it will always succeed.  The status of the operation
must be checked to ascertain its true state, and if necessary, the
application must wait for the operation to complete or terminate
prematurely.  This call always returns immediately.  It does not wait to
see whether the operation was indeed terminated prematurely.  There is no
guarantee of prompt termination for any particular operation, since some
may be uninterruptible.  The intent is to make a best effort.  Returned
status covers only errors in the cancel operation itself.  The caller must
still use 'complete_ operation' or 'op_select' to check for completion or
successful cancellation of the operation.  

@begin(function)
int
@f[OP_STATUS](operation);
@index[OP_STATUS]
OPERATION operation;
@end(function)

Returns a code describing the progress or completion of the operation.
Returned values are OP_RUNNING, OP_COMPLETE or OP_COULDNT_START.  In the
case of OP_COMPLETE OP_CANCELLING, OP_CANCELLED, or OP_COULDNT_START, the
operation_result library routine may be used to get information about the
success or failure of the actual operation.  Note that OP_COMPLETE applies
to either success or failure, it merely implies that no further processing
remains to be done on the requested operation.  OP_CANCELLED implies that
the operation was indeed terminated prematurely by a cancellation request.
In this case, OP_RESULT is not valid.

@begin(function)
int
@f[OP_RESULT](operation);
@index[OP_RESULT]
OPERATION operation;
@end(function)

Returns detailed error information from the previous database or other
operation request.  The possible result codes depend on the particular
operation being performed, and they are documented along with the
operations themselves.  The operation result is available ONLY when
the operation is OP_COMPLETE.  Attempts to get results at other times
may result in fatal errors.
@end(functionsection)

@section(Creating new asynchronous operations)

In addition to operations like start_sending_object, which are
supplied with GDB, it is possible for users to create their own.  This
is a tricky business, because Unix lacks any notion of lightweight
process.  Until proper instructions are added to the GDB Users' Guide,
the best way to learn is to look at the source code for existing
operations provided with GDB.  

The general idea is that @f[initialize_operation] is called to fill in
the operation descriptor with enough state to drive the operation, and
then @f[queue_operation] is used to queue the descriptor on the
appropriate half connection.  When the operation reaches the head of
the queue, GDB automatically invokes the first routine of the
operation, which was specified during initialization.  This routine
proceeds as far as it can, updates its own state in the operation
descriptor (and an operation specific argument appendage), and
designates a new routine to receive control when further progress can
be made.  The following routines are among those used in creating new
operations: 

@begin(functionsection)
@begin(function)
int
@f[initialize_operation](operation, init_function, arg,
cancel_function)
@index[initialize_operation]
OPERATION operation;
int (*init_function)()
char *arg;
int (*cancel_function)()
@end(function)

Initializes the supplied operation with pointers to the supplied
initialization function with the supplied argument.  The initialization
routine will be called when and if the operation reaches the head of the
operation queue for some connection, and it will be supplied the argument
arg.  The cancel function, if non-null, will be called with the supplied
argument if for any reason the operation is to be canceled.  It can do
things like freeing the space for the argument or other associated cleanup.
If cancel_function is NULL, then no special processing is done during
cancellation.  The operation is put into the state OP_NOT_STARTED.

@begin(function)
OP_STATUS
@f[queue_operation](con, direction, operation)
@index[queue_operation]
CONNECTION con;
int  direction;<=  values are CON_INPUT
		    and CON_OUTPUT
OPERATION operation;
@end(function)

Takes the supplied operation and queues it for execution on the specified
half connection, assigning a new unique tag for the operation.  The
supplied operation must be in the state OP_NOT_STARTED (this may or may not
be checked.)  If possible, execution actually begins, and in some cases,
execution may complete by the time queue_operation returns.  Status returned
reflects the latest known state of the operation.  Transmission on all
connections may progress as a result of this operation.  If the connection
is not currently operating, then it is marked as severed.  NOTE: queue 
operation is used only by programmers creating their own asynchronous
services.  It should NOT be called by USERS of asynchronous services.
@end(functionsection)
@chapter(Miscellaneous System Services)
@Index(Miscellaneous services)
@begin(functionsection)
@begin(function)
void
@f[gdb_debug](flag)
@index[gdb_debug]
int	flag;
@end(function)

Toggles the specified debugging flag.  Flags currently supported are:
@begin(itemize)
@begin(multiple)
@end(multiple)
GDB_LOG@index[GDB_LOG flag]

When set, this flag tells gdb to write a variety of debugging information on
the file gdb_log.  By default, this file is set to @f[stderr]@index[stderr], 
but users may put their own stream pointers in gdb_log.  The debugging 
information is very detailed.  It is intended primarily for those with
detailed knowledge of GDB's inner workings.
@begin(multiple)
GDB_NOFORK@index[GDB_NOFORK flag]

Tells GDB not to fork when a client arrives for a forking server.  
When this is set, the server accepts only a single client, and terminates
when the client severs its connection.  This is useful for debugging servers
with dbx@index[dbx] or other debuggers which are incapable of handling 
forking programs. 
@end(multiple)
@end(itemize)
Warning:  the interface to this routine is likely to change in subsequent
releases of GDB.

@end(functionsection)

@Appendix(Revisions to this document)

@begin(table)
@caption(Library Reference Manual Revisions)
@tabset(.75in, 2in)
@\5/26/86@\Base document created

@\6/23/86@\Re-define database operations for consistency
@\@\with protocol.specification.

@\7/10@\Add reset_operation

@\7/18@\Added some notes in sever_connection on	
@\@\questions relating to maintenenance of
@\@\an out of band control stream.
@\@\Also, added requirement to call gdb_init.

@\7/25@\Considerations for re-queueing operations
@\@\Changed names on OP_STATUS, OP_RESULT

@\8/27@\General cleanup to match current state
@\@\of implemented system

@\9/2@\Correct documentation of string routines
@\@\and show requirement for stdio.h

@\9/10@\Corrections to start_db_operation and
@\@\start_db_query

@\9/17@\Added information on printing

@\11/11@\Added gdb_debug and associated flags.

@\11/19@\Added DB_STATUS and associated return codes.
@\@\Fixed parm list to db_query.

@\12/31@\Added host and user fields to client tuple

@\1/9/87@\Fixed return values on send/receive object
@end(table)
