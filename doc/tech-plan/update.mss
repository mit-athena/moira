@part[update, root="moira.mss"]
@Section(Moira-to-Server Update Protocol)

Moira provides a reliable mechanism for updating the servers it manages.
The use of an update protocol allows the servers to be reliably
managed.  The goals of the server update protocol are:

@Begin(Itemize)

Completely automatic update for normal cases and expected kinds of failures.

Survives clean server crashes.

Survives clean Moira crashes.

Easy to understand state and recovery by hand.

@End(Itemize)

General approach: perform updates using atomic operations only.  All
updates should be of a nature such that a reboot will fix an
inconsistent database.  (For example, the RVD database is sent to the
server upon booting, so if the machine crashes between installation of
the file and delivery of the information to the server, no harm is
done.)  Updates not received will be retried at a later point until
they succeed.  All actions are initiated by the DCM.

@SubHeading(Strategy)

@begin(Enumerate)

Transfer phase.  This step puts all of the files on the server.
@begin(Enumerate)

Connect to the server host and send authentication.

Transfer the data files to be installed to the server.  These are
stored in the @i(target) as recorded for the service in the Moira
database.  The file transfer includes a checksum to insure data
integrity.  Only one file is transferred, although it may be a tar
file containing many more.

Transfer the installation instruction sequence to the server.  This is
the @i(script) as recorded for the service in the Moira database.  It
will be stored in a temporary file on the server.

Flush all data on the server to disk.

@end(Enumerate)

Execution phase.  If all portions of the preparation phase are
completed without error, the execution phase is initiated by the DCM.
On a single command from the Moira, the server begins execution of the
instruction sequence supplied.  These can include the following:
@begin(Enumerate)

Extract data files from the tar file.  Rather than extract all of the
files at once, only the ones that are needed are extracted one at a
time.

Swap new data files in.  This is done using atomic filesystem
@t(rename) operations.  The cost of this step is kept to an absolute
minimum by keeping both files in the same partition and by not
changing the link count on any files during the rename.

Revert the file -- identical to swapping in the new data file, but
instead puts the old file back.  May be useful in the case of an
erroneous installation.

Send a signal to a specified process.  The process_id is assumed to be
recorded in a file; the pathname of this file is a parameter to this
instruction.  The process_id is read out of the file at the time of
execution of this instruction.

Execute a supplied command.

@end(Enumerate)

Confirm installation.  The server sends back a reply indicating that
the installation was successful, or what error occurred.  The DCM then
records this information in the database.

@end(Enumerate)

@SubHeading(Trouble Recovery Procedures)

@begin(Enumerate)
@begin(Multiple)

Server fails to perform action.

If an error is detected in the update procedure, the information is
relayed back to the DCM.  The 'success' flag is cleared, and the error
code and message are recorded in the update table.  The error value
returned is logged to the appropriate file; zephyr is used to notify
the system maintainers a failure occured.

A timeout is used in both sides of the connection during the preparation
phase, and during the actual installation on the Moira.  If any single
operation takes longer than a reasonable amount of time, the connection
is closed, and the installation assumed to have failed.  This is to
prevent network lossage and machine crashes from causing arbitrarily
long delays, and instead falls back to the error condition, so that the
installation will be attempted again later.  (Since the all the data
files being prepared are valid, extra installations are not harmful.)

@end(Multiple)
@begin(Multiple)

Server crashes.

If a server crashes, it may fail to respond to the next attempted Moira
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
Moira, and the existing @t(filename.moira_update) file will be deleted (as
it may be incomplete) when the next update starts.

@end(Multiple)
@begin(Multiple)

Moira crashes.

Since the Moira update table is driven by absolute times and offsets,
crashes of the Moira machine will result in (at worst) delays in updates.
If updates were in progress when the Moira crashed, those that did not
have the install command sent will have a few extra files on the
servers, which will be deleted in the update that will be started the
first time the update table is scanned for updates due to be performed.
Updates for which the install command had been issued may get repeated
if notification of completion was not returned to the Moira.

@end(Multiple)
@end(Enumerate)

@comment[Should this be someplace else?]

@SubHeading(Considerations)

What happens if the Moira broadcasts an invalid data file to servers?
In the case of name service, the Moira may not be able to locate the
servers again if the name service is lost.  Also, if the server
machine crashes, it may not be able to come up to full operational
capacity if it relies on the databases which have been corrupted; in
this case, it is possible that the database may not be easily
replaceable.  Manual intervention would be required for recovery.

@SubSection(Catastrophic Crashes - Robustness Engineering)

In the event of a catostrophic system crash, Moira must have the
capability to be brought up with consistent data.  There are a list of
scenarios which indicate that a complete set of recovery tools are
needed to address this isssue.  Thought will be given in order that
the system reliably is restored.  In many cases, the answer to a
catastrophic crash will be manual intervention.

@SubSection(Data Transport Security)

Kerberos is used to verify the identity of both ends at connection
set-up time.  The data will not be encrypted.  Since a TCP stream is
used, the connection should be secure from tampering.  This will allow
detection of lost or damaged packets, as well as detection of
deliberate attempts to damage or change data while it is in transit.
