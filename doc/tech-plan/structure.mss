@part[structure, root "sms.mss"]
@newpage()
@MajorSection(Data Fields and Relationships)

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

@I(uid)@\Unix uid.  Temporarily necessary due to NFS client code problems.
Ultimately, this field will be removed and uids will be assigned arbitrarily
for each client-server connection.

@I(last, first, middle)@\The user's full name, broken down for convenient
indexing. 

@I(shell)@\the user's default shell.

@I(home)@\name of the users home file system.

@I(status)@\contains flags for the user.  The only currently defined flag is
bit 0, which when set indicates that the user is active.  (An active user is
one who has been assigned system resources such as a mailbox, a home
directory, and who may be a member of one or more lists.)

@I(mit_id)@\the user's encrypted MIT id.

@I(mit_year)@\a student's academic year, not modifiable by the student.
Used for Athean administrative purposes.

@I(expdate)@\the expiration date of the user and the user's resources (the user
becomes inactive.)

@I(modtime)@\the time that the user record was last modified (or created).

@End(Description)

See Section @Ref(Users) for the list of queries associated with this table.

There are no entries for password and primary gid because these are
being subsumed by other services (Kerberos, ACLS).]

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

See Section @Ref(Finger) for the list of queries associated with this table.

@End(Description)]

@C(machine)@\@Multiple[Machine Information.  

@Begin(Description)

@I(name)@\the canonical hostname.

@I(machine_id)@\an internal database id for this record.

@I(type)@\machine type: VS, RTPC, VAX

@I(model)@\machine model: VS2, VS2000, RTFLOOR, RTDESK, 750, 785.

@I(status)@\machine status: PUBLIC, PRIVATE, SERVER, TIMESHARE.

@I(serial)@\serial number.

@I(sys_type)@\system type (for use by release engineering and operations).

@End(Description)

See Section @Ref(Machine) for the list of queries associated with this table.
]

@C(cluster)@\@Multiple[Cluster Infomation.  There are several named clusters
throughout Athena that correspond roughly to subnets and/or geographical
areas.

@Begin(Description)

@I(name)@\cluster name.

@I(description)@\cluster description.

@I(location)@\cluster location.

@I(cluster_id)@\internal database identifier for this record.

@End(Description)

See Section @Ref(Cluster) for the list of queries associated with this
table.]

@C(machclumap)@\@Multiple[Machine-Cluster Map.  This tables is used to
assign machines to clusters.

@Begin(Description)

@I(cluster_id)@\cluster id.

@I(machine_id)@\machine id.

@End(Description)

See Section @Ref(Cluster) for the list of queries associated with this
table.]

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

@End(Description)

See Section @Ref(Svc) for the list of queries associated with this table.
]

@C(prclumap)@\@Multiple[This table provides a mapping between printer
service cluster names and printer names.

@Begin(Description)

@I(prcluster)@\printer cluster name

@I(p_id)@\printer id.

@End(Description)

See Section @Ref(Prclusters) for the list of queries associated with this
table.
]

@C(servers)@\@Multiple[Server Information.  This table contains information
needed by the Data Control Manager or applications for each known server.

@Begin(Description)

@I(service_name)@\name of service.

@I(update_int)@\server update interval in minutes (for DCM).

@I(target_file)@\target file on server for DCM generated server files.

@I(dfgen)@\time of server file generation 

@I(script)@\shell script used by servers for particular use.

@End(Description)

See Section @Ref(Servers) for the list of queries associated with this table.
]

@C(Hosts)@\@Multiple[Server to Host mapping table.  Used by the
Data Control Manager to map a server to a list of server hosts.]

@Begin(Description)

@I(service_name)@\name of service.

@I(mach_id)@\Machine id for a host containing the service.

@I(enable)@\Enable switch for DCM. This switch controls whether or not 
the DCM updates a server. (0 - Do not Update, 1 - Update)

@I(override)@\Override time (minutes).  Used by DCM and update mechanism to
indicate that an update has failed.  This time is used to update services
at a different time from the default update interval time.  (-1 - Use
the default interval time, 0 or greater - Use the override interval).

@I(ltt)@\Last time tried.  Used by dcm, this field is adjusted each time
a service is attemted to be updated, regardless of success or failure.

@I(success)@\Flag indicating successful completion of server update.

@I(value1)@\server-specific data used by applications (i.e., number of servers
permitted per machine).

@I(value2)@\additional server-specific data.

@End(Description)

See Section @Ref(Servers) for the list of queries associated with this
table.
]

@C(services)@\@Multiple[TCP/UDP Port Information.  This is the information
currently in /etc/services.  In a workstation environment with SMS and the
Hesiod name server, service information will be obtained from the name
server.

@Begin(Description)

@I(service)@\service name.

@I(protocol)@\protocol: one of TCP, UDP.

@I(port)@\port number.

@I(description)@\description of service.

@End(Description)

See Section @Ref(Services) for the list of queries associated with this
table.
]

