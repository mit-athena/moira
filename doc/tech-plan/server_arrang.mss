@part[db, root="sms.mss"]
@section(Server Arrangement)

Currently, sms acts to update a variety of servers.  Although the data 
control manager performs this update task, each server requires a 
different set of update parameters.  This section details the method 
by which every sms-supported server is updated.  In addition, a 
description of the data being sent and how it is retrieved is discussed.
The services which sms now supports are:
@begin(itemize, spread 1)
Hesiod - The athena nameserver.

RVD - Remote virtual disk.

NFS - Network file system.

MDQS - Multiple device queueing system.

Zephyr - The athena notification service.
@end(itemize)

@subsection(Server Assumptions)

The requirements of each server suggests a level of detail describing 
server location, update mechanism, data format, and file structure.
The following assumptions are identified:
@begin(itemize, spread 1)
Server : Hesiod
Update Interval : 6 Hours
Data Format : BIND
Locations : 
@\@\JASON.MIT.EDU
@\@\ZEUS.MIT.EDU
@\@\MENELAUS.MIT.EDU

Files Updated:
@\@\File Path: /etc/athena/nameserver
@\@\hesiod.db - Hesiod data 
@\@\query support:
@\@\@\
@\@\cluster.db - cluster data
@\@\query support:
@\@\@\@\@\service.db - services
@\@\passwd.db - password
@\@\query support:
@\@\@\@\@\printers.db - MDQS printer info
@\@\query support:
@\@\@\@\@\lpr.db - lpr printer info
@\@\printcap.db
@\@\query support:
@\@\@\@\@\pobox.db - post office info
@\@\query support:
@\@\@\@\@\sloc.db - service location
@\@\query support:
@\@\@\@\@\rvdtab.db - RVD info
@\@\query support:
@\@\@\@\@\zephyr.db - zephyr info
@\@\query support:
@\@\@\@\@\file_exchange.db - file exchange
@\@\query support:
@\@\@\

Update mechanism:

Updating hesiod is a relatively simple process.  Every six
hours the DCM will initiate a build on each of the above
files(assuming the informtion has changed).  Once a file
is constructed, the update mechanism will transport the file
to each of the above machine.   

