@part[tools, root "moira.mss"]
@Define(TItemize=Itemize, LeftMargin +2, Indent -2, RightMargin 0, Spread 0)
@Define(TVerbatim=Verbatim, LeftMargin 0, RightMargin 0, Group, Size -1)
@NewPage()
@MajorSection(Specialized Management Tools - User Interface)
@label(Tools)

Moira will include a set of specialized management tools to enable system
administrators to control system resources.  As the system evolves, more
management tools will become a part of the Moira's application program
library.  These tools provide the fundamental administrative use of Moira.

In response to complaints about the user interface of current database
maintenance tools such as madm, gadm, and (to a lesser extent)
register, the Moira tools will use a slightly different strategy.  To
accommodate novice and occasional users, a menu interface similar to
the interface in register will be the default.  For regular users, a
command-line switch (such as @t(-nomenu)) will be provided that will
use a line-oriented interface such as those in discuss and kermit.
This should provide speed and directness for users familiar with the
system, while being reasonably helpful to novices and occasional
users.  A specialized menu building tool has been developed in order
that new application programs can be developed quickly.  An X
interface is being planned, but is of secondary importance to the
functioning of the base system.

Fields in the database will have associated with them lists of legal
values.  A null list will indicate that any value is possible.  This is
useful for fields such as user_name, address, and so forth.  The
application programs will, before attempting to modify anything in the
database, will request this information, and compare it with the
proposed new value.  If an invalid value is discovered, it will be
reported to the user, who will be given the opportunity to change the
value, or "insist" that it is a new, legal value.  (The ability to
update data in the database will not necessarily indicate the ability to
add new legal values to the database.)

Applications should be aware of the ramifications of their actions, and
notify the user if appropriate.  For example, an administrator deleting
a user should be informed of storage space that is being reclaimed,
mailing lists that are being modified.  Objects that need to be modified
at once (such as the ownership of a mailing list) should present
themselves to be dealt with.

The following list of programs will be found on subsequent pages:
@begin(itemize, spread 0)
BLANCHE - batch list maintanence tool

CHFN - change finger information

CHPOBOX - change forwarding post office

CHSH - change default shell

REG_TAPE - Registrar's tape entry program

MOIRA - administrative client

MRCHECK - verify that updates have been successful

USERREG - New user registration.
@end(itemize)

@i[For clarity, each new program begins on a new page.]
@Begin(Description, LeftMargin +18, Indent -18)
@NewPage()

PROGRAM NAME:@\BLANCHE - Batch list operations

DESCRIPTION:@\This program allows one to examine a list, and to
examine or modify the membership of a list.  Rather than using menus
as the other Moira clients, it takes initial command line arguments, and
uses standard input and standard output.

PRE-DEFINED QUERIES USED:@\@Begin(TItemize)

get_list_info

count_members_of_list

get_members_of_list

add_member_to_list

delete_member_from_list

@End(TItemize)

Manipulates the following fields:@\@Begin(TItemize)
(name, active, public, hidden, maillist, group, gid, desc, acl_type,
acl_id, modtime, modby, modwith) - LIST relation

(list_id, member_type, member_id) - MEMBERS relation
@End(TItemize)

SUPPORTED SERVICE(S):@\@Begin(TItemize)
Mailing lists

Unix groups

Moira access control lists
@End(TItemize)

END USER:@\List maintainers

A SESSION USING BLANCHE:@\@Begin(TVerbatim)
% blanche -info mar
List: mar
Description: User Group
Flags: active, private, and hidden
mar is a maillist and is a group with GID 5271
Owner: LIST mar
Last modified by mar with blanche on 14-sep-1988 14:03:20
% blanche -a carla mar -m
carla
mar
% 
@End(TVerbatim)
@NewPage()

PROGRAM NAME:@\CHFN - Finger Information.

DESCRIPTION:@\This program allows users to change their 
finger information.  This is the information put into the password
lines stored in hesiod and the master password file.  It should be
functionally equivalent to the standard Berkeley Unix chfn program.

PRE-DEFINED QUERIES USED:@\@Begin(TItemize)

get_finger_by_login

update_finger_by_login
@End(TItemize)

Manipulates the following fields:@\
@Begin(TItemize)
(fullname, nickname, home_address, home_phone, office_phone, department,
year) - FINGER relation
@End(TItemize)

SUPPORTED SERVICE(S):@\
@Begin(TItemize)
User Community - finger
@End(TItemize)

