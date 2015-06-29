@part[serverarr, root="moira.mss"]
@Define(hesverbatim=verbatim, Above 1, size -2,
	LeftMargin -1 inch, group)
@section(Server Arrangement)

Currently, Moira acts to update a variety of servers.  Although the data 
control manager performs this update task, each server requires a 
different set of update parameters.  To date, the DCM uses c programs,
not SDFs, to implement the construction of the server specific files. 
Each c program is checked in via the dcm_maint program.   The DCM then
calls the appropriate module when the update interval is reached.

For each server file propagated, there is at least one application 
interface which provides the capability to manipulate the Moira database.
Since the Moira database acts as a single point of contact, the
changes made to the database are reflected in the contents of 
recipient servers. 

The services which Moira now supports are:
@begin(itemize, spread .5)
Hesiod - The athena nameserver.

NFS - Network file system.

/usr/lib/aliases - Mail Service.

Zephyr - The athena notification service.
@end(itemize)

@SubSection(Server Assumptions)

The requirements of each server suggests a level of detail describing
the following:

@Begin(Itemize, Spread .5, Above 1)

Service name.

Service description.

Propagation interval.

Data format.

Target location.

Generated files.

File description.

Queries used to generate the file (including fields queried).

How the file is modified (application interface).

Example of file contents.
@End(Itemize)

@SubSection(Server Descriptions)

@Begin(Description, Spread 1)
Service:@\@U(Hesiod)

Description:@\@Multiple[The hesiod server is a primary source of contact for
many athena operations.  It is reponsible for providing information reliably
and quickly.  Moira's responsibility to hesiod is to provide authoritative
data.  Hesiod uses a BIND data format in all of it's data files.  Moira will
provide BIND format to hesiod.  There are several files which hesiod uses.
To date, they are known to include the following:

@Begin(Itemize, Spread 0, Above 1, Below 1)

cluster.db

filsys.db

gid.db

group.db

grplist.db

passwd.db

pobox.db

printcap.db

service.db 

sloc.db

uid.db

@End(Itemize)

Each of these files are described in detail below.  The hesiod server uses
these files from virtual memory on the target machine.  The server
automatically loads the files from disk into memory when it is started.
Moira will propagate hesiod files to the target disk and the run a shell script 
which will kill the running server and then restart it, causing the newly
updated files to be read into memory. 

With hesiod, all target machines receive identical files.  Practically, 
therefore, the DCM will prepare only one set of files and then will
propagate to several target hosts.

For additional technical information on @i[hesiod], please refer to the
Hesiod technical plan.
]

Propagation interval:@\6 Hours

Data format:@\tar file of several BIND files

Target locations:@\
@begin(Format, Above 0, Below 0) 
SUOMI.MIT.EDU: /tmp/hesiod.out
@end(Format)

Files:@\
@Begin(Description, LeftMargin +0)

@B(CLUSTER.DB)@\Cluster data

Description:@\Cluster.db holds the relationships between machines,
clusters, and services to service clusters.  It must be possible to
look up a cluster by name, and find all of the cluster data.  It must
also be possible to look up a machine by name, and get the union of
all of the cluster data for each cluster the machine is a member of.

Queries used:@\

Client(s):@\save_cluster_info

Example contents:@\
@Begin(HesVerbatim)
; lines for per-cluster info (both vs and rt) (type UNSPECA)
; and a line for each machine (CNAME referring to one of the lines
; above) 
;
bldge40-vs.cluster HS UNSPECA "zephyr neskaya.mit.edu"
bldge40-rt.cluster HS UNSPECA "zephyr neskaya.mit.edu"
bldge40-vs.cluster HS UNSPECA "lpr e40"
bldge40-rt.cluster HS UNSPECA "lpr e40"
TOTO.cluster	   HS CNAME bldge40-vs.cluster   
SCARECROW.cluster  HS CNAME bldge40-rt.cluster
@End(hesVerbatim)
Note that a machine may be in more than one cluster.  In this case, a
pseudo-cluster will be made by Moira which has as its cluster data, the
union of the data of each of the other clusters this machine is in.
Then the machine in question will be CNAME'd into this pseudo-cluster.
@Hinge()

@B(FILSYS.DB)@\filesystems

Description@\These are all of the filesystem entries needed to find
and attach NFS lockers and RVDs by name.  Each entry contains the name
of the filesystem, its name on the fileserver (directory name for NFS
filesystems, or packname for RVDs), the name of the server, the
default attach mode (r = read-only, w = read/write), and the default
client mount point.

Queries used:

Clients(s):@\attach

