/* $Id: queries2.c,v 2.67 2000-01-11 19:50:03 danw Exp $
 *
 * This file defines the query dispatch table
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include "mr_server.h"
#include "query.h"

#include "mr_et.h"

/* VALOBJS
 * These are commonly used validation objects, defined here so that they
 * can be shared.
 */

static struct valobj VOuser0[] = {
  {V_ID, 0, USERS_TABLE, "login", "users_id", MR_USER},
};

static struct valobj VOmach0[] = {
  {V_ID, 0, MACHINE_TABLE, "name", "mach_id", MR_MACHINE},
};

static struct valobj VOclu0[] = {
  {V_ID, 0, CLUSTERS_TABLE, "name", "clu_id", MR_CLUSTER},
};

static struct valobj VOsnet0[] = {
  {V_ID, 0, SUBNET_TABLE, "name", "snet_id", MR_SUBNET},
};

static struct valobj VOlist0[] = {
  {V_ID, 0, LIST_TABLE, "name", "list_id", MR_LIST},
};

static struct valobj VOfilesys0[] = {
  {V_ID, 0, FILESYS_TABLE, "label", "filsys_id", MR_FILESYS},
};

static struct valobj VOnum0[] = {
  {V_NUM, 0},
};

/* Validations
 * Commonly used validation records defined here so that they may
 * be shared.
 */

static struct validate VDmach = { VOmach0, 1 };

static struct validate VDfix_modby = {
  0,
  0,
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
  "login", "unix_uid", "shell", "last", "first", "middle",
};

static char *gual_fields[] = {
  "login",
  "login", "unix_uid", "shell", "last", "first", "middle", "status",
  "clearid", "class", "comments", "signature", "secure",
  "modtime", "modby", "modwith",
};

static char *gubl_fields[] = {
  "login",
  "login", "unix_uid", "shell", "last", "first", "middle", "status",
  "clearid", "class", "modtime", "modby", "modwith",
};

static struct validate gubl_validate =
{
  0,
  0,
  0,
  0,
  0,
  0,
  access_login,
  0,
  followup_guax,
};

static char *guau_fields[] = {
  "unix_uid",
  "login", "unix_uid", "shell", "last", "first", "middle", "status",
  "clearid", "class", "comments", "signature", "secure",
  "modtime", "modby", "modwith",
};

static char *guan_fields[] = {
  "first", "last",
  "login", "unix_uid", "shell", "last", "first", "middle", "status",
  "clearid", "class", "comments", "signature", "secure",
  "modtime", "modby", "modwith",
};

static struct validate guan_validate =
{
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  followup_guax,
};

static char *guac_fields[] = {
  "class",
  "login", "unix_uid", "shell", "last", "first", "middle", "status",
  "clearid", "class", "comments", "signature", "secure",
  "modtime", "modby", "modwith",
};

static char *guam_fields[] = {
  "clearid",
  "login", "unix_uid", "shell", "last", "first", "middle", "status",
  "clearid", "class", "comments", "signature", "secure",
  "modtime", "modby", "modwith",
};

static char *gubu_fields[] = {
  "unix_uid",
  "login", "unix_uid", "shell", "last", "first", "middle", "status",
  "clearid", "class", "modtime", "modby", "modwith",
};

static struct validate gubu_validate =
{
  VOnum0,
  1,
  0,
  0,
  0,
  0,
  access_login,
  0,
  followup_fix_modby,
};

static char *gubn_fields[] = {
  "first", "last",
  "login", "unix_uid", "shell", "last", "first", "middle", "status",
  "clearid", "class", "modtime", "modby", "modwith",
};

static struct validate gubn_validate =
{
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  followup_fix_modby,
};

static char *gubc_fields[] = {
  "class",
  "login", "unix_uid", "shell", "last", "first", "middle", "status",
  "clearid", "class", "modtime", "modby", "modwith",
};

static char *gubm_fields[] = {
  "clearid",
  "login", "unix_uid", "shell", "last", "first", "middle", "status",
  "clearid", "class", "modtime", "modby", "modwith",
};

static char *auac_fields[] = {
  "login", "unix_uid", "shell", "last", "first", "middle", "status",
  "clearid", "class", "comments", "signature", "secure",
};

static struct valobj auac_valobj[] = {
  {V_CHAR, 0, USERS_TABLE, "login"},
  {V_NUM, 1},
  {V_CHAR, 2, USERS_TABLE, "shell"},
  {V_CHAR, 3, USERS_TABLE, "last"},
  {V_CHAR, 4, USERS_TABLE, "first"},
  {V_CHAR, 5, USERS_TABLE, "middle"},
  {V_NUM, 6},
  {V_CHAR, 7, USERS_TABLE, "clearid"},
  {V_TYPE, 8, 0, "class", 0, MR_BAD_CLASS},
  {V_ID, 9, STRINGS_TABLE, "string", "string_id", MR_NO_MATCH},
  {V_NUM, 11},
};

static struct validate auac_validate = {
  auac_valobj,
  11,
  "login",
  "login = '%s'",
  1,
  "users_id",
  0,
  setup_ausr,
  followup_ausr,
};

static struct validate ausr_validate = {
  auac_valobj,
  9,
  "login",
  "login = '%s'",
  1,
  "users_id",
  0,
  setup_ausr,
  followup_ausr,
};

static char *rusr_fields[] = {
  "unix_uid", "login", "pobox_type"
};

static struct valobj rusr_valobj[] = {
  {V_NUM, 0},
  {V_CHAR, 1, USERS_TABLE, "login"},
};

static struct validate rusr_validate = {
  rusr_valobj,
  2,
  0,
  0,
  0,
  0,
  0,
  register_user,
  0,
};

static char *uuac_fields[] = {
  "login",
  "newlogin", "unix_uid", "shell", "last", "first", "middle", "status",
  "clearid", "class", "comments", "signature", "secure",
};

static struct valobj uuac_valobj[] = {
  {V_ID, 0, USERS_TABLE, "login", "users_id", MR_USER},
  {V_RENAME, 1, USERS_TABLE, "login", "users_id", MR_NOT_UNIQUE},
  {V_NUM, 2},
  {V_CHAR, 3, USERS_TABLE, "shell"},
  {V_CHAR, 4, USERS_TABLE, "first"},
  {V_CHAR, 5, USERS_TABLE, "last"},
  {V_CHAR, 6, USERS_TABLE, "middle"},
  {V_NUM, 7},
  {V_CHAR, 8, USERS_TABLE, "clearid"},
  {V_TYPE, 9, 0, "class", 0, MR_BAD_CLASS},
  {V_ID, 10, STRINGS_TABLE, "string", "string_id", MR_NO_MATCH},
  {V_NUM, 12},
};

static struct validate uuac_validate = {
  uuac_valobj,
  12,
  0,
  0,
  0,
  "users_id",
  0,
  setup_ausr,
  followup_uuac,
};

static struct validate uusr_validate = {
  uuac_valobj,
  11,
  0,
  0,
  0,
  "users_id",
  0,
  setup_ausr,
  set_modtime_by_id,
};

static char *uush_fields[] = {
  "login",
  "shell",
};

static struct valobj uush_valobj[] = {
  {V_ID, 0, USERS_TABLE, "login", "users_id", MR_USER},
  {V_CHAR, 1, USERS_TABLE, "shell"},
};

static struct validate uush_validate = {
  uush_valobj,
  2,
  0,
  0,
  0,
  "users_id",
  access_user,
  0,
  set_finger_modtime,
};

static char *uust_fields[] = {
  "login",
  "status",
};

static char *uuss_fields[] = {
  "login",
  "secure",
};

static struct valobj uust_valobj[] = {
  {V_ID, 0, USERS_TABLE, "login", "users_id", MR_USER},
  {V_NUM, 1},
  {V_NUM, 2},
};

static struct validate uust_validate = {
  uust_valobj,
  2,
  0,
  0,
  0,
  "users_id",
  0,
  0,
  set_modtime_by_id,
};

