@part[tools, root "sms.mss"]
@MajorSection(Specialized Management Tools)

From a functional standpoint, SMS will include a set of specialized
management tools.  These tools enable system adminstrators and operators the
flexibility and control of system resources.  As the system evolves, more
management tools will become a part of the SMS's application program
library.

These tools provide the fundamental administrative use of SMS.  The
following tools are included:

@Begin(Itemize, Spread 1)
@Begin(Multiple)

RVD - Create/updating an RVD. This administrative program will allow for the
master copy of RVD's to be updated and created.  The DCM will distribute the
RVD information automatically to the servers requiring RVD data.  Presently,
the system administrator keeps an up-to-date file of RVD data and then
copies the data to the RVD server. The format of the RVD server database:

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

By providing to the DCM the required data structure for the RVD servers, the
present file /site/rvd/rvddb can be removed.  The DCM will automatically
load the RVD server with information.
@End(Multiple)

RVD - Adding a user, project, or course to a RVD.  This program will allow
the administrator to associate to an RVD a user, project, or a course.
Right now, each workstation has the file /etc/rvdtab which is manually
updated by the operations staff.  By associating a course to a RVD in the
SMS database and using Hesiod, the Athena name server, for finding which
course is associated with a RVD the system will no longer require
/etc/rvdtab.  Additionally, SMS will update Hesiod with new RVD information.

@Begin(Multiple)

User Registration - Adding/changing user information.  Presently, there are
two programs which the system administrator uses to register a new user:
register and admin.  These programs register the user and enter password
information to Kerberos, respectively.  The new application will provide a
combination of these programs and offer a third feature which allows the
administrator to check the fields of the SMS database without exiting and
verifying that all of the database fields are correct.  Currently this is
done by exiting the program and using INGRES query commands to verify data.
The user registration fields which this program will effect are:

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
This program allows for the adminstrator to check storage allocation on a
machine and allocate or change a storage space for a user.  The information
will be held in the SMS database and will be available to the name service
when clients ask where their home storage locker is.  The allocation of a
user's locker can be done automatically at register time by looking at the
disk free % and allocating based on free space.

Mail - Allocate/change home mail machine.  The name service and a mail
forwarding service needs to know where a user's post office is.  When a new
user registers this program checks the load of various post offices and
allocates space to a user in a post office with available space.

Update current Users - Add/Remove students from the system using Registrar's
tape.  Each term, when the Registrar releases a tape of current students,
the system administrator must load the names of new users and delete all old
users.  This program will automatically use the Registrar's tape as a means
of keeping current the SMS database.

Servers - Add/update system servers.  This program not only interacts with
the database and updates the server tuple set, but also allows the
administrator to update the SDF, server description files.  This gives the
DCM the knowledge necessary to update the system.

@End(Itemize)