Example contents:
@Begin(hesVerbatim)
aab.filsys      HS UNSPECA "NFS /mit/aab charon w /mit/aab"
aabiyaba.filsys HS UNSPECA "NFS /mit/aabiyaba eurydice w /mit/aabiyaba"
ade.filsys      HS UNSPECA "RVD ade helen r /mnt/ade"
@End(hesVerbatim)
@Hinge()

@B(GID.DB)@\group IDs

Description:@\This file maps group ID numbers to the group names.
There is an entry in this file for each entry in the group.db file,
pointing to a corresponding entry in the group.db file.

Queries used:

Clients(s):

Example contents:
@Begin(hesVerbatim)
10914.gid       HS CNAME babette.group
10915.gid       HS CNAME 14.31.group
10916.gid       HS CNAME abarba.group
10917.gid       HS CNAME mga.group
10918.gid       HS CNAME rs1maint.group
10919.gid       HS CNAME pjd.group
@End(hesVerbatim)
@Hinge()

@B(GROUP.DB)@\unix groups

Description:@\This file maps group names to their unix group ID
numbers.  The returned entries are of the same form as lines in an
/etc/group file, although none of the members are actually filled in.
An entry is only placed in this file if the group is marked @i(active)
in the Moira database.

Queries used:

Clients(s):

Example contents:
@Begin(hesVerbatim)
babette.group   HS UNSPECA "babette:*:10914:"
14.31.group     HS UNSPECA "14.31:*:10915:"
abarba.group    HS UNSPECA "abarba:*:10916:"
mga.group       HS UNSPECA "mga:*:10917:"
rs1maint.group  HS UNSPECA "rs1maint:*:10918:"
pjd.group       HS UNSPECA "pjd:*:10919:"
@End(hesVerbatim)
@Hinge()

@B(GRPLIST.DB)@\group lists

Description:@\This file lists the groups that each user is a member
of.  Each entry consists of a colon-separated list of colon-separated
pairs of groupname, group id.  No meaning is placed on the order of
the groups listed.  Only users whose @i(status) is active will have
entries generated, and only groups that are marked @i(active) in Moira
will be output.

Queries used:

Clients(s):@\login

Example contents:
@Begin(hesVerbatim)
mtalford.grplist  HS UNSPECA "mtalford:5904:3_d0004:689"
mswelsh.grplist	  HS UNSPECA "mswelsh:5901:13_461t:867:13_461sa:868:13_012t:800"
mstai.grplist     HS UNSPECA "mstai:5899"
@End(hesVerbatim)
@Hinge()

@B(PASSWD.DB)@\password entries

Description:@\This file contains lines similar to those found in
/etc/passwd for each active user of Athena.  Only users whose
@i(status) is active will have entries generated.

Queries used:

Clients(s):@\login

Example contents:
@Begin(hesVerbatim)
babette.passwd  HS UNSPECA "babette:*:6530:101:Harmon C Fowler,,,,:/mit/babette
:/bin/csh"
abarba.passwd   HS UNSPECA "abarba:*:6531:101:Angela Barba,,,,:/mit/abarba:/bin
/csh"
mga.passwd      HS UNSPECA "mga:*:6532:101:Gerhard Messmer,,,,:/mit/mga:/bin/cs
h"
kazimi.passwd   HS UNSPECA "kazimi:*:6533:101:Martin Zimmermann,,,,:/mit/kazimi
:/bin/csh"
pjd.passwd      HS UNSPECA "pjd:*:6535:101:Peter J Delaney,,,,:/mit/pjd:/bin/cs
h"
@End(hesVerbatim)
@Hinge()

@B(POBOX.DB)@\Post Office Box locations

Description:@\This file locates each user's post office box.  Only
active users whose pobox type is "POP" will have entries output.

Queries used:

Clients(s):@\inc, movemail

Example contents:
@Begin(hesVerbatim)
babette.pobox   HS UNSPECA "POP ATHENA-PO-2.MIT.EDU babette"
abarba.pobox    HS UNSPECA "POP ATHENA-PO-1.MIT.EDU abarba"
mga.pobox       HS UNSPECA "POP ATHENA-PO-1.MIT.EDU mga"
kazimi.pobox    HS UNSPECA "POP ATHENA-PO-2.MIT.EDU kazimi"
@End(hesVerbatim)
@Hinge()

@B(PRINTCAP.DB)@\printer capability entries

Description:@\This file contains the information from the standard
/etc/printcap file.

Queries used:

Clients(s):@\lpr, lpq, lprm

Example contents:
@Begin(hesVerbatim)
linus.pcap      HS UNSPECA "linus:rp=linus:rm=BLANKET.MIT.EDU:sd=/usr/spool/pri
nter/linus"
la-pika.pcap    HS UNSPECA "la-pika:rp=la-pika:rm=EVE.PIKA.MIT.EDU:sd=/usr/spoo
l/printer/la-pika"
ln03-pika.pcap  HS UNSPECA "ln03-pika:rp=ln03-pika:rm=EVE.PIKA.MIT.EDU:sd=/usr/
spool/printer/ln03-pika"
helios.pcap     HS UNSPECA "helios:rp=helios:rm=M16-034-P.MIT.EDU:sd=/usr/spool
/printer/helios"
@End(hesVerbatim)
@Hinge()

