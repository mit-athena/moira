@Comment[
	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/doc/tech-plan/dcm.mss,v $
	$Author: ambar $
	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/doc/tech-plan/dcm.mss,v 1.4 1987-05-29 14:29:29 ambar Exp $

	$Log: not supported by cvs2svn $
Revision 1.3  87/05/29  03:27:37  ambar
fixed scribe error

Revision 1.2  87/05/29  03:14:20  ambar
Added in Peter's changes.

Revision 1.1  87/05/20  14:42:38  wesommer
Initial revision


]
@part[dcm, root="sms.mss"]
@Section(The Data Control Manager)

The data control manager, or DCM, is a program responsible for
distributing information to servers. Basically, the DCM is invoked by
cron at times which are relevant to the data update needs of each
server.  The update frequency is stored in the sms database.  A
server/host relationship is unique to each update time.  Through the sms
query mechanism, the dcm extracts sms data and converts it to server
dependent form.  The conversion of database specific information to site
specific information is done through a server description file, a
sms-unique language which is used to describe the personality of a
target service.

When invoked the dcm will perform some preliminary operations to
establish the relationship between the sms data and each server.
The very first time the dcm is called, a table is constructed 
describing the relationship between servers and update frequency.
The table will be the primary mechanism used by the dcm for recognizing
the servers which need updating at given times.  As a note here,
crontab will invoke the dcm at a pre-established time interval,
say every 15 minutes.  Obviously, the maximum update time will be limited to
the time interval the dcm is being invoked at.  Every interval,
the dcm will search the constructed table and determine whether or 
not a server's update time is within the current time range.  The table
has the following components:

@begin(verbatim)

Last time|Success|Time    |Server|Hostname|Target  |Override|Enable|
tried    |       |interval|      |        |Pathname|        |      |
update   |       |        |      |        |        |        |      |

@end(verbatim)

A description of each field follows:
@begin(itemize, spread 0)

@i[Last Time of Update] - This field holds the time when a last 
successful update occured.  This time will be used against 
the current time to determine if the interval criteria has been met.

@i[Success] - Flag for indicating whether or not the last time tried was
successful. 0-fail, 1-success

@i[Time interval] - Dervived from the sms database.  Gives the interval
update time for each server's information needs.

@i[Server] - This is the server name.  Derived from sms database.

@i[Hostname] - This is the host name where the server resides. Derived from
the sms database.

@i[Target Pathname] - Gives the location of the file which needs to be 
updated on the target or server end.  Derived from the sms database.

@i[Override] - Provides an automatic facility for the authorized
user to invoke a used-once mechanism for overriding the established
time interval.  The facility will be very useful when a server has received
bogus data and needs updating immediately.  After the dcm uses this
value, the field is reset to -1.
		Value: -1 - Use established time interval.
			0 and greater - New once-used interval.

@i[Enable] - This switch allows the authorized user to turn the update
facility on or off.  
		Value: 0 - Off, Non-zero - On.
@end(itemize)

Each time the dcm is invoked, a search through this table will 
indicate which servers need updating.  Once located, the dcm will
use the server/hostname combination to identify the server description
files to process. 
Of course, if the enable switch is off, the 
update will not occur.  

@SubSection(DCM Operation)

The data control manager acts as an interpreter on the SDF's.  The basic
mechanism is for the DCM to read the above entry table, determine which
servers need updating and then locate the appropriate SDF for interpretation.
The breakdown of the SDF is a procedure based primarily on the associated
query handle and it's associated input and output structure.  The ouput of the
DCM is a file stored on the sms host which is exactly the same
format of the server-based file.  The update mechanism takes this localized
data and ships it over the net. 

The most used statements in an SDF are the commands which set up a query 
request.  In partcular, these commands are:

@begin(verbatim)

		Begin query
			handle: handle number
			input: field1 = "string", field2 = "string2"
			output: var1
		End query

	Where:
		handle number -  is the associated handle of the query.
		input - is the input to be entered with the query.
		output - is the output cache for the query result.

		field1 and field2 - refer to the precise structure fields
			found in struct.h, the sms input file of associated
			query handles and input structures.
		var1 - the canonical name given to the buffer whose contents
			will contain the output of this query.

		
		"string1" and "string2" - these refer to verbatim input.
			NOTE: on subsequent queries, the output of a 
				previous query can become the input of
				current query.  The input variable is
				referenced through the same canonical name
				as was used in a previous output 
				statement.
@end(verbatim)

The complete operation of a DCM intrpretive cycle follows:
@begin(itemize, spread 1)

The input fields are checked to see if they exist given the query handle.
Two files are checked.  One file handle.h contains the mapping between
the query handle and the input structure name. Its format is:
@begin(verbatim)

		HANDLE.H
	HANDLE		STRUCT NAME
	  1, 		_struct1
	  2,		_struct2
	  3, 		_struct3
	  n, 		_structn

