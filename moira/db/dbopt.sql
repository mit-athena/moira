create unique index i_usr_usid on users (users_id);
create index i_usr_login on users (login);
create index i_usr_name on  users (first, last);
create index i_usr_last on  users (last, first);
create index i_usr_uid on  users (unix_uid);
create index i_usr_pop on  users (pop_id);
create index i_usr_imap on users (imap_id);
create index i_usr_exchange on users (exchange_id);
create index i_usr_clearid on users (clearid);
create index i_usr_resv on users (reservations);
create index i_usr_sponsor on users (sponsor_id);
create index i_usr_vpn_group on users (default_vpn_group);
create index i_usr_twofactor on users (twofactor_status);

create unique index i_krb_usid on krbmap  (users_id);
create index i_krb_str on  krbmap (string_id);

create unique index i_mac_mid on machine  (mach_id);
create index i_mac_name on  machine (name);
create index i_mac_loc on machine (location);
create index i_mac_owner on machine (owner_id);
create index i_mac_acctnumber on machine(account_number);

create unique index i_hal_name on hostalias  (name);
create index i_hal_id on hostalias (mach_id);

create index i_haddr_id on hostaddress (mach_id);
create index i_haddr_snet on hostaddress (snet_id);
create index i_haddr_addr on hostaddress (address);

create index i_hrec_id on hostrecord (mach_id);
create index i_hec_value on hostrecord (rr_value);

create unique index i_snt_sid on subnet  (snet_id);
create index i_snt_name on subnet (name);
create index i_snt_saddr on subnet(saddr);
create index i_snt_vlan on subnet (vlan_tag);

create unique index i_clu_clid on clusters (clu_id);
create index i_clu_name on  clusters (name);

create index i_mcm_clid on mcmap (clu_id);
create index i_mcm_mach on  mcmap (mach_id);

create index i_svc_clid on svc (clu_id);

create index i_srv_name on servers (name);

create index i_sho_srv on serverhosts (service);
create index i_sho_mach on  serverhosts (mach_id);

create unique index i_lis_lid on list  (list_id);
create index i_lis_name on  list (name);
create index i_lis_mail on  list (maillist);
create index i_lis_group on  list (grouplist);
create index i_lis_gid on  list (gid);
create index i_lis_nfsgroup on list (nfsgroup);
create index i_lis_acl on list (acl_id);
create index i_lis_memacl on list (memacl_id);

create index i_imem_mid on imembers (member_id, member_type);
create index i_imem_list on  imembers (list_id);

create unique index i_str_sid on strings  (string_id);
create unique index i_str_name on  strings (string);

create unique index i_nfs_nid on nfsphys  (nfsphys_id);
create index i_nfs_mach on  nfsphys (mach_id);

create unique index i_fil_fid on filesys  (filsys_id);
create index i_fil_name on  filesys (label);
create index i_fil_mach on  filesys (mach_id);
create index i_fil_phys on  filesys (phys_id);
create index i_fil_grp on  filesys (owners);
create index i_fil_user on  filesys (owner);

create index i_fsg_gid on fsgroup (group_id);
create index i_fsg_filsys on  fsgroup (filsys_id);

create index i_qot_eid on quota (entity_id);
create index i_qot_fil on  quota (filsys_id);
create index i_qot_phys on  quota (phys_id);

create unique index i_zph_class on zephyr  (class);

create unique index i_hac_mid on hostaccess  (mach_id);

create index i_acl_mt on acl (mach_id, target);

create index i_prn_name on printers (name);
create index i_prn_dname on printers (duplexname);
create index i_prn_mach on printers (mach_id);
create index i_prn_rm on printers (rm);
create index i_prn_loc on printers (location);
create index i_prn_con on printers (contact);

create index i_cap_tag on capacls (tag);

create unique index i_containers_cntid on containers (cnt_id);
create unique index i_containers_cname on containers (name);

create unique index i_mcntmap_machid on mcntmap (mach_id);
create index i_mcntmap_cntid on mcntmap (cnt_id);

create index i_machidentifiermap_machid on machidentifiermap (mach_id);
create index i_machidentifiermap_type on machidentifiermap (mach_identifier_type);
create index i_machidentifiermap_value on machidentifiermap (mach_identifier);

create index i_incr_queue_ts on incremental_queue (timestamp);
create index i_incr_queue_serv on incremental_queue (service);
create index i_incr_queue_tsserv on incremental_queue (timestamp, service);
create sequence incremental_seq increment by 1 start with 1 minvalue 1 maxvalue 2147483648 cycle nocache;
