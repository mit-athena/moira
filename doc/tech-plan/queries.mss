@Device(Postscript)
@part[queries, root="moira.mss"]
@NewPage()

@MajorSection(Predefined Queries - List of Database Interfaces)
@Label(Queries)

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

Providing a generallized layer of functions affords Moira the capability
of being database independent.  Today, we are using INGRES; however,
in the future, if a different database is required, the application
interface will not change.  The only change needed at that point will
be a new Moira server, linking the pre-defined queries to a new
set of data manipulation procedures.  

The following list of queries are a predefined list.  This list provides
the mechanism for reading, writing, updating, and deleting information
in the database.

In each query description below there are descriptions of the required
arguments, the return values, integrity constraints, possible error
codes, and side effects, if any.  In addition to the error codes
specifically listed for each query, any query may return: MR_PERM
"Insufficient permission to perform requested database access" or
MR_ARGS "Incorrect number of arguments".  Any retrieval query may
return MR_NO_MATCH "No records in database match query".  Any add or
update query may return MR_BAD_CHAR "Illegal character in argument"
if a bad character is in an argument that has character restrictions,
or MR_EXISTS if the new object to be added or new name of existing
object conflicts with another object already in the database.  Other
errors are listed with each query.


@SubSection(Users, Finger, and Post Office Boxes)
@label(Users)

@multiple[@b[get_all_logins]

@u[Args]: none

@u[Returns]: {login, uid, shell, last, first, mi}

Returns info on every account in the database.  The returned info is a
summary of the account info, not the complete information.]

@multiple[@b[get_all_active_logins]

@u[Args]: none

@u[Returns]: {login, uid, shell, last, first, mi}

Returns info on every account for which the status field is non-zero.
The returned info is a summary of the account info, not the complete
information.]

@multiple[@b[get_user_by_login]

@u[Args]: login

@u[Returns]: {login, uid, shell, last, first, mi, state, mitid, 
class, modtime, modby, modwith}

Returns complete account information on the named account.  Wildcards
may be used in the @i(login) name specified.  If the person executing
the query is not on the query ACL, then the query only succeeds if the
only retrieved information is about the user making the request.]

@multiple[@b[get_user_by_uid]

@u[Args]: uid

@u[Returns]: {login, uid, shell, last, first, mi, state, mitid,
class, modtime, modby, modwith}

Returns complete account information on any account with the specified
@i(uid).  If the person executing the query is not on the query ACL,
then the query only succeeds if the only retrieved information is
about the user making the request.]

@multiple[@b[get_user_by_name]

@u[Args]: {first, last}

@u[Returns]: {login, uid, shell, last, first, mi, state, mitid,
class, modtime, modby, modwith}

Returns complete account information on any account with matching
@i(first) and @i(last) name fields.  Either or both names may contain
wildcards, so that this query can do the equivalent of lookup by
firstname or lookup by lastname.]

@multiple[@b[get_user_by_class]

@u[Args]: class

@u[Returns]: {login, uid, shell, last, first, mi, state, mitid,
class, modtime, modby, modwith}

Returns complete account information on any account with a matching
class field.  The given class may contain wildcards.]

@multiple[@b[get_user_by_mitid]

@u[Args]: crypt(id)

@u[Returns]: {login, uid, shell, last, first, mi, state, mitid,
class, modtime, modby, modwith}

Returns complete account information on any account with a matching
MIT ID field.  The given id may contain wildcards.]

