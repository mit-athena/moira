@part[structure, root "sms.mss"]
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

For now, the database is written using RTI INGRES, a commercially
available database toolkit.  Its advantage is that it is available and
it works.  INGRES provides the Athena plant with a complete query
system, a C library of routines, and a command interface language.

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

Lists (mail, ACL, groups)

Aliases

@End(Itemize)

@SubSection(Data Fields and Relationships)

The knowledge base of SMS enables system services and servers to be updated
with correct information.  The database has the reponsibility of storing
information which will be transmitted to the services.  The database will
not, however, be responsible for knowing the format of data to be sent.
This information will be inherent
to the Data Control Manager.  Specific fields of the database are
organized to represent the needs of system.  We define the following
tuples:

@Begin(Description)

@UX(Table)@\@UX(Fields and Description)

users@\@Multiple[login, unique id, shell, last name, first name, middle
name, status, mit_id, expiration date, modification date.

User Information.  There are two types of user required information:
information necessary to identify a user and enable a user to obtain a
service (e.g. to login), and personal information about the user (finger).

In this table @i(login) is the user's unique username; the @i(unique id) is
an internal database identifier for the user (this is not the same as the
current Unix uid).  The @i(status) field is used to indicate whether the
user is active or privileged.  @i(Mit_id) is the user's encrypted MIT id.
There are no entries for password, uid, and primary gid because these are
being subsumed by other services (Kerberos, ACLS).  There is no entry for
home directory as it has little meaning in a workstation environment.  (See
below on User Filesys Mapping).]

finger@\@Multiple[@i(users) id, fullname, nickname, home address, home
phone, MIT address, MIT phone, MIT affiliation, department, office, year,
modification date.

This table contains the "finger" information for users.  The @i(users id)
field corresponds to a unique id in the @i(users) table.  Expiration dates
for @i(finger) entries are taken from the corresponding @i(users) entries.]

machine@\@Multiple[name, unique id, type, model, status, serial, sys_type.

Machine Information.  @i(Name) is the canonical hostname.  The @i(unique id)
is used for internal database references to machines.  @i(Type) is one
of VS2, RPTC, 750.  @i(Model) is one of VS2, VS2000, RTFLOOR, RTDESK, etc.
@i(status) is one of PUBLIC, PRIVATE, SERVER, TIMESHARE.  @i(Sys_type) is
for use by release engineering.]

cluster@\@Multiple[name, location, @i(acl) id, unique id.

Cluster Infomation.  There are several named clusters throughout Athena that
correspond roughly to subnets and/or geographical areas.  Entries in this
table include the cluster name, its location, an access control list
indicating who can modify information regarding this cluster, and an
internal database id.]

svc@\@Multiple[@i(cluster) id, environment variable, service cluster.

For each cluster there is a set of services that serve the machines
in that cluster.  These services are described by an environment variable
(to be set on client workstations at login) and a @i(service cluster) name.
Use of the service cluster name is service dependent but in general refers
to a group of entities that provide the named service in the particular
cluster.]

services@\@Multiple[name, protocol, port.

TCP/UDP Port Information.  This is the information currently in
/etc/services.]

filesys@\@Multiple[label, type, @i(machine) id, name, mount, access, @i(acl)
id.

File System Information.  This section desribes the file system information
necessary for a workstation to attach a file system.  Entries consist of a
@i(label) which is a unique name for an attachable file system; @i(type)
which is currently one of RVD, NFS, or RFS; a reference to the file server
machine, the name of the file system on the server machine, the default
mount point on a workstation, the default access for the file system, and an
access control list for the file system.

A given label may map to one or more identical file systems.  An example of
multiple mapping is the "usrlib" service cluster which maps to several
identical file systems that can be attached by a workstation.]

ufs@\@Multiple[@i(users) id, @i(filesys) label.

User File System Mapping.  Each user has a default set of file systems that
are attached at login time.  This table provides the mapping for these file
sytems. ]

rvdsrv@\@Multiple[@i(machine) id, operations pwd, admin pwd, shutdown pwd.

RVD Server Information.  This table contains the top level passwords for
each rvd server.  Each password field can be interpreted either as a
password or as the name of an access control list.  Any other per-server
information should be added here.]

