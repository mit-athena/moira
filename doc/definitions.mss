@Device(PostScript)

Here it is necessary to define each of the entities that Moira manages.
If the definitions are accurate and complete, then the database and
queries can be designed in terms of these definitions.

@begin(Description)

Person@\A @i(person) is the human behind a particular @i(user) account.
This person might or might not have an @i(account) at this time.  Each
person has attributes such as their real name, their ID number, and
their class (academic year, staff, etc) which will not change or will
change very infrequently.  The person's real name is not necessarily
unique, although their ID number is.  This information is usually
obtained from the registrar.  Only administrators are allowed to
change this information.  It is considered private, and is not
publicly available.  Each person also has a version of their name
that they show to the public, along with their address and phone
number.  This "finger information" is public, and can be modified by
that person.

User@\The term @i(user) will refer usually to a @c(unix) account, not to
the @i(person) using the account.  An account has a @c(unix) login name, a
UID, a GID, and associated information, and is also a kerberos
principal.  The login name is initially chosen by the person, and may
not be changed except by an administrator.  The UID and GID are
assigned by athena and may not be changed by the user.  The account
also has a shell and a @i(mail drop) which can be set by the user.  A
person is associated with most accounts, although a person may have
more than one account, and an account might not belong to a single
person.  All account information is publicly available.

Mail Drop@\This is where mail addressed to a particular account is
sent.  It may be either a post office box, or an SMTP address.  A post
office box is a drop on a post office server machine.  The box name is
the same as the user's login name, and the machine must be a
registered post office.  SMTP addresses are character strings which
are not parsed by the Service Management System.  They should be mail
addresses that make sense from the Athena Mail Hub.  This information
is publicly available.

Machine@\A machine is a host computer on the network.  It may be a
workstation or a server.  It has a name, one or more addresses, and a
type such as @b(vax) or @b(rt).  It also belongs to a collection of
service @i(clusters).  This information is publicly available.

Cluster@\A @i(cluster) is a mapping of @i(machines) to service
locations.  In general, it corresponds to a group of machines in a
room.  When a machine requests its @i(cluster) information, it gets
the @i(cluster data) associated with each of the clusters it is a
member of.  For example if a room has several vaxen and several rt's,
then you might need 3 clusters: one that all of the vaxen belong to
that lists the vax-specific services, another that all of the rt's
belong to the lists the rt-specific services, and a third that all of
the workstations in that room belong to that lists all of the common
services (although you could do this with just two clusters,
duplicating the common info in the vax & rt -specific clusters).  This
information is publicly available.

Cluster Data@\Each @i(machine) has a minimum set of services that it
must be able to locate: @b(SYSLIB, USRLIB, LPR, ZEPHYR, HESIOD), and
@b(KERBEROS).  These are identified by @b(type), i.e. the service it
locates, the cluster @b(name), and the name of the specific @b(service
instance).  This information is publicly available.

Filesystem@\A @i(filesystem) is a reference to all or part of a
physical filesystem.  There are two kinds of filesystems: @b(nfs) and
@b(rvd).  An @b(rvd) filesystem exists on a particular @i(machine),
and has a number of other parameters which are not significant to Moira.
An @b(nfs) filesystem exists on a registered @i(NFS physical
filesystem).  A single @i(filesystem) name may correspond to several
different physical filesystems, ordered in a particular way.  Each
filesystem has a name that makes sense to the server machine, a mount
point the client needs, and an access mode.  It may be necessary for a
structure to be created on the server, for this both an individual
owner and group owning list are specified.  Each filesystem is of a
particular type (such as @b(HOMEDIR), @b(PROJECT), or @b(SYSTEM)).
This information is publicly available.

Locker@\A @i(locker) is a special case of a @i(filesystem).  Its name
is also a username.  It is of type @b(nfs), and has a mount point of
"/mit/@i(name)", an owner of @i(name), an owning group of @i(name)'s
@i(user group), and a locker-type of @b(HOMEDIR).  There is only one
physical filesystem matching this filesystem.  There is a quota on the
filesystem.

