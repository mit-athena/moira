/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/display.c,v 1.3 1991-06-05 11:22:06 mar Exp $
 */

#include <stdio.h>
#include <moira.h>
#include <moira_site.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <Xm/Xm.h>
#include "mmoira.h"


/******* temporary ********/
char *atot(s) char *s; { return s; }


static save_info(argc, argv, sq)
int argc;
char **argv;
struct save_queue *sq;
{
    sq_save_args(argc, argv, sq);
}


int DisplayCallback(argc, info, form)
int argc;
char **info;
EntryForm *form;
{
    char buf[1024], name[128];
    char *MOD_FORMAT = "Modified by %s at %s with %s\n";
    int i;

    switch (form->menu->operation) {
    case MM_SHOW_USER:
       	sprintf(name, "%s, %s %s", info[U_LAST], info[U_FIRST], info[U_MIDDLE]);
	sprintf(buf, "Login name: %-20s Full name: %s\n", info[U_NAME], name);
	AppendToLog(buf);
	sprintf(buf, "User id: %-23s Login shell %-10s Class: %s\n", 
		info[U_UID], info[U_SHELL], info[U_CLASS]);
	AppendToLog(buf);
	sprintf(buf, "Account is: %-20s Encrypted MIT ID number: %s\n",
		user_states[atoi(info[U_STATE])], info[U_MITID]);
	AppendToLog(buf);
	sprintf(buf, MOD_FORMAT,
		info[U_MODBY], info[U_MODTIME],info[U_MODWITH]);
	AppendToLog(buf);
	break;
    case MM_SHOW_FINGER:
	sprintf(buf, "Finger information for user %s:\n", info[F_NAME]);
	AppendToLog(buf);
	sprintf(buf, "Login name: %-27s In real life: %s\n", info[F_NAME],
		info[F_FULLNAME]);
	AppendToLog(buf);
	sprintf(buf, "Nickname: %-29s Department: %s\n", info[F_NICKNAME],
		info[F_MIT_DEPT]);
	AppendToLog(buf);
	sprintf(buf, "Home: %-33s Home phone: %s\n", info[F_HOME_ADDR],
		info[F_HOME_PHONE]);
	AppendToLog(buf);
	sprintf(buf, "Office: %-31s Office phone: %s\n", info[F_OFFICE_ADDR],
		info[F_OFFICE_PHONE]);
	AppendToLog(buf);
	sprintf(buf, MOD_FORMAT, info[F_MODBY], info[F_MODTIME],
		info[F_MODWITH]);
	AppendToLog(buf);
	break;
    case MM_SHOW_POBOX:
	sprintf(buf, "Post Office information for user %s:\n", info[PO_NAME]);
	AppendToLog(buf);
	sprintf(buf, "type: %-8s Box: %s\n", info[PO_TYPE], info[PO_BOX]);
	AppendToLog(buf);
	sprintf(buf, MOD_FORMAT, info[4], info[3], info[5]);
	AppendToLog(buf);
	break;
    case MM_SHOW_KRBMAP:
	sprintf(buf, "Kerberos mapping: User %-9s Principal %s\n",
		info[KMAP_USER], info[KMAP_PRINCIPAL]);
	AppendToLog(buf);
	break;
    case MM_SHOW_FILSYS:
    case MM_SHOW_FSGROUP:
	if (!strcmp(info[FS_TYPE], "FSGROUP")) {
	    int stat;
	    char **argv;
	    struct save_queue *sq;

	    sprintf(buf,"%20s Filesystem Group: %s\n", " ", info[FS_NAME]);
	    AppendToLog(buf);

	    sprintf(buf,"Comments: %s\n",info[FS_COMMENTS]);
	    AppendToLog(buf);
	    sprintf(buf, MOD_FORMAT, info[FS_MODBY], info[FS_MODTIME], 
		    info[FS_MODWITH]);
	    AppendToLog(buf);
	    AppendToLog("Containing the filesystems (in order):");

	    sq = sq_create();
	    if ((stat = MoiraQuery("get_fsgroup_members", 1, &info[FS_NAME],
				    save_info, (char *)sq)) != 0) {
		if (stat == MR_NO_MATCH)
		  AppendToLog("    [no members]");
		else
		  com_err(program_name, stat, NULL);
	    } else {
		while (sq_get_data(sq, &argv)) {
		    sprintf(buf, "  Filesystem: %-32s (sort key: %s)",
			    info[0], info[1]);
		    free(argv[0]);
		    free(argv[1]);
		    free(argv);
		}
		sq_destroy(sq);
	    }
	} else {
	    sprintf(buf,"Filesystem: %s\n", info[FS_NAME]);
	    AppendToLog(buf);
	    sprintf(buf,"Type: %-40s Machine: %-15s\n",
		    info[FS_TYPE], info[FS_MACHINE]);
	    AppendToLog(buf);
	    sprintf(buf,"Default Access: %-2s Packname: %-17s Mountpoint %s \n",
		    info[FS_ACCESS], info[FS_PACK], info[FS_M_POINT]);
	    AppendToLog(buf);
	    sprintf(buf,"Comments: %s\n",info[FS_COMMENTS]);
	    AppendToLog(buf);
	    sprintf(buf, "User Ownership: %-30s Group Ownership: %s\n",
		    info[FS_OWNER], info[FS_OWNERS]);
	    AppendToLog(buf);
	    sprintf(buf, "Auto Create: %-34s Locker Type: %s\n",
		    atoi(info[FS_CREATE]) ? "ON" : "OFF", 
		    info[FS_L_TYPE]);
	    AppendToLog(buf);
	    sprintf(buf, MOD_FORMAT, info[FS_MODBY], info[FS_MODTIME], 
		    info[FS_MODWITH]);
	    AppendToLog(buf);
	}
	break;
    case MM_SHOW_NFS:
	sprintf(buf,"Machine: %-20s Directory: %-15s Device: %s\n",
		info[NFS_NAME], info[NFS_DIR], info[NFS_DEVICE]);
	AppendToLog(buf);
	sprintf(buf, "Status: %s\n",
		format_filesys_type(info[NFS_STATUS]));
	AppendToLog(buf);
	sprintf(buf, "Quota Allocated: %-17s Size: %s\n",
		info[NFS_ALLOC], info[NFS_SIZE]);
	AppendToLog(buf);
	sprintf(buf, MOD_FORMAT, info[NFS_MODBY], info[NFS_MODTIME],
		info[NFS_MODWITH]);
	AppendToLog(buf);
	break;
    case MM_SHOW_QUOTA:
	if (!strcmp(info[Q_TYPE], "ANY"))
	  sprintf(buf, "Filesystem: %s\n", info[Q_FILESYS]);
	else
	  sprintf(buf, "Filesystem: %-45s %s %s\n", info[Q_FILESYS],
		  info[Q_TYPE], info[Q_NAME]);
	AppendToLog(buf);
	sprintf(buf, "Machine: %-20s Directory: %-15s\n",
		info[Q_MACHINE], info[Q_DIRECTORY]);
	AppendToLog(buf);
	sprintf(buf, "Quota: %s\n", info[Q_QUOTA]);
	AppendToLog(buf);
	sprintf(buf, MOD_FORMAT, info[Q_MODBY], info[Q_MODTIME], info[Q_MODWITH]);
	AppendToLog(buf);
	break;
    case MM_SHOW_LIST:
	(void) sprintf(buf, "List: %s\n", info[L_NAME]);
	AppendToLog(buf);
	(void) sprintf(buf, "Description: %s\n", info[L_DESC]);
	AppendToLog(buf);
	if ( atoi(info[L_MAILLIST]))
	  AppendToLog("This list is a mailing list.\n");
	else
	  AppendToLog("This list is NOT a mailing list.\n");
	if (atoi(info[L_GROUP])) {
	    (void) sprintf(buf,"This list is a Group and its ID number is %s\n",
			   info[L_GID]);
	    AppendToLog(buf);
	} else
	  AppendToLog("This list is NOT a Group.\n");
	if (strcmp(info[L_ACE_TYPE],"NONE") == 0)
	  AppendToLog("This list has no Administrator, how strange?!\n");
	else {
	    sprintf(buf, "The Administrator of this list is the %s: %s\n",
		    info[L_ACE_TYPE], info[L_ACE_NAME]);
	    AppendToLog(buf);
	}
	(void) sprintf(buf, "This list is: %s, %s, and %s\n",
		       atoi(info[L_ACTIVE]) ? "active" : "inactive",
		       atoi(info[L_PUBLIC]) ? "public" : "private",
		       atoi(info[L_HIDDEN]) ? "hidden" : "visible");
	AppendToLog(buf);
	sprintf(buf, MOD_FORMAT, info[L_MODBY], info[L_MODTIME], info[L_MODWITH]);
	AppendToLog(buf);
	break;
    case MM_SHOW_MACH:
	sprintf(buf, "Machine: %-30s Type: %s\n", info[M_NAME], info[M_TYPE]);
	AppendToLog(buf);
	sprintf(buf, MOD_FORMAT, info[M_MODBY], info[M_MODTIME], info[M_MODWITH]);
	AppendToLog(buf);
	break;
    case MM_SHOW_CLUSTER:
	sprintf(buf, "Cluster:     %s\n", info[C_NAME]);
	AppendToLog(buf);
	sprintf(buf, "Description: %s\n", info[C_DESCRIPT]);
	AppendToLog(buf);
	sprintf(buf, "Location:    %s\n", info[C_LOCATION]);
	AppendToLog(buf);
	sprintf(buf, MOD_FORMAT, info[C_MODBY], info[C_MODTIME], info[C_MODWITH]);
	AppendToLog(buf);
	break;
    case MM_SHOW_CLDATA:
	sprintf(buf, "Cluster: %-20s Label: %-15s Data: %s\n",
		info[CD_NAME], info[CD_LABEL], info[CD_DATA]);
	AppendToLog(buf);
	break;
    case MM_SHOW_MCMAP:
	sprintf(buf, "Machine: %-20s Cluster: %s\n", info[0], info[1]);
	AppendToLog(buf);
	break;
    case MM_SHOW_MEMBERS:
	if (argc == 2)
	  sprintf(buf, "%-9s %s\n", info[0], info[1]);
	else
	  sprintf(buf, "%s\n", info[0]);
	AppendToLog(buf);
	break;
    case MM_STATS:
	sprintf(buf, "Table: %-20s Modified: %s\n", info[0], info[5]);
	AppendToLog(buf);
	sprintf(buf, "  %-8D appends, %-8d updates, %-8d deletes\n",
		info[2], info[3], info[4]);
	AppendToLog(buf);
	break;
    case MM_CLIENTS:
	{
	    unsigned long host_address;
	    struct hostent *host_entry;

	    host_address = inet_addr(info[1]);
	    if (host_address != NULL) {
		host_entry = gethostbyaddr((char *)&host_address, 4, AF_INET);
		if (host_entry != NULL) {
		    free(info[1]);
		    info[1] = strsave(host_entry->h_name);
		}
	    }
	}
	sprintf(buf, "Principal %s on %s (%s)\n", info[0], info[1], info[2]);
	AppendToLog(buf);
	sprintf(buf, "  Connected at %s, client %s\n", info[3], info[4]);
	AppendToLog(buf);
	break;
    case MM_SHOW_VALUE:
	sprintf(buf, "Variable: %-20s Value: %s\n", stringval(form, 0),
		info[0]);
	AppendToLog(buf);
	break;
    case MM_SHOW_ALIAS:
	sprintf(buf, "Alias: %-20s Type: %-8s Value: %s\n",
		info[0], info[1], info[2]);
	AppendToLog(buf);
	break;
    case MM_SHOW_HOST:
	sprintf(buf, "%s:%s  mod by %s on %s with %s\n", info[SH_MACHINE],
		info[SH_SERVICE], info[SH_MODBY], info[SH_MODTIME],
		info[SH_MODWITH]);
	AppendToLog(buf);
	if (atoi(info[SH_HOSTERROR]))
	  sprintf(name, "Error %d: %s", atoi(info[SH_HOSTERROR]),
		  info[SH_ERRMSG]);
	else
	  strcpy(name, "No error");
	sprintf(buf, "  %s/%s/%s/%s/%s\n",
		atoi(info[SH_ENABLE]) ? "Enabled" : "Disabled",
		atoi(info[SH_SUCCESS]) ? "Success" : "Failure",
		atoi(info[SH_INPROGRESS]) ? "InProgress" : "Idle",
		atoi(info[SH_OVERRIDE]) ? "Override" : "Normal", name);
	AppendToLog(buf);
	AppendToLog("  Last Try             Last Success         Value1    Value2    Value3\n");
	strcpy(name, atot(info[SH_LASTTRY]));
	sprintf(buf, "  %-20s %-20s %-9d %-9d %s\n", name,
		atot(info[SH_LASTSUCCESS]), atoi(info[SH_VALUE1]),
		atoi(info[SH_VALUE2]), info[SH_VALUE3]);
	AppendToLog(buf);
	break;
    case MM_SHOW_DQUOTA:
	sprintf(buf, "The default quota is %s Kbytes.\n", info[0]);
	AppendToLog(buf);
	break;
    case MM_SHOW_DCM:
	if (argc == 2)
	  sprintf(buf, "%s:%s\n", info[0], info[1]);
	else
	  sprintf(buf, "%s\n", info[0]);
	AppendToLog(buf);
	break;
    case MM_SHOW_PCAP:
	sprintf(buf, "Printer: %-35s Spool host: %s\n", info[PCAP_NAME],
		info[PCAP_SPOOL_HOST]);
	AppendToLog(buf);
	sprintf(buf, "Spool directory: %-27s Remote Printer Name: %s\n",
		info[PCAP_SPOOL_DIR], info[PCAP_RPRINTER]);
	AppendToLog(buf);
	sprintf(buf, "Authentication: %-3s Price/page: %-3s  Quota Server: %s\n",
		atoi(info[PCAP_AUTH]) ? "yes" : "no",
		info[PCAP_PRICE], info[PCAP_QSERVER]);
	AppendToLog(buf);
	sprintf(buf, "Comments: %s\n", info[PCAP_COMMENTS]);
	AppendToLog(buf);
	sprintf(buf, MOD_FORMAT, info[PCAP_MODBY], info[PCAP_MODTIME], 
		info[PCAP_MODWITH]);
	AppendToLog(buf);
	break;
    case MM_SHOW_ZEPHYR:
	sprintf(buf, "Zephyr class: %s\n", info[ZA_CLASS]);
	AppendToLog(buf);
	if (!strcmp("NONE", info[ZA_XMT_TYPE]))
	  name[0] = 0;
	else
	  sprintf(name, "Name: %s", info[ZA_XMT_ID]);
	sprintf(buf, "XMT ACL Type %s %s\n", info[ZA_XMT_TYPE], name);
	AppendToLog(buf);
	if (!strcmp("NONE", info[ZA_SUB_TYPE]))
	  name[0] = 0;
	else
	  sprintf(name, "Name: %s", info[ZA_SUB_ID]);
	sprintf(buf, "SUB ACL Type %s %s\n", info[ZA_SUB_TYPE], name);
	AppendToLog(buf);
	if (!strcmp("NONE", info[ZA_IWS_TYPE]))
	  name[0] = 0;
	else
	  sprintf(name, "Name: %s", info[ZA_IWS_ID]);
	sprintf(buf, "IWS ACL Type %s %s\n", info[ZA_IWS_TYPE], name);
	AppendToLog(buf);
	if (!strcmp("NONE", info[ZA_IUI_TYPE]))
	  name[0] = 0;
	else
	  sprintf(name, "Name: %s", info[ZA_IUI_ID]);
	sprintf(buf, "IUI ACL Type %s %s\n", info[ZA_IUI_TYPE], name);
	AppendToLog(buf);
	break;
    default:
	for (i = 0; i < argc; i++) {
	    if (i != 0) AppendToLog(", ");
	    AppendToLog(info[i]);
	}
	AppendToLog("\n");
    }
    return(MR_CONT);
}
