modify users to cbtree unique on users_id
CREATE INDEX i_usr_login ON users (login) 
modify i_usr_login to btree

CREATE INDEX i_usr_name ON users (first, last)
modify i_usr_name to cbtree on first, last

CREATE INDEX i_usr_last ON users (last, first)
modify i_usr_last to cbtree on last, first

CREATE INDEX i_usr_uid ON users (uid)
modify i_usr_uid to btree
CREATE INDEX i_usr_pop ON users (pop_id)
modify i_usr_pop to btree

modify krbmap to btree unique on users_id
CREATE INDEX i_krb_str ON krbmap  (string_id)
modify i_krb_str to btree

modify machine to cbtree unique on mach_id
CREATE INDEX i_mac_name ON machine (name)
modify i_mac_name to btree
 
modify cluster to cbtree unique on clu_id
CREATE INDEX i_clu_name ON cluster (name)
modify i_clu_name to btree

modify mcmap to cbtree on clu_id
CREATE INDEX i_mcm_mach ON mcmap (mach_id)
modify i_mcm_mach to cbtree

modify svc to cbtree on clu_id

modify servers to btree on name

modify serverhosts to btree on service
CREATE INDEX i_sho_mach ON serverhosts (mach_id)
modify i_sho_mach to cbtree

modify list to cbtree unique on list_id
CREATE INDEX i_lis_name ON list(name)
modify i_lis_name to btree
CREATE INDEX i_lis_mail ON list (maillist)
CREATE INDEX i_lis_group ON list (group)
CREATE INDEX i_lis_gid ON list (gid)
modify i_lis_gid to btree

modify imembers to cbtree on member_id, member_type
CREATE INDEX i_imem_list ON imembers (list_id)
modify i_imem_list to hash

modify strings to cbtree unique on string_id
CREATE INDEX i_str_name ON strings (string)
modify i_str_name to hash

modify nfsphys to cbtree unique on nfsphys_id
CREATE INDEX i_nfs_mach ON nfsphys (mach_id)
modify i_nfs_mach to btree

modify filesys to cbtree unique on filsys_id
CREATE INDEX i_fil_name ON filesys (label)
modify i_fil_name to cbtree
CREATE INDEX i_fil_mach ON filesys (mach_id)
modify i_fil_mach to btree
CREATE INDEX i_fil_phys ON filesys (phys_id)
modify i_fil_phys to cbtree
CREATE INDEX i_fil_grp ON filesys (owners)
modify i_fil_grp to btree
CREATE INDEX i_fil_user ON filesys (owner)
modify i_fil_user to btree

modify fsgroup to cbtree on group_id
CREATE INDEX i_fsg_filsys ON fsgroup (filsys_id)
modify i_fsg_filsys to btree

modify quota to cbtree on entity_id
CREATE INDEX i_qot_fil ON quota (filsys_id)
modify i_qot_fil to cbtree
CREATE INDEX i_qot_phys ON quota (phys_id)
modify i_qot_phys to cbtree

modify zephyr to cbtree unique on class

modify hostaccess to cbtree unique on mach_id

modify printcap to cbtree on name
CREATE INDEX i_pcp_mach ON printcap (mach_id)
modify i_pcp_mach to btree

modify palladium to cbtree on name

modify capacls to cbtree on tag
