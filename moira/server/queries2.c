/* This file defines the query dispatch table for version 2 of the protocol
 *
 * $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/queries2.c,v 1.8 1988-08-25 19:24:21 mar Exp $
 *
 * Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 */

#include "query.h"
#include "sms_et.h"

/* Specialized Support Query Routines */

/* Special Access Check Routines */
int access_user();
int access_login();
int access_list();
int access_visible_list();
int access_vis_list_by_name();
int access_member();
int access_qgli();
int access_service();
int access_filesys();

/* Query Setup Routines */
int setup_ausr();
int setup_dusr();
int setup_spop();
int setup_dpob();
int setup_dmac();
int setup_dclu();
int setup_alis();
int setup_dlis();
int setup_dsin();
int setup_dshi();
int setup_afil();
int setup_ufil();
int setup_dfil();
int setup_dnfp();
int setup_dnfq();

/* Query Followup Routines */
int followup_fix_modby();
int followup_ausr();
int followup_gpob();
int followup_glin();
int followup_amtl();
int followup_anfq();
int followup_gzcl();
int followup_gsha();
int followup_gnfq();

int set_modtime();
int set_modtime_by_id();
int set_finger_modtime();
int set_pobox_modtime();
int set_mach_modtime();
int set_mach_modtime_by_id();
int set_cluster_modtime_by_id();
int set_serverhost_modtime();
int set_nfsphys_modtime();
int set_filesys_modtime();
int set_zephyr_modtime();

/* Special Queries */
int set_pobox();
int get_list_info();
int get_ace_use();
int qualified_get_lists();
int get_members_of_list();
int qualified_get_server();
int qualified_get_serverhost();
int trigger_dcm();
int count_members_of_list();
int get_lists_of_member();
int register_user();
int add_user_group();



/* String compression
 * These are commonly used strings in the table, defined here so that
 * they can be shared.
 */

static char ACE_NAME[] = "ace_name";
static char ACE_TYPE[] = "ace_type";
static char CLASS[] = "class";
static char CLU_ID[] = "clu_id";
static char CLUSTER[] = "cluster";
static char DESC[] = "description";
static char DEVICE[] = "device";
static char DIR[] = "dir";
static char FILESYS[] = "filesys";
static char FILSYS_ID[] = "filsys_id";
static char FIRST[] = "firstname";
static char LABEL[] = "label";
static char LAST[] = "lastname";
static char LIST[] = "list";
static char LIST_ID[] = "list_id";
static char LOCATION[] = "location";
static char LOGIN[] = "login";
static char MACH_ID[] = "mach_id";
static char MACHINE[] = "machine";
static char MIDDLE[] = "middle";
static char MIT_ID[] = "mit_id";
static char MOD1[] = "modtime";
static char MOD2[] = "modby";
static char MOD3[] = "modwith";
static char NAME[] = "name";
static char SERVICE[] = "service";
static char SHELL[] = "shell";
static char STATUS[] = "status";
static char TYPE[] = "type";
static char USERS[] = "users";
static char USERS_ID[] = "users_id";
static char UID[] = "uid";



/* VALOBJS
 * These are commonly used validation objects, defined here so that they
 * can be shared.
 */

static struct valobj VOsort0[] = {
  {V_SORT, 0, 0, 0, 0, 0},
};

static struct valobj VOdate1[] = {
  {V_DATE, 1, 0, 0, 0, SMS_DATE},
};

static struct valobj VOuser0[] = {
  {V_ID, 0, USERS, LOGIN, USERS_ID, SMS_USER},
};

static struct valobj VOmach0[] = {
  {V_ID, 0, MACHINE, NAME, MACH_ID, SMS_MACHINE},
};

static struct valobj VOclu0[] = {
  {V_ID, 0, CLUSTER, NAME, CLU_ID, SMS_CLUSTER},
};

static struct valobj VOlist0[] = {
  {V_ID, 0, LIST, NAME, LIST_ID, SMS_LIST},
};

static struct valobj VOchar0[] = {
  {V_CHAR, 0},
};

static struct valobj VOsort01[] = {
  {V_SORT, 1},
  {V_SORT, 0},
};

static struct valobj VOuser0sort[] = {
  {V_ID, 0, USERS, LOGIN, USERS_ID, SMS_USER},
  {V_SORT, 0},
};

static struct valobj VOfilsys0user1[] = {
  {V_ID, 0, FILESYS, LABEL, FILSYS_ID, SMS_FILESYS},
  {V_ID, 1, USERS, LOGIN, USERS_ID, SMS_USER},
};


/* Validations
 * Commonly used validation records defined here so that they may
 * be shared.
 */

static struct validate VDmach = { VOmach0, 1 };
static struct validate VDsort2= { VOsort01,2 };
static struct validate VDsortf = { 
    VOsort0,
    1,
    0,
    0,
    0,
    0,
    0,
    0,
    followup_fix_modby,
};



/* Query data */

static char *galo_fields[] = {
  LOGIN, UID, SHELL, LAST, FIRST, MIDDLE,
};

static char *gubl_fields[] = {
  LOGIN,
  LOGIN, UID, SHELL, LAST, FIRST, MIDDLE, STATUS, 
  MIT_ID, CLASS, MOD1, MOD2, MOD3
};

static struct validate gubx_validate =	/* gubl, gubu */
{
  VOsort0,
  1,
  0,
  0,
  0,
  0,
  access_login,
  0,
  followup_fix_modby,
};

static char *gubu_fields[] = {
  UID,
  LOGIN, UID, SHELL, LAST, FIRST, MIDDLE, STATUS, 
  MIT_ID, CLASS, MOD1, MOD2, MOD3,
};

static char *gubn_fields[] = {
  FIRST, LAST,
  LOGIN, UID, SHELL, LAST, FIRST, MIDDLE, STATUS, 
  MIT_ID, CLASS, MOD1, MOD2, MOD3,
};

static char *gubc_fields[] = {
  CLASS,
  LOGIN, UID, SHELL, LAST, FIRST, MIDDLE, STATUS, 
  MIT_ID, CLASS, MOD1, MOD2, MOD3,
};

static char *gubm_fields[] = {
  MIT_ID,
  LOGIN, UID, SHELL, LAST, FIRST, MIDDLE, STATUS, 
  MIT_ID, CLASS, MOD1, MOD2, MOD3,
};

static char *ausr_fields[] = {
  LOGIN, UID, SHELL, LAST, FIRST, MIDDLE, STATUS, 
  MIT_ID, CLASS,
};

static struct valobj ausr_valobj[] = {
  {V_CHAR, 0},
  {V_CHAR, 3},
  {V_CHAR, 4},
  {V_CHAR, 5},
  {V_CHAR, 7},
  {V_TYPE, 8, CLASS, 0, 0, SMS_BAD_CLASS},
};

static struct validate ausr_validate = {
  ausr_valobj,
  6,
  LOGIN,
  "u.login = \"%s\"",
  1,
  USERS_ID,
  0,
  setup_ausr,
  followup_ausr,
};

static char *rusr_fields[] = {
  UID, LOGIN, "fs_type"
};

static struct validate rusr_validate = {
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  register_user,
  0,
};

static char *uusr_fields[] = {
  LOGIN,
  "newlogin", UID, SHELL, LAST, FIRST, MIDDLE, STATUS, 
  MIT_ID, CLASS,
};

static struct valobj uusr_valobj[] = {
    {V_ID, 0, USERS, LOGIN, USERS_ID, SMS_USER},
    {V_RENAME, 1, USERS, LOGIN, USERS_ID, SMS_NOT_UNIQUE},
    {V_CHAR, 4},
    {V_CHAR, 5},
    {V_CHAR, 6},
    {V_CHAR, 8},
    {V_TYPE, 9, CLASS, 0, 0, SMS_BAD_CLASS},
};

static struct validate uusr_validate = {
  uusr_valobj,
  7,
  0,
  0,
  0,
  USERS_ID,
  0,
  0,
  set_modtime_by_id,
};

static char *uush_fields[] = {
  LOGIN,
  SHELL,
};

static struct validate uush_validate = {
  VOuser0,
  1,
  0,
  0,
  0,
  USERS_ID,
  access_user,
  0,
  set_modtime_by_id,
};

static char *uust_fields[] = {
  LOGIN,
  STATUS,
};

static struct validate uust_validate = {
  VOuser0,
  1,
  0,
  0,
  0,
  USERS_ID,
  0,
  0,
  set_modtime_by_id,
};

static char *dusr_fields[] = {
  LOGIN,
};

static struct validate dusr_validate = {
  VOuser0,
  1,
  0,
  0,
  0,
  0,
  0,
  setup_dusr,
  0,
};

static char *dubu_fields[] = {
  UID,
};

static struct valobj dubu_valobj[] = {
    {V_ID, 0, USERS, UID, USERS_ID, SMS_USER}
};

static struct validate dubu_validate = {
  dubu_valobj,
  1,
  0,
  0,
  0,
  0,
  0,
  setup_dusr,
  0,
};

static char *gfbl_fields[] = {
  LOGIN,
  LOGIN, "fullname", "nickname", "home_addr",
  "home_phone", "office_address", "office_phone", "mit_dept", 
  "mit_affil", MOD1, MOD2, MOD3,
};

static struct validate gfbl_validate = {
  VOuser0,
  1,
  0,
  0,
  0,
  0,
  access_user,
  0,
  followup_fix_modby,
};

static char *ufbl_fields[] = {
  LOGIN,
  "fullname", "nickname", "home_addr",
  "home_phone", "office_address", "office_phone", "mit_dept", 
  "mit_affil",
};

static struct validate ufbl_validate = {
  VOuser0,
  1,
  0,
  0,
  0,
  0,
  access_user,
  0,
  set_finger_modtime,
};

static char *gpob_fields[] = {
  LOGIN,
  LOGIN, TYPE, "box", MOD1, MOD2, MOD3,
};

static struct validate gpob_validate = {
  VOuser0,
  1,
  "potype",
  "u.potype != \"NONE\" and u.users_id=%d",
  1,
  0,
  access_user,
  0,
  followup_gpob,
};

static char *gpox_fields[] = {
  LOGIN, TYPE, "box",
};

