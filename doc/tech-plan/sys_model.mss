@part[sysmodel, root "sms.mss"]
@newpage()
@MajorSection(System Model)

The model is derived from requirements listed in the previous section.  As
previously mentioned, SMS is composed of six components.

@Begin(Itemize, Spread 1)

The database.

The SMS server.

The application library.

The SMS protocol.

The Data Control Manager, DCM.

The server-specific files.
@End(Itemize)

Because SMS has a variety of interfaces, a distinction must be
maintained between applications which directly read and write to SMS
(i.e., administrative programs) and services which use information
distributed from SMS (i.e. name server).  In both cases the interface to
SMS database is through the SMS server, using the SMS protocol.  The
significant difference is that server update is handled automatically
through a data control manager; administrative programs are executed by
users.

@BlankSpace(5 inches)

@Begin(Center)
THE SMS SYSTEM STRUCTURE
@End(Center)

In all cases, a client of SMS uses the application library. The library
communicates with the SMS server via a network protocol.  The server
will process database specific requests to retrieve or update
information.

@section(System Assumptions)

The support and function of sms is derived exclusively in response to
the environment which it supports.  This section presents factors
of the design dealing with considerations such as scalability, size,
deployment, and support.  
@begin(enumerate)
The system is designed optimally for 10,000 active users.  The database
has been designed to delineate between active and non-active status.
Active refers to those individuals who have permission to use the system.

SMS supports a number of system services.  To date there are six system
services which are supported.  They are:
@begin(itemize, spread 0)
Hesiod

RVD

NFS

Mail Service

MDQS - not fully supported 

Zephyr - to date, not supported
@end(itemize)
These services are, however, each supported by a collection of server-specific
data files.  To date, there are over 20 separate files used to support the 
above services.

Each service is supported by a collection of database fields.  Over 100
query handles provide efficient, database independent methods of accessing
data.  All applications use this method.

The system is designed to allow futher expansion of the current database,
with the ultimate capability of sms supporting multiple databases through
the same query mechanism.  Provision for many more services is recognized
through this design.

The distribution of server-specific files can occur every 15 minutes, with 
an optimal time interval being greater than 6 hours.  The data control manager is
designed to only generate and propagate new files if the database has changed
within the previous time interval.

The system supports three hesiod servers, 10 library servers running RVD, 20
locker servers running NFS, five class libraries running NFS, one 
/usr/lib/aliases propagation.  A hesiod server requires 9 separate files;
Each hesiod server will receive the same 9 files. Each RVD server requires one 
file, each file being different.  Each NFS server requires two files,
one identical file to all NFS servers, one different file to each filesystem
on each server.  Usr/lib/aliases is one file.

@StandardTable( Name FileOrg, Columns 6, TopHeading FlushLeft, 
		ColumnHeadings FlushLeft, FlushTop)
@Begin(FileOrg)
@TableId(FileOrg)

@TableHeading( Immediate, RowFormat FileOrgTopHeading, 
	       Line 'File Organization')

@TableHeading( Immediate, RowFormat FileOrgColumnHeadings,
	       Line 'Service@\File@\Size@\Number@\Propagations@\Interval')

Hesiod@\cluster.db@\22300@\1@\3@Hsp(.625inch)@\6 hours

@\service.db@\10100@\1@\3@Hsp(.625inch)@\6 hours

@\printers.db@\3833@\1@\3@\6 hours

@\lpr.db@\3250@\1@\3@\6 hours

@\printcap.db@\9800@\1@\3@\6 hours

@\pobox.db@\325000@\1@\3@\6 hours

@\sloc.db@\300@\1@\3@\6 hours

@\filesys.db@\36000@\1@\3@\6 hours

@\passwd.db@\880000@\1@\3@\6 hours

@BlankSpace(1 line)@\@\@\@\@\

RVD@\/site/rvddb@\2000(90%) - 
20000@\10@\10@\15 minutes

@\/site/rvd/acl/OP.acl@\small@\10@\10@\15 min

@\/site/rvd/acl/AD.acl@\small@\10@\10@\15 min

@\/site/rvd/acl/SH.acl@\small@\10@\10@\15 min

@\/site/rvd/acl/file.acl@\small@\10@\10@\15 min

@BlankSpace(1 line)@\@\@\@\@\

NFS@\/site/nfsid@\880000@\1@\25@\6 hours

@\/mit/quota@\35000@\25@\25@\6 hours

@BlankSpace(1 line)@\@\@\@\@\ 

Mail@\/usr/lib/aliases@\445000@\1@\1@\24 hours

@BlankSpace(1 line)@\@\@\@\@\

TOTAL@\@\@\86@\110@\

@End(FileOrg)

@Begin(Description)
NOTE:@\The above files will only be generated and propagated if the
data has changed during the time interval.  For example,  although
the RVD interval is 15 minutes, there is no effect on
system resources unless the information relevant to RVD's changed
during the previous 15 minute interval.
@End(Description)

Application interfaces provide all the mechanisms to change database
fields.  There will be no need for any sms updating to be done
by directly manipulating the database.  For each service, there is
at least one application interface.  Currently there are twelve
interface programs.

@end(enumerate)

@Section(The Database)

The database is the core of SMS.  It provides the storage mechanism for
the complete system.

The database is written using RTI INGRES, a commercially
available database toolkit.  Its advantage is that it is available and
it works.  INGRES provides the Athena plant with a complete query
system, a C library of routines, and a command interface language.
SMS does not depend on any special feature of INGRES.  In fact,
SMS can easily utilize other relational databases.

A complete description of the INGRES design can be found in RTI's INGRES
users' manuals; this paper does not discuss the structure of INGRES.
This documentation does, however, describe, in detail the structure of
the SMS database.

The database contains the following:

@Begin(Itemize, Spread 1)

User information

Machine information

General service information (service location, /etc/services)

File system information (RVD, NFS)

Printer information (Berkeley, MDQS)

Post office information

Lists (mail, acl, groups)

Aliases

@End(Itemize)

The database field are described in the section @i[Database Fields],
described later in this document.  

The database is a completely independent entity from the sms
system.  The Ingres query bindings and database specific routines
are layered at the lower levels of the sms server.  @i[All applications
are independent of the database specific routines.]  This independence
is acheived through the use of query handles, sms specifice functions
providing data access and updating.  An application passes query handles to the
sms server which then resolves the request.  This request is passed to
the database via a database specific call.  Allowing for additional
data and future expansions, sms can use other databases for information.
This mechanism, although not functional at this time, is achieved by having
a set of query handles for each accessable database.  Then, the application
merely passes a query handle to a function, which then resolves the database and 
query. 

The current database supports all activities inherent to operation and
data requirements of the previously listed sms-supported services. 
No attempt is made to circumvent sms as the central point of contact.
When needed and where applicable, as more services are required, new 
fields and query handles will be provided for support.



