NFS Physical Filesystem@\This names a disk partition that is exported
for NFS use.  It is identified by the @i(machine) name and the device
name of the partition.  It has a particular size and allocation.  It
also has a status which indicates what types of filesystems may be
placed on it.  The allocation is the sum of all of the quotas assigned
to the partition.

Quota@\A @i(quota) is the amount of storage a particular user is
allowed on a particular filesystem.  If a user is granted quotas on
two or more filesystems on the same physical filesystem, he will have
the sum of those two quotas to split between the filesystems.  A user
without an explicit quota on a filesystem will have a (very small)
quota assigned.

List@\A list is a collection of @i(users), @i(lists), and character
strings.  A list has a name, a description, some attributes, and an
owner in addition to its members.  The owner may be a user or a list.
If the owner is a list, it may be the same list (self-referential), or
another list.  The list attributes are @b(active, hidden, public,
maillist), and @b(group).  If a list is @b(active), it will appear to
the name servers and mail servers as appropriate, otherwise it cannot
be used.  A @b(hidden) list's existence should not be known unless the
person inquiring is an owner.  The list may still be visible from the
mail server or name server, but the Service Management System will not
disclose it.  A @b(public) list is one that anyone may add themselves
to or remove themselves from.  A list may be a @b(maillist),
indicating that it should be loaded onto the central mail hub so that
the Athena mail server can use it.  A list may also be a @b(group) and
have a @c(unix) UID associated with it.  With the exception of @b(public)
lists, only the list owners may change the membership of a list.  The
owners may also change the description and the attributes.  This
information is publicly available.

Maillist@\A maillist is a @i(list) with the @b(maillist) attribute
set.  If the list is also @b(active), then it will be loaded onto the
mail hub so that mail sent to the list name will be sent to each of
the members of the list.  Because of this expansion, each member of
the list must be a user, a string which is really an SMTP address, or
another maillist.  So any list which is a member of a maillist must
itself be a maillist.  @i(User groups) should not be on maillists,
since a usergroup cannot be distinguished from a reference to the
actual @i(user).

Group@\A @i(group) is a @i(list) with the @b(group) attribute set and
a GID assigned.  If the @b(active) attribute is also set, then it will
be loaded into the hesiod nameserver so that it may be used as a @c(unix)
user group.

User Group@\A @i(user group) is a @i(list) which is also a @i(group),
and its name is the same as a username.  Usually the only member is
that user, and the user is also the owner.  Because there are many
user groups, and most of them are never used except to feed the
nameserver, their storage is optimized if they are not referenced by
another list and do not have any other members.

ACL@\Many entities in the database have @i(Access Control List)s
associated with them: lists, services, filesystems, zephyr and queries.
Each ACL has a type field and a name field.  The type may be @b(NONE),
indicating that no access is to be allowed.  If may be @b(USER), and
the name will be a login name, indicating that that user will have
access.  Or it may be @b(LIST) and the name of a list, indicating that
anyone on that list or any of its sub-lists will have access to the
entity.

Zephyr class@\A given @i(class) of zephyr message may need access
control.  There are four different access control lists that are
needed for each zephyr class: @b(xmt, sub, iws), and @b(iui).  The
absence of an ACL is different from an empty ACL.

Service@\A service is a software system that gets updated by Moira, for
example @b(HESIOD) or @b(NFS).  There are many pieces of data
necessary to keep track of the correct and efficient updating of these
services.  There may be a few items that are service specific that
need to be remembered for each instance of that service.

Printer@\There are lots of things you might want to know about a
printer, but we don't have a queuing system to make use of them.  So
for now, Moira only manages printcap entries for the nameserver.

Aliases@\Alternate names for @i(filesystems) and @i(services) are
remembered as aliases.  When the nameserver is loaded, these aliases
are loaded as additional names for these objects.

@end(description)