static struct validate gpox_validate = {
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  followup_gpob,
};

static char *spob_fields[] = {
  LOGIN, TYPE, "box",
};

static struct valobj spob_valobj[] = {
  {V_ID, 0, USERS, LOGIN, USERS_ID, SMS_USER},
  {V_TYPE, 1, "pobox", 0, 0, SMS_TYPE},
};

static struct validate spob_validate =	/* SET_POBOX */
{
  spob_valobj,
  2,
  0,
  0,
  0,
  0,
  access_user,
  0,
  set_pobox,
};

static struct validate spop_validate =	/* SET_POBOX_POP */
{
  VOuser0,
  1,
  0,
  0,
  0,
  0,
  access_user,
  setup_spop,
  set_pobox_modtime,
};

static struct validate dpob_validate =	/* DELETE_POBOX */
{
  VOuser0,
  1,
  0,
  0,
  0,
  0,
  access_user,
  setup_dpob,
  set_pobox_modtime,
};

static char *gmac_fields[] = {
  NAME,
  NAME, TYPE, MOD1, MOD2, MOD3,
};

static char *amac_fields[] = {
  NAME, TYPE
};

static struct valobj amac_valobj[] = {
  {V_CHAR, 0},
  {V_TYPE, 1, "mac_type", 0, 0, SMS_TYPE},
};

static struct validate amac_validate = {
  amac_valobj,
  2,
  NAME,
  "m.name = uppercase(\"%s\")",
  1,
  MACH_ID,
  0,
  0,
  set_mach_modtime,
};

static char *umac_fields[] = {
  NAME,
  "newname", TYPE,
};

static struct valobj umac_valobj[] = {
  {V_ID, 0, MACHINE, NAME, MACH_ID, SMS_MACHINE},
  {V_RENAME, 1, MACHINE, NAME, MACH_ID, SMS_NOT_UNIQUE},
  {V_TYPE, 2, "mac_type", 0, 0, SMS_TYPE},
};

static struct validate umac_validate = {
  umac_valobj,
  3,
  0,
  0,
  0,
  MACH_ID,
  0,
  0,
  set_modtime_by_id,
};

static char *dmac_fields[] = {
  NAME,
};

static struct validate dmac_validate = {
  VOmach0,
  1,
  0,
  0,
  0,
  0,
  0,
  setup_dmac,
  0,
};

static char *gclu_fields[] = {
  NAME,
  NAME, DESC, LOCATION, MOD1, MOD2, MOD3, 
};

static char *aclu_fields[] = {
  NAME, DESC, LOCATION,
};
  
static struct validate aclu_validate =	/* for aclu  */
{
  VOchar0,
  1,
  NAME,
  "c.name = \"%s\"",
  1,
  "clu_id",
  0,
  0,
  set_modtime,
};

static char *uclu_fields[] = {
  NAME,
  "newname", DESC, LOCATION,
};

static struct valobj uclu_valobj[] = {
  {V_ID, 0, CLUSTER, NAME, CLU_ID, SMS_CLUSTER},
  {V_RENAME, 1, CLUSTER, NAME, CLU_ID, SMS_NOT_UNIQUE},
};

static struct validate uclu_validate = {
  uclu_valobj,
  2,
  0,
  0,
  0,
  CLU_ID,
  0,
  0,
  set_modtime_by_id,
};

static char *dclu_fields[] = {
  NAME,
};

static struct validate dclu_validate = {
    VOclu0,
    1,
    0,
    0,
    0,
    0,
    0,
    setup_dclu,
    0,
};

static char *gmcm_fields[] = {
  MACHINE, CLUSTER,
  MACHINE, CLUSTER,
};

static struct valobj amtc_valobj[] =	/* ADD_MACHINE_TO_CLUSTER */
{					/* DELETE_MACHINE_FROM_CLUSTER */
  {V_ID, 0, MACHINE, NAME, MACH_ID, SMS_MACHINE},
  {V_ID, 1, CLUSTER, NAME, CLU_ID, SMS_CLUSTER},
};

static struct validate amtc_validate = /* for amtc and dmfc */
{
  amtc_valobj,
  2,
  MACH_ID,
  "m.mach_id = %d and m.clu_id = %d",
  2,
  0,
  0,
  0,
  set_mach_modtime_by_id,
};

static char *gcld_fields[] = {
  CLUSTER, "serv_label",
  CLUSTER, "serv_label", "serv_cluster",
};

static char *acld_fields[] = {
  CLUSTER, "serv_label", "serv_cluster",
};

static struct valobj acld_valobj[] =
{
  {V_ID, 0, CLUSTER, NAME, CLU_ID, SMS_CLUSTER},
  {V_CHAR, 2},
};

static struct validate acld_validate =	/* ADD_CLUSTER_DATA */
{					/* DELETE_CLUSTER_DATA */
  acld_valobj,
  2,
  CLU_ID,
  "s.clu_id = %d and s.serv_label = \"%s\" and s.serv_cluster = \"%s\"",
  3,
  0,
  0,
  0,
  set_cluster_modtime_by_id,
};

static char *gsin_fields[] = {
  SERVICE,
  SERVICE, "update_int", "target_file", "script", "dfgen", "dfcheck",
  TYPE, "enable", "inprogress", "harderror", "errmsg",
  ACE_TYPE, ACE_NAME, MOD1, MOD2, MOD3,
};

static struct validate gsin_validate = 
{
    0,
    0,
    0,
    0,
    0,
    0,
    access_service,
    0,
    followup_glin,
};

static char *qgsv_fields[] = {
    "enable", "inprogress", "harderror",
    "service",
};

static struct valobj qgsv_valobj[] = {
  {V_TYPE, 0, "boolean", 0, 0, SMS_TYPE},
  {V_TYPE, 1, "boolean", 0, 0, SMS_TYPE},
  {V_TYPE, 2, "boolean", 0, 0, SMS_TYPE},
};

static struct validate qgsv_validate = {
    qgsv_valobj,
    3,
    0,
    0,
    0,
    0,
    0,
    0,
    qualified_get_server,
};

static char *asin_fields[] = {
  SERVICE, "update_int", "target_file", "script", TYPE, "enable",
  ACE_TYPE, ACE_NAME,
};

static struct valobj asin_valobj[] = {
  {V_CHAR, 0},
  {V_TYPE, 4, "service", 0, 0, SMS_TYPE},
  {V_TYPE, 6, ACE_TYPE, 0, 0, SMS_ACE},
  {V_TYPEDATA, 7, 0, 0, 0, SMS_ACE},
};

static struct validate asin_validate =	/* for asin, usin */
{
  asin_valobj,
  4,
  NAME,
  "s.name = uppercase(\"%s\")",
  1,
  0,
  0,
  0,
  set_modtime,
};

static struct validate rsve_validate = {
  0,
  0,
  NAME,
  "s.name = uppercase(\"%s\")",
  1,
  0,
  access_service,
  0,
  set_modtime,
};

static char *ssif_fields[] = {
  SERVICE, "dfgen", "dfcheck", "inprogress", "harderror", "errmsg",
};

static struct validate ssif_validate = {
  0,
  0,
  NAME,
  "s.name = uppercase(\"%s\")",
  1,
  0,
  0,
  0,
  0,
};

static char *dsin_fields[] = {
  SERVICE,
};

static struct validate dsin_validate = {
  0,
  0,
  NAME,
  "s.name = uppercase(\"%s\")",
  1,
  0,
  0,
  setup_dsin,
  0,
};

static char *gshi_fields[] = {
  SERVICE, MACHINE,
  SERVICE, MACHINE, "enable", "override", "success", "inprogress",
  "hosterror", "hosterrormsg", "ltt", "lts", "value1", "value2",
  "value3", MOD1, MOD2, MOD3, 
};

static struct validate gshi_validate = {
  0,
  0,
  0,
  0,
  0,
  0,
  access_service,
  0,
  followup_fix_modby,
};

static char *qgsh_fields[] = {
    "service", "enable", "override", "success", "inprogress", "hosterror",
    "service", MACHINE,
};

static struct valobj qgsh_valobj[] = {
  {V_TYPE, 1, "boolean", 0, 0, SMS_TYPE},
  {V_TYPE, 2, "boolean", 0, 0, SMS_TYPE},
  {V_TYPE, 3, "boolean", 0, 0, SMS_TYPE},
  {V_TYPE, 4, "boolean", 0, 0, SMS_TYPE},
  {V_TYPE, 5, "boolean", 0, 0, SMS_TYPE},
};

static struct validate qgsh_validate = {
    qgsh_valobj,
    5,
    0,
    0,
    0,
    0,
    0,
    0,
    qualified_get_serverhost,
};

static char *ashi_fields[] = {
  SERVICE, MACHINE, "enable", "value1", "value2", "value3",
};

static struct valobj ashi_valobj[] = {
  {V_NAME, 0, "servers", NAME, 0, SMS_SERVICE},
  {V_ID, 1, MACHINE, NAME, MACH_ID, SMS_MACHINE},
};

static struct validate ashi_validate = /* ashi & ushi */
{
  ashi_valobj,
  2,
  SERVICE,
  "sh.service = uppercase(\"%s\") and sh.mach_id = %d",
  2,
  0,
  access_service,
  0,
  set_serverhost_modtime,
};

static struct validate rshe_validate =	
{
  ashi_valobj,
  2,
  SERVICE,
  "sh.service = uppercase(\"%s\") and sh.mach_id = %d",
  2,
  0,
  access_service,
  0,
  set_serverhost_modtime,
};

static struct validate ssho_validate =	
{
  ashi_valobj,
  2,
  SERVICE,
  "sh.service = uppercase(\"%s\") and sh.mach_id = %d",
  2,
  0,
  access_service,
  0,
  trigger_dcm,
};

static char *sshi_fields[] = {
    "service", MACHINE, "override", "success", "inprogress",
    "hosterror", "errmsg", "ltt", "lts"
};

static struct valobj sshi_valobj[] = {
  {V_NAME, 0, "servers", NAME, 0, SMS_SERVICE},
  {V_ID, 1, MACHINE, NAME, MACH_ID, SMS_MACHINE},
};

