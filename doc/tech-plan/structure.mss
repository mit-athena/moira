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

Lists (mail, acl, groups)

Aliases

@End(Itemize)

@SubSection(Data Fields and Relationships)

The knowledge base of SMS enables system services and servers to be updated
with correct information.  The database has the reponsibility of storing
information which will be transmitted to the services.  The database will
not, however, be responsible for knowing the format of data to be sent.
This information will be inherent to the Data Control Manager.  Specific
fields of the database are organized to represent the needs of system.  
The current SMS database is comprised of the following tables:

@Begin(Description)

@UX(Table)@\@UX(Fields and Description)

@C(users)@\@Multiple[User Information.  There are two types of user required
information: information necessary to identify a user and enable a user to
obtain a service (e.g. to login), and personal information about the user
(finger).

@Begin(Description)
@I(login)@\a unique username, equivalent to the user's Kerberos principal name.

@I(users_id)@\an internal database indentifier for the user record.  This is
not the same as the Unix uid.

@I(last, first, middle)@\The user's full name, broken down for convenient
indexing. 

@I(status)@\contains flags for the user.  The only currently defined flag is
bit 0, which when set indicates that the user is active.  (An active user is
one who has been assigned system resources such as a mailbox, a home
directory, and who may be a member of one or more lists.)

@I(mit_id)@\the user's encrypted MIT id.

@I(shell)@\the user's default shell.

@I(expdate)@\the expiration date of the user and the user's resources (the user
becomes inactive.)

@I(modtime)@\the time that the user record was last modified (or created).
@End(Description)

There are no entries for password, uid, and primary gid because these are
being subsumed by other services (Kerberos, ACLS).  There is no entry for
home directory as it has little meaning in a workstation environment.  (See
below on User Filesys Mapping).]

@C(finger)@\@Multiple[This table contains the "finger" information for users.  

@Begin(Description)
@I(users_id)@\corresponds to the users_id in the users table.

@I(fullname)@\the user's full name.

@I(nickname)@\the user's nickname.

@I(home_address)@\home address.

@I(home_phone)@\home phone.

@I(mit_address)@\MIT address; this is for on-campus students' living addresses.

@I(mit_phone)@\MIT phone.

@I(office)@\office address.

@I(affiliation)@\one of undergraduate, graduate, staff, faculty, other.

@I(department)@\student's major or employee's department.

@I(year)@\student's year or "G".

@I(modtime)@\time finger record was last modified.

@End(Description)]

@C(machine)@\@Multiple[Machine Information.  

@Begin(Description)

@I(name)@\the canonical hostname.

@I(machine_id)@\an internal database id for this record.

@I(type)@\machine type: VS, RTPC, VAX

@I(model)@\machine model: VS2, VS2000, RTFLOOR, RTDESK, 750, 785.

@I(status)@\machine status: PUBLIC, PRIVATE, SERVER, TIMESHARE.

@I(serial)@\serial number.

@I(ethernet)@\ethernet address.

@I(sys_type)@\system type (for use by release engineering and operations).

@End(Description)]

@C(cluster)@\@Multiple[Cluster Infomation.  There are several named clusters
throughout Athena that correspond roughly to subnets and/or geographical
areas.

@Begin(Description)

@I(name)@\cluster name.

@I(description)@\cluster description.

@I(location)@\cluster location.

@I(cluster_id)@\internal database identifier for this record.

@I(acl_id)@\id for an access control list for cluster specific data.

@End(Description)]

@C(svc)@\@Multiple[For each cluster there is a set of services that serve the
machines in that cluster.  These services are described by an environment
variable (to be set on client workstations at login) and a @i(service
cluster) name.  Use of the service cluster name is service dependent but in
general refers to a group of entities that provide the named service in the
particular cluster.

@Begin(Description)

@I(cluster_id)@\references an entry in the cluster table.

@I(serv_label)@\label of a service cluster type (e.g. "prcluster", "usrlib",
"syslib")  

@I(serv_cluster)@\specific service cluster name (e.g. "e40-prcluster")

@End(Description)]

@C(services)@\@Multiple[TCP/UDP Port Information.  This is the information
currently in /etc/services.  In a workstation environment with SMS and the
Hesiod name server, service information will be obtained from the name
server.

@Begin(Description)

@I(service)@\service name.

@I(protocol)@\protocol: one of TCP, UDP.

@I(port)@\port number.

@End(Description)]

@C(filesys)@\@Multiple[File System Information.  This section desribes the file
system information necessary for a workstation to attach a file system.

@Begin(Description)

@I(label)@\a unique name for an attachable file system.

@I(type)@\currently one of RVD, NFS, or RFS.

@I(machine_id)@\file server machine.

@I(name)@\name of file system on the server.

@I(mount)@\default mount point for file system.

@I(access)@\default access mode for file system.

@I(acl_id)@\access control list for file system.

@End(Description)]

