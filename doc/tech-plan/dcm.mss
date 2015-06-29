@part[dcm, root="moira.mss"]
@Section(The Data Control Manager)

The data control manager, or DCM, is a program responsible for
distributing information to servers.  Basically, the DCM is invoked
regularly by cron at intervals which become the minimum update time
for any service.  Whenever the DCM runs, it will determine which
services and hosts should be updated now.  The update frequency is
stored in the Moira database.  A server/host relationship is unique to
each update time.  Through the Moira query mechanism, the DCM extracts
Moira data and converts it to server dependent form.  The conversion of
the Moira data to the server-specific format is done by a sub-program
specific to that service.

@Subsection(DCM Operation)

On startup, the DCM first checks for the existance of the disable file
@i(/etc/nodcm); if this file exists, it exits quietly.  Next it
connects to the database and authenticates as @b(root).  Then it
retrieves the value of @b(dcm_enable) from the values relation of the
database; if this value is zero, it will exit, logging this action.

Next, the DCM scans the services table.  This table contains:

@begin(description)

Name@\The name of each service.

Type@\The type of service.  Currently defined service types are
@b(unique) and @b(replicat)ed.  This type affects some parts of the
update algorythm, described below.

ACE type and ACE name@\These specify the access control entity which
owns the service.  The type may be @b(list), @b(user), or @b(none).
The name will then be a list name, a login name, or @b(none),
respectively.  The owner is allowed to manipulate the service or
service/host tuples supporting that service.

Interval@\Gives the minimum time between updates of this service, in
minutes.

Target@\The name of the target file on the servers.  This is where Moira
will deposit the new configuration files.

Script@\This is the name of the script file on Moira which will be
executed on the server to install the new configuration files.

DFGen@\This is the time that the data files were last generated for
this service.  It is stored as a unix format time (number of seconds
since January 1, 1970 GMT).

DFCheck@\This is the time that the data files were last checked to see
if they needed to be regenerated.  It is stored as a unix format time.

Enable@\This boolean flag indicates if updates should be performed on
this service.  It may be set and cleared by the user.

InProgress@\This boolean flag indicates that an update is currently in
progress for this service.  It is set and cleared by only by the DCM.
It is @b(not) relyed upon for locking.

Harderror@\This field records the error number of any hard errors that
occur during an update. 

Errmsg@\This is a textual representation of the error reported in
@i(harderror).

ModTime@\The time this data was created or last modified.  This refers
only to modification by a user, not by the DCM.

ModBy@\The user name of who last modified this record.

ModWith@\The name of the application that was used for the last
modification.

@end(description)

Each time the DCM is invoked, a search through this table will
indicate which servers need updating.  It will first identify those
services which are @i(enable)d, do not have @i(hard errors), have a
non-zero @i(interval), and do have a generator module.  For each of
these services, it compares @i(dfcheck) and the update @i(interval)
against the current time.  If it is time for another update, it will
obtain an exclusive lock on the service, set the @i(inprogress) flag,
then run the generator.

The generator is a sub-program that does the actual extract.  Each
generator lives in @i(/u1/sms/bin/@p(service).gen).  A generator takes
as an argument the name of the output file it should generate.  It's
exit status will be zero on success, otherwise the number of any
error.  Note that a common ``error'' for a generator is
@t(MR_NO_CHANGE), indicating that nothing in the database has changed
and the data files were not re-built.

If the generator finishes without error, @i(dfgen) and @i(dfcheck) are
updated to the current time.  If the generator exits indicating that
nothing has changed, only @i(dfcheck) is updated to the current time.
If there is a soft error (an expected error that might go away if we
try again later) then the @i(error_message) is updated to reflect
this, but @i(hard error) is not set.  If there is a hard error, the
@i(hard_error) and @i(error_message) are set, and a zephyr message is
sent to class @b(MOIRA) instance @b(DCM) indicating this error.  After
this attempt, the lock on the service is released.

For each of the services which passed the initial check above
(@i(enable)d, no @i(hard errors), non-zero @i(interval), and a
generator exists) regardless of the result of attempting to build data
files, or even if it was time to build data files, the hosts will be
scanned.  The serverhost table contains:
@begin(description)

Service@\The name of the service.

Machine@\The name of the machine supporting this service.

Enable@\A boolean indicating that this host should be updated.

Override@\A boolean indicating that this host should be updated as
soon as possible, disregarding the service time interval.