@multiple[@b[add_user]

@u[Args]: {login, uid, shell, last, first, mi, state, mitid, class}

@u[Returns]: none

Adds a new user to the database.  @i(login) must not match any
existing @i(login)s.  @i(uid) and @i(state) must be integers.  If the
given uid is @b(UNIQUE_UID) as defined in @i(<moira.h>), the next unused
uid will be assigned.  If @i(login) is @b(UNIQUE_LOGIN) as defined in
@i(<moira.h>), the login name will be a "#" followed by the uid.  For
example, when adding a person so that they may register later, the
query ausr(UNIQUE_LOGIN, UNIQUE_UID, /bin/csh, @i(Last, First, M), 0,
[encrypted ID], @i(class)) is used.  The @i(class) field must contain
a value specified as a @b(TYPE) alias for @b(class).  This query also
initializes the finger record for this user with just their full name,
and sets their pobox to @b(NONE).  It updates the modtime on the user,
finger and pobox records.  Errors: MR_NOT_UNIQUE "Arguments not
unique" if the login name is not unique, or MR_BAD_CLASS "Specified
class is not known" if the class is not in the alias database.]

@multiple[@b[register_user]

@u[Args]: {uid, login, fstype}

@u[Returns]: none

Registers a user.  This consists of changing their username, and
creating a pobox, a group list, a filesystem, and a quota for them.
The user is identified by @i(uid), which must match exactly one
existing user.  Further, this user must currently have a @i(status) of
0.  The user will be left with a @i(status) of 2.  The pobox created
will be of type @b(POP) on the least loaded post office.  The group
list will have the user as an owner, and a unique GID will be assigned.
The filesystem will be allocated on the least loaded fileserver which
supports @i(fstype), where @i(fstype) is @b(MR_FS_STUDENT,
MR_FS_FACULTY, MR_FS_STAFF), or @b(MR_FS_MISC) as defined in
@i(<moira.h>).  A quota will be assigned to the user on his filesystem
with the value taken from @b(def_quota) in the values table.  Errors:
MR_NO_MATCH, MR_NOT_UNIQUE "Arguments not unique" if the uid does
not specify exactly one user; MR_IN_USE "Object is in use" if the
login name is already taken.]

@multiple[@b[update_user]

@u[Args]: {login, newlogin, uid, shell, last, first, mi, state, mitid, class}

@u[Returns]: none

Updates the info in a user entry.  @i(login) specifies the existing
login name, the remaining arguments will replace the current values
of those fields.  This is not equivalent to deleting the user and
adding a new one, as all references to this user will still exist,
even if the login name is changed.  All fields must be specified, even
if the value is to remain unchanged.  @i(login) must match exactly one
user in the database.  @i(newlogin) must either match the existing
login or be unique in the database.  The @i(class) field must contain
a value specified as a @b(TYPE) alias for @b(class).  @i(uid) and
@i(state) must be integers.  The modtime fields in the user's record
will be updated.  Errors: MR_USER "No such user" if the login name
does not match exactly one user, MR_NOT_UNIQUE "Arguments not unique"
if the new login name is not unique, or MR_BAD_CLASS "Specified class
is not known" if the class is not in the alias database.]

@multiple[@b[update_user_shell]

@u[Args]: {login, shell}

@u[Returns]: none

Updates a user's shell.  @i(login) must match exactly one user.  The
modtime fields in the user's record will be updated.  This query may
be executed by the target user or by someone on the query ACL.
Errors: MR_USER "No such user" if the login name does not match
exactly one user.]

@multiple[@b[update_user_status]

@u[Args]: {login, status}

@u[Returns]: none

Updates a user's @i(status).  @i(login) must match exactly one user.
The modtime fields in the user's record will be updated.  Errors:
MR_USER "No such user" if the login name does not match exactly one
user.]

@multiple[@b[delete_user]

@u[Args]: login

@u[Returns]: none

Deletes a user record.  @i(login) must match exactly one user.  @b(The
user must have a status of zero, or MR_IN_USE will be returned.)
This will only be allowed if the user is not a member of any lists,
has any quotas assigned, or is the owner of an object.  It will also
delete associated finger information, post office box, and any quotas
the user has.  Errors: MR_USER "No such user" if the login name does
not match exactly one user, MR_IN_USE "Object is in use" if the user
is a member of a list, has a quota or is an ACE.]

@multiple[@b[delete_user_by_uid]

@u[Args]: uid

@u[Returns]: none

Deletes a user record.  @i(uid) must match exactly one user.  This
will only be allowed if the user is not a member of any lists or is
the owner of an object.  It will also delete associated finger
information and post office box.  Errors: MR_USER "No such user" if
the uid does not match exactly one user, MR_IN_USE "Object is in use"
if the user is a member of a list or is an ACE.]

@multiple[@b[get_finger_by_login]

@u[Args]: login

@u[Returns]: {login, fullname, nickname, home_addr, home_phone,
office_addr, office_phone, department, affiliation,
modtime, modby, modwith}

Gets all of the finger information for the specified user.  @i(login)
must match exactly one user.  The target user may retrieve his
information.  It is safe to point the query ACL at the list of all
users.  Errors: MR_USER "No such user" if the login name does not
match exactly one user.]

@multiple[@b[update_finger_by_login]

@u[Args]: {login, fullname, nicname, home_addr, home_phone,
office_addr, office_phone, department, affiliation}

@u[Returns]: none

Allows any part of the finger information to be changed for a
specified account.  @i(login) must match exactly one user.  The
remaining fields are free-form, and may contain anything.  The modtime
fields in the finger record will be updated.  A user may update his
own information.  Errors: MR_USER "No such user" if the login name
does not match exactly one user.]

@multiple[@b[get_pobox]

@u[Args]: login

@u[Returns]: {login, type, box, modtime, modby, modwith}

Retrieves a user's post office box assignment.  The @i(login) name
must match exactly one user.  See @i(set_pobox) for a summary of the
returned fields.  The owner of the pobox may perform this query.
Errors: MR_USER "No such user" if the login name does not match
exactly one user.]

@multiple[@b[get_all_poboxes]

@u[Args]: none

@u[Returns]: {login, type, box}

Retrieves all of the post office boxes from the database.  See
@i(set_pobox) for a summary of the returned fields.]

@multiple[@b[get_poboxes_pop]

@u[Args]: none

@u[Returns]: {login, type, machine}

Retrieves all of the post office boxes of type @b(POP) from the
database.  See @i(set_pobox) for a summary of the returned fields.]

@multiple[@b[get_poboxes_smtp]

@u[Args]: none

@u[Returns]: {login, type, box}

Retrieves all of the post office boxes of type @b(SMTP) from the
database.  See @i(set_pobox) for a summary of the returned fields.]

@multiple[@b[set_pobox]

@u[Args]: {login, type, box}

@u[Returns]: none

Establishes a user's post office box.  The given @i(login) must match
exactly one user.  The @i(type) will be checked against the alias
database for valid @b(pobox) types.  Currently allowed types are
@b(POP), @b(SMTP), and @b(NONE).  If the type is @b(POP), then box
must name a machine known by Moira.  If the type is @b(SMTP), then box
is the user's mail address with no other interpretation by Moira.  A
type of @b(NONE) is the same as not having a pobox.  The modtime
fields on the pobox record will be set.  The owner of the target pobox
may perform this query.  Errors: MR_USER "No such user" if the login
name does not match exactly one user, or MR_TYPE "Invalid type" if
the type is not @b(POP), @b(SMTP), or @b(NONE).]

@multiple[@b[set_pobox_pop]

@u[Args]: login

@u[Returns]: none

Forces a user's pobox to be type @b(POP).  The @i(login) name must
match exactly one user.  If the user's pobox is already of type
@b(POP), nothing will be changed.  If the user has previously had a
pobox of type @b(POP), then the previous post office machine
assignment will be restored.  If there was no previous post office
assignment, the query will fail with MR_MACHINE "Unknown machine"
since it will be unable to choose a post office machine.  The modtime
fields on the pobox record will be set.  The owner of the target pobox
may perform this query.  Errors: MR_USER "No such user" if login does
not match exactly one user, or MR_MACHINE.]

@multiple[@b[delete_pobox]

@u[Args]: login

@u[Returns]: none

Effectively deletes a user's pobox, by setting the type to @b(NONE).
The @i(login) name must match exactly one user.  The modtime fields on
the pobox record will be set.  The owner of the target pobox may
perform this query.  Errors: MR_USER "No such user" if login does not
match exactly one user.]


@SubSection(Machines and Clusters)
@label(Machine)
@label(Cluster)
@label(Mcmap)
@label(Svc)

@multiple[@b[get_machine]

@u[Args]: name

@u[Returns]: {name, type, modtime, modby, modwith}

Get all the information on the specified machine(s).  Wildcarding may
be used in the machine @i(name).  All machine names are case
insensitive, and are returned in uppercase.  It is safe for the query
ACL to be the list containing everybody.]

@multiple[@b[add_machine]

@u[Args]: {name, type}

@u[Returns]: none

Enters a new machine into the database.  The given @i(name) will be
converted to uppercase.  Then it will be checked for uniqueness in the
database.  The @i(type) field will be checked against the aliases
database for valid @b(mach_type)s.  Currently defined @b(mach_types)
are @b(RT) and @b(VAX).  The modtime fields will be set.  Errors:
MR_NOT_UNIQUE "Arguments not unique" if a machine with the given
@i(name) already exists, or MR_TYPE "Invalid type" if the given
@i(type) is not in the alias database.]

@multiple[@b[update_machine]

@u[Args]: {name, newname, type}

@u[Returns]: none

Update the information on a machine.  The @i(name) must match exactly
one machine.  The @i(newname) must either be the same as the old name,
or must be unique among machine names in the database after being
converted to uppercase.  The type field will be checked against the
aliases database for valid @b(mach_type)s.  The modtime fields will be
set.  Errors: MR_MACHINE "No such machine" if the old @i(name) does not
match exactly one machine, MR_NOT_UNIQUE "Arguments not unique" if
the @i(newname) does not either match the old name or is unique, or
MR_TYPE "Invalid type" if the given @i(type) is not in the alias
database.]

@multiple[@b[delete_machine]

@u[Args]: name

@u[Returns]: none

Delete a machine from the database.  The given @i(name) must match
exactly one machine.  A machine that is in use (post office, file
system, printer spooling host, server_host_access, or DCM service
update) cannot be deleted.  Errors: MR_MACHINE "No such machine" if the
@i(name) does not match exactly one machine, or MR_IN_USE "Object is
in use" if the machine is being referenced as a post office,
filesystem, spooling host, or server updated by the DCM.]

@multiple[@b[get_cluster]

@u[Args]: name

@u[Returns]: {name, description, location, modtime, modby, modwith}

Returns all the information in the database about one or more
clusters.  The cluster @i(name) may contain wildcards.  It is safe for
the query ACL to be the list containing everybody.]

@multiple[@b[add_cluster]

@u[Args]: {name, description, location}

@u[Returns]: none

Adds a new cluster to the database.  The @i(name) must be unique among
the existing cluster names.  The names are case sensitive.  There are
no constraints on the remaining data.  The modtime fields will be set.
Errors: MR_NOT_UNIQUE "Arguments not unique" if the cluster @i(name)
is not unique.]

@multiple[@b[update_cluster]

@u[Args]: {name, newname, description, location}

@u[Returns]: none

Changes the information about a cluster.  The old @i(name) must match
exactly one cluster.  The @i(newname) must either match the old name
or be unique among the existing cluster names.  The names are case
sensitive.  There are no constraints on the remaining data.  The
modtime fields will be set.  Errors: MR_CLUSTER "Unknown cluster" if
the old cluster name does not match exactly one cluster, or
MR_NOT_UNIQUE "Arguments not unique" if the new name does not either
match the old name or is unique.]

@multiple[@b[delete_cluster]

@u[Args]: name

@u[Returns]: none

Removes a cluster from the database.  The @i(name) must match exactly
one cluster.  The cluster must not have any machines assigned to it.
Any service cluster information assigned to the cluster will be
deleted.  Errors: MR_CLUSTER "Unknown cluster" if the old cluster
name does not match exactly one cluster, or MR_IN_USE "Object in use"
if the cluster has machines assigned to it.]

@multiple[@b[get_machine_to_cluster_map]

@u[Args]: {machine, cluster}

@u[Returns]: {machine, cluster}

Retrieves machine to cluster mappings for the specified @i[machine(s)]
and @i[cluster(s)].  Either of the fields may contain wildcards.  It
is safe for the query ACL to be the list containing everybody.]

@multiple[@b[add_machine_to_cluster]

@u[Args]: {machine, cluster}

@u[Returns]: none

Add a machine to a cluster.  The @i(machine) name must match exactly
one machine.  The @i(cluster) name must match exactly one cluster.
The machine's modtime fields will be updated.  Errors: MR_MACHINE "No
such machine" or MR_CLUSTER "No such cluster" if one of them does not
match exactly one object in the database.]

@multiple[@b[delete_machine_from_cluster]

@u[Args]: {machine, cluster}

@u[Returns]: none

Delete a machine from a cluster.  The @i(machine) name must match
exactly one machine.  The @i(cluster) name must match exactly one
cluster.  The named machine must belong to the named cluster.  The
machine's modtime fields will be updated.  Errors: MR_MACHINE "No
such machine" or MR_CLUSTER "No such cluster" if one of them does not
match exactly one object in the database.]

@multiple[@b[get_cluster_data]

@u[Args]: {cluster, label}

@u[Returns]: {cluster, label, data}

Retrieve all cluster data matching the named @i(cluster) and @i(label).
Either or both may use wildcards.  Thus all data for a cluster may be
retrieved with gcld(cluster, *), and all data of a particular type may
be retrieved with gcld(*, label).  It is safe for the query ACL to be
the list containing everybody.]

@multiple[@b[add_cluster_data]

@u[Args]: {cluster, label, data}

@u[Returns]: none

Add new data to a cluster.  The @i(cluster) name must match exactly
one cluster.  The service @i(label) must be a registered @b(slabel) in
the alias database.  The @i(data) is an arbitrary string.  The
cluster's modtime fields will be updated.  Errors: MR_CLUSTER "No
such cluster" if the @i(cluster) name does not match exactly one
cluster, or MR_TYPE "Invalid type" if the @i(label) is not in the
alias database.]

@multiple[@b[delete_cluster_data]

@u[Args]: {cluster, label, data}

@u[Returns]: none

Delete the specified cluster data.  The @i(cluster) name must match
exactly one cluster, and the remaining two arguments must exactly
match an existing service cluster.  The cluster's modtime fields will
be updated.  Errors: MR_CLUSTER "No such cluster" if the @i(cluster)
name does not match exactly one cluster, or MR_NOT_UNIQUE "Arguments
not unique" if the @i(label) and @i(data) do not match exactly one
existing piece of data for the cluster.]


@SubSection(Lists)
@label(Lists)
@label(Members)

@multiple[@b[get_list_info]

@u[Args]: list

@u[Returns]: {list, active, public, hidden, maillist, group, gid, 
ace_type, ace_name, description, modtime, modby, modwith}

Returns information about the named list.  The @i(list) name may
contain wildcards.  @i(active, public, hidden, maillist), and
@i(group) are booleans returned as integers (0 is false, non-zero is
true).  The @i(ace-type) is either @b(USER), @b(LIST), or @b(NONE),
and the @i(ace_name) will be either a login name, a list name, or
@b(NONE), respectively.  This query is allowed if the list is not
hidden or the user executing the query is on the ACE of the target
list.  If the user executing this query is on the query ACL, he may
use wildcards in the @i(list) name, otherwise wildcards are not
allowed.]

@multiple[@b[expand_list_names]

@u[Args]: list

@u[Returns]: {list}

Expands wildcards in a @i(list) name.  A name is passed which may
contain wildcards, and a set of matching names are returned.]

@multiple[@b[add_list]

@u[Args]: {list, active, public, hidden, maillist, group, gid,
ace_type, ace_name, description}

@u[Returns]: none

Creates a new list and adds it to the database.  The @i(list) name
must be unique among existing list names.  @i(active, public, hidden,
maillist,) and @i(group) are booleans passed as integers (0 is false,
non-zero is true).  If @i(group) is true and @i(gid) is @b(UNIQUE_GID)
as defined in @i(<mr.h>), a new unique group ID will be assigned,
otherwise the integer value given for @i(gid) will be assigned to the
GID.  The @i(ace-type) is either @b(USER), @b(LIST), or @b(NONE), and
the @i(ace_name) will be either a login name, a list name, or
@b(NONE), respectively.  The access list may be the list that is being
created (self-referential).  The list modtime will be set.  Errors:
MR_EXISTS "Record already exists" if a list already exists by that
name, MR_ACE "No such access control entity" if the @i(ace_type) is not
@b(USER), @b(LIST), or @b(NONE), or if the @i(ace_name) cannot be
resolved relative to the @i(ace_type).]

@multiple[@b[update_list]

@u[Args]: {list, newname, active, public, hidden, maillist,
group, gid, ace_type, ace_name, description}

@u[Returns]: none

Allows the list information and attributes to be changed.  This is not
equivalent to deleting the list and creating a new one, since
references to the old name will still apply to the new name if it is
renamed.  The @i(list) name must match exactly one list.  The @i(new
name) must either match the old name or be unique among list names in
the database.  The @i(active, public, hidden, maillist), and @i(group)
flags should be 0 if the flag is false, or non-zero if it is true.
The @i(gid) may be @b(UNIQUE_GID) as defined in @i(<mr.h>), in which
case a new unique GID will be assigned.  The @i(ace-type) is either
@b(USER), @b(LIST), or @b(NONE), and the @i(ace_name) will be either a
login name, a list name, or @b(NONE), respectively.  The list modtime
will be set.  This query may be executed by anyone on the ACE of the
target list.  Errors: MR_LIST "No such list" if the named list does
not match exactly one list, MR_NOT_UNIQUE "Arguments not unique" if
the new list name doesn't either match the old one or is unique,
MR_ACE "No such access control entity" if the @i(ace_type) is not
@b(USER), @b(LIST), or @b(NONE), or if the
@i(ace_name) cannot be resolved relative to the @i(ace_type).]

@multiple[@b[delete_list]

@u[Args]: list

@u[Returns]: none

Deletes a list from the database.  A list may only be deleted if it is
not in use as a member of any other list or as an ACL for an object,
and the list itself must be empty.  This query may be executed by
someone who is on the current ace of the target list.  Errors:
MR_LIST "No such list" if the named list does not exist, or
MR_IN_USE "Object is in use" if the list is still being referenced.]

@multiple[@b[add_member_to_list]

@u[Args]: {list, type, member}

@u[Returns]: none

Adds a member to a list.  The specified @i(list) must match exactly
one list.  Type must be either @b(USER), @b(LIST), or @b(STRING).
@i(member) is either a login name, a list name, or a text string,
respectively.  The modtime on the list is updated.  This query may
be executed by: anyone adding themselves as a @i(USER) to a list with the
@i(public) bit set or anyone on the ACE of the list being modified.
Errors: MR_LIST "No such list" if the named list does not exist,
MR_TYPE "Invalid type" if the member @i(type) is not @b(USER), @b(LIST), or
@b(STRING), or MR_NO_MATCH "No records in database match query" if the
@i(member) name cannot be resolved with the member @i(type).]

@multiple[@b[delete_member_from_list]

@u[Args]: {list, type, member}

@u[Returns]: none

Deletes a member from a list.  The specified @i(list) must match exactly
one list.  The specified @i(type) and @i(member) must exactly match an
existing member of that list.  The modtime on the list is updated.
This query may be executed by anyone deleting themselves as a @b(USER)
from a list with the @i(public) bit set or anyone on the ACE of the list
being modified.  Errors: MR_LIST "No such list" if the named list
does not exist, MR_TYPE "Invalid type" if the member type is not
@b(USER), @b(LIST), or @b(STRING), or MR_NO_MATCH "No records in database
match query" if the @i(member) name cannot be resolved with the member
@i(type) or if there is no such member in the list.]

@multiple[@b[get_ace_use]

@u[Args]: {ace_type, ace_name}

@u[Returns]: {object_type, object_name}

Finds references to an object as an ACE.  Valid @i(ace_types) are
@b(USER), @b(LIST), @b(RUSER), and @b(RLIST).  For types @b(USER) and
@b(RUSER), the @i(ace_name) must be a login name.  If the type is
@b(USER), then only objects whose ACE is the named user will be found;
if it is @b(RUSER), it will recursively check down sub-lists of the
ACE lists looking to see if the user is a member of that ACE.  The
types @b(LIST) and @b(RLIST) apply to a list name in a similar manner.
The returned tuples will be @b(LIST), list name; @b(SERVICE), service
name; @b(FILESYS) and a filesystem label; @b(QUERY), query name;
@b(HOSTACCESS), machine name; or @b(ZEPHYR), zephyr class name.  This
query may be executed by a user asking about himself or a person on an
ACE of a list asking about that list.  Errors: MR_TYPE "Invalid type"
if the @i(ace_type) is not one of @b(LIST), @b(RLIST), @b(USER), or
@b(RUSER); MR_NO_MATCH "No objects in database match query" if the
@i(ace_name) doesn't match a user or list.]

@multiple[@b[qualified_get_lists]

@u[Args]: {active, public, hidden, maillist, group}

@u[Returns]: {list}

Finds the names of any lists that meet the specified criteria.  Each
of the inputs may be one of @b(TRUE, FALSE), or @b(DONTCARE).  Any
user may execute this query with @i(active) @b(TRUE) and @i(hidden)
@b(FALSE).  Errors: MR_TYPE "Invalid type" if one of the arguments is
something other than @b(TRUE), @b(FALSE), or @b(DONTCARE).]

@multiple[@b[get_members_of_list]

@u[Args]: list

@u[Returns]: {type, value}

Retrieves all of the members of the named list.  The @i(list) must
match exactly one list in the database.  The returned pairs consist of
the type @b(USER), @b(LIST), or @b(STRING), followed by the login
name, list name, or text string respectively.  This query may be
executed by anyone if the list is not hidden; otherwise by someone on
the ACE of the list being modified.]

@multiple[@b[get_lists_of_member]

@u[Args]: {type, value}

@u[Returns]: {list, active, public, hidden, maillist, group}

Retrieves the name and flags of every list containing the named
member.  The member @I(type) must be one of @B(USER), @B(LIST), or
@B(STRING), and the @i(value) a login name, list name, or text string
respectively.  The @I(type) may also be one of @b(RUSER, RLIST), or
@b(RSTRING), in which case it will also find any lists that contain as
sublists a list that the target is a member of.  This query may be
executed by someone asking about themselves or a person on the ace of
a list asking about that list.  Errors: MR_TYPE "Invalid type" if the
@i(type) is not @b(USER, LIST, STRING, RUSER, RLIST), or @b(RSTRING);
or MR_NO_MATCH "No records in database match query" if @i(value) does
not match an existing value for the given type.]

@multiple[@b[count_members_of_list]

@u[Args]: list

@u[Returns]: {count}

Determines how many members are on the specified list.  The @i(list)
name must match exactly one list.  This query may be executed by a
anyone on a visible list, or someone on the ACE of the target list.
MR_LIST "Invalid list" if the list name does not match exactly one
list.]


@SubSection(Servers and Serverhosts)
@label(Servers)
@label(Serverhosts)

@multiple[@b[get_server_info]

@u[Args]: name

@u[Returns]: {service, interval, target, script, dfgen, dfcheck,
type, enable, inprogress, harderror, errmsg, ace_type, ace_name,
modtime, modby, modwith}

Retrieves service information from the database.  This is the
per-service information used by the DCM for updates.  The service
@i(name) may contain wildcards.  Note that all service names are
stored in uppercase, and the passed name will be upper-cased before
comparing it.  The returned @i(interval) is in minutes.  @i(dfgen) is
the data files were last generated, and @i(dfcheck) is the date we
last checked to see if we needed to generate them.  These are passed
as integers (unix format date).  The @i(type) must be a
@b(service-type) as stored in the aliases database.  @i(enable,
inprogress), and @i(harderror) are booleans (0 = false, non-zero =
true).  @i(ace_type) is either @b(USER, LIST), or @b(NONE), and
@i(ace_name) is a login name, a list name, or @b(NONE), respectively.
This query may be executed by someone on the service ace if only one
service is retrieved.]

@multiple[@b[qualified_get_server]

@u[Args]: {enable, inprogress, harderror}

@u[Returns]: service

Finds the names of any services that meet the specified criteria.
Each of the inputs may be one of @b(TRUE, FALSE), or @b(DONTCARE).
Errors: MR_TYPE "Invalid type" if any of the flags are not one of the
three legal values.]

@multiple[@b[add_server_info]

@u[Args]: {service, interval, target, script, type, enable, ace_type, ace_name}

@u[Returns]: none

Adds a new service to the database.  This is the per-service
information used by the DCM for updates.  Note that only a subset of
the information is added in this query, as the remaining fields are
only changed by the DCM with the @i(set_server_internal_flags) query.
The @i(service) name will be converted to uppercase.  The @i(interval)
is in minutes.  The @i(type) must be a @b(service-type) as stored in
the aliases database.  @i(Enable) is a boolean (0 = false, non-zero =
true).  @i(ace_type) is either @b(USER, LIST), or @b(NONE), and
@i(ace_name) is a login name, a list name, or @b(NONE), respectively.
The service modtime will be set.  Errors: MR_TYPE "Invalid type" if
the type field is not a valid @b(service-type) in the alias database,
or MR_ACE "No such access control entity" if the @i(ace_type) is not
@b(USER, LIST), or @b(NONE) or the @i(ace_name) cannot be resolved
based on the @i(ace_type).]

@multiple[@b[update_server_info]

@u[Args]: {service, interval, target, script, type, enable, ace_type, ace_name}

@u[Returns]: none

Updates a service in the database.  This is the per-service
information used by the DCM for updates.  Note that only a subset of
the information can be modified with this query, as the remaining
fields are only changed by Moira itself.  The @i(service) name must
match exactly one existing service after being converted to uppercase.
The @i(interval) is in minutes.  The @i(type) must be a
@b(service-type) as stored in the aliases database.  @i(Enable) is a
boolean (0 = false, non-zero = true).  @i(Ace_type) is either @b(USER,
LIST), or @b(NONE), and @i(ace_name) is a login name, a list name, or
@b(NONE), respectively.  The service modtime will be set.  This query
may be used by someone on the ACE of the target service.  Errors:
MR_TYPE "Invalid type" if the @i(type) field is not a valid
@b(service-type) in the alias database, or MR_ACE "No such access
control entity" if the @i(ace_type) is not @b(USER, LIST), or @b(NONE)
or the @i(ace_name) cannot be resolved based on the @i(ace_type).]

@multiple[@b[reset_server_error]

@u[Args]: service

@u[Returns]: none

Updates the specified service by changing the harderror flag from
@b(TRUE) to @b(FALSE), and sets @i(dfcheck) to be the same as
@i(dfgen).  The @i(service) name must match exactly on existing
service after being converted to uppercase.  The service modtime will
be set.  This query may be executed by someone on the ACE of the
target service.]

@multiple[@b[set_server_internal_flags]

@u[Args]: {service, dfgen, dfcheck, inprogress, harderr, errmsg}

@u[Returns]: none

Updates the specified service.  This is intended to only be used by
the DCM, as it changes flags that the user should not have control
over.  The @i(service) name must match exactly one existing service
after being converted to uppercase.  @i(dfgen) and @i(dfcheck) are
unix format dates (long integers).  @i(inprogress) and @i(harderr) are
booleans (0 = false, non-zero = true).  The service modtime will NOT
be set.]

@multiple[@b[delete_server_info]

@u[Args]: service

@u[Returns]: none

Deletes a set of service information from the database.  The
@i(service) name must match exactly one service in the database after
being converted to uppercase.  A service may not be deleted if there
are any server-hosts assigned to that service, or if the inprogress
bit is set for that service.  Error: MR_IN_USE "Object is in use" if
there are hosts assigned to that service.]

@multiple[@b[get_server_host_info]

@u[Args]: {service, machine}

@u[Returns]: {service, machine, enable, override, success, inprogress,
hosterror, errmsg, lasttry, lastsuccess, value1, value2, value3,
modtime, modby, modwith}

Retrieves server-host information from the database.  This is the
per-host information used by the DCM for updates.  The given
@i(service) and @i(machine) names may contain wildcards.  @i(Enable,
override, success, inprogress), and @i(hosterror) are booleans (0 =
false, non-zero = true).  @i(lasttry) and @i(lastsuccess) are unix
format dates (long integers).  This query may be executed by someone
on the ACE for the target service.]

@multiple[@b[qualified_get_server_host]

@u[Args]: {service, enable, override, success, inprogress, hosterror}

@u[Returns]: {service, machine}

Finds the names of any machine/services pairs that meet the specified
criteria.  The @i(service) name may contain wildcards.  Each of the
remaining inputs may be one of @b(TRUE, FALSE), or @b(DONTCARE).
Errors: MR_TYPE "Invalid type" if any of the flags are not one of the
three legal values.]

@multiple[@b[add_server_host_info]

@u[Args]: {service, machine, enable, value1, value2, value3}

@u[Returns]: none

Adds information for a new server-host to the database.  This is the
per-host information used by the DCM for updates.  Note that only a
subset of the information is dealt with in this query, as the
remaining fields are only changed by the DCM with the
@i(set_server_host_internal) query.  @i(Service) and @i(machine) must
each match exactly one existing service and machine, respectively.
@i(Enable) is a boolean (0 = false, non-zero = true).  The 3 values
are service specific in function; @i(value1) and @i(value2) are
integers, @i(value3) is a string.  The server-host's modtime will be
set.  This query may be used by someone on the ACE for the target
service.  Errors: MR_SERVICE "Unknown service" if the @i(service)
name does not match exactly one existing service, or MR_MACHINE "Invalid
machine" if the @i(machine) name does not match exactly one machine.]

@multiple[@b[update_server_host_info]

@u[Args]: {service, machine, enable, value1, value2, value3}

@u[Returns]: none

Updates information for a server-host in the database.  This is the
per-host information used by the DCM for updates.  Note that only a
subset of the information is dealt with in this query, as the
remaining fields are only changed by the DCM with the
@i(set_server_host_internal) query.  @i(Service) and @i(machine) must
each match exactly one existing service and machine, respectively.
@i(Enable) is a boolean (0 = false, non-zero = true).  The 3 values
are service specific in function; @i(value1) and @i(value2) are
integers, @i(value3) is a string.  The server-host's modtime will be
set.  This query may only be executed when the inprogress bit is not
currently set for the specified server_host.  This query may be used
by someone on the ACE for the target service.  Errors: MR_SERVICE
"Unknown service" if the @i(service) name does not match exactly one
existing service, or MR_MACHINE "Invalid machine" if the @i(machine)
name does not match exactly one machine.]


@multiple[@b[reset_server_host_error]

@u[Args]: {service, machine}

@u[Returns]: none

Resets the hosterr flag for the specified server_host.  The
@i(service) and @i(machine) must each match exactly one service and
host.  The server_host's modtime will be updated.  This query may be
used by someone on the ACE for the target service.  Errors:
MR_SERVICE "Unknown service" if the @i(service) name does not match
exactly one existing service, or MR_MACHINE "Invalid machine" if the
@i(machine) name does not match exactly one machine.]

@multiple[@b[set_server_host_override]

@u[Args]: {service, machine}

@u[Returns]: none

This will set the override flag for a server_host, and start a new DCM
running.  The @i(service) and @i(machine) must each match exactly one
service and host.  The server_host's modtime will be updated.  This
query may be used by someone on the ACE for the target service.
Errors: MR_SERVICE "Unknown service" if the @i(service) name does not
match exactly one existing service, or MR_MACHINE "Invalid machine" if
the @i(machine) name does not match exactly one machine.]

@multiple[@b[set_server_host_internal]

@u[Args]: {service, machine, override, success, inprogress, hosterror,
errmsg, lasttry, lastsuccess}

@u[Returns]: none

Updates the specified service_host.  This is intended to only be used
by the DCM, as it changes flags that the user should not have control
over.  The @i(service) and @i(host) names name must match exactly one
existing service and host each.  @i(override, success, inprogress) and
@i(hosterror) are booleans (0 = false, non-zero = true).  @i(lasttry)
and @i(lastsuccess) are unix format dates (long integers).  The
service_host modtime will NOT be set.  Errors: MR_SERVICE "Unknown
service" if the @i(service) name does not match exactly one existing
service, or MR_MACHINE "Invalid machine" if the @i(machine) name does
not match exactly one machine]

@multiple[@b[delete_server_host_info]

@u[Args]: {service, machine}

@u[Returns]: none

Deletes a server-host from the database.  The @i(service) and
@i(machine) names each must match exactly one existing service or
host.  A server-host may not be deleted if the inprogress bit is set
for that server-host.  This query may be used by someone on the ACE
for the target service.  Errors: MR_SERVICE "Unknown service" if the
@i(service) name does not match exactly one existing service, MR_MACH
"Invalid machine" if the @i(machine) name does not match exactly one
machine, or MR_IN_USE "Object is in use" if the inprogress bit is
set.]

@multiple[@b[get_server_locations]

@u[Args]: service

@u[Returns]: {service, machine}

This query tells which machines support a given service.  It does this
by listing each of the server-hosts for that service.  The @i(service)
name may contain wildcards, and will be converted to uppercase before
any comparisons are made.  It is safe for this query's ACL to be the
list containing everybody.]

@SubSection(Filesystems)
@label(Filesys)
@label(Nfsphys)
@label(Nfsquota)

@multiple[@b[get_filesys_by_label]

@u[Args]: name

@u[Returns]: {name, fstype, machine, packname, mountpoint, access,
comments, owner, owners, create, lockertype, modtime, modby, modwith}

Retrieves all the information about a specific filesystem from the
database.  The @i(name) may contain wildcards.  @i(fstype) is one of
@b(NFS) or @b(RVD), recorded as aliases of @b(filesys).  @i(machine)
must match exactly one existing machine.  @i(owner) must match exactly
one user, @i(owners) must match exactly one list.  @i(create) is a
boolean (0 = false, non-zero = true) indicating that the locker should
be automatically created.  @i(lockertype) is a @b(lockertype) as
recorded in the alias database, currently one of @b(SYSTEM, HOMEDIR,
PROJECT), or @b(OTHER).  The @i(packname, mountpoint), and @i(access)
vary depending on the filesystem type.]

@multiple[@b[get_filesys_by_machine]

@u[Args]: machine

@u[Returns]: {name, fstype, machine, packname, mountpoint, access,
comments, owner, owners, create, lockertype, modtime, modby, modwith}

Retrieves the information about any filesystems on the named machine.
The @i(machine) name must match exactly one machine in the database.
The returned information is as specified above for
@i(get_filesys_by_label).  Errors: MR_MACHINE "No such machine" if
the named machine does not match an existing machine.]

@multiple[@b[get_filesys_by_nfsphys]

@u[Args]: {machine, partition}

@u[Returns]: {name, fstype, machine, packname, mountpoint, access, comments,
owner, owners, create, lockertype, modtime, modby, modwith}

Retrieves the information about all NFS filesystems that reside on the
specified NFS server partition.  @i(machine) must match exactly one
machine.  @i(partition) is the mount point of the NFS physical
partition.  Errors: MR_MACHINE "Invalid machine" if the machine name
does not match exactly one machine, or MR_NO_MATCH "No records in
database match query" if the partition does not match anything.]

@multiple[@b[get_filesys_by_group]

@u[Args]: list

@u[Returns]: {name, fstype, machine, packname, mountpoint, access,
comments, owner, owners, create, lockertype, modtime, modby, modwith}

Retrieves the information about all filesystems that have the
specified group as the owners list.  The @i(list) must match exactly
one existing list.  This query may be executed by a member of the
target list.  Errors: MR_LIST "No such list" if the given list does
not match exactly one list in the database.]

@multiple[@b[add_filesys]

@u[Args]: {name, fstype, machine, packname, mountpoint,
access, comments, owner, owners, create, lockertype}

@u[Returns]: none

Adds a new filesystem to the database.  The @i(name) must be unique
among the existing filesystems.  @i(fstype) is one of @b(NFS) or
@b(RVD).  @i(machine) must match exactly one existing machine.
@i(owner) must match exactly one user, @i(owners) must match exactly
one list.  @i(create) is a boolean (0 = false, non-zero = true)
indicating that the locker should be automatically created.
@i(lockertype) is a @b(lockertype) as recorded in the alias database,
currently one of @b(SYSTEM, HOMEDIR, PROJECT), or @b(OTHER).  The
@i(packname) and @i(access) vary depending on the filesystem type.
For an RVD filesystem, they may contain anything.  For NFS
filesystems, the @i(packname) must match an existing NFS physical
filesystem, and @i(access) must be one of @b(r) or @b(w).  The
filesystem's modtime will be set.  Errors: MR_FSTYPE "Invalid filesys
type" if the @i(fstype) is not a valid @b(filesys) type, MR_TYPE
"Invalid type" if the @i(lockertype) is not a valid @b(lockertype),
MR_MACHINE "No such machine" if the @i(machine) name does not match
exactly one machine, MR_USER "No such user" if the @i(owner) does not
match exactly one user, MR_LIST "No such list" if the @i(owners) does
not match exactly one list, MR_NFS "Specified directory not exported"
if the @i(machine) and @i(packname) do not match an existing NFS
physical partition, or MR_FILESYS_ACCESS if the @i(fstype) is @b(NFS)
and the access mode is not @b(r) or @b(w).]

@multiple[@b[update_filesys]

@u[Args]: {name, newname, fstype, machine, packname, mountpoint, access,
comments, owner, owners, create, lockertype}

@u[Returns]: none

Updates the information about a filesystem in the database.  The
@i(name) must match exactly one existing filesystem.  The @i(new name)
must either match the existing one or be unique among the filesystems.
@i(fstype) is one of @b(NFS) or @b(RVD).  @i(machine) must match
exactly one existing machine.  @i(owner) must match exactly one user,
@i(owners) must match exactly one list.  @i(create) is a boolean (0 =
false, non-zero = true) indicating that the locker should be
automatically created.  @i(lockertype) is a @b(lockertype) as recorded
in the alias database, currently one of @b(SYSTEM, HOMEDIR, PROJECT),
or @b(OTHER).  The @i(packname) and @i(access) vary depending on the
filesystem type.  For an @b(RVD) filesystem, they may contain
anything.  For @b(NFS) filesystems, the @i(packname) must match an
existing NFS physical filesystem, and @i(access) must be one of @b(r)
or @b(w).  The filesystem's modtime will be updated.  Errors:
MR_NOT_UNIQUE "Arguments not unique" if the @i(new name) does not
either match the old one or is unique among filesystems, MR_FSTYPE
"Invalid filesys type" if the @i(fstype) is not a valid @b(filesys)
type, MR_TYPE "Invalid type" if the @i(lockertype) is not a valid
@b(lockertype), MR_MACHINE "No such machine" if the @i(machine) name
does not match exactly one machine, MR_USER "No such user" if the
@i(owner) does not match exactly one user, MR_LIST "No such list" if
the @i(owners) does not match exactly one list, MR_NFS "Specified
directory not exported" if the @i(machine) and @i(packname) do not
match an existing NFS physical partition, or MR_FILESYS_ACCESS if the
@i(fstype) is @b(NFS) and the @i(access) mode is not @b(r) or @b(w).]

@multiple[@b[delete_filesys]

@u[Args]: name

@u[Returns]: none

Deletes a filesystem from the database.  The @i(name) must match
exactly one existing filesystem.  Any quotas assigned to that
filesystem will be deleted, and the allocation count on the nfs
physical partition will be decremented accordingly.  Errors:
MR_FILESYS "No such file system" if the name does not match an
existing filesystem.]

@multiple[@b[get_all_nfsphys]

@u[Args]: none

@u[Returns]: {machine, dir, device, status, allocated, size, modtime, 
modby, modwith}

Retrieves information about NFS physical filesystems.  These are the
filesystems which are exported by NFS servers.]

@multiple[@b[get_nfsphys]

@u[Args]: {machine, dir}

@u[Returns]: {machine, dir, device, status, allocated, size,
modtime, modby, modwith}

Retrieves information about a specific NFS physical filesystem.  The
@i(machine) must match exactly one existing machine.  The
@i(dir)ectory name may contain wildcards.  Errors: MR_MACHINE "No
such machine" if the @i(machine) name does not match exactly one
existing machine.]

@multiple[@b[add_nfsphys]

@u[Args]: {machine, directory, device, status, allocated, size}

@u[Returns]: none

Adds a new NFS physical filesystem to the database.  The @i(machine)
name must match exactly one existing machine.  The @i(directory) and
@i(device) must be unique among existing NFS physical filesystems for
this machine.  @i(status) is an integer, with bit encodings
@b(MR_FS_STUDENT, MR_FS_FACULTY, MR_FS_STAFF), or @b(MR_FS_MISC)
as defined in @I(<mr.h>).  @i(allocated) keeps track of quota
allocation, the initial value should be zero unless there is something
besides lockers on this filesystem.  @i(size) is the actual size (in
blocks) of the filesystem.  The modtime will be set for this
filesystem.  Errors: MR_MACHINE "No such machine" if the @i(machine)
name does not match exactly one existing machine.]

@multiple[@b[update_nfsphys]

@u[Args]: {machine, directory, device, status, allocated, size}

@u[Returns]: none

Changes information about an NFS physical filesystem in the database.
The @i(machine) name must match exactly one existing machine.  The
@i(directory) must match an existing NFS physical filesystem on that
machine.  The remaining arguments will replace the current values of
those fields.  The modtime will be updated for this filesystem.
Errors: "No such machine" if the @i(machine) name does not match
exactly one existing machine.]

@multiple[@b[adjust_nfsphys_allocation]

@u[Args]: {machine, directory, delta}

@u[Returns]: none

Changes the allocation for an NFS physical filesystem.  @i(machine)
must match exactly one existing machine.  @i(directory) must match an
existing NFS physical filesystem on that machine.  The current
allocation for this filesystem will have @i(delta) (which may be
positive or negative) added to it.  Errors: MR_MACHINE "No such
machine" if the @i(machine) name does not match exactly one existing
machine.]

@multiple[@b[delete_nfsphys]

@u[Args]: {machine, directory}

@u[Returns]: none

Deletes an NFS physical filesystem from the database.  The @i(machine)
name must match exactly one existing machine.  The @i(directory) name must
match exactly one existing NFS physical filesystem on that machine.
The physical filesystem must not be in use with logical filesystems.
Errors: MR_MACHINE "No such machine" if the @i(machine) name does not
match exactly one existing machine, or MR_IN_USE "Object is in use"
if there are any filesystems assigned to this partition.]

@multiple[@b[get_nfs_quota]

@u[Args]: {filesys, login}

@u[Returns]: {filesys,login, quota, directory, machine, modtime, modby, modwith}

Retrieves the quotas assigned to the named filesystems and user.  The
@i(filesystem) name may contain wildcards.  The @i(login) name must
match exactly one user.  This query may be executed by the owner of
the target filesystem.]

@multiple[@b[get_nfs_quotas_by_partition]

@u[Args]: {machine, directory}

@u[Returns]: {filesys, login, quota, directory, machine}

Retrieves the quotas assigned to a given device.  The @i(machine) must
match exactly one existing machine.  The @i(directory) name may
contain wildcards.  Errors: MR_MACHINE "No such machine" if the
@i(machine) name does not match exactly one existing machine.]

@multiple[@b[add_nfs_quota]

@u[Args]: {filesystem, login, quota}

@u[Returns]: none

Adds a new quota to the database.  The @i(filesystem) name must match
exactly one existing filesystem.  The @i(login) name must match
exactly one existing user.  The @i(quota) may be any positive
integer.  The modtime on the quota record will be set.  The allocation
count for that NFS physical filesystem will also be updated.  Errors:
MR_FILESYS "No such file system" if the @i(filesystem) does not match
exactly one existing filesystem, or MR_USER "No such user" if the
@i(login) name does not match exactly one existing user.]

@multiple[@b[update_nfs_quota]

@u[Args]: {filesystem, login, quota}

@u[Returns]: none

Changes a quota in the database.  The @i(filesystem) name must match
exactly one existing filesystem.  The @i(login) name must match
exactly one existing user, and that user must have a quota assigned on
that filesystem.  The quota may be any positive integer, and will
replace the existing quota.  The modtime on the quota record will be
set.  The allocation count for that NFS physical filesystem will also
be updated.  Errors: MR_FILESYS "No such file system" if the
@i(filesystem) does not match exactly one existing filesystem, or
MR_USER "No such user" if the @i(login) name does not match exactly
one existing user.]

@multiple[@b[delete_nfs_quota]

@u[Args]: {filesystem, login}

@u[Returns]: none

Deletes a quota from the database.  The @i(filesystem) name must match
exactly one existing filesystem.  The @i(login) name must match
exactly one existing user, and that user must have a quota assigned on
that filesystem.  The allocation count for that NFS physical
filesystem will also be updated.  Errors: MR_FILESYS "No such file
system" if the @i(filesystem) does not match exactly one existing
filesystem, or MR_USER "No such user" if the @i(login) name does not
match exactly one existing user.]


@Subsection(Zephyr)
@label(Zephyr)

@multiple[@b[get_zephyr_class]

@u[Args]: class

@u[Returns]: {class, xmttype, xmtname, subtype, subname, iwstype, iwsname,
iuitype, iuiname, modtime, modby, modwith}

Retrieves zephyr class information from the database.  The @i(class)
name may contain wildcards.  There are four pairs of types and names:
each type is one of @b(USER, LIST), or @b(NONE), and each name is a
login name, a list name, or @b(NONE), respectively.]

@multiple[@b[add_zephyr_class]

@u[Args]: {class, xmttype, xmtname, subtype, subname, iwstype, iwsname,
iuitype, iuiname}

@u[Returns]: none

Adds a new zephyr class to the database.  The @i(class) name must be
unique among the existing class names.  There are four pairs of types
and names: each type is one of @b(USER, LIST), or @b(NONE), and each
name is a login name, a list name, or @b(NONE), respectively.  The
class's modtime will be updated.]

@multiple[@b[update_zephyr_class]

@u[Args]: {class, newclass, xmttype, xmtname, subtype, subname,
iwstype, iwsname, iuitype, iuiname}

@u[Returns]: none

Change a zephyr class in the database.  The @i(class) name must match
exactly one existing class.  The @i(new class) name must either match
the old one or be unique among the existing class names.  There are
four pairs of types and names: each type is one of @b(USER, LIST), or
@b(NONE), and each name is a login name, a list name, or @b(NONE),
respectively.  The class's modtime will be updated.]

@multiple[@b[delete_zephyr_class]

@u[Args]: class

@u[Returns]: none

Deletes a zephyr class from the database.  The @i(class) name must
match exactly one existing class.]


@SubSection(Miscellaneous)
@label(Hostaccess)
@label(Strings)
@label(Services)
@label(Printcap)
@label(Capacls)
@label(Alias)
@label(Values)
@label(Tblstats)

@multiple[@b[get_server_host_access]

@u[Args]: machine

@u[Returns]: {machine, ace_type, ace_name, modtime, modby, modwith}

Returns information about who has access to a given machine.  This
will be used to load the @i(/.klogin) file on that machine.  The
@i(machine) name may contain wildcards.  The @i(ace_type) is either
@b(USER), @b(LIST), or @b(NONE), and the @i(ace_name) is either a
login name, a list name, or @b(NONE), respectively.]

@multiple[@b[add_server_host_access]

@u[Args]: {machine, ace_type, ace_name}

@u[Returns]: none

Adds information about who has access to a given machine to the
database.  The @i(machine) name must match exactly one existing
machine.  The @i(ace_type) is either @b(USER, LIST), or @b(NONE), and
the @i(ace_name) is either a login name, a list name, or @b(NONE),
respectively.  The modtime on the record will be set.  Errors:
MR_MACHINE "No such machine" if the @i(machine) name does not match
exactly one existing machine, MR_ACE "Invalid access control entity"
if the @i(ace_type) and @i(ace_name) together do not specify a valid
entity.]

@multiple[@b[update_server_host_access]

@u[Args]: {machine, ace_type, ace_name}

@u[Returns]: none

Updates the information about who has access to a given machine.  The
@i(machine) name must match exactly one existing machine.  The
@i(ace_type) is either @b(USER, LIST), or @b(NONE), and the
@i(ace_name) is either a login name, a list name, or @b(NONE),
respectively.  The modtime on the record will be updated.  Errors:
MR_MACHINE "No such machine" if the @i(machine) name does not match
exactly one existing machine, MR_ACE "Invalid access control entity"
if the @i(ace_type) and @i(ace_name) together do not specify a valid
entity.]

@multiple[@b[delete_server_host_access]

@u[Args]: machine

@u[Returns]: none

Updates the information about who has access to a given machine.  The
@i(machine) name must match exactly one existing machine.]

@multiple[@b[add_service]

@u[Args]: {service, protocol, port, description}

@u[Returns]: none

Adds information about a new network service to the database.  The
service name must not match any existing services.  The protocol must
be listed as a "protocol" in the aliases database, currently "UDP" and
"TCP".]

@multiple[@b[delete_service]

@u[Args]: service

@u[Returns]: none

Deletes information about a network service from the database.  The
service name must match exactly one existing service.]}

@multiple[@b[get_printcap]

@u[Args]: printer

@u[Returns]: {printer, spool_host, spool_directory, rprinter,
comments, modtime, modby, modwith}

Retrieves information about a printer.  The @i(printer) name may
contain wildcards.  It is safe for this query's ACL to be the list
containing everybody.]

@multiple[@b[add_printcap]

@u[Args]: {printer, spool_host, spool_directory, rprinter, comments}

@u[Returns]: none

Adds information about a new printer to the database.  The @i(printer)
name must not match any existing printers.  @i(spool_host) must name
exactly one existing machine in the database.  The printer's modtime
will be set.  Error: MR_MACHINE if @i(spool_host) does not match
exactly one machine.]

@multiple[@b[delete_printcap]

@u[Args]: printer

@u[Returns]: none

Deletes information about a printer from the database.  The
@i(printer) name must match exactly one existing printer.]

@multiple[@b[get_alias]

@u[Args]: {name, type, translation}

@u[Returns]: {name, type, translation}

Looks up an alias in the alias database.  This database is used both
for user information like alternate names of filesystems, and keyword
validation for various queries.  Note that type validation entries are
of the form (@i{[type name, usually in lower case]}, @b(TYPE),
@i{[type string, always in upper case]}).  Some type validation
entries are used to further identify another field.  These have
entries of the form (@i{[type string in uppercase]}, @b(TYPEDATA),
@i{[type, one of: @b(none, user, list, string, machine)]}).  The
@i(name, type), and @i(translation) may contain wildcards.  It is safe
for this query to be the list containing everybody.]

@multiple[@b[add_alias]

@u[Args]: {name, type, translation}

@u[Returns]: none

Adds a new alias to the alias database.  The @i(type) must be a known
type as recorded under @b(alias) in the alias database.  Duplicate
translations for a given (name, type) pair are allowed.  Note that
type validation entries are of the form (@i{[type name, usually in
lower case]}, @b(TYPE), @i{[type string, always in upper case]}).
Some type validation entries are used to further identify another
field.  These have entries of the form (@i{[type string in
uppercase]}, @b(TYPEDATA), @i{[type, one of: @b(none, user, list,
string, machine)]}).  The @i(name, type), and @i(translation) may
contain wildcards.]

@multiple[@b[delete_alias]

@u[Args]: {name, type, translation}

@u[Returns]: none

Deletes an alias from the alias database.  The combination of all
three input arguments must match exactly one alias.]

@multiple[@b[get_value]

@u[Args]: variable

@u[Returns]: value

Look up a value in the values database.  This is used for DCM flags
and Moira internal ID hints.  The @i(variable) name must match exactly
one existing value name in the database.  It is safe for this query's
ACL to be the list containing everybody.  Errors: MR_NO_MATCH "No
records in database match query" if the name does not match exactly
one variable name.]

@multiple[@b[add_value]

@u[Args]: {variable, value}

@u[Returns]: none

Adds a new value to the values database.  The @i(variable) name must
be unique among the variables already in the database.  The @i(value)
is an integer.]

@multiple[@b[update_value]

@u[Args]: {variable, value}

@u[Returns]: none

Changes the value of an existing variable in the values database.  The
@i(variable) name must match exactly one existing variable.  Its
@i(value) will be replaced with the supplied value.  Errors:
MR_NO_MATCH "No records in database match query" if the name does not
match exactly one variable name.]

@multiple[@b[delete_value]

@u[Args]: variable

@u[Returns]: none

Deletes a variable from the values database.  The @i(variable) name
must match exactly one existing variable.  Errors: MR_NO_MATCH "No
records in database match query" if the name does not match exactly
one variable name.]

@multiple[@b[get_all_table_stats]

@u[Args]: none

@u[Returns]: {table, retrieves, appends, updates, deletes, modtime}

Retrieves a summary of the table statistics.  Each tuple consists of
the @i(table) name, how many @i(retrieves, appends, updates), and
@i(deletes) have been performed on that table, and the date of the
last change to the table.  It is safe for this query's ACL to be the
list containing everybody.]

@SubSection(Build-in Special Queries)

@multiple[@b[_help]

@u[Args]: query

@u[Returns]: help_message

Returns the short name of the query and a list of arguments and return
values.  The query name must match an existing query.  This query may
be executed by anyone.]

@multiple[@b[_list_queries]

@u[Args]: none

@u[Returns]: {long_query_name, short_query_name}

Returns a list of every query name.  This query may be executed by
anyone. ]

@multiple[@b[_list_users]

@u[Args]: none

@u[Returns]: {kerberos_principal, host_address, port_number,
connect_time, client_number}

Returns a list of every client currently using the Moira server.  This
query may be executed by anyone.]


@Section(Errors)

@Begin(Format)

General errors (may be returned by all queries):

MR_ARG_TOO_LONG - An argument contains too many characters
MR_ARGS - Incorrect number of arguments
MR_DEADLOCK - Database deadlock; try again later
MR_INGRES_ERR - An unexpected error occured in Ingres, the underlying DBMS
MR_INTERNAL - Internal consistency failure
MR_NO_HANDLE - Unknown query specified
MR_NO_MEM - Server ran out of memory
MR_PERM - Insufficient permission to perform requested database access

Any retrieval query may return

MR_NO_MATCH - No records in database match query

Any add or update query may return

MR_BAD_CHAR - Illegal character in argument
MR_EXISTS - new object conflicts with object already in the database
MR_INTEGER - String could not be parsed as an integer
MR_NO_ID - Cannot allocate new ID
MR_NOT_UNIQUE - An attempt to update more than one object at once

Any delete query may return

MR_IN_USE - Object is in use

Query specific errors:

MR_ACE - No such access control entity
MR_BAD_CLASS - Specified class is not known
MR_BAD_GROUP - Invalid group ID
MR_CLUSTER - Unknown cluster
MR_DATE - Invalid date
MR_FILESYS - Named file system does not exist
MR_FILESYS_EXISTS - Named file system already exists
MR_FILESYS_ACCESS - invalid filesys access
MR_FSTYPE - Invalid filesys type
MR_LIST - No such list
MR_MACHINE - Unknown machine
MR_NFS - specified directory not exported
MR_NFSPHYS - Machine/device pair not in nfsphys relation
MR_NO_FILESYS - Cannot find space for filesys
MR_NO_MATCH - Arguments not unique
MR_NO_POBOX - Cannot find space for pobox
MR_NO_QUOTA - No default quota specified
MR_PRINTER - Unknown printer
MR_SERVICE - Unknown service
MR_STRING - Unknown string
MR_TYPE - Invalid type
MR_USER - No such user
MR_WILDCARD - Wildcards not allowed in this case

@End(Format)