static struct validate sshi_validate =	
{
  sshi_valobj,
  2,
  SERVICE,
  "s.service = uppercase(\"%s\") and s.mach_id = %d",
  2,
  0,
  0,
  0,
  0,
};

static char *dshi_fields[] = {
  SERVICE, MACHINE,
};

static struct validate dshi_validate =	
{
  ashi_valobj,
  2,
  SERVICE,
  "sh.service = uppercase(\"%s\") and sh.mach_id = %d",
  2,
  0,
  access_service,
  setup_dshi,
  0,
};

static char *gsha_fields[] = {
  MACHINE,
  MACHINE, ACE_TYPE, ACE_NAME, MOD1, MOD2, MOD3,
};

static struct validate gsha_validate =	
{
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  followup_gsha,
};

static char *asha_fields[] = {
  MACHINE, ACE_TYPE, ACE_NAME,
};

static struct valobj asha_valobj[] = {
  {V_ID, 0, MACHINE, NAME, MACH_ID, SMS_MACHINE},
  {V_TYPE, 1, ACE_TYPE, 0, 0, SMS_ACE},
  {V_TYPEDATA, 2, 0, 0, 0, SMS_ACE},
};

static struct validate asha_validate =	
{
  asha_valobj,
  3,
  MACH_ID,
  "ha.mach_id = %d",
  1,
  MACH_ID,
  0,
  0,
  set_modtime_by_id,
};

static char *gslo_fields[] = {
  "server",
  "server", MACHINE,
};

static char *gfsl_fields[] = {
  LABEL,
  LABEL, TYPE, MACHINE, NAME, "mount", "access", "comments", "owner", "owners",
  "create", "lockertype", MOD1, MOD2, MOD3,
};

static char *gfsm_fields[] = {
  MACHINE,
  LABEL, TYPE, MACHINE, NAME, "mount", "access", "comments", "owner", "owners",
  "create", "lockertype", MOD1, MOD2, MOD3,
};

static struct validate gfsm_validate = {
  VOmach0,
  1,
  0,
  0,
  0,
  0,
  0,
  0,
  followup_fix_modby,
};

static char *gfsn_fields[] = {
  MACHINE, "parition",
  LABEL, TYPE, MACHINE, NAME, "mount", "access", "comments", "owner", "owners",
  "create", "lockertype", MOD1, MOD2, MOD3,
};

static char *gfsg_fields[] = {
  LIST,
  LABEL, TYPE, MACHINE, NAME, "mount", "access", "comments", "owner", "owners",
  "create", "lockertype", MOD1, MOD2, MOD3,
};

static struct validate gfsg_validate = {
  VOlist0,
  1,
  0,
  0,
  0,
  0,
  access_list,
  0,
  followup_fix_modby,
};

static char *afil_fields[] = {
  LABEL, TYPE, MACHINE, NAME, "mount", "access", "comments", "owner", "owners",
  "create", "lockertype",
};

static struct valobj afil_valobj[] = {
  {V_CHAR, 0},
  {V_TYPE, 1, FILESYS, 0, 0, SMS_FSTYPE},
  {V_ID, 2, MACHINE, NAME, MACH_ID, SMS_MACHINE},
  {V_CHAR, 3},
  {V_ID, 7, USERS, LOGIN, USERS_ID, SMS_USER},
  {V_ID, 8, LIST, NAME, LIST_ID, SMS_LIST},
  {V_TYPE, 10, "lockertype", 0, 0, SMS_TYPE}
};

static struct validate afil_validate = {
  afil_valobj,
  7,
  LABEL,
  "fs.label = \"%s\"",
  1,
  FILSYS_ID,
  0,
  setup_afil,
  set_filesys_modtime,
};

static char *ufil_fields[] = {
  LABEL, "newlabel", TYPE, MACHINE, NAME, "mount", "access", "comments",
  "owner", "owners", "create", "lockertype",
};

static struct valobj ufil_valobj[] = {
  {V_ID, 0, FILESYS, LABEL, FILSYS_ID, SMS_FILESYS},
  {V_RENAME, 1, FILESYS, LABEL, FILSYS_ID, SMS_NOT_UNIQUE},
  {V_TYPE, 2, FILESYS, 0, 0, SMS_FSTYPE},
  {V_ID, 3, MACHINE, NAME, MACH_ID, SMS_MACHINE},
  {V_CHAR, 4},
  {V_ID, 8, USERS, LOGIN, USERS_ID, SMS_USER},
  {V_ID, 9, LIST, NAME, LIST_ID, SMS_LIST},
  {V_TYPE, 11, "lockertype", 0, 0, SMS_TYPE}
};

static struct validate ufil_validate = {
  ufil_valobj,
  8,
  LABEL,
  "fs.filsys_id = %d",
  1,
  FILSYS_ID,
  0,
  setup_ufil,
  set_filesys_modtime,
};

static char *dfil_fields[] = {
  LABEL,
};

static struct valobj dfil_valobj[] = {
  {V_ID, 0, FILESYS, LABEL, FILSYS_ID, SMS_FILESYS},
};

static struct validate dfil_validate = {
  dfil_valobj,
  1,
  "label",
  "fs.filsys_id = %d",
  1,
  0,
  0,
  setup_dfil,
  0,
};

static char *ganf_fields[] = {
  MACHINE, DIR, DEVICE, STATUS, "allocated", "size", MOD1, MOD2, MOD3,
};

static char *gnfp_fields[] = {
  MACHINE, DIR,
  MACHINE, DIR, DEVICE, STATUS, "allocated", "size", MOD1, MOD2, MOD3,
};

static struct validate gnfp_validate = {
  VOmach0,
  1,
  0,
  0,
  0,
  0,
  0,
  0,
  followup_fix_modby,
};

static struct valobj anfp_valobj[] = {
  {V_ID, 0, MACHINE, NAME, MACH_ID, SMS_MACHINE},
  {V_CHAR, 1},
};

static struct validate anfp_validate = {
  anfp_valobj,
  2,
  DIR,
  "np.mach_id = %d and np.dir = \"%s\"",
  2,
  "nfsphys_id",
  0,
  0,
  set_nfsphys_modtime,
};

static char *dnfp_fields[] = {
  MACHINE, DIR,
};

static struct validate dnfp_validate = {
  VOmach0,
  1,
  DIR,
  "np.mach_id = %d and np.dir = \"%s\"",
  2,
  "nfsphys_id",
  0,
  setup_dnfp,
  set_nfsphys_modtime,
};

static char *gnfq_fields[] = {
  FILESYS, LOGIN,
  FILESYS, LOGIN, "quota", DIR, MACHINE, MOD1, MOD2, MOD3,
};

static struct valobj gnfq_valobj[] = {
  {V_ID, 1, USERS, LOGIN, USERS_ID, SMS_USER},
  {V_SORT, 0},
};

static struct validate gnfq_validate = {
  gnfq_valobj,
  2,
  0,
  0,
  0,
  0,
  access_filesys,
  0,
  followup_fix_modby,
};

static char *gnqp_fields[] = {
  MACHINE, DIR,
  FILESYS, LOGIN, "quota", DIR, MACHINE, MOD1, MOD2, MOD3,
};

static char *anfq_fields[] = {
  FILESYS, LOGIN, "quota",
};

static struct validate anfq_validate = {
  VOfilsys0user1,
  2,
  USERS_ID,
  "nq.filsys_id = %d and nq.users_id = %d",
  2,
  0,
  0,
  0,
  followup_anfq,
};

static struct validate unfq_validate = {
  VOfilsys0user1,
  2,
  USERS_ID,
  "nq.filsys_id = %d and nq.users_id = %d",
  2,
  0,
  0,
  setup_dnfq,
  followup_anfq,
};

static struct validate dnfq_validate = {
  VOfilsys0user1,
  2,
  USERS_ID,
  "nq.filsys_id = %d and nq.users_id = %d",
  2,
  0,
  0,
  setup_dnfq,
  0,
};

static char *glin_fields[] = {
  NAME,
  NAME, "active", "public", "hidden", "maillist", "group", "gid",
  ACE_TYPE, ACE_NAME, DESC, MOD1, MOD2, MOD3,
};

static struct validate glin_validate = {
  0,
  0,
  0,
  0,
  0,
  0,
  access_vis_list_by_name,
  0,
  get_list_info,
};

static char *alis_fields[] = {
  NAME, "active", "public", "hidden", "maillist", "group", "gid",
  ACE_TYPE, ACE_NAME, DESC,
};

static struct valobj alis_valobj[] = {
  {V_CHAR, 0},
  {V_TYPE, 7, ACE_TYPE, 0, 0, SMS_ACE},
  {V_TYPEDATA, 8, 0, 0, LIST_ID, SMS_ACE},
};

static struct validate alis_validate = {
  alis_valobj,
  3,
  NAME,
  "l.name = \"%s\"",
  1,
  LIST_ID,
  0,
  setup_alis,
  set_modtime,
};

static char *ulis_fields[] = {
  NAME,
  "newname", "active", "public", "hidden", "maillist", "group", "gid",
  ACE_TYPE, ACE_NAME, DESC,
};

static struct valobj ulis_valobj[] = {
  {V_ID, 0, LIST, NAME, LIST_ID, SMS_LIST},
  {V_RENAME, 1, LIST, NAME, LIST_ID, SMS_NOT_UNIQUE},
  {V_TYPE, 8, ACE_TYPE, 0, 0, SMS_ACE},
  {V_TYPEDATA, 9, 0, 0, LIST_ID, SMS_ACE},
};

static struct validate ulis_validate = {
  ulis_valobj,
  4,
  NAME,
  "l.list_id = %d",
  1,
  LIST_ID,
  access_list,
  setup_alis,
  set_modtime_by_id,
};

static char *dlis_fields[] = {
  NAME,
};

static struct validate dlis_validate = {
  VOlist0,
  1,
  NAME,
  "l.list_id = %d",
  1,
  0,
  access_list,
  setup_dlis,
  0,
};

static char *amtl_fields[] = {
  "list_name", "member_type", "member_name",
};

static struct valobj amtl_valobj[] = {
  {V_ID, 0, LIST, NAME, LIST_ID, SMS_LIST},
  {V_TYPE, 1, "member", 0, 0, SMS_TYPE},
  {V_TYPEDATA, 2, 0, 0, 0, SMS_NO_MATCH},
};

