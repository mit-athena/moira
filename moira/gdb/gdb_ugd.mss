@device(PostScript)  
@make(Manual)  
@style(FontFamily "Helvetica", Size 11)  
@style(hyphenation on)  
@style(indent 0)  
@style(leftmargin +4cm)  
@style(footnotes "")  
@modify(example,Size 10, below 1cm, above 1cm, leftmargin +3, rightmargin +0)  
@define(F, FaceCode F, Size 11, TabExport)  
@modify(HD1, Below .75cm, Above 1cm, indent -1cm)  
@modify(HD1A, Below .75cm, Above 1cm)  
@modify(HD2, Below .6cm, Above 1cm, indent -1cm)  
@modify(HD3, Below .6cm, Above 1cm)  
@modify(itemize, Below .5cm, Above .6cm)  
@modify(example, Below .5cm, Above .6cm)  
@modify(float, Below .5cm, Above .6cm)  
@begin(titlepage)  
@begin(titlebox)  
@majorheading(A Guide to Using GDB)  
@heading(Noah Mendelsohn)  
  
@end(titlebox)  
Document Version: 0.4 (DRAFT)  
For use with GDB Release: 0.4
This draft printed on: @value(Date)  
@copyrightnotice(MIT Project Athena)  
@end(titlepage)  
@Counter(ProgramNumber, Numbered <@1>, Referenced <@1>)  
@set(ProgramNumber=1)  
@Chapter(Introduction)  
  
Many of the most important computer applications involve sharing  
information among the users of a computer system.  
These applications may be simple to create if all users do their  
work on a single timesharing system, but the job is much harder in  
a @i[distributed] system, where each user has his or her own workstation.  
The purpose of GDB Global Database system is to simplify the creation   
of distributed applications in which information sharing is important.  
  
This guide is intended for prospective users of GDB who want to know  
what GDB can do for them and need a general guide to doing the   
job right.    
A complete description of GDB services and interfaces is provided in  
another document titled: "GDB C Library Reference Manual".  
Both guides are intended for readers with a good working knowledge  
of C and Unix@+[TM]@foot(@+[TM]Unix is a trademark of  
AT&T Bell Laboratories) programming.    
A short note titled: "GDB Global Databases for Project Athena" provides  
a general introduction to GDB   
for non-programmers.   
The sections below are intended to answer some of the most basic  
questions about GDB and using this guide.  
@section(What can GDB do?)  
Here is a brief list of the programming tasks that GDB facilitates:  
@begin(itemize)  
@begin(multiple)  
@b[Sharing a relational database through a network]  
  
GDB includes everything needed for a program running on one machine  
in a Berkeley Unix network to create, alter, and query the relations  
in an RTI Ingres database stored on some other machine in the network.  
Programs running on several machines may access the same data simultaneously,  
and a single program may use more than one database at a time.    
The programs may easily be ported across machine architectures, RT/PC  
to Vax for example, and programs running on different machine types  
may share a common database.  
@end(multiple)  
  
@begin(multiple)  
@b[Writing network servers and their clients]  
  
GDB handles most of the bookkeeping and error recovery needed to build  
servers and clients in a Berkeley Unix environment.  A completely   
functional demonstration server capable of supporting multiple clients  
with reasonable error detection has been written in about 20 lines  
of C code using the services of GDB (page @pageref[tfsr]).  
Clients running on various types of machine may easily access common  
servers, with GDB handling the necessary data conversions automatically.  
@end(multiple)  
  
@begin(multiple)  
@b[Single Process Unix Servers]  
  
It is customary when writing servers under Unix to fork a separate  
server process to handle each client.    
In addition to this traditional model, GDB provides a rich set  
of asynchronous communication services which facilitate the  
creation of servers in which a single process supports all clients.  
This model is particularly appropriate to the implementation   
of such high performance applications as network games.  
@end(multiple)  
  
@begin(multiple)  
@b[Peer-to-peer communication]  
  
Some communicating applications, like the Berkeley Unix @f[talk] program,  
@index(talk)  
don't fit naturally into a server/client model.  GDB also provides   
services for managing connections between @i[peer] programs, of which  
neither is viewed as master or slave.  
@end(multiple)  
@end(itemize)  
  
@section(Which Chapters in this Guide Should I Read?)  
@Index(Chapters, guide to)  
  
Although GDB has a rich set of features for elaborate  
asynchronous programming, many useful applications can be built  
with just the simplest GDB tools.  This guide is intended to be read  
selectively.  The following are some hints on what to read, depending  
on the job you are trying to do:  
  
@begin(itemize)  
Everyone who uses GDB should read the rest of this "Introduction",  
and the chapter titled "Managing Structured Data".  
  
If you wish to use GDB to access a remote Ingres database, then the  
only other chapter you have to read is "Creating and Using Relational  
Databases with GDB".  If you're interested in doing database accesses  
@i[asynchronously], allowing you to overlap your access to several  
databases or to keep control of the local terminal while a query is  
executing, then you should also read the chapter titled "Synchronous  
and Asynchronous Operations".  
  
If you are planning to write your own servers, clients, or other   
communicating programs, then you can skip the chapter on relational  
databases, but you should read instead the chapter "Creating  
Servers, Clients, and Other Communicating Programs".  This gives  
all the information you need to use the basic communication capabilities  
of GDB, which will be adequate for the majority of applications.  
Asynchronous communication techniques are described in the chapter  
"Synchronous and Asynchronous Operations".  
@end(itemize)  
  
Three other chapters provide additional material which may be useful on  
occasion.   "Hints and Tricks" has some suggestions which may be helpful  
if you can't figure out how to make GDB do exactly what you want.  
"Bugs" lists some of the known shortcomings in current implementations.  
If you're having trouble, this is one of the places to look.    
"How GDB Works" gives a general overview of GDB's internal  
structure, information which should not be needed in writing an  
application, but which may be of interest to serious users.  The  
sample programs in the appendices are referenced in the appropriate  
chapters.  
  
@section(What is needed to run GDB?)  
The services of GDB are accessed through a C library.  
You will have to insure that the appropriate C include files are available  
on your system, and that the gdb library is available when you link your  
application.  Your system administrator can take care of these things  
for you.    
@i[Note to early users:  You may receive the pieces of GDB in slightly  
different form.  Documentation supplied with the distribution should  
explain what to do.]  
  
@begin(group)  
@blankspace(1 line)  
Every C main program  
that uses GDB should include the following:  
  
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
  
Once these steps have been followed, the full services of the Ingres  
database are available to GDB clients throughout the network.  In  
future releases of GDB, the Kerberos Authentication  
Service@index(Kerberos) will be used to verify the identity of clients  
and to provide some options for access control.  In the meantime, keep  
in mind that any GDB user on the internet@index(Internet, access  
rights from)  
effectively inherits the access rights of the userid under which the  
database server is running.  
  
@Chapter(Managing Structured Data)  
@label(structure)  
  
This chapter presents a brief overview of the services that GDB  
provides for managing structured data.    
Although the discussion in this chapter uses relational database  
examples, the same GDB features are used to implement servers and  
clients of all kinds.  
  
When Ingres is used to retrieve data from a relational database, the  
result of the query is known as a @i[relation]@index(relation).  Each  
relation consists of 0 or more @i[tuples]@index(tuple), and the tuples  
themselves consist of @i[fields]@index(field).  Informally, one may  
think of the relation as a table containing one row for each "match"  
found in the database.  The rows are called tuples, and each such  
tuple contains the fields which were requested in the query.  For  
example, a query to a database containing information on military  
personnel might be:  
  
@begin(example)  
@center[Get the name, rank, and serial number of all personal with over   
5 years of service]  
@end(example)  
  
If there were two such people, then the resulting relation would contain  
two tuples, each with three fields.  For example:  
  
@begin(example)  
          Name Rank Serial  
          ----------------------  
          Mary  123 876543  
          John  121 875432  
@end(example)  
  
When GDB is used to perform a query, then some standard  
representation must be used to hold the resulting relation.  
Furthermore, GDB must know quite a bit about the @i[types] of the  
fields, whether they are @i[integer] or @i[real] for example, so that  
it can do necessary conversions when transmitting between  
incompatible machines.  Since C itself provides no convenient  
way of doing this, GDB uses the following conventions:  
  
GDB has its own notion of @i[data object]@index(Data object, GDB),  
which is slightly different from a C variable or structure.  A GDB  
data object may be as simple as a single integer or as complex as an  
entire relation.  Each GDB object has a @i[type]@Index(Typing of GDB  
objects), which tells GDB what kind of object it is.  Though some of  
the types supported by GDB are similar to those provided by the C  
language, GDB has its own typing mechanism which is intended to  
support the transmission and data conversion services that GDB must  
provide.  
  
Every GDB object has one of the following types:  
  
@begin(itemize)  
@begin(multiple)  
INTEGER_T   
@index[INTEGER_T]     
  
Usually the same as a @i[long] in C.  
@end(multiple)  
  
@begin(multiple)  
REAL_T      
@index[REAL_T]   
  
Usually the same as a @i[double] in C.  
@end(multiple)  
  
@begin(multiple)  
STRING_T    
@index[STRING_T]      
  
For various reasons, GDB uses counted byte strings.  The actual implementation  
of a STRING_T is:  
       
@begin(example)  
	struct {
		char *ptr;        /* pointer to first*/
			          /* byte of data, or*/
				  /* NULL if none */
		int length;       /* number of bytes */
				  /* allocated */
	}
@end(example)  
  
Though these strings are commonly used to hold C language null  
terminated strings, that is not required.  Any null(s) must be  
included in the length.  GDB includes library services for creating  
and manipulating STRING_T data (page @pageref[string_data_in_gdb]).  
Future releases of GDB may include a C_STRING_T type, which would  
correspond directly to a C null terminated string.  @end(multiple)  
  
@begin(multiple)  
DATE_T      
@index[DATE_T]   
  
Used to contain a date field retrieved from Ingres.  This is implemented  
as a 25 byte character array.  See Ingres documentation for conventions.  
@end(multiple)  
  
@begin(multiple)  
TUPLE_DESCRIPTOR_T    
@index[TUPLE_DESCRIPTOR_T]      
  
Every tuple and relation is described by a tuple descriptor.  The tuple  
descriptor contains the names of the fields, their position in the tuple,  
and the types of the fields.  Tuple descriptors are themselves GDB objects.  
@end(multiple)  
  
@begin(multiple)  
TUPLE_T     
@index[TUPLE_T]       
  
Refers to an entire tuple, including its descriptor.  An item of  
type TUPLE_T is self-describing.  GDB can tell by inspection what  
its structure is, as well as the names, types and contents of its fields.  
@end(multiple)  
  
@begin(multiple)  
RELATION_T       
@index[RELATION_T]    
  
Refers to an entire relation, all its tuples and their descriptors.  An item of 

type RELATION_T is also self-describing.  
@end(multiple)  
  
@begin(multiple)  
TUPLE_DATA_T     
@index[TUPLE_DATA_T]       
  
This is for internal use of GDB. It refers to just the data part of  
a tuple, when the description can be inferred.    
@end(multiple)  
  
@begin(multiple)  
User Defined Types  
@Index[User defined types]  
  
GDB allows users to define their own types.  This can be useful for  
sophisticated applications which want GDB to take care of transmission  
and data conversion for unusual data structures.  Few applications  
will actually require this flexibility, which is explained briefly under  
"Hints and Tricks".  
@end(multiple)  
@end(itemize)  
  
Why go to all this trouble, and what does it mean to say that things  
as simple as INTEGER_T and as complex as RELATION_T are both just types?  
As it turns out, this convention is the basis of a very powerful scheme  
for transmitting structured data from one computer to another.  GDB  
includes a service called @f[send_object] which is used to transmit  
@index(send_object)  
information through a network, and a matching service called   
@f[receive_object].    
@index(receive_object)  
Because of the uniform typing scheme outlined above, the same routine  
used to send a single integer may be used to send an entire relation.  
Furthermore, the typing information is just what GDB needs to do efficient  
data conversions when two incompatible machines are communicating.  
  
There is one more point of confusion which should be clarified.  As  
@Index(Types, GDB vs. C)  
shown above, the system supplied GDB types have names like INTEGER_T,  
STRING_T, and so on.      
The confusion arises because these are @i[not] C language typedefs,  
they are preprocessor constants used as indices into GDB's type  
tables.  For each GDB type there is usually a corresponding C language  
type.  For example, INTEGER_T is GDB's term for what C calls a  
@f[long].  Table @ref[GDBTypes] shows the correspondence between GDB types  
and C types:  
  
@begin(table)  
@caption(GDB Types and C Types)  
@tag[GDBTypes]  
@tabset(.75in, 3.5in)  
  
@u[@\GDB Type@\C Language Type]  
  
@\INTEGER_T@\long  
  
@\REAL_T@\double  
  
@\STRING_T@\STRING  
  
@\DATE_T@\char xx[25]  
  
@\TUPLE_DESCRIPTOR_T@\TUPLE_DESCRIPTOR  
  
@\TUPLE_T@\TUPLE  
  
@\RELATION_T@\RELATION  
  
@\TUPLE_DATA@\TUPLE->   
@\@\(i.e. a de-referenced tuple)  
@end(table)  
  
All of the GDB type identifiers, as well as the C language types with  
uppercase names are defined for you in @f[gdb.h].   
@index(gdb.h)  
  
