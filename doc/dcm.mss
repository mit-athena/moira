@device(postscript)
@Heading(The DCM)
@Center(Draft of @value(date))

The Data Control Manager, or DCM, is the part of Moira that keeps all of
the system servers updated with current data from the Moira database.
It is run periodically by the cron daemon.  Currently this is every 15
minutes.  It's operation may be controlled external to Moira by the
presence of @i(/etc/nodcm), or through Moira by the value of
@b(dcm_enable) in the database.

@SubHeading(Options)

The debug level is set to the value of the environment variable
@b(DEBUG).  If the command line argument @b(-d) @i(level) is present,
this will override the environment value.  If neither is given, the
debug level is 0.  The following bits in the debug level are
significant: DBG_PLAIN = 01, DBG_VERBOSE = 02, DBG_TRACE = 04.

All logging will be done to @i(/u1/sms/dcm.log) (actually, logging is
done to standard output, and @i(startdcm) captures this and directs it
to the log file).

@SubHeading(Operation)

On startup, the DCM first checks for the existence of @i(/etc/nodcm);
if this file exists, it exits quietly.  Next it connects to the
database and authenticates as @b(root).  Then it retrieves the value
of @b(dcm_enable) from the values relation of the database; if this
value is zero, it will exit, logging this action.

It then scans each of the services which:
@itemize{
are @i(enable)d 

do not have @i(hard_errors)

the @i(update_interval) is non-zero

@i(/u1/sms/bin/@p(service).gen) exists

} It first compares @i(dfcheck) and the @i(update_interval) against
the current time.  If it is time for an update, it will get an
exclusive lock on the service, then run
@i(/u1/sms/bin/@p(service).gen), instructing it to produce
@i(/u1/sms/dcm/@p(service).out).  If this finishes without error,
@i(dfgen) and @i(dfcheck) are updated to the current time.  If it
exits indicating that nothing has changed, only @i(dfcheck) is updated
to the current time.  If there is a soft error (an expected error that
might go away if we try again) then the @i(error_message) is updated
to reflect this, and nothing else is changed.  If there is a hard
error, the @i(hard_error) and @i(error_message) are set, and a zephyr
message is sent to class @b(MOIRA) instance @b(DCM) and a mail message
to the @i(dcm_error) address.  After this attempt, the lock on the
service is released.

For each of the services which pass the 4 checks above, regardless of
the result of attempting to build data files, or even if it was not
time to build data files, the hosts will be scanned.  If the service
is of @i(type) @b(REPLICAT)ED, then an exclusive lock is obtained on
the service; otherwise (the service is @b(UNIQUE)) a shared lock is
obtained on the service.  Then the hosts are scanned to find those
which:
@itemize{
are enabled

do not have hard errors

have not been successfully updated since the data files were generated

} For each of these hosts, if the last try was not @i(success)ful, or
the host @i(override) is set, or it has been at least
@i(update_interval) minutes since the last update, then an update is
attempted.  First an exclusive lock is obtained on the host/service
combination.  Then the update occurs, with the data and @i(script)
being sent and then the @i(script) executed.  If the update is
successful, the @i(last_time_tried) and @i(last_time_successful) are
updated.  If there is a soft fail, then just the @i(last_time_tried)
is updated, and the @i(error_message) is recorded.  If there is a hard
failure, then a soft fail is taken, plus @i(hard_error) is set and a
zephyrgram and mail are sent about it.  If there is a hard failure and
the service is @b(REPLICAT)ED, then the error code & message are also
set in the service record so that no more updates will be attempted to
hosts supporting this service.  Then the host lock is freed.

When the host scan is complete, the service lock is freed, and the
service scan continues.

@SubHeading(Locks)

The DCM locking is performed using unix @i(flock).  For each service,
the lock file is named @i(/u1/sms/dcm/locks/@@db@@.@p(service)), and
each machine/service tuple's lockfile is
@i(/u1/sms/dcm/locks/@p(machine).@p(service)).  These are zero-length
files which are flocked in the appropriate modes by the DCM.

All locks are obtained non-blocking, so if the lock isn't available,
the DCM will go on to do other work.  Locks may be shared or
exclusive, as specified in the Operations section above.


@SubHeading(Queries)

During initialization, a @i(get_value) and a @i(get_alias) query are
performed.

Service scanning consists of a @i(qualified_get_server), followed by
many @i(get_server_info) and @i(set_server_internal_flags) queries.

Host scanning consists of a @i(qualified_get_server_host), followed
by many @i(get_server_host_info) and @i(set_server_host_internal)
queries.

Note that the only changes to the database are made through
@i(set_server_internal_flags) and @i(set_server_host_internal), both
of which do not set the modification fields on the records they touch
or in the table statistics.

@SubHeading(Files)

The source is primarily in @i(/mit/moiradev/dcm/dcm.c).  Some of the
utility routines (such as locking and mail notifications) are in
@i(/mit/moiradev/dcm/utils.c).  The routines to perform a server update
are in @i(/mit/moiradev/update/client.c) and associated files in that
directory.

The DCM links against libmoiraglue so that it touches the database
directly rather than go through the Moira server.

@SubHeading(Data File Generators)

There must be one of these for each service that is supported.  Each
generator is an executable program stored in
@i(/u1/sms/bin/@p(service).gen), where @p(service) is the service name
in lower case.  The DCM will invoke it with one argument, the name of
the output file it should produce.  Most of the generators will write
to standard output if no output file is specified, although this is
not required.  The exit status should be a value from <mr_et.h>.  It
must be a value from this set, because only the least significant 8
bits of the return code are available to the parent process, so the
com_err table identifier is lost.  In particular, if nothing has
changed and the files do not need to be re-generated, MR_NO_CHANGE is
returned.

Each generator is different, although there are some conventions.
Most of them check the mod-time on the target file, and compare this
with the mod-times in the tblstats relation in the database for each
table that affects that file.  This determines if the data files need
to be regenerated.  Some of the generators produce a tar file
containing several other files.  These have their working directory
hard-coded in, usually @i(/u1/sms/dcm/@p(service)).  Each file that is
produced is written to a different name (usually the desired name with
a twiddle appended), then a rename is performed if the file is
produced successfully.  In this way, if part of the generation fails,
whatever is left should be a set of complete files.

The source to the generators lives in @i(/mit/moiradev/gen).

