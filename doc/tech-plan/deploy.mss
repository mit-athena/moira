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

Because Athena will not support time sharing systems in the future, SMS will
not have any responsibility supporting time sharing.  This then assumes that
the old database will support the time sharing environment until Athena no
longer recognizes time sharing.

The following development schedule is planned:

@Begin(Itemize)

March 12 - 27 : Finish the technical plan.  Distribute to Athena staff and
gather comments.

March 30 - April 10 : Begin design and implementation of SMS data base and
Data Control Manager Operation.  Study the process of bringing information
over from Athenareg.

April 13 - 24 : Start the first implementation of SMS database.  Use INGRES
to build tuples and corresponding fields.  Study the actual use of remote
administrative operation and determine the priority of the SMS protocol.
Begin DCM development.(April 20 pjlevine runs BAA Marathon).

April 27 - May 8 : Database development is at the point where clients can
begin to access information.  DCM development continues as it can be tested
with queries made to SMS database.  SMS protocol defined and development of
SMS server begins.

May 11 - 25 : Database development finalizing with real data being ported
from Athenareg.  Begin to see need for application management tools.  Speak
with operations regarding the use of specialized tools needed for effective
data manipulation.  SMS server and remote applications defined.  Application
library functions studied. Local management application developed for
manipulating SMS data on the SMS machine.

June 1 - 12 : Database complete.  Begin thorough testing and debugging of
the database and its effectiveness. Limited DCM operation: feeds information
to Hesiod, the Athena name server.  SMS server and protocol see limited use
as they are tested to support remote applications.  Remote applications use
parts of the application library. All network operation uses Kerberos for
data security. Operations begin to manipulate, locally, SMS data directly
through management interface.

June 15 - 26 : Database is tested and fully operational.  Study the effects
of adding new items as the Athena plant becomes more complex.  SMS server
near completion with remote applications performing limited tasks.  The DCM
becomes more robust and starts to feed information to other services.

June 29 - July 10 : Operations update all relevant fields in SMS along with
maintaining athenareg.  The DCM now supports more servers and has the
capability of manipulating generic information into many different site
specific formats.  SMS protocol and server complete.  SMS capable of
supporting remote application.

** July 1 - Operational SMS for deployment.  Capable of supporting system
servers.  Current user utilities (i.e., chhome, madm, finger, etc.)
converted to use SMS application library.

July 13 - 31 : Application library documentation begins.  Remote
applications are developed in response to the needs of the community.
Operations recognize the use of more tools and begin to rely on SMS
exclusively service management.

August 3 - 14 : Testing and debugging phase for remote applications and
management tools.  Begin to see what the next phase of development SMS will
go through.  Documentation and support of product begins.

August 17 - 28 : Time sharing becomes a thing of the past.  SMS is used
exclusively for Athena service management.  Begin next development phase.

@End(Itemize)

The above schedule reflects an optimal approach toward SMS development and
deployment.  Certainly, as fundamental dates slip, sequential time deadlines
become more unrealistic.  The constant evolution of the product during its
development phase will also reflect much added detail to this technical
plan.

@MajorSection(Long Term Support Strategy)

The eventual use and application of SMS will be by the operations component
of the Athena organization.  The support of SMS will be left to the
operations and informations systems departments.

@Verbatim[more to come here after talk w/ geer]