static struct validate amtl_validate =	/* for amtl and dmfl */
{
  amtl_valobj,
  3,
  "member_id",
  "m.list_id = %d and m.member_type = \"%s\" and m.member_id = %d",
  3,
  0,
  access_list,
  0,
  followup_amtl,
};

static char *gaus_fields[] = {
    ACE_TYPE, ACE_NAME,
    "use_type", "use_name",
};

static struct valobj gaus_valobj[] = {
  {V_TYPE, 0, "gaus", 0, 0, SMS_TYPE},
  {V_TYPEDATA, 1, 0, 0, 0, SMS_NO_MATCH},
};

static struct validate gaus_validate = {
    gaus_valobj,
    2,
    0,
    0,
    0,
    0,
    access_member,
    0,
    get_ace_use,
};

static char *qgli_fields[] = {
    "active", "public", "hidden", "maillist", "group",
    "list",
};

static struct valobj qgli_valobj[] = {
  {V_TYPE, 0, "boolean", 0, 0, SMS_TYPE},
  {V_TYPE, 1, "boolean", 0, 0, SMS_TYPE},
  {V_TYPE, 2, "boolean", 0, 0, SMS_TYPE},
  {V_TYPE, 3, "boolean", 0, 0, SMS_TYPE},
  {V_TYPE, 4, "boolean", 0, 0, SMS_TYPE},
};

static struct validate qgli_validate = {
    qgli_valobj,
    5,
    0,
    0,
    0,
    0,
    access_qgli,
    0,
    qualified_get_lists,
};

static char *gmol_fields[] = {
  "list_name",
  "member_type", "member_name",
};

static struct validate gmol_validate = {
  VOlist0,
  1,
  0,
  0,
  0,
  0,
  access_visible_list,
  0,
  get_members_of_list,
};

static char *glom_fields[] = {
  "member_type", "member_name",
  "list_name", "active", "public", "hidden", "maillist", "group",
};

static struct valobj glom_valobj[] = {
  {V_TYPE, 0, "rmember", 0, 0, SMS_TYPE},
  {V_TYPEDATA, 1, 0, 0, 0, SMS_NO_MATCH},
  {V_SORT, 0},
};

static struct validate glom_validate = {
  glom_valobj,
  3,
  0,
  0,
  0,
  0,
  access_member,
  0,
  get_lists_of_member,
};

static char *cmol_fields[] = {
  "list_name",
  "count",
};

static struct validate cmol_validate = {
  VOlist0,
  1,
  0,
  0,
  0,
  0,
  access_visible_list,
  0,
  count_members_of_list,
};

static char *augr_fields[] = {
  LOGIN,
};

static struct validate augr_validate = {
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  add_user_group,
};

static char *gzcl_fields[] = {
    CLASS,
    CLASS, "xmt_type", "xmt_name", "sub_type", "sub_name",
    "iws_type", "iws_name", "iui_type", "iui_name", MOD1, MOD2, MOD3, 
};

static struct validate gzcl_validate = {
  VOsort0,
  1,
  0,
  0,
  0,
  0,
  0,
  0,
  followup_gzcl,
};

static char *azcl_fields[] = {
    CLASS, "xmt_type", "xmt_name", "sub_type", "sub_name",
    "iws_type", "iws_name", "iui_type", "iui_name",
};

static struct valobj azcl_valobj[] = {
  {V_CHAR, 0},
  {V_TYPE, 1, ACE_TYPE, 0, 0, SMS_ACE},
  {V_TYPEDATA, 2, 0, 0, LIST_ID, SMS_ACE},
  {V_TYPE, 3, ACE_TYPE, 0, 0, SMS_ACE},
  {V_TYPEDATA, 4, 0, 0, LIST_ID, SMS_ACE},
  {V_TYPE, 5, ACE_TYPE, 0, 0, SMS_ACE},
  {V_TYPEDATA, 6, 0, 0, LIST_ID, SMS_ACE},
  {V_TYPE, 7, ACE_TYPE, 0, 0, SMS_ACE},
  {V_TYPEDATA, 8, 0, 0, LIST_ID, SMS_ACE},
};

static struct validate azcl_validate = {
  azcl_valobj,
  8,
  CLASS,
  "z.class = \"%s\"",
  1,
  0,
  0,
  0,
  set_zephyr_modtime,
};

static char *uzcl_fields[] = {
    CLASS, "newclass", "xmt_type", "xmt_name", "sub_type", "sub_name",
    "iws_type", "iws_name", "iui_type", "iui_name",
};

static struct valobj uzcl_valobj[] = {
  {V_RENAME, 1, "zephyr", CLASS, 0, SMS_NOT_UNIQUE},
  {V_TYPE, 2, ACE_TYPE, 0, 0, SMS_ACE},
  {V_TYPEDATA, 3, 0, 0, LIST_ID, SMS_ACE},
  {V_TYPE, 4, ACE_TYPE, 0, 0, SMS_ACE},
  {V_TYPEDATA, 5, 0, 0, LIST_ID, SMS_ACE},
  {V_TYPE, 6, ACE_TYPE, 0, 0, SMS_ACE},
  {V_TYPEDATA, 7, 0, 0, LIST_ID, SMS_ACE},
  {V_TYPE, 8, ACE_TYPE, 0, 0, SMS_ACE},
  {V_TYPEDATA, 9, 0, 0, LIST_ID, SMS_ACE},
};

static struct validate uzcl_validate = {
  uzcl_valobj,
  9,
  CLASS,
  "z.class = \"%s\"",
  1,
  0,
  0,
  0,
  set_zephyr_modtime,
};

static struct validate dzcl_validate = {
  0,
  0,
  CLASS,
  "z.class = \"%s\"",
  1,
  0,
  0,
  0,
  0,
};

static char *gasv_fields[] = {
  SERVICE, "protocol", "port", DESC,
};

static char *gpcp_fields[] = {
    "printer",
    "printer", "spooling_host", "spool_directory", "rprinter", "comments",
    MOD1, MOD2, MOD3,
};

static char *apcp_fields[] = {
    "printer", "spooling_host", "spool_directory", "rprinter", "comments",
    MOD1, MOD2, MOD3,
};

static struct valobj apcp_valobj[] = {
    {V_CHAR, 0},
    {V_ID, 1, MACHINE, NAME, MACH_ID, SMS_MACHINE},
};

static struct validate apcp_validate = {
  apcp_valobj,
  2,
  NAME,
  "p.name = \"%s\"",
  1,
  0,
  0,
  0,
  set_modtime,
};

static struct validate dpcp_validate = {
  0,
  0,
  NAME,
  "p.name = \"%s\"",
  1,
  0,
  0,
  0,
  0,
};

static char *gali_fields[] = {
  NAME, TYPE, "trans",
  NAME, TYPE, "trans",
};

static char *aali_fields[] = {
  NAME, TYPE, "trans",
};

static struct valobj aali_valobj[] = {
  {V_CHAR, 0},
  {V_TYPE, 1, "alias", 0, 0, SMS_TYPE},
  {V_CHAR, 2},
};

static struct validate aali_validate = {
  aali_valobj,
  3,
  "trans",
  "a.name = \"%s\" and a.type = \"%s\" and a.trans = \"%s\"",
  3,
  0,
  0,
  0,
  0,
};

static struct validate dali_validate =	/* DELETE_ALIAS */
{
  0,
  0,
  "trans",
  "a.name = \"%s\" and a.type = \"%s\" and a.trans = \"%s\"",
  3,
  0,
  0,
  0,
  0,
};

static char *gval_fields[] = {
  NAME, "value",
};

static struct valobj gval_valobj[] = {
  {V_NAME, 0, "values", NAME, 0, SMS_NO_MATCH},
};

static struct validate gval_validate = {
  gval_valobj,
  1,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
};

static char *aval_fields[] = {
  NAME, "value",
};

static struct validate aval_validate =	/* for aval, uval, and dval */
{
  VOchar0,
  1,
  NAME,
  "v.name = \"%s\"",
  1,
  0,
  0,
  0,
  0,
};

static char *dval_fields[] = {
  NAME,
};

static char *gats_fields[] = {
  "table", "retrieves", "appends", "updates", "deletes", MOD1, MOD2, MOD3,
};



/* Generalized Query Definitions */

struct query Queries2[] = {
  {
    /* Q_GALO - GET_ALL_LOGINS */
    "get_all_logins",
    "galo",
    RETRIEVE,
    "u",
    USERS,
    "%c = u.login, %c = text(u.uid), %c = u.shell, %c = u.last, %c = u.first, %c = u.middle",
    galo_fields,
    6,
    "u.users_id != 0",
    0,
    0,
  },

  {
    /* Q_GAAL - GET_ALL_ACTIVE_LOGINS */
    "get_all_active_logins",
    "gaal",
    RETRIEVE,
    "u",
    USERS,
    "%c = u.login, %c = text(u.uid), %c = u.shell, %c = u.last, %c = u.first, %c = u.middle",
    galo_fields,
    6,
    "u.status = 1",
    0,
    0,
  },

  {
    /* Q_GUBL - GET_USER_BY_LOGIN */
    "get_user_by_login",
    "gubl",
    RETRIEVE,
    "u",
    USERS,
    "%c = u.login, %c = text(u.uid), %c = u.shell, %c = u.last, %c = u.first, %c = u.middle, %c = text(u.status), %c = u.mit_id, %c = u.mit_year, %c = u.modtime, %c = text(u.modby), %c = u.modwith",
    gubl_fields,
    12,
    "u.login = \"%s\" and u.users_id != 0",
    1,
    &gubx_validate,
  },

  {
    /* Q_GUBU - GET_USER_BY_UID */
    "get_user_by_uid",
    "gubu",
    RETRIEVE,
    "u",
    USERS,
    "%c = u.login, %c = text(u.uid), %c = u.shell, %c = u.last, %c = u.first, %c = u.middle, %c = text(u.status), %c = u.mit_id, %c = u.mit_year, %c = u.modtime, %c = text(u.modby), %c = u.modwith",
    gubu_fields,
    12,
    "u.uid = %s and u.users_id != 0",
    1,
    &gubx_validate,
  },

