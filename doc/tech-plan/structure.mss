@part[structure, root "moira.mss"]
@newpage()
@MajorSection(Data Fields and Relationships)
@tag(database_fields)

The knowledge base of Moira enables system services and servers to be updated
with correct information.  The database has the reponsibility of storing
information which will be transmitted to the services.  The database will
not, however, be responsible for knowing the format of data to be sent.
This information will be inherent to the Data Control Manager.  Specific
fields of the database are organized to represent the needs of system.  
The current Moira database is comprised of the following tables:

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

@I(shell)@\the user's default shell.

@I(last, first, middle)@\The user's full name, broken down for convenient
indexing. 

@I(status)@\contains the user's account status.  Currently defined
statuses are:
@format{
0 - Not registered, but registerable
1 - Active account
2 - Half-registered
3 - Marked for deletion
4 - Not registerable
}
Only accounts in state 1 will have their information show up in
database extracts for system services.

@I(mit_id)@\the user's encrypted MIT id.

@I(mit_year)@\a student's academic year, not modifiable by the student.
Used for Athena administrative purposes.

@I(modtime)@\the time that this record was last modified (or created).

@I(modby)@\the person who modified this record last.

@I(modwith)@\the service that modified this record last.

@I(fullname)@\the user's full name.

@I(nickname)@\the user's nickname.

@I(home_addr)@\home address.

@I(home_phone)@\home phone.

@I(office_addr)@\office address.

@I(office_phone)@\office phone.

@I(mit_dept)@\MIT address; this is for on-campus students' living addresses.

@I(mit_affil)@\one of undergraduate, graduate, staff, faculty, other.

@I(fmodtime)@\time finger record was last modified.

@I(fmodby)@\person who made this modification.

@I(fmodwith)@\service that made this modification.

@I(potype)@\mailbox type: one of POP, SMTP, or NONE.

@I(pop_id)@\Machine ID of current or last known POP server.

@I(box_id)@\String ID of box name if type is SMTP.

@I(pmodtime)@\time pobox record was last modified.

@I(pmodby)@\person who made this modification.

@I(pmodwith)@\service that made this modification.

@I(gid)@\@i([unused]) intended for optimized user groups, but never implemented.

@I(uglist_id)@\@i([unused]) intended for optimized user groups, but never implemented.

@I(ugdefault)@\@i([unused]) intended for optimized user groups, but never implemented.

@End(Description)

See Section @Ref(Users) for the list of queries associated with this table.

There is no entry for a password because it is being subsumed by another 
service (Kerberos).]

@C(machine)@\@Multiple[Machine Information.  

@Begin(Description)

@I(name)@\the canonical hostname.

@I(mach_id)@\an internal database id for this record.

@I(type)@\machine type: VAX, RT

@I(modtime)@\the time that this record was last modified (or created).

@I(modby)@\the person who modified this record last.

@I(modwith)@\the service that modified this record last.

@End(Description)

See Section @Ref(Machine) for the list of queries associated with this table.
]

@C(cluster)@\@Multiple[Cluster Infomation. There are several named clusters
throughout Athena that correspond roughly to subnets and/or geographical
areas.

@Begin(Description)

@I(name)@\cluster name.

@I(clu_id)@\internal database identifier for this record.

@I(desc)@\cluster description.

@I(location)@\cluster location.

@I(modtime)@\the time that this record was last modified (or created).

@I(modby)@\the person who modified this record last.

@I(modwith)@\the service that modified this record last.

@End(Description)

See Section @Ref(Cluster) for the list of queries associated with this
table.]

@C(mcmap)@\@Multiple[Machine-Cluster Map. This tables is used to
assign machines to clusters.

@Begin(Description)

@I(mach_id)@\machine id.

@I(clu_id)@\cluster id.

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

@I(clu_id)@\references an entry in the cluster table.

@I(serv_label)@\label of a service cluster type (e.g. "usrlib",
"syslib", "zephyr")

@I(serv_cluster)@\specific service cluster data (e.g. "bldgw20-vssys")

@End(Description)

See Section @Ref(Svc) for the list of queries associated with this table.
]