@end(verbatim)

Once a handle is associated with a given structure name, the file struct.h
is checked to see that the input fields map to the same fields in the
input structure definition.  This file defines the queries expected by every
request to the data base based on query handle.

The file struct.h has the following format:

@begin(verbatim)

	struct _struct1 {
		char *field1;
		char *field2;
	};

	struct _struct2 {
		char *field1;
		char *field2;
	};

	struct _struct3 {
		char *field1;
		char *field2;
	};

	struct _structn {
		char *field1;
		char *field2;
	};

@end(verbatim)

Provided the fields are located, a local structure is filled.  The structure
contains a snapshot of the local environment.  The structure, _qstruct,
provides the dcm with the ability to allocate and generate a memory block
which is identical to the expected input structure required by each
query request.  _Qstruct has the following components:

@begin(verbatim)

typedef struct _input {
	char type;	/* type of input verbatim or variable */	
	int input_offset;/* offset into current handle struct */
	char *input;	/* local input name */
	int where_from_handle;	/* where the input came from */
	int where_from_offset;
}INPUT;

typedef struct _qstruct {
	int lineno;	/* line number */
	int handle;	/* query handle */
	int structpos;	/* where the structure is in struct.h */
	INPUT *input[MAXVAR];	/* points to input vars */
	char *output;   /* local output name */
	struct _qstruct *next;	/* pointer to next struct */
	struct _qstruct *last;	/* pointer to previous struct */
}QSTRUCT;

@end(verbatim)

With the above structure filled, the dcm simply generates a memory
block with the exact components expected by the query.  This 
application allows for the addition of query handles and input
structures without having to recompile the dcm.  Basically,
by parsing the struct.h file and understanding the attributes
of a given structure, the DCM is capable of making a memory 
image of the structure and then passing the memory image and not 
the structure itself to the query routine.

The converse is applied on output.  The canonical output name is
used to map a memory image of the output structure.  This structure
stays resident throuought the DCM operation and can be referenced
by subsequent query requests.  

@SubSection(Server Description Files)

The server description files, or SDF, are files which contains a unique
description of each server SMS updates.

Hand created, these files hold information, in english-like syntax, 
which the DCM uses for
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

Each SDF will be accessed by a hostname/server combination.  This
combination will, in fact, be the search path on the source machine.
When the dcm needs a SDF to process, it will find a unique
server description file in the directory: /hostname/server

Part of the dcm is an interpreter which will parse the SDF and run a 
generalized syntax and logic check.  Assuming the file is syntactically
correct, the dcm will use the format of the SDF to generate a server
specific file.  

The SDF is comprised of a generallized syntax which allows the user to
create the information needs of any system server.  The limitations are
that the data must be in character format. 

The Server Description Language consists of key words and commands which the DCM
interprets.  The format of the files is line oriented and parsable.  An
example of an SDF is:

@Begin(Verbatim, LeftMargin +.5inch)

#one sharp sign is for this file only
##two sharp signs puts data in the target file too

#variables to be used

var name, attribute

begin header

	<"this is verbatim infomation">
	<"this is the header">

end header
	##query 1, all print clusters
	begin query
		#get all print clusters

		handle = 1 #this is the associated query handle to use
		input : NULL
			 #no input
		output : name
			#name is a declared variable having the 
			 structure of the query's associated output 
			 structure.
		<"cluster = ", name.prcluster>
			#verbatim info with passed in output variable
	end query
		#the above query will continue to query until all
		 fields are found.
	##
	##query 2, printers by cluster
	begin query #another query
		handle = 1  #get all print clusters
		input : NULL
		output : name
		begin query
			handle = 2 #get printers by cluster 
			input : prcluster = name.prcluster  
			#input the output of the previous query
			output : attribute

		<"print cluster = ", name.prcluster
        	 "printer name = ", attribute.printer
		 "printer type = ", attribute.type>
		end query
	end query

@end(verbatim)

The target file would look like the following:

#two sharp signs puts data in the target file too
#query 1, all print clusters
cluster = e40
cluster = sloan
cluster = admin

#query 2, printers by cluster
print cluster = e40
printer name = ln03-bldge40-3 
printer type = laser
printer name = ln03-bldge40-2
printer type = laser
printer name = ln03-bldge40-4
printer type = laser
print cluster = sloan
printer name = ln03-sloan-1 
printer type = laser
print cluster admin
printer name = juki-admin-1
printer type = daisy

@end(verbatim)

From a generic database the DCM will take the information, process it into
localized form, and cache it locally.  From here, the server update mechanism
grabs the file and serves it over the net.

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