END USER:@\All.

A SESSION USING CHFN:
@Begin(TVerbatim)
% chfn

Changing finger information for pjlevine.
Info last changed on 22-mar-1988 15:13:33 by user pjlevine using chfn
Default values are printed inside of of '[]'.
To accept the default, type <return>.
To have a blank entry, type the word 'none'.

Full name [Peter Levine]: 

Nickname [Pete]: 
Home address (Ex: Bemis 304) [24 kilsyth rd Brookline]: 
Home phone number (Ex: 4660000) [1234567]: 
Office address (Exs: 597 Tech Square or 10-256) [E40-342a]: 
Office phone (Ex: 3-1300) [0000]: 
MIT department (Exs: EECS, Biology, Information Services) []: 
MIT year (Exs: 1989, '91, Faculty, Grad) [staf]: 
% 
@End(TVerbatim)

@NewPage()

PROGRAM NAME:@\CHPOBOX - View / change home mail host.

DESCRIPTION:@\The name service and a mail forwarding service need to
know where a user's post office is.  This program allows the user the
capability to forward his mail to a different machine.  This program
is a command line interface.  It will report a user's current mailbox.
It can also set a user's mailbox.

PRE-DEFINED QUERIES USED:@\@Begin(TItemize)

get_pobox

set_pobox

set_pobox_pop

get_server_locations

@End(TItemize)

Manipulates the following fields:@\@Begin(TItemize)
(potype, pop_id, box_id) - USER relation

(string_id, string) - STRINGS relation
@End(TItemize)

SUPPORTED SERVICE(S):@\@Begin(TItemize)
Mail forwarding

Mail reading
@End(TItemize)

END USERS:@\All.

A SESSION USING CHPOBOX:
@Begin(TVerbatim)

% chpobox
User mar, Type POP, Box: mar@@E40-PO.MIT.EDU
  Modified by mar on 06-oct-1988 17:43:35 with moira
%
% chpobox -s mar@@xx.lcs.mit.edu
User mar, Type SMTP, Box: mar@@xx.lcs.mit.edu
  Modified by mar on 18-oct-1988 17:51:32 with chpobox
%
% chpobox -p
User mar, Type POP, Box: mar@@E40-PO.MIT.EDU
  Modified by mar on 18-oct-1988 17:52:25 with chpobox
%
@end(TVerbatim)

@NewPage()

PROGRAM NAME:@\CHSH - Change shell.

DESCRIPTION:@\This program allows users to change their default shell.

PRE-DEFINED QUERIES USED:@\@Begin(TItemize)
get_user_by_login

update_user_shell
@End(TItemize)

Manipulates the following fields:@\@Begin(TItemize)
(shell) - USER relation
@End(TItemize)

SUPPORTED SERVICE(S):@\
@Begin(TItemize)
login
@End(TItemize)

END USERS:@\All

A SESSION USING CHSH:
@Begin(TVerbatim)
% chsh

Changing login shell for pjlevine.
Account info last changed on 02-sep-1988 13:56:03 by user pjlevine using moira
Current shell for pjlevine is /bin/csh
New shell: /bin/csh
Changing shell to /bin/csh
%
@End(TVerbatim)

@newpage()

PROGRAM NAME:@\DBCK - Database consistency checker

DESCRIPTION:@\This program verifies the internal consistency of the
database.  It verifies that there are no duplicates of supposedly
unique data, that all references to other objects are references which
actually exist, there are no unused objects, and that the counts of
quotas and poboxes are correct.  This is written in the spirit of the
unix filesystem checker, @i(fsck).

PRE-DEFINED QUERIES USED:@\None.  It access the database directly.

END USERS:@\Database administrator

A SESSION USING DBCK:
@Begin(TVerbatim)
% dbck
Opening database sms...done
Phase 1 - Looking for duplicates
Phase 2 - Checking references
Phase 3 - Finding unused objects
Warning: List saltzer is empty
Warning: List bug-rt is empty
Warning: List athena-bug-scribe is empty
Warning: List ccref is empty
Warning: List rparmelee is empty
Warning: List alens-testers is empty
Unreferenced string mar@@xx.lcs.mit.edu id 77
Delete (Y/N/Q)? y
1 entry deleted
Unreferenced string a random string (with * wildcards) id 76
Delete (Y/N/Q)? y
1 entry deleted
Phase 4 - Checking counts
Done.
%
@End(TVerbatim)

@newpage()

PROGRAM NAME:@\REG_TAPE - Add or remove students from the system using
Registrar's tape.

DESCRIPTION:@\@Multiple[Each term, when the Registrar releases a tape of
current students, the system administrator must load the names of new users
and delete all old users.  This program will automatically use the
Registrar's tape as a means of keeping current the Moira database.

The problem of deleting users is a sensitive issue.  The removal of a user
will reflect this sensitivity.  When deleting a user, the expiration
date field will be set to the current date, but the user will not be
removed.  The program db_maint will, among other things, check the
expiration stamp of the users.  If a stamp is within critical expiration
time, the program will notify the administrator that a time-to-live date
has been reached.  If correct, the administrator will set the user's
status field to INACTIVE and set the time to some date in the future.
When that date and INACTIVE status are reached, the user is flushed.  If
incorrect, the administrator will set the date to some time in the
future and leave the status field ACTIVE.
]

PRE-DEFINED QUERIES USED:@\
@Begin(TItemize)

update_user

update_user_status

@End(TItemize)

Manipulates the following fields:@\
@Begin(TItemize)
(status, expdate) - USERS relation.
@End(TItemize)

SUPPORTED SERVICE(S):@\
@Begin(TItemize)
Moira
@End(TItemize)

END USERS:@\Administrator.

@NewPage()
PROGRAM NAME:@\Moira - Moira client interface.

DESCRIPTION:@\This is the primary Moira client.  It is capable of
examining and modifying all fields in the database.  It is menu based,
using the curses library.  For backwards compatability with older
clients, it will look at the name it is invoked with, and start in a
sub-menu if it recognizes a menu name.

SUPPORTED SERVICES:@\@Begin(TItemize)
All
@End(TItemize)

END USER:@\@Begin(TItemize)
All

Operations

System administrators

User accounts administrator
@End(TItemize)

MENUS:

@Begin(Description, LeftMargin +1)@Begin(Group)

MENU:@\Top Level Menu

DESCRIPTION:@\This is the main menu of the program.  It lists sub
menus for each of the main types of data that Moira handles.

DISPLAY:@Begin(TVerbatim)
                           Moira Database Manipulation
 1. (cluster)      Cluster Menu.
 2. (filesys)      Filesystem Menu.
 3. (list)         Lists and Group Menu.
 4. (machine)      Machine Menu.
 5. (nfs)          NFS Physical Menu.
 6. (user)         User Menu.
 7. (printer)      Printer Menu.
 8. (dcm)          DCM Menu.
 9. (misc)         Miscellaneous Menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.
@End(TVerbatim)

QUERIES USED:@\@Begin(Description, spread 0)
@End(Description)
@Hinge()
MENU:@\Cluster Menu

DESCRIPTION:@\This menu allows the manipulation of clusters.  It also
has sub-menus to allow the user to examine machine to cluster mappings
and cluster data.

DISPLAY:@Begin(TVerbatim)
                                  Cluster Menu
 1. (show)         Get cluster information.
 2. (add)          Add a new cluster.
 3. (update)       Update cluster information.
 4. (delete)       Delete this cluster.
 5. (mappings)     Machine To Cluster Mappings Menu.
 6. (c_data)       Cluster Data Menu.
 7. (verbose)      Toggle Verbosity of Delete.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.
@End(TVerbatim)

QUERIES USED:@\@Begin(Description, spread 0)
show@\get_cluster

add@\get_cluster, add_cluster

update@\get_cluster, update_cluster

delete@\get_cluster, delete_cluster
@End(Description)
@Hinge()
MENU:@\Filesystem Menu

DESCRIPTION:@\This menu allows the manipulation of filesystems.  This
includes both the filesystem themselves and aliases for filesystems.
It also includes a sub-menu for manipulation of quotas.

DISPLAY:@Begin(TVerbatim)
                                Filesystem Menu
 1. (get)          Get Filesystem Name Information.
 2. (add)          Add New Filesystem to Database.
 3. (change)       Update Filesystem Information.
 4. (delete)       Delete Filesystem.
 5. (check)        Check An Association.
 6. (alias)        Associate with a Filesystem.
 7. (unalias)      Disassociate from a Filesystem.
 8. (quotas)       Quota Menu.
 9. (verbose)      Toggle Verbosity of Delete.
10. (help)         Help ...
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.
@End(TVerbatim)

QUERIES USED:@\@Begin(Description, spread 0)
get@\get_filesys_by_label

add@\get_filesys_by_label, add_filesys

change@\get_filesys_by_label, update_filesys

delete@\get_filesys_by_label, delete_filesys

check@\get_alias

alias@\get_alias, add_alias

unalias@\get_alias, delete_alias
@End(Description)
@Hinge()
MENU:@\List Menu

DESCRIPTION:@\This menu allows the manipulation of lists, including
retrieval by name, creation, deletion, and updating the
characteristics.  Note that deleting a list will double-check
everything to maintain database consistency, and may prompt the user
to take further actions.  There is also an option called query_remove
which will find all of the lists a member belongs to, and ask the user
one at a time which ones the member should be removed from.  There are
also sub-menus to manipulate the members of a list and to search
though lists.

DISPLAY:@Begin(TVerbatim)
                                   List Menu
 1. (show)         Display information about a list.
 2. (add)          Create new List.
 3. (update)       Update characteristics of a list.
 4. (delete)       Delete a List.
 5. (query_remove) Interactively remove an item from all lists.
 6. (members)      Member Menu - Change/Show Members of a List..
 7. (list_info)    List Info Menu.
 8. (quotas)       Quota Menu.
 9. (verbose)      Toggle Verbosity of Delete.
10. (help)         Print Help.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.
@End(TVerbatim)

QUERIES USED:@\@Begin(Description, spread 0)
show@\get_list_info

add@\get_list_info, add_list

update@\get_list_info, update_list

delete@\get_list_info, get_ace_use, count_members_of_list,
get_lists_of_member, get_members_of_list, delete_list,
delete_member_from_list 

query_remove@\get_lists_of_member, delete_member_from_list
@End(Description)
@Hinge()
MENU:@\Machine Menu

DESCRIPTION:@\This allows for machines to be manipulated.  It includes
a sub-menu for machine-to-cluster mapping.

DISPLAY:@Begin(TVerbatim)
                                  Machine Menu
 1. (show)         Get machine information.
 2. (add)          Add a new machine.
 3. (update)       Update machine information.
 4. (delete)       Delete this machine.
 5. (mappings)     Machine To Cluster Mappings Menu.
 6. (verbose)      Toggle Verbosity of Delete.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.
@End(TVerbatim)

QUERIES USED:@\@Begin(Description, spread 0)
show@\get_machine

add@\get_machine, add_machine

update@\get_machine, update_machine

delete@\get_machine, delete_machine
@End(Description)
@Hinge()
MENU:@\NFS Physical Menu

DESCRIPTION:@\This allows for NFS physical filesystems to be
manipulated.  It includes a submenu for quota manipulation as well.

DISPLAY:@Begin(TVerbatim)
                               NFS Physical Menu
 1. (show)         Show an NFS server.
 2. (add)          Add NFS server.
 3. (update)       Update NFS server.
 4. (delete)       Delete NFS server.
 5. (quotas)       Quota Menu.
 6. (verbose)      Toggle Verbosity of Delete.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.
@End(TVerbatim)

QUERIES USED:@\@Begin(Description, spread 0)
show@\get_nfsphys

add@\get_nfsphys, add_nfsphys

update@\get_nfsphys, update_nfsphys

delete@\get_nfsphys, get_filesys_by_nfsphys, delete_nfsphys
@End(Description)
@Hinge()
MENU:@\User Menu

DESCRIPTION:@\This allows for user accounts to be manipulated.  Lookup
may be done by login name, real name, or class.  Modifications may be
of all fields, or a simple deactivate (changing the account status to
indicate ``marked for deletion''.  Registering a user consists of
changing the login name and status, and creating a user group,
filesystem, and pobox.  To expunge a user is to actually delete all
record of them from the database (including prompting to delete their
filesystem and user group).  Sub-menus for manipulation of poboxes and
quotas are also provided.

DISPLAY:@Begin(TVerbatim)
                                   User Menu
 1. (login)        Show user information by login name.
 2. (name)         Show user information by name.
 3. (class)        Show names of users in a given class.
 4. (modify)       Change all user fields.
 5. (adduser)      Add a new user to the database.
 6. (register)     Register a user.
 7. (deactivate)   Deactivate user.
 8. (expunge)      Expunge user.
 9. (pobox)        Post Office Box Menu.
10. (quota)        Quota Menu.
11. (verbose)      Toggle Verbosity of Delete.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.
@End(TVerbatim)

QUERIES USED:@\@Begin(Description, spread 0)
login@\get_user_by_login

name@\get_user_by_name

class@\get_user_by_class

modify@\get_user_by_login, update_user

adduser@\get_user_by_name, add_user

register@\get_user_by_name, register_user

deactivate@\get_user_by_name, update_user_status

expunge@\delete_user, get_filesys_by_label, delete_filesys,
get_members-of-list, delete_list, count_members_of_list,
get_lists_of_member, delete_member_from_list, get_list_info,
get_ace_use
@End(Description)
@Hinge()
MENU:@\Printer Menu

DESCRIPTION:@\This allows printcap entries to be manipulated.

DISPLAY:@Begin(TVerbatim)
                                  Printer Menu
 1. (get)          Get Printcap Entry Information.
 2. (add)          Add New Printcap Entry to Database.
 3. (change)       Update Printer Information.
 4. (delete)       Delete Printcap Entry.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.
@End(TVerbatim)

QUERIES USED:@\@Begin(Description, spread 0)
get@\get_printcap

add@\get_printcap, add_printcap

change@\get_printcap, delete_printcap, add_printcap

delete@\get_printcap, delete_printcap
@End(Description)
@Hinge()
MENU:@\DCM Menu

DESCRIPTION:@\This menu allows for the DCM and it's control
information to be manipulated.  Options include enabling the DCM,
getting the current status of updates, and starting an update
immediately.  Sub-menus exist for manipulating service information and
host/service tuple information.

DISPLAY:@Begin(TVerbatim)
                                    DCM Menu
 1. (enable)       Enable/disable DCM.
 2. (service)      DCM Service Menu.
 3. (host)         DCM Host Menu.
 4. (active)       Display entries currently being updated.
 5. (failed)       Display entries with errors to be reset.
 6. (dcm)          Invoke a DCM update now.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.
@End(TVerbatim)

QUERIES USED:@\@Begin(Description, spread 0)
enable@\get_value, update_value

active@\qualified_get_server, qualified_get_server_host

failed@\quelified_get_server, qualified_get_server_host

dcm@\trigger_dcm
@End(Description)
@Hinge()
MENU:@\Miscellaneous Menu

DESCRIPTION:@\This menu contains miscellaneous functions which are not
necessary, but may be useful in maintaining the Moira system.  These
include fetching the table use statistics, listing currently active
connections to the Moira server, and fetching values and aliases from
the database.

DISPLAY:@Begin(TVerbatim)
                               Miscellaneous Menu
 1. (statistics)   Show database statistics.
 2. (clients)      Show active Moira clients.
 3. (getval)       Show a database variable value.
 4. (getalias)     Show an alias relation.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.
@End(TVerbatim)

QUERIES USED:@\@Begin(Description, spread 0)
statistics@\get_all_table_stats

clients@\_list_users

getval@\get_value

getalias@\get_alias
@End(Description)
@Hinge()
MENU:@\Cluster Data Menu

DESCRIPTION:@\This menu allows the manipulation of data associated
with clusters.

DISPLAY:@Begin(TVerbatim)
                               Cluster Data Menu
 1. (show)         Show Data on a given Cluster.
 2. (add)          Add Data to a given Cluster.
 3. (delete)       Remove Data to a given Cluster.
 4. (verbose)      Toggle Verbosity of Delete.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.
@End(TVerbatim)

QUERIES USED:@\@Begin(Description, spread 0)
show@\get_cluster_data

add@\add_cluster_data

delete@\get_cluster_data, delete_cluster_data
@End(Description)
@Hinge()
MENU:@\Mappings Menu

DESCRIPTION:@\This cluster allows the machine to cluster mappings to
be manipulated.

DISPLAY:@Begin(TVerbatim)
                        Machine To Cluster Mappings Menu
 1. (map)          Show Machine to cluster mapping.
 2. (addcluster)   Add machines to a clusters.
 3. (remcluster)   Remove machines from clusters.
 4. (verbose)      Toggle Verbosity of Delete.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.
@End(TVerbatim)

QUERIES USED:@\@Begin(Description, spread 0)
map@\get_machine_to_cluster_map

addcluster@\get_machine, get_cluster, add_machine_to_cluster

remcluster@\get_machine_to_cluster_map, delete_machine_from_cluster
@End(Description)
@Hinge()
MENU:@\Quota Menu

DESCRIPTION:@\This menu allows users' quotas and the default quota to
be manipulated.  The default quota is the quota that new users are
assigned when they register for an account.

DISPLAY:@Begin(TVerbatim)
                                   Quota Menu
 1. (shdef)        Show default user quota (in KB).
 2. (chdef)        Change default user quota.
 3. (shquota)      Show a user's disk quota on a filesytem.
 4. (addquota)     Add a new disk quota for user on a filesytem.
 5. (chquota)      Change a user's disk quota on a filesytem.
 6. (rmquota)      Remove a user's disk quota on a filesytem.
 7. (verbose)      Toggle Verbosity of Delete.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.
@End(TVerbatim)

QUERIES USED:@\@Begin(Description, spread 0)
shdef@\get_value

chdef@\get_value, update_value

shquota@\get_nfs_quota

addquota@\add_nfs_quota

chquota@\get_nfs_quota, update_nfs_quota

rmquota@\get_nfs_quota, delete_nfs_quota
@End(Description)
@Hinge()
MENU:@\Members Menu

DESCRIPTION:@\This allows the membership of lists to be manipulated.
On entry to this menu, Moira will prompt for the name of the list to be
manipulated.  Membership may be fetched by a specific type.

DISPLAY:@Begin(TVerbatim)
                     Change/Display membership of 'dbadmin'
 1. (add)          Add a member to this list.
 2. (remove)       Remove a member from this list.
 3. (all)          Show the members of this list.
 4. (user)         Show the members of type USER.
 5. (list)         Show the members of type LIST.
 6. (string)       Show the members of type STRING.
 7. (verbose)      Toggle Verbosity of Delete.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.
@End(TVerbatim)

QUERIES USED:@\@Begin(Description, spread 0)
add@\add_member_to_list

remove@\delete_member_from_list

all@\get_members_of_list

user@\get_members_of_list

list@\get_members_of_list

string@\get_members_of_list
@End(Description)
@Hinge()
MENU:@\List Information Menu

DESCRIPTION:@\This menu allows one to get various summaries of lists.
They can be retrieved by membership, administration, groups, or
maillists.

DISPLAY:@Begin(TVerbatim)
                             List Information Menu
 1. (member)       Show all lists to which a given member belongs.
 2. (admin)        Show all items which a given member can administer.
 3. (groups)       Show all lists which are groups.
 4. (public)       Show all public mailing lists.
 5. (maillists)    Show all mailing lists.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.
@End(TVerbatim)

QUERIES USED:@\@Begin(Description, spread 0)
member@\get_lists_of_member

admin@\get_ace_use

groups@\qualified_get_lists

public@\qualified_get_lists

maillists@\qualified_get_lists
@End(Description)
@Hinge()
MENU:@\Post Office Box Menu

DESCRIPTION:@\This menu allows users' poboxes to be manipulated.  The
``set'' option allows the user to return a pobox back from a foreign
maildrop to a previously used pobox server, in addition to allowing
changes in pobox servers or foriegn addresses.

DISPLAY:@Begin(TVerbatim)
                              Post Office Box Menu
 1. (show)         Show a user's post office box.
 2. (set)          Set (Add or Change) a user's post office box.
 3. (remove)       Remove a user's post office box.
 4. (verbose)      Toggle Verbosity of Delete.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.
@End(TVerbatim)

QUERIES USED:@\@Begin(Description, spread 0)
show@\get_pobox

set@\get_pobox, set_pobox_pop, get_server_locations, set_pobox

remove@\delete_pobox
@End(Description)
@Hinge()
MENU:@\DCM Service Menu

DESCRIPTION:@\This menu allows DCM services to be manipulated.  A
service may have an error indicator reset, or its entire state reset,
in addition to the usual updates.  Note that reseting a service state
is a dangerous action that should be used carefully.

DISPLAY:@Begin(TVerbatim)
                                DCM Service Menu
 1. (showserv)     Show service information.
 2. (addserv)      Add a new service.
 3. (updateserv)   Update service information.
 4. (resetsrverr)  Reset service error.
 5. (resetsrvc)    Reset service state.
 6. (delserv)      Delete service info.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.
@End(TVerbatim)

QUERIES USED:@\@Begin(Description, spread 0)
showserv@\get_server_info

addserv@\add_server_info

updateserv@\get_server_info, update_server_info

resetsrverr@\reset_server_error

resetsrvc@\set_server_internal_flags

delserv@\delete_server_info
@End(Description)
@Hinge()
MENU:@\DCM Host Menu

DESCRIPTION:@\This menu allows DCM service/host tuples to be
manipulated.  A tuple may have an error indicator reset, its entire
state reset, or an override set in addition to the usual updates.
Note that reseting a tuple's state is a dangerous action that should
be used carefully.

DISPLAY:@Begin(TVerbatim)
                                 DCM Host Menu
 1. (showhost)     Show service/host tuple information.
 2. (addhost)      Add a new service/host tuple.
 3. (updatehost)   Update a service/host tuple.
 4. (resethosterr) Reset service/host error.
 5. (resethost)    Reset service/host state.
 6. (override)     Set service/host override.
 7. (delhost)      Delete service/host tuple.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.
@End(TVerbatim)

QUERIES USED:@\@Begin(Description, spread 0)
showhost@\get_server_host_info

addhost@\add_server_host_info

updatehost@\get_server_host_info, update_server_host_info

resethosterr@\reset_server_host_error

resethost@\set_server_host_internal

override@\set_server_host_override

delhost@\delete_server_host_info
@End(Description)
@Hinge()
MENU:@\Zephyr ACL Menu

DESCRIPTION:@\This menu allows zephyr class access control lists to be
manipulated.  The list menu can be accessed as a sub-menu.

DISPLAY:@Begin(TVerbatim)
			Zephyr Class ACL Menu
 1. (show)	Show zephyr class information.
 2. (add)	Add a new zephyr class ACL.
 3. (update)	Change a zephyr class ACL.
 4. (delete)	Delete ACL information for a zephyr class.
 5. (list)	List Menu.
 r. (return)	Return to previous menu.
 t. (toggle)	Toggle logging on and off.
 q. (quit)	Quit.
@End(TVerbatim)

QUERIES USED:@\@Begin(Description, spread 0)
show@\get_zephyr_class

add@\get_zephyr_class, add_zephyr_class

update@\get_zephyr_class, update_zephyr_class

delete@\get_zehpyr_class, delete_zephyr_class
@End(Description)
@Hinge()
MENU:@\Network Services Menu

DESCRIPTION:@\This menu allows the network service ports and their
aliases to be manipulated.

DISPLAY:@Begin(TVerbatim)
			Network Services Menu
 1. (show)	Show service information.
 2. (add)	Add information about a new service.
 3. (update)	Change information about a service.
 4. (delete)	Delete information about a service.
 5. (alias)	Add an alias for a service.
 6. (unalias)	Remove an alias for a service.
 r. (return)	Return to previous menu.
 t. (toggle)	Toggle logging on and off.
 q. (quit)	Quit.
@End(TVerbatim)

QUERIES USED:@\@Begin(Description, spread 0)
show@\get_service

add@\@\get_service, add_service

update@\get_service, delete_service, add_service

delete@\get_service, delete_service

alias@\get_alias, get_service

unalias@\get_alias, delete_alias
@End(Description)

@End(Group)
@End(Description)
@End(Description)

@NewPage()

PROGRAM NAME:@\MRCHECK - Check to see if updates have been successful

DESCRIPTION:@\This program lists any DCM updates which have failed.

PRE-DEFINED QUERIES USED:@\@Begin(TItemize)
qualified_get_server

qualified_get_server_host

get_server_info

get_server_host_info
@End(TItemize)

SERVICE(S) EXAMINED:@\
@Begin(TItemize)
all
@End(TItemize)

END USERS:@\Moira system administrator

A SESSION USING MRCHECK:
@Begin(TVerbatim)
% mrcheck
Service AFS, error 43: Unable to build archive of config files
        last success Jan  4 12:15:13 1989, last try Jan  4 16:30:10 1989
Host HESIOD:KIWI.MIT.EDU, error 44: Unable to open DCM file
        last success Jan 11 14:15:18 1989, last try Jan 24 17:40:16 1989
Host AFS:FOO.MIT.EDU, error 31: Kerberos error: Can't decode authenticator
        last success Dec 12 14:51:06 1988, last try Dec 12 19:00:09 1988
Host NFS:CHIROPTERA.MIT.EDU, error 8: Kerberos principal unknown
        last success Nov 15 23:44:37 1988, last try Nov 18 03:01:52 1988
4 things have failed at this time
%
@End(TVerbatim)

