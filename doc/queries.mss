@Comment($Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/doc/queries.mss,v 1.1 1988-05-13 14:26:19 mar Exp $)
@Device(PostScript)
@Pagefooting(center "Draft of @value(date)")
@MajorHeading(SMS Queries)

The following descriptions contain the long name of the query, the
short name, any arguments in parenthesis, then a "=>" followed by any
return values.  Zero or more sets of values may be returned.  The
three values "modtime, modby, modwith" return the date and time of the
last modification, the name of the kerberos principal who made the
modification, and the name of the program that performed the query.

If a description says nothing about access control, then it may only
be executed by people on the ACL for that query.  Most query ACLs
contain a small number of people, the "gods" for that query.  There is
also a magic list called "default" (the fact that it contains the user
named "default" is actually the magic) which is treated as if it
contained the name of every user.  This list is used as the query ACL
for some public queries.

Any query may return: SMS_PERM "Insufficient permission to perform
requested database access" or SMS_ARGS "Insufficient number of
arguments".  Any retrieval query may return SMS_NO_MATCH "No records
in database match query".  Other errors are listed with each query.


@Begin(Description, Spread 0)
@Heading(Users, Finger, and Post Office Boxes)

@multiple[@b[
get_all_logins, galo(]@i[]@b[) =>]@i[ login, uid, shell, last, first, mi]

Returns info on every account in the database.  The returned info is a
summary of the account info, not the complete information.]

@multiple[@b[
get_all_active_logins, gaal(]@i[]@b[) =>]@i[ login, uid, shell, last, first, mi]

Returns info on every account for which the status field is non-zero.
The returned info is a summary of the account info, not the complete
information.]

@multiple[@b[
get_all_active_users, gaau(]@i[]@b[)  =>]@i[ login, uid]

Returns just the login name and unix UID for every account for which
the status field is non-zero.]

@multiple[
@b[get_user_by_login, gubl(]@i[login]@b[) =>] @i[login, uid, shell, last, first, mi,
state, mitid, class, modtime, modby, modwith]

Returns complete account information on the named account.  Wildcards
may be used in the login name specified.  If the person executing the
query is not on the query ACL, then the query only succeeds if the
only retrieved information is about the user making the request.]

@multiple[@b[
get_user_by_uid, gubu(]@i[uid]@b[) =>]@i[ login, uid, shell, last, first, mi,
state, mitid, class, modtime, modby, modwith]

Returns complete account information on any account with the specified
uid.  If the person executing the query is not on the query ACL, then
the query only succeeds if the only retrieved information is about the
user making the request.]

@multiple[@b[
get_user_by_name, gubn(]@i[first, last]@b[) =>]@i[ login, uid, shell,
last, first, mi, state, mitid, class, modtime, modby, modwith]

Returns complete account information on any account with matching
first and last name fields.  Either or both names may contain
wildcards, so that this query can do the equivalent of lookup by
firstname or lookup by lastname.]

@multiple[@b[
get_user_by_class, gubc(]@i[class]@b[) =>]@i[ login, uid, shell, last,
first, mi, state, mitid, class, modtime, modby, modwith]

Returns complete account information on any account with a matching
class field.  The given class may contain wildcards.]

@multiple[@b[
get_user_by_mitid, gubm(]@i[crypt(id]@b[)) =>]@i[ login, uid, shell, last, first,
mi, state, mitid, class, modtime, modby, modwith]

Returns complete account information on any account with a matching
MIT ID field.  The given id may contain wildcards.]

@multiple[@b[
add_user, ausr(]@i[login, uid, shell, last, first, mi, state, mitid,
class]@b[) =>]@i[]

Adds a new user to the database.  The login must not match any
existing logins.  uid and state must be integers.  If the given uid is
"#", the next unused uid will be assigned.  If the given login is "#",
the login name will be a "#" followed by the uid.  For example, when
adding a person so that they may register later, the query ausr(#, #,
/bin/csh, Last, First, M, 0, [encrypted ID], class) is used.  The
class field must contain a value specified as a "TYPE" alias for
"class".  This query also initializes the finger record for this user
with just their full name, and sets their pobox to "NONE".  It updates
the modtime on the user, finger and pobox records.  Errors:
SMS_NOT_UNIQUE "Arguments not unique" if the login name is not unique,
or SMS_BAD_CLASS "Specified class is not known" if the class is not in
the alias database.]