  {
    /* Q_GUBN - GET_USER_BY_NAME */
    "get_user_by_name",
    "gubn",
    RETRIEVE,
    "u",
    USERS,
    "%c = u.login, %c = text(u.uid), %c = u.shell, %c = u.last, %c = u.first, %c = u.middle, %c = text(u.status), %c = u.mit_id, %c = u.mit_year, %c = u.modtime, %c = text(u.modby), %c = u.modwith",
    gubn_fields,
    12,
    "u.first = \"%s\" and u.last = \"%s\" and u.users_id != 0",
    2,
    &VDsortf,
  },

  {
    /* Q_GUBC - GET_USER_BY_CLASS */
    "get_user_by_class",
    "gubc",
    RETRIEVE,
    "u",
    USERS,
    "%c = u.login, %c = text(u.uid), %c = u.shell, %c = u.last, %c = u.first, %c = u.middle, %c = text(u.status), %c = u.mit_id, %c = u.mit_year, %c = u.modtime, %c = text(u.modby), %c = u.modwith",
    gubc_fields,
    12,
    "u.mit_year = uppercase(\"%s\") and u.users_id != 0",
    1,
    &VDsortf,
  },

  {
    /* Q_GUBM - GET_USER_BY_MITID */
    "get_user_by_mitid",
    "gubm",
    RETRIEVE,
    "u",
    USERS,
    "%c = u.login, %c = text(u.uid), %c = u.shell, %c = u.last, %c = u.first, %c = u.middle, %c = text(u.status), %c = u.mit_id, %c = u.mit_year, %c = u.modtime, %c = text(u.modby), %c = u.modwith",
    gubm_fields,
    12,
    "u.mit_id = \"%s\" and u.users_id != 0",
    1,
    &VDsortf,
  },

  {
    /* Q_AUSR - ADD_USER */
    "add_user",
    "ausr",
    APPEND,
    "u",
    USERS,
    "login = %c, users_id = values.value, uid = int4(%c), shell = %c, last = %c, first = %c, middle = %c, status = int4(%c), mit_id = %c, mit_year = %c",
    ausr_fields,
    9,
    "values.name = \"users_id\"",
    0,
    &ausr_validate,
  },

  {
    /* Q_RUSR - REGISTER_USER */
    "register_user",
    "rusr",
    APPEND,
    0,
    0,
    0,
    rusr_fields,
    3,
    0,
    0,
    &rusr_validate,
  },

  {
    /* Q_UUSR - UPDATE_USER */
    "update_user",
    "uusr",
    UPDATE,
    "u",
    USERS,
    "login = %c, uid = int4(%c), shell = %c, last = %c, first = %c, middle = %c, status = int4(%c), mit_id = %c, mit_year = %c",
    uusr_fields,
    9,
    "u.users_id = %d",
    1,
    &uusr_validate,
  },

  {
    /* Q_UUSH - UPDATE_USER_SHELL */
    "update_user_shell",
    "uush",
    UPDATE,
    "u",
    USERS,
    "shell = %c",
    uush_fields,
    1,
    "u.users_id = %d",
    1,
    &uush_validate,
  },

  {
    /* Q_UUST - UPDATE_USER_STATUS */
    "update_user_status",
    "uust",
    UPDATE,
    "u",
    USERS,
    "status = int4(%c)",
    uust_fields,
    1,
    "u.users_id = %d",
    1,
    &uust_validate,
  },

  {
    /* Q_DUSR - DELETE_USER */
    "delete_user",
    "dusr",
    DELETE,
    "u",
    USERS,
    (char *)0,
    dusr_fields,
    0,
    "u.users_id = %d",
    1,
    &dusr_validate,
  },

  {
    /* Q_DUBU - DELETE_USER_BY_UID */
    "delete_user_by_uid",
    "dubu",
    DELETE,
    "u",
    USERS,
    (char *)0,
    dubu_fields,
    0,
    "u.users_id = %d",
    1,
    &dubu_validate,
  },

  {
    /* Q_GFBL - GET_FINGER_BY_LOGIN */
    "get_finger_by_login",
    "gfbl",
    RETRIEVE,
    "u",
    USERS,
    "%c = u.login,%c=u.fullname,%c=u.nickname,%c=u.home_addr,%c=u.home_phone,%c=u.office_addr,%c=u.office_phone,%c=u.mit_dept,%c=u.mit_affil,%c=u.fmodtime,%c=text(u.fmodby),%c=u.fmodwith",
    gfbl_fields,
    12,
    "u.users_id = %d",
    1,
    &gfbl_validate,
  },

  {
    /* Q_UFBL - UPDATE_FINGER_BY_LOGIN */
    "update_finger_by_login",
    "ufbl",
    UPDATE,
    "u",
    USERS,
    "fullname=%c,nickname=%c,home_addr=%c,home_phone=%c,office_addr=%c,office_phone=%c,mit_dept=%c,mit_affil=%c",
    ufbl_fields,
    8,
    "u.users_id = %d",
    1,
    &ufbl_validate,
  },

  {
    /* Q_GPOB - GET_POBOX */
    "get_pobox",
    "gpob",
    RETRIEVE,
    "u",
    USERS,
    "%c = u.login, %c = u.potype, %c = text(u.pop_id) + \":\" + text(u.box_id), %c = u.pmodtime, %c = text(u.pmodby), %c = u.pmodwith",
    gpob_fields,
    6,
    "u.users_id = %d",
    1,
    &gpob_validate,
  },

  {
    /* Q_GAPO - GET_ALL_POBOXES */
    "get_all_poboxes",
    "gapo",
    RETRIEVE,
    "u",
    USERS,
    "%c = u.login, %c = u.potype, %c = text(u.pop_id) + \":\" + text(u.box_id)",
    gpox_fields,
    3,
    "u.potype != \"NONE\"",
    0,
    &gpox_validate,
  },
      
  {
    /* Q_GPOP - GET_POBOXES_POP */
    "get_poboxes_pop",
    "gpop",
    RETRIEVE,
    "u",
    USERS,
    "%c = u.login, %c = u.potype, %c = text(u.pop_id) + \":\" + text(u.box_id)",
    gpox_fields,
    3,
    "u.potype = \"POP\"",
    0,
    &gpox_validate
  },

  {
    /* Q_GPOF - GET_POBOXES_SMTP */
    "get_poboxes_smtp",
    "gpos",
    RETRIEVE,
    "u",
    USERS,
    "%c = u.login, %c = u.potype, %c = text(u.pop_id) + \":\" + text(u.box_id)",
    gpox_fields,
    3,
    "u.potype = \"SMTP\"",
    0,
    &gpox_validate
  },

  {
    /* Q_SPOB - SET_POBOX */
    "set_pobox",
    "spob",
    UPDATE,
    0,
    USERS,
    0,
    spob_fields,
    3,
    (char *)0,
    0,
    &spob_validate,
  },

  {
    /* Q_SPOP - SET_POBOX_POP */
    "set_pobox_pop",
    "spop",
    UPDATE,
    "u",
    USERS,
    "potype = \"POP\"",
    spob_fields,
    0,
    "u.users_id = %d",
    1,
    &spop_validate,
  },

  {
    /* Q_DPOB - DELETE_POBOX */
    "delete_pobox",
    "dpob",
    UPDATE,
    "u",
    USERS,
    "potype = \"NONE\"",
    spob_fields,
    0,
    "u.users_id = %d",
    1,
    &dpob_validate,
  },

  {
    /* Q_GMAC - GET_MACHINE */
    "get_machine",
    "gmac",
    RETRIEVE,
    "m",
    MACHINE,
    "%c = m.name, %c = m.type, %c = m.modtime, %c = text(m.modby), %c =m.modwith",
    gmac_fields,
    5,
    "m.name = uppercase(\"%s\") and m.mach_id != 0",
    1,
    &VDsortf,
  },

  {
    /* Q_AMAC - ADD_MACHINE */
    "add_machine",
    "amac",
    APPEND,
    "m",
    MACHINE,
    "name = uppercase(%c), mach_id = values.value, type = %c",
    amac_fields,
    2,
    "values.name = \"mach_id\"",
    0,
    &amac_validate,
  },

  {
    /* Q_UMAC - UPDATE_MACHINE */
    "update_machine",
    "umac",
    UPDATE,
    "m",
    MACHINE,
    "name = uppercase(%c), type = %c",
    umac_fields,
    2,
    "m.mach_id = %d",
    1,
    &umac_validate,
  },

  {
    /* Q_DMAC - DELETE_MACHINE */
    "delete_machine",
    "dmac",
    DELETE,
    "m",
    MACHINE,
    (char *)0,
    dmac_fields,
    0,
    "m.mach_id = %d",
    1,
    &dmac_validate,
  },

  {
    /* Q_GCLU - GET_CLUSTER */
    "get_cluster",
    "gclu",
    RETRIEVE,
    "c",
    CLUSTER,
    "%c = c.name, %c = c.desc, %c = c.location, %c = c.modtime, %c = text(c.modby), %c = c.modwith",
    gclu_fields,
    6,
    "c.name = \"%s\" and c.clu_id != 0",
    1,
    &VDsortf,
  },

  {
    /* Q_ACLU - ADD_CLUSTER */
    "add_cluster",
    "aclu",
    APPEND,
    "c",
    CLUSTER,
    "name = %c, clu_id = values.value, desc = %c, location = %c",
    aclu_fields,
    3,
    "values.name = \"clu_id\"",
    0,
    &aclu_validate,
  },

  {
    /* Q_UCLU - UPDATE_CLUSTER */
    "update_cluster",
    "uclu",
    UPDATE,
    "c",
    CLUSTER,
    "name = %c, desc = %c, location = %c",
    uclu_fields,
    3,
    "c.clu_id = %d",
    1,
    &uclu_validate,
  },

  {
    /* Q_DCLU - DELETE_CLUSTER */
    "delete_cluster",
    "dclu",
    DELETE,
    "c",
    CLUSTER,
    (char *)0,
    dclu_fields,
    0,
    "c.clu_id = %d",
    1,
    &dclu_validate,
  },