@B(SERVICE.DB)@\network service map

Description:@\This file contains the information from the standard
/etc/services file.  It comes from the services relation of the
database, and the aliases.

Queries used:

Clients(s):

Example contents:
@Begin(hesVerbatim)
gdb_test3.service       HS UNSPECA "gdb_test3 tcp 2253"
qotd.service    HS UNSPECA "qotd tcp 17"
rpc_ns.service  HS UNSPECA "rpc_ns udp 32767"
smtp.service    HS UNSPECA "smtp tcp 25"
X1.service      HS UNSPECA "X1 tcp 5801"
@End(hesVerbatim)
@Hinge()

@B(SLOC.DB)@\service location information

Description:@\This file identifies which hosts support which
services.  It is a listing of DCM service/host tuples, indexed by
service.

Queries used:

Clients(s):@\zhm, chpobox, get_message

Example contents:
@Begin(hesVerbatim)
ATHENA_MESSAGE.sloc     HS UNSPECA BITSY.MIT.EDU
FOO.sloc        HS UNSPECA TOTO.MIT.EDU
GMOTD.sloc      HS UNSPECA BITSY.MIT.EDU
HESIOD.sloc     HS UNSPECA KIWI.MIT.EDU
HESIOD.sloc     HS UNSPECA SUOMI.MIT.EDU
LOCAL.sloc      HS UNSPECA HACTAR.MIT.EDU
@End(hesVerbatim)
@Hinge()

@B(UID.DB)@\user ID mappings

Description:@\This file maps unix user IDs to the user password
entries.  There is a corresponding entry in this file for every entry
in the passwd.db file.

Queries used:

Clients(s):

Example contents:
@Begin(hesVerbatim)
6530.uid        HS CNAME babette.passwd
6531.uid        HS CNAME abarba.passwd
6532.uid        HS CNAME mga.passwd
6533.uid        HS CNAME kazimi.passwd
6535.uid        HS CNAME pjd.passwd
@End(hesVerbatim)
@end(Description)


Service:@\@U(NFS)

Description:@\@Multiple[Moira supports three files which are necessary
components of NFS operation.  These files are:

@Begin(Itemize, Spread 0)
/usr/etc/credentials

The quotas file

The directories file
@End(Itemize)

The credentials file determines access permissions to files on the NFS
server.  It contains mappings from user name to unix UID and a group
list.  This file can be generated from a list in the Moira database, or
may contain all active users.  Which credentials file is loaded on a
particular server is determined by the value3 field of the serverhost
relation.  If this field is non-blank, it specifies the list whose
membership will appear in the credentials file.  If the field is
blank, then all active users will appear in the credentials file.

The quotas and directories files are in a private format to Moira, which
is understood by the shell script which updates quotas and creates
directories.  The quotas file contains many tuples matching a unix UID
with a quota.  The directories file lists directory name, owning user,
owning group, and directory type.  The type is used to control the
directory mode and which init files are loaded into it.

These files reside on the NFS target machine and are used to allocate NFS 
directories on a per user basis.  The mechanism employed is for all
programs to communicate to the Moira database, and then for the dcm to handle
the propagation and creating of NFS lockers.  The best illustration of this 
process is indicated by the following example:

During new user registration, a person will sit down to a workstation and
type 'userreg' for his login name.  When validated the user will 
type a 'real' login name and a password.  In addition, the userreg
program will allocate, automatically, for the user a post office and
an NFS directory.  However, the user will not benefit from this
allocation for a maximum of six hours.  This lag time is due to the
operation of Moira and its creation of NFS lockers.  During registration,
the userreg program communicates exclusively with
the Moira database for NFS allocation.  Since the NFS file generation
is started by the DCM every 6 hours, the real change is not noticed
for a period of time.  When the 6 hour time is reached the DCM will
create the above two files and send them to the appropriate
target servers.  Once on the target machine, the dcm will invoke a shell 
script which reads the /mit/quota file and then creates the NFS directory.
The basic operation of the script is:

@Begin(Format, Above 1, LeftMargin +4)
mkdir <username>, chown, chgrp, chmod - using directories file
setquota <quota> - using quotas file
@End(Format)
]

Propagation interval:@\12 hours

Data Format:@\ASCII

Client(s):@\
@begin(Format)
NFS server
Moira shell script for creating directories and user quotas.   
@end(Format)

Files updated:@\
@Begin(Description, LeftMargin +0)