@multiple[@b[
update_user, uusr(]@i[login, newlogin, uid, shell, last, first, mi, state,
mitid, class]@b[) =>]@i[]

Updates the info in a user entry.  login specifies the existing login
name, the remaining arguements will replace the current values of
those fields.  This is not equivalent to deleting the user and adding
a new one, as all references to this user will still exist, even if
the login name is changed.  All fields must be specified, even if the
value is to remain unchanged.  login must match exactly one user in
the database.  newlogin must either match the existing login or be
unique in the database.  The class field must contain a value
specified as a "TYPE" alias for "class".  UID and state must be
integers.  The modtime fields in the user's record will be updated.
Errors: SMS_USER "No such user" if the login name does not match
exactly one user, SMS_NOT_UNIQUE "Arguments not unique" if the new
login name is not unique, or SMS_BAD_CLASS "Specified class is not
known" if the class is not in the alias database.]

@multiple[@b[
update_user_shell, uush(]@i[login, shell]@b[) =>]@i[]

Updates a user's shell.  The specified login must match exactly one
user.  The modtime fields in the user's record will be updated.  This
query may be executed by the target user or by someone on the query
ACL.  Errors: SMS_USER "No such user" if the login name does not match
exactly one user.]

@multiple[@b[
update_user_status, uust(]@i[login, status]@b[) =>]@i[]

Updates a user's status.  The specified login must match exactly one
user.  The modtime fields in the user's record will be updated.
Errors: SMS_USER "No such user" if the login name does not match
exactly one user.]

@multiple[@b[
delete_user, dusr(]@i[login]@b[) =>]@i[]

Delete's a user record.  The specified login must match exactly one
user.  This will only be allowed if the user is not a member of any
lists or is the owner of an object.  It will also delete associated
finger information and post office box.  Errors: SMS_USER "No such
user" if the login name does not match exactly one user, SMS_IN_USE
"Object is in use" if the user is a member of a list or an ACL.]

@multiple[@b[
delete_user_by_uid, dubu(]@i[uid]@b[) =>]@i[]

Delete's a user record.  The specified UID must match exactly one
user.  This will only be allowed if the user is not a member of any
lists or is the owner of an object.  It will also delete associated
finger information and post office box.  Errors: SMS_USER "No such
user" if the uid does not match exactly one user, SMS_IN_USE "Object
is in use" if the user is a member of a list or an ACL.]

@multiple[@b[
get_finger_by_login, gfbl(]@i[login]@b[) =>]@i[ login, fullname, nicname,
home_addr, home_phone, office_addr, office_phone, department,
affiliation, modtime, modby, modwith]

Gets all of the finger information for the specified user.  The login
name must match exactly one user.  The target user may retrieve his
information.  It is safe to point the query ACL at the list of all
users.  Errors: SMS_USER "No such user" if the login name does not
match exactly one user.]

@multiple[@b[
update_finger_by_login, ufbl(]@i[login, fullname, nicname, home_addr,
home_phone, office_addr, office_phone, department, affiliation]@b[) =>]@i[]

Allows any part of the finger information to be changed for a
specified account.  The login name must match exactly one user.  The
remaining fields are free-form, and may contain anything.  The modtime
fields in the finger record will be updated.  A user may update his
own information.  Errors: SMS_USER "No such user" if the login name
does not match exactly one user.]

@multiple[@b[
get_pobox, gpob(]@i[login]@b[) =>]@i[ login, type, box]

Retrieves a user's post office box assignment.  The login name must
match exactly one user.  See set_pobox for a summary of the returned
fields.  The owner of the pobox may perform this query.  Errors:
SMS_USER "No such user" if the login name does not match exactly one
user.]

@multiple[@b[
get_all_poboxes, gapo(]@i[]@b[) =>]@i[ login, type, box]

Retrieves all of the post office boxes from the database.  See
set_pobox for a summary of the returned fields.]

@multiple[@b[
get_poboxes_pop, gpop(]@i[]@b[) =>]@i[ login, type, machine]

Retrieves all of the post office boxes of type POP from the database.
See set_pobox for a summary of the returned fields.]

@multiple[@b[
get_poboxes_smtp, gpos(]@i[]@b[) =>]@i[ login, type, box]

Retrieves all of the post office boxes of type SMTP from the database.
See set_pobox for a summary of the returned fields.]

@multiple[@b[
set_pobox, spob(]@i[login, type, box]@b[) =>]@i[]

Establishes a user's post office box.  The given login must match
exactly one user.  The type will be checked against the alias database
for valid "pobox" types.  Currently allowed types are "POP", "SMTP",
and "NONE".  If the type is POP, then box must name a machine known by
SMS.  If the type is SMTP, then box is the user's mail address with no
other interpretation by SMS.  A type of NONE is the same as not having
a pobox.  The modtime fields on the pobox record will be set.  The
owner of the target pobox may perform this query.  Errors: SMS_USER
"No such user" if the login name does not match exactly one user, or
SMS_TYPE "Invalid type" if the type is not "POP", "SMTP", or "NONE".]

@multiple[@b[
set_pobox_pop, spop(]@i[login]@b[) =>]@i[]

Forces a user's pobox to be type POP.  The login name must match
exactly one user.  If the user's pobox is already of type POP, nothing
will be changed.  If the user has previously had a pobox of type POP,
then the previous post office machine assignment will be restored.  If
there was no previous post office assignment, the query will fail with
SMS_MACHINE "Unknown machine" since it will be unable to choose a post
office machine.  The modtime fields on the pobox record will be set.
The owner of the target pobox may perform this query.  Errors:
SMS_USER "No such user" if login does not match exactly one user, or
SMS_MACHINE.]

@multiple[@b[
delete_pobox, dpob(]@i[login]@b[) =>]@i[]

Effectively deletes a user's pobox, by setting the type to "NONE".
The login name must match exactly one user.  The modtime fields on the
pobox record will be set.  The owner of the target pobox may perform
this query.  Errors: SMS_USER "No such user" if login does not match
exactly one user.]

@Heading(Machines and Clusters)

@multiple[@b[
get_machine, gmac(]@i[name]@b[) =>]@i[ name, type, modtime, modby, modwith]

Get all the information on the specified machine(s).  Wildcarding may
be used in the machine name.  All machine names are case insensitive,
and are returned in uppercase.  It is safe for the query ACL to be the
list containing everybody.]

@multiple[@b[
add_machine, amac(]@i[name, type]@b[) =>]@i[]

Enters a new machine into the database.  The given name will be
converted to uppercase.  Then it will be checked for uniqueness in the
database.  The type field will be checked against the aliases database
for valid "mach_type"s.  Currently define mach_types are "RT" and
"VAX".  The modtime fields will be set.  Errors: SMS_NOT_UNIQUE
"Arguments not unique" if a machine with the given name already
exists, or SMS_TYPE "Invalid type" if the given type is not in the
alias database.]

@multiple[@b[
update_machine, umac(]@i[name, newname, type]@b[) =>]@i[  ]

Update the information on a machine.  The name must match exactly one
machine.  The new name must either be the same as the old name, or
must be unique among machine names in the database after being
converted to uppercase.  The type field will be checked against the
aliases database for valid "mach_type"s.  The modtime fields will be
set.  Errors: SMS_MACH "No such machine" if the old name does not
match exactly one machine, SMS_NOT_UNIQUE "Arguments not unique" if
the new name does not either match the old name or is unique, or
SMS_TYPE "Invalid type" if the given type is not in the alias
database.]

@multiple[@b[
delete_machine, dmac(]@i[name]@b[) =>]@i[]

Delete a machine from the database.  The given name must match exactly
one machine.  A machine that is in use (post office, file system, or
DCM service update) cannot be deleted.  Errors: SMS_MACH "No such
machine" if the name does not match exactly one machine, or SMS_IN_USE
"Object is in use" if the machine is being referenced as a post
office, filesystem, or server updated by the DCM.]

@multiple[@b[
get_cluster, gclu(]@i[name]@b[) =>]@i[ name, description, location, modtime,
modby, modwith]

Returns all the information in the database about one or more
clusters.  The cluster name may contain wildcards.  It is safe for the
query ACL to be the list containing everybody.]

@multiple[@b[
add_cluster, aclu(]@i[name, description, location]@b[) =>]@i[]

Adds a new cluster to the database.  The name must be unique among the
existing cluster names.  The names are case sensitive.  There are no
constraints on the remaining data.  The modtime fields will be set.
Errors: SMS_NOT_UNIQUE "Arguments not unique" if the cluster name is
not unique.]

@multiple[@b[
update_cluster, uclu(]@i[name, newname, description, location]@b[) =>]@i[]

Changes the information about a cluster.  The old name must match
exactly one cluster.  The new name must either match the old name or
be unique among the existing cluster names.  The names are case
sensitive.  There are no constraints on the remaining data.  The
modtime fields will be set.  Errors: SMS_CLUSTER "Unknown cluster" if the old
cluster name does not match exactly one cluster, or SMS_NOT_UNIQUE
"Arguments not unique" if the new name does not either match the old
name or is unique.]

@multiple[@b[
delete_cluster, dclu(]@i[name]@b[) =>]@i[]

Removes a cluster from the database.  The name must match exactly one
cluster.  The cluster must not have any machines assigned to it.  Any
service cluster information assigned to the cluster will be deleted.
Errors: SMS_CLUSTER "Unknown cluster" if the old cluster name does not
match exactly one cluster, or SMS_IN_USE "Object in use" if the
cluster has machines assigned to it.]

@multiple[@b[
get_machines_to_cluster_map, gmcm(]@i[machine, cluster]@b[)
=>]@i[ machine, cluster]

Retrieves machine to cluster mappings for the specified machine(s) and
cluster(s).  Either of the fields may contain wildcards.  It is safe
for the query ACL to be the list containing everybody.]

@multiple[@b[
add_machine_to_cluster, amtc(]@i[machine, cluster]@b[) =>]@i[]

Add a machine to a cluster.  The machine name must match exactly one
machine.  The cluster name must match exactly one cluster.  The
machine's modtime fields will be updated.  Errors: SMS_MACHINE "No
such machine" or SMS_CLUSTER "No such cluster" if one of them does not
match exactly one object in the database.]

@multiple[@b[
delete_machine_from_cluster, dmfc(]@i[machine, cluster]@b[) =>]@i[]

Delete a machine from a cluster.  The machine name must match exactly
one machine.  The cluster name must match exactly one cluster.  The
named machine must belong to the named cluster.  The machine's modtime
fields will be updated.  Errors: SMS_MACHINE "No such machine" or
SMS_CLUSTER "No such cluster" if one of them does not match exactly
one object in the database.]

@multiple[@b[
get_cluster_data, gcld(]@i[cluster, label]@b[) =>]@i[ cluster, label, data]

Retrieve all cluster data matching the named cluster and label.
Either or both may use wildcards.  Thus all data for a cluster may be
retrieved with gcld(cluster, *), and all data of a particular type may
be retrieved with gcld(*, label).  It is safe for the query ACL to be
the list containing everybody.]

@multiple[@b[
add_cluster_data, acld(]@i[cluster, label, data]@b[) =>]@i[]

Add new data to a cluster.  The cluster name must match exactly one
cluster.  The service label must be a registered "slabel" in the alias
database.  The data is an arbitrary string.  The cluster's modtime
fields will be updated.  Errors: SMS_CLUSTER "No such cluster" if the
cluster name does not match exactly one cluster, or SMS_TYPE "Invalid
type" if the label is not in the alias database.]

@multiple[@b[
delete_cluster_data, dcld(]@i[cluster, label, data]@b[) =>]@i[]

Delete the specified cluster data.  The cluster name must match
exactly one cluster, and the remaining two arguments must exactly
match an existing service cluster.  The cluster's modtime fields will
be updated.  Errors: SMS_CLUSTER "No such cluster" if the cluster name
does not match exactly one cluster, or SMS_NOT_UNIQUE "Arguments not
unique" if the label and data do not match exactly one existing piece
of data for the cluster.]

@Heading(Lists)

@multiple[@b[
get_list_info, glin(]@i[list]@b[) =>]@i[ list, active, public, hidden, maillist,
group, gid, acl_type, acl_name, description, modtime, modby, modwith]

Returns information about the named list.  The list name must match
exactly one list.  Active, public, hidden, maillist, and group are
booleans returned as integers: 0 is false, non-zero is true.  The
acl-type is either "USER", "LIST", or "NONE", and the acl_name will be
either a login name, a list name, or "NONE", respectively.  This query
is allowed if the list is not hidden or the user executing the query
is on the ACL of the target list.]

@multiple[@b[
add_list, alis(]@i[list, active, public, hidden, maillist, group, gid
acl_type, acl_name, description]@b[) =>]@i[]

Creates a new list and adds it to the database.  The list name must be
unique among existing list names.  Active, public, hidden, maillist,
and group are booleans passed as integers: 0 is false, 1 is true.  If
group is true and gid is "#", a new unique group ID will be assigned,
otherwise the integer value given for gid will be assigned to the GID.
The acl-type is either "USER", "LIST", or "NONE", and the acl_name
will be either a login name, a list name, or "NONE", respectively.
The list modtime will be set.]

@multiple[@b[
update_list, ulis(]@i[list, newname, active, public, hidden, maillist,
group, gid, acl_type, acl_name, description]@b[) =>]@i[]

Allows the list information and attributes to be changed.  This is not
equivalent to deleting the list and creating a new one, since
references to the old name will still apply to the new name if it is
renamed.  The list name must match exactly one list.  The new name
must either match the old name or be unique among list names in the
database.  The active, public, hidden, maillist, and group flags
should be 0 if the flag is false, or 1 if it is true.  An error will
be generated if the list becomes a mailing list, and it contains a
user group.  The gid may be "#", in which case a new unique GID will
be assigned.  The acl-type is either "USER", "LIST", or "NONE", and
the acl_name will be either a login name, a list name, or "NONE",
respectively.  The list modtime will be set.  This query may be
executed by anyone on the ACL of the target list.]

@multiple[@b[
delete_list, dlis(]@i[list]@b[) =>]@i[]

Deletes a list from the database.  A list may only be deleted if it is
not in use as a member of any other list or as an ACL for an object,
and the list itself must be empty.  This query may be executed by
someone who is on the current acl of the target list.]

@multiple[@b[
add_member_to_list, amtl(]@i[list, type, member]@b[) =>]@i[]

Adds a member to a list.  The specified list must match exactly one
list.  Type must be either "USER", "LIST", or "STRING".  Member is
either a login name, a list name, or a text string, respectively.
Returns an error if you attempt to add a user group to a mailing list.
The modtime on the list is updated.  This query may be executed by:
anyone adding themselves as a USER to a list with the PUBLIC bit set
or anyone on the acl of the list being modified.]

@multiple[@b[
delete_member_from_list, dmfl(]@i[list, type, member]@b[) =>]@i[]

Deletes a member from a list.  The specified list must match exactly
one list.  The specified type and member must exactly match an
existing member of that list.  The modtime on the list is updated.
This query may be executed by anyone deleting themselves as a USER
from a list with the PUBLIC bit set or anyone on the acl of the list
being modified.]

@multiple[@b[
get_acl_use, gaus(]@i[acl_type, acl_name]@b[) =>]@i[ object_type, object_name]

Finds references to an object as an ACL.  Valid acl_types are "USER",
"LIST", "RUSER", and "RLIST".  For types "USER" and "RUSER", the
acl_name must be a login name.  If the type is "USER", then only
objects whose ACL is the named user will be found; if it is "RUSER",
it will recursively check down sub-lists of the ACL lists looking to
see if the user is a member of that ACL.  The types "LIST" and "RLIST"
apply to a list name in a similar manner.  The returned tuples will be
"LIST", list name; "SERVICE", service name; "FILESYS" and a filesystem
label; "QUERY", short query name; or "ZEPHYR", zephyr class name.
This query may be executed by a user asking about himself or a person
on an ACL of a list asking about that list.]

@multiple[@b[
qualified_get_lists, qgli(]@i[active, public, hidden, maillist, group]@b[) =>]@i[
list]

Finds the names of any lists that meet the specified criteria.  Each
of the inputs may be one of "TRUE", "FALSE", or "DONTCARE".  Any user
may execute this query with active TRUE and hidden FALSE.]

@multiple[@b[
get_members_of_list, gmol(]@i[list]@b[) =>]@i[ type, value]

Retrieves all of the members of the named list.  The list must match
exactly one list in the database.  The returned pairs consist of the
type "USER", "LIST", or "STRING", followed by the login name, list
name, or text string respectively.  This query may be executed by
anyone if the list is not hidden; otherwise by someone on the acl of
the list being modified.]

@multiple[@b[
get_lists_of_member, glom(]@i[type, value]@b[) =>]@i[ list]

Retrieves the name of every list containing the named member.  It will
also find any lists that contain as sublists a list that the target is
a member of.  The member type must be one of USER, LIST, or STRING,
and the value a login name, list name, or text string respectively.
This query may be executed by someone asking about themselves or a
person on the acl of a list asking about that list.]

@multiple[@b[
count_members_of_list, cmol(]@i[list]@b[) =>]@i[ count]

Determines how many members are on the specified list.  The list name
must match exactly one list.  This query may be executed by a person
on the ACL of the target list.]

@multiple[@b[
add_user_group, augr(]@i[login]@b[) =>]@i[]

Creates a user group for the specified user.  This is *ALMOST*
equivalent to alis(login, 1, 0, 0, 0, 1, #, USER, login, "User
Group").  The difference is that user groups that aren't otherwise
being used are internally stored in a more efficient manner.  This
query will create one of these more efficient lists.  If this group is
ever used in any way other than being fetched by the DCM, it will be
converted into a standard list.]

@Heading(DCM Information)

@multiple[@b[
get_server_info, gsin(]@i[name]@b[) =>]@i[ service, interval, target, script,
dfgen, type, enable, inprogress, harderror, errmsg, acl_type,
acl_name, modtime, modby, modwith]

Retrieves service information from the database.  This is the
per-service information used by the DCM for updates.  The service name
may contain wildcards.  The returned interval is in minutes.  dfgen is
the date the data files were last generated.  The type must be a
"service-type" as stored in the aliases database.  enable, inprogress,
and harderror are booleans (0=false, 1=true).  Acl_type is either
"USER", "LIST", or "NONE", and acl_name is a login name, a list name,
or "NONE", respectively.  This query may be executed by someone on the
service acl.]

@multiple[@b[
qualified_get_server, qgsv(]@i[enable, inprogress, harderror]@b[) =>]@i[ service]

Finds the names of any services that meet the specified criteria.  Each
of the inputs may be one of "TRUE", "FALSE", or "DONTCARE".]

@multiple[@b[
add_server_info, asin(]@i[service, interval, target, script, type, enable,
acl_type, acl_name]@b[) =>]@i[]

Adds a new service to the database.  This is the per-service
information used by the DCM for updates.  Note that only a subset of
the information is added in this query, as the remaining fields are
only changed by the DCM with the set_server_internal_flags query.  The
interval is in minutes.  The type must be a "service-type" as stored
in the aliases database.  Enable is a boolean (0=false, 1=true).
Acl_type is either "USER", "LIST", or "NONE", and acl_name is a login
name, a list name, or "NONE", respectively.  The service modtime will
be set.]

@multiple[@b[
update_server_info, usin(]@i[service, interval, target, script, type,
enable, acl_type, acl_name]@b[) =>]@i[]

Updates a new service to the database.  This is the per-service
information used by the DCM for updates.  Note that only a subset of
the information is added in this query, as the remaining fields are
only changed by SMS itself.  The service name must match exactly one
existing servcie.  The interval is in minutes.  The type must be a
"service-type" as stored in the aliases database.  Enable is a boolean
(0=false, 1=true).  Acl_type is either "USER", "LIST", or "NONE", and
acl_name is a login name, a list name, or "NONE", respectively.  The
service modtime will be set.  This query may be used by someone on the
ACL of the target service.]

@multiple[@b[
reset_server_error, rsve(]@i[service]@b[) =>]@i[]

Updates the specified service by changing the harderror flag from TRUE
to FALSE.  The service modtime will be set.  This query may be
executed by someone on the ACL of the target service.]

@multiple[@b[
set_server_internal_flags, ssif(]@i[service, dfgen, inprogress, harderr,
errmsg]@b[) =>]@i[]

Updates the specified service.  This is intended to only be used by
the DCM, as it changes flags that the user should not have control
over.  The service name must match exactly one existing service.
dfgen is a date.  Inprogress and harderr are booleans (0=false,
1=true).  The service modtime will NOT be set.]

@multiple[@b[
delete_server_info, dsin(]@i[service]@b[) =>]@i[]

Deletes a set of service information from the database.  The service
name must match exactly one service in the database.  A service may
not be deleted if there are any server-hosts assigned to that service,
or if the inprogress bit is set for that service.]

@multiple[@b[
get_server_host_info, gshi(]@i[service, machine]@b[) =>]@i[ service, machine,
enable, override, success, inprogress, hosterror, errmsg, lasttry,
lastsuccess, value1, value2, value3, modtime, modby, modwith]

Retrieves server-host information from the database.  This is the
per-host information used by the DCM for updates.  The given service
and machine names may contain wildcards.  Enable, override, success,
inprogress, and hosterror are booleans (1 = TRUE, 0 = FALSE).  This
query may be executed by someone on the ACL for the target service.]

@multiple[@b[
add_server_host_info, ashi(]@i[service, machine, enable, value1, value2,
value3]@b[) =>]@i[]

Adds inforation for a new server-host to the database.  This is the
per-host information used by the DCM for updates.  Note that only a
subset of the information is dealt with in this query, as the
remaining fields are only changed by the DCM with the
set_server_host_internal query.  Service and machine must each match
exactly one existing service and machine, respectively.  Enable is a
boolean (0=false, 1=true).  The 3 values are service specific in
function, value1 and value2 are integers, value3 is a string.  The
server-host's modtime will be set.  This query may be used by someone
on the ACL for the target service.]

@multiple[@b[
update_server_host_info, ushi(]@i[service, machine, enable, value1,
value2, value3]@b[) =>]@i[]

Updates inforation for a server-host in the database.  This is the
per-host information used by the DCM for updates.  Note that only a
subset of the information is dealt with in this query, as the
remaining fields are only changed by the DCM with the
set_server_host_internal query.  Service and machine must each match
exactly one existing service and machine, respectively.  Enable is a
boolean (0=false, 1=true).  The 3 values are service specific in
function, value1 and value2 are integers, value3 is a string.  The
server-host's modtime will be set.  This query may only be executed
when the inprogress bit is not currently set for the specified
server_host.  This query may be used by someone on the ACL for the
target service.]

@multiple[@b[
reset_server_host_error, rshe(]@i[service, machine]@b[) =>]@i[]

Resets the hosterr flag for the specified server_host.  The service
and machine must each match exactly one service and host.  The
server_host's modtime will be updated.  This query may be used by
someone on the ACL for the target service.]

@multiple[@b[
set_server_host_override, ssho(]@i[service, machine]@b[) =>]@i[ ]

This will set the override flag for a server_host, and start a new DCM
running.  The service and machine must each match exactly one service
and host.  The server_host's modtime will be updated.  This query may
be used by someone on the ACL for the target service.]

@multiple[@b[
set_server_host_internal, sshi(]@i[service, machine, override, success,
inprogress, hosterror, errmsg, lasttry, lastsuccess]@b[) =>]@i[]

Updates the specified service_host.  This is intended to only be used
by the DCM, as it changes flags that the user should not have control
over.  The service and host names name must match exactly one existing
service and host each.  Override, success, inprogress and hosterror
are booleans (0=false, 1=true).  Lasttry and lastsuccess are dates.
The service_host modtime will NOT be set.]

@multiple[@b[
delete_server_host_info, dshi(]@i[service, machine]@b[) =>]@i[]

Deletes a server-host from the database.  The service and host names
each must match exactly one existing service or host.  A server-host
may not be deleted if the inprogress bit is set for that server-host.
This query may be used by someone on the ACL for the target service.]

@multiple[@b[
get_server_locations, gslo(]@i[service]@b[) =>]@i[ service, machine]

This query tells which machines support a given service.  It does this
by listing each of the server-hosts for that service.  The service
name may contain wildcards.  It is safe for this query's ACL to be the
list containing everybody.]

@Heading(Filesystems)

@multiple[@b[
get_filesys_by_label, gfsl(]@i[name]@b[) =>]@i[ name, order, fstype, machine,
packname, mountpoint, access, comments, owner, owners, create,
lockertype, modtime, modby, modwith]

Retrieves all the information about a specific filesystem from the
database.  The name may contain wildcards.  Order is an integer
indicating sort order for multiple filesystems with the same name.
fstype is one of "NFS" or "RVD".  Machine must match exactly one
existing machine.  Owner must match exactly one user, owners must
match exactly one list.  Create is a boolean (0=false, 1=true)
indicating that the locker should be automatically created.
Lockertype is a "lockertype" as recorded in the alias database,
currently one of "SYSTEM", "HOMEDIR", "PROJECT", or "OTHER".  The
packname, mountpoint, and access vary depending on the filesystem
type.]

@multiple[@b[
get_filesys_by_machine, gfsm(]@i[machine]@b[) =>]@i[ name, order, fstype, machine,
packname, mountpoint, access, comments, owner, owners, create,
lockertype, modtime, modby, modwith]

Retrieves the information about any filesystems on the named machine.
The machine name must match exactly one machine in the database.  The
returned information is as specified above for get_filesys_by_label.]

@multiple[@b[
get_filesys_by_group, gfsg(]@i[list]@b[) =>]@i[ name, order, lockertype, comments]

Retrieves a summary of all filesystems that have the specified group
as the owners list.  The list must match exactly one existing list.
This query may be executed by a member of the target list.]

@multiple[@b[
add_filesys, afil(]@i[name, order, fstype, machine, packname, mountpoint,
access, comments, owner, owners, create, lockertype]@b[) =>]@i[]

Adds a new filesystem to the database.  The name must be unique among
the existing filesystems.  Order is an integer indicating sort order
for multiple filesystems with the same name.  Fstype is one of "NFS"
or "RVD".  Machine must match exactly one existing machine.  Owner
must match exactly one user, owners must match exactly one list.
Create is a boolean (0=false, 1=true) indicating that the locker
should be automatically created.  Lockertype is a "lockertype" as
recorded in the alias database, currently one of "SYSTEM", "HOMEDIR",
"PROJECT", or "OTHER".  The packname and access vary depending on the
filesystem type.  For RVD filesystem, they may contain anything.  For
NFS filesystems, the packname must match an existing NFS physical
filesystem, and access must be one of "r" or "w".  The filesystem's
modtime will be set.]

@multiple[@b[
update_filesys, ufil(]@i[name, order, newname, neworder, fstype, machine,
packname, mountpoint, access, comments, owner, owners, create,
lockertype]@b[) =>]@i[]

Updates the information about a filesystem in the database.  The name
and order must match exactly one existing filesystem.  The new name
and order must either match the existing ones or be unique among the
filesystems.  Fstype is one of "NFS" or "RVD".  Machine must match
exactly one existing machine.  Owner must match exactly one user,
owners must match exactly one list.  Create is a boolean (0=false,
1=true) indicating that the locker should be automatically created.
Lockertype is a "lockertype" as recorded in the alias database,
currently one of "SYSTEM", "HOMEDIR", "PROJECT", or "OTHER".  The
packname and access vary depending on the filesystem type.  For RVD
filesystem, they may contain anything.  For NFS filesystems, the
packname must match an existing NFS physical filesystem, and access
must be one of "r" or "w".  The filesystem's modtime will be updated.]

@multiple[@b[
delete_filesys, dfil(]@i[name, order]@b[) =>]@i[]

Deletes a filesystem from the database.  The name and order together
must match exactly one existing filesystem.]

@multiple[@b[
add_locker, aloc(]@i[login, machine, device, quota]@b[) =>]@i[]

This is an optimized query to add a filesystem and a quota.  It
retrieves the NFS physical information to determine where the named
device mounts.  Then it performs: add_filesys([login], 0, NFS,
[machine], [device-mount-point]/[login], /mit/[login], w, "User's
Locker", [login], [login], 1, HOMEDIR); and add_nfs_quota([machine],
[device], [login], [quota]).  It will also increment the allocated
quota count for the named NFS fileserver.]

@multiple[@b[
delete_locker, dloc(]@i[login]@b[) =>]@i[]

This is an optimized query to delete a filesystem and a quota.  It
will find out which machine and device the filesystem whose name
matches the login name resides on, and delete any quotas for this user
on that device.  Then it will delete the filesystem itself.  It will
also decrement the allocated quota count for the named NFS fileserver.]

@multiple[@b[
get_all_nfsphys, ganf(]@i[]@b[) =>]@i[ machine, device, dir, status, allocated,
size, modtime, modby, modwith]

Retrieves information about NFS physical filesystems.  These are the
filesystems which are exported by NFS servers.]

@multiple[@b[
get_nfsphys, gnfp(]@i[machine, device]@b[) =>]@i[ dir, status, allocated, size,
modtime, modby, modwith]

Retrieves information about a specific NFS physical filesystem.  The
machine must match exactly one existing machine.  The device name may
contain wildcards.]

@multiple[@b[
add_nfsphys, anfp(]@i[machine, device, directory, status, allocated, size]@b[) =>]@i[]

Adds a new NFS physical filesystem to the database.  The machine name
must match exactly one existing machine.  The device and directory
must be unique among existing NFS phsyical filesystems for this
machine.  Status is an integer, with bits encoding: ???.  The
allocation keeps track of quota allocation, the initial value should
be zero unless there is something besides lockers on this filesystem.
Size is the actual size (in blocks) of the filesystem.  The modtime
will be set for this filesystem.]

@multiple[@b[
update_nfsphys, unfp(]@i[machine, device, directory, status, allocated, size]@b[) =>]@i[]

Changes information about an NFS physical filesystem in the database.
The machine name must match exactly one existing machine.  The device
must match an existing NFS physical filesystem on that machine.  The
remaining arguments will replace the current values of those fields.
The modtime wil be updated for this filesystem.]

@multiple[@b[
adjust_nfsphys_allocation, ajnf(]@i[machine, device, delta]@b[) =>]@i[ allocation]

Changes the allocation for an NFS physical filesystem.  Machine must
match exactly one existing machine.  Device must match an existing NFS
physical filesystem on that machine.  The current allocation for this
filesystem will have delta (which may be positive or negative) added
to it.  The new allocation is returned.]

@multiple[@b[
delete_nfsphys, dnfp(]@i[machine, device]@b[) =>]@i[]

Deletes an NFS physical filesystem from the database.  The machine
name must match exactly one existing machine.  The device name must
match exactly one existing NFS physical filesystem on that machine.
The physical filesystem must not be in use with logical filesystems.]

@multiple[@b[
get_nfs_quotas, gnfq(]@i[machine, device]@b[) =>]@i[ login, quota]

Retrieves the quotas assigned to a given device.  The machine must
match exactly one existing machine.  The device name may contain
wildcards (although they are not useful since the device names are not
returned with the other information).]

@multiple[@b[
get_nfs_quotas_by_user, gnqu(]@i[login]@b[) =>]@i[ machine, device, quota]

Retrieves the quotas assigned to a particular user.  This query may be
executed by the target user.]

@multiple[@b[
add_nfs_quota, anfq(]@i[machine, device, login, quota]@b[) =>]@i[]

Adds a new quota to the database.  The machine name must match exactly
one existing machine.  The device name must match exactly one existing
NFS physical filesystem on that machine.  The login name must match
exactly one existing user.  The quota may be any postitive integer.
The allocation count for that NFS physical filesystem will also be
updated.]

@multiple[@b[
update_nfs_quota, unfq(]@i[machine, device, login, quota]@b[) =>]@i[]

Changes a quota in the database.  The machine name must match exactly
one existing machine.  The device name must match exactly one existing
NFS physical filesystem on that machine.  The login name must match
exactly one existing user, and that user must have a quota assigned on
that filesystem.  The quota may be any postitive integer, and will
replace the existing quota.  The allocation count for that NFS
physical filesystem will also be updated.]

@multiple[@b[
delete_nfs_quota, dnfq(]@i[machine, device, login]@b[) =>]@i[]

Deletes a quota from the database.  The machine name must match
exactly one existing machine.  The device name must match exactly one
existing NFS physical filesystem on that machine.  The login name must
match exactly one existing user, and that user must have a quota
assigned on that filesystem.  The allocation count for that NFS
physical filesystem will also be updated.]

@Heading(Zephyr)

@multiple[@b[
get_zephyr_class, gzcl(]@i[class]@b[) =>]@i[ class, xmttype, xmtname, subtype,
subname, iwstype, iwsname, iuitype, iuiname, modtime, modby, modwith]

Retrieves zephyr class information from the database.  The class name
may contain wildcards.  There are four pairs of types and names: each
type is one of "USER", "LIST", or "NONE", and each name is a login
name, a list name, or "NONE", respectively.]

@multiple[@b[
add_zephyr_class, azcl(]@i[class, xmttype, xmtname, subtype, subname,
iwstype, iwsname, iuitype, iuiname]@b[) =>]@i[]

Adds a new zephyr class to the database.  The class name must be
unique among the existing class names.  There are four pairs of types
and names: each type is one of "USER", "LIST", or "NONE", and each
name is a login name, a list name, or "NONE", respectively.  The
class's modtime will be updated.]

@multiple[@b[
update_zephyr_class, uzcl(]@i[class, newclass, xmttype, xmtname, subtype,
subname, iwstype, iwsname, iuitype, iuiname]@b[) =>]@i[]

Change a zephyr class in the database.  The class name must match
exacly one existing class.  The new class name must either match the
old one or be unique among the existing class names.  There are four
pairs of types and names: each type is one of "USER", "LIST", or
"NONE", and each name is a login name, a list name, or "NONE",
respectively.  The class's modtime will be updated.]

@multiple[@b[
delete_zephyr_class, dzcl(]@i[class]@b[) =>]@i[]

Deletes a zephyr class from the database.  The class name must match
exactly one existing class.]

@Heading(Miscellaneous)

@multiple[@b[
get_server_host_access, gsha(]@i[machine]@b[) =>]@i[ machine, acl_type, acl_name]

Returns information about who has access to a given machine.  The
machine name may contain wildcards.  The acl type is either "USER" or
"LIST", and the acl name is either a login name or a list name,
respectively.]

@multiple[@b[
add_server_host_access, usha(]@i[machine, acl_type, acl_name]@b[) =>]@i[]

Adds information about who has access to a given machine to the
database.  The machine name must match exactly one existing machine.
The acl type is either "USER" or "LIST", and the acl name is either a
login name or a list name, respectively.]

@multiple[@b[
update_server_host_access, usha(]@i[machine, acl_type, acl_name]@b[) =>]@i[]

Updates the information about who has access to a given machine.  The
machine name must match exactly one existing machine.  The acl type is
either "USER" or "LIST", and the acl name is either a login name or a
list name, respectively.]

@multiple[@b[
get_service, gsvc(]@i[service]@b[) =>]@i[ service, protocol, port, description]

Retrieves information about a network service.  The service name may
contain wildcards.  It is safe for this query's ACL to be the list
containing everybody.]

@multiple[@b[
add_service, asvc(]@i[service, protocol, port, description]@b[) =>]@i[]

Adds information about a new network service to the database.  The
service name must not match any existing services.  The protocol must
be listed as a "protocol" in the aliases database, currently "UDP" and
"TCP".]

@multiple[@b[
delete_service, dsvc(]@i[service]@b[) =>]@i[]

Deletes information about a network service from the database.  The
service name must match exactly one existing service.]

@multiple[@b[
get_printcap, gpcp(]@i[printer]@b[) =>]@i[ printer, printcap]

Retrieves information about a printer.  The printer name may contain
wildcards.  It is safe for this query's ACL to be the list containing
everybody.]

@multiple[@b[
add_printcap, apcp(]@i[printer, printcap]@b[) =>]@i[]

Adds information about a new printer to the database.  The printer
name must not match any existing printers.]

@multiple[@b[
delete_printcap, dpcp(]@i[printer]@b[) =>]@i[]

Deletes information about a printer from the database.  The printer
name must match exactly one existing printer.]

@multiple[@b[
get_alias, gali(]@i[name, type]@b[) =>]@i[ name, type, translation]

Looks up an alias in the alias database.  This database is used both
for user information like alternate names of filesystems, and keyword
validation for various queries.  The name and type may contain
wildcards.  It is safe for this query to be the list containing
everybody.]

@multiple[@b[
add_alias, aali(]@i[name, type, translation]@b[) =>]@i[]

Adds a new alias to the alias database.  The type must be a known type
as recorded under "alias" in the alias database.  Duplicate
translations for a given (name, type) pair are allowed.]

@multiple[@b[
delete_alias, dali(]@i[name, type, translation]@b[) =>]@i[]

Deletes an alias from the alias database.  The combination of all
three input arguments must match exactly one alias.]

@multiple[@b[
get_value, gval(]@i[variable]@b[) =>]@i[ value]

Look up a value in the values database.  This is used for DCM flags
and sms internal ID hints.  The variable name may contain wildcards.
It is safe for this query's ACL to be the list containing everybody.]

@multiple[@b[
add_value, aval(]@i[variable, value]@b[) =>]@i[]

Adds a new value to the values database.  The variable name must be
unique among the variables already in the database.  The value is an
integer.]

@multiple[@b[
update_value, uval(]@i[variable, value]@b[) =>]@i[]

Changes the value of an existing variable in the values database.  The
variable name must match exactly one existing variable.  Its value
will be replaced with the supplied value.]

@multiple[@b[
delete_value, dval(]@i[variable]@b[) =>]@i[]

Deletes a variable from the values database.  The variable name must
match exactly one existing variable.]

@multiple[@b[
get_all_table_stats, gats(]@i[]@b[) =>]@i[ table, retrieves, appends, updates,
deletes, modtime]

Retrieves a summary of the table statistics.  Each tuple consists of
the table name, how many retrieves, appends, updates, and deletes have
been performed on that table, and the date of the last change to the
table.  It is safe for this query's ACL to be the list containing
everybody.]

@Heading(Build-in Special Queries)

@multiple[@b[
_help(]@i[query]@b[) =>]@i[ help_message]

Returns the short name of the query and a list of arguments and return
values.  The query name must match an existing query.  This query may
be executed by anyone.]

@multiple[@b[
_list_queries(]@i[]@b[) =>]@i[ long_query_name, short_query_name]

Returns a list of every query name.  This query may be executed by
anyone. ]

@multiple[@b[
_list_users(]@i[]@b[) =>]@i[ user_info]

Returns a list of every client currently using the SMS server.  This
query may be executed by anyone.]

@end(description)