Success@\A boolean indicating that the last attempted update was
successful.

InProgress@\A boolean indicating that this host is currently being
udpated. 

HostError@\This field recrods the error number of any hard errors that
occur during an update.

Errmsg@\This is a textual representation of the error report in
@i(hosterror).

LastTry@\This is the time that the DCM last attempted to update this
host.  It is stored as a unix format time.

LastSuccess@\This is the time of the last successful DCM update of
this host.  It is stored as a unix format time.

Value1@\This is service specific information, stored as an integer.
For POP servers, it is the number of poboxes assigned to this server.

Value2@\This is service specific information, stored as an integer.
For POP servers, it is the maximum number of poboxes that may be
assigned to this server.

Value3@\This is service specific information, stored as a string.  For
NFS servers, it indicates who should be in the credentials file.

ModTime@\The time this data was created or last modified.  This refers
only to modification by a user, not by the DCM.

ModBy@\The user name of who last modified this record.

ModWith@\The name of the application that was used for the last
modification.

@end(description)

During the host scan, the DCM first locks the service.  It will lock
it exclusively if the service @i(type) is @b(replicated), otherwise it
will acquire a shared lock.  Then the DCM makes a list of hosts which
are @i(enabled), do not have @i(hard errors), and have not been
successfully updated sine the data files were generated for this
service (or @i(override) is set).  It will then step through these
hosts, updating them.  The first part of an update is to obtain an
exclusive lock on the host and set the @i(inprogress) bit.  Then it
sends the generated file to the @i(target) file on the host and then
executes the @i(script) on that host.  The exit value of the script is
reported back, with zero being success and anything else being the
error number.  If the update is successful, the @i(last_time_tried)
and @i(last_time_successfull) are updated.  If there is a soft fail,
then just the @i(last_time_tried) is updated, and the
@i(error_message) is recorded.  If there is a hard failure, then a
soft fail is taken, plus @i(hard_error) is set and a zephyrgram and
mail are sent about it.  If there is a hard failure and the service is
@b(replicat)ed, then the error code & message are also set in the
service record so that no more updates will be attempted to hosts
supporting this service.  Then the host lock is freed.

When the host scan is complete, the service lock is freed, and the
service scan continues with the next service.

@Begin(Comment)

The data control manager acts as an interpreter on the SDF's, or as
an initiator of executable programs.  The basic
mechanism is for the DCM to read the above entry table, determine which
servers need updating and then locate the appropriate SDF for interpretation.
The breakdown of the SDF is a procedure based primarily on the associated
query handle and it's associated input and output structure.  The output of the
DCM is a file stored on the Moira host which is exactly the same
format of the server-based file.  The update mechanism takes this localized
data and ships it over the net. 

The DCM is capable of accepting either SDF or executable format.  Currently all
programs the DCM calls are written in C.
 
The DCM, therefore, is an application program designed to orchestrate
the distribution and generation of server-specific files.

@SubSection(Server Description Files)

The server description files, or SDF, are files which contains a unique
description of each server Moira updates.  The SDF description has
been developed to make support of Moira and the addition of
new services a reasonably easy process.  The demand of Moira support is
clearly rising.  Without an easy method of adding new servers, Moira
is circumvented in search for easier methods of service support.
To date, there are over 15 different files which need generation and
propagation.  With the support of zephyr this number will likely double
(propagation of at least 20 more different acls).

Hand created, these files hold information, in English-like syntax, 
which the DCM uses for
manipulating generic data into server specific form.  Accommodating additional
servers of primary server type requires, simply, adding 
a new SDF to the system.  The purpose of
the server description files is to provide a parseable, readable text files
for determining the structure and personality of a given server.  The three
reasons for SDF:

@Begin(Itemize, Spread 0)

To provide a local, uniform and expandable method of providing server
information to the Data Control Manager.

To maintain simplicity and readability.

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

The SDF is comprised of a generalized syntax which allows the user to
create the information needs of any system server.  The limitations are
that the data must be in character format. 

The Server Description Language consists of key words and commands which the DCM
interprets.  The format of the files is line oriented and parseable.  An
example of an SDF is:

@Begin(Verbatim, LeftMargin +.5inch)

#one sharp sign is for this file only
##two sharp signs puts data in the target file too

#variables to be used

var name, attribute

begin header

	<"this is verbatim information">
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

@end(comment)