rvdphys@\@Multiple[@i(machine) id, device, blocks.

Physical device partition table.  This table contains for each rvd server
machine a list of the physical devices and their sizes.]

rvdvirt@\@Multiple[@i(machine) id, physical device, name, pack id, owner,
rocap, excap, shcap, modes, offset, blocks, ownhost.

Virtual device table.  This table contains the list of virtual devices for
each rvd server machine.  Information per device includes the rvd physical
device it resides on, its name, unique pack id, owner (@i<users> id), access
control lists for the device (@i<rocap, excap, shcap>), allowable access
modes, the offset within the physical device, its size, and a @i(machine) id
for a host that has default access to the device.]

nfsphys@\@Multiple[@i(machine) id, device, @i(acl) id.

NFS Server Information.  This table contains for each nfs server machine a
list of the physical device partitions from which directories may be
exported.  For each such partition an access control list is provided.]

printer@\@Multiple[name, unique id, type, @i(machine) id, @i(acl) id.

Printer Information.  All printers have a unique name.  The characteristics
of a particular printer are the server machine it is connected to, its
hardward type (lps40, 3812), and its printing abilities (text, postscript,
windowdumps).  Each printer also has an access control list which specifies
who can modify the attributes of the printer.]

prability@\@Multiple[@i(printer) id, ability.

Printer abilities.  This table contains a list of @i(printer) ids and
ability names.]

queue@\@Multiple[@i(machine) id, name, @i(queue) id, ability, status,
@i(acl) id.

Printer queues.  This table contains a list of unique queue names and the
attributes of each queue.  Attributes of queue are its @i(ability), a status
string (used by MDQS servers), and an access control list.]

pqm@\@Multiple[@i(printer) id, @i(queue) id.

Printer to queue mapping.  This table provides the mapping between printers
and queues.]

qdev@\@Multiple[@i(machine) id, @i(qdev) id, name, device, status, @i(acl)
id.

MDQS device information.  Each MDQS server has a device table that assigns a
logical name and status information to each known physical printer device.]

printcap@\@Multiple[name, printcap string.

Berkeley line printer software printcap entries.]

pobox@\@Multiple[@i(users) id, type, @i(machine) id, box.

Post Office Information.  This list matches users with one or more
post office boxes.  A post office box is identified by its type (POP,
LOCAL), the machine on which the box resides, and the box name on that
machine.]

list@\@Multiple[name, unique id, type, flags, description, owner, @i(acl)
id, expiration date, modification date.

Lists are used to associate a group of names with a particular
label.  The definition of this table should be generic enough to support
many types of lists.  Entries in this table contain a list name, a unique
id, a list type (string), flags (ACTIVE, PUBLIC, HIDDEN), a description
string, an owner @i(users) id, an ACL @i(list) id, an expiration date, and a
modification date.  Currently known list types are "mail", "acl", "group",
and "project".

Lists must have either an owner, an ACL, or both.  An access control list
specifies users who are allowed to administer the list.  List administrators
fall into three categories: simple administrators who are allowed to add
members to and delete members from a list; master-administrators who are
allowed to add and delete administrators as well as members of the list; and
super-administrators who in addtion to the above are allowed to designate
master and super administrators.  An owner is equivalent to a
super-administrator and is the person primarily responsible for a list.]

members@\@Multiple[@i(list) id, member id, member type.

List members.  List members can be of three types: a user as found in the
@i(users) table, another list, or an arbitrary string.  For efficiency
reasons, this table uses a @i(member id) field which references either a
@i(users) id, a @i(list) id, or a @i(strings) id depending on the @i(member
type).]

strings@\@Multiple[@i(members) id, string.

Used for list members of @i(string) type.  An optimization for dealing
with (usually long) foreign mail addresses.]

alias@\@Multiple[name, type, translation, translation type.

Aliases are used by several different services to provide alternative names
for objects.  Aliases could be implemented as lists with single members, but
their usage is wide-spread enough to justify a separate table with less
overhead.  Some examples of alias usage are printer aliases, printer cluster
maps, file system aliases, service aliases, service locations, and
machine-cluster mappings.

An alias entry consists of its name, the alias type (string), the
translation, and the translation type (string, alias type-specific).]

@End(Description)

@SubSection(Predefined Database Queries)

@Verbatim([To be supplied.])
