Comment[
	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/doc/tech-plan/dcm.mss,v $
	$Author: pjlevine $
	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/doc/tech-plan/dcm.mss,v 1.14 1987-08-05 18:02:04 pjlevine Exp $

	$Log: not supported by cvs2svn $
Revision 1.13  87/08/05  17:19:52  pjlevine
*** empty log message ***

Revision 1.12  87/06/19  16:20:55  spook
Removed update stuff (moving to update.mss).

Revision 1.11  87/06/19  11:27:34  pjlevine
pjlevine adds words of wisdom

Revision 1.10  87/06/02  15:42:45  ambar
spelling fixes.

Revision 1.9  87/06/01  16:27:39  ambar
consistency checks.

Revision 1.8  87/06/01  10:51:22  spook
Merged update.mss into dcm.mss where it belongs.

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
server.  The update frequency is stored in the SMS database.  A
server/host relationship is unique to each update time.  Through the SMS
query mechanism, the DCM extracts SMS data and converts it to server
dependent form.  The conversion of database specific information to site
specific information is done through a server description file, a
SMS-unique language which is used to describe the personality of a
target service.

When invoked the DCM will perform some preliminary operations to
establish the relationship between the SMS data and each server.
The very first time the DCM is called, a table is constructed (as a disk file) 
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
successful update occurred.  This time will be used against 
the current time to determine if the interval criteria has been met.

@i[Success] - Flag for indicating whether or not the last time tried was
successful. 0-fail, 1-success

@i[Time interval] - Derived from the SMS database.  Gives the interval
update time for each server's information needs.

@i[Server] - This is the server name.  Derived from SMS database.

@i[Hostname] - This is the host name where the server resides. Derived from
the SMS database.

@i[Target Pathname] - Gives the location of the file which needs to be 
updated on the target or server end.  Derived from the SMS database.

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

The data control manager acts as an interpreter on the SDF's, or as
an initiator of executable programs.  The basic
mechanism is for the DCM to read the above entry table, determine which
servers need updating and then locate the appropriate SDF for interpretation.
The breakdown of the SDF is a procedure based primarily on the associated
query handle and it's associated input and output structure.  The output of the
DCM is a file stored on the SMS host which is exactly the same
format of the server-based file.  The update mechanism takes this localized
data and ships it over the net. 

The DCM is capable of accepting either SDF or executable format.  Currently all
programs the DCM calls are written in C.
 
The DCM, therefore, is an application program designed to orchestrate
the distribution and generation of server-specific files.

@SubSection(Server Description Files)

The server description files, or SDF, are files which contains a unique
description of each server SMS updates.  The SDF description has
been developed to make support of SMS and the addition of
new services a reasonably easy process.  The demand of SMS support is
clearly rising.  Without an easy method of adding new servers, SMS
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
