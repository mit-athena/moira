% Filesystems
qy dfil test
qy dfil testing

qy _help gfsl
%qy gfsl *
qy gfsl mar
qy gfsl no-such-fs

qy _help gfsm
%qy gfsm *
qy gfsm toto.mit.edu
qy gfsm no-such-mach

qy _help gfsn
%qy gfsn * *
qy gfsn kiwi.mit.edu *
qy gfsn * /site
qy gfsn kiwi.mit.edu /site
qy gfsn no-such-mach /site
qy gfsn kiwi.mit.edu no-such-dir

qy _help gfsg
%qy gfsg *
qy gfsg mar
qy gfsg no-such-list

qy _help afil
qy afil test rvd kiwi.mit.edu pack mount r comments mar mar 0 other
qy afil test rvd kiwi.mit.edu pack mount r comments mar mar 0 other
qy afil tset rvd no-such-mach pack mount r comments mar mar 0 other
qy afil tset rvd kiwi.mit.edu pack mount h comments mar mar 0 other
qy afil tset rvd kiwi.mit.edu pack mount r comments no-user mar 0 other
qy afil tset rvd kiwi.mit.edu pack mount r comments mar no-list 0 other
qy afil tset rvd kiwi.mit.edu pack mount r comments mar mar 0 no-locker
qy afil testing nfs kiwi.mit.edu /site/test mount w none mar mar 1 homedir
qy afil testing nfs kiwi.mit.edu /site/test mount w none mar mar 1 homedir
qy afil tset nfs no-such-mach /site/test mount w none mar mar 1 homedir
qy afil tset nfs kiwi.mit.edu /bin/test mount w none mar mar 1 homedir
qy afil tset nfs kiwi.mit.edu /site/test mount h none mar mar 1 homedir
qy afil tset bad-type kiwi.mit.edu /site/test mount h none mar mar 1 other
qy afil tsetse afs \[NONE\] /afs/athena.mit.edu/astaff/project/longname /mit/longname r "" mar mar 0 other

qy _help ufil
qy ufil test tested rvd kiwi.mit.edu pack mount r comments mar mar 0 other
qy gfsl test*
qy ufil tested testing rvd kiwi.mit.edu pack mount r comments mar mar 0 other
qy ufil tested tested rvd kiwi.mit.edu pack mount r comments mar mar 0 other
qy ufil tested tested badtype kiwi.mit.edu pack mount r comments mar mar 0 other
qy ufil tested tested rvd no-such-mach pack mount r comments mar mar 0 other
qy ufil tested tested rvd kiwi.mit.edu pack mount h comments mar mar 0 other
qy ufil tested tested rvd kiwi.mit.edu pack mount r comments no-user mar 0 other
qy ufil tested tested rvd kiwi.mit.edu pack mount r comments mar no-list 0 other
qy ufil tested tested rvd kiwi.mit.edu pack mount r comments mar mar 0 bad-type
qy ufil testing testing nfs kiwi.mit.edu /site/test /mit/test r "" mar mar 1 project

qy _help dfil
qy dfil *
qy dfil no-such-fs
qy dfil tested
qy dfil tsetse

qy _help ganf
qy ganf

qy _help gnfp
%qy gnfp * *
qy gnfp kiwi.mit.edu *
qy gnfp * /site
qy gnfp kiwi.mit.edu /site
qy gnfp no-such-mach /site
qy gnfp kiwi.mit.edu /bin

qy _help anfp
qy anfp kiwi.mit.edu /u1 /dev/ra1c 0 0 1000
qy anfp kiwi.mit.edu /u1 /dev/ra1c 0 0 1000
qy anfp no-such-mach /u1 /dev/ra1c 0 0 1000
qy anfp kiwi.mit.edu /u1 /dev/ra1c bad 0 1000

qy _help unfp
qy unfp kiwi.mit.edu /u1 /dev/ra1c 1 100 2000
qy gnfp kiwi.mit.edu /u1

qy _help ajnf
qy ajnf kiwi.mit.edu /u1 -666
qy gnfp kiwi.mit.edu /u1
qy ajnf no-such-mach /u1 0
qy ajnf kiwi.mit.edu /bin 100
qy ajnf e40-po.mit.edu /site 50
qy ajnf * * 1
qy ajnf kiwi.mit.edu * 2
qy ajnf * /site 3

qy _help dnfp
qy dnfp kiwi.mit.edu /u1
qy dnfp * /site
qy dnfp kiwi.mit.edu *
qy dnfp kiwi.mit.edu /site
qy dnfp no-such-mach /site

qy _help gnfq
%qy gnfq * *
qy gnfq * mar
qy gnfq test *
qy gnfq mar mar
qy gnfq no-such-fs mar

qy _help gnqp
qy gnqp kiwi.mit.edu /site
qy gnqp * /site
qy gnqp kiwi.mit.edu *
qy gnqp no-such-mach /site

qy _help anfq
qy gnfp kiwi.mit.edu /site
qy anfq testing mar 500
gy gnfp kiwi.mit.edu /site
qy anfq testing mar 500
qy anfq no-such-fs mar 500
qy anfq testing no-such-user 500
qy anfq testing mar no-number
qy anfq * mar 500
qy gnfq * mar

qy _help unfq
qy unfq testing mar 400
qy gnfp kiwi.mit.edu /site
qy unfq no-such-fs mar 45400
qy unfq poto mar 500
qy unfq poto no-such-user 5

qy _help dnfq
qy dnfq testing mar
qy gnfq testing mar
qy dnfq poto qjb
qy dnfq no-such-fs mar
qy dnfq testing no-such-user
qy dnfq testing poto
