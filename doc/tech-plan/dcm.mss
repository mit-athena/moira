@Comment[
	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/doc/tech-plan/dcm.mss,v $
	$Author: wesommer $
	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/doc/tech-plan/dcm.mss,v 1.1 1987-05-20 14:42:38 wesommer Exp $

	$Log: not supported by cvs2svn $

]
@part[dcm, root="sms.mss"]
@Section(The Data Control Manager)

The data control manager, or DCM, is a program responsible for distributing
information to servers. Basically, the DCM is invoked by crontab at times
which are relevant to the data update needs of each server.  The DCM
extracts SMS data, converts it to server-dependent form, and transfers it to
the servers.  Because each server has unique information requirements, along
with varrying data structure requirements, a generic server description
syntax is used to represent the special features of each server.

@SubSection(Server Description Files)

The server description files, or SDF, are files which contains a unique
description of each server SMS updates.

These files hold information, in english-like syntax, which the DCM for
manipulating generic data into server specific form.  Accomodating new
servers requires, simply, adding a new SDF to the system.  The purpose of
the server description files is to provide a parsable, readable text files
for determining the structure and personality of a given server.  The three
reasons for SDF:

@Begin(Itemize, Spread 0)

To provide a local, uniform and expandable method of providing server
information to the Data Control Manager.

To maintain simplicity and readabilility.

To present a regular way of describing many models of servers.

@End(Itemize)

The SDF will be comprised of the following three files:

@StandardTable(Name SDF, Columns 2, FirstColumn FlushLeft, 
	OtherColumns Text, OtherColumnWidth 2.5inches, 
	ColumnHeadings FlushLeft, Boxed, FlushTop)
@Begin(SDF)
@TableID(SDF)
@TableHeading(Immediate, RowFormat SDFColumnHeadings, 
	Line "File@\Description")

~/sms/serverType@\directory on SMS machine to hold information for DCM.
Where serverType is the type of server.  (i.e. Name server, RVD server, etc,
...)

~/serverType/server.info@\file holding server information, the number of
these servers to update, location, etc.

~/serverType/server.data@\describes the specific format of the data.

@End(SDF)

Based on this information the DCM will query the database and put together
the necessary information.

The SDF consists of a collection of key words and commands which the DCM
interprets.  The format of the files is line oriented and parsable.  An
example of SDF syntax is:

@Begin(Verbatim, LeftMargin +.5inch)

SERVICENAME =
FREQUENCY = 	
HEADER = 
BEGIN HEADER DATA
	
END HEADER DATA	
BEGIN BODY DATA

END BODY DATA
@End(Verbatim)

There are two types of data which the SDF handles: Static data and dymamic
data.  The static data is data which resides in the SDF as a component of
the SDF.  This data is entered during the creation of the SDF and is changed
only when the system adminitrator changes the format of the SDF. An example
of static data:

@Begin(Verbatim, LeftMargin +.5inch)

SERVICENAME = static data (i.e. "userlib")
FREQUENCY = static data (i.e. "6 HOURS")
HEADER = static data (i.e. "TRUE")
BEGIN HEADER DATA

END HEADER DATA

BEGIN BODY DATA
	
END BODY DATA
@End(Verbatim)

Dynamic data is data which changes based on query responses.  This data is
derived from a pre-defined list of queries.  Each query used by SDF has an
associated pre-defined query handle.  The pre-defined handle is used as a
method of representing q particular query.  For example, a list of queries
exist, representing all the required relationships for retreiving data from
the SMS database. A simple list may look like the following:

@Begin(Verbatim, LeftMargin +.5inch)

HANDLE	    QUERY
	
1	    GET ALL NAMES BEGINING WITH <VAR>
		Where VAR is a letter

2	    GET ALL CLUSTER NAMES

3 	    GET MACHINE INFORMATION FOR ALL MACHINES OF 
	    TYPE <VAR>
		Where VAR is VS2, RTPC, 750.
@End(Verbatim)

The SDF uses each handle to describe the server format.  A SDF uses the
handle in the following method:

@Begin(Verbatim, LeftMargin +.5inch)

BEGIN HEADER DATA

<USE 1 WITH DATA "L"> #Retreive all names beginning with "L"

END HEADER DATA
@End(Verbatim)

So a simple SDF would look like the following:

@Begin(Verbatim, LeftMargin +.5inch)

#Sharp sign is for comments
##Double sharp sign puts comment in transport file

