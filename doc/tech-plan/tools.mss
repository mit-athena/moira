@part[tools, root "sms.mss"]
@MajorSection(Specialized Management Tools)

SMS will include a set of specialized management tools to enable system
adminstrators to control system resources.  As the system evolves, more
management tools will become a part of the SMS's application program
library.  These tools provide the fundamental administrative use of SMS.

In response to complaints about the user interface of current database
maintenance tools such as madm, gadm, and (to a lesser extent) register,
the SMS tools will use a slightly different strategy.  To accomodate
novice and occasional users, a menu interface similar to the interface
in register will be the default.  For regular users, a command-line
switch (such as @t(-nomenu)) will be provided that will use a
line-oriented interface such as those in discuss and kermit.  This
should provide speed and directness for users familiar with the system,
while being reasonably helpful to novices and occasional users.

@Begin(Itemize, Spread 1)
@Begin(Multiple)

RVD - Create/update an RVD. This administrative program will allow for
the master copy of rvddb's to be updated and created.  The DCM will
distribute the RVD information automatically to the servers requiring
RVD data.  Presently, the system administrator keeps an up-to-date file
of RVD data and then copies the data to the RVD server. The format of
the RVD server database:

@Begin(Verbatim, LeftMargin +.5inch)
operation = add_physical |
filename = /dev/         |
blocks= 		 | - This is the header 
created=		 |   unique to each 
modified=		 |   physical disk on a machine
operation=add_virtual	 |
physical=		 |
name=			 |
owner=			 |
rocap=			 | - This is the information
excap=			 |   unique to each virtual
shcap=			 |   disk.
modes=			 |  
offset=			 |   Block gets repeated n 
blocks=			 |   times.
created=		 |   Where n is the number
modified=		 |   of allocated RVDs on a 
ownhost=		 |   physical disk.
uid=			 |
@End(Verbatim)

By providing to the DCM the required data structure for the RVD servers,
the present file /site/rvd/rvddb can be removed.@foot(Hmmm.  What
happens when an RVD server reboots?)  The DCM will automatically load
the RVD server with information.

@End(Multiple)

RVD - Adding a user, project, or course to a RVD.  This program will
allow the administrator to associate a user, project, or a course to an
RVD pack.  Right now, each workstation has the file /etc/rvdtab which is
manually updated by the operations staff.  By associating a course to a
RVD in the SMS database, Hesiod, the Athena name server, will be able to
find arbitrary RVD information, and the system will no longer require
/etc/rvdtab.

@Begin(Multiple)

User Registration - Adding/changing user information.  Presently, there
are two programs which the system administrator uses to register a new
user: register and admin.  These programs register the user and enter
the private key information to Kerberos, respectively.  The new
application will provide these, and offer a third feature which allows
the administrator to check the fields of the SMS database and verify
that all of the database fields are correct.  (Currently this is done by
exiting the program and using INGRES query commands to verify data.)
The user registration fields which this program will affect are:

@Begin(Verbatim, LeftMargin +.5inch)

THE USER INFO DATA STRUCTURE

    name     

    login         
    mit_id       
    first 
    last 
    mid_init       
    exp_date
    shell              	
    nickname       
    full_name             	
    mit_addr       
    mit_phone       
    mit_affl_id          	
    mit_dept       
    mit_year        
    local_phone            	
    local_str      
    local_city             	
    local_state    
    local_zip       
@End(Verbatim)
@End(Multiple)

Disk Storage Allocation - Allocate/change home directory storage space.
This program allows the adminstrator to check storage allocation on a
server and allocate or change a storage space for a user.  The
information will be held in the SMS database and will be passed to the
name service.  The allocation of a user's locker can be done
automatically at register time by looking at the disk free % and
allocating based on free space.@foot(How does SMS know the disk free %?)

Mail - Allocate/change home mail host.  The name service and a mail
forwarding service@foot(Who's working on this???) need to know where a
user's post office is.  When a new user registers this program checks
the load of various post offices and allocates space to a user in a post
office with available space.@foot(Again, how does SMS determine the load?)

Update current Users - Add/Remove students from the system using
Registrar's tape.  Each term, when the Registrar releases a tape of
current students, the system administrator must load the names of new
users and delete all old users.  This program will automatically use the
Registrar's tape as a means of keeping current the SMS
database.@foot("Does this convert tape to otherwise normal protocol
transactions?" -- Dan)

Servers - Add/update system servers.  This program not only interacts with
the database and updates the server tuple set, but also allows the
administrator to update the SDF, server description files.  This gives the
DCM the knowledge necessary to update the system.

@End(Itemize)