@C(list)@\@Multiple[Lists are used as a general purpose means of
grouping serveral objects togther.  This table contains descriptive
information for each list; the @c[members] table contains the the list
of objects that are in the list.  The ability to add or delete objects
in a list is controlled by an access control entity associated with
the list.  The access control entity may be a USER, a LIST, or NONE.
An access control entity names the user or list of users who have the
capability to manipulate the object specifying the access control
list.

@Begin(Description)

@I(name)@\list name.

@I(list_id)@\internal database id for this list.

@I(active)@\Indicates this list should be extracted in service updates.

@I(public)@\Indicates any user may add or delete themselves as
members of this list.

@I(hidden)@\Indicates that neither the list information or membership
may be divulged to anyone who is not an administrator of this list.

@I(maillist)@\Indicates that this list is a maillist.

@I(group)@\Indicates that this list is a unix group.

@I(gid)@\Unix GID, if this list is a unix group.  This will have value
-1 to indicate: assign a unique GID if this list is ever made a unix
group.

@I(desc)@\description of list.

@I(acl_type)@\Type of access control entity: LIST, USER, or NONE.

@I(acl_id)@\Access control entity identifier; a list ID, user ID, or
ignored if type is NONE.

@I(modtime)@\the time that this record was last modified (or created).

@I(modby)@\the person who modified this record last.

@I(modwith)@\the service that modified this record last.

@End(Description)

See Section @Ref(Lists) for the list of queries associated with this table.
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

@C(servers)@\@Multiple[Server Information. This table contains information
needed by the Data Control Manager or applications for each known
service to be updated.

@Begin(Description)

@I(name)@\name of service.

@I(update_int)@\server update interval in minutes (for DCM).

@I(target_file)@\where on the server being updated to put the file
generated by the DCM.

@I(script)@\where on Moira to find the shell script which will install
new configuration files on a server.

@I(dfgen)@\time of last server file generation.

@I(dfcheck)@\time of last check to see if server files needed to be
generated.

@I(type)@\service type: UNIQUE or REPLICAT(ed).

@I(enable)@\Enable switch for DCM. This switch controls whether or not 
the DCM generates files for this service. (0 - Do not Update, 1 - Update)

@I(inprogress)@\indicator that a DCM is generating new configuration
files for this service right now.

@I(harderror)@\indication that an error has occured while generating
files (or while propogating files is service type is replicated).
This is not a boolean, but the actual error number.

@I(errmsg)@\a text description of the @i(harderror) reported above.

@I(acl_type)@\type of access control entity for this service.

@I(acl_id)@\id of access control entity for this service.

@I(modtime)@\the time that this record was last modified (or created).

@I(modby)@\the person who modified this record last.

@I(modwith)@\the service that modified this record last.

@End(Description)

See Section @Ref(Servers) for the list of queries associated with this table.
]

@C(serverhosts)@\@Multiple[Server to Host mapping table. Used by the
Data Control Manager to map a server to a list of server hosts.

@Begin(Description)

@I(service)@\name of service.

@I(mach_id)@\Machine id for a host containing the service.

@I(success)@\Flag indicating successful completion of most recent
server update.

@I(enable)@\Enable switch for DCM. This switch controls whether or not 
the DCM updates a server. (0 - Do not Update, 1 - Update)

@I(override)@\Override flag.  Used by DCM and update mechanism to
indicate that it should attempt to update this host, even if the
necessary time interval has not elapsed.

@I(inprogress)@\indicator that a DCM is updating this host right now.

@I(hosterror)@\indication that an error has occured while updating
this host.  This is not a boolean, but the actual error number.

@I(hosterrmsg)@\a text description of the @i(hosterror) reported above.

@I(ltt)@\Last time tried.  Used by the DCM, this field is adjusted
each time a service is attemted to be updated, regardless of success
or failure.

@I(lts)@\Last time successful.  This records the last time the DCM
successfully updated the server.

@I(value1)@\server-specific integer data used by applications (i.e.,
number of servers permitted per machine).

@I(value2)@\additional server-specific integer data.

@I(value3)@\additional server-specific string data.

@I(modtime)@\the time that this record was last modified (or created).

@I(modby)@\the person who modified this record last.

@I(modwith)@\the service that modified this record last.

@End(Description)

See Section @Ref(Servers) for the list of queries associated with this
table.
]

@C(filesys)@\@Multiple[File System Information.  This section desribes the file
system information necessary for a workstation to attach a file system.

@Begin(Description)

@I(label)@\a name for an attachable file system.  This is not
necessarily unique.

@I(order)@\an integer used to indicate sort-order for multiple
filesystems with the same label.  Also, (@i(label, order)) tuples are
unique among filesystems.

@I(filsys_id)@\internal database identifier for the filesystem record.

@I(phys_id)@\internal database identifier for the phsical partition
containing the logical filesystem.  For filesystems of type NFS this
is an nfsphys_id.  For other types, it is unused.

@I(type)@\currently one of RVD, NFS, or ERR.

@I(mach_id)@\file server machine.

@I(name)@\name of file system on the server.  For type RVD, this is
the pack name.  For type NFS, this is the directory name on the server.

@I(mount)@\default mount point for file system.

@I(access)@\default access mode for file system.

@I(comments)@\any special notes about the filesystem.

@I(owner)@\this is the users_id of the owner of the filesystem.  If
the filesystem is automatically created, this user will own it.

@I(owners)@\this is the list_id of the owning group of the filesystem.
If the filesystem is automatically created, this group will own it.

@I(createflg)@\indicates that the filesystem should be automatically
created if it does not already exist.

@I(lockertype)@\one of HOMEDIR, PROJECT, COURSE, SYSTEM, etc.  This
may affect what is done when a filesystem is automatically created.

@I(modtime)@\the time that this record was last modified (or created).

@I(modby)@\the person who modified this record last.

@I(modwith)@\the service that modified this record last.

@End(Description)

See Section @Ref(Filesys) for the list of queries associated with this table.
]

@C(nfsphys)@\@Multiple[NFS Server Information.  This table contains for each
nfs server machine a list of the physical device partitions from which
directories may be exported.  For each such partition an access control list
is provided.

@Begin(Description)

@I(nfsphys_id)@\internal database identifier of this parition.

@I(mach_id)@\server machine.

@I(dir)@\top-level directory of device.

@I(device)@\partition name.

@I(status)@\a bit field encoding what the parition is used for.
Current assignments are:@format{
bit 0 (LSB) - Student lockers
bit 1 -       Faculty lockers
bit 2 -       Staff lockers
bit 3 -       Miscellaneous
}

@I(allocated)@\number of quota units allocated to this device.

@I(size)@\capacity of this device in quota units.

@I(modtime)@\the time that this record was last modified (or created).

@I(modby)@\the person who modified this record last.

@I(modwith)@\the service that modified this record last.

@End(Description)

See Section @Ref(Nfsphys) for the list of queries associated with this table.
]

@C(nfsquota)@\@Multiple[NFS Server Quota Information.  This table contains
per user per server quota information.

@Begin(Description)

@I(users_id)@\user id of account this quota belongs to.

@I(filsys_id)@\filesys_id of logical filesystem this quota applies to.

@I(phys_id)@\nfsphys_id of partition that filesystem resides on.  This
is redundant information, here for performance reasons.

@I(quota)@\user quota in quota units.

@I(modtime)@\the time that this record was last modified (or created).

@I(modby)@\the person who modified this record last.

@I(modwith)@\the service that modified this record last.

@End(Description)

See Section @Ref(Nfsquota) for the list of queries associated with this
table.
]

@C(zephyr)@\@Multiple[Zephyr class access control list information.
This table contains an entry for each controlled class.  Each records
records an access control entity for each of the four functions of
that class.

@Begin(Description)

@I(class)@\name of the zephyr class.

@I(xmt_type)@\access control entity type for transmit

@I(xmt_id)@\access control entity id for transmit

@I(sub_type)@\access control entity type for subscriptions

@I(sub_id)@\access control entity id for subscriptions

@I(iws_type)@\access control entity type for instance wildcard specification

@I(iws_id)@\access control entity id for instance wildcard specification

@I(iui_type)@\access control entity type for instance UID identity

@I(iui_id)@\access control entity id for instance UID identity

@I(modtime)@\the time that this record was last modified (or created).

@I(modby)@\the person who modified this record last.

@I(modwith)@\the service that modified this record last.

@End(Description)

See Section @Ref(Zephyr) for the list of queries associated with this table.
]

@C(hostaccess)@\@Multiple[This table contains the necessary
information for Moira to be generating /.klogin or /etc/passwd files.
It associates an access control entity with a machine.

@Begin(Description)

@I(mach_id)@\machine.

@I(acl_type)@\access control entity type

@I(acl_id)@\access control entity id

@I(modtime)@\the time that this record was last modified (or created).

@I(modby)@\the person who modified this record last.

@I(modwith)@\the service that modified this record last.

@End(Description)

See Section @Ref(Hostaccess) for the list of queries associated with this
table.
]

@C(strings)@\@Multiple[Used for list members of @i(string) type.  An
optimization for dealing with foreign mail addresses in poboxes or as
list members.

@Begin(Description)

@I(string_id)@\member id.

@I(string)@\string.

@End(Description)

See Section @Ref(Strings) for the list of queries associated with this table.
]

@C(services)@\@Multiple[TCP/UDP Port Information.  This is the information
currently in /etc/services.  In a workstation environment with Moira and the
Hesiod name server, service information will be obtained from the name
server.

@Begin(Description)

@I(name)@\service name.

@I(protocol)@\protocol: one of TCP, UDP.

@I(port)@\port number.

@I(desc)@\description of service.

@I(modtime)@\the time that this record was last modified (or created).

@I(modby)@\the person who modified this record last.

@I(modwith)@\the service that modified this record last.

@End(Description)

See Section @Ref(Services) for the list of queries associated with this
table.
]

@C(printcap)@\@Multiple[Printer capability table.  This contains the
information currently in /etc/printcap.

@Begin(Description)

@I(name)@\a unique printer name.

@I(mach_id)@\server machine.

@I(dir)@\spooling directory

@I(rp)@\remote printer name

@I(comments)@\description of service.

@I(modtime)@\the time that this record was last modified (or created).

@I(modby)@\the person who modified this record last.

@I(modwith)@\the service that modified this record last.

@End(Description)

See Section @Ref(Printcap) for the list of queries associated with this
table.
]

@C(capacls)@\@Multiple[This table associates access control lists with
particular capabilities.  An important use of this table is for defining the
access allowed for executing each of the Moira predefined queries.  Each query
name appears as a capability name in this list.

@Begin(Description)

@I(capability)@\a string, usually the full name of a query.

@I(tag)@\four character tag name for this capability, usually the
short name of a query.

@I(list_id)@\a list id.

@End(Description)

See Section @Ref(Capacls) for the list of queries associated with this table.
]

@C(alias)@\@Multiple[Aliases are used by several different services to
provide alternative names for objects or a mapping one type of object and
another.  They are also used to record legal values for type-checked
fields, and type translations of some type fields.

Some examples of alias usage are file system aliases, service
aliases, and printer aliases.  Each alias of this form will have the
alias as the @i(name), a @i(type) of PRINTER, SERVICE, or FILESYS, and
a @i(trans) of the real name of the object.

All type-checking aliases are of @i(type) TYPE.  The @i(name) is the
name of the type-checked field, and the @i(trans) is a possible value
for that field.  Some type-checked fields are "alias", "boolean",
"class", "filesys", etc.  For example, alias types themselves are
typechecked, so that one cannot store an alias of type @b(aliastype)
unless there is an alias entry of the form (alias, TYPE, aliastype).

Type translations are used to record things like the data stored with
an SMTP pobox is of type string.  These aliases have a @i(name) which
is the type string the user enters, a @i(type) of TYPEDATA, and a
@i(trans) of the actual data type, i.e. user, list, string, machine, etc.

@Begin(Description)

@I(name)@\alias name.

@I(type)@\alias type: currently one of TYPE, PRINTER, SERVICE,
FILESYS, TYPEDATA.

@I(trans)@\alias translation.

@End(Description)

See Section @Ref(Alias) for the list of queries associated with this table.
]

@C(values)@\@Multiple[Values needed by the server or application
programs.  These are hints for the next ID number to assign, some
state variables such as the dcm enable flag, and values such as the
default quota for new users.

@Begin(Description)

@I(name)@\value name.

@I(value)@\value.

@End(Description)

See Section @Ref(Values) for the list of queries associated with this table.
]

@C(tblstats)@\@Multiple[Table Statistics.  For each table in the Moira
database statistics are kept for the number of appends, updates, and
deletes performed on the table.  In addition, the last modification
time of the table is kept.

@Begin(Description)

@I(table)@\table name.

@I(modtime)@\time of last modification (append, update, or delete).

@I(retrieves)@\@i(obsolete) count of retrievals on this table.  This
is unused now for performance reasons.

@I(appends)@\count of additions to this table.

@I(updates)@\count of updates to this table.

@I(deletes)@\count of deletions to this table.

@End(Description)

See Section @Ref(Tblstats) for the list of queries associated with this table.
]

@End(Description)
