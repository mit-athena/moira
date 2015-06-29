% test user queries

qy _help galo
%qy galo
qy _help gaal
%qy gaal

qy _help gubl
%qy gubl *
qy gubl root
qy gubl no-one
qy gubl mar

qy _help gubu
qy gubu 17822
qy gubu 0
qy gubu 32777

qy _help gubn
%qy gubn * *
qy gubn Jesus *
qy gubn * Rosenstein
qy gubn Mark Rosenstein
qy gubn No One

qy _help gubc
qy gubc guest
qy gubc none
%qy gubc *

qy _help gubm
%qy gubm *
qy gubm mitid
qy gubm qwerty

qy _help ausr
qy dusr testing
qy dusr tested
qy ausr testing 4321 /bin/csh Last First M 0 mitid staff
qy ausr testing 1234 /bin/csh Last First M 0 mitid staff
qy ausr tested  1234 /bin/csh Last First M 0 mitid notone
qy ausr tested  1234 /bin/csh Last First M 0 mitid staff
qy ausr "create unique login ID" "create unique UID" /bin/sh TestL TestF TestM 0 mitid staff
qy gubn TestF TestL

qy _help rusr
qy rusr 1234 root 1
qy rusr 1234 tested 1
qy gubu 1234

qy _help uusr
qy uusr tested tested 4321 /bin/csh Last First MI 0 MITID guest
qy gubl tested
qy uusr tested root 4321 /bin/csh Last First MI 0 MITID guest
qy uusr tested testnew 4321 /bin/csh Last First MI 0 MITID guest
qy gubl tested
qy gubl testnew
qy uusr testnew tested 1234 /bin/csh Last First MI 0 MITID guest
qy gubl tested

qy _help uush
qy gubl tested
qy uush tested /bin/bash
qy gubl tested

qy _help uust
qy uust tested 1
qy gubl tested

qy _help dusr
qy dusr *
qy dusr testing
qy dusr mar
qy dusr no-one

qy _help dubu
qy dubu 1234
qy dubu 32777

qy _help gkum
qy gkum * *
qy gkum no-one *
qy gkum * no-kerberos
qy gkum mar *
qy gkum * mar@ATHENA.MIT.EDU

qy _help akum
qy akum testuser testuser@ATHENA.MIT.EDU
qy akum testuser testuser.root@ATHENA.MIT.EDU
qy akum root testuser@ATHENA.MIT.EDU
qy akum no-one foo.bar@ATHENA.MIT.EDU
qy akum * *

qy _help dkum
qy dkum testuser testuser@ATHENA.MIT.EDU
qy dkum mar foo.bar@ATHENA.MIT.EDU
qy dkum no-one foo.bar@ATHENA.MIT.EDU

% finger
qy ausr testing 4321 /bin/csh Last First M 0 mitid staff
qy _help gfbl
qy gfbl testing
%qy gfbl *
qy gfbl no-one

qy _help ufbl
qy ufbl testing "Mr. Testing" nn ha hp oa op d STAFF
qy gfbl testing

% PoBoxes
qy _help gpob
qy gpob mar
qy gpob testing
%qy gpob *
qy gpob no-one

qy _help gapo
%qy gapo
qy _help gpop
%qy gpop
qy _help gpos
%qy gpos

qy _help spob
qy spob testing no-type box
qy spob testing pop no-mach
qy spob testing pop e40-po.mit.edu
qy gpob testing
qy spob testing smtp "a random string (with * wildcards)"
qy gpob testing
qy spob testing none foo
qy gpob testing

qy _help spop
qy spop testing
qy gpob testing

qy _help dpob
qy dpob testing
qy gpob testing
qy dpob testing