static char *dusr_fields[] = {
  "login",
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

static char *aurv_fields[] = {
  "login", "reservation",
};

static struct validate aurv_validate = {
  VOuser0,
  1,
  0,
  0,
  0,
  "users_id",
  0,
  0,
  do_user_reservation,
};

static char *gurv_fields[] = {
  "login",
  "reservation",
};

static struct validate gurv_validate = {
  VOuser0,
  1,
  0,
  0,
  0,
  0,
  0,
  0,
  get_user_reservations,
};

static char *gubr_fields[] = {
  "reservation",
  "login",
};

static struct validate gubr_validate = {
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  get_user_by_reservation,
};

static char *gkum_fields[] = { "login", "kerberos",
				   "login", "kerberos" };

static char *akum_fields[] = { "login", "kerberos" };

static struct valobj akum_valobj[] =
{
  {V_ID, 0, USERS_TABLE, "login", "users_id", MR_USER},
  {V_ID, 1, STRINGS_TABLE, "string", "string_id", MR_NO_MATCH},
};

static struct validate akum_validate =
{
  akum_valobj,
  2,
  "users_id",
  "users_id = %d or string_id = %d",
  2,
  "users_id",
  access_user,
  0,
  0,
};

static struct validate dkum_validate =
{
  akum_valobj,
  2,
  "users_id",
  "users_id = %d and string_id = %d",
  2,
  "users_id",
  access_user,
  0,
  0,
};

static char *gfbl_fields[] = {
  "login",
  "login", "fullname", "nickname", "home_addr",
  "home_phone", "office_addr", "office_phone", "department",
  "affiliation", "modtime", "modby", "modwith",
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
  "login",
  "fullname", "nickname", "home_addr",
  "home_phone", "office_addr", "office_phone", "department",
  "affiliation",
};

static struct valobj ufbl_valobj[] = {
  {V_ID, 0, USERS_TABLE, "login", "users_id", MR_USER},
  {V_LEN, 1, USERS_TABLE, "fullname"},
  {V_LEN, 2, USERS_TABLE, "nickname"},
  {V_LEN, 3, USERS_TABLE, "home_addr"},
  {V_LEN, 4, USERS_TABLE, "home_phone"},
  {V_LEN, 5, USERS_TABLE, "office_addr"},
  {V_LEN, 6, USERS_TABLE, "office_phone"},
  {V_LEN, 7, USERS_TABLE, "department"},
  {V_LEN, 8, USERS_TABLE, "affiliation"},
};

static struct validate ufbl_validate = {
  ufbl_valobj,
  9,
  0,
  0,
  0,
  0,
  access_user,
  0,
  set_finger_modtime,
};

static char *gpob_fields[] = {
  "login",
  "login", "type", "box", "address", "modtime", "modby", "modwith",
};

static struct validate gpob_validate = {
  VOuser0,
  1,
  "potype",
  "potype != 'NONE' and users_id = %d",
  1,
  0,
  access_user,
  0,
  followup_gpob,
};

static char *gpox_fields[] = {
  "login", "type", "box",
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
  "login", "type", "box",
};

static struct valobj spob_valobj[] = {
  {V_ID, 0, USERS_TABLE, "login", "users_id", MR_USER},
  {V_TYPE, 1, 0, "pobox", 0, MR_TYPE},
};

static struct validate spob_validate =	/* SET_POBOX */
{
  spob_valobj,
  2,
  0,
  0,
  0,
  0,
  access_spob,
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
  0,
  set_pobox_pop,
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
  "name",
  "name", "type", "modtime", "modby", "modwith",
};

static char *ghst_fields[] = {
  "name", "address", "location", "network",
  "name", "vendor", "model", "os", "location", "contact", "use",
  "status", "status_change", "network", "address", "ace_type",
  "ace_name", "admin_comment", "ops_comment", "created", "creator",
  "inuse", "modtime", "modby", "modwith",
};

static struct validate ghst_validate = {
  0,
  0,
  0,
  0,
  0,
  0,
  access_host,
  0,
  followup_ghst,
};

static char *ghha_fields[] = {
  "name",
  "hwaddr",
};

static char *ahst_fields[] = {
  "name", "vendor", "model", "os", "location", "contact", "use",
  "status", "subnet", "address", "ace_type", "ace_name",
  "admin_comment", "ops_comment",
};

static struct valobj ahst_valobj[] = {
  {V_CHAR, 0, MACHINE_TABLE, "name"},
  {V_CHAR, 1, MACHINE_TABLE, "vendor"},
  {V_CHAR, 2, MACHINE_TABLE, "model"},
  {V_CHAR, 3, MACHINE_TABLE, "os"},
  {V_CHAR, 4, MACHINE_TABLE, "location"},
  {V_CHAR, 5, MACHINE_TABLE, "contact"},
  {V_NUM, 6},
  {V_NUM, 7},
  {V_ID, 8, SUBNET_TABLE, "name", "snet_id", MR_SUBNET},
  {V_TYPE, 10, 0, "ace_type", 0, MR_ACE},
  {V_TYPEDATA, 11, 0, 0, 0, MR_ACE},
  {V_ID, 12, STRINGS_TABLE, "string", "string_id", MR_NO_MATCH},
  {V_ID, 13, STRINGS_TABLE, "string", "string_id", MR_NO_MATCH},
};

static struct validate ahst_validate = {
  ahst_valobj,
  13,
  "name",
  "name = UPPER('%s')",
  1,
  "mach_id",
  access_host,
  setup_ahst,
  set_uppercase_modtime,
};

static char *uhst_fields[] = {
  "name",
  "newname", "vendor", "model", "os", "location", "contact", "use",
  "status", "subnet", "address", "ace_type", "ace_name",
  "admin_comment", "ops_comment",
};

static struct valobj uhst_valobj[] = {
  {V_CHAR, 0, MACHINE_TABLE, "name"},
  {V_ID, 0, MACHINE_TABLE, "name", "mach_id", MR_MACHINE},
  {V_RENAME, 1, MACHINE_TABLE, "name", "mach_id", MR_NOT_UNIQUE},
  {V_CHAR, 2, MACHINE_TABLE, "vendor"},
  {V_CHAR, 3, MACHINE_TABLE, "model"},
  {V_CHAR, 4, MACHINE_TABLE, "os"},
  {V_CHAR, 5, MACHINE_TABLE, "location"},
  {V_CHAR, 6, MACHINE_TABLE, "contact"},
  {V_NUM, 7},
  {V_NUM, 8},
  {V_ID, 9, SUBNET_TABLE, "name", "snet_id", MR_SUBNET},
  {V_TYPE, 11, 0, "ace_type", 0, MR_ACE},
  {V_TYPEDATA, 12, 0, 0, 0, MR_ACE},
  {V_ID, 13, STRINGS_TABLE, "string", "string_id", MR_NO_MATCH},
  {V_ID, 14, STRINGS_TABLE, "string", "string_id", MR_NO_MATCH},
};

static struct validate uhst_validate = {
  uhst_valobj,
  15,
  0,
  0,
  0,
  "mach_id",
  access_host,
  setup_ahst,
  set_modtime_by_id,
};

static char *uhha_fields[] = {
  "name",
  "hwaddr"
};

static struct valobj uhha_valobj[] = {
  {V_ID, 0, MACHINE_TABLE, "name", "mach_id", MR_MACHINE},
  {V_CHAR, 1, MACHINE_TABLE, "hwaddr"},
};

static struct validate uhha_validate = {
  uhha_valobj,
  2,
  0,
  0,
  0,
  "mach_id",
  0,
  setup_uhha,
  set_modtime_by_id,
};

static char *dhst_fields[] = {
  "name",
};

static struct validate dhst_validate = {
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

static char *ghal_fields[] = {
  "alias", "canonical_hostname",
  "alias", "canonical_hostname"
};

static struct validate ghal_validate = {
  0,
  0,
  0,
  0,
  0,
  0,
  access_ahal,
  0,
  0,
};

static struct valobj ahal_valobj[] = {
  {V_CHAR, 0, HOSTALIAS_TABLE, "name"},
  {V_ID, 1, MACHINE_TABLE, "name", "mach_id", MR_MACHINE},
};

static struct validate ahal_validate = {
  ahal_valobj,
  2,
  "name",
  "name = UPPER('%s')",
  1,
  "mach_id",
  access_ahal,
  setup_ahal,
  0,
};

static struct valobj dhal_valobj[] = {
  {V_ID, 1, MACHINE_TABLE, "name", "mach_id", MR_MACHINE},
};

static struct validate dhal_validate = {
  dhal_valobj,
  1,
  "name",
  "name = UPPER('%s') AND mach_id = %d",
  2,
  "mach_id",
  access_ahal,
  0,
  0,
};

static char *gsnt_fields[] = {
  "name",
  "name", "description", "address", "mask", "low", "high", "prefix",
  "ace_type", "ace_name", "modtime", "modby", "modwith"
};

static struct validate gsnt_validate = {
  0,
  0,
  0,
  0,
  0,
  0,
  access_snt,
  0,
  followup_gsnt,
};

static char *asnt_fields[] = {
  "name", "description", "address", "mask", "low", "high", "prefix",
  "ace_type", "ace_name",
};

static struct valobj asnt_valobj[] = {
  {V_CHAR, 0, SUBNET_TABLE, "name"},
  {V_LEN, 1, SUBNET_TABLE, "description"},
  {V_NUM, 2},
  {V_NUM, 3},
  {V_NUM, 4},
  {V_NUM, 5},
  {V_LEN, 6, SUBNET_TABLE, "prefix"},
  {V_TYPE, 7, 0, "ace_type", 0, MR_ACE},
  {V_TYPEDATA, 8, 0, 0, 0, MR_ACE},
};

static struct validate asnt_validate =
{
  asnt_valobj,
  9,
  "name",
  "name = UPPER('%s')",
  1,
  "snet_id",
  0,
  prefetch_value,
  set_uppercase_modtime,
};

static char *usnt_fields[] = {
  "name",
  "newname", "description", "address", "mask", "low", "high", "prefix",
  "ace_type", "ace_name",
};

static struct valobj usnt_valobj[] = {
  {V_ID, 0, SUBNET_TABLE, "name", "snet_id", MR_NO_MATCH},
  {V_RENAME, 1, SUBNET_TABLE, "name", "snet_id", MR_NOT_UNIQUE},
  {V_LEN, 2, SUBNET_TABLE, "description"},
  {V_NUM, 3},
  {V_NUM, 4},
  {V_NUM, 5},
  {V_NUM, 6},
  {V_LEN, 7, SUBNET_TABLE, "prefix"},
  {V_TYPE, 8, 0, "ace_type", 0, MR_ACE},
  {V_TYPEDATA, 9, 0, 0, 0, MR_ACE},
};

static struct validate usnt_validate =
{
  usnt_valobj,
  10,
  "name",
  "snet_id = %d",
  1,
  "snet_id",
  0,
  0,
  set_modtime_by_id,
};

static char *dsnt_fields[] = {
  "name",
};

static struct validate dsnt_validate = {
  VOsnet0,
  1,
  0,
  0,
  0,
  0,
  0,
  setup_dsnt,
  0,
};

static char *gclu_fields[] = {
  "name",
  "name", "description", "location", "modtime", "modby", "modwith",
};

static char *aclu_fields[] = {
  "name", "description", "location",
};

static struct valobj aclu_valobj[] = {
  {V_CHAR, 0, CLUSTERS_TABLE, "name"},
  {V_LEN, 1, CLUSTERS_TABLE, "description"},
  {V_LEN, 2, CLUSTERS_TABLE, "location"},
};

static struct validate aclu_validate =
{
  aclu_valobj,
  3,
  "name",
  "name = '%s'",
  1,
  "clu_id",
  0,
  prefetch_value,
  set_modtime,
};

static char *uclu_fields[] = {
  "name",
  "newname", "description", "location",
};

static struct valobj uclu_valobj[] = {
  {V_ID, 0, CLUSTERS_TABLE, "name", "clu_id", MR_CLUSTER},
  {V_RENAME, 1, CLUSTERS_TABLE, "name", "clu_id", MR_NOT_UNIQUE},
  {V_LEN, 2, CLUSTERS_TABLE, "description"},
  {V_LEN, 3, CLUSTERS_TABLE, "location"},
};

static struct validate uclu_validate = {
  uclu_valobj,
  4,
  0,
  0,
  0,
  "clu_id",
  0,
  0,
  set_modtime_by_id,
};

static char *dclu_fields[] = {
  "name",
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
  "machine", "cluster",
  "machine", "cluster",
};

static struct valobj amtc_valobj[] =	/* ADD_MACHINE_TO_CLUSTER */
{					/* DELETE_MACHINE_FROM_CLUSTER */
  {V_ID, 0, MACHINE_TABLE, "name", "mach_id", MR_MACHINE},
  {V_ID, 1, CLUSTERS_TABLE, "name", "clu_id", MR_CLUSTER},
};

static struct validate amtc_validate = /* for amtc and dmfc */
{
  amtc_valobj,
  2,
  "mach_id",
  "mach_id = %d and clu_id = %d",
  2,
  0,
  0,
  0,
  set_mach_modtime_by_id,
};

static char *gcld_fields[] = {
  "cluster", "serv_label",
  "cluster", "serv_label", "serv_cluster",
};

static char *acld_fields[] = {
  "cluster", "serv_label", "serv_cluster",
};

static struct valobj acld_valobj[] =
{
  {V_ID, 0, CLUSTERS_TABLE, "name", "clu_id", MR_CLUSTER},
  {V_CHAR, 1, SVC_TABLE, "serv_label"},
  {V_CHAR, 2, SVC_TABLE, "serv_cluster"}
};

static struct validate acld_validate =
{
  acld_valobj,
  3,
  "clu_id",
  "clu_id = %d AND serv_label = '%s' AND serv_cluster = '%s'",
  3,
  0,
  0,
  0,
  set_cluster_modtime_by_id,
};

static struct validate dcld_validate =
{
  VOclu0,
  1,
  "clu_id",
  "clu_id = %d AND serv_label = '%s' AND serv_cluster = '%s'",
  3,
  0,
  0,
  0,
  set_cluster_modtime_by_id,
};

static char *glin_fields[] = {
  "name",
  "name", "active", "publicflg", "hidden", "maillist", "grouplist", "gid",
  "ace_type", "ace_name", "description", "modtime", "modby", "modwith",
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
  followup_glin,
};

static char *alis_fields[] = {
  "name", "active", "publicflg", "hidden", "maillist", "grouplist", "gid",
  "ace_type", "ace_name", "description",
};

static struct valobj alis_valobj[] = {
  {V_CHAR, 0, LIST_TABLE, "name"},
  {V_NUM, 1},
  {V_NUM, 2},
  {V_NUM, 3},
  {V_NUM, 4},
  {V_NUM, 5},
  {V_NUM, 6},
  {V_TYPE, 7, 0, "ace_type", 0, MR_ACE},
  {V_TYPEDATA, 8, 0, 0, "list_id", MR_ACE},
  {V_LEN, 9, LIST_TABLE, "description"},
};

static struct validate alis_validate = {
  alis_valobj,
  10,
  "name",
  "name = '%s'",
  1,
  "list_id",
  0,
  setup_alis,
  set_modtime,
};

static char *ulis_fields[] = {
  "name",
  "newname", "active", "publicflg", "hidden", "maillist", "grouplist", "gid",
  "ace_type", "ace_name", "description",
};

static struct valobj ulis_valobj[] = {
  {V_ID, 0, LIST_TABLE, "name", "list_id", MR_LIST},
  {V_RENAME, 1, LIST_TABLE, "name", "list_id", MR_NOT_UNIQUE},
  {V_NUM, 2},
  {V_NUM, 3},
  {V_NUM, 4},
  {V_NUM, 5},
  {V_NUM, 6},
  {V_NUM, 7},
  {V_TYPE, 8, 0, "ace_type", 0, MR_ACE},
  {V_TYPEDATA, 9, 0, 0, "list_id", MR_ACE},
  {V_LEN, 10, LIST_TABLE, "description"},
};

static struct validate ulis_validate = {
  ulis_valobj,
  11,
  "name",
  "list_id = %d",
  1,
  "list_id",
  access_list,
  setup_alis,
  set_modtime_by_id,
};

static char *dlis_fields[] = {
  "name",
};

static struct validate dlis_validate = {
  VOlist0,
  1,
  "name",
  "list_id = %d",
  1,
  0,
  access_list,
  setup_dlis,
  0,
};

static char *amtl_fields[] = {
  "list_name", "member_type", "member_name",
};

static char *atml_fields[] = {
  "list_name", "member_type", "member_name", "tag",
};

static struct valobj amtl_valobj[] = {
  {V_ID, 0, LIST_TABLE, "name", "list_id", MR_LIST},
  {V_TYPE, 1, 0, "member", 0, MR_TYPE},
  {V_TYPEDATA, 2, 0, 0, 0, MR_NO_MATCH},
  {V_ID, 3, STRINGS_TABLE, "string", "string_id", MR_NO_MATCH},
};

static struct validate amtl_validate =
{
  amtl_valobj,
  3,
  0,
  0,
  0,
  0,
  access_list,
  0,
  add_member_to_list,
};

static struct validate atml_validate =
{
  amtl_valobj,
  4,
  0,
  0,
  0,
  0,
  access_list,
  0,
  add_member_to_list,
};

static struct validate tmol_validate =
{
  amtl_valobj,
  4,
  "tag",
  "list_id = %d AND member_type = '%s' AND member_id = %d",
  3,
  0,
  access_list,
  0,
  0,
};

static struct validate dmfl_validate =
{
  amtl_valobj,
  3,
  0,
  0,
  0,
  0,
  access_list,
  0,
  delete_member_from_list,
};

static char *gaus_fields[] = {
  "ace_type", "ace_name",
  "use_type", "use_name",
};

static struct valobj gaus_valobj[] = {
  {V_TYPE, 0, 0, "gaus", 0, MR_TYPE},
  {V_TYPEDATA, 1, 0, 0, 0, MR_NO_MATCH},
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
  "active", "publicflg", "hidden", "maillist", "grouplist",
  "list",
};

static struct valobj qgli_valobj[] = {
  {V_TYPE, 0, 0, "boolean", 0, MR_TYPE},
  {V_TYPE, 1, 0, "boolean", 0, MR_TYPE},
  {V_TYPE, 2, 0, "boolean", 0, MR_TYPE},
  {V_TYPE, 3, 0, "boolean", 0, MR_TYPE},
  {V_TYPE, 4, 0, "boolean", 0, MR_TYPE},
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

static char *gtml_fields[] = {
  "list_name",
  "member_type", "member_name", "tag",
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
  "list_name", "active", "publicflg", "hidden", "maillist", "grouplist",
};

static struct valobj glom_valobj[] = {
  {V_TYPE, 0, 0, "rmember", 0, MR_TYPE},
  {V_TYPEDATA, 1, 0, 0, 0, MR_NO_MATCH},
};

static struct validate glom_validate = {
  glom_valobj,
  2,
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

static char *gsin_fields[] = {
  "service",
  "service", "update_int", "target_file", "script", "dfgen", "dfcheck",
  "type", "enable", "inprogress", "harderror", "errmsg",
  "ace_type", "ace_name", "modtime", "modby", "modwith",
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
  {V_TYPE, 0, 0, "boolean", 0, MR_TYPE},
  {V_TYPE, 1, 0, "boolean", 0, MR_TYPE},
  {V_TYPE, 2, 0, "boolean", 0, MR_TYPE},
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
  "service", "update_int", "target_file", "script", "type", "enable",
  "ace_type", "ace_name",
};

static struct valobj asin_valobj[] = {
  {V_CHAR, 0, SERVERS_TABLE, "name"},
  {V_NUM, 1},
  {V_LEN, 2, SERVERS_TABLE, "target_file"},
  {V_LEN, 3, SERVERS_TABLE, "script"},
  {V_TYPE, 4, 0, "service", 0, MR_TYPE},
  {V_NUM, 5},
  {V_TYPE, 6, 0, "ace_type", 0, MR_ACE},
  {V_TYPEDATA, 7, 0, 0, 0, MR_ACE},
};

static struct validate asin_validate =	/* for asin, usin */
{
  asin_valobj,
  8,
  "name",
  "name = UPPER('%s')",
  1,
  0,
  0,
  0,
  set_uppercase_modtime,
};

static struct validate rsve_validate = {
  asin_valobj,
  1,
  "name",
  "name = UPPER('%s')",
  1,
  0,
  access_service,
  0,
  set_uppercase_modtime,
};

static char *ssif_fields[] = {
  "service", "dfgen", "dfcheck", "inprogress", "harderror", "errmsg",
};

static struct valobj ssif_valobj[] = {
  {V_NAME, 0, SERVERS_TABLE, "name", 0, MR_SERVICE},
  {V_NUM, 1},
  {V_NUM, 2},
  {V_NUM, 3},
  {V_NUM, 4},
  {V_LEN, 5, SERVERS_TABLE, "errmsg"},
};

static struct validate ssif_validate = {
  ssif_valobj,
  6,
  "name",
  "name = UPPER('%s')",
  1,
  0,
  0,
  0,
  0,
};

static char *dsin_fields[] = {
  "service",
};

static struct validate dsin_validate = {
  asin_valobj,
  1,
  "name",
  "name = UPPER('%s')",
  1,
  0,
  0,
  setup_dsin,
  0,
};

static char *gshi_fields[] = {
  "service", "machine",
  "service", "machine", "enable", "override", "success", "inprogress",
  "hosterror", "hosterrormsg", "ltt", "lts", "value1", "value2",
  "value3", "modtime", "modby", "modwith",
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
  "service", "machine",
};

static struct valobj qgsh_valobj[] = {
  {V_TYPE, 1, 0, "boolean", 0, MR_TYPE},
  {V_TYPE, 2, 0, "boolean", 0, MR_TYPE},
  {V_TYPE, 3, 0, "boolean", 0, MR_TYPE},
  {V_TYPE, 4, 0, "boolean", 0, MR_TYPE},
  {V_TYPE, 5, 0, "boolean", 0, MR_TYPE},
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
  "service", "machine", "enable", "value1", "value2", "value3",
};

static struct valobj ashi_valobj[] = {
  {V_NAME, 0, SERVERS_TABLE, "name", 0, MR_SERVICE},
  {V_ID, 1, MACHINE_TABLE, "name", "mach_id", MR_MACHINE},
  {V_NUM, 2},
  {V_NUM, 3},
  {V_NUM, 4},
  {V_LEN, 5, SERVERHOSTS_TABLE, "value3"},
};

static struct validate ashi_validate = /* ashi & ushi */
{
  ashi_valobj,
  6,
  "service",
  "service = UPPER('%s') AND mach_id = %d",
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
  "service",
  "service = UPPER('%s') AND mach_id = %d",
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
  "service",
  "service = UPPER('%s') AND mach_id = %d",
  2,
  0,
  access_service,
  0,
  trigger_dcm,
};

static char *sshi_fields[] = {
  "service", "machine", "override", "success", "inprogress",
  "hosterror", "errmsg", "ltt", "lts",
};

static struct valobj sshi_valobj[] = {
  {V_NAME, 0, SERVERS_TABLE, "name", 0, MR_SERVICE},
  {V_ID, 1, MACHINE_TABLE, "name", "mach_id", MR_MACHINE},
  {V_NUM, 2},
  {V_NUM, 3},
  {V_NUM, 4},
  {V_NUM, 5},
  {V_LEN, 6, SERVERHOSTS_TABLE, "hosterrmsg"},
  {V_NUM, 7},
  {V_NUM, 8},
};

static struct validate sshi_validate =
{
  sshi_valobj,
  9,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
};

static char *dshi_fields[] = {
  "service", "machine",
};

static struct validate dshi_validate =
{
  ashi_valobj,
  2,
  "service",
  "service = UPPER('%s') AND mach_id = %d",
  2,
  0,
  access_service,
  setup_dshi,
  0,
};

static char *gslo_fields[] = {
  "server",
  "server", "machine",
};

static char *gfsl_fields[] = {
  "label",
  "label", "type", "machine", "name", "mount", "access", "comments",
  "owner", "owners", "create", "lockertype", "modtime", "modby", "modwith",
};

static struct validate gfsl_validate = {
  0,
  0,
  0,
  0,
  0,
  0,
  access_filesys,
  0,
  followup_fix_modby,
};

static char *gfsm_fields[] = {
  "machine",
  "label", "type", "machine", "name", "mount", "access", "comments",
  "owner", "owners", "create", "lockertype", "modtime", "modby", "modwith",
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
  "machine", "partition",
  "label", "type", "machine", "name", "mount", "access", "comments",
  "owner", "owners", "create", "lockertype", "modtime", "modby", "modwith",
};

static struct validate gfsn_validate = {
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

static char *gfsp_fields[] = {
  "path",
  "label", "type", "machine", "name", "mount", "access", "comments",
  "owner", "owners", "create", "lockertype", "modtime", "modby", "modwith",
};

static char *gfsg_fields[] = {
  "list",
  "label", "type", "machine", "name", "mount", "access", "comments",
  "owner", "owners", "create", "lockertype", "modtime", "modby", "modwith",
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
  "label", "type", "machine", "name", "mount", "access", "comments",
  "owner", "owners", "create", "lockertype",
};

static struct valobj afil_valobj[] = {
  {V_CHAR, 0, FILESYS_TABLE, "label"},
  {V_TYPE, 1, 0, "filesys", 0, MR_FSTYPE},
  {V_ID, 2, MACHINE_TABLE, "name", "mach_id", MR_MACHINE},
  {V_CHAR, 3, FILESYS_TABLE, "name"},
  {V_LEN, 4, FILESYS_TABLE, "mount"},
  {V_LEN, 5, FILESYS_TABLE, "rwaccess"},
  {V_LEN, 6, FILESYS_TABLE, "comments"},
  {V_ID, 7, USERS_TABLE, "login", "users_id", MR_USER},
  {V_ID, 8, LIST_TABLE, "name", "list_id", MR_LIST},
  {V_NUM, 9},
  {V_TYPE, 10, 0, "lockertype", 0, MR_TYPE},
};

static struct validate afil_validate = {
  afil_valobj,
  11,
  "label",
  "label = '%s'",
  1,
  "filsys_id",
  0,
  setup_afil,
  set_filesys_modtime,
};

static char *ufil_fields[] = {
  "label", "newlabel", "type", "machine", "name", "mount", "access",
  "comments", "owner", "owners", "create", "lockertype",
};

static struct valobj ufil_valobj[] = {
  {V_ID, 0, FILESYS_TABLE, "label", "filsys_id", MR_FILESYS},
  {V_RENAME, 1, FILESYS_TABLE, "label", "filsys_id", MR_NOT_UNIQUE},
  {V_TYPE, 2, 0, "filesys", 0, MR_FSTYPE},
  {V_ID, 3, MACHINE_TABLE, "name", "mach_id", MR_MACHINE},
  {V_CHAR, 4, FILESYS_TABLE, "name"},
  {V_LEN, 5, FILESYS_TABLE, "mount"},
  {V_LEN, 6, FILESYS_TABLE, "rwaccess"},
  {V_LEN, 7, FILESYS_TABLE, "comments"},
  {V_ID, 8, USERS_TABLE, "login", "users_id", MR_USER},
  {V_ID, 9, LIST_TABLE, "name", "list_id", MR_LIST},
  {V_NUM, 10},
  {V_TYPE, 11, 0, "lockertype", 0, MR_TYPE},
};

static struct validate ufil_validate = {
  ufil_valobj,
  12,
  "label",
  "filsys_id = %d",
  1,
  "filsys_id",
  0,
  setup_ufil,
  set_filesys_modtime,
};

static char *dfil_fields[] = {
  "label",
};

static struct validate dfil_validate = {
  VOfilesys0,
  1,
  "label",
  "filsys_id = %d",
  1,
  0,
  0,
  setup_dfil,
  0,
};

static char *gfgm_fields[] = {
  "fsgroup", "filesys", "sortkey"
};

static struct validate gfgm_validate = {
  VOfilesys0,
  1,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
};

static struct valobj aftg_valobj[] = {
  {V_ID, 0, FILESYS_TABLE, "label", "filsys_id", MR_FILESYS},
  {V_ID, 1, FILESYS_TABLE, "label", "filsys_id", MR_FILESYS},
};

static struct validate aftg_validate = {
  aftg_valobj,
  2,
  "group_id",
  "group_id = %d and filsys_id = %d",
  2,
  0,
  0,
  0,
  0,
};

static char *ganf_fields[] = {
  "machine", "dir", "device", "status", "allocated", "size",
  "modtime", "modby", "modwith",
};

static char *gnfp_fields[] = {
  "machine", "dir",
  "machine", "dir", "device", "status", "allocated", "size",
  "modtime", "modby", "modwith",
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
  {V_ID, 0, MACHINE_TABLE, "name", "mach_id", MR_MACHINE},
  {V_CHAR, 1, NFSPHYS_TABLE, "dir"},
  {V_LEN, 2, NFSPHYS_TABLE, "device"},
  {V_NUM, 3},
  {V_NUM, 4},
  {V_NUM, 5},
};

static struct validate anfp_validate = {
  anfp_valobj,
  6,
  "dir",
  "mach_id = %d and dir = '%s'",
  2,
  "nfsphys_id",
  0,
  prefetch_value,
  set_nfsphys_modtime,
};

static struct validate unfp_validate = {
  anfp_valobj,
  6,
  "dir",
  "mach_id = %d and dir = '%s'",
  2,
  "nfsphys_id",
  0,
  0,
  set_nfsphys_modtime,
};

static char *ajnf_fields[] = {
  "machine", "dir", "adjustment",
};

static struct valobj ajnf_valobj[] = {
  {V_ID, 0, MACHINE_TABLE, NAME, "mach_id", MR_MACHINE},
  {V_CHAR, 1, NFSPHYS_TABLE, "dir"},
  {V_NUM, 2},
};

static struct validate ajnf_validate = {
  ajnf_valobj,
  3,
  "dir",
  "mach_id = %d and dir = '%s'",
  2,
  "nfsphys_id",
  0,
  0,
  set_nfsphys_modtime,
};

static char *dnfp_fields[] = {
  "machine", "dir",
};

static struct validate dnfp_validate = {
  anfp_valobj,
  2,
  "dir",
  "mach_id = %d and dir = '%s'",
  2,
  "nfsphys_id",
  0,
  setup_dnfp,
  set_nfsphys_modtime,
};

static char *gqot_fields[] = {
  "filesys", "type", "name",
  "filesys", "type", "name", "quota", "dir", "machine",
  "modtime", "modby", "modwith",
};

static struct valobj gqot_valobj[] = {
  {V_TYPE, 1, 0, "quota_type", 0, MR_TYPE},
  {V_TYPEDATA, 2, 0, 0, 0, MR_ACE},
};

static struct validate gqot_validate = {
  gqot_valobj,
  2,
  0,
  0,
  0,
  0,
  0,
  0,
  followup_gqot,
};

static char *gqbf_fields[] = {
  "filesys",
  "filesys", "type", "name", "quota", "dir", "machine",
  "modtime", "modby", "modwith",
};

static struct validate gqbf_validate = {
  0,
  0,
  0,
  0,
  0,
  0,
  access_filesys,
  0,
  followup_gqot,
};

static char *aqot_fields[] = {
  "filesys", "type", "name", "quota",
};

static struct valobj aqot_valobj[] = {
  {V_ID, 0, FILESYS_TABLE, "label", "filsys_id", MR_FILESYS},
  {V_TYPE, 1, 0, "quota_type", 0, MR_TYPE},
  {V_TYPEDATA, 2, 0, 0, 0, MR_ACE},
  {V_NUM, 3},
};

static struct validate aqot_validate = {
  aqot_valobj,
  4,
  "filsys_id",
  "filsys_id = %d and type = '%s' and entity_id = %d",
  3,
  0,
  0,
  prefetch_filesys,
  followup_aqot,
};

static struct validate uqot_validate = {
  aqot_valobj,
  4,
  "filsys_id",
  "filsys_id = %d AND type = '%s' AND entity_id = %d",
  3,
  0,
  0,
  setup_dqot,
  followup_aqot,
};

static struct validate dqot_validate = {
  aqot_valobj,
  3,
  "filsys_id",
  "filsys_id = %d AND type = '%s' AND entity_id = %d",
  3,
  0,
  0,
  setup_dqot,
  followup_dqot,
};

static char *gnfq_fields[] = {
  "filesys", "login",
  "filesys", "login", "quota", "dir", "machine",
  "modtime", "modby", "modwith",
};

static struct validate gnfq_validate = {
  0,
  0,
  0,
  0,
  0,
  0,
  access_filesys,
  0,
  followup_gqot,
};

static char *gnqp_fields[] = {
  "machine", "dir",
  "filesys", "login", "quota", "dir", "machine",
  "modtime", "modby", "modwith",
};

static char *anfq_fields[] = {
  "filesys", "login", "quota",
};

static struct valobj anfq_valobj[] = {
  {V_ID, 0, FILESYS_TABLE, "label", "filsys_id", MR_FILESYS},
  {V_ID, 1, USERS_TABLE, "login", "users_id", MR_USER},
  {V_NUM, 2},
};

static struct validate anfq_validate = {
  anfq_valobj,
  3,
  "filsys_id",
  "filsys_id = %d AND type = 'USER' AND entity_id = %d",
  2,
  0, /* object ? */
  0,
  prefetch_filesys,
  followup_aqot,
};

static struct validate unfq_validate = {
  anfq_valobj,
  3,
  "filsys_id",
  "filsys_id = %d AND type = 'USER' AND entity_id = %d",
  2,
  0,
  0,
  setup_dqot,
  followup_aqot,
};

static struct validate dnfq_validate = {
  anfq_valobj,
  3,
  "filsys_id",
  "filsys_id = %d AND entity_id = %d",
  2,
  0,
  0,
  setup_dqot,
  followup_dqot,
};

static char *gzcl_fields[] = {
  "class",
  "class", "xmt_type", "xmt_name", "sub_type", "sub_name",
  "iws_type", "iws_name", "iui_type", "iui_name",
  "modtime", "modby", "modwith",
};

static struct validate gzcl_validate = {
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  followup_gzcl,
};

static char *azcl_fields[] = {
  "class", "xmt_type", "xmt_name", "sub_type", "sub_name",
  "iws_type", "iws_name", "iui_type", "iui_name",
};

static struct valobj azcl_valobj[] = {
  {V_CHAR, 0, ZEPHYR_TABLE, "class"},
  {V_TYPE, 1, 0, "zace_type", 0, MR_ACE},
  {V_TYPEDATA, 2, 0, 0, "list_id", MR_ACE},
  {V_TYPE, 3, 0, "zace_type", 0, MR_ACE},
  {V_TYPEDATA, 4, 0, 0, "list_id", MR_ACE},
  {V_TYPE, 5, 0, "zace_type", 0, MR_ACE},
  {V_TYPEDATA, 6, 0, 0, "list_id", MR_ACE},
  {V_TYPE, 7, 0, "zace_type", 0, MR_ACE},
  {V_TYPEDATA, 8, 0, 0, "list_id", MR_ACE},
};

static struct validate azcl_validate = {
  azcl_valobj,
  9,
  "class",
  "class = '%s'",
  1,
  0,
  0,
  0,
  set_zephyr_modtime,
};

static char *uzcl_fields[] = {
  "class", "newclass", "xmt_type", "xmt_name", "sub_type", "sub_name",
  "iws_type", "iws_name", "iui_type", "iui_name",
};

static struct valobj uzcl_valobj[] = {
  {V_NAME, 0, ZEPHYR_TABLE, "class", 0, MR_BAD_CLASS},
  {V_RENAME, 1, ZEPHYR_TABLE, "class", 0, MR_NOT_UNIQUE},
  {V_TYPE, 2, 0, "zace_type", 0, MR_ACE},
  {V_TYPEDATA, 3, 0, 0, "list_id", MR_ACE},
  {V_TYPE, 4, 0, "zace_type", 0, MR_ACE},
  {V_TYPEDATA, 5, 0, 0, "list_id", MR_ACE},
  {V_TYPE, 6, 0, "zace_type", 0, MR_ACE},
  {V_TYPEDATA, 7, 0, 0, "list_id", MR_ACE},
  {V_TYPE, 8, 0, "zace_type", 0, MR_ACE},
  {V_TYPEDATA, 9, 0, 0, "list_id", MR_ACE},
};

static struct validate uzcl_validate = {
  uzcl_valobj,
  10,
  "class",
  "class = '%s'",
  1,
  0,
  0,
  0,
  set_zephyr_modtime,
};

static struct validate dzcl_validate = {
  0,
  0,
  "class",
  "class = '%s'",
  1,
  0,
  0,
  0,
  0,
};

static char *gsha_fields[] = {
  "machine",
  "machine", "ace_type", "ace_name", "modtime", "modby", "modwith",
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
  "machine", "ace_type", "ace_name",
};

static struct valobj asha_valobj[] = {
  {V_ID, 0, MACHINE_TABLE, "name", "mach_id", MR_MACHINE},
  {V_TYPE, 1, 0, "ace_type", 0, MR_ACE},
  {V_TYPEDATA, 2, 0, 0, 0, MR_ACE},
};

static struct validate asha_validate =
{
  asha_valobj,
  3,
  "mach_id",
  "mach_id = %d",
  1,
  "mach_id",
  0,
  0,
  set_modtime_by_id,
};

static char *gacl_fields[] = {
  "machine", "target",
  "machine", "target", "kind", "list",
};

static char *aacl_fields[] = {
  "machine", "target", "kind", "list",
};

static char *dacl_fields[] = {
  "machine", "target",
};

static struct valobj aacl_valobj[] = {
  {V_ID, 0, MACHINE_TABLE, "name", "mach_id", MR_MACHINE},
  {V_CHAR, 1, ACL_TABLE, "target"},
  {V_TYPE, 2, 0, "acl_kind", 0, MR_TYPE},
  {V_ID, 3, LIST_TABLE, "name", "list_id", MR_LIST},
};

static struct validate gacl_validate =
{
  aacl_valobj,
  1,
  NULL,
  NULL,
  0,
  "mach_id",
  /* access_acl */ 0,
  0,
  0,
};

static struct validate aacl_validate =
{
  aacl_valobj,
  4,
  "mach_id",
  "mach_id = %d AND target = '%s'",
  2,
  "mach_id",
  /* access_acl */ 0,
  0,
  0,
};

static char *gsvc_fields[] = {
  "service",
  "service", "protocol", "port", "description", "modtime", "modby", "modwith",
};

static char *asvc_fields[] = {
  "service", "protocol", "port", "description",
};

static struct valobj asvc_valobj[] = {
  {V_CHAR, 0, SERVICES_TABLE, "name"},
  {V_CHAR, 1, SERVICES_TABLE, "protocol"},
  {V_NUM, 2},
  {V_CHAR, 3, SERVICES_TABLE, "description"},
};

static struct validate asvc_validate = {
  asvc_valobj,
  4,
  "name",
  "name = '%s'",
  1,
  0,
  0,
  0,
  set_modtime,
};

static char *gprn_fields[] = {
  "name",
  "name", "type", "hwtype", "duplexname", "hostname",
  "loghost", "rm", "rp", "rq", "ka", "pc", "ac", "lpc_acl",
  "banner", "location", "contact", "modtime", "modby", "modwith"
};

static char *gpbd_fields[] = {
  "duplexname",
  "name", "type", "hwtype", "duplexname", "hostname",
  "loghost", "rm", "rp", "rq", "ka", "pc", "ac", "lpc_acl",
  "banner", "location", "contact", "modtime", "modby", "modwith"
};

static char *gpbe_fields[] = {
  "hwaddr",
  "name", "type", "hwtype", "duplexname", "hostname",
  "loghost", "rm", "rp", "rq", "ka", "pc", "ac", "lpc_acl",
  "banner", "location", "contact", "modtime", "modby", "modwith"
};

static char *gpbh_fields[] = {
  "hostname",
  "name", "type", "hwtype", "duplexname", "hostname",
  "loghost", "rm", "rp", "rq", "ka", "pc", "ac", "lpc_acl",
  "banner", "location", "contact", "modtime", "modby", "modwith"
};

static char *gpbr_fields[] = {
  "rm",
  "name", "type", "hwtype", "duplexname", "hostname",
  "loghost", "rm", "rp", "rq", "ka", "pc", "ac", "lpc_acl",
  "banner", "location", "contact", "modtime", "modby", "modwith"
};

static char *gpbl_fields[] = {
  "location",
  "name", "type", "hwtype", "duplexname", "hostname",
  "loghost", "rm", "rp", "rq", "ka", "pc", "ac", "lpc_acl",
  "banner", "location", "contact", "modtime", "modby", "modwith"
};

static char *gpbc_fields[] = {
  "contact",
  "name", "type", "hwtype", "duplexname", "hostname",
  "loghost", "rm", "rp", "rq", "ka", "pc", "ac", "lpc_acl",
  "banner", "location", "contact", "modtime", "modby", "modwith"
};

static char *aprn_fields[] = {
  "printer", "type", "hwtype", "duplexname", "hostname",
  "loghost", "rm", "rp", "rq", "ka", "pc", "ac", "lpc_acl",
  "banner", "location", "contact",
};

static struct valobj aprn_valobj[] = {
  {V_CHAR, 0, PRINTERS_TABLE, "name"},
  {V_TYPE, 1, 0, "printertype", 0, MR_TYPE},
  {V_TYPE, 2, 0, "printerhwtype", 0, MR_TYPE},
  {V_CHAR, 3, PRINTERS_TABLE, "duplexname"},
  {V_ID, 4, MACHINE_TABLE, "name", "mach_id", MR_MACHINE},
  {V_ID, 5, MACHINE_TABLE, "name", "mach_id", MR_MACHINE},
  {V_ID, 6, MACHINE_TABLE, "name", "mach_id", MR_MACHINE},
  {V_CHAR, 7, PRINTERS_TABLE, "rp"},
  {V_ID, 8, MACHINE_TABLE, "name", "mach_id", MR_MACHINE},
  {V_NUM, 9},
  {V_NUM, 10},
  {V_ID, 11, LIST_TABLE, "name", "list_id", MR_LIST},
  {V_ID, 12, LIST_TABLE, "name", "list_id", MR_LIST},
  {V_NUM, 13},
  {V_CHAR, 14, PRINTERS_TABLE, "location"},
  {V_CHAR, 15, PRINTERS_TABLE, "contact"},
};

static struct validate aprn_validate = {
  aprn_valobj,
  16,
  "name",
  "name = '%s'",
  1,
  0,
  access_printer,
  setup_aprn,
  set_modtime,
};

static char *uprn_fields[] = {
  "printer",
  "printer", "type", "hwtype", "duplexname", "hostname",
  "loghost", "rm", "rp", "rq", "ka", "pc", "ac", "lpc_acl",
  "banner", "location", "contact",
};

static struct valobj uprn_valobj[] = {
  {V_CHAR, 0, PRINTERS_TABLE, "name"},
  {V_CHAR, 1, PRINTERS_TABLE, "name"},
  {V_TYPE, 2, 0, "printertype", 0, MR_TYPE},
  {V_TYPE, 3, 0, "printerhwtype", 0, MR_TYPE},
  {V_CHAR, 4, PRINTERS_TABLE, "duplexname"},
  {V_ID, 5, MACHINE_TABLE, "name", "mach_id", MR_MACHINE},
  {V_ID, 6, MACHINE_TABLE, "name", "mach_id", MR_MACHINE},
  {V_ID, 7, MACHINE_TABLE, "name", "mach_id", MR_MACHINE},
  {V_CHAR, 8, PRINTERS_TABLE, "rp"},
  {V_ID, 9, MACHINE_TABLE, "name", "mach_id", MR_MACHINE},
  {V_NUM, 10},
  {V_NUM, 11},
  {V_ID, 12, LIST_TABLE, "name", "list_id", MR_LIST},
  {V_ID, 13, LIST_TABLE, "name", "list_id", MR_LIST},
  {V_NUM, 14},
  {V_CHAR, 15, PRINTERS_TABLE, "location"},
  {V_CHAR, 16, PRINTERS_TABLE, "contact"},
};

static struct validate uprn_validate = {
  uprn_valobj,
  16,
  "name",
  "name = '%s'",
  1,
  0,
  access_printer,
  setup_aprn,
  set_modtime,
};

static struct validate dprn_validate = {
  0,
  0,
  "name",
  "name = '%s'",
  1,
  0,
  access_printer,
  0,
  0,
};

static char *gpsv_fields[] = {
  "host",
  "host", "kind", "printer_types", "owner_type", "owner_name",
  "lpc_acl", "modtime", "modby", "modwith"
};

static struct valobj gpsv_valobj[] = {
  {V_ID, 0, MACHINE_TABLE, "name", "mach_id", MR_MACHINE},
};

static struct validate gpsv_validate = {
  gpsv_valobj,
  1,
  0,
  0,
  0,
  0,
  0,
  0,
  followup_gpsv,
};

static char *apsv_fields[] = {
  "host", "kind", "printer_types", "owner_type", "owner_name", "lpc_acl"
};

static struct valobj apsv_valobj[] = {
  {V_ID, 0, MACHINE_TABLE, "name", "mach_id", MR_MACHINE},
  {V_TYPE, 1, 0, "lpd_kind", 0, MR_TYPE},
  {V_ID, 2, STRINGS_TABLE, "string", "string_id", MR_STRING},
  {V_TYPE, 3, 0, "ace_type", 0, MR_ACE},
  {V_TYPEDATA, 4, 0, 0, 0, MR_ACE},
  {V_ID, 5, LIST_TABLE, "name", "list_id", MR_LIST},
};

static struct validate apsv_validate = {
  apsv_valobj,
  6,
  "mach_id",
  "mach_id = %d",
  1,
  "mach_id",
  0,
  0,
  set_modtime_by_id,
};

static char *dpsv_fields[] = {
  "host",
};

static struct validate dpsv_validate = {
  gpsv_valobj,
  1,
  "mach_id",
  "mach_id = %d",
  1,
  0,
  0,
  setup_dpsv,
  0,
};  

static char *gali_fields[] = {
  "name", "type", "trans",
  "name", "type", "trans",
};

static char *aali_fields[] = {
  "name", "type", "trans",
};

static struct valobj aali_valobj[] = {
  {V_CHAR, 0, ALIAS_TABLE, "name"},
  {V_TYPE, 1, 0, "alias", 0, MR_TYPE},
  {V_CHAR, 2, ALIAS_TABLE, "trans"},
};

static struct validate aali_validate = {
  aali_valobj,
  3,
  "trans",
  "name = '%s' and type = '%s' and trans = '%s'",
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
  "name = '%s' and type = '%s' and trans = '%s'",
  3,
  0,
  0,
  0,
  0,
};

static char *gval_fields[] = {
  "name", "value",
};

static struct valobj gval_valobj[] = {
  {V_NAME, 0, NUMVALUES_TABLE, "name", 0, MR_NO_MATCH},
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
  "name", "value",
};

static struct valobj aval_valobj[] = {
  {V_CHAR, 0, NUMVALUES_TABLE, "name"},
  {V_NUM, 1},
};

static struct validate aval_validate =	/* for aval, uval, and dval */
{
  aval_valobj,
  2,
  "name",
  "name = '%s'",
  1,
  0,
  0,
  0,
  0,
};

static char *dval_fields[] = {
  "name",
};

static char *gats_fields[] = {
  "table_name", "appends", "updates", "deletes",
  "modtime", "modby", "modwith",
};

static char *_sdl_fields[] = {
  "level",
};

static struct validate _sdl_validate =
{
  VOnum0,
  1,
  NULL,
  NULL,
  0,
  0,
  0,
  0,
  _sdl_followup,
};



/* Generalized Query Definitions */

/* Multiple versions of the same query MUST be listed in ascending
 * order.
 */

/* Note: For any query which uses prefetch_value, the vcnt should be
 * one less than the number of %-format specifiers in the tlist.
 */

struct query Queries[] = {
  {
    /* Q_GALO - GET_ALL_LOGINS */
    "get_all_logins",
    "galo",
    2,
    RETRIEVE,
    "u",
    USERS_TABLE,
    "u.login, u.unix_uid, u.shell, u.last, u.first, u.middle FROM users u",
    galo_fields,
    6,
    "u.users_id != 0",
    0,
    "u.login",
    0,
  },

  {
    /* Q_GAAL - GET_ALL_ACTIVE_LOGINS */
    "get_all_active_logins",
    "gaal",
    2,
    RETRIEVE,
    "u",
    USERS_TABLE,
    "u.login, u.unix_uid, u.shell, u.last, u.first, u.middle FROM users u",
    galo_fields,
    6,
    "u.status = 1",
    0,
    "u.login",
    0,
  },

  {
    /* Q_GUAL - GET_USER_ACCOUNT_BY_LOGIN */
    "get_user_account_by_login",
    "gual",
    2,
    RETRIEVE,
    "u",
    USERS_TABLE,
    "u.login, u.unix_uid, u.shell, u.last, u.first, u.middle, u.status, u.clearid, u.type, str.string, u.signature, u.secure, TO_CHAR(u.modtime, 'DD-mon-YYYY HH24:MI:SS'), u.modby, u.modwith FROM users u, strings str",
    gual_fields,
    15,
    "u.login LIKE '%s' AND u.users_id != 0 AND u.comments = str.string_id",
    1,
    "u.login",
    &gubl_validate,
  },

  {
    /* Q_GUAU - GET_USER_ACCOUNT_BY_UID */
    "get_user_account_by_uid",
    "guau",
    2,
    RETRIEVE,
    "u",
    USERS_TABLE,
    "u.login, u.unix_uid, u.shell, u.last, u.first, u.middle, u.status, u.clearid, u.type, str.string, u.signature, u.secure, TO_CHAR(u.modtime, 'DD-mon-YYYY HH24:MI:SS'), u.modby, u.modwith FROM users u, strings str",
    guau_fields,
    15,
    "u.unix_uid = %s AND u.users_id != 0 AND u.comments = str.string_id",
    1,
    "u.login",
    &gubu_validate,
  },

  {
    /* Q_GUAN - GET_USER_ACCOUNT_BY_NAME */
    "get_user_account_by_name",
    "guan",
    2,
    RETRIEVE,
    "u",
    USERS_TABLE,
    "u.login, u.unix_uid, u.shell, u.last, u.first, u.middle, u.status, u.clearid, u.type, str.string, u.signature, u.secure, TO_CHAR(u.modtime, 'DD-mon-YYYY HH24:MI:SS'), u.modby, u.modwith FROM users u, strings str",
    guan_fields,
    15,
    "u.first LIKE '%s' AND u.last LIKE '%s' AND u.users_id != 0 and u.comments = str.string_id",
    2,
    "u.login",
    &guan_validate,
  },

  {
    /* Q_GUAC - GET_USER_ACCOUNT_BY_CLASS */
    "get_user_account_by_class",
    "guac",
    2,
    RETRIEVE,
    "u",
    USERS_TABLE,
    "u.login, u.unix_uid, u.shell, u.last, u.first, u.middle, u.status, u.clearid, u.type, str.string, u.signature, u.secure, TO_CHAR(u.modtime, 'DD-mon-YYYY HH24:MI:SS'), u.modby, u.modwith FROM users u, strings str",
    guac_fields,
    15,
    "u.type = UPPER('%s') AND u.users_id != 0 AND u.comments = str.string_id",
    1,
    "u.login",
    &VDfix_modby,
  },

  {
    /* Q_GUAM - GET_USER_ACCOUNT_BY_MITID */
    "get_user_account_by_id",
    "guai",
    2,
    RETRIEVE,
    "u",
    USERS_TABLE,
    "u.login, u.unix_uid, u.shell, u.last, u.first, u.middle, u.status, u.clearid, u.type, str.string, u.signature, u.secure, TO_CHAR(u.modtime, 'DD-mon-YYYY HH24:MI:SS'), u.modby, u.modwith FROM users u, strings str",
    guam_fields,
    15,
    "u.clearid LIKE '%s' AND u.users_id != 0 AND u.comments = str.string_id",
    1,
    "u.login",
    &VDfix_modby,
  },

  {
    /* Q_GUBL - GET_USER_BY_LOGIN */
    "get_user_by_login",
    "gubl",
    2,
    RETRIEVE,
    "u",
    USERS_TABLE,
    "u.login, u.unix_uid, u.shell, u.last, u.first, u.middle, u.status, u.clearid, u.type, TO_CHAR(u.modtime, 'DD-mon-YYYY HH24:MI:SS'), u.modby, u.modwith FROM users u",
    gubl_fields,
    12,
    "u.login LIKE '%s' AND u.users_id != 0",
    1,
    "u.login",
    &gubl_validate,
  },

  {
    /* Q_GUBU - GET_USER_BY_UID */
    "get_user_by_uid",
    "gubu",
    2,
    RETRIEVE,
    "u",
    USERS_TABLE,
    "u.login, u.unix_uid, u.shell, u.last, u.first, u.middle, u.status, u.clearid, u.type, TO_CHAR(u.modtime, 'DD-mon-YYYY HH24:MI:SS'), u.modby, u.modwith FROM users u",
    gubu_fields,
    12,
    "u.unix_uid = %s AND u.users_id != 0",
    1,
    "u.login",
    &gubu_validate,
  },

  {
    /* Q_GUBN - GET_USER_BY_NAME */
    "get_user_by_name",
    "gubn",
    2,
    RETRIEVE,
    "u",
    USERS_TABLE,
    "u.login, u.unix_uid, u.shell, u.last, u.first, u.middle, u.status, u.clearid, u.type, TO_CHAR(u.modtime, 'DD-mon-YYYY HH24:MI:SS'), u.modby, u.modwith FROM users u",
    gubn_fields,
    12,
    "u.first LIKE '%s' AND u.last LIKE '%s' AND u.users_id != 0",
    2,
    "u.login",
    &gubn_validate,
  },

  {
    /* Q_GUBC - GET_USER_BY_CLASS */
    "get_user_by_class",
    "gubc",
    2,
    RETRIEVE,
    "u",
    USERS_TABLE,
    "u.login, u.unix_uid, u.shell, u.last, u.first, u.middle, u.status, u.clearid, u.type, TO_CHAR(u.modtime, 'DD-mon-YYYY HH24:MI:SS'), u.modby, u.modwith FROM users u",
    gubc_fields,
    12,
    "u.type = UPPER('%s') AND u.users_id != 0",
    1,
    "u.login",
    &VDfix_modby,
  },

  {
    /* Q_GUBM - GET_USER_BY_MITID */
    "get_user_by_mitid",
    "gubm",
    2,
    RETRIEVE,
    "u",
    USERS_TABLE,
    "u.login, u.unix_uid, u.shell, u.last, u.first, u.middle, u.status, u.clearid, u.type, TO_CHAR(u.modtime, 'DD-mon-YYYY HH24:MI:SS'), u.modby, u.modwith FROM users u",
    gubm_fields,
    12,
    "u.clearid LIKE '%s' AND u.users_id != 0",
    1,
    "u.login",
    &VDfix_modby,
  },

  {
    /* Q_AUAC - ADD_USER_ACCOUNT */  /* uses prefetch_value() for users_id */
    "add_user_account",
    "auac",
    2,
    APPEND,
    "u",
    USERS_TABLE,
    "INTO users (login, unix_uid, shell, last, first, middle, status, clearid, type, comments, signature, secure, users_id) VALUES ('%s', %s, '%s', NVL('%s', CHR(0)), NVL('%s', CHR(0)), NVL('%s', CHR(0)), %s, NVL('%s', CHR(0)), '%s', %d, LENGTH(NVL('%s', CHR(0))), %s, %s)", /* followup_ausr fixes signature field */
    auac_fields,
    12,
    NULL,
    0,
    NULL,
    &auac_validate,
  },

  {
    /* Q_AUSR - ADD_USER */  /* uses prefetch_value() for users_id */
    "add_user",
    "ausr",
    2,
    APPEND,
    "u",
    USERS_TABLE,
    "INTO users (login, unix_uid, shell, last, first, middle, status, clearid, type, comments, signature, secure, users_id) VALUES ('%s', %s, '%s', NVL('%s', CHR(0)), NVL('%s', CHR(0)), NVL('%s', CHR(0)), %s, NVL('%s', CHR(0)), '%s', 0, CHR(0), 0, %s)",
    auac_fields,
    9,
    0,
    0,
    NULL,
    &ausr_validate,
  },

  {
    /* Q_RUSR - REGISTER_USER */
    "register_user",
    "rusr",
    2,
    APPEND,
    0,
    0,
    0,
    rusr_fields,
    3,
    0,
    0,
    NULL,
    &rusr_validate,
  },

  {
    /* Q_UUAC - UPDATE_USER_ACCOUNT */
    "update_user_account",
    "uuac",
    2,
    UPDATE,
    "u",
    USERS_TABLE,
    "users SET login = '%s', unix_uid = %s, shell = '%s', last = NVL('%s', CHR(0)), first = NVL('%s', CHR(0)), middle = NVL('%s', CHR(0)), status = %s, clearid = NVL('%s', CHR(0)), type = '%s', comments = %d, signature = LENGTH(NVL('%s', CHR(0))), secure = %s", /* followup_uuac fixes signature */
    uuac_fields,
    12,
    "users_id = %d",
    1,
    NULL,
    &uuac_validate,
  },

  {
    /* Q_UUSR - UPDATE_USER */
    "update_user",
    "uusr",
    2,
    UPDATE,
    "u",
    USERS_TABLE,
    "users SET login = '%s', unix_uid = %s, shell = '%s', last = NVL('%s', CHR(0)), first = NVL('%s', CHR(0)), middle = NVL('%s', CHR(0)), status = %s, clearid = NVL('%s', CHR(0)),  type = '%s'",
    uuac_fields,
    9,
    "users_id = %d",
    1,
    NULL,
    &uusr_validate,
  },

  {
    /* Q_UUSH - UPDATE_USER_SHELL */
    "update_user_shell",
    "uush",
    2,
    UPDATE,
    "u",
    USERS_TABLE,
    "users SET shell = '%s'",
    uush_fields,
    1,
    "users_id = %d",
    1,
    NULL,
    &uush_validate,
  },

  {
    /* Q_UUST - UPDATE_USER_STATUS */
    "update_user_status",
    "uust",
    2,
    UPDATE,
    "u",
    USERS_TABLE,
    "users SET status = %s",
    uust_fields,
    1,
    "users_id = %d",
    1,
    NULL,
    &uust_validate,
  },

  {
    /* Q_UUSS - UPDATE_USER_SECURITY_STATUS */
    "update_user_security_status",
    "uuss",
    2,
    UPDATE,
    "u",
    USERS_TABLE,
    "users SET secure = %s",
    uuss_fields,
    1,
    "users_id = %d",
    1,
    NULL,
    &uust_validate,
  },

  {
    /* Q_DUSR - DELETE_USER */
    "delete_user",
    "dusr",
    2,
    DELETE,
    "u",
    USERS_TABLE,
    NULL,
    dusr_fields,
    0,
    "users_id = %d",
    1,
    NULL,
    &dusr_validate,
  },

  {
    /* Q_AURV - ADD_USER_RESERVATION */
    "add_user_reservation",
    "aurv",
    2,
    UPDATE,
    0,
    USERS_TABLE,
    0,
    aurv_fields,
    2,
    0,
    0,
    NULL,
    &aurv_validate,
  },

  {
    /* Q_GURV - GET_USER_RESERVATIONS */
    "get_user_reservations",
    "gurv",
    2,
    RETRIEVE,
    0,
    USERS_TABLE,
    0,
    gurv_fields,
    2,
    0,
    1,
    NULL,
    &gurv_validate,
  },

  {
    /* Q_GUBR - GET_USER_BY_RESERVATION */
    "get_user_by_reservation",
    "gubr",
    2,
    RETRIEVE,
    0,
    USERS_TABLE,
    0,
    gubr_fields,
    1,
    0,
    1,
    NULL,
    &gubr_validate,
  },

  {
    /* Q_DURV - DELETE_USER_RESERVATION */
    "delete_user_reservation",
    "durv",
    2,
    UPDATE,
    0,
    USERS_TABLE,
    0,
    aurv_fields,
    2,
    0,
    0,
    NULL,
    &aurv_validate,
  },

  {
    /* Q_GKUM - GET_KERBEROS_USER_MAP */
    "get_kerberos_user_map",
    "gkum",
    2,
    RETRIEVE,
    "k",
    KRBMAP_TABLE,
    "u.login, str.string FROM krbmap km, users u, strings str",
    gkum_fields,
    2,
    "u.login LIKE '%s' AND str.string LIKE '%s' AND km.users_id = u.users_id AND km.string_id = str.string_id",
    2,
    "u.login, str.string",
    NULL,
  },

  {
    /* Q_AKUM - ADD_KERBEROS_USER_MAP */
    "add_kerberos_user_map",
    "akum",
    2,
    APPEND,
    "k",
    KRBMAP_TABLE,
    "INTO krbmap (users_id, string_id) VALUES (%d, %d)",
    akum_fields,
    2,
    0,
    0,
    NULL,
    &akum_validate,
  },

  {
    /* Q_DKUM - DELETE_KERBEROS_USER_MAP */
    "delete_kerberos_user_map",
    "dkum",
    2,
    DELETE,
    "k",
    KRBMAP_TABLE,
    0,
    akum_fields,
    0,
    "users_id = %d AND string_id = %d",
    2,
    NULL,
    &dkum_validate,
  },

  {
    /* Q_GFBL - GET_FINGER_BY_LOGIN */
    "get_finger_by_login",
    "gfbl",
    2,
    RETRIEVE,
    "u",
    USERS_TABLE,
    "login, fullname, nickname, home_addr, home_phone, office_addr, office_phone, department, affiliation, TO_CHAR(fmodtime, 'DD-mon-YYYY HH24:MI:SS'), fmodby, fmodwith FROM users",
    gfbl_fields,
    12,
    "users_id = %d",
    1,
    "login",
    &gfbl_validate,
  },

  {
    /* Q_UFBL - UPDATE_FINGER_BY_LOGIN */
    "update_finger_by_login",
    "ufbl",
    2,
    UPDATE,
    "u",
    USERS_TABLE,
    "users SET fullname= NVL('%s', CHR(0)), nickname= NVL('%s', CHR(0)), home_addr= NVL('%s', CHR(0)), home_phone= NVL('%s', CHR(0)), office_addr= NVL('%s', CHR(0)), office_phone= NVL('%s', CHR(0)), department= NVL('%s', CHR(0)), affiliation= NVL('%s', CHR(0))",
    ufbl_fields,
    8,
    "users_id = %d",
    1,
    NULL,
    &ufbl_validate,
  },

  {
    /* Q_GPOB - GET_POBOX */
    "get_pobox",
    "gpob",
    2,
    RETRIEVE,
    "u",
    USERS_TABLE,
    "login, potype, users_id, CHR(0), TO_CHAR(pmodtime, 'DD-mon-YYYY HH24:MI:SS'), pmodby, pmodwith FROM users",
    gpob_fields,
    7,
    "users_id = %d",
    1,
    "login",
    &gpob_validate,
  },

  {
    /* Q_GAPO - GET_ALL_POBOXES */
    "get_all_poboxes",
    "gapo",
    2,
    RETRIEVE,
    "u",
    USERS_TABLE,
    "login, potype, pop_id || ':' || box_id FROM users",
    gpox_fields,
    3,
    "potype != 'NONE'",
    0,
    "login",
    &gpox_validate,
  },

  {
    /* Q_GPOP - GET_POBOXES_POP */
    "get_poboxes_pop",
    "gpop",
    2,
    RETRIEVE,
    "u",
    USERS_TABLE,
    "login, potype, users_id FROM users",
    gpox_fields,
    3,
    "potype = 'POP'",
    0,
    "login",
    &gpox_validate
  },

  {
    /* Q_GPOF - GET_POBOXES_SMTP */
    "get_poboxes_smtp",
    "gpos",
    2,
    RETRIEVE,
    "u",
    USERS_TABLE,
    "login, potype, users_id FROM users",
    gpox_fields,
    3,
    "potype = 'SMTP'",
    0,
    "login",
    &gpox_validate
  },

  {
    /* Q_SPOB - SET_POBOX */
    "set_pobox",
    "spob",
    2,
    UPDATE,
    0,
    USERS_TABLE,
    0,
    spob_fields,
    3,
    NULL,
    0,
    NULL,
    &spob_validate,
  },

  {
    /* Q_SPOP - SET_POBOX_POP */
    "set_pobox_pop",
    "spop",
    2,
    UPDATE,
    0,
    USERS_TABLE,
    0,
    spob_fields,
    1,
    NULL,
    0,
    NULL,
    &spop_validate,
  },

  {
    /* Q_DPOB - DELETE_POBOX */
    "delete_pobox",
    "dpob",
    2,
    UPDATE,
    "u",
    USERS_TABLE,
    "users SET potype = 'NONE'",
    spob_fields,
    0,
    "users_id = %d",
    1,
    NULL,
    &dpob_validate,
  },

  {
    /* Q_GHST - GET_HOST */
    "get_host",
    "ghst",
    2,
    RETRIEVE,
    "m",
    MACHINE_TABLE,
    "m.name, m.vendor, m.model, m.os, m.location, m.contact, m.use, m.status, TO_CHAR(m.statuschange, 'DD-mon-YYYY HH24:MI:SS'), s.name, m.address, m.owner_type, m.owner_id, m.acomment, m.ocomment, TO_CHAR(m.created, 'DD-mon-YYYY HH24:MI:SS'), m.creator, TO_CHAR(m.inuse, 'DD-mon-YYYY HH24:MI:SS'), TO_CHAR(m.modtime, 'DD-mon-YYYY HH24:MI:SS'), m.modby, m.modwith FROM machine m, subnet s",
    ghst_fields,
    21,
    "m.name LIKE UPPER('%s') AND m.address LIKE '%s' AND m.location LIKE UPPER('%s') AND s.name LIKE UPPER('%s') AND m.mach_id != 0 AND s.snet_id = m.snet_id",
    4,
    "m.name",
    &ghst_validate,
  },

  {
    /* Q_GHBH - GET_HOST_BY_HWADDR */
    "get_host_by_hwaddr",
    "ghbh",
    2,
    RETRIEVE,
    "m",
    MACHINE_TABLE,
    "m.name, m.vendor, m.model, m.os, m.location, m.contact, m.use, m.status, TO_CHAR(m.statuschange, 'DD-mon-YYYY HH24:MI:SS'), s.name, m.address, m.owner_type, m.owner_id, m.acomment, m.ocomment, TO_CHAR(m.created, 'DD-mon-YYYY HH24:MI:SS'), m.creator, TO_CHAR(m.inuse, 'DD-mon-YYYY HH24:MI:SS'), TO_CHAR(m.modtime, 'DD-mon-YYYY HH24:MI:SS'), m.modby, m.modwith FROM machine m, subnet s",
    ghst_fields,
    21,
    "m.hwaddr LIKE LOWER('%s') AND m.mach_id != 0 AND s.snet_id = m.snet_id",
    1,
    "m.name",
    NULL,
  },

  {
    /* Q_GHHA - GET_HOST_HWADDR */
    "get_host_hwaddr",
    "ghha",
    2,
    RETRIEVE,
    "m",
    MACHINE_TABLE,
    "m.hwaddr FROM machine m",
    ghha_fields,
    1,
    "m.name LIKE UPPER('%s')",
    1,
    NULL,
    NULL,
  },

  {
    /* Q_AHST - ADD_HOST */ /* uses prefetch_value() for mach_id */
    "add_host",
    "ahst",
    2,
    APPEND,
    "m",
    MACHINE_TABLE,
    "INTO machine (name, vendor, model, os, location, contact, use, status, statuschange, snet_id, address, owner_type, owner_id, acomment, ocomment, created, inuse, mach_id, creator) VALUES (UPPER('%s'), NVL(UPPER('%s'), CHR(0)), NVL(UPPER('%s'), CHR(0)), NVL(UPPER('%s'), CHR(0)), NVL(UPPER('%s'), CHR(0)), NVL('%s', CHR(0)), %s, %s, SYSDATE, %d, '%s', '%s', %d, %d, %d, SYSDATE, SYSDATE, %s, %s)",
    ahst_fields,
    14,
    0,
    0,
    NULL,
    &ahst_validate,
  },

  {
    /* Q_UHST - UPDATE_HOST */
    "update_host",
    "uhst",
    2,
    UPDATE,
    "m",
    MACHINE_TABLE,
    "machine SET name = NVL(UPPER('%s'), CHR(0)), vendor = NVL(UPPER('%s'), CHR(0)), model = NVL(UPPER('%s'), CHR(0)), os = NVL(UPPER('%s'), CHR(0)), location = NVL(UPPER('%s'), CHR(0)), contact = NVL('%s', CHR(0)), use = %s, status = %s, snet_id = %d, address = '%s', owner_type = '%s', owner_id = %d, acomment = %d, ocomment = %d",
    uhst_fields,
    14,
    "mach_id = %d",
    1,
    NULL,
    &uhst_validate,
  },

  {
    /* Q_UHHA - UPDATE_HOST_HWADDR */
    "update_host_hwaddr",
    "uhha",
    2,
    UPDATE,
    "m",
    MACHINE_TABLE,
    "machine SET hwaddr = NVL('%s', CHR(0))",
    uhha_fields,
    1,
    "mach_id = %d",
    1,
    NULL,
    &uhha_validate,
  },

  {
    /* Q_DHST - DELETE_HOST */
    "delete_host",
    "dhst",
    2,
    DELETE,
    "m",
    MACHINE_TABLE,
    NULL,
    dhst_fields,
    0,
    "mach_id = %d",
    1,
    NULL,
    &dhst_validate,
  },

  {
    /* Q_GMAC - GET_MACHINE */
    "get_machine",
    "gmac",
    2,
    RETRIEVE,
    "m",
    MACHINE_TABLE,
    "name, vendor, TO_CHAR(modtime, 'DD-mon-YYYY HH24:MI:SS'), modby, modwith FROM machine",
    gmac_fields,
    5,
    "name LIKE UPPER('%s') AND mach_id != 0",
    1,
    "name",
    &VDfix_modby,
  },

  {
    /* Q_GHAL - GET_HOSTALIAS */
    "get_hostalias",
    "ghal",
    2,
    RETRIEVE,
    "a",
    HOSTALIAS_TABLE,
    "a.name, m.name FROM hostalias a, machine m",
    ghal_fields,
    2,
    "m.mach_id = a.mach_id and a.name LIKE UPPER('%s') AND m.name LIKE UPPER('%s')",
    2,
    "a.name",
    &ghal_validate,
  },

  {
    /* Q_AHAL - ADD_HOSTALIAS */
    "add_hostalias",
    "ahal",
    2,
    APPEND,
    "a",
    HOSTALIAS_TABLE,
    "INTO hostalias (name, mach_id) VALUES (UPPER('%s'), %d)",
    ghal_fields,
    2,
    0,
    0,
    NULL,
    &ahal_validate,
  },

  {
    /* Q_DHAL - DELETE_HOSTALIAS */
    "delete_hostalias",
    "dhal",
    2,
    DELETE,
    "a",
    HOSTALIAS_TABLE,
    NULL,
    ghal_fields,
    0,
    "name = UPPER('%s') AND mach_id = %d",
    2,
    NULL,
    &dhal_validate,
  },

  {
    /* Q_GSNT - GET_SUBNET */
    "get_subnet",
    "gsnt",
    2,
    RETRIEVE,
    "s",
    SUBNET_TABLE,
    "name, description, saddr, mask, low, high, prefix, owner_type, owner_id, TO_CHAR(modtime, 'DD-mon-YYYY HH24:MI:SS'), modby, modwith FROM subnet",
    gsnt_fields,
    12,
    "name LIKE UPPER('%s')",
    1,
    "name",
    &gsnt_validate,
  },

  {
    /* Q_ASNT - ADD_SUBNET */
    "add_subnet",
    "asnt",
    2,
    APPEND,
    "s",
    SUBNET_TABLE,
    "INTO subnet (name, description, saddr, mask, low, high, prefix, owner_type, owner_id, snet_id) VALUES (UPPER('%s'), NVL('%s', CHR(0)), %s, %s, %s, %s, NVL('%s', CHR(0)), '%s', %d, %s)",
    asnt_fields,
    9,
    0,
    0,
    NULL,
    &asnt_validate,
  },

  {
    /* Q_USNT - UPDATE_SUBNET */
    "update_subnet",
    "usnt",
    2,
    UPDATE,
    "s",
    SUBNET_TABLE,
    "subnet SET name = UPPER('%s'), description = NVL('%s', CHR(0)), saddr = %s, mask = %s, low = %s, high = %s, prefix = NVL('%s', CHR(0)), owner_type = '%s', owner_id = %d",
    usnt_fields,
    9,
    "snet_id = %d",
    1,
    NULL,
    &usnt_validate,
  },

  {
    /* Q_DSNT - DELETE_SUBNET */
    "delete_subnet",
    "dsnt",
    2,
    DELETE,
    "s",
    SUBNET_TABLE,
    NULL,
    dsnt_fields,
    0,
    "snet_id = %d",
    1,
    NULL,
    &dsnt_validate,
  },

  {
    /* Q_GCLU - GET_CLUSTER */
    "get_cluster",
    "gclu",
    2,
    RETRIEVE,
    "c",
    CLUSTERS_TABLE,
    "name, description, location, TO_CHAR(modtime, 'DD-mon-YYYY HH24:MI:SS'), modby, modwith FROM clusters",
    gclu_fields,
    6,
    "name LIKE '%s' AND clu_id != 0",
    1,
    "name",
    &VDfix_modby,
  },

  {
    /* Q_ACLU - ADD_CLUSTER */ /* uses prefetch_value() for clu_id */
    "add_cluster",
    "aclu",
    2,
    APPEND,
    "c",
    CLUSTERS_TABLE,
    "INTO clusters (name, description, location, clu_id) VALUES ('%s', NVL('%s', CHR(0)), NVL('%s', CHR(0)), %s)",
    aclu_fields,
    3,
    0,
    0,
    NULL,
    &aclu_validate,
  },

  {
    /* Q_UCLU - UPDATE_CLUSTER */
    "update_cluster",
    "uclu",
    2,
    UPDATE,
    "c",
    CLUSTERS_TABLE,
    "clusters SET name = '%s', description = NVL('%s', CHR(0)), location = NVL('%s', CHR(0))",
    uclu_fields,
    3,
    "clu_id = %d",
    1,
    NULL,
    &uclu_validate,
  },

  {
    /* Q_DCLU - DELETE_CLUSTER */
    "delete_cluster",
    "dclu",
    2,
    DELETE,
    "c",
    CLUSTERS_TABLE,
    NULL,
    dclu_fields,
    0,
    "clu_id = %d",
    1,
    NULL,
    &dclu_validate,
  },

  {
    /* Q_GMCM - GET_MACHINE_TO_CLUSTER_MAP */
    "get_machine_to_cluster_map",
    "gmcm",
    2,
    RETRIEVE,
    "mcm",
    MCMAP_TABLE,
    "m.name, c.name FROM machine m, clusters c, mcmap mcm",
    gmcm_fields,
    2,
    "m.name LIKE UPPER('%s') AND c.name LIKE '%s' AND mcm.clu_id = c.clu_id AND mcm.mach_id = m.mach_id",
    2,
    "m.name",
    NULL,
  },

  {
    /* Q_AMTC - ADD_MACHINE_TO_CLUSTER */
    "add_machine_to_cluster",
    "amtc",
    2,
    APPEND,
    "mcm",
    MCMAP_TABLE,
    "INTO mcmap (mach_id, clu_id) VALUES (%d, %d)",
    gmcm_fields,
    2,
    0,
    0,
    NULL,
    &amtc_validate,
  },

  {
    /* Q_DMFC - DELETE_MACHINE_FROM_CLUSTER */
    "delete_machine_from_cluster",
    "dmfc",
    2,
    DELETE,
    "mcm",
    MCMAP_TABLE,
    0,
    gmcm_fields,
    0,
    "mach_id = %d AND clu_id = %d",
    2,
    NULL,
    &amtc_validate,
  },

  {
    /* Q_GCLD - GET_CLUSTER_DATA */
    "get_cluster_data",
    "gcld",
    2,
    RETRIEVE,
    "svc",
    SVC_TABLE,
    "c.name, svc.serv_label, svc.serv_cluster FROM svc svc, clusters c",
    gcld_fields,
    3,
    "c.clu_id = svc.clu_id AND c.name LIKE '%s' AND svc.serv_label LIKE '%s'",
    2,
    "c.name, svc.serv_label",
    NULL,
  },

  {
    /* Q_ACLD - ADD_CLUSTER_DATA */
    "add_cluster_data",
    "acld",
    2,
    APPEND,
    "svc",
    SVC_TABLE,
    "INTO svc (clu_id, serv_label, serv_cluster) VALUES (%d, '%s', '%s')",
    acld_fields,
    3,
    NULL,
    0,
    NULL,
    &acld_validate,
  },

  {
    /* Q_DCLD - DELETE_CLUSTER_DATA */
    "delete_cluster_data",
    "dcld",
    2,
    DELETE,
    "svc",
    SVC_TABLE,
    NULL,
    acld_fields,
    0,
    "clu_id = %d AND serv_label = '%s' AND serv_cluster = '%s'",
    3,
    NULL,
    &dcld_validate,
  },

  {
    /* Q_GLIN - GET_LIST_INFO */
    "get_list_info",
    "glin",
    2,
    RETRIEVE,
    "l",
    LIST_TABLE,
    "name, active, publicflg, hidden, maillist, grouplist, gid, acl_type, acl_id, description, TO_CHAR(modtime, 'DD-mon-YYYY HH24:MI:SS'), modby, modwith FROM list",
    glin_fields,
    13,
    "name LIKE '%s'",
    1,
    "name",
    &glin_validate,
  },

  {
    /* Q_EXLN - EXPAND_LIST_NAMES */
    "expand_list_names",
    "exln",
    2,
    RETRIEVE,
    "l",
    LIST_TABLE,
    "name FROM list",
    glin_fields,
    1,
    "name LIKE '%s' AND list_id != 0",
    1,
    "name",
    NULL,
  },

  {
    /* Q_ALIS - ADD_LIST */ /* uses prefetch_value() for list_id */
    "add_list",
    "alis",
    2,
    APPEND,
    "l",
    LIST_TABLE,
    "INTO list (name, active, publicflg, hidden, maillist, grouplist, gid, acl_type, acl_id, description, list_id) VALUES ('%s', %s, %s, %s, %s, %s, %s, '%s', %d, NVL('%s', CHR(0)), %s)",
    alis_fields,
    10,
    0,
    0,
    NULL,
    &alis_validate,
  },

  {
    /* Q_ULIS - UPDATE_LIST */
    "update_list",
    "ulis",
    2,
    UPDATE,
    "l",
    LIST_TABLE,
    "list SET name = '%s', active = %s, publicflg = %s, hidden = %s, maillist = %s, grouplist = %s, gid = %s, acl_type = '%s', acl_id = %d, description = NVL('%s', CHR(0))",
    ulis_fields,
    10,
    "list_id = %d",
    1,
    NULL,
    &ulis_validate,
  },

  {
    /* Q_DLIS - DELETE_LIST */
    "delete_list",
    "dlis",
    2,
    DELETE,
    "l",
    LIST_TABLE,
    NULL,
    dlis_fields,
    0,
    "list_id = %d",
    1,
    NULL,
    &dlis_validate,
  },

  {
    /* Q_AMTL - ADD_MEMBER_TO_LIST */
    "add_member_to_list",
    "amtl",
    2,
    APPEND,
    0,
    IMEMBERS_TABLE,
    0,
    amtl_fields,
    3,
    NULL,
    0,
    NULL,
    &amtl_validate,
  },

  {
    /* Q_ATML - ADD_TAGGED_MEMBER_TO_LIST */
    "add_tagged_member_to_list",
    "atml",
    2,
    APPEND,
    0,
    IMEMBERS_TABLE,
    0,
    atml_fields,
    4,
    NULL,
    0,
    NULL,
    &atml_validate,
  },

  {
    /* Q_TMOL - TAG_MEMBER_OF_LIST */
    "tag_member_of_list",
    "tmol",
    2,
    UPDATE,
    "im",
    IMEMBERS_TABLE,
    "imembers SET tag = %d",
    atml_fields,
    1,
    "list_id = %d AND member_type = '%s' and member_id = %d",
    3,
    NULL,
    &tmol_validate,
  },

  {
    /* Q_DMFL - DELETE_MEMBER_FROM_LIST */
    "delete_member_from_list",
    "dmfl",
    2,
    DELETE,
    0,
    IMEMBERS_TABLE,
    NULL,
    amtl_fields,
    0,
    0,
    3,
    NULL,
    &dmfl_validate,
  },

  {
    /* Q_GAUS - GET_ACE_USE */
    "get_ace_use",
    "gaus",
    2,
    RETRIEVE,
    0,
    0,
    0,
    gaus_fields,
    2,
    0,
    2,
    NULL,
    &gaus_validate,
  },

  {
    /* Q_QGLI - QUALIFIED_GET_LISTS */
    "qualified_get_lists",
    "qgli",
    2,
    RETRIEVE,
    0,
    LIST_TABLE,
    0,
    qgli_fields,
    1,
    0,
    5,
    NULL,
    &qgli_validate,
  },

  {
    /* Q_GMOL - GET_MEMBERS_OF_LIST */
    "get_members_of_list",
    "gmol",
    2,
    RETRIEVE,
    NULL,
    IMEMBERS_TABLE,
    NULL,
    gmol_fields,
    2,
    NULL,
    1,
    NULL,
    &gmol_validate,
  },

  {
    /* Q_GEML - GET_END_MEMBERS_OF_LIST */
    "get_end_members_of_list",
    "geml",
    2,
    RETRIEVE,
    NULL,
    IMEMBERS_TABLE,
    NULL,
    gmol_fields,
    2,
    NULL,
    1,
    NULL,
    &gmol_validate,
  },

  {
    /* Q_GTML - GET_TAGGED_MEMBERS_OF_LIST */
    "get_tagged_members_of_list",
    "gtml",
    2,
    RETRIEVE,
    NULL,
    IMEMBERS_TABLE,
    NULL,
    gtml_fields,
    3,
    NULL,
    1,
    NULL,
    &gmol_validate,
  },

  {
    /* Q_GLOM - GET_LISTS_OF_MEMBER */
    "get_lists_of_member",
    "glom",
    2,
    RETRIEVE,
    0,
    IMEMBERS_TABLE,
    0,
    glom_fields,
    6,
    0,
    2,
    NULL,
    &glom_validate,
  },

  {
    /* Q_CMOL - COUNT_MEMBERS_OF_LIST */
    "count_members_of_list",
    "cmol",
    2,
    RETRIEVE,
    0,
    IMEMBERS_TABLE,
    0,
    cmol_fields,
    1,
    0,
    1,
    NULL,
    &cmol_validate,
  },

  {
    /* Q_GSIN - GET_SERVER_INFO */
    "get_server_info",
    "gsin",
    2,
    RETRIEVE,
    "s",
    SERVERS_TABLE,
    "name, update_int, target_file, script, dfgen, dfcheck, type, enable, inprogress, harderror, errmsg, acl_type, acl_id, TO_CHAR(modtime, 'DD-mon-YYYY HH24:MI:SS'), modby, modwith FROM servers",
    gsin_fields,
    16,
    "name LIKE UPPER('%s')",
    1,
    "name",
    &gsin_validate,
  },

  {
    /* Q_QGSV - QUALIFIED_GET_SERVER */
    "qualified_get_server",
    "qgsv",
    2,
    RETRIEVE,
    0,
    SERVERS_TABLE,
    0,
    qgsv_fields,
    1,
    0,
    3,
    NULL,
    &qgsv_validate,
  },

  {
    /* Q_ASIN - ADD_SERVER_INFO */
    "add_server_info",
    "asin",
    2,
    APPEND,
    "s",
    SERVERS_TABLE,
    "INTO servers (name, update_int, target_file, script, type, enable, acl_type, acl_id) VALUES (UPPER('%s'), %s, '%s', '%s', '%s', %s, '%s', %d)",
    asin_fields,
    8,
    NULL,
    0,
    NULL,
    &asin_validate,
  },

  {
    /* Q_USIN - UPDATE_SERVER_INFO */
    "update_server_info",
    "usin",
    2,
    UPDATE,
    "s",
    SERVERS_TABLE,
    "servers SET update_int = %s, target_file = '%s', script = '%s', type = '%s', enable = %s, acl_type = '%s', acl_id = %d",
    asin_fields,
    7,
    "name = UPPER('%s')",
    1,
    NULL,
    &asin_validate,
  },

  {
    /* Q_RSVE - RESET_SERVER_ERROR */
    "reset_server_error",
    "rsve",
    2,
    UPDATE,
    "s",
    SERVERS_TABLE,
    "servers SET harderror = 0, dfcheck = dfgen",
    dsin_fields,
    0,
    "name = UPPER('%s')",
    1,
    NULL,
    &rsve_validate,
  },

  {
    /* Q_SSIF - SET_SERVER_INTERNAL_FLAGS */
    "set_server_internal_flags",
    "ssif",
    2,
    UPDATE,
    "s",
    SERVERS_TABLE,
    "servers SET dfgen = %s, dfcheck = %s, inprogress = %s, harderror = %s, errmsg = NVL('%s', CHR(0))",
    ssif_fields,
    5,
    "name = UPPER('%s')",
    1,
    NULL,
    &ssif_validate,
  },

  {
    /* Q_DSIN - DELETE_SERVER_INFO */
    "delete_server_info",
    "dsin",
    2,
    DELETE,
    "s",
    SERVERS_TABLE,
    NULL,
    dsin_fields,
    0,
    "name = UPPER('%s')",
    1,
    NULL,
    &dsin_validate,
  },

  {
    /* Q_GSHI - GET_SERVER_HOST_INFO */
    "get_server_host_info",
    "gshi",
    2,
    RETRIEVE,
    "sh",
    SERVERHOSTS_TABLE,
    "sh.service, m.name, sh.enable, sh.override, sh.success, sh.inprogress, sh.hosterror, sh.hosterrmsg, sh.ltt, sh.lts, sh.value1, sh.value2, sh.value3, TO_CHAR(sh.modtime, 'DD-mon-YYYY HH24:MI:SS'), sh.modby, sh.modwith FROM serverhosts sh, machine m",
    gshi_fields,
    16,
    "sh.service LIKE UPPER('%s') AND m.name LIKE UPPER('%s') AND m.mach_id = sh.mach_id",
    2,
    "sh.service, m.name",
    &gshi_validate,
  },

  {
    /* Q_QGSH - QUALIFIED_GET_SERVER_HOST */
    "qualified_get_server_host",
    "qgsh",
    2,
    RETRIEVE,
    0,
    SERVERHOSTS_TABLE,
    0,
    qgsh_fields,
    2,
    0,
    6,
    NULL,
    &qgsh_validate,
  },

  {
    /* Q_ASHI - ADD_SERVER_HOST_INFO */
    "add_server_host_info",
    "ashi",
    2,
    APPEND,
    "sh",
    SERVERHOSTS_TABLE,
    "INTO serverhosts (service, mach_id, enable, value1, value2, value3) VALUES (UPPER('%s'), %d, %s, %s, %s, NVL('%s', CHR(0)))",
    ashi_fields,
    6,
    NULL,
    0,
    NULL,
    &ashi_validate,
  },

  {
    /* Q_USHI - UPDATE_SERVER_HOST_INFO */
    "update_server_host_info",
    "ushi",
    2,
    UPDATE,
    "sh",
    SERVERHOSTS_TABLE,
    "serverhosts SET enable = %s, value1 = %s, value2 = %s, value3 = NVL('%s', CHR(0))",
    ashi_fields,
    4,
    "service = UPPER('%s') AND mach_id = %d",
    2,
    NULL,
    &ashi_validate,
  },

  {
    /* Q_RSHE - RESET_SERVER_HOST_ERROR */
    "reset_server_host_error",
    "rshe",
    2,
    UPDATE,
    "sh",
    SERVERHOSTS_TABLE,
    "serverhosts SET hosterror = 0",
    dshi_fields,
    0,
    "service = UPPER('%s') AND mach_id = %d",
    2,
    NULL,
    &rshe_validate,
  },

  {
    /* Q_SSHO - SET_SERVER_HOST_OVERRIDE */
    "set_server_host_override",
    "ssho",
    2,
    UPDATE,
    "sh",
    SERVERHOSTS_TABLE,
    "serverhosts SET override = 1",
    dshi_fields,
    0,
    "service = UPPER('%s') AND mach_id = %d",
    2,
    NULL,
    &ssho_validate,
  },

  {
    /* Q_SSHI - SET_SERVER_HOST_INTERNAL */
    "set_server_host_internal",
    "sshi",
    2,
    UPDATE,
    "s",
    SERVERHOSTS_TABLE,
    "serverhosts SET override = %s, success = %s, inprogress = %s, hosterror = %s, hosterrmsg = NVL('%s', CHR(0)), ltt = %s, lts = %s",
    sshi_fields,
    7,
    "service = UPPER('%s') AND mach_id = %d",
    2,
    NULL,
    &sshi_validate,
  },

  {
    /* Q_DSHI - DELETE_SERVER_HOST_INFO */
    "delete_server_host_info",
    "dshi",
    2,
    DELETE,
    "sh",
    SERVERHOSTS_TABLE,
    NULL,
    dshi_fields,
    0,
    "service = UPPER('%s') AND mach_id = %d",
    2,
    NULL,
    &dshi_validate,
  },

  {
    /* Q_GSLO - GET_SERVER_LOCATIONS */
    "get_server_locations",
    "gslo",
    2,
    RETRIEVE,
    "sh",
    SERVERHOSTS_TABLE,
    "sh.service, m.name FROM serverhosts sh, machine m",
    gslo_fields,
    2,
    "sh.service LIKE UPPER('%s') AND sh.mach_id = m.mach_id",
    1,
    "sh.service, m.name",
    NULL,
  },

  {
    /* Q_GFSL - GET_FILESYS_BY_LABEL */
    "get_filesys_by_label",
    "gfsl",
    2,
    RETRIEVE,
    "fs",
    FILESYS_TABLE,
    "fs.label, fs.type, m.name, fs.name, fs.mount, fs.rwaccess, fs.comments, u.login, l.name, fs.createflg, fs.lockertype, TO_CHAR(fs.modtime, 'DD-mon-YYYY HH24:MI:SS'), fs.modby, fs.modwith FROM filesys fs, machine m, users u, list l",
    gfsl_fields,
    14,
    "fs.label LIKE '%s' AND fs.mach_id = m.mach_id AND fs.owner = u.users_id AND fs.owners = l.list_id",
    1,
    "fs.label",
    &gfsl_validate,
  },

  {
    /* Q_GFSM - GET_FILESYS_BY_MACHINE */
    "get_filesys_by_machine",
    "gfsm",
    2,
    RETRIEVE,
    "fs",
    FILESYS_TABLE,
    "fs.label, fs.type, m.name, fs.name, fs.mount, fs.rwaccess, fs.comments, u.login, l.name, fs.createflg, fs.lockertype, TO_CHAR(fs.modtime, 'DD-mon-YYYY HH24:MI:SS'), fs.modby, fs.modwith FROM filesys fs, machine m, users u, list l",
    gfsm_fields,
    14,
    "fs.mach_id = %d AND m.mach_id = fs.mach_id AND fs.owner = u.users_id AND fs.owners = l.list_id",
    1,
    "fs.label",
    &gfsm_validate,
  },

  {
    /* Q_GFSN - GET_FILESYS_BY_NFSPHYS */
    "get_filesys_by_nfsphys",
    "gfsn",
    2,
    RETRIEVE,
    "fs",
    FILESYS_TABLE,
    "fs.label, fs.type, m.name, fs.name, fs.mount, fs.rwaccess, fs.comments, u.login, l.name, fs.createflg, fs.lockertype, TO_CHAR(fs.modtime, 'DD-mon-YYYY HH24:MI:SS'), fs.modby, fs.modwith FROM filesys fs, machine m, users u, list l, nfsphys np",
    gfsn_fields,
    14,
    "fs.mach_id = %d AND m.mach_id = fs.mach_id AND fs.owner = u.users_id AND fs.owners = l.list_id AND np.nfsphys_id = fs.phys_id AND np.dir LIKE '%s'",
    2,
    "fs.label",
    &gfsn_validate,
  },

  {
    /* Q_GFSG - GET_FILESYS_BY_GROUP */
    "get_filesys_by_group",
    "gfsg",
    2,
    RETRIEVE,
    "fs",
    FILESYS_TABLE,
    "fs.label, fs.type, m.name, fs.name, fs.mount, fs.rwaccess, fs.comments, u.login, l.name, fs.createflg, fs.lockertype, TO_CHAR(fs.modtime, 'DD-mon-YYYY HH24:MI:SS'), fs.modby, fs.modwith FROM filesys fs, machine m, users u, list l",
    gfsg_fields,
    14,
    "fs.owners = %d AND m.mach_id = fs.mach_id AND fs.owner = u.users_id AND fs.owners = l.list_id",
    1,
    "fs.label",
    &gfsg_validate,
  },

  {
    /* Q_GFSP - GET_FILESYS_BY_PATH */
    "get_filesys_by_path",
    "gfsp",
    2,
    RETRIEVE,
    "fs",
    FILESYS_TABLE,
    "fs.label, fs.type, m.name, fs.name, fs.mount, fs.rwaccess, fs.comments, u.login, l.name, fs.createflg, fs.lockertype, TO_CHAR(fs.modtime, 'DD-mon-YYYY HH24:MI:SS'), fs.modby, fs.modwith FROM filesys fs, machine m, users u, list l",
    gfsp_fields,
    14,
    "fs.name LIKE '%s' AND m.mach_id = fs.mach_id AND fs.owner = u.users_id AND fs.owners = list_id",
    1,
    "fs.label",
    &VDfix_modby,
  },

  {
    /* Q_AFIL - ADD_FILESYS */ /* uses prefetch_value() for filsys_id */
    "add_filesys",
    "afil",
    2,
    APPEND,
    "fs",
    FILESYS_TABLE,
    "INTO filesys (label, type, mach_id, name, mount, rwaccess, comments, owner, owners, createflg, lockertype, filsys_id) VALUES ('%s', '%s', %d, NVL('%s', CHR(0)), NVL('%s', CHR(0)), '%s', NVL('%s', CHR(0)), %d, %d, %s, '%s', %s)",
    afil_fields,
    11,
    0,
    0,
    NULL,
    &afil_validate,
  },

  {
    /* Q_UFIL - UPDATE_FILESYS */
    "update_filesys",
    "ufil",
    2,
    UPDATE,
    "fs",
    FILESYS_TABLE,
    "filesys SET label = '%s', type = '%s', mach_id = %d, name = NVL('%s', CHR(0)), mount = NVL('%s', CHR(0)), rwaccess = '%s', comments = NVL('%s', CHR(0)), owner = %d, owners = %d, createflg = %s, lockertype = '%s'",
    ufil_fields,
    11,
    "filsys_id = %d",
    1,
    NULL,
    &ufil_validate,
  },

  {
    /* Q_DFIL - DELETE_FILESYS */
    "delete_filesys",
    "dfil",
    2,
    DELETE,
    "fs",
    FILESYS_TABLE,
    NULL,
    dfil_fields,
    0,
    "filsys_id = %d",
    1,
    NULL,
    &dfil_validate,
  },

  {
    /* Q_GFGM - GET_FSGROUP_MEMBERS */
    "get_fsgroup_members",
    "gfgm",
    2,
    RETRIEVE,
    "fg",
    FSGROUP_TABLE,
    "fs.label, fg.key FROM fsgroup fg, filesys fs",
    gfgm_fields,
    2,
    "fg.group_id = %d AND fs.filsys_id = fg.filsys_id",
    1,
    "fs.label, fg.key",
    &gfgm_validate,
  },

  {
    /* Q_AFTG - ADD_FILESYS_TO_FSGROUP */
    "add_filesys_to_fsgroup",
    "aftg",
    2,
    APPEND,
    "fg",
    FSGROUP_TABLE,
    "INTO fsgroup (group_id, filsys_id, key) VALUES (%d, %d, '%s')",
    gfgm_fields,
    3,
    NULL,
    0,
    NULL,
    &aftg_validate,
  },

  {
    /* Q_RFFG - REMOVE_FILESYS_FROM_FSGROUP */
    "remove_filesys_from_fsgroup",
    "rffg",
    2,
    DELETE,
    "fg",
    FSGROUP_TABLE,
    NULL,
    gfgm_fields,
    0,
    "group_id = %d AND filsys_id = %d",
    2,
    NULL,
    &aftg_validate,
  },

  {
    /* Q_GANF - GET_ALL_NFSPHYS */
    "get_all_nfsphys",
    "ganf",
    2,
    RETRIEVE,
    "np",
    NFSPHYS_TABLE,
    "m.name, np.dir, np.device, np.status, np.allocated, np.partsize, TO_CHAR(np.modtime, 'DD-mon-YYYY HH24:MI:SS'), np.modby, np.modwith FROM nfsphys np, machine m",
    ganf_fields,
    9,
    "m.mach_id = np.mach_id",
    0,
    "m.name, np.dir",
    &VDfix_modby,
  },

  {
    /* Q_GNFP - GET_NFSPHYS */
    "get_nfsphys",
    "gnfp",
    2,
    RETRIEVE,
    "np",
    NFSPHYS_TABLE,
    "m.name, np.dir, np.device, np.status, np.allocated, np.partsize, TO_CHAR(np.modtime, 'DD-mon-YYYY HH24:MI:SS'), np.modby, np.modwith FROM nfsphys np, machine m",
    gnfp_fields,
    9,
    "np.mach_id = %d AND np.dir LIKE '%s' AND m.mach_id = np.mach_id",
    2,
    "m.name, np.dir",
    &gnfp_validate,
  },

  {
    /* Q_ANFP - ADD_NFSPHYS */ /* uses prefetch_value() for nfsphys_id */
    "add_nfsphys",
    "anfp",
    2,
    APPEND,
    "np",
    NFSPHYS_TABLE,
    "INTO nfsphys (mach_id, dir, device, status, allocated, partsize, nfsphys_id) VALUES (%d, '%s', NVL('%s', CHR(0)), %s, %s, %s, %s)",
    ganf_fields,
    6,
    0,
    0,
    NULL,
    &anfp_validate,
  },

  {
    /* Q_UNFP - UPDATE_NFSPHYS */
    "update_nfsphys",
    "unfp",
    2,
    UPDATE,
    "np",
    NFSPHYS_TABLE,
    "nfsphys SET device = NVL('%s', CHR(0)), status = %s, allocated = %s, partsize = %s",
    ganf_fields,
    4,
    "mach_id = %d AND dir = '%s'",
    2,
    NULL,
    &unfp_validate,
  },

  {
    /* Q_AJNF - ADJUST_NFSPHYS_ALLOCATION */
    "adjust_nfsphys_allocation",
    "ajnf",
    2,
    UPDATE,
    "np",
    NFSPHYS_TABLE,
    "nfsphys SET allocated = allocated + %s",
    ajnf_fields,
    1,
    "mach_id = %d AND dir = '%s'",
    2,
    NULL,
    &ajnf_validate,
  },

  {
    /* Q_DNFP - DELETE_NFSPHYS */
    "delete_nfsphys",
    "dnfp",
    2,
    DELETE,
    "np",
    NFSPHYS_TABLE,
    NULL,
    dnfp_fields,
    0,
    "mach_id = %d AND dir = '%s'",
    2,
    NULL,
    &dnfp_validate,
  },

  {
    /* Q_GQOT - GET_QUOTA */
    "get_quota",
    "gqot",
    2,
    RETRIEVE,
    "q",
    QUOTA_TABLE,
    "fs.label, q.type, q.entity_id, q.quota, q.phys_id, m.name, TO_CHAR(q.modtime, 'DD-mon-YYYY HH24:MI:SS'), q.modby, q.modwith FROM quota q, filesys fs, machine m",
    gqot_fields,
    9,
    "fs.label LIKE '%s' AND q.type = '%s' AND q.entity_id = %d AND fs.filsys_id = q.filsys_id AND m.mach_id = fs.mach_id",
    3,
    NULL,
    &gqot_validate,
  },

  {
    /* Q_GQBF - GET_QUOTA_BY_FILESYS */
    "get_quota_by_filesys",
    "gqbf",
    2,
    RETRIEVE,
    "q",
    QUOTA_TABLE,
    "fs.label, q.type, q.entity_id, q.quota, q.phys_id, m.name, TO_CHAR(q.modtime, 'DD-mon-YYYY HH24:MI:SS'), q.modby, q.modwith FROM quota q, filesys fs, machine m",
    gqbf_fields,
    9,
    "fs.label LIKE '%s' AND fs.filsys_id = q.filsys_id AND m.mach_id = fs.mach_id",
    1,
    "fs.label, q.type",
    &gqbf_validate,
  },

  {
    /* Q_AQOT - ADD_QUOTA */ /* prefetch_filsys() gets last 1 value */
    "add_quota",
    "aqot",
    2,
    APPEND,
    0,
    QUOTA_TABLE,
    "INTO quota (filsys_id, type, entity_id, quota, phys_id) VALUES ('%s', %d, %d, %s, %s)",
    aqot_fields,
    4,
    NULL,
    0,
    NULL,
    &aqot_validate,
  },

  {
    /* Q_UQOT - UPDATE_QUOTA */
    "update_quota",
    "uqot",
    2,
    UPDATE,
    0,
    QUOTA_TABLE,
    "quota SET quota = %s",
    aqot_fields,
    1,
    0,
    3,
    NULL,
    &uqot_validate,
  },

  {
    /* Q_DQOT - DELETE_QUOTA */
    "delete_quota",
    "dqot",
    2,
    DELETE,
    0,
    QUOTA_TABLE,
    NULL,
    aqot_fields,
    0,
    0,
    3,
    NULL,
    &dqot_validate,
  },

  {
    /* Q_GNFQ - GET_NFS_QUOTAS */
    "get_nfs_quota",
    "gnfq",
    2,
    RETRIEVE,
    "q",
    QUOTA_TABLE,
    "fs.label, u.login, q.quota, q.phys_id, m.name, TO_CHAR(q.modtime, 'DD-mon-YYYY HH24:MI:SS'), q.modby, q.modwith FROM quota q, filesys fs, users u, machine m",
    gnfq_fields,
    8,
    "fs.label LIKE '%s' AND q.type = 'USER' AND q.entity_id = u.users_id AND fs.filsys_id = q.filsys_id AND m.mach_id = fs.mach_id AND u.login = '%s'",
    2,
    "fs.label, u.login",
    &gnfq_validate,
  },

  {
    /* Q_GNQP - GET_NFS_QUOTAS_BY_PARTITION */
    "get_nfs_quotas_by_partition",
    "gnqp",
    2,
    RETRIEVE,
    "q",
    QUOTA_TABLE,
    "fs.label, u.login, q.quota, np.dir, m.name FROM quota q, filesys fs, users u, nfsphys np, machine m",
    gnqp_fields,
    5,
    "np.mach_id = %d AND np.dir LIKE '%s' AND q.phys_id = np.nfsphys_id AND fs.filsys_id = q.filsys_id AND q.type = 'USER' AND u.users_id = q.entity_id AND m.mach_id = np.mach_id",
    2,
    "fs.label",
    NULL,
  },

  {
    /* Q_ANFQ - ADD_NFS_QUOTA */ /* prefetch_filsys() gets last 1 value */
    "add_nfs_quota",
    "anfq",
    2,
    APPEND,
    0,
    QUOTA_TABLE,
    "INTO quota (type, filsys_id, entity_id, quota, phys_id ) VALUES ('USER', %d, %d, %s, %s)",
    anfq_fields,
    3,
    NULL,
    0,
    NULL,
    &anfq_validate,
  },

  {
    /* Q_UNFQ - UPDATE_NFS_QUOTA */
    "update_nfs_quota",
    "unfq",
    2,
    UPDATE,
    0,
    QUOTA_TABLE,
    "quota SET quota = %s",
    anfq_fields,
    1,
    0,
    2,
    NULL,
    &unfq_validate,
  },

  {
    /* Q_DNFQ - DELETE_NFS_QUOTA */
    "delete_nfs_quota",
    "dnfq",
    2,
    DELETE,
    0,
    QUOTA_TABLE,
    NULL,
    anfq_fields,
    0,
    0,
    2,
    NULL,
    &dnfq_validate,
  },

  {
    /* Q_GZCL - GET_ZEPHYR_CLASS */
    "get_zephyr_class",
    "gzcl",
    2,
    RETRIEVE,
    "z",
    ZEPHYR_TABLE,
    "class, xmt_type, xmt_id, sub_type, sub_id, iws_type, iws_id, iui_type, iui_id, TO_CHAR(modtime, 'DD-mon-YYYY HH24:MI:SS'), modby, modwith FROM zephyr",
    gzcl_fields,
    12,
    "class LIKE '%s'",
    1,
    "class",
    &gzcl_validate,
  },

  {
    /* Q_AZCL - ADD_ZEPHYR_CLASS */
    "add_zephyr_class",
    "azcl",
    2,
    APPEND,
    "z",
    ZEPHYR_TABLE,
    "INTO zephyr (class, xmt_type, xmt_id, sub_type, sub_id, iws_type, iws_id, iui_type, iui_id) VALUES ('%s', '%s', %d, '%s', %d, '%s', %d, '%s', %d)",
    azcl_fields,
    9,
    0,
    0,
    NULL,
    &azcl_validate,
  },

  {
    /* Q_UZCL - UPDATE_ZEPHYR_CLASS */
    "update_zephyr_class",
    "uzcl",
    2,
    UPDATE,
    "z",
    ZEPHYR_TABLE,
    "zephyr SET class = '%s', xmt_type = '%s', xmt_id = %d, sub_type = '%s', sub_id = %d, iws_type = '%s', iws_id = %d, iui_type = '%s', iui_id = %d",
    uzcl_fields,
    9,
    "class = '%s'",
    1,
    NULL,
    &uzcl_validate,
  },

  {
    /* Q_DZCL - DELETE_ZEPHYR_CLASS */
    "delete_zephyr_class",
    "dzcl",
    2,
    DELETE,
    "z",
    ZEPHYR_TABLE,
    0,
    uzcl_fields,
    0,
    "class = '%s'",
    1,
    NULL,
    &dzcl_validate,
  },

  {
    /* Q_GSHA - GET_SERVER_HOST_ACCESS */
    "get_server_host_access",
    "gsha",
    2,
    RETRIEVE,
    "ha",
    HOSTACCESS_TABLE,
    "m.name, ha.acl_type, ha.acl_id, TO_CHAR(ha.modtime, 'DD-mon-YYYY HH24:MI:SS'), ha.modby, ha.modwith FROM hostaccess ha, machine m",
    gsha_fields,
    6,
    "m.name LIKE UPPER('%s') AND ha.mach_id = m.mach_id",
    1,
    "m.name",
    &gsha_validate,
  },

  {
    /* Q_ASHA - ADD_SERVER_HOST_ACCESS */
    "add_server_host_access",
    "asha",
    2,
    APPEND,
    "ha",
    HOSTACCESS_TABLE,
    "INTO hostaccess (mach_id, acl_type, acl_id) VALUES (%d, '%s', %d)",
    asha_fields,
    3,
    0,
    0,
    NULL,
    &asha_validate,
  },

  {
    /* Q_USHA - UPDATE_SERVER_HOST_ACCESS */
    "update_server_host_access",
    "usha",
    2,
    UPDATE,
    "ha",
    HOSTACCESS_TABLE,
    "hostaccess SET acl_type = '%s', acl_id = %d",
    asha_fields,
    2,
    "mach_id = %d",
    1,
    NULL,
    &asha_validate,
  },

  {
    /* Q_DSHA - DELETE_SERVER_HOST_ACCESS */
    "delete_server_host_access",
    "dsha",
    2,
    DELETE,
    "ha",
    HOSTACCESS_TABLE,
    0,
    asha_fields,
    0,
    "mach_id = %d",
    1,
    NULL,
    &VDmach,
  },

  {
    /* Q_GACL - GET_ACL */
    "get_acl",
    "gacl",
    2,
    RETRIEVE,
    "ac",
    ACL_TABLE,
    "m.name, ac.target, ac.kind, l.name FROM acl ac, machine m, list l",
    gacl_fields,
    4,
    "m.mach_id = %d AND m.mach_id = ac.mach_id AND ac.target LIKE '%s' AND l.list_id = ac.list_id",
    2,
    "m.name, ac.target, ac.kind",
    &gacl_validate,
  },

  {
    /* Q_AACL - ADD_ACL */
    "add_acl",
    "aacl",
    2,
    APPEND,
    "ac",
    ACL_TABLE,
    "INTO acl (mach_id, target, kind, list_id) VALUES (%d, '%s', '%s', %d)",
    aacl_fields,
    4,
    0,
    0,
    NULL,
    &aacl_validate,
  },

  {
    /* Q_DACL - DELETE_ACL */
    "delete_acl",
    "dacl",
    2,
    DELETE,
    "ac",
    ACL_TABLE,
    0,
    dacl_fields,
    0,
    "mach_id = %d AND target = '%s'",
    2,
    NULL,
    &gacl_validate,
  },

  {
    /* Q_GSVC - GET_SERVICE */
    "get_service",
    "gsvc",
    2,
    RETRIEVE,
    "ss",
    SERVICES_TABLE,
    "name, protocol, port, description, TO_CHAR(modtime, 'DD-mon-YYYY HH24:MI:SS'), modby, modwith FROM services",
    gsvc_fields,
    7,
    "name LIKE '%s'",
    1,
    "name",
    &VDfix_modby,
  },

  {
    /* Q_ASVC - ADD_SERVICE */
    "add_service",
    "asvc",
    2,
    APPEND,
    "ss",
    SERVICES_TABLE,
    "INTO services (name, protocol, port, description) VALUES ('%s', '%s', %s, NVL('%s', CHR(0)))",
    asvc_fields,
    4,
    NULL,
    0,
    NULL,
    &asvc_validate,
  },

  {
    /* Q_DSVC - DELETE_SERVICE */
    "delete_service",
    "dsvc",
    2,
    DELETE,
    "ss",
    SERVICES_TABLE,
    0,
    asvc_fields,
    0,
    "name = '%s'",
    1,
    NULL,
    &asvc_validate,
  },

  {
    /* Q_GPRN - GET_PRINTER */
    "get_printer",
    "gprn",
    2,
    RETRIEVE,
    "pr",
    PRINTERS_TABLE,
    "pr.name, pr.type, pr.hwtype, pr.duplexname, m1.name, m2.name, m3.name, pr.rp, m4.name, pr.ka, pr.pc, l1.name, l2.name, pr.banner, pr.location, pr.contact, TO_CHAR(pr.modtime, 'DD-mon-YYYY HH24:MI:SS'), pr.modby, pr.modwith FROM printers pr, machine m1, machine m2, machine m3, machine m4, list l1, list l2",
    gprn_fields,
    19,
    "pr.name LIKE '%s' AND m1.mach_id = pr.mach_id AND m2.mach_id = pr.loghost AND m3.mach_id = pr.rm AND m4.mach_id = pr.rq AND l1.list_id = pr.ac AND l2.list_id = pr.lpc_acl",
    1,
    "pr.name",
    &VDfix_modby,
  },

  {
    /* Q_GPBD - GET_PRINTER_BY_DUPLEXNAME */
    "get_printer_by_duplexname",
    "gpbd",
    2,
    RETRIEVE,
    "pr",
    PRINTERS_TABLE,
    "pr.name, pr.type, pr.hwtype, pr.duplexname, m1.name, m2.name, m3.name, pr.rp, m4.name, pr.ka, pr.pc, l1.name, l2.name, pr.banner, pr.location, pr.contact, TO_CHAR(pr.modtime, 'DD-mon-YYYY HH24:MI:SS'), pr.modby, pr.modwith FROM printers pr, machine m1, machine m2, machine m3, machine m4, list l1, list l2",
    gpbd_fields,
    19,
    "pr.duplexname LIKE '%s' AND m1.mach_id = pr.mach_id AND m2.mach_id = pr.loghost AND m3.mach_id = pr.rm AND m4.mach_id = pr.rq AND l1.list_id = pr.ac AND l2.list_id = pr.lpc_acl",
    1,
    "pr.name",
    &VDfix_modby,
  },

  {
    /* Q_GPBE - GET_PRINTER_BY_ETHERNET */
    "get_printer_by_ethernet",
    "gpbe",
    2,
    RETRIEVE,
    "pr",
    PRINTERS_TABLE,
    "pr.name, pr.type, pr.hwtype, pr.duplexname, m1.name, m2.name, m3.name, pr.rp, m4.name, pr.ka, pr.pc, l1.name, l2.name, pr.banner, pr.location, pr.contact, TO_CHAR(pr.modtime, 'DD-mon-YYYY HH24:MI:SS'), pr.modby, pr.modwith FROM printers pr, machine m1, machine m2, machine m3, machine m4, list l1, list l2",
    gpbd_fields,
    19,
    "m1.hwaddr LIKE LOWER('%s') AND m1.mach_id = pr.mach_id AND m2.mach_id = pr.loghost AND m3.mach_id = pr.rm AND m4.mach_id = pr.rq AND l1.list_id = pr.ac AND l2.list_id = pr.lpc_acl",
    1,
    "pr.name",
    &VDfix_modby,
  },

  {
    /* Q_GPBH - GET_PRINTER_BY_HOSTNAME */
    "get_printer_by_hostname",
    "gpbh",
    2,
    RETRIEVE,
    "pr",
    PRINTERS_TABLE,
    "pr.name, pr.type, pr.hwtype, pr.duplexname, m1.name, m2.name, m3.name, pr.rp, m4.name, pr.ka, pr.pc, l1.name, l2.name, pr.banner, pr.location, pr.contact, TO_CHAR(pr.modtime, 'DD-mon-YYYY HH24:MI:SS'), pr.modby, pr.modwith FROM printers pr, machine m1, machine m2, machine m3, machine m4, list l1, list l2",
    gpbh_fields,
    19,
    "m1.name LIKE UPPER('%s') AND m1.mach_id = pr.mach_id AND m2.mach_id = pr.loghost AND m3.mach_id = pr.rm AND m4.mach_id = pr.rq AND l1.list_id = pr.ac AND l2.list_id = pr.lpc_acl",
    1,
    "pr.name",
    &VDfix_modby,
  },

  {
    /* Q_GPBR - GET_PRINTER_BY_RM */
    "get_printer_by_rm",
    "gpbr",
    2,
    RETRIEVE,
    "pr",
    PRINTERS_TABLE,
    "pr.name, pr.type, pr.hwtype, pr.duplexname, m1.name, m2.name, m3.name, pr.rp, m4.name, pr.ka, pr.pc, l1.name, l2.name, pr.banner, pr.location, pr.contact, TO_CHAR(pr.modtime, 'DD-mon-YYYY HH24:MI:SS'), pr.modby, pr.modwith FROM printers pr, machine m1, machine m2, machine m3, machine m4, list l1, list l2",
    gpbr_fields,
    19,
    "m3.name LIKE UPPER('%s') AND m1.mach_id = pr.mach_id AND m2.mach_id = pr.loghost AND m3.mach_id = pr.rm AND m4.mach_id = pr.rq AND l1.list_id = pr.ac AND l2.list_id = pr.lpc_acl",
    1,
    "pr.name",
    &VDfix_modby,
  },

  {
    /* Q_GPBL - GET_PRINTER_BY_LOCATION */
    "get_printer_by_location",
    "gpbl",
    2,
    RETRIEVE,
    "pr",
    PRINTERS_TABLE,
    "pr.name, pr.type, pr.hwtype, pr.duplexname, m1.name, m2.name, m3.name, pr.rp, m4.name, pr.ka, pr.pc, l1.name, l2.name, pr.banner, pr.location, pr.contact, TO_CHAR(pr.modtime, 'DD-mon-YYYY HH24:MI:SS'), pr.modby, pr.modwith FROM printers pr, machine m1, machine m2, machine m3, machine m4, list l1, list l2",
    gpbl_fields,
    19,
    "UPPER(pr.location) LIKE UPPER('%s') AND m1.mach_id = pr.mach_id AND m2.mach_id = pr.loghost AND m3.mach_id = pr.rm AND m4.mach_id = pr.rq AND l1.list_id = pr.ac AND l2.list_id = pr.lpc_acl",
    1,
    "pr.name",
    &VDfix_modby,
  },

  {
    /* Q_GPBC - GET_PRINTER_BY_CONTACT */
    "get_printer_by_contact",
    "gpbc",
    2,
    RETRIEVE,
    "pr",
    PRINTERS_TABLE,
    "pr.name, pr.type, pr.hwtype, pr.duplexname, m1.name, m2.name, m3.name, pr.rp, m4.name, pr.ka, pr.pc, l1.name, l2.name, pr.banner, pr.location, pr.contact, TO_CHAR(pr.modtime, 'DD-mon-YYYY HH24:MI:SS'), pr.modby, pr.modwith FROM printers pr, machine m1, machine m2, machine m3, machine m4, list l1, list l2",
    gpbc_fields,
    19,
    "UPPER(pr.contact) LIKE UPPER('%s') AND m1.mach_id = pr.mach_id AND m2.mach_id = pr.loghost AND m3.mach_id = pr.rm AND m4.mach_id = pr.rq AND l1.list_id = pr.ac AND l2.list_id = pr.lpc_acl",
    1,
    "pr.name",
    &VDfix_modby,
  },

  {
    /* Q_APRN - ADD_PRINTER */
    "add_printer",
    "aprn",
    2,
    APPEND,
    "pr",
    PRINTERS_TABLE,
    "INTO printers (name, type, hwtype, duplexname, mach_id, loghost, rm, rp, rq, ka, pc, ac, lpc_acl, banner, location, contact) VALUES ('%s', '%s', '%s', NVL('%s', CHR(0)), %d, %d, %d, NVL('%s', CHR(0)), %d, %s, %s, %d, %d, %s, NVL('%s', CHR(0)), NVL('%s', CHR(0)))",
    aprn_fields,
    16,
    0,
    0,
    NULL,
    &aprn_validate,
  },

  {
    /* Q_UPRN - UPDATE_PRINTER */
    "update_printer",
    "uprn",
    2,
    UPDATE,
    "pr",
    PRINTERS_TABLE,
    "printers SET name = '%s', type = '%s', hwtype = '%s', duplexname = NVL('%s', CHR(0)), mach_id = %d, loghost = %d, rm = %d, rp = NVL('%s', CHR(0)), rq = %d, ka = %d, pc = %d, ac = %d, lpc_acl = %d, banner = %d, location = NVL('%s', CHR(0)), contact = NVL('%s', CHR(0))",
    uprn_fields,
    16,
    0,
    0,
    NULL,
    &uprn_validate,
  },

  {
    /* Q_DPRN - DELETE_PRINTER */
    "delete_printer",
    "dprn",
    2,
    DELETE,
    "pr",
    PRINTERS_TABLE,
    0,
    aprn_fields,
    0,
    "name = '%s'",
    1,
    NULL,
    &dprn_validate,
  },

  {
    /* Q_GPSV - GET_PRINT_SERVER */
    "get_print_server",
    "gpsv",
    2,
    RETRIEVE,
    "ps",
    PRINTSERVERS_TABLE,
    "m.name, ps.kind, s.string, ps.owner_type, ps.owner_id, l.name, TO_CHAR(ps.modtime, 'DD-mon-YYYY HH24:MI:SS'), ps.modby, ps.modwith FROM printservers ps, machine m, strings s, list l",
    gpsv_fields,
    9,
    "ps.mach_id = %d AND m.mach_id = ps.mach_id AND s.string_id = ps.printer_types AND l.list_id = ps.lpc_acl",
    1,
    NULL,
    &gpsv_validate,
  },

  {
    /* Q_APSV - ADD_PRINT_SERVER */
    "add_print_server",
    "apsv",
    2,
    APPEND,
    "ps",
    PRINTSERVERS_TABLE,
    "INTO printservers (mach_id, kind, printer_types, owner_type, owner_id, lpc_acl) VALUES (%d, '%s', %d, '%s', %d, %d)",
    apsv_fields,
    6,
    0,
    0,
    NULL,
    &apsv_validate,
  },

  {
    /* Q_UPSV - UPDATE_PRINT_SERVER */
    "update_print_server",
    "upsv",
    2,
    UPDATE,
    "ps",
    PRINTSERVERS_TABLE,
    "printservers SET kind = '%s', printer_types = %d, owner_type = '%s', owner_id = %d, lpc_acl = %d",
    apsv_fields,
    5,
    "mach_id = %d",
    1,
    NULL,
    &apsv_validate,
  },

  {
    /* Q_DPSV - DELETE_PRINT_SERVER */
    "delete_print_server",
    "dpsv",
    2,
    DELETE,
    "ps",
    PRINTSERVERS_TABLE,
    0,
    dpsv_fields,
    0,
    "mach_id = %d",
    1,
    NULL,
    &dpsv_validate,
  },

  {
    /* Q_GALI - GET_ALIAS */
    "get_alias",
    "gali",
    2,
    RETRIEVE,
    "a",
    ALIAS_TABLE,
    "name, type, trans FROM alias",
    gali_fields,
    3,
    "name LIKE '%s' AND type LIKE '%s' AND trans LIKE '%s'",
    3,
    "type, name, trans",
    NULL,
  },

  {
    /* Q_AALI - ADD_ALIAS */
    "add_alias",
    "aali",
    2,
    APPEND,
    "a",
    ALIAS_TABLE,
    "INTO alias (name, type, trans) VALUES ('%s', '%s', '%s')",
    aali_fields,
    3,
    NULL,
    0,
    NULL,
    &aali_validate,
  },

  {
    /* Q_DALI - DELETE_ALIAS */
    "delete_alias",
    "dali",
    2,
    DELETE,
    "a",
    ALIAS_TABLE,
    NULL,
    aali_fields,
    0,
    "name = '%s' AND type = '%s' AND  trans = '%s'",
    3,
    NULL,
    &dali_validate,
  },

  {
    /* Q_GVAL - GET_VALUE */
    "get_value",
    "gval",
    2,
    RETRIEVE,
    "val",
    NUMVALUES_TABLE,
    "value FROM numvalues",
    gval_fields,
    1,
    "name = '%s'",
    1,
    NULL,
    &gval_validate,
  },

  {
    /* Q_AVAL - ADD_VALUE */
    "add_value",
    "aval",
    2,
    APPEND,
    "val",
    NUMVALUES_TABLE,
    "INTO numvalues (name, value) VALUES ('%s', %s)",
    aval_fields,
    2,
    NULL,
    0,
    NULL,
    &aval_validate,
  },

  {
    /* Q_UVAL - UPDATE_VALUE */
    "update_value",
    "uval",
    2,
    UPDATE,
    "val",
    NUMVALUES_TABLE,
    "numvalues SET value = %s",
    aval_fields,
    1,
    "name = '%s'",
    1,
    NULL,
    &aval_validate,
  },

  {
    /* Q_DVAL - DELETE_VALUE */
    "delete_value",
    "dval",
    2,
    DELETE,
    "val",
    NUMVALUES_TABLE,
    NULL,
    dval_fields,
    0,
    "name = '%s'",
    1,
    NULL,
    &aval_validate,
  },

  {
    /* Q_GATS - GET_ALL_TABLE_STATS */
    "get_all_table_stats",
    "gats",
    2,
    RETRIEVE,
    "tbs",
    TBLSTATS_TABLE,
    "table_name, appends, updates, deletes, TO_CHAR(modtime, 'DD-mon-YYYY HH24:MI:SS') FROM tblstats",
    gats_fields,
    5,
    NULL,
    0,
    "table_name",
    0,
  },

  {
    /* Q__SDL - _SET_DEBUG_LEVEL */
    "_set_debug_level",
    "_sdl",
    2,
    UPDATE,
    NULL,
    0,
    NULL,
    _sdl_fields,
    1,
    NULL,
    0,
    NULL,
    &_sdl_validate,
  },

};

int QueryCount = (sizeof(Queries) / sizeof(struct query));