@C(ufs)@\@Multiple[User File System Mapping.  Each user has a default set of
file systems that are attached at login time.  This table provides the
mapping for these file sytems.

@Begin(Description)

@I(users_id)@\id of @C(users) entry.

@I(filesys_label)@\file system label.

@End(Description)]

@C(rvdsrv)@\@Multiple[RVD Server Information.  This table contains the top
level access control lists for each rvd server.  Any other per-server
information should be added here.

@Begin(Description)

@I(machine_id)@\server machine.

@I(operations_acl)@\operations access control list.

@I(admin_acl)@\administrative access control list.

@I(shutdown_acl)@\shutdown access control list.

@End(Description)]

@C(rvdphys)@\@Multiple[Physical device partition table.

@Begin(Description)

@I(machine_id)@\server machine.

@I(device)@\rvd physical device.

@I(size)@\size in 512-byte blocks.

@I(created)@\creation time.

@I(modified)@\modification time.

@End(Description)]

@C(rvdvirt)@\@Multiple[Virtual device table.  This table contains the list of
virtual devices for each rvd server machine.  Information per device
includes the rvd physical device it resides on, its name, unique pack id,
owner (@i<users> id), access control lists for the device (@i<rocap, excap,
shcap>), allowable access modes, the offset within the physical device, its
size, a @i(machine) id for a host that has default access to the device, and
the creation and modifications dates for the device.

@Begin(Description)

@I(machine_id)@\server machine.

@I(device)@\rvd physical device.

@I(name)@\virtual device name.

@I(packid)@\unique pack id.

@I(owner)@\owner (currently an arbitrary string, should be a @C(users) entry
id.) 

@I(rocap, excap, shcap)@\currently three passwords providing read-only,
exclusive, and shared accesss to the file system.  These should be condensed
into one access control list id.  The access control list would indicate
read and write access (shared access has never been implemented).

@I(modes)@\allowable access modes for device.

@I(offset)@\offset of virtual device into physical device (in blocks).

@I(blocks)@\size of virtual device.

@I(ownhost)@\name of host from which device may be mounted without a
password (not used with acl's?).

@I(created)@\creation time.

@I(modified)@\modification time.

@End(Description)]

@C(nfsphys)@\@Multiple[NFS Server Information.  This table contains for each
nfs server machine a list of the physical device partitions from which
directories may be exported.  For each such partition an access control list
is provided.

@Begin(Description)

@I(machine_id)@\server machine.

@I(device)@\file system name.

@I(acl_id)@\access control list.

@End(Description)]

@C(printer)@\@Multiple[Printer Information.

@Begin(Description)

@I(name)@\a unique printer name.

@I(printer_id)@\internal database identifier for this record.

@I(type)@\printer hardware type: one of LPS40, 3812, LN03, LN01, etc.

@I(machine_id)@\print server machine for this printer.

@I(acl_id)@\access control list specifying who may modify the attributes of
this printer in the database.

@End(Description)]

@C(prability)@\@Multiple[Printer abilities.

@Begin(Description)

@I(printer_id)@\id for a printer.

@I(ability)@\printer ability: one of TEXT, POSTSCRIPT, WINDOWDUMP, etc.

@End(Description)]

@C(queue)@\@Multiple[Printer queues.  This table contains a list of unique
queue names and the attributes of each queue.  Attributes of queue are its
@i(ability), a status string (used by MDQS servers), and an access control
list.

@Begin(Description)

@I(name)@\unique queue name.

@I(queue_id)@\internal database id for this queue.

@I(machine_id)@\server machine.

@I(ability)@\printer ability associated with this queue.

@I(status)@\queue status string (used by MDQS).

@I(acl_id)@\access control list specifying who may modify the attributes of
this queue in the database.

@End(Description)]

@C(pqm)@\@Multiple[Printer to queue mapping.  This table provides the mapping
between printers and queues.

@Begin(Description)

@I(printer_id)@\printer id.

@I(queue_id)@\queue id.

@End(Description)]

@C(qdev)@\@Multiple[MDQS device information.  Each MDQS server has a device
table that assigns a logical name and status information to each known
physical printer device.

@Begin(Description)

@I(machine_id)@\MDQS server machine.

@I(qdev_id)@\internal database id for this device.

@I(name)@\logical name of device.

@I(device)@\physcial device name (e.g.: /dev/lp0)

@I(status)@\status string kept by MDQS.

@I(acl_id)@\access control list for device.

@End(Description)]

@C(qdm)@\@Multiple[MDQS queue to device mapping.  This table ties together
the queue and device information for each MDQS server.  Printer names are
not actually known to MDQS; printer to queue mapping is handled by Hesiod
using the @C(pqm) table information described above.

@Begin(Description)

@I(machine_id)@\MDQS server machine.

@I(queue_id)@\queue id.

@I(device_id)@\device id.

@I(server)@\name of a server program to invoke for jobs using this queue and
device. 

@End(Description)]

@C(printcap)@\@Multiple[Berkeley line printer software printcap entries.

@Begin(Description)

@I(name)@\printer name.

@I(string)@\printcap string.

@End(Description)]

@C(pobox)@\@Multiple[Post Office Information.  This list matches users with
one or more post office boxes.  A post office box is identified by its type
(POP, LOCAL), the machine on which the box resides, and the box name on that
machine.

@Begin(Description)

@I(users_id)@\id for a @C(users) entry.

@I(type)@\mailbox type: one of POP, LOCAL.

@I(machine_id)@\post office server machine.

@I(box)@\mailbox name on server.

@End(Description)]

@C(list)@\@Multiple[Lists are used to associate a group of names with a
particular label.  The definition of this table should be generic enough to
support many types of lists.  List members are specified in the @c(members)
table.  Associated with each members is a list of flags indicating access
control and list administrator permissions.  There must be at least one list
administrator for a list; there can be many.  In fact a list may be
administered by all of the members of another list.  

@Begin(Description)

@I(name)@\list name.

@I(type)@\list type: currently one of MAIL, ACL, GROUP

@I(list_id)@\internal database id for this list.

@I(flags)@\currently one or more of ACTIVE, PUBLIC, HIDDEN.  (These flags
are used for mailing lists.)

@I(description)@\description of list.

@I(expdate)@\expiration date of list.

@I(modtime)@\time list was last modified (@c(list) entry or @C(members)
entry). 

@End(Description)]

@C(members)@\@Multiple[List members.  Members are specified by a member type
and a member id pair.  Each member entry also has a status field which
contains access control information.

@Begin(Description)

@I(list_id)@\id of a list.

@I(member_type)@\member type: one of USER, LIST, STRING.

@I(member_id)@\id of a member (a @c[users] id, @c[list] id, or @c[string]
id.)

@I(member_status)@\@Multiple[access control and list administrator
permissions.  Current values are:

@Begin(Format, Above .8, LeftMargin +.375inch, Size -1)
@TabSet(+1.2inch)
@U(Bit)@\@U(Function)
0@\Read
1@\Write
2@\Delete
3@\Append
4@\Administrator
5@\Master Administrator
6@\Super Administrator (Owner(s))
@End(Format)

The first three bits specify standard read/write/delete access.  The first
four bits specify database access capabilities (Retrieve, Replace, Delete,
and Append) for table access control lists.  In the case of MAIL and GROUP
lists these permission bits may have no meaning; however, since a list of
one type may contain as a member a list of another type, these bits can be
used to indicate allowable access when referenced through an access control
list.  The last three bits are list administration capabilities.

NOTE: Need to determine how to mask the status field of members of type LIST
with the status fields of the corresponding list members.]

@End(Description)]

@C(strings)@\@Multiple[Used for list members of @i(string) type.  An
optimization for dealing with (usually long) foreign mail addresses.

@Begin(Description)

@I(member_id)@\member id.

@I(string)@\string.

@End(Description)]

@C(alias)@\@Multiple[Aliases are used by several different services to
provide alternative names for objects or a mapping one type of object and
another.  Some examples of alias usage are printer aliases, service aliases,
cluster aliases, file system aliases, print cluster to printer maps, and the
machine to cluster map.

@Begin(Description)

@I(name)@\alias name.

@I(type)@\alias type: currently one of PRINTER, SERVICE, CLUSTER, FILESYS,
PRCLUSTER, MACH-CLU-MAP.

@I(trans)@\alias translation.

@I(trans_type)@\translation object type: alias type dependent (usually
STRING) (possibly unnecessary)

@End(Description)]

@C(tbinfo)@\@Multiple[Table Info.  This table contains table specific
information such as access control lists for each table, and current maximum
values for unique ids.  Table names will in general correspond directly to
the list of tables described in this section.  However logical table names
may be used that correspond to a part of larger table.  For example, there
is a separate entry for each alias type ("alias-printer", "alias-service",
etc.) 

@Begin(Description)

@I(table)@\table name.

@I(value)@\currently is used to specify the current maximum unique id for
tables with unique id fields.

@I(acl_id)@\access control list for specified table.

@End(Description)]

@End(Description)

@SubSection(Predefined Database Queries)

All access to the database is provided through the application
library/database server interface.  This interface provides a limited set of
predefined, named queries, which allows for tightly controlled access to
database information.  Queries fall into four classes: retrieve, update,
delete, and append.  An attempt has been made to define a set of queries
that provide sufficient flexibility to meet all of the needs of the Data
Control Manager and each of the indivual application programs.  However,
since the database can be modified and extended in the future, the server
and application library have been designed to allow for the easy addtion of
queries.






