@part[db, root="moira.mss"]
@subsection(Input Data Checking)

Without proper checks on input values, a user could easily enter data
of the wrong type or of a nonsensical value for that
type into Moira.  For example, consider the case of updating a user's mail
address.  If, instead of typing e40-po (a valid post office server),
the user typed in e40-p0 (a nonexistant machine), all the user's mail
would be "returned to sender" as undelivereable. 

Input checking is done by both the Moira server and by applications
using Moira.  Each query supported by the server may have a validation
routine supplied which checks that the arguments to the query are
legitimate.  Queries which do not have side effects on the database do
not need a validation routine.

Some checks are better done in applications programs; for example, the
Moira server is not in a good position to tell if a user's new choice
for a login shell exists.  However, other checks, such as verifying
that a user's home directory is a valid filesystem name, are conducted
by the server.  An error condition will be returned if the value
specified is incorrect.  The list of predefined queries (Section
@Ref(Queries)) defines those fields which require explicit data
checking. 

@subsection(Backup)
@comment[Rewritten 7/13 WES]
It is not absolutely critical that the Moira database be available 24
hours a day; what is important is that the database remain internally
consistant, and that the bulk of the data not be lost.  With that in
mind, the database backup system for Moira has been set up to maximize
recoverability in the event of damage to the database.

Two programs (@t[mrbackup] and @t[mrrestore]) are generated
automatically (using an @t[awk] script) from the database description
file @t[db/newdb]@footnote[All pathnames are relative to the
root of the Moira source tree].  @t[mrbackup] copies each relation of
the current Moira database into an ASCII file.  Each row of the database
is converted into a single line of text; each line consists of several
colon separated fields followed by a newline character (ASCII code 10
decimal).  Colons and backslashes inside fields are replaced by \: and
\\, respectively.  Non-printing characters are replaced by \nnn, where
nnn is the octal value of the ASCII code for the non-printing
character.  The full database dump takes roughly 12 minutes with the
current (albeit partially-populated) database; the ascii files take up
about 3.2 MB of space.  It is intended that mrbackup be
invoked by a shell script run periodically by the @t[cron] daemon;
this shell script (currently called @t[nightly.sh]) maintains the last
three backups on line.
It is intended that these backups be put on a separate physical disk
drive from the drive containing the actual database.  Also, they
should be dumped to tape using @t[tar], or copied to another machine,
on a regular basis.  Whether they should be dumped to TK50 or
reel-to-reel tape is open to discussion at this point. 

@t[mrrestore] does the inverse of @t[mrbackup].  It requires the
existance of a empty database named "smstemp"@footnote(The eventual
production version will work on a database named "sms"; however, for
test use, "smstemp" is used instead), created as follows:
@begin(programexample, Size -1) 
# createdb smstemp
# quel smstemp
* \i /mit/moira/src/db/newdb		@i(load DB definition)
* \g					@i(execute DB definition)
* \q					@i(quit)
# mrrestore
Do you *REALLY* want to wipe the Moira database? (yes or no): yes
Have you initialized an empty database named smstemp? (yes or no): yes
Opening database...done
Prefix of backup to restore: /site/sms/backup_1/
Are you sure? (yes or no): yes
Working on /site/sms/backup_1/users
	...
@end(programexample)

This system by itself provides recovery with the loss of no more than
roughly a day's transactions.  To improve this, the journal file kept by
the Moira server daemon contains a listing of all successful changes to
the database.

RTI Ingres provides some checkpointing and journalling facilities.
However, past experience with them has shown that they are not
particularly reliable.  Also, a common failure mode, at least with
version 3 of RTI ingres, has been corruption in the binary structure
of the database.  Since the checkpointing mechanism used is simply a
@t[tar] format copy of the database directory, restoring from the
checkpoint will probably not cure the corruption, particularly since
they may go for days without being noticed.  The only known cure is to
dump the entire database to text files, and recreate it from scratch
from the text files.  Because of this dubious history, it was decided
that the RTI checkpoint and journalling mechanism was not sufficiently
reliable for use with Moira.