  {
    /* Q_GMCM - GET_MACHINE_TO_CLUSTER_MAP */
    "get_machine_to_cluster_map",
    "gmcm",
    RETRIEVE,
    "m",
    "mcmap",
    "%c = machine.name, %c = cluster.name",
    gmcm_fields,
    2,
    "machine.name = uppercase(\"%s\") and cluster.name = \"%s\" and m.clu_id = cluster.clu_id and m.mach_id = machine.mach_id",
    2,
    &VDsort2,
  },

  {
    /* Q_AMTC - ADD_MACHINE_TO_CLUSTER */
    "add_machine_to_cluster",
    "amtc",
    APPEND,
    "m",
    "mcmap",
    "mach_id = %i4, clu_id = %i4",
    gmcm_fields,
    2,
    0,
    0,
    &amtc_validate,
  },

  {
    /* Q_DMFC - DELETE_MACHINE_FROM_CLUSTER */
    "delete_machine_from_cluster",
    "dmfc",
    DELETE,
    "m",
    "mcmap",
    0,
    gmcm_fields,
    0,
    "m.mach_id = %d and m.clu_id = %d",
    2,
    &amtc_validate,
  },

  {
    /* Q_GCLD - GET_CLUSTER_DATA */
    "get_cluster_data",
    "gcld",
    RETRIEVE,
    "s",
    "svc",
    "%c = cluster.name, %c = s.serv_label, %c = s.serv_cluster",
    gcld_fields,
    3,
    "cluster.clu_id = s.clu_id and cluster.name = \"%s\" and s.serv_label = \"%s\"",
    2,
    &VDsort2,
  },

  {
    /* Q_ACLD - ADD_CLUSTER_DATA */
    "add_cluster_data",
    "acld",
    APPEND,
    "s",
    "svc",
    "clu_id = %i4, serv_label = %c, serv_cluster = %c",
    acld_fields,
    3,
    (char *)0,
    0,
    &acld_validate,
  },

  {
    /* Q_DCLD - DELETE_CLUSTER_DATA */
    "delete_cluster_data",
    "dcld",
    DELETE,
    "s",
    "svc",
    (char *)0,
    acld_fields,
    0,
    "s.clu_id = %d and s.serv_label = \"%s\" and s.serv_cluster = \"%s\"",
    3,
    &acld_validate,
  },

  {
    /* Q_GSIN - GET_SERVER_INFO */
    "get_server_info",
    "gsin",
    RETRIEVE,
    "s",
    "servers",
    "%c = s.name, %c = text(s.update_int), %c = s.target_file, %c = s.script, %c = text(s.dfgen), %c = text(s.dfcheck), %c = s.type, %c = text(s.enable), %c = text(s.inprogress), %c = text(s.harderror), %c = s.errmsg, %c = s.acl_type, %c = text(s.acl_id), %c = s.modtime, %c = text(s.modby), %c = s.modwith",
    gsin_fields,
    16,
    "s.name = uppercase(\"%s\")",
    1,
    &gsin_validate,
  },

  {
    /* Q_QGSV - QUALIFIED_GET_SERVER */
    "qualified_get_server",
    "qgsv",
    RETRIEVE,
    0,
    "servers",
    0,
    qgsv_fields,
    1,
    0,
    3,
    &qgsv_validate,
  },

  {
    /* Q_ASIN - ADD_SERVER_INFO */
    "add_server_info",
    "asin",
    APPEND,
    "s",
    "servers",
    "name = uppercase(%c), update_int = int4(%c), target_file = %c, script = %c, type = %c, enable = int1(%c), acl_type = %c, acl_id = %i4",
    asin_fields,
    8,
    (char *)0,
    0,
    &asin_validate,
  },

  {
    /* Q_USIN - UPDATE_SERVER_INFO */
    "update_server_info",
    "usin",
    UPDATE,
    "s",
    "servers",
    "update_int = int4(%c), target_file = %c, script = %c, type = %c, enable = int1(%c), acl_type = %c, acl_id = %i4",
    asin_fields,
    7,
    "s.name = uppercase(\"%s\")",
    1,
    &asin_validate,
  },

  {
    /* Q_RSVE - RESET_SERVER_ERROR */
    "reset_server_error",
    "rsve",
    UPDATE,
    "s",
    "servers",
    "harderror = 0, dfcheck = s.dfgen",
    dsin_fields,
    0,
    "s.name = uppercase(\"%s\")",
    1,
    &rsve_validate,
  },

  {
    /* Q_SSIF - SET_SERVER_INTERNAL_FLAGS */
    "set_server_internal_flags",
    "ssif",
    UPDATE,
    "s",
    "servers",
    "dfgen = int4(%c), dfcheck = int4(%c), inprogress = int1(%c), harderror = int1(%c), errmsg = %c",
    ssif_fields,
    5,
    "s.name = uppercase(\"%s\")",
    1,
    &ssif_validate,
  },

  {
    /* Q_DSIN - DELETE_SERVER_INFO */
    "delete_server_info",
    "dsin",
    DELETE,
    "s",
    "servers",
    (char *)0,
    dsin_fields,
    0,
    "s.name = uppercase(\"%s\")",
    1,
    &dsin_validate,
  },

  {
    /* Q_GSHI - GET_SERVER_HOST_INFO */
    "get_server_host_info",
    "gshi",
    RETRIEVE,
    "sh",
    "serverhosts",
    "%c = sh.service, %c = machine.name, %c = text(sh.enable), %c = text(sh.override), %c = text(sh.success), %c = text(sh.inprogress), %c = text(sh.hosterror), %c = sh.hosterrmsg, %c = text(sh.ltt), %c = text(sh.lts), %c = text(sh.value1), %c = text(sh.value2), %c = sh.value3, %c = sh.modtime, %c = text(sh.modby), %c = sh.modwith",
    gshi_fields,
    16,
    "sh.service = uppercase(\"%s\") and machine.name = uppercase(\"%s\") and machine.mach_id = sh.mach_id",
    2,
    &gshi_validate,
  },

  {
    /* Q_QGSV - QUALIFIED_GET_SERVER_HOST */
    "qualified_get_server_host",
    "qgsh",
    RETRIEVE,
    0,
    "serverhosts",
    0,
    qgsh_fields,
    2,
    0,
    6,
    &qgsh_validate,
  },

  {
    /* Q_ASHI - ADD_SERVER_HOST_INFO */
    "add_server_host_info",
    "ashi",
    APPEND,
    "sh",
    "serverhosts",
    "service = uppercase(%c), mach_id = %i4, enable = int1(%c), value1 = int4(%c), value2 = int4(%c), value3 = %c",
    ashi_fields,
    6,
    (char *)0,
    0,
    &ashi_validate,
  },

  {
    /* Q_USHI - UPDATE_SERVER_HOST_INFO */
    "update_server_host_info",
    "ushi",
    UPDATE,
    "sh",
    "serverhosts",
    "enable = int1(%c), value1 = int4(%c), value2 = int4(%c), value3 = %c",
    ashi_fields,
    4,
    "sh.service = uppercase(\"%s\") and sh.mach_id = %d",
    2,
    &ashi_validate,
  },

  {
    /* Q_RSHE - RESET_SERVER_HOST_ERROR */
    "reset_server_host_error",
    "rshe",
    UPDATE,
    "sh",
    "serverhosts",
    "hosterror = 0",
    dshi_fields,
    0,
    "sh.service = uppercase(\"%s\") and sh.mach_id = %d",
    2,
    &rshe_validate,
  },

  {
    /* Q_SSHO - SET_SERVER_HOST_OVERRIDE */
    "set_server_host_override",
    "ssho",
    UPDATE,
    "sh",
    "serverhosts",
    "override = 1",
    dshi_fields,
    0,
    "sh.service = uppercase(\"%s\") and sh.mach_id = %d",
    2,
    &ssho_validate,
  },

  {
    /* Q_SSHI - SET_SERVER_HOST_INTERNAL */
    "set_server_host_internal",
    "sshi",
    UPDATE,
    "s",
    "serverhosts",
    "override = int1(%c), success = int1(%c), inprogress = int1(%c), hosterror = int1(%c), hosterrmsg = %c, ltt = int4(%c), lts = int4(%c)",
    sshi_fields,
    7,
    "s.service = uppercase(\"%s\") and s.mach_id = %d",
    2,
    &sshi_validate,
  },

  {
    /* Q_DSHI - DELETE_SERVER_HOST_INFO */
    "delete_server_host_info",
    "dshi",
    DELETE,
    "sh",
    "serverhosts",
    (char *)0,
    dshi_fields,
    0,
    "sh.service = uppercase(\"%s\") and sh.mach_id = %d",
    2,
    &dshi_validate,
  },

  {
    /* Q_GSHA - GET_SERVER_HOST_ACCESS */
    "get_server_host_access",
    "gsha",
    RETRIEVE,
    "ha",
    "hostaccess",
    "%c = machine.name, %c = ha.acl_type, %c = text(ha.acl_id), %c = ha.modtime, %c = text(ha.modby), %c = ha.modwith",
    gsha_fields,
    6,
    "machine.name = uppercase(\"%s\") and ha.mach_id = machine.mach_id",
    1,
    &gsha_validate,
  },

  {
    /* Q_ASHA - ADD_SERVER_HOST_ACCESS */
    "add_server_host_access",
    "asha",
    APPEND,
    "ha",
    "hostaccess",
    "mach_id = %i4, acl_type = %c, acl_id = %i4",
    asha_fields,
    3,
    0,
    0,
    &asha_validate,
  },

  {
    /* Q_USHA - UPDATE_SERVER_HOST_ACCESS */
    "update_server_host_access",
    "usha",
    UPDATE,
    "ha",
    "hostaccess",
    "acl_type = %c, acl_id = %i4",
    asha_fields,
    2,
    "ha.mach_id = %d",
    1,
    &asha_validate,
  },

  {
    /* Q_DSHA - DELETE_SERVER_HOST_ACCESS */
    "delete_server_host_access",
    "dsha",
    DELETE,
    "ha",
    "hostaccess",
    0,
    asha_fields,
    0,
    "ha.mach_id = %d",
    1,
    &VDmach,
  },

