
@part[deploy, root "sms.mss"]
@MajorSection(Deployment, Integration, and Scheduling)

The integration of SMS into the existing environment is as important as the
design of SMS itself.  The factors involved in deploying SMS into the
current system revolve around the system's changing environment and
increased demand for system resources.  For SMS to be a functional tool in
the Athena environment, it must allow the system administrator, from the
start, a method of accessing and controlling data.

SMS's deployment will occur in conjunction with the existing administrative
process.  The current database will be used as SMS begins to take on
responsibiliy.  Basically, SMS will take on more responsibility as the
current system relinquishes responsibility.  Administrative changes,
therefore, must be carefully monitored so that the SMS fields which are
"on-line" receive the most relevant data.

Because Athena will not support time sharing systems in the future, SMS
will not support time sharing.  This then assumes that the old database
will support the time sharing environment as long as it still
exists.

@subsection(Deployment)

SMS is a system which does not operate in peices.  For every service,
there is a corresponding application interface.  For every service,
there is a field in the database; and, for every service, there
is a consumer.  The deployment and testing of SMS is a difficult
and sensitive process.  Basically, once SMS is committed to
support Athena, everything must be in place.  There is no redundancy.
For example, the support of hesiod alone requires seven application
programs and the generation and propagation of
nine different files.  Tetsing the system components individually 
is a relatively
straight forward process; testing the complete system and 
simulating real usage patterns is much more difficult.  At this time,
a plan is being drafted which will present the testing and deployment
schedule for SMS.

A suitable plan, under consideration, is to provide a dummy database
which will be a test bed for SMS.  Staff members at Athena will be
notified of this database.  For a week's time this database will be
tested as a dummy, allowing staff time to flush out bugs.  ALL
CHANGES MADE TO THIS DATABASE WILL NOT BE PERMANENT.  This means
that if I change my 'finger' information in this dummy database,
it will not be affected in the real world.  Testing must first occur in a 
closed-cell environment, where potential snags will not disrupt
the day-to-day operations of Athena.  Each application program
will be tested in as realistic of an environment as possible.
Userreg, the new student registration program, is especially
important.  This program will see great demand during the
first week of school and therefore must work without a problem.
There will be no time for debugging userreg after school starts.
Therefore, userreg will be stressed by having many staff members
try to use the program all at the same time.  This will attempt to
simulate a worst case scenario of operation.

The deployment plan will be flushed out forthcoming.

@MajorSection(Long Term Support Strategy)

The eventual users of SMS will be the operations component of the Athena
organization.  The support of SMS will be left to the operations and
informations systems groups.