@C(filesys)@\@Multiple[File System Information.  This section desribes the file
system information necessary for a workstation to attach a file system.

@Begin(Description)

@I(label)@\a unique name for an attachable file system.

@I(type)@\currently one of RVD, NFS, or RFS.

@I(machine_id)@\file server machine.

@I(name)@\name of file system on the server.

@I(mount)@\default mount point for file system.

@I(access)@\default access mode for file system.

@End(Description)

See Section @Ref(Filesys) for the list of queries associated with this table.
]

@C(rvdsrv)@\@Multiple[RVD Server Information.  This table contains the top
level access control lists for each rvd server.  Any other per-server
information should be added here.

@Begin(Description)

@I(machine_id)@\server machine.

@I(operations_acl)@\operations access control list.

@I(admin_acl)@\administrative access control list.

@I(shutdown_acl)@\shutdown access control list.

@End(Description)

See Section @Ref(Rvdsrv) for the list of queries associated with this table.
]

@C(rvdphys)@\@Multiple[Physical device partition table.

@Begin(Description)

@I(machine_id)@\server machine.

@I(device)@\rvd physical device.

@I(size)@\size in 512-byte blocks.

@I(created)@\creation time.

@I(modified)@\modification time.

@End(Description)

See Section @Ref(Rvdphys) for the list of queries associated with this table.
]

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

@End(Description)

See Section @Ref(Rvdvirt) for the list of queries associated with this table.
]

@C(nfsphys)@\@Multiple[NFS Server Information.  This table contains for each
nfs server machine a list of the physical device partitions from which
directories may be exported.  For each such partition an access control list
is provided.

@Begin(Description)

@I(machine_id)@\server machine.

@I(device)@\file system name.

@I(dir)@\top-level directory of device.

@I(allocated)@\number of quota units allocated to this device.

@I(size)@\capacity of this device in quota units.

@End(Description)

See Section @Ref(Nfsphys) for the list of queries associated with this table.
]

@C(nfsquota)@\@Multiple[NFS Server Quota Information.  This table contains
per user per server quota information.

@Begin(Description)

@I(machine_id)@\nfs server machine.

@I(device)@\nfs server file system.

@I(users_id)@\user id.

@I(quota)@\user quota in quota units.

@End(Description)

See Section @Ref(Nfsquota) for the list of queries associated with this
table.
]

@C(printer)@\@Multiple[Printer Information.

@Begin(Description)

@I(name)@\a unique printer name.

@I(printer_id)@\internal database identifier for this record.

@I(type)@\printer hardware type: one of LPS40, 3812, LN03, LN01, etc.

@I(description)@\description of this printer.

@End(Description)

See Section @Ref(Printer) for the list of queries associated with this table.
]

@C(queue)@\@Multiple[Printer queues.  This table contains a list of unique
queue names and the attributes of each queue.  Attributes of queue are its
@i(ability), a status string (used by MDQS servers), and an access control
list.

@Begin(Description)

@I(name)@\unique queue name.

@I(queue_id)@\internal database id for this queue.

@I(machine_id)@\server machine.

@I(abilities)@\printer abilities associated with this queue (stored as an
integer bitmask).

@I(default)@\flag indicating whether this printer should be treated as a
default printer.

@I(status)@\queue status string (used by MDQS).

@End(Description)

See Section @Ref(Queue) for the list of queries associated with this table.
]

@C(pqm)@\@Multiple[Printer to queue mapping.  This table provides the mapping
between printers and queues.

@Begin(Description)

@I(printer_id)@\printer id.

@I(queue_id)@\queue id.

@End(Description)

See Section @Ref(Pqm) for the list of queries associated with this table.
]

@C(qdev)@\@Multiple[MDQS device information.  Each MDQS server has a device
table that assigns a logical name and status information to each known
physical printer device.

@Begin(Description)

@I(machine_id)@\MDQS server machine.

@I(qdev_id)@\internal database id for this device.

@I(name)@\logical name of device.

@I(device)@\physcial device name (e.g.: /dev/lp0)

@I(status)@\status string kept by MDQS.

@End(Description)

See Section @Ref(Qdev) for the list of queries associated with this table.
]

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

@End(Description)

See Section @Ref(Qdm) for the list of queries associated with this table.
]

@C(pobox)@\@Multiple[Post Office Information.  This list matches users with
one or more post office boxes.  A post office box is identified by its type
(POP, LOCAL), the machine on which the box resides, and the box name on that
machine.

@Begin(Description)

@I(users_id)@\id for a @C(users) entry.

@I(type)@\mailbox type: one of POP, LOCAL, or FOREIGN.

@I(machine_id)@\post office server machine (or string_id if type is FOREIGN).

@I(box)@\mailbox name on server.

@End(Description)

See Section @Ref(Pobox) for the list of queries associated with this table.
]