  {
    /* Q_GSLO - GET_SERVER_LOCATIONS */
    "get_server_locations",
    "gslo",
    RETRIEVE,
    "sh",
    "serverhosts",
    "%c = sh.service, %c = machine.name",
    gslo_fields,
    2,
    "sh.service = uppercase(\"%s\") and sh.mach_id = machine.mach_id",
    1,
    0,
  },

  {
    /* Q_GFSL - GET_FILESYS_BY_LABEL */
    "get_filesys_by_label",
    "gfsl",
    RETRIEVE,
    "fs",
    FILESYS,
    "%c = fs.label, %c = fs.type, %c = machine.name, %c = fs.name, %c = fs.mount, %c = fs.access, %c = fs.comments, %c = users.login, %c = list.name, %c = text(fs.createflg), %c = fs.lockertype, %c = fs.modtime, %c = text(fs.modby), %c = fs.modwith",
    gfsl_fields,
    14,
    "fs.label = \"%s\" and fs.mach_id = machine.mach_id and fs.owner = users.users_id and fs.owners = list.list_id",
    1,
    &VDsortf,
  },

  {
    /* Q_GFSM - GET_FILESYS_BY_MACHINE */
    "get_filesys_by_machine",
    "gfsm",
    RETRIEVE,
    "fs",
    FILESYS,
    "%c = fs.label, %c = fs.type, %c = machine.name, %c = fs.name, %c = fs.mount, %c = fs.access, %c = fs.comments, %c = users.login, %c = list.name, %c = text(fs.createflg), %c = fs.lockertype, %c = fs.modtime, %c = text(fs.modby), %c = fs.modwith",
    gfsm_fields,
    14,
    "fs.mach_id = %d and machine.mach_id = fs.mach_id and fs.owner = users.users_id and fs.owners = list.list_id",
    1,
    &gfsm_validate,
  },

  {
    /* Q_GFSN - GET_FILESYS_BY_NFSPHYS */
    "get_filesys_by_nfsphys",
    "gfsn",
    RETRIEVE,
    "fs",
    FILESYS,
    "%c = fs.label, %c = fs.type, %c = machine.name, %c = fs.name, %c = fs.mount, %c = fs.access, %c = fs.comments, %c = users.login, %c = list.name, %c = text(fs.createflg), %c = fs.lockertype, %c = fs.modtime, %c = text(fs.modby), %c = fs.modwith",
    gfsn_fields,
    14,
    "fs.mach_id = %d and machine.mach_id = fs.mach_id and fs.owner = users.users_id and fs.owners = list.list_id and nfsphys.nfsphys_id = fs.phys_id and nfsphys.dir = \"%s\" and fs.type = \"NFS\"",
    2,
    &gfsm_validate,
  },

  {
    /* Q_GFSG - GET_FILESYS_BY_GROUP */
    "get_filesys_by_group",
    "gfsg",
    RETRIEVE,
    "fs",
    FILESYS,
    "%c = fs.label, %c = fs.type, %c = machine.name, %c = fs.name, %c = fs.mount, %c = fs.access, %c = fs.comments, %c = users.login, %c = list.name, %c = text(fs.createflg), %c = fs.lockertype, %c = fs.modtime, %c = text(fs.modby), %c = fs.modwith",
    gfsg_fields,
    14,
    "fs.owners = %d and machine.mach_id = fs.mach_id and fs.owner = users.users_id and fs.owners = list.list_id",
    1,
    &gfsg_validate,
  },

  {
    /* Q_AFIL - ADD_FILESYS */
    "add_filesys",
    "afil",
    APPEND,
    "fs",
    FILESYS,
    "filsys_id = values.value, label = %c, type = %c, mach_id = %i4, name = %c, mount = %c, access = %c, comments = %c, owner = %i4, owners = %i4, createflg = int1(%c), lockertype = %c",
    afil_fields,
    11,
    "values.name = \"filsys_id\"",
    0,
    &afil_validate,
  },

  {
    /* Q_UFIL - UPDATE_FILESYS */
    "update_filesys",
    "ufil",
    UPDATE,
    "fs",
    FILESYS,
    "label = %c, type = %c, mach_id = %i4, name = %c, mount = %c, access = %c, comments = %c, owner = %i4, owners = %i4, createflg = int1(%c), lockertype = %c",
    ufil_fields,
    11,
    "fs.filsys_id = %d",
    1,
    &ufil_validate,
  },

  {
    /* Q_DFIL - DELETE_FILESYS */
    "delete_filesys",
    "dfil",
    DELETE,
    "fs",
    FILESYS,
    (char *)0,
    dfil_fields,
    0,
    "fs.filsys_id = %d",
    1,
    &dfil_validate,
  },

  {
    /* Q_GANF - GET_ALL_NFSPHYS */
    "get_all_nfsphys",
    "ganf",
    RETRIEVE,
    "np",
    "nfsphys",
    "%c = machine.name, %c = np.dir, %c = np.device, %c = text(np.status), %c = text(np.allocated), %c = text(np.size), %c = np.modtime, %c = text(np.modby), %c = np.modwith",
    ganf_fields,
    9,
    "machine.mach_id = np.mach_id",
    0,
    &VDsortf,
  },

  {
    /* Q_GNFP - GET_NFSPHYS */
    "get_nfsphys",
    "gnfp",
    RETRIEVE,
    "np",
    "nfsphys",
    "%c = machine.name, %c = np.dir, %c = np.device, %c = text(np.status), %c = text(np.allocated), %c = text(np.size), %c = np.modtime, %c = text(np.modby), %c = np.modwith",
    gnfp_fields,
    9,
    "np.mach_id = %d and np.dir = \"%s\" and machine.mach_id = np.mach_id",
    2,
    &gnfp_validate,
  },

  {
    /* Q_ANFP - ADD_NFSPHYS */
    "add_nfsphys",
    "anfp",
    APPEND,
    "np",
    "nfsphys",
    "nfsphys_id = values.value, mach_id = %i4, dir = %c, device = %c, status = int4(%c), allocated = int4(%c), size = int4(%c)",
    ganf_fields,
    6,
    "values.name = \"nfsphys_id\"",
    0,
    &anfp_validate,
  },

  {
    /* Q_UNFP - UPDATE_NFSPHYS */
    "update_nfsphys",
    "unfp",
    UPDATE,
    "np",
    "nfsphys",
    "device = %c, status = int4(%c), allocated = int4(%c), size = int4(%c)",
    gnfp_fields,
    4,
    "np.mach_id = %d and np.dir = \"%s\"",
    2,
    &anfp_validate,
  },

  {
    /* Q_AJNF - ADJUST_NFSPHYS_ALLOCATION */
    "adjust_nfsphys_allocation",
    "ajnf",
    UPDATE,
    "np",
    "nfsphys",
    "allocated = np.allocated + int4(%c)",
    dnfp_fields,
    1,
    "np.mach_id = %d and np.dir = \"%s\"",
    2,
    &anfp_validate,
  },

  {
    /* Q_DNFP - DELETE_NFSPHYS */
    "delete_nfsphys",
    "dnfp",
    DELETE,
    "np",
    "nfsphys",
    (char *)0,
    dnfp_fields,
    0,
    "np.mach_id = %d and np.dir = \"%s\"",
    2,
    &dnfp_validate,
  },

  {
    /* Q_GNFQ - GET_NFS_QUOTAS */
    "get_nfs_quota",
    "gnfq",
    RETRIEVE,
    "nq",
    "nfsquota",
    "%c = filesys.label, %c = users.login, %c = text(nq.quota), %c = nfsphys.dir, %c = machine.name, %c = nq.modtime, %c = text(nq.modby), %c = nq.modwith",
    gnfq_fields,
    8,
    "filesys.label = \"%s\" and nq.users_id = %d and filesys.filsys_id = nq.filsys_id and nfsphys.nfsphys_id = nq.phys_id and machine.mach_id = filesys.mach_id and users.users_id = nq.users_id",
    2,
    &gnfq_validate,
  },

  {
    /* Q_GNQP - GET_NFS_QUOTAS_BY_PARTITION */
    "get_nfs_quotas_by_partition",
    "gnqp",
    RETRIEVE,
    "nq",
    "nfsquota",
    "%c = filesys.label, %c = users.login, %c = text(nq.quota), %c = nfsphys.dir, %c = machine.name",
    gnqp_fields,
    5,
    "nfsphys.mach_id = %d and nfsphys.dir = \"%s\" and nq.phys_id = nfsphys.nfsphys_id and filesys.filsys_id = nq.filsys_id and users.users_id = nq.users_id and machine.mach_id = nfsphys.mach_id",
    2,
    &VDmach,
  },

  {
    /* Q_ANFQ - ADD_NFS_QUOTA */
    "add_nfs_quota",
    "anfq",
    APPEND,
    "nq",
    "nfsquota",
    "filsys_id = filesys.filsys_id, users_id = %i4, quota = int4(%c), phys_id = filesys.phys_id",
    anfq_fields,
    2,
    "filesys.filsys_id = %d",
    1,
    &anfq_validate,
  },

  {
    /* Q_UNFQ - UPDATE_NFS_QUOTA */
    "update_nfs_quota",
    "unfq",
    UPDATE,
    "nq",
    "nfsquota",
    "quota = int4(%c)",
    anfq_fields,
    1,
    "nq.filsys_id = %d and nq.users_id = %d",
    2,
    &unfq_validate,
  },

  {
    /* Q_DNFQ - DELETE_NFS_QUOTA */
    "delete_nfs_quota",
    "dnfq",
    DELETE,
    "nq",
    "nfsquota",
    (char *)0,
    anfq_fields,
    0,
    "nq.filsys_id = %d and nq.users_id = %d",
    2,
    &dnfq_validate,
  },

  {
    /* Q_GLIN - GET_LIST_INFO */
    "get_list_info",
    "glin",
    RETRIEVE,
    0,
    LIST,
    0,
    glin_fields,
    13,
    0,
    1,
    &glin_validate,
  },
    
  {
    /* Q_EXLN - EXPAND_LIST_NAMES */
    "expand_list_names",
    "exln",
    RETRIEVE,
    "l",
    LIST,
    "%c = l.name",
    glin_fields,
    1,
    "l.name = \"%s\" and l.list_id != 0",
    1,
    0
  },
    
