@part[sysmodel, root "moira.mss"]

@Comment{Table Environment Definitions}
@Define(ColH=FlushLeft, Underline All)
@Define(Col1=FlushLeft, MinWidth 1inch)
@Define(Col2=FlushLeft)
@Define(Col3=FlushLeft)
@Define(Col4=FlushLeft, MinWidth .625inch)
@Define(Col5=FlushLeft, MinWidth .975inch)
@Define(Col6=FlushLeft, MinWidth .625inch)
@DefineRowFormat( ColHeadFormat,
		  Columns = [Column ColH, Column ColH, Column ColH,
			     Column ColH, Column ColH, Column ColH])
@DefineRowFormat( BodyFormat, FlushTop,
		  Columns = [Column Col1, Column Col2, Column Col3,
			     Column Col4, Column Col5, Column Col6])


@MajorSection(System Model)

The model is derived from requirements listed in the previous section.  As
previously mentioned, Moira is composed of six components.

@Begin(Itemize, Spread .5)

The database.

The Moira server.

The application library.

The Moira protocol.

The Data Control Manager, DCM.

The server-specific files.
@End(Itemize)

Because Moira has a variety of interfaces, a distinction must be
maintained between applications which directly read and write to Moira
(i.e., administrative programs) and services which use information
distributed from Moira (i.e. name server).  In both cases the interface to
Moira database is through the Moira server, using the Moira protocol.  The
significant difference is that server update is handled automatically
through a data control manager; administrative programs are executed by
users.

@Figure{
@picture(height = 2.25 inches, ScaleableLaser = components.ps)
@Caption(The Moira System Structure)
}

In all cases, a client of Moira uses the application library. The library
communicates with the Moira server via a network protocol.  The server
will process database specific requests to retrieve or update
information.

@section(System Assumptions)

The support and function of Moira is derived exclusively in response to
the environment which it supports.  This section presents factors
of the design dealing with considerations such as scalability, size,
deployment, and support.  

@begin(enumerate, RightMargin 0)

The system is designed optimally for 10,000 active users.  The database
has been designed to delineate between active and non-active status.
Active refers to those individuals who have permission to use the system.

Moira supports a number of system services.  To date there are four system
services which are supported.  They are:
@begin(itemize, spread 0)
Hesiod

NFS

Mail Service

Zephyr - to date, supported but not in use
@end(itemize)
These services are, however, each supported by a collection of server-specific
data files.  To date, there are over 20 separate files used to support the 
above services.

Each service is supported by a collection of database fields.  Over 100
query handles provide efficient, database independent methods of accessing
data.  All applications use this method.

The system is designed to allow futher expansion of the current database,
with the ultimate capability of Moira supporting multiple databases through
the same query mechanism.  Provision for many more services is recognized
through this design.

The distribution of server-specific files can occur every 15 minutes, with 
an optimal time interval being greater than 6 hours.  The data control manager is
designed to only generate and propagate new files if the database has changed
within the previous time interval.

The system supports one hesiod server, 20 locker servers running NFS,
one /usr/lib/aliases propagation.  A hesiod server requires 11 separate
files; each hesiod server will receive the same 11 files.  Each NFS
server requires three files, one of these files will be the same for
most NFS servers.  /usr/lib/aliases is one file.

File Organization:
@Begin(Multiple)
@Begin(Text, TableColumns BodyFormat, LeftMargin 3ems, Spacing 1, Above 1,
       Spread .2lines, BoxLM .125inch)
@TableId(FileOrg)

@TableHeading( Immediate, Continued, RowFormat ColHeadFormat,
	       Line 'Service@\File@\Size@\Number@\Propagations@\Interval')

Hesiod@\cluster.db@\53656@\1@\1@\6 hours

@\filesys.db@\541482@\1@\1@\6 hours

@\gid.db@\341012@\1@\1@\6 hours

@\group.db@\453636@\1@\1@\6 hours

@\grplist.db@\357662@\1@\1@\6 hours

@\passwd.db@\712446@\1@\1@\6 hours

@\pobox.db@\415688@\1@\1@\6 hours

@\printcap.db@\4318@\1@\1@\6 hours

@\service.db@\9052@\1@\1@\6 hours

@\sloc.db@\3734@\1@\1@\6 hours

@\uid.db@\256381@\1@\1@\6 hours

@BlankSpace(1 line)@\@\@\@\@\

NFS@\@i(partition).dirs@\2784@\20@\20@\12 hours

@\@i(partition).quotas@\1205@\20@\20@\12 hours

@\credentials@\152648@\1@\20@\12 hours

@BlankSpace(1 line)@\@\@\@\@\ 

Mail@\/usr/lib/aliases@\445000@\1@\1@\24 hours

@BlankSpace(1 line)@\@\@\@\@\

Zephyr@\@i(class).acl@\100@\6@\18@\24 hours

TOTAL@\@\@\59@\90@\

@End(Text)
@Begin(Description, LeftMargin +8, Indent -8)
NOTE:@\The above files will only be generated and propagated if the
data has changed during the time interval.  For example,  although
the hesiod interval is 6 hours, there is no effect on
system resources unless the information relevant to hesiod has changed
during the previous 6 hour interval.
@End(Description)
@End(Multiple)

Application interfaces provide all the mechanisms to change database
fields.  There will be no need for any Moira updating to be done
by directly manipulating the database.  For each service, there is
at least one application interface.  Currently there are twelve
interface programs.

@end(enumerate)

@Section(The Database)

The database is the core of Moira.  It provides the storage mechanism for
the complete system.

The database is written using RTI INGRES, a commercially
available database toolkit.  Its advantage is that it is available and
it works.  INGRES provides the Athena plant with a complete query
system, a C library of routines, and a command interface language.
Moira does not depend on any special feature of INGRES.  In fact,
Moira can easily utilize other relational databases.

A complete description of the INGRES design can be found in RTI's INGRES
users' manuals; this paper does not discuss the structure of INGRES.
This documentation does, however, describe, in detail the structure of
the Moira database.

The database contains the following:

@Begin(Itemize, Spread 0)

User information (account, finger)

Machine information

Cluster information

General service information (service location, /etc/services)

File system information (NFS partitions, logical filesystems)

Printcap information

Post office information

Lists (mail, acl, groups)

Aliases

@End(Itemize)

The database fields are described in section @ref[database_fields]
starting on page @pageref(database_fields) of this document.  

The database is a completely independent entity from the Moira
system.  The Ingres query bindings and database specific routines
are layered at the lower levels of the Moira server.  @i[All applications
are independent of the database specific routines.]  This independence
is acheived through the use of query handles, Moira specifice functions
providing data access and updating.  An application passes query handles to the
Moira server which then resolves the request.  This request is passed to
the database via a database specific call.  Allowing for additional
data and future expansions, Moira can use other databases for information.
This mechanism, although not functional at this time, is achieved by having
a set of query handles for each accessable database.  Then, the application
merely passes a query handle to a function, which then resolves the
database and query. 

The current database supports all activities inherent to operation and
data requirements of the previously listed Moira-supported services. 
No attempt is made to circumvent Moira as the central point of contact.
When needed and where applicable, as more services are required, new 
fields and query handles will be provided for support.