@C(list)@\@Multiple[Lists are used as a general purpose means of grouping
serveral objects togther.  This table contains descriptive information for
each list; the @c[members] table contains the the list of objects that are
in the list.  The ability to add or delete objects in a list is controlled
by an access control list associated with the list.  An access control list,
which is itself a list, contains as members a set of users who have the
capability to manipulate the object specifying the access control list.

@Begin(Description)

@I(name)@\list name.

@I(list_id)@\internal database id for this list.

@I(flags)@\currently one or more of ACTIVE, PUBLIC, HIDDEN.  (These flags
are used for mailing lists.)

@I(description)@\description of list.

@I(acl_id)@\a list id for the administrators' list.

@I(creator)@\users id of the creator of this list.

@I(expdate)@\expiration date of list.

@I(modtime)@\time list was last modified (@c(list) entry or @C(members)
entry). 

@End(Description)

See Section @Ref(List) for the list of queries associated with this table.
]

@C(members)@\@Multiple[List members.  Members are specified by a member type
and a member id pair.

@Begin(Description)

@I(list_id)@\id of a list.

@I(member_type)@\member type: one of USER, LIST, STRING.

@I(member_id)@\id of a member (a @c[users] id, @c[list] id, or @c[string]
id.)

@End(Description)

See Section @Ref(Members) for the list of queries associated with this table.
]

@C(strings)@\@Multiple[Used for list members of @i(string) type.  An
optimization for dealing with (usually long) foreign mail addresses.

@Begin(Description)

@I(string_id)@\member id.

@I(string)@\string.

@I(refc)@\Reference count.  A single string can be a member of multiple
lists.  When the reference count goes to zero, the string is deleted.

@End(Description)

See Section @Ref(Strings) for the list of queries associated with this table.
]

@C(maillists)@\@Multiple[This table contains the set of list ids for the
lists which are to be used as mailing lists.

@Begin(Description)

@I(list_id)@\a list id.

@End(Description)

See Section @Ref(Maillists) for the list of queries associated with this
table.
]

@C(groups)@\@Multiple[This table contains the set of list ids for the lists
which are to be used as groups.

@Begin(Description)

@I(list_id)@\a list id.

@I(gid)@\unix gid.

@End(Description)

See Section @Ref(Groups) for the list of queries associated with this table.
]

@C(acls)@\@Multiple[This table contains a set of service, list id pairs
which define the access control lists that are needed for each service.

@Begin(Description)

@I(service)@\service name.

@I(list_id)@\a list id.

@End(Description)

See Section @Ref(Acls) for the list of queries associated with this table.
]

@C(capacls)@\@Multiple[This table associates access control lists with
particular capabilities.  An important use of this table is for defining the
access allowed for executing each of the SMS predefined queries.  Each query
name appears as a capability name in this list.

@Begin(Description)

@I(capability)@\a string.

@I(tag)@\four character tag name for this capability.

@I(list_id)@\a list id.

@End(Description)

See Section @Ref(Capacls) for the list of queries associated with this table.
]

@C(alias)@\@Multiple[Aliases are used by several different services to
provide alternative names for objects or a mapping one type of object and
another.  Some examples of alias usage are printer aliases, service aliases,
cluster aliases, file system aliases, and print cluster to printer maps.  As
an integrity constraint it is required that all aliases be of a known type.
The list of known alias types is actually stored in the database as the set
of translations of aliases with name "alias" and type "type".  Therefore, it
is also quite easy to add new alias types.  Another use of the "type" alias
type is for storing known field values for validated table fields.

@Begin(Description)

@I(name)@\alias name.

@I(type)@\alias type: currently one of TYPE, PRINTER, SERVICE, CLUSTER,
FILESYS, PRCLUSTER, MACH-CLU-MAP.

@I(trans)@\alias translation.

@End(Description)

See Section @Ref(Alias) for the list of queries associated with this table.
]

@C(values)@\@Multiple[Values needed by the server or application programs.

@Begin(Description)

@I(name)@\value name.

@I(value)@\value.

@End(Description)

See Section @Ref(Values) for the list of queries associated with this table.
]

@C(tblstats)@\@Multiple[Table Statistics.  For each table in the SMS
database statistics are kept for the number of retrieves, appends, updates,
and delete performed on the table.  In addition, the last modification time
is kept.

@Begin(Description)

@I(table)@\table name.

@I(retrieves)@\count of retrievals on this table.

@I(appends)@\count of additions to this table.

@I(updates)@\count of updates to this table.

@I(deletes)@\count of deletions to this table.

@I(modtime)@\time of last modification (append, update, or delete).

@End(Description)

See Section @Ref(Tblstats) for the list of queries associated with this table.
]

@End(Description)