#This example does nothing.
##Text to go in transport file

SERVICENAME = "userlib"
FREQUENCY = "6 HOURS"
HEADER = TRUE
##Header
BEGIN HEADER DATA
		
"Created=4-8-87"

END HEADER DATA
##Body
BEGIN BODY DATA

<USE 1 WITH DATA "L">  "DYNAMIC DATA"
#Retreive all names beginning with "L" and make a list of 
#names and a field called DYNAMIC DATA

END BODY DATA
##End transport file
@End(Verbatim)
	
The net result is a file bound by the DCM which has the following format:

@Begin(Verbatim, LeftMargin +.5inch)

#Double sharp sign puts comment in transport file
#Text to go in transport file
#Header
Created=4-8-87
#Body
LOU	DYNAMIC DATA
LEVINE	DYNAMIC DATA
LUCY	DYNAMIC DATA
LAST	DYNAMIC DATA
#End transport file
@End(Verbatim)	

From a generic database the DCM will take the information, process it into
localized form, and serve it over the net. The DCM is responsible for
updating servers(i.e. a name server) along with generating and distributing
system configuration files.  The advantage of having this mechanism is to
provide a systematic, automated approach to distributing centralized
information.

@SubSection(SMS-to-Server Update Protocol)

(@b[NOTE]: This information has been used verbatim from J.H. Saltzer's
document @i[Reliable sms-to-server update protocol], draft
of 14 March 1987)

SMS provides a reliable mechanism for updating the servers it manages.  The
use of an update protocol allows the servers to be reliably managed.  The
goals of the server update protocol are:

@Begin(Itemize)

Completely automatic update for normal cases and expected kinds of failures.

Survives clean server crashes.

Survives clean SMS crashes.

Easy to understand state and recovery by hand.

@End(Itemize)

General approach: serialized two-phase atomic update.  SMS and the server
each maintain a stable record of the serial number of the current update
level.  The algorithm constrains updates to take place in serial number
order.

Supporting facilities: all data movement is initiated by SMS; data files can
be moved from the SMS to the server, and the server's commit record can be
checked by SMS, using a Kerberos-protected rcp, ftp, tftp, or NFS/RFS/Vice
remote file system interface.  Whatever movement method is used should have
the property that a death timer can be placed around its invocation, and if
the timer goes off a clean shutdown of the transfer can be done.  One
control message from the server back to the SMS (confirming commitment or
aborting of the transaction) can be carried by an unreliable datagram; that
datagram should be Kerberos-authenticated.  A method is needed to flush file
system caches to the disk; for BSD UNIX a call to fsync followed by a
two-second pause may be an adequate approximation.

@SubHeading(Strategy)

@Begin(Enumerate)

Preparation phase.  Initiated by SMS when it notices that it has
something new that should be passed to the server.  May be triggered
by a change in data, by an administrator, by a timer (crontab), or by
an SMS restart.
@Begin(Enumerate)

Check whether or not the previous update is committed yet.  If not (previous
attempt at update either aborted or hasn't finished yet), do nothing but
report to (human) system administrator that updates are piling up for some
reason.

Check server's serial update level; compare with SMS record of update level.
If different, report to system administrator that the world is screwed up.
If identical, proceed.

Figure out next serial update number for this server, create a commit record
for this update, with initial state UNCOMMITTED.  Discard previous commit
record at SMS for this server, if any.

Create an intentions file, store stably at SMS.  It contains
@Begin(Itemize, Spread 0)
serial update number

identity of target server

list of files to be transferred from SMS to server

list of actions to be performed by server
@End(Itemize)
[For BSD Unix, the serial update number can be the prefix of the name
of the intentions file, e.g., "update741".  The commit record can be
coded into the suffix of the name of the intentions file, e.g.,
.UNCOMMITTED, .COMMITTED, or .ABORTED.  The state of the commit
record is changed atomically with rename system calls, and directory
operations are used to learn the latest serial update level and to
read the commit record value encoded in the name.]

Flush SMS caches to disk, to ensure that commit record and transaction
details are safe from an SMS crash.

Copy detail files to server's buffer area.  (The buffer area at the server
is under the control of the SMS at this point; the SMS can delete anything
it finds in that buffer area except the most recent transaction commit
record.)

Copy intentions file to a stable location at the server.

Get server to flush caches to disk.