When should you use the GDB type and when should you use the C type?  
In general, the C type is used whenever you are declaring a variable  
to C, or casting @index[casting pointers] a pointer to a new type.  
For example:  
  
@begin(example)  
int
main()
{
	TUPLE t;                        /* C declaration */
					/* for a TUPLE */
	RELATION r;                     /* C declaration */
					/* for a RELATION */
	STRING s;                       /* C declaration */
					/* for a counted */
					/* byte string */

	string_alloc(&s,25);		/* allocate a 25 */
					/* character string */
}
@end(example)  
  
The GDB types are used when you are telling @i[GDB] about the type  
of data it's going to manipulate:  
  
@begin(example)  
int
main()
{
	STRING s;                       /* C declaration */
					/* for a counted */
					/* byte string */
	CONNECTION c;

	/*
	 * send the data in STRING s on connection c.  The
	 * STRING_T indication below tells GDB what kind of
	 * object is being sent.
	 */
	send_object(c, &s, STRING_T);
}
@end(example)  
  
@section(Building and Using Relations)  
  
@index[Relations, creating] Each time you do an Ingres query, the  
results are added to the local relation that you supply.  That means  
that you will usually want to create an empty relation before doing a  
query, then pass that to the @f[db_query] routine @index(db_query)  
@index(querying Ingres databases)  
which will fill it in with the results of the query.  Creating the  
relation is done in two steps: (1) you create a  
TUPLE_DESCRIPTOR@index[tuple descriptor]@index[TUPLE_DESCRIPTOR]  
which contains information about the names and types of the fields  
which will be in the tuples of the relation and (2) using the  
descriptor, you create the empty relation.  Note that the same  
descriptor may be used repeatedly to create many relations.  Here is  
an example with one descriptor and two relations:  
  
@begin(example)  
int  
main()  
{  
     char *fld_names[] = {"name", "rank", "serial"};  
     FIELD_TYPE fld_types[] = {STRING_T, INTEGER_T, INTEGER_T};  
  
     TUPLE_DESCRIPTOR desc;  
     RELATION old_timers;  
     RELATION new_recruits;  
  
     desc = create_tuple_descriptor(3, fld_names, fld_types);  
  
     old_timers = create_relation(desc);  
     new_recruits = create_relation(desc);  
  
}  
@end(example)  
  
GDB provides many macros and procedures for manipulating relations,  
tuples, and fields.  Let's assume that the program goes on to do two  
successful queries, one for old_timers and one for new_recruits.  The  
declarations and code in Program #@ref(ProgramNumber) might be added  
to the example to print the results of the new recruits query.  
  
@begin(float)  
@index(Fields, accessing)  
@index(Relations, tuples in)  
@index(Tuples in relations)  
  
