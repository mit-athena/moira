% Machines

qy dmac test.mit.edu
qy dmac testing.mit.edu

query _help gmac
%qy gmac *
qy gmac no-such-host
qy gmac toto.mit.edu
qy gmac TOTO.MIT.EDU

query _help amac
qy amac test.mit.edu vax
qy amac test.mit.edu vax
qy amac testing.mit.edu no-type
qy gmac test*

query _help umac
qy umac test.mit.edu test.mit.edu rt
qy gmac test.mit.edu
qy umac test.mit.edu test.mit.edu no-type
qy umac test.mit.edu testing.mit.edu rt
qy umac testing.mit.edu toto.mit.edu rt
qy umac no-such-host newhost vax
qy gmac test*

query _help dmac
qy dmac testing.mit.edu
qy gmac testing.mit.edu
qy dmac toto.mit.edu
qy dmac no-such-host


% clusters

qy dclu testclu
qy dclu testing

query _help gclu
%qy gclu *
qy gclu no-such-clu
qy gclu bldge40-vs

query _help aclu
qy aclu testclu desc loc
qy aclu testclu desc loc
qy gclu testclu

query _help uclu
qy uclu testclu testclu newdesc newloc
qy gclu testclu
qy uclu testclu testing desc loc
qy uclu testing bldge40-vs desc loc
qy gclu test*

query _help dclu
qy dclu testing
qy dclu testing
qy dclu bldge40-vs


% Mappings

qy aclu testclu desc loc
qy amac test.mit.edu vax

qy _help gmcm
%qy gmcm * *
qy gmcm toto.mit.edu *
qy gmcm * bldge40-vs
qy gmcm no-such-host no-such-clu
qy gmcm toto.mit.edu bldge40-rt

qy _help amtc
qy amtc test.mit.edu testclu
qy amtc test.mit.edu testclu
qy amtc test.mit.edu no-such-clu
qy amtc no-such-host testclu
qy gmcm test* test*
qy amtc test.mit.edu bldge40-vs
qy amtc toto.mit.edu testclu
qy gmcm test* *
qy gmcm * test*

qy _help dmfc
qy dmfc * *
qy dmfc test.mit.edu *
qy dmfc * testclu
qy dmfc test.mit.edu bldge40-rt
qy dmfc test.mit.edu testclu

% Cluster data

qy _help gcld
%qy gcld * *
qy gcld testclu *
qy gcld no-such-clu *
qy gclu bldge40-vs *

qy _help acld
qy acld testclu LPR data
qy acld testclu LPR data1
qy acld testclu LPR data
qy acld testclu no-label data
qy acld no-such-clu LPR data

qy _help dcld
qy dcld * lpr data
qy dcld testclu * data
qy dcld testclu lpr *
qy dcld testclu LPR data
qy dcld testclu LPR data
qy dcld no-such-clu FOO data
qy dcld testclu no-label data