Copy commit record to stable storage of server and get server to flush
caches to disk.  This copy, when flushed to the disk, is the thing that
guarantees that the transaction will actually take place.  Until the
transaction gets to this point, the update can vanish without a trace.  This
action turns control of the buffer are over to the server.  [With the BSD
UNIX implementation suggested above, the intentions file might be copied
with a temporary name, and then renamed to create the commit record.]

Update SMS authoritative data to show that server has been asked to move to
the new serial update level.  (The SMS copy of the commit record tells
whether or not SMS knows that the server completed the update.)

Ask the server to do the actions.  Acknowledgement of this request is not
required, but this step might be repeated until some acknowledgement comes
back, to minimize the performance cost of a lost packet.  (Indeed, except
for getting the update done on a timely basis, this step can even be
omitted, because there is a birddog process at the server that looks for
commit records and starts processing on their associated intentions files.)
@End(Enumerate)

Commit phase.  Initiated by server as a result of discovering a
commit record that is in the UNCOMMITTED state.  The discovery may be
because the SMS woke it up (the usual case), because of a crontab
trigger, or as part of a system restart.
@Begin(Enumerate)
Discard any old commit records.

Do actions in intentions file.

Change server copy of commit record to COMMITTED.

Discard buffer copies of detail files and the intentions file.  (If the name
of the intentions file is the commit record, then just truncate the
intentions file.)

Notify SMS of completion.  This notification returns control of the server
buffer area to SMS; the commit record remains as a record of the update
level of the server.

SMS changes its commit record state to COMMITTED and discards its copy of
intentions file, retaining the commit record as evidence of the actual
update level of the server.
@End(Enumerate)
@End(Enumerate)

@SubHeading(Trouble Recovery Procedures)

@Begin(Enumerate)

Server finds that actions can't be done.  (Out of disk, or some other
unrecoverable problem.)
@Begin(Enumerate, Spread 0)
Change server copy of commit record to ABORTED

Notify SMS that transaction aborted.

SMS changes its copy of commit record to ABORTED and sends message alerting
system manager.
@End(Enumerate)
General rule: an aborted transaction requires human intervention to
figure out what went wrong.  Step 1 of the preparation phase stalls
all additional attempts at update until the system administrator
fixes the problem and deletes the commit record for the aborted
transaction at the SMS end.

Server crashes/restarts:
@Begin(Enumerate)
Kill off any previously running server update processes.

Recover server data bases to last committed update level.  If an update was
in progress that had not reached the point of changing the server's copy of
the commit record to COMMITTED, the server returns to the previous update
level.

Check for an UNCOMMITTED commit record.  If found, perform commit phase, as
earlier.  In any case, start a bird dog procedure that does this check every
thirty minutes.
@End(Enumerate)

SMS crashes/restarts
@Begin(Enumerate)
Kill off any previously running SMS server update processes.

Look for commit records.  For each one found, branch on state:
@Begin(Description)
COMMITTED@\skip this record

ABORTED@\skip this record

UNCOMMITTED@\look at corresponding server commit record, branch on its state:
@Begin(Description)
COMMITTED@\record that state in the SMS commit record.

ABORTED@\record that state in the SMS commit record and send alert message
to system administrator.

UNCOMMITTED@\skip.

nonexistent@\discard SMS commit record, intentions file, and detail files,
and alert system administrator of failed update.  (SMS's own recovery
procedures should lead to a retry of the update.)

server doesn't respond@\alert system administrator, and skip this commit
record.
@End(Description)
@End(Description)

Run procedure to recover SMS authoritative data bases.
@End(Enumerate)

Death timeout in SMS preparation phase.  If any step of the preparation
phase other than copying the commit file to the server hangs up because of
network failure, a death timer eventually goes off.  The timer handler
alerts the system administrator of trouble, then backs out of this update by
deleting the commit record, intentions file, and detail files.  If copying
the commit file hangs up, the timer handler alerts the system administrator
that the outcome of this update is unknown, but does not back out of the
transaction.  (The copy may have been successful, and the server may be
doing the update.)

Network outages that cause failures of individual steps of this protocol are
recovered by the server birddog procedure and the SMS death timer.
@End(Enumerate)

@SubSection(Data Transport Security)

Each datagram which is transmitted over the network is secure using
Kerberos, the Athena authentication system.  Enciphered information headers
will proceed each datagram.  The servers decrypt the header information and
use the packet accordingly.