@center(PROGRAM #@ref(ProgramNumber))  
@set(ProgramNumber=+1)  
@begin(example)  
TUPLE t;  
STRING *name;  
int *rank;  
int *serial;  
/*  
 * Define symbolic names for the positions of the fields  
 * in the tuples.  There are other ways to do this,  
 * but this does work:  
 */  
#define NAME 0  
#define RANK 1  
#define SERIAL 2  
/*  
 * the following code would go after the query  
 *  
 *    the loop is executed once for each new  
 *    recruit returned by the query  
 */  
 for(t=FIRST_TUPLE_IN_RELATION(new_recruits);  
     t != NULL;  
     t = NEXT_TUPLE_IN_RELATION(new_recruits, t)) {  
     /*  
      * Get a pointer to each field in the tuple  
      */  
      name = (STRING *)FIELD_FROM_TUPLE(t, NAME);  
      rank = (int *)FIELD_FROM_TUPLE(t, RANK);  
      serial = (int *)FIELD_FROM_TUPLE(t, SERIAL);  
     /*  
      * print the fields  
      */  
      printf("name=%s rank=%d serial=%d\n",   
          STRING_DATA(*name), *rank, *serial);            
 }  
/*  
 * return the memory used to hold the new_recruits relation  
 */  
delete_relation(new_recruits);  
@end(example)  
@end(float)  
  
The example illustrates several important points.  Variables  
representing structured objects  
like TUPLE, TUPLE_DESCRIPTOR and RELATION are actually pointers.    
@index(Relations, pointer representation of)  
@index(Tuples, pointer representation of)  
@index(Relations, tuples in)  
@index(Tuples in relations)  
Because TUPLE is a pointer type, TUPLE variables may be used  
as cursors in a relation.  For example, you might declare a TUPLE  
variable called @f[ranking_recruit] and set it in the loop above to  
the tuple of the recruit with the highest rank.    
Lots of TUPLE variables may point into  
the same relation, or even to the same tuple, but like any other  
pointers, they become invalid if the corresponding structures are  
deleted.  The C value NULL is used to represent non-existent TUPLES,  
RELATIONS, and descriptors.  
  
As shown in the program, the @f[FIELD_FROM_TUPLE] macro is used to find  
@index(Fields, accessing)  
@index(FIELD_FROM_TUPLE)  
a field within a tuple.  @f[FIELD_FROM_TUPLE] returns a  
@i[pointer] to the requested field, and it is up to you to cast the  
pointer according to the type of field being retrieved.  In the  
program, @f(name) is of type STRING_T, so the  
cast @f[(STRING *)] is used.  Pointer casting is a feature of  
the C language which is explained in Kernighan and Ritchie and other C  
programming guides.  
  
Note that @f[FIELD_FROM_TUPLE] refers to fields by their  
position in the tuple, not by name.  This was done for efficiency, as  
@f[FIELD_FROM_TUPLE] is likely to be called frequently by GDB  
applications.  The argument supplied may be a variable,  
but it is your responsibility to correctly interpret the type of  
the resulting pointer.  GDB can also help you find fields by symbolic  
name, but that takes longer.  The @f[field_index]   
@Index(Fields, finding by name)  
function takes  
as its argument a tuple descriptor (@i[not] a tuple) and the string  
name of a field;  it returns the field number of the named field.  If  
you want to pull fields by name from several similar tuples, the  
efficient way is to find the index once using @f[field_index],  
then use that repeatedly in calls to @f[FIELD_FROM_TUPLE].  
  
A brief guide to the functions and macros provided for manipulating structured  
data is provided in Appendix I.  
  
@section(STRING Data in GDB)  
@label(string_data_in_gdb)  
  
@index(string)  
@index(STRING_T)  
As noted previously, GDB represents string data as counted fields of  
bytes.  If you create a tuple with a field of type STRING_T, then the  
space actually allocated in the tuple is just that needed to hold the  
location and length of the string data, not the space for the data  
itself.  @f[gdb.h] @index[gdb.h] contains the following definition,  
which may be used when referring to STRING fields, or to create your  
own variables of type STRING.  
  
@begin(example)  
@index[FIELD_FROM_TUPLE]
@index[string_alloc]
@index[MAX_STRING_SIZE]
@index[STRING_DATA]
	typedef struct str_dat {
		char *ptr;            /* pointer to first */
				      /* byte of data, or  */
				      /* NULL if none */
		int length;           /* number of bytes of*/
				      /* data allocated */
	} STRING;
@end(example)  
  
If you have such a tuple and wish to initialize the string field to have  
space for up to 100 bytes of data, then the code in Program  
#@ref(ProgramNumber) might be used.  
  
@begin(float)  
  
@center(PROGRAM #@ref(ProgramNumber))  
@set(ProgramNumber=+1)  
@begin(example)  
	TUPLE t;
	#define STRFIELD 3	      /* index of the string*/
				      /* field in the tuple */
	STRING *sp;		      /* pointer to the field*/

       /*
	* Get a pointer to the string field in the tuple
	*/
	sp = (STRING *)FIELD_FROM_TUPLE(t,STRFIELD);
@index[FIELD_FROM_TUPLE]
       /*
	* Allocate space for 100 bytes
	*/
	string_alloc(sp, 100);
@index[string_alloc]
       /*
	* Copy some text into the new buffer
	*/
	strcpy(STRING_DATA(*sp), "Hi there");
@index[STRING_DATA]
       /*
	* Note, at this point:
	*
	*	MAX_STRING_SIZE(*sp) == 100
	*
	*	strlen(STRING_DATA(*sp)) == 9
	*/
@end(example)  
@end(float)  
  
A call to @f[string_free] would free all 100 bytes of data.  Note that  
@index(string_free)  
a routine named @f[null_tuple_strings] frees all the strings in a given  
@index(null_tuple_strings)  
tuple.    
  
If you're writing your own servers and clients, then STRING variables  
are a convenient way of sending uninterpreted byte strings from one  
machine to another, as shown in Program #@ref(ProgramNumber).  
  
@begin(float)  
  
@center(PROGRAM #@ref(ProgramNumber))  
@set(ProgramNumber=+1)  
@begin(example)  
	CONNECTION con;
	char buffer[500];
	STRING s;

       /*
	* Set up string descriptor s as required by gdb
	*/

	STRING_DATA(s) = buffer;       /* copy the pointer*/

	/*
	 * For demonstration purposes, put some data into the
	 * buffer.
	 */
	
	 strcpy(buffer, "Some string");
	 MAX_STRING_SIZE(s) = strlen(buffer)+1; 
					/* include space for */
					/* the null */

	/*
	 * send the object through the connection.
	 * we assume that the connection is already
	 * started
	 */
	 send_object(con, &s, STRING_T);	
@end(example)  
@end(float)  
  
As of this writing, the STRING_T type is the only means GDB provides  
for manipulating byte strings.  Future releases may also support a  
C_STRING_T, which would more closely match C's convention of null  
terminated strings.  The current implementation was chosen for its  
generality and efficiency.  STRING_T may be used to hold any array of  
bytes, including those with embedded null characters, and for long  
strings, STRING_T avoids the overhead of searching the string to  
compute its length.  
  
@Chapter(Creating and Using Relational Databases with GDB)  
  
@index[Relational databases]  
GDB may be used to obtain the full services of an RTI Ingres  
@index[RTI Ingres]  
@index[Ingres]  
relational database from a program running at any node in a Berkeley  
Unix network.  In addition to most of the services provided by RTI  
Ingres version 3, GDB supports:  
  
@begin(itemize)  
Remote access from any machine in a network.  
  
Parallel access to multiple databases, possibly but not necessarily at  
multiple sites.  
  
Transparent access from incompatible machines (e.g. RT/PC to VAX.)  
@end(itemize)  
  
In fact, GDB is designed to be used with a variety of database  
products, but RTI Ingres is the only one for which an implementation  
exists at present.  Nevertheless, every attempt has been made to  
isolate the Ingres dependencies in GDB.  
GDB's current support of relational databases is based on the Ingres QUEL  
query language, which is documented in the appropriate manuals from  
RTI.  It is @i[essential] that you familiarize yourself with QUEL  
before attempting to use GDB to manipulate relational databases.  A  
knowledge of QUEL is presumed in the sections which follow.  
  
@section(Accessing Databases)  
  
The general procedure for using relational databases in GDB is:  
  
@begin(itemize)  
Use @f[access_db] to initiate access to the database(s) you want to  
@index(access_db)  
use.  You specify the name of the database and the host in the network  
which stores it, and GDB sets up the appropriate connection to the  
database server.  
  
Use @f[perform_db_operation] and @f[db_query] to manipulate or query  
@index(db_query)  
@index(perform_db_operation)  
the databases.  
  
Use @f[terminate_db] to sever the connection to the database server.  
@index(terminate_db)  
It is also acceptable for your program to exit without terminating its  
databases, as  
the server will notice that the connection has been severed.  
@end(itemize)  
  
Because some programs require access to several databases at once, GDB  
uses DATABASE@index[DATABASE] variables to keep track of the various  
databases which are being used:  
  
@begin(example)  
int
main()
{
	DATABASE personnel, inventory;

	personnel = access_db("people_data@@host1");
	inventory = access_db("stuff_data@@host2")
	/*
	 * Both databases are now available.  Either one
	 * may be queried or manipulated, or both may be used
	 * in parallel.
	 */
}
@end(example)  
  
Once an attempt is made to access a database, the status of the  
corresponding database connection may be checked by using the  
DB_STATUS@index[DB_STATUS] macro.  The values of DB_STATUS are shown in  
table  
@ref(DB_STATUS).  
  
@begin(table)  
@caption(Database Status Values)  
@tag(DB_STATUS)  
@tabset(.75in, 2.5in)  
  
@\DB_OPEN@index[DB_OPEN]@\The connection to the database  
@\@\appears to be intact.  Operations  
@\@\on the database may be attempted.  
  
@\DB_CLOSED@index[DB_CLOSED]@\The connection to the database  
@\@\has been lost. @f[terminate_db] is the  
@index(terminate_db)  
@\@\only operation which will be accepted.  
@end(table)  
  
There is a distinction between a database which is @f[DB_CLOSED] and  
@index(DB_CLOSED) one which has been explicitly terminated.  
@f[terminate_db] allows GDB @index(terminate_db) to clean up the data  
structures which had been used to control access to the database.  It  
sets the supplied database variable to NULL.  @f[DB_STATUS] should not  
be used on a database which has been @index(DB_STATUS) explicitly  
terminated; it is intended for checking the status of a database which  
is believed to be accessible.  @section(Performing Operations on  
Databases) With the exception of queries, all Ingres QUEL@index[QUEL]  
operations may be performed using the GDB @f[perform_db_operation]  
function.  @index(perform_db_operation) Program #@ref(ProgramNumber)  
accesses the database named @f[personnel_data] at site @f[host1],  
creates an empty table named @f[new_recruits], and puts three entries  
in the new table.  
  
@begin(float)  
  
@center(PROGRAM #@ref(ProgramNumber))  
@set(ProgramNumber=+1)  
@begin(example)  
int
main()
{
	DATABASE personnel;
	char     *quel_command;
	int      retcode;
	/*
	 * Access the database and check for errors
	 */
	access_db("personnel@@host1", &personnel);
	if (DB_STATUS(personnel) != DB_OPEN) {
		fprintf(stderr,"Could not access database\n");
		exit(4);
	}
	/*
	 * Create the new table by sending the appropriate QUEL
	 * command.
	 */
	quel_command=
	    "create recruits (name=c8, rank = i4, serial=i4)");
	retcode = perform_db_operation(personnel,quel_command);
	if (retcode != OP_SUCCESS)  {
		fprintf(stderr,"Could not create table.\n");
		exit(4);
	}
	/*
	 * Add three rows to the table
	 */
	quel_command=
"append to  recruits (name=\"Mary\", rank = 123, serial=876543)";
	retcode = perform_db_operation(personnel,quel_command);
	if (retcode != OP_SUCCESS)  {
		fprintf(stderr,"Could not append\n");
		exit(8);
	}
	quel_command=
"append to  recruits (name=\"John\", rank = 121, serial=875432)";
	retcode = perform_db_operation(personnel,quel_command);
	if (retcode != OP_SUCCESS)  {
		fprintf(stderr,"Could not append\n");
		exit(8);
	}
	quel_command=
"append to  recruits (name=\"Noah\", rank = 1, serial=123)";
	retcode = perform_db_operation(personnel,quel_command);
	if (retcode != OP_SUCCESS)  {
		fprintf(stderr,"Could not append\n");
		exit(8);
	}
	/*
	 * Sever the connection to the database.  This is 
	 * optional.
	 */
	terminate_db(&personnel);
}
@end(example)  
@end(float)  
  
It is a complete example, with all necessary error recovery.  The  
return code provided by @f[perform_db_operation] is the Ingres error  
@index(perform_db_operation)  
@index(Ingres error)  
@index(Error, Ingres)  
number for the operation attempted, or else the special reserved value  
@f[OP_CANCELLED].  The latter indicates that GDB lost its connection  
@index(OP_CANCELLED)  
to the server either just before or just after the operation was  
attempted.  There is no way to tell whether the operation took effect  
before the connection was lost.  Subsequent operations on the database  
are unlikely to work unless another @f[access_db] is done.  
@index(access_db)  
  
Note that @i[any] QUEL@index[QUEL commands] command, with the  
exception of a retrieve, may be done using @f[perform_db_operation].  
All clients are considered to @index(perform_db_operation) have the  
same access and update rights as the userid under which the @f[dbserv]  
program is running.  @index(dbserv)@index(Internet, access  
rights from)@Index(Access rights)  
  
Hint:  the @f[sprintf] function is a very useful means of putting  
@index(sprintf)  
variable information into command strings.  Just use @f[sprintf] to  
format the QUEL command in a C character array, then pass that to  
@f[perform_db_operation] for execution.  @f[sprintf] is found in  
the standard Unix C library.  Try @f[man sprintf] if you don't know  
how to use it.  
@section(Performing Database Queries)  
  
@index[Retrieval from relational database]  
@index[Querying relational databases]  
Retrieves are different from most other QUEL commands because they  
return query result data in addition to the usual return code.  For this  
reason, GDB provides a special @f[db_query] function which is used to  
@index(db_query)  
do information retrieval from an Ingres database into a GDB relation.  
Techniques for building an empty relation and for getting at the  
fields of a result were discussed in Chapter @ref(structure).  All  
that remains is to explain how the query itself is formulated and sent  
to the server.    
  
@begin(multiple) The format of a GDB retrieve request is exactly the  
same as the corresponding QUEL@index[QUEL] retrieve @i[except that the  
target of each retrieve is specified differently].  In QUEL, the  
command:  
  
@begin(example)  
retrieve (name=recruits.name, rank=recruits.rank, 
	  serial=recruits.serial) 	
	 where recruits.rank >10
@end(example)  
  
would result in a table with three columns named @f[name, rank] and  
@f[serial].  If we assume that a null GDB relation @f[result] has been created  
with fields named @f[name, rank] and  
@f[serial], then the following call to @f[db_query] would accomplish a  
similar query:  
  
@begin(example)  
db_query(personnel, result,  
      "(>*name*<=recruits.name, >*rank*<=recruits.rank,   
       >*serial*<=recruits.serial)        
      where recruits.rank >10");  
@end(example)  
  
Here are the general rules for converting a QUEL query into a GDB  
query:  
@end(multiple)  
  
@begin(itemize)  
Create a GDB relation with fields suitable for holding the result.  
The types of the fields must match the data to be retrieved.  
Extra fields are OK, they will be set to null values in the retrieved  
tuples.  You may find the extra fields useful for various purposes in  
your program.  
  
Prepare a C character string which specifying the query to be  
performed.  The string has exactly the same form as the  
arguments to a QUEL retrieve, except that @i[each target will be the  
name of a field in the relation, bracketed by >*...*<].  All the usual  
syntactic rules governing QUEL retrieves apply.  In particular,  
remember the parenthesis which are required around the target list.  
The verb @i[retrieve] should @i[not] be specified at the beginning of  
the string.  
  
Use @f[db_query] to perform the query.  
@index(db_query)  
@end(itemize)  
  
GDB sends the query request to the remote host and executes the query  
there.  Return codes are handled in the same manner as for  
@index(Return codes, Ingres)  
@index(Ingres error)  
@index(Error, Ingres)  
@f[perform_db_operation].  In the case where no errors are found,  
tuples resulting from the query are appended to the supplied relation.  
If the server is running on a different type of machine than your  
client program, GDB does any necessary data conversions to local  
representation for you.  
It is quite possible to get a return code of OP_SUCCESS with no tuples  
added to the relation.  This indicates that Ingres had no trouble  
executing the query, but found no data matching your retrieval  
criteria.  
  
You may use @f[perform_db_operation] to establish range names which  
@index(perform_db_operation)  
may then be used in your query.  Range names are private to each user  
of the database, and to each connection to the database.  
  
Program #@ref(ProgramNumber) is a copy of the sample program from  
Chapter @ref(structure), fleshed out to show the actual retrieval.  
For brevity, no error checking is done in this example.  In practice,  
the return code from each GDB function should be checked.  
  
@begin(float)  
  
@center(PROGRAM #@ref(ProgramNumber))  
@set(ProgramNumber=+1)  
@begin(example)  
int
main()
{
	DATABASE personnel;

	char *fld_names[] = {"name", "rank", "serial"};
	FIELD_TYPE fld_types[] = {STRING_T, INTEGER_T, INTEGER_T};
#define NAME 0
#define RANK 1
#define SERIAL 2

	TUPLE_DESCRIPTOR desc;
	RELATION new_recruits;
	TUPLE t;
	STRING *name;
	int *rank;
	int *serial;

	/*
	 * Access the database
	 */
	access_db("personnel@@host1", &personnel);

	/*
	 * Create a null relation to hold the results
	 */
	desc = create_tuple_descriptor(3, fld_names, fld_types);
	new_recruits = create_relation(desc);

	/*
	 * Do the query
	 */
	db_query(personnel, new_recruits,
		 "(>*name*<=recruits.name, >*rank*<=recruits.rank, 
		  >*serial*<=recruits.serial) 	
		 where recruits.rank >10");
	/*
	 * print the results
	 */
	for(t=FIRST_TUPLE_IN_RELATION(new_recruits);
	    t != NULL;
	    t = NEXT_TUPLE_IN_RELATION(new_recruits, t)) {
	 	/*
		 * Get a pointer to each field in the tuple
		 */
		name = (STRING *)FIELD_FROM_TUPLE(t, NAME);
		rank = (int *)FIELD_FROM_TUPLE(t, RANK);
		serial = (int *)FIELD_FROM_TUPLE(t, SERIAL);
		/*
		 * print the fields
		 */
		printf("name=%s rank=%d serial=%d\n", 
		 	STRING_DATA(*name), *rank, *serial);		 
	}
}
@end(example)  
@end(float)  
  
  
@Chapter(Creating Servers, Clients, and Other Communicating Programs)  
@Index(Communicating programs, writing)  
  
One of the main purposes of GDB is to facilitate the programming of  
servers, clients, and other communicating programs.  Many useful  
programs can be created in just a few lines of code, but GDB also  
allows you to write very complex asynchronous servers which support  
many clients from a single Unix process.  This chapter outlines the various  
techniques which may be used for writing communicating programs in  
GDB.  
  
@section(Connections)  
  
All communication in GDB is done using connections.  The state of  
each connection is kept in a variable of type @f[CONNECTION], or more  
@index(CONNECTION)  
specifically, in a structure to which the connection variable points.  
GDB has several functions for initiating and terminating connections.  
They are implemented using Berkeley Unix socket facilities, but  
are intended to be much easier to use.  There are several different  
ways to start connections, depending mainly on whether you are writing  
a server, a client, or sets of peers.  These are described in sections  
below.  Once successfully started, all  
connections are full duplex, reliable, ordered data paths on which GDB  
objects may be sent and received.    
  
Once it is started, each connection has a status which may be  
determined using the @f[connection_status] macro.  Values of concern  
@index(connection_status)  
to users are shown in table @ref(ConnectionStatus).  
  
@begin(table)  
@caption(Connection Status Values)  
@tag(ConnectionStatus)  
@tabset(.75in, 2.5in)  
  
@\CON_UP@index[CON_UP]@\Connection is operational.  
@\@\Data transmission may be attempted.  
  
@\CON_STOPPING@index[CON_STOPPING]@\Connection has failed.

@\@\User may check the @f[connection_errno]
@\@\@index[connection_errno] for cause, or may call
@\@\@f[connection_perror]@index[connection_perror] to print
@\@\message.  In any case, user must issue a @f[sever_connection]@index[sever_connection]
@\@\in order for the connection to be re-used by gdb.

@\CON_STOPPED@index[CON_STOPPED]@\Connection has been 
@\@\severed by the user.
@end(table)  
  
Note that the routines which start connections may return NULL values
in case of failure or they may return a connection descriptor in the
CON_STOPPING state.  
When a connection enters the CON_STOPPING state, the only operations which
users may perform are to check the @f[connection_status] and the @f[connection_errno], or to sever the connection.

If any connection, including a listening connection, fails due to an
error reported by Unix on a system call, GDB records the corresponding
Unix @f[errno]@index[errno] in the connection descriptor, and puts the
connection in the CON_STOPPING state.  All pending operations on the
connection are cancelled.  This errno value may be queried with the
@f[connection_errno] macro.

Users may terminate connections at any time from
either end by using the @f[sever_connection] function.  All operations
queued at either @index(sever_connection) end are cancelled and the
communication path is closed.  Connection @i[variables] which have
been severed may be re-used to start other connections.  A connection
which has entered the @f[CON_STOPPING] @index(CON_STOPPING) state
should be severed before its connection variable is re-used.
  
  
@section(Sending and Receiving Data)  
  
GDB provides a @f[send_object] function which may be used to send any  
@index(send_object)  
GDB object on a connection, and a matching @f[receive_object]  
@index(receive_object)  
function.  Objects are sent in order on each connection.  Calls to  
@f[receive_object] should match one for one with calls to  
@f[send_object], and the types specified on the receives must  
match those on the sends.  @i[Failure to do this will result in  
unpredictable program failures including segmentation faults, hangs,  
prematurely severed connections, etc.]  
  
Chapter @ref(structure) describes GDB's typing scheme.  
@f[send_object] is a @i[polymorphic] routine which can transmit any  
type of GDB object.  Each object is automatically converted to the  
local representation of the receiving machine.  Program  
#@ref(ProgramNumber) is a simple example which starts a connection to  
a server, sends a single integer to the server, and gets back a  
relation.  
  
@begin(float)  
  
@center(PROGRAM #@ref(ProgramNumber))  
@set(ProgramNumber=+1)  
@begin(example)  
int
main{
	CONNECTION server;
	int i = ???somevalue???;
	RELATION result;
	int rc;

	server = start_server_connection("serverid", "");

	if (server == NULL) {
		fprintf(stderr, "Failed to connect to server\n")
		exit(4);
	}

	rc = send_object(server, &i, INTEGER_T);
	if (rc == OP_CANCELLED) {
		fprintf(stderr, "Could not send to server.\n")
		exit(4);
	}

	rc = receive_object(server, &result, RELATION_T);
	if (rc == OP_CANCELLED) {
		fprintf(stderr, "Could not receive from server.\n")
		exit(4);
	}

	sever_connection(server);

}
@end(example)  
@end(float)  
  
This program is complete, if not particularly useful,   
and it contains all testing needed to detect  
communication related errors.  Many very useful applications employ GDB  
in just this simple manner.  GDB takes care of locating the server  
(naming of servers is discussed below), managing the connection,  
converting data for transmission, and detecting errors.  
  
As discussed in chapter @ref(async), many GDB operations have  
@index[Asynchronous operations]  
asynchronous equivalents.  @f[start_sending_object] and  
@index(start_sending_object)  
@f[start_receiving_object] may be used to asynchronously send and  
@index(start_receiving_object)  
receive data on GDB connections.  GDB maintains a separate operation  
queue for the inbound and the outbound activity on each connection.  
If you invoke @f[start_sending_object] several times in rapid  
succession on the same connection, the requests queue up in  
order.  The same is true for @f[start_receiving_object], but sends  
and receives are queued independently.  In certain cases, it may  
be more efficient to queue several asynchronous transmissions at once  
than to do just one at a time.    You must not change any  
variables being sent or received until the operations have completed,  
otherwise unpredictable results may occur.  If an ordinary @f[send_object] or  
@f[receive_object] is invoked, then the corresponding queue is flushed  
before the synchronous operation is attempted.  
  
@section(Naming Communicating Programs)  
@Index(Naming communicating programs)  
  
A GDB program attempting to connect to a correspondent somewhere else  
in the network must have some means of addressing its connection  
request.   Network servers awaiting connections from clients must have  
some means of establishing their own identity to the network.    The  
appropriate form for such addresses may change over time and according  
to the organization in which GDB is being used.  To isolate these  
concerns in the cleanest possible manner, all GDB routines use an  
ordinary C character string to encode any desired network or service  
address.  Though the form of the string may change when GDB is  
re-implemented in new environments, the number and nature of  
parameters to GDB's functions will not.  
  
At present, GDB uses the naming facilities of Berkeley 4.2 or 4.3  
Unix.  Berkeley Unix uses a file called @f[/etc/services] to define  
@index(/etc/services)  
the names of network services and to map them to internal addresses  
known as port numbers.  Setting up @f[/etc/services] is a job for your  
Unix system administrator, but you must ensure that each GDB service name  
you intend to use has been defined in @f[/etc/services] at all  
communicating sites to be a @i[tcp port].  Your system administrator  
should know what that means.  Once this has been done, the service  
name is available for use by GDB.  For example, a program intended to  
act as a server for clients playing chess with each other might be  
created with the GDB function:  
  
@begin(example)  
                 client = create_forking_server("chess", NULL)  
@end(example)  
  
where @f[chess] is a service name defined in @f[/etc/services].  
@index(/etc/services)  
(Other considerations in using @f[create_forking_server] are discussed  
@index(create_forking_server)  
below.  Here we are interested only in the service name @f[chess].)  
  
A client desiring a connection to this server would have to specify  
the host at which the server was running as well as its service name.  
The client might create the connection with:  
  
@begin(example)  
          server = start_server_connection("hostname:chess", NULL)  
@end(example)  
  
@Index(start_server_connection)  
where @f[hostname] is the name of the host where the server is  
running.  GDB takes care of finding the host, looking up the service  
name @f[chess] in @f[/etc/services], and making the appropriate  
connection.   
  
Sometimes its convenient to bypass @f[/etc/services] and specify a  
port number directly.  GDB assumes that any service name beginning  
with @f[#] is a port number.  For example:  
@Index[# (Port number designator)]  
@Index(Port numbers)  
  
@begin(example)  
                 client = create_forking_server("#9502", NULL)  
@end(example)  
  
creates a server at port 9502.  No checking for conflicts with  
existing services or entries in @f[/etc/services] is done.  In  
general, you should be sure that you and your system administrator  
agree on any service names or port numbers you are going to use before  
you try it.  Otherwise, you may interfere with other network services  
at your site.  
  
@section(Connecting to A Server)  
@Index(Connecting to a server)  
  
The code fragment:  
  
@begin(example)  
     	CONNECTION server;  
  
     	server = start_server_connection("hostname:chess", NULL)  
     	if (server == NULL)   
        	fprintf(stderr, "Could not start connection.\n");  
@end(example)  
@Index(start_server_connection)  
  
may be used to start a connection to a service named @f[chess] on the  
specified host.  If the returned connection variable is not NULL, then  
a GDB connection has been created which may be used for sending and  
receiving GDB objects.  The @f[sever_connection] function may be used  
@index(sever_connection)  
to break the connection at any time, or the client program may simply  
exit.  A properly written server will notice that the connection has  
failed and do the appropriate cleanup.  
The second parameter to @f[start_server_connection] is an optional  
parameter string which is made available to the server.  Rules for  
using this string are established by each server.  
There is no asynchronous form of start_server_connection at the  
present time.  GDB delays until the request either succeeds or  
encounters an unrecoverable error.  
  
@section(Writing Servers)  
  
There are several kinds of GDB server, each of which is discussed in a  
separate section below.  
  
@subsection(Forking Servers - the simplest kind)  
  
@index[Forking servers]  
The simplest kind of GDB server, which most programmers will use, is  
called the @i[forking] server.    
This is the traditional kind of Unix server in which a new process is  
forked to deal with each client.  GDB does the forking for you, and  
takes care of most of the Unix bookkeeping required in a forking  
program.  
Creation of forking servers is best illustrated by the simple example  
in Program #@ref(ProgramNumber).   
  
@modify(example,Size 9)  
@begin(float)  
  
@center(PROGRAM #@ref(ProgramNumber))  
@set(ProgramNumber=+1)  
@begin(example)  
@label(tfsr)
{
	CONNECTION client;	/* talk on this to client */
	int data;		/* receive data here */

	gdb_init();		/* set up gdb */

	client = create_forking_server("echoserver",NULL);
	fprintf(stderr,"forked\n");

	while (TRUE) {
		if (receive_object(client, &data, INTEGER_T) ==
		    OP_CANCELLED) {
			fprintf(stderr,"Client has terminated\n");
			exit(4);
		}
		if (data >= 'a' && data <= 'z')
			data += 'A'-'a'; /* upcase the response */
		if (send_object(client, &data, INTEGER_T) ==
		    OP_CANCELLED) {
			fprintf(stderr,"send error\n");
			exit(4);
		}
	}
}
@end(example)  
@end(float)  
  
This is a fully functional server capable of supporting an arbitrary  
number of clients.  It does the trivial work of accepting a stream of  
characters (stored in integers) converting them to uppercase, and  
sending the result back to the client.  The @f[create_forking_server]  
@index(create_forking_server)  
function behaves somewhat like the Unix @f[fork] system call.  Each  
@index(fork)  
time a new client connects to the server, a fork is done and the  
variable @f[client] in the child process is set to the new client  
@index(client)  
connection.  In other words, the code below @f[create_forking_server]  
is automatically invoked over and over again in a new process each  
time a connection is received from a client.  When the processing for  
each client is done, an @f[exit] or @f[return] is all that is needed  
@index(return)  
@index(exit)  
to cause the appropriate cleanup in the server.  Note that this sample  
server detects termination of its clients by checking for errors when  
receiving and sending.  This technique is convenient and reliable,  
since the connection is always severed as soon as the client  
terminates.  
  
@subsection(Single Process Servers)  
  
@index[Single process servers]  
Though forking servers are particularly easy to write, they have the  
disadvantage of breaking the server into a separate process for each  
client.  In particular, forking servers are unsuited to applications  
in which data is to be shared among the various clients.  A server  
which implemented a bridge game, for example, might relay the bids  
played by one client to all of the three others.  This kind of program  
is very difficult to implement in a forking server, so GDB also allows  
you to write servers in which all clients are handled by a single Unix  
process.  Because so much asynchronous activity has to be coordinated  
within a single process, these servers are generally much harder to  
design and debug.  Nevertheless, they can be the basis of some of the  
most exciting, high performance applications which can be written with  
GDB.    
  
Chapter @ref(async) introduces the techniques  
used to do asynchronous communication with GDB.  
Read it now, if you have not already done so, before proceeding   
with this section.  
Here we show how a  
single server process may @i[asynchronously] acquire new connections  
from its clients.  In general, a single process server works in the  
following manner:  
  
@begin(itemize)  
@f[create_listening_connection] is used to create a special connection  
@index(create_listening_connection)  
on which connection requests from clients arrive.  
  
The asynchronous operation @f[start_accepting_client] is  
@index(start_accepting_client)  
queued on the listening connection.  This operation completes whenever  
a new client requests a connection.  The server immediately issues a  
new @f[start_accepting_client] in case another client is  
trying to connect.  
  
The server examines the parameters supplied by the client, and if it  
decides to accept the client's connection, transmits its decision  
by using the asynchronous operation @f[start_replying_to_client].  
@index(start_replying_to_client)  
When this becomes OP_COMPLETE, the new client is ready for use.  
  
@f[start_sending_object] and @f[start_receiving_object] are used in  
@index(start_receiving_object)  
@index(start_sending_object)  
the usual manner to send data to and from the clients.  
@end(itemize)  
  
The typical server must be capable of doing all of these operations  
simultaneously and in any order.  It maintains a LIST_OF_OPERATIONS  
which includes all of the operations on which completion is awaited,  
and it drops into an @f[op_select_any] to wait for something to  
@index(op_select_any)  
happen.  Based on the operation which has completed, it determines the  
next step to perform.  If a client has sent data, it may queue a  
response or prepare data for transmission to other clients.  If a new  
connection request is received it must queue the appropriate response,  
and so on.    
  
There are many ways to organize such a program.  A sample, which  
performs the same character echoing function as the forking server on  
page @PageRef(tfsr), is included in Appendix @ref(nonforkingserver).  
Studying this sample is the best way to learn about creating  
non-forking servers.  The GDB library routines are documented in the  
Library Reference Manual.  
  
@section(Communicating Peers)  
  
@index[Peer to peer communication]  
Sometimes it is useful to write communicating @i[peer] programs which  
do not fit the client/server model.   
When two peers communicate, neither may be presumed to start before  
the other.  GDB provides a  
@f[start_peer_connection]@index[start_peer_connection] function which  
starts a connection to a peer elsewhere in the network.    
If necessary, @f[start_peer_connection] delays indefinitely until its  
peer is started and issues a matching @f[start_peer_connection].  Once  
started, peer connections are used just like any others.  
@f[start_peer_connection] is documented in the Library Reference Manual.  
  
@Chapter(Synchronous and Asynchronous Operations)  
@label(async)  
  
  
Most of the GDB programs discussed so far have been completely  
@i[synchronous], meaning that they only try to do one thing  
at a time.  The @f[perform_db_operation] and @f[db_query] routines  
@index(db_query)  
@index(perform_db_operation)  
retain control until the requested operations have run to completion,  
which makes it impossible for a program to do several queries in  
parallel, or to continue with its own execution while a database is  
being updated.  GDB also provides @i[asynchronous] versions of many of  
@index(Asynchronous operations)  
its services, allowing execution of the client to proceed while the  
requested operation is attempted.  Asynchronous programming is usually  
more difficult than synchronous, but it is much more flexible.  This  
chapter describes GDB's facilities for managing asynchronous  
activities.  
  
@section(Operations)  
@index(Asynchronous operations)  
@index(OPERATION)  
  
An asynchronous program may have several operations running in  
parallel while its own local execution continues.  In order to track  
the progress of these activities, the program uses an OPERATION  
@index[OPERATION]  
variable for each one, as illustrated in Program  
#@ref(ProgramNumber).  This program fragment presumes that DATABASEs  
@f[personnel] and @f[inventory] have been successfully accessed.  
Several important points are illustrated by the example.  Each  
operation variable must be initialized with the @f[create_operation]  
@index(create_operation)  
function.  Like most other data structures in GDB, OPERATIONS are  
really pointer types;  @f[create_operation] allocates and initializes  
the data structure which GDB actually uses to track the operation.    
  
@begin(float)  
  
@center(PROGRAM #@ref(ProgramNumber))  
@set(ProgramNumber=+1)  
@begin(example)  
	DATABASE personnel, inventory;
	char     *quel_command;
	RELATION new_recruits;

	OPERATION query_op, append_op;

       /*
        * Each operation must be created before it 
        * can be used
        */
	query_op = create_operation();
	append_op = create_operation();

       /*
        * Start an append to the parts database in parallel
        * with a query to the personnel database. We assume that
        * both DB's are accessed and the relation was created.
        */
	quel_command=
   "append to  parts (type=\"resistor\", price = .12, quantity=150)";
	retcode = start_performing_db_operation(append_op, 
						personnel,
						quel_command);
	start_db_query(query_op, personnel, new_recruits,
		 "(>*name*<=recruits.name, >*rank*<=recruits.rank, 
		  >*serial*<=recruits.serial) 	
		 where recruits.rank >10");

       /*
        * Wait for both to complete.  Order of the following 
        * two statements does NOT matter!
        */
	complete_operation(append_op);
	complete_operation(query_op);

	/*
	 * Check for errors
	 */
	if(OP_RESULT(append_op) != OP_SUCCESS ||
	   OP_RESULT(query_op) != OP_SUCCESS)
		fprintf(stderr,"Something didn't work\n");
@end(example)  
@end(float)  
  
There are many ways to check for progress or await completion of GDB  
operations.    
One of the simplest is @f[complete_operation], which is  
@index(complete_operation)  
shown in this example.  The program hangs in GDB until the specified  
operation completes.  As noted in the program comments, the order of  
the two calls to @f[complete_operation] is @i[not] significant.  This  
might be surprising, as the calls seem to imply that @f[append_op]  
must complete before @f[query_op] will proceed.  That is not the case.  
@i[Whenever GDB is given control for any reason, it always makes  
progress as quickly as possible on all pending operations.]  This  
means that @f[query_op] may progress or even complete while   
awaiting completion of  
@f[append_op].  Should @f[query_op] complete before  
@f[append_op], the second call to @f[complete_operation] will not delay  
at all.  
  
Each operation has two types of status, which may be queried by  
@f[OP_STATUS] and @f[OP_RESULT] respectively.  OP_STATUS is used to  
@index(OP_RESULT)  
@index(OP_STATUS)  
track the progress of an operation while it is executing.  It has the  
values shown in table @ref(OP_STATUS).  
  
@begin(table)  
@caption(OP_STATUS Values)  
@tag(OP_STATUS)  
@tabset(.75in, 2.5in)  
  
@\OP_NOT_STARTED@\The operation  
@\@\variable is not in use.  
  
@\OP_COMPLETE@\The operation completed  
@\@\without any transmission  
@\@\related errors.    
  
@\OP_CANCELLED@\GDB was forced to  
@\@\give up on the operation  
@\@\prematurely. The likeliest  
@\@\explanation is failure of  
@\@\the connection.  
  
@\other@\OP_STATUS takes on   
@\@\several other values while  
@\@\an operation is queued or  
@\@\executing.  These are not  
@\@\of concern to users of  
@\@\the library.  
@end(table)  
  
  
The operation should be considered running until its status becomes  
either  
@f[OP_COMPLETE] or @f[OP_CANCELLED].  The macro @f[OP_DONE] may be  
@index(OP_DONE)  
@index(OP_CANCELLED)  
@index(OP_COMPLETE)  
@index[Return codes from GDB operations]  
used to test for both of these.  When an operation reaches  
@f[OP_COMPLETE] status, then the @f[OP_RESULT] macro may be used to  
get the return code from the operation itself.  In the case of an  
Ingres query, for example, the operation will be considered complete  
when all results have been returned to the client.  @f[OP_STATUS] will be  
@index(OP_STATUS)  
@f[OP_COMPLETE] and @f[OP_RESULT] will contain the return code from Ingres.  
If the connection fails before the query is successfully attempted,  
@f[OP_STATUS] will be @f[OP_CANCELLED] and @f[OP_RESULT] will be  
undefined.  The result value @f[OP_SUCCESS] is usually used to  
indicate that an operation completed successfully.  
  
@f[OPERATIONS] are created in the state @f[OP_NOT_STARTED].  After  
@index(OP_NOT_STARTED)  
use, the state is generally @f[OP_CANCELLED] or @f[OP_COMPLETE].  It is  
possible to re-use such an @f[OPERATION], which saves the overhead of  
deleting and re-creating it.  You must use @f[reset_operation] to  
prepare an operation for re-use, returning it to the state  
@f[OP_NOT_STARTED].   
  
Hint: it is often handy to create arrays of operations, or to include  
@Index(Arrays of operations)  
operations in other C structures.  Make sure to do a  
@f[create_operation] on each one before it is used.  
@f[delete_operation] may be used to reclaim the space for operations  
@index(delete_operation)  
which have terminated and are no longer needed.  
  
@section(op_select_any and op_select_all)  
  
In certain programs you may wish to take explicit action when any of a  
list of pending operations completes, or you may wish to monitor  
activities on file descriptors not controlled by GDB.  Berkeley Unix  
uses @f[select] to control asynchronous file handling, and GDB  
@index(select)  
provides two generalized forms which facilitate control of GDB's  
operations.    
  
@f[op_select_any] has semantics similar to the select system  
@index(op_select_any)  
call, but in addition, it allows pending GDB operations to  
progress.  Along with a list of file descriptors, you must give  
@f[op_select_any] a list of the GDB operations whose completion is of  
interest.  Like @f[select], this routine waits quietly when all  
descriptors are blocked.  Unlike @f[select], @f[op_select_any] allows  
processing to proceed on all GDB connections, and it returns only when  
one of the operations specified in the list actually completes.  While  
@f[select] would return whenever any data could be read or written on  
a connection, @f[op_select_any] returns only when enough progress   
been made that one of the specified operations actually completes.  One  
may think of @f[op_select_any] as raising the level of abstraction on  
a connection from single bytes to entire operations.  As with  
@f[select], you may supply your own file descriptors and a timeout,  
and GDB will treat these in the same manner as @f[select] would.  
@f[op_select_all] is similar to @f[op_select_any], but it waits for  
@index(op_select_all)  
all of the specified operations to complete before returning.  
  
Both of these functions take as arguments a @f[LIST_OF_OPERATIONS],  
@index(LIST_OF_OPERATIONS)  
which may be created using the @f[create_list_of_operations] function.  
@index(create_list_of_operations)  
Since manipulating lists is clumsy, it is sometimes useful to create  
one long list with all the operations you ever expect to wait for, and  
just pass that to @f[op_select_any].  Once an operation has completed,  
you should use @f[reset_operation] to keep it from pre-satisfying the  
next select call.  
The program in Appendix @ref(nonforkingserver) illustrates all of these  
techniques.  
  
@chapter(Hints and Tricks)  
@blankspace(2in)  
@center(To be supplied.)  
@chapter(Bugs)  
@blankspace(2in)  
@center(To be supplied.)  
  
@chapter(How GDB Works)  
  
The ultimate authority on this is, of course, the GDB code.    
@index[source code organization]
Most of it is pretty well commented (some would say over-commented,  
but that is my style), which means that you can usually figure out  
what's happening IF you know where to look and have some general idea  
of how things work.  
This chapter presents the general concepts and implementation  
techniques which underlie GDB.  
It is not comprehensive, but I do recommend that you read this before  
looking at the code.  
  
GDB is organized into several interacting sub-components or layers.  
Some are so trivial as to require little or no explanation.    
Others, such as the layers which do asynchronous operations and  
communication,  
are much more complex.  
The sections below describe the components of GDB and their  
relationship to each other.  
A thorough knowledge of Berkeley Unix, C programming, and creation and  
manipulation of sockets is presumed.  
It is also presumed that the reader is familiar with the other  
chapters in this guide.  
  
@section(General Code Layout)  
  
GDB is supplied as a suite of .c source files which are compiled to  
build the libgdb.a library archive.  The gdb.h include file is used by  
all of these, and also by user applications.  
@index[dbserv, compiling]
The @f[dbserv] database server is supplied in a separate @f[dbserv.qc]  
file, which can @i[only] be compiled on a machine which has RTI Ingres  
installed and accessible.  See RTI's documentation for details.  
If you are not using dbserv then there is no need to build it.  The  
rest of GDB, including database client applications, may be compiled  
and used on machines which do not have RTI's products installed.  
  
The file named gdb.c contains the routine @f[gdb_init], which is  
@index[gdb_init]
called first by all users of gdb.  
Initialization of GDB data structures can be tracked by reading this  
source.  
This source file also contains a few utility routines.  
Most of the other source is organized more or less by layer, with some  
of the larger layers split into several source files.  
The chapters below give hints, or you can grep for the name  
of the routine in which you are interested.   
It is always safe to grep in g*.c; this will get all of the executable  
GDB source.  
  
gdb.h is organized more or less by gdb layer, with a ^L page break  
@index[gdb.h]
between each section.  
You should always have the corresponding section of gdb.h handy while  
trying to learn about a piece of GDB.  It is essential.  
  
@section(Memory Management)  
  
@index[Memory management]
GDB makes extensive use of dynamically allocated (heap) memory.  
ALL calls for dynamic memory are done through the two routines  
@index[db_alloc]
@index[db_free]
@f[db_alloc] and @f[db_free].  These are actually defines in gdb.h  
which cause invocation of @f[*gdb_amv] and @f[*gdb_fmv] respectively.  
@index[gdb_amv]
@index[gdb_fmv]
@index[gdb_am]
@index[gdb_fm]
These in turn point, by default, to @f[gdb_am] and @f[gdb_fm], which  
just call malloc and free.  
This is all done so an application can easily replace the default  
memory allocators.   
The addresses of the new routines should be plugged into the vectors  
prior to calling @f[gdb_init].  Note that the interfaces are @i[not]  
quite the same as to malloc and free;  check the source for details.  
  
Many dynamically allocated GDB objects contain 4 character @f[id]  
fields.  These are just eye catchers for debugging, initialized to  
"REL" for relations, "TUP" for tuples, and so on.  
Most parameters passed by users are  
checked for these fields, which helps catch bad parameters and some  
memory management errors.  
  
@section(GDB Type Management)
@index[GDB types]
@index[Types]
 
Make sure you understand GDB types from a users point of view (Chapter  
@ref[structure]) before continuing here.  
  
The primary purpose of GDB types is to support the polymorphic  
@index[Polymorphism]
operations @f[start_sending_object] and @f[start_receiving_object].  
These two operations suffice to send and receive @i[any] GDB object,  
regardless of its structure or complexity.  
@index[Type definition]
@index[GDB Type definition]
Given these limited goals, GDB needs only some very basic information  
about its types:  
  
@begin(itemize)  
The length of an item as represented locally (may depend on local  
machine type).  
  
Required alignment (e.g. 4 or fullword, 8 for double)  
  
A function to return a null value for the type.  
This is used to initialize newly allocated variables.  
  
A function to encode values of this type for transmission.  
  
A function to decode data of this type from its transmitted form to the  
local representation (which is usually machine dependent.)  
  
A function to return the length which a value would take when encoded  
for transmission.  
GDB calls these functions to decide how much memory to allocate for  
the transmission buffer before calling the encode routine to fill in  
the buffer.  
  
A format function which is used to format variables of this type for  
debugging output.  
  
A string name for the type.  Used only for debugging output.  
@end(itemize)   
  
@index[Properties]
@index[FCN_PROPERTY]
@index[INT_PROPERTY]
@index[STR_PROPERTY]
All this information is coded into the array named g_type_table.  
The macros INT_PROPERTY, FCN_PROPERTY, and STR_PROPERTY are used by  
the GDB code to index into the type table and pull out any desired  
property for the given type.  
For example, INT_PROPERTY(REAL_T, LENGTH_PROPERTY) returns the length  
of the local representation of a real number.  
  
@begin(example)  
       FCN_PROPERTY(REAL_T, ENCODE_PROPERTY) (....)  
@end(example)  
  
calls the encode routine on a real number, with the supplied (....)  
arguments.  Though the macro expansions look messy, they are designed  
to be optimizable by a good compiler.  
  
The routine @f[gdb_i_stype] is used to initialize the type table,  
@index[gdb_i_stype]
thereby defining types.  The typing system is extensible in that new  
types may be defined by adding entries to the type table at any time.  
The most difficult aspect of this is writing the encode and decode  
functions for the new type.  Note that the routines for encoding and  
decoding structured types, like tuples and relations, generally make  
repeated calls to the encoding routines for components.  Thus, the  
relation encoding routine calls the tuple encoding routine, which  
calls the routines for the various fields.  The end result is a single  
buffer containing the entire structured datum flattened for  
transmission.  
  
@subsection (The TUPLE_T and RELATION_T Structured Types)  
  
@index[TUPLE_T]
@index[RELATION_T]
The @f[TUPLE_T] and @f[RELATION_T] types are supplied with GDB; they  
are commonly used to contain the results of a relational database  
query, but they are also useful for transmitting other types of  
structured data.  For brevity, we will refer simply to tuples and  
relations.  
  
Both tuples and relations are self-describing.    
Given a tuple or relation, one can determine its complete structure as  
well as its contents.  
A tuple is an ordered, non-extensible collection of named, typed,  
fields, and a relation is an ordered, extensible collection of tuples.  
Because one program, and certainly one relation, will commonly use  
many tuples with the same structure (i.e. the same field names and  
types), the description information for these tuples may be shared in  
a single @f[TUPLE_DESCRIPTOR].   
@index[Tuple descriptors]
The tuple descriptor is allocated and initialized by  
@f[create_tuple_descriptor] with the names and types of a set of  
fields.  
Internally, the descriptor is also initialized to contain the offset  
and length of the fields within a tuple.  
Accessing a field within a tuple is thus extremely fast, as the  
offsets are pre-computed.  
Note that the offsets and lengths of the fields are local properties  
which may vary from one machine type to another.  Also, this structure  
supports retrieval of a field whose index or name (and thus its type)  
is variable at execution time.  
  
A single tuple descriptor may be shared by an arbitrary number of  
tuples, some of which may be members of relations.    
Also, GDB may implicitly allocate tuple descriptors for tuples and  
relations received through the network.  
To facilitate management of these descriptors GDB uses reference  
counting.  Each time a descriptor is used in a tuple or  
relation, its reference count is incremented.  Each time such a tuple  
or relation is deleted, the reference count is decremented.    
When the reference count goes to 0, the descriptor itself is  
reclaimed.  
@i[As a general rule, users should explicitly de-allocate the  
descriptors, tuples, and relations that they create, and they should  
de-allocate objects that they explicitly receive from the network.  
Implicitly created descriptors will be reclaimed automatically by the  
system.]  
  
@index[TUPLE]
@index[TUPLE_DESCRIPTOR]
@index[RELATION]
The names @F[TUPLE, TUPLE_DESCRIPTOR] and @f[RELATION] supplied in  
gdb.h are actually pointers and may be manipulated as such (e.g.  
copied, passed as arguments to functions, etc.)  The create functions  
dynamically allocate memory and fill in the pointers.  
  
A tuple descriptor consists of a fixed header, an array containing  
descriptive information for each field, and then a contiguous list of  
null terminated strings, which are the text names of the fields.  
  
A TUPLE is a pointer to a single contiguous chunk of memory  
containing a header followed by the data for the tuple fields.  
The header includes chain pointers, which are used @i[only] when the  
tuple is contained in a relation, and a pointer to the corresponding  
tuple descriptor.  The data follows immediately, at the offsets listed  
in the tuple descriptor.    
Offsets are relative to the start of the first field, not the tuple  
header.  
  
Relations consist of a header structure circularly double linked to a  
(possibly null) list of tuples.  
The last tuple points to the relation header, as does the back pointer  
from the first tuple.  
A null relation has forward and back pointers to itself.  
The queue manipulation macros like ADD_TUPLE_TO_RELATION depend on the  
@index[ADD_TUPLE_TO_RELATION]
fact that the next and prev fields are at the same offset in both the  
tuple and relation header data structures;  this may not actually be  
true on machines which allocate structures backwards, and as much, it  
represents a non-portability.  
  
@section(Communications and Asynchronous Operations)  
  
@index[Communication services]
@index[Asynchronous operations]
This layer of GDB provides management for connections between programs  
on separate hosts, and for asynchronous transmission of typed data  
between those hosts.  It relies on the memory management and data  
typing layers discussed above.  Due primarily to the extensive support  
for asynchronous, non-blocking communication on multiple connections,  
this is by far the most complicated layer of GDB.  Rather than trying  
to discuss every detail, I will emphasize the most important general  
concepts, and give warnings about implementation features which may be  
difficult to follow in the code.  
  
@subsection(Connections)  
  
@index[Connections]
A @f[connection] is an abstraction for an asynchronous, reliable,  
full-duplex path on which GDB can transmit and receive its typed data.  
The current implementation uses Berkeley TCP stream sockets for  
transport.  
  
Unlike most GDB data structures, all connection descriptors are  
@index[gdb_cons]
statically allocated in the @f[gdb_cons] array.  When a new connection is  
started, the first available slot in the array is returned as the  
corresponding connection descriptor.  NOTE:  the actual state of each  
connection descriptor is contained in its status field.  @f[gdb_mcons]  
is a high water mark variable indicating the highest numbered  
connection which has ever been used;  it provides an optimization for  
the common case of a process which only uses one or two connections.  
Below the water mark, the status fields must be checked to determine  
whether a connection descriptor is actually in use.  
@index[GDB_MAX_CONNECTIONS]
GDB_MAX_CONNECTIONS determines the size of the array, and hence limits  
the number of simultaneous connections which may be controlled by a  
single process.  The GDB library may be rebuilt with a larger value if  
desired.    
  
Each connection actually consists of two half connections, one for  
each direction.   In the current implementation, they use the same  
file descriptor, but this could be changed for other networking  
environments.  There is also some stray code which refers to  
out-of-band connections.  This was intended to be used for out of band  
signalling, primarily to cancel ongoing operations.  It was never  
implemented.  
  
@subsection(Operations and Half Connections)  
  
@index[Operations]
@index[Half connections]
Each transmission operation which GDB supports on a half connection is  
fundamentally asynchronous, meaning that the application program can  
continue to execute while the operation proceeds.    
GDB is @i[not] signal driven;  it uses non-blocking I/O to make as  
much progress as possible whenever it gets control, leaving 
@index[SIGIO]
SIGIO available to
the application 
programmer.  GDB does set a handler for SIGPIPE.
@index[SIGPIPE]
Though  
synchronous versions are provided for most operations, these usually  
are implemented by invoking the asynchronous version and then  
immediately waiting for it to complete.  
  
Each pending operation is represented by a data structure of type  
OPERATION (actually, OPERATION is the pointer to the data structure)  
@i[which is queued on the corresponding half connection.]  It is thus  
a fundamental limitation of the current version of GDB that  
asynchronous activities exist @i[only] in the context of a connection.  
The only reason that such an operation may ever block is due to lack  
of progress in data transmission on the corresponding path.  GDB does  
not provide for any more general kind of multiplexing or lightweight  
process management.  
  
The operation data structure contains the complete state of the  
pending operation.  This is summarized in a status field, which  
indicates that the operation is either OP_QUEUED (waiting behind  
others in a queue), OP_RUNNING (at the head of the queue and  
proceeding as fast a data transmission will allow), OP_COMPLETE  
(completed without GDB detected error) or OP_CANCELLED (abandoned  
before completion, usually due to loss of the corresponding  
connection.)  There is also a result field, which becomes valid  
@i[only] when and if the operation reaches the OP_COMPLETE state.  It  
is effectively the final return code, with legal values depending on the  
operation being attempted.  In addition to these state fields, the  
operation structure contains a pointer to a function which is to be  
invoked the next time progress can be made (i.e. the next time the  
communication path unblocks) and the @f[arg] field, which points to a  
dynamically allocated structure used by the functions to contain their  
own ongoing state.  
  
The operation actually running on each half connection is the one at  
the head of the queue.  The only reason such an operation may be  
blocked is because it has tried to send or receive data which the  
@f[gdb_move_data] routine was unable to move immediately.  The pending state of
this partially 
completed  
@index[next_byte]
@index[remaining]
transmission is summarized by the @f[next_byte] and @f[remaining]  
fields of the half_connection structure.  If an operation seems not to  
be progressing, and the @f[remaining] field is non-zero, it may be  
that the socket is not making progress.      
  
The heart of GDB's transport layer is a routine called  
@index[gdb_progress]
@f[gdb_progress], which has the effect of making all possible progress  
on all pending operations regardless of the connections involved.  
This routine never blocks.    
Instead, it cycles through the connections, trying repeatedly to make  
progress, and returning when no further progress can be made.  
The actual work is done by 
@index[gdb_hcon_progress]
@f[gdb_hcon_progress], which is one of the  
most important (and deceptively complicated) in GDB.  
If the operation at the head of the queue has never been run before,  
then its initialization function (@f[*init] from the operation  
structure) is called to start it off.  
If the operation has been run before, then it @i[must] be blocked  
waiting for data to be transmitted.  In this case, 
@index[gdb_move_data]
@f[gdb_move_data]  
is called to move the data.  
If that still results in blockage, then no further progress can be  
made;  
otherwise, the operation's continuation function (@f[*cont] from the operation  
structure) is called to process the data or proceed in some other  
manner.  
  
Most of the subtlety in this area comes from the interplay between  
multiple connections and operations.  
If connection 1 blocks, connection 2 may be able to proceed. By the  
time 2 finishes, 1 may be able to progress after all.  For this  
reason, @f[gdb_hcon_progress] is called over and over again until a  
full sweep is made through all active connections without a report of  
any progress at all.  Furthermore, it is sometimes the case that an  
ongoing operation will actually create and queue new operations on  
either the same or a different connection, which may result in a  
recursive invocation of @f[gdb_progress].  The 
@index[HCON_BUSY]
@f[HCON_BUSY] flag is  
used to avoid recursive processing of the same half connection.  
Another subtle flag is 
@index[HCON_PROGRESS]
@f[HCON_PROGRESS], which is used by  
@f[gdb_move_data] to indicate whether progress was made on a given half  
connection.  It looks to me in hindsight like this should have been  
passed as a parameter, perhaps there was a good reason why not.  In  
any case, it is effectively a value returned from @f[gdb_move_data].  
A final complication arises because an operation is allowed to  
re-queue itself from one half connection to another.  This effectively  
puts it back from OP_RUNNING to OP_QUEUED state, to be caught by a  
subsequent sweep of @f[gdb_hcon_progress].  
  
@subsection(GDB Select Features)  
  
@index[Select operations]
In a Berkeley Unix system, the customary means of waiting for I/O to  
progress is to hang in the @f[select] system call.  
One of the purposes of GDB is to hide byte level communication from  
applications, which are interested only in the progress of their higher  
level requests to GDB.  For this reason, GDB provides the new  
selection operations @f[op_select_any] and @f[op_select_all].  The  
former is also known by the historical name @f[op_select], which will  
be used here for brevity.  It is also the more interesting of the  
two, since @f[op_select_all] is built trivially from @f[op_select].  
  
@f[op_select] is given a list of queued and/or completed GDB  
operations, a list of file descriptors not controlled by GDB, and a  
timeout value in the same form as for Berkeley select.  @f[op_select]  
returns as soon as (1) one of the operations supplied is completed or  
cancelled (2) a select call indicates that progress can be made on one  
of the other file descriptors or (3) the timeout is satisfied.   
Actually, the current version of the system has a bug which  
effectively resets the time every time any connection makes progress.  
  
@index[con_select]
The real work here is done in the routine named @f[con_select].  This  
routine contains the @i[only] significant @f[select] call in the  
entire GDB system.  There are a few others, but those never block.  
@f[con_select] is similar to a real select except that: (1) it  
implicitly selects all file descriptors controlled by connections, as  
well as those explicitly specified (2) it allows transmission and  
receipt to progress on all connections and (3) it considers a  
connection to be selected if and only if a transmission operation  
which had been pending becomes complete.  One may consider that  
@f[con_select] turns the fd's controlled by sockets into operation  
streams rather than byte streams.  Note also that this operation  
differs from a traditional select and op_select in that it is not  
robust against waiting for connections with pre-completed activity.  
This could be added, but since it's an internal routine anyway, it  
seems not to be worthwhile.  Also, this routine presumes that all  
possible progress has been made before con_select is invoked.  
  
Con_select does not know about the specific list of operations for  
which completion is awaited.  Op_select and the other selection  
routines call con_select repeatedly until the termination criteria are  
satisfied.   
  
@index[complete_operation]
Many simple applications use @f[complete_operation] rather  
than the more complex selection operations to await completion of  
asynchronous activities.  This routine merely calls con_select  
repeatedly until the desired operation completes.  NOTE: even though  
the user is waiting for activity on a particular operation, all  
connections and all operations progress.  This tends to make it easier  
for the programmer to avoid deadlocks, and it means that the order in  
which complete_operation requests are issued is frequently  
unimportant.  
  
@subsection(Connection Error Handling)  
  
@index[Error handling]
For reasons beyond the control of GDB, connections may fail at any  
time, and GDB must be prepared for this.  
In general, all queued operations on the connection are cancelled.  
Note that the application must still sever the connection, to indicate  
that it is no longer using its pointers to the connection descriptor.  
  
GDB operations are responsible for implementing their own error  
recovery.  
None of the existing operations do anything elaborate, and many of  
them cannot be cancelled at all.  It would be very difficult to  
provide on the fly cancellation of complex operations like database  
queries, given the asynchronous, full duplex nature of their  
operation.  
Right now, the only way to prematurely terminate such an operation is  
to sever the corresponding connection and restart it.   
This is always safe, if not convenient.  
  
@section(Server/Client Management)  
  
@index[Server/client management]
GDB supports two general styles of communicating programs:  
peer-to-peer, and server client.    
Within the framework of the server/client model, GDB supports both  
forking and non-forking servers.  The interfaces to the two server  
types are identical, so a given client need not be aware of the style  
of its server.  
  
@subsection(Non-forking servers)  
  
@index[Non-forking servers]
@index[Servers, non-forking]
GDB's asynchronous communication primitives support the creation of  
non-forking servers, in which multiple clients are serviced by a  
single Unix server process.  The maximum number of clients  
which can be served by a single non-forking server process is  
GDB_MAX_CONNECTIONS-1  (one connection is used for the "listening  
connection", described below.)   In addition to  
@f[start_sending_object] and @f[start_receiving_object] discussed  
above, @f[start_accepting_client] provides a service for  
@i[asynchronously] acquiring connections from new clients.  
Start_accepting_client connection is an asynchronous GDB operation,  
which may be queued @i[only] on special connections created with the  
@f[create_listening_connection] primitive.  
Create_listening_connection allocates a connection descriptor, creates  
a listening Berkeley Unix Internet Stream socket, and binds a supplied  
port address to it.  Only the inbound half connection of the  
connection descriptor is significant.  
  
The structure of start_accepting_client is subtle, and it  
serves as a good example of some of the complex ways in which GDB  
operations can be created. At the time it is issued @i[two] operations  
are queued on the listening connection. The first one merely does the  
accept of a new connection, but without filling in any of the  
connection data structures.  The second one is the one which uses the  
operation descriptor passed by the caller.  Its init routine is  
g_iacc, which therefore runs @i[after] the connection has been  
acquired from the kernel.  This routine proceeds to fill in the new  
connection descriptor so that communication may be done on the new  
stream using GDB's services.  It also calls g_ver_iprotocol to insure  
that both sides are running compatible versions of GDB.  
  
At this point, the new connection exists, but we still have to acquire  
the client's parameters.  This too must be done asynchronously.  On  
the new connection, a @f[start_receiving_object] is queued, and then  
the @f[start_accepting_client] re-queues itself onto the data  
connection.  Success of the start_accepting_client now depends on  
completion of activity on the newly created data connection, not the  
listening connection.  When the start_receiving_object for the client  
data finally completes, the original start_accepting_client operation,  
now requeued behind it, takes control again, this time in the routine  
g_i2acc.  This merely checks to insure that the receive completed  
successfully.  It's final status, returned to the caller as OP_STATUS,  
is the same as that of the receive.  The net result of all this is a  
newly allocated connection for which the accept and the acquisition of  
the initial parameters were all done without blocking.    
  
At this point a tentative connection exists, but the server still has  
three options in dealing with the new client.  These are (1) accept  
the new client (2) refuse the client or (3) bounce the client to  
another server.  To indicate the disposition, the server application  
must issue the @f[start_replying_to_client] asynchronous operation,  
indicating either @f[GDB_ACCEPTED, GDB_REFUSED,] or @f[GDB_FORWARDED].  
@f[start_replying_to_client] creates a tuple containing the response  
parameters to be sent to the client.  It then queues a  
@f[start_sending_object] for that tuple on the client connection, and  
queues itself behind that.  By the time @f[start_replying_to_client]  
receives control again, the send of the tuple has completed.  The  
tuple and other dynamically allocated data is freed, and the return  
code passed back to the application.  
  
@subsection(Forking Servers)  
  
@index[Forking servers]
@index[Servers, forking]
GDB provides a @f[create_forking_server] routine which does most of  
the bookkeeping required for a single server program to support an  
arbitrary number of clients, forking once each time a new client  
connects.  The number of simultaneous clients supported is limited  
only by the number of processes Unix allows the server to create.  
A GDB forking server has a single parent process, which hangs forever  
in the @f[create_forking_server] routine, and one child for each  
active client.  @f[create_forking_server] returns in each child the  
connection descriptor for the corresponding client.  The usual GDB  
communication services may then be used to communicate with the  
client.  The supplied @f[dbserv.qc] database server is an example of a  
forking server.  
  
Internally, create_forking_server begins in the parent by setting up  
@f[gdb_reaper] as the handler for @f[SIGCHLD].  GDB thus reaps dying  
children automatically for the application.  
  
In the parent, @f[create_forking_server] issues an ordinary  
@f[create_listening_connection] to establish its own server address.  
It then loops forever using @f[start_accepting_client] to acquire  
sockets for new clients.    
The application may supply a validation routine which is used to  
determine whether a given client is to be accepted by the server.  If  
the validation routine returns FALSE, then the  
@f[start_replying_to_client] is used to refuse the connection.  
Otherwise, the server forks the and the child issues a  
@f[start_replying_to_client] to accept the connection.  In the  
parent, the client connection is severed (to prevent file and  
connection descriptor leakage) and the listening operation is reset  
so that it may be used again.  @f[create_forking_server] thus  
hides the complexity of dynamically acquiring connections, forking,  
replying to clients, and reaping 'dead' children.  
  
The @f[gdb_debug] routine may be used to set a flag called  
@f[GDB_NOFORK]. When this flag is set, @f[create_forking_server]  
semantics are changed so that no fork is done when the first client  
connects.  Instead, the single client is served directly by the  
parent.  This flag is useful when using debuggers like dbx, which are  
incapable of debugging forking programs.  
  
@section(Peer-to-peer Communication)  
  
@index[Peer-to-peer communication]
@f[start_peer_connection] is used by programs wishing to do  
symmetrical communication, in which neither is the server or the  
client.  
The current implementation tries first to connect to the other side,  
and if that fails, it hangs in an accept.  There is a race condition  
in which the programs may hang if both are started at the same time;  
each tries a connect, which fails, and then each side hangs in an  
accept.  The only solutions I know to this problem involve either (1)  
unconditionally creating multiple connections between the two sides,  
or (2) running randomized delay loops to re-try the connects  
periodically.  Even option (2) will sometimes result in multiple  
connections.  Either of these implementations is a possibility for the  
future.  In the meantime, @f[start_peer_connection] merely fills in a  
connection descriptor, and does connects and accepts as described  
above.  
A check is also made to insure that both partners are running  
compatible versions of GDB (see g_ver_iprotocol and g_ver_oprotocol.)  
  
@section(Database Management)  
  
@index[Database management]
GDB's database services are cleanly layered on top of the GDB 
facilities described above.  The supplied @f[dbserv.qc] is a forking 
server implementing a fairly complete set of database access protocols 
for remote clients.  Client functions are provided which encapsulate 
most of the low level GDB transmission services used for accessing the 
server.  

Database clients use operations like @f[start_db_query] and 
@f[perform_db_operation] to manipulate the database.  
These functions make extensive use of GDB's 
asynchronous communication capabilities to maximize the overlap 
between server and client processing, and to achieve pipelining of 
data transfers for retrievals returning large quantities of data. 
Application programmers are given asynchronous versions of most 
database functions, allowing them to overlap their own execution with 
GDB's access to the database.  A single application may have 
simultaneous connections to multiple databases at multiple sites, and 
may have parallel operations in progress to the several databases 
simultaneously.  The current implementation allows a single client to 
have multiple connections to a single database, with (simulated) 
parallel activity. 
 
@subsection(Database Client Operations) 
 
@f[start_accessing_database] returns a pointer of type DATABASE to a 
dynamically allocated structure used as a handle for subsequent access 
to that database.  Stored within the structure is a pointer of type 
CONNECTION, which is used for data transmission to and from the 
server.  In reviewing the code I am embarrassed to note that the 
@f[terminate_db] function described in the library reference manual 
was never implemented.  It's function should be to sever the 
corresponding connection and de-allocate the database descriptor.  In 
fact, the client may safely terminate or sever the connection at 
anytime if desired.  The server will detect the loss of the connection 
and do the appropriate cleanup. 
 
The application uses @f[start_performing_db_operation] and 
@f[start_db_query] and their synchronous equivalents to perform 
operations against the database.  @f[start_performing_db_operation] is 
suitable for any Ingres QUEL operation which is expressible as a 
single string, and which returns only an integer return code.  This is 
in fact true of almost all QUEL statements except for retrieve, which 
returns a relation as well as a return code.  @f[start_db_query] is 
used for retrieval. 
 
Both of these database access operations are asynchronous and full 
duplex.  This means that an application can stream multiple requests 
to the server, mixing regular operations in with retrievals.  The requests are transmitted 
to the server as fast as possible, and the responses are received 
asynchronously, but in order, as they become available. 

@f[start_performing_db_operation] takes the string containing the QUEL 
statement, converts it to the GDB type STRING_T, and queues an 
ordinary @f[start_sending_object] for the string on the outbound half 
connection.  It then queues a @f[start_receiving_object] of an 
INTEGER_T on the inbound connection to receive the corresponding 
return code, and queues itself behind that on the @i[inbound] 
half connection.  By the time this operation takes control again, 
the return code has been received, and cleanup can be done.  Note that 
all phases of this activity are fully asynchronous.  In particular, 
the outbound and inbound activities are completely independent as seen 
at the client.  Of course, the server does not generate the return 
code until after it has inspected the request, but the client side is 
fully asynchronous.  If another request is issued by the application, 
then the outbound 
data is queued for transmission immediately, regardless of how far the 
first operation has progressed, and the operations to receive the 
return code are queued behind those from the first request. 
For example, applications commonly queue in immediate succession the 
operations to begin a transaction, do several updates, and commit 
the transaction, waiting for completion only on the commit.  These 
activities proceed much more rapidly than they would if each piece 
were implemented synchronously. 
 
@f[start_db_query] is similar in spirit, but it must also handle the 
receipt of the retrieved relation. 
In addition to sending the query string itself, the client sends a 
copy of the tuple descriptor for the relation to be created.  This 
allows the server to properly format tuples for transmission back to 
the client, and it also provides type information for each field. 
As above, a receive operation is queued for a return code, and then 
the query operation itself is queued on the inbound half connection. 
It receives control in function g_idbq once the return code has been 
received.   If the return code indicates failure, then the 
operation is complete.  If success is indicated, then the resulting 
(possibly null) relation must be received from the server. 
 
There is no way to tell at this point how many tuples must be
received.  The server therefore prefaces each tuple transmission with
a YES/NO flag, indicating whether another tuple is to follow.  The
client repeatedly receives these flags, and each time a YES is
received, it prepares to receive another TUPLE.  These tuples are then
inserted into the application's relation.  The @f[g_cdbq] routine, which gets
control each time a flag or tuple is received,  runs in the
context of the OPERATION supplied by the application.  Each time it
receives control, it preempts itself by issuing
@f[preempt_and_start_receiving_object], effectively sticking a receive
operation ahead of itself on the inbound half connection.  This
continues until all of the flags and tuple data have been received.
 
Note that @f[start_db_query] and the corresponding server could have 
been written to move the entire relation as a single GDB object. 
Though the code for this is much simpler, it means that the entire 
relation must be prepared at the server before any transmission can 
begin.  The current implementation provides pipelining;  each tuple is 
sent on its way as soon as it is retrieved from Ingres.  Transmission 
time tends to be hidden by the overhead of the Ingres retrieval loop. 
 
@subsection(The dbserv.qc Database Server) 
 
@index[dbserv]
As noted above, the supplied Ingres server is an ordinary GDB forking 
server whose purpose is to receive database requests from clients and 
pass them on to Ingres. 
Most of its operation is straightforward, with transmission of query 
results being a notable exception. 
 
When dbserv receives a query, it first parses the query string to 
determine which tuple fields will be assigned by the retrieval. 
The array named @f[offs] in the routine @f[do_query] is 
filled in with the offset that each retrieved field occupies in the 
tuple. 
A query buffer is prepared containing the text of the query in the 
form required by Ingres.  The Ingres type indicators (e.g. %i4 for 
integer) are placed into the queries in place of the GDB field name 
indicators.  
 
Text fields are handled specially, because their lengths cannot be 
determined until after retrieval.  do_query has a large local array of 
character strings, into which the actual Ingres retrieval is done. 
The routine named @f[fix_strings] is then used to copy the resulting 
text into GDB STRING_T variables, which are assigned to the tuple 
fields.  Copies are needed because the large text buffers may be 
re-used for subsequent retrieves before the earlier tuples have been 
transmitted.  
 
The main retrieval loop in @f[do_query] repeatedly retrieves 
information from Ingres into a dummy tuple.  This is promptly copied 
to buffer it for transmission, and @f[fix_strings] is called, as noted 
above. 
 
do_query may have several transmissions outstanding at once, the maximum 
being determined by the constant @f[OVERLAP].  The @f[pending] 
structure array, which contains the state of each outstanding 
transmission is used circularly.  This has the effect of insuring that 
that the server never hangs in a @f[complete_operation] unless more 
than @f[OVERLAP] transmissions are outstanding at a time.  At the time 
of this writing, @f[OVERLAP] is defined as 10.  For each tuple, the 
server sends the YES/NO flag described above, followed by the data.   
Note that the data is sent as type TUPLE_DATA_T, rather than TUPLE_T, 
to eliminate redundant transmissions of the tuple descriptor. 
  
@appendix(Functions and Macros for Manipulating Structured Data)  
  
Here is a partial list of the GDB functions and macros which may be  
used to manipulate structured data.  This list is intended as a quick  
reference guide.  All user callable GDB routines are documented in the  
Library Reference Manual.  
  
@begin(description)  
@f[descriptor_from_tuple]@\returns the tuple descriptor of the supplied tuple.  
@index(descriptor_from_tuple)  
  
@f[descriptor_from_relation]@\returns the tuple descriptor of the supplied  
@index(descriptor_from_relation)  
relation.  
  
@f[create_tuple_descriptor]@\allocates a tuple descriptor.  
@index(create_tuple_descriptor)  
  
@f[delete_tuple_descriptor]@\destroys a tuple descriptor, deallocating the  
@index(delete_tuple_descriptor)  
memory which had been used to hold it.  
  
@f[create_tuple]@\allocates a tuple.  
@index(create_tuple)  
  
@f[null_tuple_strings]@\If a tuple contains fields of type STRING_T, then  
@index(null_tuple_strings)  
the memory for these is allocated separately from that used for other  
fields in the tuple.  This routine releases the memory used for all  
string fields in the tuple.  It should usually be called just before  
invoking @f[delete_tuple].  
  
@f[delete_tuple]@\destroys a tuple, deallocating the  
@index(delete_tuple)  
memory which had been used to hold it.    
  
@f[string_alloc]@\allocates space for a STRING, filling in the pointer and  
@index(string_alloc)  
length fields of the string descriptor.  
  
@f[string_free]@\returns the memory used for a STRING.  
@index(string_free)  
  
@f[db_alloc]@\Allocate memory using GDB's memory allocator.  In most   
@index(db_alloc)  
implementations this just calls Unix @f[malloc], but GDB's memory allocators  
@index(malloc)  
can be replaced by users running in unusual environments.  
  
@f[db_free]@\Return memory allocated by db_alloc.  
@index(db_free)  
  
@f[STRING_DATA]@\Given an argument of type STRING, returns a @f[(char *)]  
@index(STRING_DATA)  
pointer to the string data.  This is a macro which may appear as the  
target of an assignment statement;  you can construct your own STRING  
by setting the STRING_DATA and MAX_STRING_SIZE.  
  
@f[MAX_STRING_SIZE]@\Given an argument of type STRING, returns an integer equal 

@index(MAX_STRING_SIZE)  
to the size of the allocated string data.  If the STRING is used to   
hold a null terminated C character string, then @f[strlen] of that string  
@index(strlen)  
must be @f[<= MAX_STRING_SIZE].  
  
@f[ADD_TUPLE_TO_RELATION]@\a macro which inserts a new tuple after all the  
@index(ADD_TUPLE_TO_RELATION)  
others in a relation.  The tuple must share the same descriptor as the  
relation itself.  
  
@f[ADD_TUPLE_AFTER_TUPLE]@\inserts a tuple into the middle of a relation.  
@index(ADD_TUPLE_AFTER_TUPLE)  
  
@f[REMOVE_TUPLE_FROM_RELATION]@\the specified tuple is removed from the  
@index(REMOVE_TUPLE_FROM_RELATION)  
relation, but not de-allocated.  It may subsequently be inserted in  
another relation if desired.  Each tuple may be in at most one  
relation at a time.  
  
@f[FIRST_TUPLE_IN_RELATION]@\Finds the first tuple in a relation.  Returns  
@index(FIRST_TUPLE_IN_RELATION)  
NULL if the relation is empty.  

@f[NEXT_TUPLE_IN_RELATION]@\Given a relation and a tuple which is presumed
@index(NEXT_TUPLE_IN_RELATION)
to be in that relation, return the tuple which follows the one
supplied.  NULL is returned if the tuple supplied is the last one in
the relation.

@f[PREV_TUPLE_IN_RELATION]@\Given a relation and a tuple which is presumed
@index(PREV_TUPLE_IN_RELATION)
to be in that relation, return the tuple which preceeds the one
supplied.  NULL is returned if the tuple supplied is the first one in
the relation.

@f[DESCRIPTOR_FROM_TUPLE]@\Given a tuple, get back its tuple descriptor.
@index(DESCRIPTOR_FROM_TUPLE)

@f[DESCRIPTOR_FROM_RELATION]@\Given a relation, get back its tuple descriptor.
@index(DESCRIPTOR_FROM_RELATION)

@f[FIELD_FROM_TUPLE]@\Given a tuple and a field index, return a pointer to the 
@index(FIELD_FROM_TUPLE)
field.  The results of this macro must be cast properly by the caller.

@f[FIELD_TYPE_IN_TUPLE]@\Given a tuple descriptor (@i[not] a tuple) and
@index(FIELD_TYPE_IN_TUPLE)
the index (@i[not] the name) of a field, return the type code of the
field.  Symbolic names for type codes, such as @f[STRING_T] and
@Index(STRING_T)
@f(INTEGER_T) are defined in gdb.h;  they may be stored in
@Index(INTEGER_T)
variables of type @f[FIELD_TYPE].
@index(FIELD_TYPE)

@f[field_index]@\Given a tuple descriptor and the name of a field, return the
@index(field_index)
index of that field.

@end(description)

The following routines are often useful for debugging GDB
applications.  @b[Important note: each of these routines takes two
arguments, the first is an i.d. string, the second is the object to be printed!]

@begin(description)
@f[print_tuple]@\Prints a tuple on the gdb_log file (which defaults to
@index(print_tuple)
stderr.)  All fields are correctly labeled and formatted according to
their type.

@f[print_relation]@\Prints an entire relation, with all of its tuples, on gdb_log.
@index(print_relation)
@end(description)
@appendix(Sample Non-Forking Server)
@label(nonforkingserver)
@modify(example,Size 8)
@begin(example)
/************************************************************************/
/*	
/*		          tsr (test server)
/*			  -----------------
/*	
/*	Author: Noah Mendelsohn (IBM T.J. Watson Research and MIT Project
/*				 Athena)
/*	
/*	Copyright: 1986 MIT Project Athena
/*	
/************************************************************************/
/*	
/*	PURPOSE
/*	-------
/*	
/*	A GDB server program demonstrating techniques for asynchronously
/*	communicating with an arbitrary number of clients from a single
/*	Unix server process.  This server accepts GDB connections from
/*	clients as requests come in (up to the arbitrary maximum
/*	MAXCLIENTS.)  On each connection, it receives a stream of integers,
/*	which it interprets as ASCII characters.  The characters are
/*	converted to uppercase, and then sent back to the client from
/*	which they came.  
/*	
/*	All of this is done completely asynchronously.  No client is 
/*	locked out while characters are being echoed to another, and
/*	new connections are accepted at any time.
/*	
/*	NOTES
/*	-----
/*	
/*	1) The complete state of each client is kept in the array
/*	   named client.  The client[i].state variable indicates whether
/*	   client i is active, and if so, the client[i].action variable
/*	   indicates what kind of asynchronous activity the client is 
/*	   engaged in.  Note that these are local conventions, having
/*	   nothing to do with GDB or its interfaces.  
/*	
/*	2) Communication to each client is done over its connection,
/*	   named client[i].con.
/*	
/*	3) There is at most one asynchronous activity pending to 
/*	   each client at any given time, and its state is tracked
/*	   in the variable named client[i].pending_op.  The operation
/*	   may be a send, a receive, or an accept, depending on 
/*	   the contents of client[i].action.  These operations are
/*	   allocated when the server starts up, and then re-used
/*	   repeatedly.  They are the GDB analog of a lightweight process,
/*	   which is activated when queued on a connection.
/*	
/*	4) A special form of connection and a special listening operation
/*	   are used for asynchronously listening for new connection
/*	   requests.  These are 'listencon' and 'listenop' respectively.
/*	
/*	5) GDB includes a special form of select which waits for 
/*	   completion of operations as well as for activity on user
/*	   specified file descriptors.  The list of operations to be
/*	   monitored is stored in the variable
/*	   named op_list.  The call to op_select_any hangs until one
/*	   or more of these operations complete, then terminates.
/*	
/*	6) The main server loop acts on any new connection requests,
/*	   processes any newly completed activity on the active
/*	   clients, then drops into op_select_any to allow asynchronous
/*	   activities to progress.  
/*	
/*	
/************************************************************************/
@end(example)
@begin(example)
#include <stdio.h>
#include "gdb.h"

/************************************************************************/
/*	
/*			     DECLARATIONS
/*	
/************************************************************************/

#define MAXCLIENTS 10

	/*----------------------------------------------------------*/
	/*	
	/*	State of each possible client
	/*	
	/*----------------------------------------------------------*/

struct client {
	int state;				/* state of this client */
						/* descriptor */
#define CL_DEAD 1				/* client not started */
#define CL_STARTING 2				/* accepted, reply ongoing */
#define CL_UP 3					/* ready to go */
	int action;				/* what are we doing now */
#define CL_RECEIVE 4				/* waiting for a packet */
#define CL_SEND    5				/* sending a packet */
#define CL_ACCEPT  6				/* sending a reply */
	CONNECTION con;				/* connection to this */
						/* client, if any */
	OPERATION pending_op;			/* pending operation */
						/* on this connection, */
						/* if any */
	int data;				/* the character to echo */
						/* goes here, expressed as */
						/* an int  */
};

struct client client[MAXCLIENTS];

	/*----------------------------------------------------------*/
	/*	
	/*	Connections and operations for listening for
	/*	new clients.
	/*	
	/*----------------------------------------------------------*/

CONNECTION listencon;				/* listen on this */
						/* connection */
OPERATION  listenop;				/* this operation is used */
						/* repeatedly for listening */
						/* for new clients */

int	nextcl = 0;				/* index of the next client */
						/* we'll accept */

	/*----------------------------------------------------------*/
	/*	
	/*	Miscellaneous variables used in acquiring connections.
	/*	These are ignored in a simple server like this;  a 
	/*	more sophisticated server might want to validate the
	/*	names of its clients before accepting connections.
	/*	
	/*----------------------------------------------------------*/

TUPLE client_tuple;				/* client request goes */
						/* here */
char otherside[100];
int othersize;
@end(example)
@begin(example)
/************************************************************************/
/*	
/*				MAIN
/*	
/************************************************************************/

int
main(argc, argv)
int argc;
char *argv[];
{
	/*----------------------------------------------------------*/
	/*	
	/*		       LOCAL VARIABLES
	/*	
	/*----------------------------------------------------------*/

	register int i;				/* loop index */
	LIST_OF_OPERATIONS op_list;		/* for op_select_any */

	/*----------------------------------------------------------*/
	/*	
	/*	Check parameters
	/*	
	/*----------------------------------------------------------*/

	if (argc != 2) {
		fprintf(stderr,"Correct form is %s <servicename>\n",
			argv[0]);
		exit(4);
	}

	/*----------------------------------------------------------*/
	/*	
	/*			Initialize
	/*	
	/*----------------------------------------------------------*/

	gdb_init();				/* set up gdb */
	init_clients();				/* null the client states */
	do_listen(argv[1]);			/* start the listening */
						/* connection and queue */
						/* a listening operation */
	make_oplist(&op_list);			/* create wait list */

	/*----------------------------------------------------------*/
	/*	
	/*	     Loop forever taking care of business.
	/*	
	/*	1) If any new connection requests have come in,
	/*	   accept them.
	/*	
	/*	2) For each client on which some activity is newly
	/*	   completed, take care of it.
	/*	
	/*----------------------------------------------------------*/

	while (TRUE) {
		if (OP_DONE(listenop))
			new_connection();
		for (i=0; i<MAXCLIENTS; i++) {
			if (OP_DONE(client[i].pending_op))
				do_client(i);
		}
		op_select_any(op_list, 0, NULL, NULL, NULL, NULL);
	}
}
@end(example)
@begin(example)
/************************************************************************/
/*	
/*				do_client
/*	
/*	An operation has completed on the specified client.
/*	
/************************************************************************/

int
do_client(id)
int id;
{
	register struct client *cp = &(client[id]);

       /*
        * If there has been an error, shutdown the client.
        */
	if (OP_STATUS(cp->pending_op) == OP_CANCELLED) {
		sever_connection(cp->con);
		reset_operation(cp->pending_op);
		cp->state = CL_DEAD;
		cp->action = 0;
		return;
	}
       /*
        * The operation completed successfully.  Figure out what it was
        * and do the right thing.
        */
	switch (cp->action) {
	      case CL_ACCEPT:
	      case CL_SEND:
		start_receiving_object(cp->pending_op, cp->con, 
				       (char *)&cp->data,
				       INTEGER_T);
		cp->action = CL_RECEIVE;
		break;
	      case CL_RECEIVE:
		if (cp->data >= 'a' && cp->data <= 'z')
			cp->data += 'A'-'a';	/* upcase the response */
		start_sending_object(cp->pending_op, cp->con, 
				       (char *)&cp->data,
				       INTEGER_T);
		cp->action = CL_SEND;
	}
}
@newpage
/************************************************************************/
/*	
/*			  init_clients
/*	
/************************************************************************/

int
init_clients()
{
	register struct client *c;

	for (c=client; c<client+MAXCLIENTS; c++){
		c->state = CL_DEAD;
		c->action = 0;
		c->con = NULL;
		c->pending_op = create_operation();
		reset_operation(c->pending_op);		
	}
}


@end(example)
@begin(example)
/************************************************************************/
/*	
/*			make_oplist
/*	
/************************************************************************/

int
make_oplist(oplp)
LIST_OF_OPERATIONS *oplp;
{
       /*
        * ugh! we've got to fix create_list_of_operations to be
        * more flexible!!
        */

	*oplp = create_list_of_operations(MAXCLIENTS+1, listenop,
					  client[0].pending_op,
					  client[1].pending_op,
					  client[2].pending_op,
					  client[3].pending_op,
					  client[4].pending_op,
					  client[5].pending_op,
					  client[6].pending_op,
					  client[7].pending_op,
					  client[8].pending_op,
					  client[9].pending_op);
}
@end(example)
@begin(example)
/************************************************************************/
/*	
/*				do_listen
/*	
/*	Do the one time setup for listening for clients, and
/*	also start a listen for an actual client.
/*	
/************************************************************************/

int
do_listen(service)
char *service;
{

	/*----------------------------------------------------------*/
	/*	
	/*	Make a listening connection
	/*	
	/*----------------------------------------------------------*/

	fprintf(stderr, "Server creating listening connection\n");
	listencon = create_listening_connection(service);

	if (listencon == NULL) {
		fprintf(stderr,"tsr: could not create listening connection\n");
		exit (4);
	}

	/*----------------------------------------------------------*/
	/*	
	/*	On that connection, put up an operation to listen
	/*	for our first client.
	/*	
	/*----------------------------------------------------------*/

	listenop = create_operation();

	othersize = sizeof(otherside);

	start_accepting_client(listencon, listenop, &(client[nextcl].con),
			       (char *)otherside,
			       &othersize, &client_tuple);

}
@end(example)
@begin(example)
/************************************************************************/
/*	
/*			new_connection
/*	
/*	We have just gotten a connection for client nextcl.  
/*	
/************************************************************************/

int
new_connection()
{
	register struct client *cp = &client[nextcl];
       /*
        * Make sure there's been no error
        */
	if(OP_STATUS(listenop) != OP_COMPLETE ||
	   cp->con == NULL) {
		fprintf(stderr,"Error on listening operation\n");
		exit(8);
	}
       /*
        * Set up the new connection and reply to the client
        */
	cp->state = CL_STARTING;
	cp->action = CL_ACCEPT;
	start_replying_to_client(cp->pending_op, cp->con, GDB_ACCEPTED,
				 "", "");

       /*
        * Find a new free connection descriptor.  Blow up if we've used the
        * last one
        */
	for (nextcl=0; nextcl<MAXCLIENTS; nextcl++)
		if (client[nextcl].state == CL_DEAD)
			break;

	if (nextcl == MAXCLIENTS) {
		fprintf(stderr,"Too many clients, giving up\n");
		exit(8);
	}
       /*
        * Start listening again
        */
	reset_operation(listenop);
	othersize = sizeof(otherside);

	start_accepting_client(listencon, listenop, &(client[nextcl].con),
			       (char *)otherside,
			       &othersize, &client_tuple);


}
@end(example)

@begin(comment)

@Chapter(Hints and Tricks)
select variants

redefining GDB_GIVEUP

replacing storage allocators

print_relation, tuple, etc.

tuples as analogs for structures.

Using procedures to keep tuples up to date.

Creating your own types

Debug switch and the log

Output from dbserv

Reference counting of tuple descriptors.

@Chapter(Bugs)
Macros which require no ; after invocation

/etc/services entries for database server

Only one database server per site.
@Chapter(How GDB Works)

@end(comment)
