@part[sys_model, root "sms.mss"]
@MajorSection(System Model)

The model is derived from requirements listed in the previous section.  As
previously mentioned, SMS is composed of five components.

@Begin(Itemize, Spread 1)

The database.

The SMS server.

The application library.

The SMS protocol.

The Data Control Manager, DCM.

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


@Section(The Database)

The database is the core of SMS.  It provides the storage mechanism for
the complete system.  This section focuses on the fields of the
database, their expandability, and their limitations.

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