  {
    /* Q_ALIS - ADD_LIST */
    "add_list",
    "alis",
    APPEND,
    "l",
    LIST,
    "list_id=values.value, name=%c, active=int1(%c), public=int1(%c), hidden=int1(%c), maillist=int1(%c), group=int1(%c), gid=int2(%c), acl_type=%c, acl_id=%i4, desc=%c",
    alis_fields,
    10,
    "values.name = \"list_id\"",
    0,
    &alis_validate,
  },

  {
    /* Q_ULIS - UPDATE_LIST */
    "update_list",
    "ulis",
    UPDATE,
    "l",
    LIST,
    "name=%c, active=int1(%c), public=int1(%c), hidden=int1(%c), maillist=int1(%c), group=int1(%c), gid=int2(%c), acl_type=%c, acl_id=%i4, desc=%c",
    ulis_fields,
    10,
    "l.list_id = %d",
    1,
    &ulis_validate,
  },

  {
    /* Q_DLIS - DELETE_LIST */
    "delete_list",
    "dlis",
    DELETE,
    "l",
    LIST,
    (char *)0,
    dlis_fields,
    0,
    "l.list_id = %d",
    1,
    &dlis_validate,
  },

  {
    /* Q_AMTL - ADD_MEMBER_TO_LIST */
    "add_member_to_list",
    "amtl",
    APPEND,
    "m",
    "members",
    "list_id=%i4, member_type=%c, member_id=%i4",
    amtl_fields,
    3,
    (char *)0,
    0,
    &amtl_validate,
  },

  {
    /* Q_DMFL - DELETE_MEMBER_FROM_LIST */
    "delete_member_from_list",
    "dmfl",
    DELETE,
    "m",
    "members",
    (char *)0,
    amtl_fields,
    0,
    "m.list_id = %d and m.member_type = \"%s\" and m.member_id = %d",
    3,
    &amtl_validate,
  },

  {
    /* Q_GAUS - GET_ACE_USE */
    "get_ace_use",
    "gaus",
    RETRIEVE,
    0,
    0,
    0,
    gaus_fields,
    2,
    0,
    2,
    &gaus_validate,
  },

  {
    /* Q_QGLI - QUALIFIED_GET_LISTS */
    "qualified_get_lists",
    "qgli",
    RETRIEVE,
    0,
    LIST,
    0,
    qgli_fields,
    1,
    0,
    5,
    &qgli_validate,
  },

  {
    /* Q_GMOL - GET_MEMBERS_OF_LIST */
    "get_members_of_list",
    "gmol",
    RETRIEVE,
    (char *)0,
    "members",
    (char *)0,
    gmol_fields,
    0,
    (char *)0,
    1,
    &gmol_validate,
  },

  {
    /* Q_GLOM - GET_LISTS_OF_MEMBER */
    "get_lists_of_member",
    "glom",
    RETRIEVE,
    0,
    "members",
    0,
    glom_fields,
    6,
    0,
    2,
    &glom_validate,
  },

  {
    /* Q_CMOL - COUNT_MEMBERS_OF_LIST */
    "count_members_of_list",
    "cmol",
    RETRIEVE,
    0,
    "members",
    0,
    cmol_fields,
    1,
    0,
    1,
    &cmol_validate,
  },
#ifdef notdef
  {
    /* Q_AUGR - ADD_USER_GROUP */
    "add_user_group",
    "augr",
    APPEND,
    (char *)0,
    (char *)0,
    (char *)0,
    augr_fields,
    1,
    (char *)0,
    0,
    &augr_validate,
  },    
#endif
  {
    /* Q_GZCL - GET_ZEPHYR_CLASS */
    "get_zephyr_class",
    "gzcl",
    RETRIEVE,
    "z",
    "zephyr",
    "%c = z.class, %c = z.xmt_type, %c = text(z.xmt_id),%c = z.sub_type, %c = text(z.sub_id),%c = z.iws_type, %c = text(z.iws_id),%c = z.iui_type, %c = text(z.iui_id), %c=z.modtime, %c = text(z.modby), %c = z.modwith",
    gzcl_fields,
    12,
    "z.class = \"%s\"",
    1,
    &gzcl_validate,
  },    

  {
    /* Q_AZCL - ADD_ZEPHYR_CLASS */
    "add_zephyr_class",
    "azcl",
    APPEND,
    "z",
    "zephyr",
    "class = %c, xmt_type = %c, xmt_id = %i4, sub_type = %c, sub_id = %i4, iws_type = %c, iws_id = %i4, iui_type = %c, iui_id = %i4",
    azcl_fields,
    9,
    0,
    0,
    &azcl_validate,
  },    

  {
    /* Q_UZCL - UPDATE_ZEPHYR_CLASS */
    "update_zephyr_class",
    "uzcl",
    UPDATE,
    "z",
    "zephyr",
    "class = %c, xmt_type = %c, xmt_id = %i4, sub_type = %c, sub_id = %i4, iws_type = %c, iws_id = %i4, iui_type = %c, iui_id = %i4",
    uzcl_fields,
    9,
    "z.class = \"%s\"",
    1,
    &uzcl_validate,
  },    

  {
    /* Q_DZCL - DELETE_ZEPHYR_CLASS */
    "delete_zephyr_class",
    "dzcl",
    DELETE,
    "z",
    "zephyr",
    0,
    uzcl_fields,
    0,
    "z.class = \"%s\"",
    1,
    &dzcl_validate,
  },    
#ifdef notdef
  {
    /* Q_GSVC - GET_SERVICE */
    "get_service",
    "gsvc",
    RETRIEVE,
    "s",
    "services",
    "%c = s.name, %c = s.protocol, %c = text(s.port), %c = s.desc, %c = s.modtime, %c = text(s.modby), %c = s.modwith",
    gasv_fields,
    7,
    (char *)0,
    0,
    &gsvc_validate,
  },

  {
    /* Q_ASVC - ADD_SERVICE */
    "add_service",
    "asvc",
    APPEND,
    "s",
    "services",
    "name = %c, protocol = %c, port = int2(%c), desc = %c",
    gasv_fields,
    4,
    (char *)0,
    0,
    &asvc_validate,
  },

  {
    /* Q_DSVC - DELETE_SERVICE */
    "delete_service",
    "dsvc",
    DELETE,
    "s",
    "services",
    0,
    gasv_fields,
    0,
    "s.name = \"%s\"",
    1,
    &dsvc_validate,
  },
#endif
  {
    /* Q_GPCP - GET_PRINTCAP */
    "get_printcap",
    "gpcp",
    RETRIEVE,
    "p",
    "printcap",
    "%c = p.name, %c = machine.name, %c = p.dir, %c = p.rp, %c = p.comments, %c = p.modtime, %c = text(p.modby), %c = p.modwith",
    gpcp_fields,
    8,
    "p.name = \"%s\" and machine.mach_id = p.mach_id",
    1,
    &VDsortf,
  },

  {
    /* Q_APCP - ADD_PRINTCAP */
    "add_printcap",
    "apcp",
    APPEND,
    "p",
    "printcap",
    "name = %c, mach_id = %i4, dir = %c, rp = %c, comments = %c",
    apcp_fields,
    5,
    0,
    0,
    &apcp_validate,
  },

  {
    /* Q_DPCP - DELETE_PRINTCAP */
    "delete_printcap",
    "dpcp",
    DELETE,
    "p",
    "printcap",
    0,
    apcp_fields,
    0,
    "p.name = \"%s\"",
    1,
    &dpcp_validate,
  },

  {
    /* Q_GALI - GET_ALIAS */
    "get_alias",
    "gali",
    RETRIEVE,
    "a",
    "alias",
    "%c = a.name, %c = a.type, %c = a.trans",
    gali_fields,
    3,
    "a.name = \"%s\" and a.type = \"%s\" and a.trans = \"%s\"",
    3,
    0,
  },

  {
    /* Q_AALI - ADD_ALIAS */
    "add_alias",
    "aali",
    APPEND,
    "a",
    "alias",
    "name = %c, type = %c, trans = %c",
    aali_fields,
    3,
    (char *)0,
    0,
    &aali_validate,
  },

  {
    /* Q_DALI - DELETE_ALIAS */
    "delete_alias",
    "dali",
    DELETE,
    "a",
    "alias",
    (char *)0,
    aali_fields,
    0,
    "a.name = \"%s\" and a.type = \"%s\" and a.trans = \"%s\"",
    3,
    &dali_validate,
  },

  {
    /* Q_GVAL - GET_VALUE */
    "get_value",
    "gval",
    RETRIEVE,
    "v",
    "values",
    "%c = text(v.value)",
    gval_fields,
    1,
    "v.name = \"%s\"",
    1,
    &gval_validate,
  },

  {
    /* Q_AVAL - ADD_VALUE */
    "add_value",
    "aval",
    APPEND,
    "v",
    "values",
    "name = %c, value = int4(%c)",
    aval_fields,
    2,
    (char *)0,
    0,
    &aval_validate,
  },

  {
    /* Q_UVAL - UPDATE_VALUE */
    "update_value",
    "uval",
    UPDATE,
    "v",
    "values",
    "value = int4(%c)",
    aval_fields,
    1,
    "v.name = \"%s\"",
    1,
    &aval_validate,
  },

  {
    /* Q_DVAL - DELETE_VALUE */
    "delete_value",
    "dval",
    DELETE,
    "v",
    "values",
    (char *)0,
    dval_fields,
    0,
    "v.name = \"%s\"",
    1,
    &aval_validate,
  },

  {
    /* Q_GATS - GET_ALL_TABLE_STATS */
    "get_all_table_stats",
    "gats",
    RETRIEVE,
    "tbs",
    "tblstats",
    "%c = tbs.table, %c = text(tbs.retrieves), %c = text(tbs.appends), %c = text(tbs.updates), %c = text(tbs.deletes), %c = tbs.modtime",
    gats_fields,
    6,
    (char *)0,
    0,
    0,
  },

};

int QueryCount2 = (sizeof Queries2 / sizeof (struct query));
