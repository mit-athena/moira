@part[db_analysis, root="sms.mss"]
@subsection(Input Data Checking)

The integrity of the data in an authoritative data system like SMS is
critical.  Without proper restrictive control, a user could easily enter
data which is either of the wrong type or of the wrong range.  For example,
if a user had intentions of updating a machine tuple.  If, instead of
typing VS2000 he typed in VS2002, the consequences throughout the
system would be overwhelming.  It is extremely important, therefore,
to recognize an input checking mechanism to secure the integrity of
the data.  

The method of checking input will be done through the SMS server.  
When a user makes a request to add or update
a field,  the server will check the input against a list of 
accepted values for that field.  Therefore,  each field which requires
input data integrity checking will be associated to a list of
values.  An error condition will return if the value specified is
incorrect.  For authorized users, the program ******* will
allow the addition or deletion of accepted field values.  

The list of predefined queries (addendum 1) gives reference to those fields
which require explicit data checking.


@subsection(Backup)

As an authoritative system, SMS will require a specific plan for
baking up the database.  
In the event of disk failure, software
disasters, and update mistakes, the backup mechanism will
allow full data restoration.  SMS will us an ascii backup/recover
scheme for data redundancy.  Every 24 hours, the SMS database will
be backed up onto a sister machine.  The data will be processed in ascii
format allowing for easy readability of the data contents.  If a failure 
should occur the data from the sister machine will be uploaded to 
the SMS machine, restoring the database to the previous backed up
state.  This backup mechanism will insure that the integrity of data will
always be less than 24 hours incorrect.  In addition to
providing reasonable insurance against gross data error, this backup
scheme isolates SMS from database-dependent backup schemes.  

A mechanism of getting effective data closer to the time of disaster is
being researched.  A possibility is to use the INGRES journaling
capability.  Every 24 hours, at database backup time, the journals
would be zeroed.  (This essentially re-initializes the journals.)
As the database is used the journals will be updated, providing 
a history of useage.  One feature here is to update the journals
to an NFS directory instead of to the SMS disk.  In this case, if the SMS
disk crashes, the NFS file will presumably be in tact.  The recovery 
will be to upload the ascii data and then use the journals to recover the
database to its last updated state prior to the crash.  Current discussion
will reveal how realistic the use of journaling will be in this instance.