@B(CREDENTIALS)@\username to uid/gid mapping.

Description:@\ This file is used for both the NFS server information
and for the Moira shell script.  It provides a username to uid/gid
mapping.  A master credentials file is generated which contains all
active users.  In addition, smaller credentials files may be produced
if necessary, with their membership taken from an Moira list.  Each line
is an entry consisting of the username, uid, and the gid of each group
the user is in.  Each field is separated by a colon.

Queries used:

Contents example:
@Begin(hesVerbatim)
mtarriol:15786:5905
mtalford:14956:5904:689
mswelsh:13764:5901:867:868:800
mstai:9296:5899
@End(hesVerbatim)
@Hinge()

@B(QUOTAS)@\file containing user to quota mapping.

Description:@\ This file contains the mapping between username and
quota.  The file is distributed to each filesystem on the recipient
machine.  Each entry contains a unix uid and a quota.  Each of the
file's contents is unique to the filesystem which it represents.

Queries used:

Contents example:
@Begin(hesVerbatim)
219 600
567 600
1251 600
1282 600
1312 600
@End(hesVerbatim)
@Hinge()

@B(DIRECTORIES)@\file containing info for creating NFS lockers

Description:@\This file contains the info necessary to create lockers.
Only lockers with the @i(autocreate) flag set will be output.  The
file is distributed to each filesystem on the recipient machine.  Each
entry contains a directory name, owning uid and gid, and a locker
type.  If the directory does not already exist, it will be created
with the specified ownership.  If the type is "HOMEDIR", it will be
loaded with the default init files as well.

Queries used:

Contents example:
@Begin(hesVerbatim)
/mit/lockers/test1 6526 10912 HOMEDIR
/mit/lockers/babette 6530 10914 HOMEDIR
/mit/lockers/kazimi 6533 10923 HOMEDIR
/mit/lockers/mastein 14489 10928 HOMEDIR
@End(hesVerbatim)
@End(Description)


Service:@\@U(Mail)

Description:@\ The /usr/lib/aliases file is created and propagated to
athena.mit.edu.  Only one file and one propagation is required.  This
file is not automatically installed on the mailhub because the mail
spool must be disabled during the switchover.  A second file is also
propigated to the mail hub.  This is a complete password file so that
the finger server on the mailhub will know about everybody.

Data Type:@\ASCII (sendmail aliases format)

Propagation interval:@\24 hours

Target:@\ATHENA.MIT.EDU

File(s):@\

@Begin(Description, LeftMargin +0)

@B(/USR/LIB/ALIASES)@\mail forwarding information

Description:@\This file contains both mailing lists and post office
boxes.  Mailing lists are output only if the list is marked @i(active)
in the Moira database.  Poboxes are only output if the user's account is
active.

Queries Used:

Contents example:
@Begin(hesVerbatim)
#  Video Users
owner-video-users: paul
video-users: smyser, paul, mwsmith, davis, rubin@@media-lab.mit.edu,
        gid@@media-lab.mit.edu, danapple, agarvin

babette: babette@@ATHENA-PO-2.LOCAL
yvette: yvette@@ATHENA-PO-2.LOCAL
test1: test1@@ATHENA-PO-2.LOCAL
@End(hesVerbatim)
@Hinge()

@B(/ETC/PASSWD)@\user account file

Description:@\This is a standard format unix password file.  It
contains an entry for every active account at Athena.

Queries Used:

Contents example:
@Begin(hesVerbatim)
test1:*:6526:101:Test One,,,:/mit/test1:/bin/csh
babette:*:6530:101:Harmon C Fowler,,,:/mit/babette:/bin/csh
abarba:*:6531:101:Angela Barba,,,:/mit/abarba:/bin/csh
mga:*:6532:101:Gerhard Messmer,,,:/mit/mga:/bin/csh
kazimi:*:6533:101:Martin Zimmermann,,,:/mit/kazimi:/bin/csh
pjd:*:6535:101:Peter J Delaney,,,:/mit/pjd:/bin/csh
@End(hesVerbatim)
@End(Description)


Service:@\@U(ZEPHYR)

Description:@\The zephyr system has access control lists associated
with some actions on some classes of message.  Moira updates these
access control lists on the zephyr servers from lists stored in Moira.

Data Type:@\A tar file of ASCII acl files.

Propagation interval:@\24 hours

Target:each of the zephyr servers.

File(s):@\
@Begin(Description, LeftMargin +0)

@b(*.ACL)@\zephyr ACL file

Description:@\For each existing ACE (even if it is empty), the
membership will be output, one entry per line.  Recursive lists will
be expanded.

Queries Used:

Contents example:
@Begin(hesVerbatim)
*.*@@*
@End(hesVerbatim)
@End(Description)
@End(Description)

