@Comment[
	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/doc/tech-plan/dcm.mss,v $
	$Author: spook $
	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/doc/tech-plan/dcm.mss,v 1.8 1987-06-01 10:51:22 spook Exp $

	$Log: not supported by cvs2svn $
Revision 1.7  87/05/29  18:26:53  ambar
fixed scribe error.

Revision 1.6  87/05/29  17:57:42  ambar
added to the section on security of datagrams.

Revision 1.5  87/05/29  17:47:48  ambar
replaced "dcm" with "DCM" for consistency.

Revision 1.4  87/05/29  14:29:29  ambar
more changes from Peter.

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
query mechanism, the DCM extracts sms data and converts it to server
dependent form.  The conversion of database specific information to site
specific information is done through a server description file, a
sms-unique language which is used to describe the personality of a
target service.

When invoked the DCM will perform some preliminary operations to
establish the relationship between the sms data and each server.
The very first time the DCM is called, a table is constructed 
describing the relationship between servers and update frequency.
The table will be the primary mechanism used by the DCM for recognizing
the servers which need updating at given times.  As a note here,
crontab will invoke the DCM at a pre-established time interval,
say every 15 minutes.  Obviously, the maximum update time will be limited to
the time interval the DCM is being invoked at.  Every interval,
the DCM will search the constructed table and determine whether or 
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
bogus data and needs updating immediately.  After the DCM uses this
value, the field is reset to -1.
		Value: -1 - Use established time interval.
			0 and greater - New once-used interval.

@i[Enable] - This switch allows the authorized user to turn the update
facility on or off.  
		Value: 0 - Off, Non-zero - On.
@end(itemize)

Each time the DCM is invoked, a search through this table will 
indicate which servers need updating.  Once located, the DCM will
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
provides the DCM with the ability to allocate and generate a memory block
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

With the above structure filled, the DCM simply generates a memory
block with the exact components expected by the query.  This 
application allows for the addition of query handles and input
structures without having to recompile the DCM.  Basically,
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
When the DCM needs a SDF to process, it will find a unique
server description file in the directory: /hostname/server

Part of the DCM is an interpreter which will parse the SDF and run a 
generalized syntax and logic check.  Assuming the file is syntactically
correct, the DCM will use the format of the SDF to generate a server
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

		handle = 1 #this is the associated query handle
		input : NULL
			 #no input
		output : name
			#name is a declared variable having the 
			#structure of the query's associated
			#output structure.
		<"cluster = ", name.prcluster>
			#verbatim info with passed in
			#output variable
	end query
		#the above query will continue to query until
		#all fields are found.
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

@begin(verbatim)
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

SMS provides a reliable mechanism for updating the servers it manages.
The use of an update protocol allows the servers to be reliably
managed.  The goals of the server update protocol are:

@Begin(Itemize)

Completely automatic update for normal cases and expected kinds of failures.

Survives clean server crashes.

Survives clean SMS crashes.

Easy to understand state and recovery by hand.

@End(Itemize)

General approach: perform updates using atomic operations only.  All
updates should be of a nature such that a reboot will fix an
inconsistent database.  (For example, the RVD database is sent to the
server upon booting, so if the machine crashes between installation of
the file and delivery of the information to the server, no harm is
done.)  Updates not received will be retried at a later point until
they succeed.  All actions are initiated by the SMS.

@SubHeading(Strategy)

@define(en=enumerate)
@define(it=itemize)
@define(m=multiple)
@define(f, facecode f)

@begin(en)
@begin(m)

Preparation phase.  This phase is initiated by the SMS when it
determines that an update should be performed.  This can be triggered by
a change in data, by an administrator, by a timer (run by cron), or by a
restart of the SMS machine.

@begin(en)

Check whether a data file is being constructed for transmission.  If
so, do nothing and report the fact.  Otherwise, build a data file from
the SMS database.  (Building the data file is handled with a locking
strategy that ensures that "the" data file available for distribution
is not an incomplete one.  The new data file is placed in position for
transfer once it is complete using the @f(rename) system
call.)@foot(Does this mean that SMS can only update one file at any
one time? @foot(No, it should all be on a file-by-file basis. -- KR)
-- AMBAR)

Extract from SMS the list of server machines to update, and the
instructions for installing the file.  Perform the remaining steps
independently for each host.

Connect to the server host and send authentication.

Transfer the files to be installed to the server.  These are stored in
@f(filename.sms_update) until the update is actually performed.  At
the same time, the existing file is linked to @f(filename.sms_backup)
for later deinstallations, and to minimize the overhead required in
the actual installation (freeing disk pages).  (The locking strategy
employed throughout also ensures that this will not occur twice
simultaneously.)  A checksum is also transmitted to insure integrity
of the data.

Transfer the installation instruction sequence to the server.

Flush all data on the server to disk.

@end(en)
@end(m)
@begin(m)

Execution phase.  If all portions of the preparation phase are
completed without error, the execution phase is initiated by the SMS.

On a single command from the SMS, the server begins execution of the
instruction sequence supplied.  These can include the following:

@begin(en)

Swap new data files in.  This is done using atomic filesystem
@f(rename) operations.  The cost of this step is kept to an absolute
minimum by keeping both files in the same directory and by retaining
the @f(filename.sms_backup) link to the file.

Revert the file -- identical to swapping in the new data file, but
instead uses @f(filename.sms_backup).  May be useful in the case of an
erroneous installation.

Send a signal to a specified process.  The process_id is assumed to be
recorded in a file; the pathname of this file is a parameter to this
instruction.  The process_id is read out of the file at the time of
execution of this instruction.@foot(Does this mean that we're going to
have to be keeping around lots more @f(daemon.pid) files than we do
now? @foot(Yes, but it's either that or read kernel data structures
for the info, and have to recompile the update program with each new
kernel. -- KR)
-- AMBAR)

Execute a supplied command.

@end(en)
@end(m)

Confirm installation.
The server sends back a reply indicating that the installation was
successful.  The SMS then updates the last-update-tried field of the
update table, clears the override value, and sets the 'success' flag.

@end(en)

@SubHeading(Trouble Recovery Procedures)

@begin(en)
@begin(m)

Server fails to perform action.

If an error is detected in the update procedure, the information is
relayed back to the SMS.  The last-update-tried flag is set, and the
'success' flag is cleared, in the update table.  The override value
may be set, depending on the error condition and the default update
interval.

The error value returned is logged to the appropriate file; at some
point it may be desirable to use Zephyr to notify the system maintainers
when failures occur.

A timeout is used in both sides of the connection during the preparation
phase, and during the actual installation on the SMS.  If any single
operation takes longer than a reasonable amount of time, the connection
is closed, and the installation assumed to have failed.  This is to
prevent network lossage and machine crashes from causing arbitrarily
long delays, and instead falls back to the error condition, so that the
installation will be attempted again later.  (Since the all the data
files being prepared are valid, extra installations are not harmful.)

@end(m)
@begin(m)

Server crashes.

If a server crashes, it may fail to respond to the next attempted SMS
update.  In this case, it is (generally) tagged for retry at a later
time, say ten or fifteen minutes later.  This retry interval will be
repeated until an attempt to update the server succeeds (or fails due to
another error).

If a server crashes while it is receiving an update, either the file
will have been installed or it will not have been installed.  If it
has been installed, normal system startup procedures should take care
of any followup operations that would have been performed as part of
the update (such as [re]starting the server using the data file).  If
the file has not been installed, it will be updated again from the
SMS, and the existing @f(filename.sms_update) file will be deleted (as
it may be incomplete) when the next update starts.

@end(m)
@begin(m)

SMS crashes.

Since the SMS update table is driven by absolute times and offsets,
crashes of the SMS machine will result in (at worst) delays in updates.
If updates were in progress when the SMS crashed, those that did not
have the install command sent will have a few extra files on the
servers, which will be deleted in the update that will be started the
first time the update table is scanned for updates due to be performed.
Updates for which the install command had been issued may get repeated
if notification of completion was not returned to the SMS.

@end(m)
@end(en)

@comment[Should this be someplace else?]

@SubHeading(Considerations)

What happens if the SMS broadcasts an invalid data file to servers?
In the case of name service, the SMS may not be able to locate the
servers again if the name service is lost.  Also, if the server
machine crashes, it may not be able to come up to full operational
capacity if it relies on the databases which have been corrupted; in
this case, it is possible that the database may not be easily
replacable.  Manual intervention would be required for recovery.

@SubSection(Data Transport Security)

Each datagram which is transmitted over the network is secure using
Kerberos, the Athena authentication system.  Encyphered information
headers will proceed each datagram.  The servers decrypt the header
information and use the packet accordingly.

Data going over the net will be checksummed before it is sent.  This
checksum will be put in a small encrypted "header", which will be
decoded on the receiving side.  This will allow detection of lost or
damaged packets, as well as detection of deliberate attempts to damage
or change data while it is in transit.

Encryption of the data itself is an option that can be invoked depending
on the sensitivity of the data involved.  For instance, files such as
/etc/rvdtdab are not particularly secret, but the MIT id numbers of
users are.
