/* $Id: f_defs.h,v 1.35 2001-08-19 02:49:57 zacheiss Exp $
 *
 *	This is the file f_defs.h for the Moira Client, which allows users
 *      to quickly and easily maintain most parts of the Moira database.
 *	It Contains: useful definitions.
 *
 *	Created: 	4/12/88
 *	By:		Chris D. Peterson
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include "menu.h"

#ifndef _f_defs_
#define _f_defs_

/* acl.c */
int GetACL(int argc, char **argv);
int AddACL(int argc, char **argv);
int DeleteACL(int argc, char **argv);

/* attach.c */

int GetFS(int argc, char **argv);
int GetFSM(int argc, char **argv);
int AddFS(int argc, char **argv);
int ChangeFS(int argc, char **argv);
int DeleteFS(int argc, char **argv);

int AddFSToGroup(int argc, char **argv);
int RemoveFSFromGroup(int argc, char **argv);
int ChangeFSGroupOrder(int argc, char **argv);
int FSGroupHelp(int argc, char **argv);

int GetFSAlias(int argc, char **argv);
int CreateFSAlias(int argc, char **argv);
int DeleteFSAlias(int argc, char **argv);

int AttachHelp(void);

/* cluster.c */

int ShowMachineInfo(int argc, char **argv);
int ShowMachineQuery(int argc, char **argv);
int MachineByOwner(int argc, char **argv);
int AddMachine(int argc, char **argv);
int UpdateMachine(int argc, char **argv);
int DeleteMachine(int argc, char **argv);
int MachineToClusterMap(int argc, char **argv);
int AddMachineToCluster(int argc, char **argv);
int RemoveMachineFromCluster(int argc, char **argv);

int ShowClusterInfo(int argc, char **argv);
int AddCluster(int argc, char **argv);
int UpdateCluster(int argc, char **argv);
int DeleteCluster(int argc, char **argv);

int ShowClusterInfo(int argc, char **argv);
int AddCluster(int argc, char **argv);
int UpdateCluster(int argc, char **argv);
int DeleteCluster(int argc, char **argv);

int ShowClusterData(int argc, char **argv);
int AddClusterData(int argc, char **argv);
int RemoveClusterData(int argc, char **argv);

int ShowSubnetInfo(int argc, char **argv);
int AddSubnet(int argc, char **argv);
int UpdateSubnet(int argc, char **argv);
int DeleteSubnet(int argc, char **argv);

int ShowCname(int argc, char **argv);
int AddCname(int argc, char **argv);
int DeleteCname(int argc, char **argv);

int ShowContainerInfo(int argc, char **argv);
int AddContainer(int argc, char **argv);
int UpdateContainer(int argc, char **argv);
int DeleteContainer(int argc, char **argv);
int GetSubContainers(int argc, char **argv);
int MachineToContainerMap(int argc, char **argv);
int AddMachineToContainer(int argc, char **argv);
int RemoveMachineFromContainer(int argc, char **argv);
int GetMachinesOfContainer(int argc, char **argv);
int GetTopLevelCont(int argc, char **argv);

/* delete.c */

int DeleteList(int argc, char **argv);
int DeleteUser(int argc, char **argv);

/* lists.c */

int ListByMember(int argc, char **argv);
int ListByAdministrator(int argc, char **argv);
int ListAllPublicMailLists(int argc, char **argv);

int AddMember(int argc, char **argv);
int DeleteMember(int argc, char **argv);
int ListAllMembers(int argc, char **argv);
int ListUserMembers(int argc, char **argv);
int ListListMembers(int argc, char **argv);
int ListStringMembers(int argc, char **argv);

int TagMember(int argc, char **argv);
int DeleteTag(int argc, char **argv);
int ListMembersWithTags(int argc, char **argv);

int ShowListInfo(int argc, char **argv);
int UpdateList(int argc, char **argv);
int InterRemoveItemFromLists(int argc, char **argv);

int ListmaintMemberMenuEntry(Menu *m, int argc, char **argv);
int ListmaintMemberMenuExit(Menu *m);
int ListHelp(int argc, char **argv);

/* nfs.c */

int ShowNFSService(int argc, char **argv);
int AddNFSService(int argc, char **argv);
int UpdateNFSService(int argc, char **argv);
int DeleteNFSService(int argc, char **argv);

/* pobox.c */

int GetUserPOBox(int argc, char **argv);
int SetUserPOBox(int argc, char **argv);
int SplitUserPOBox(int argc, char **argv);
int RemoveUserPOBox(int argc, char **argv);

/* quota.c */

int ShowDefaultQuota(int argc, char **argv);
int ChangeDefaultQuota(int argc, char **argv);

int GetQuota(int argc, char **argv);
int GetQuotaByFilesys(int argc, char **argv);
int AddQuota(int argc, char **argv);
int UpdateQuota(int argc, char **argv);
int DeleteQuota(int argc, char **argv);

/* user.c */

int ShowUserByLogin(int argc, char **argv);
int ShowUserByName(int argc, char **argv);
int ShowUserByClass(int argc, char **argv);
int ShowUserById(int argc, char **argv);
int AddNewUser(int argc, char **argv);
int RegisterUser(int argc, char **argv);
int UpdateUser(int argc, char **argv);
int DeactivateUser(int argc, char **argv);
int DeleteUserByUid(int argc, char **argv);
int GetKrbmap(int argc, char **argv);
int AddKrbmap(int argc, char **argv);
int DeleteKrbmap(int argc, char **argv);
int GetDirFlags(int argc, char **argv);
int SetDirFlags(int argc, char **argv);
int GetUserReservations(int argc, char **argv);
int AddUserReservation(int argc, char **argv);
int DelUserReservation(int argc, char **argv);
int GetUserByReservation(int argc, char **argv);
void PrintReservationTypes(void);

/* printer.c */
int GetPrn(int argc, char **argv);
int GetPrnByEthernet(int argc, char **argv);
int GetPrnByHostname(int argc, char **argv);
int GetPrnByRM(int argc, char **argv);
int GetPrnByLocation(int argc, char **argv);
int GetPrnByContact(int argc, char **argv);
int DeletePrn(int argc, char **argv);
int AddPrn(int argc, char **argv);
int ChngPrn(int argc, char **argv);
int UpdateHWAddr(int argc, char **argv);
int GetPrintSrv(int argc, char **argv);
int AddPrintSrv(int argc, char **argv);
int DelPrintSrv(int argc, char **argv);
int ChangePrintSrv(int argc, char **argv);


/* zephyr.c */
int AddZephyr(int argc, char **argv);
int GetZephyr(int argc, char **argv);
int ChngZephyr(int argc, char **argv);
int DeleteZephyr(int argc, char **argv);

/* dcmmaint.c */
int EnableDcm(int argc, char **argv);
int InProgress(int argc, char **argv);
int DcmFailed(int argc, char **argv);
int Dcm(int argc, char **argv);
int showserv(int argc, char **argv);
int addserv(int argc, char **argv);
int updateserv(int argc, char **argv);
int delserv(int argc, char **argv);
int showhost(int argc, char **argv);
int resetsrverr(int argc, char **argv);
int resetsrvc(int argc, char **argv);
int resethosterr(int argc, char **argv);
int resethost(int argc, char **argv);
int sethostor(int argc, char **argv);
int addhost(int argc, char **argv);
int updatehost(int argc, char **argv);
int delhost(int argc, char **argv);

/* misc.c */
int TableStats(int argc, char **argv);
int ShowClients(int argc, char **argv);
int ShowValue(int argc, char **argv);
int ShowAlias(int argc, char **argv);

/* utils.c */

/*
 * For descriptions of all of these functions see the
 * internals documentation.
 */

void AddQueue(struct mqelem *elem, struct mqelem *pred);
char **CopyInfo(char **info);
void FreeInfo(char **);
void FreeQueue(struct mqelem *elem);
void FreeAndClear(char **pointer, Bool free_it);
void Loop(struct mqelem *elem, void (*func)(char **));
void QueryLoop(struct mqelem *elem, char * (*print_func)(char **),
	       void (*op_func)(char **, Bool), char *query_string);
void SlipInNewName(char **info, char *name);
int CountArgs(char **args);
int GetValueFromUser(char *prompt, char **pointer);
int GetYesNoValueFromUser(char *prompt, char **pointer);
int GetFSTypes(char **current, int options);
int Print(int argc, char **argv, void *callback);
int PrintByType(int argc, char **argv, void *callback);
int PrintHelp(char **help);
int StoreInfo(int argc, char **argv, void *data);
int ToggleVerboseMode(int argc, char **argv);
char *NullPrint(char **info);
char *atot(char *itime);
struct mqelem *QueueTop(struct mqelem *elem);
Bool Confirm(char *prompt);
Bool YesNoQuestion(char *prompt, int bool_def);
Bool YesNoQuitQuestion(char *prompt, int bool_def);
int do_mr_query(char *name, int argc, char **argv,
		int (*proc)(int, char **, void *), void *hint);
int GetTypeFromUser(char *prompt, char *tname, char **pointer);
int GetAddressFromUser(char *prompt, char **pointer);
Bool ValidName(char *s);
int QueueCount(struct mqelem *elem);
int PromptWithDefault(char *prompt, char *buf, int buflen, char *def);
struct mqelem *GetTypeValues(char *tname);

/* from clients/lib/error.c */
char *mrcl_get_message(void); 

#endif /* _f_defs_ */
