% Miscellaneous stuff

qy _help gsha
#qy gsha *
qy gsha toto.mit.edu
qy gsha no-such-mach

qy _help asha
qy asha e40-po.mit.edu list dbadmin
qy asha e40-po.mit.edu list dbadmin
qy asha kiwi.mit.edu user mar
qy asha soup.mit.edu none none
qy asha * list dbadmin
qy asha soup.mit.edu bad-type foo

qy _help usha
qy usha e40-po.mit.edu user qjb
qy gsha e40-po.mit.edu
qy usha e40-po.mit.edu bad-type foo

qy _help dsha
qy dsha e40-po.mit.edu
qy dsha *
qy dsha no-such-mach

qy _help gpcp
#qy gpcp *
qy gpcp nil
qy gpcp no-such-printer

qy _help apcp
qy apcp test test:rm=foo.mit.edu:rp=test:
qy apcp test test:rm=foo.mit.edu:rp=test:
qy apcp nil duplicate
qy apcp testing "this one has spaces * and stars"
qy gpcp test*

qy _help dpcp
qy dpcp testing
qy gpcp test
qy dpcp *
qy dpcp no-such-printer

qy _help gpdm
qy gpdm *
qy gpdm dexter
qy gpdm no-such-printer

qy _help apdm
qy apdm test 123 PARIS.MIT.EDU
qy gpdm test
qy apdm test 456 MINOS.MIT.EDU
qy apdm testing 123 no-machine

qy _help dpdm
qy dpdm test
qy gpdm test
qy dpdm no-such-printer

qy _help gali
#qy gali * * *
qy gali * TYPEDATA *
qy gali filesys TYPE *
qy gali filesys TYPE NFS
qy gali foo * *

qy _help aali
qy aali foo TYPE baz
qy aali alias TYPE foo
qy aali foo TYPE baz
qy aali foo TYPE baz
qy aali foo TYPE bar
qy gali foo TYPE *

qy _help dali
qy dali foo TYPE *
qy dali foo TYPE baz
qy dali foo TYPE bar
qy dali alias TYPE foo
qy dali foo foo foo

qy _help gval
qy gval *
qy gval users_id
qy gval no-such-value

qy _help aval
qy aval testval 5
qy aval users_id 3
qy aval * 1
qy aval newval asdf
qy gval testval

qy _help uval
qy uval testval 7
qy uval * 8
qy uval no-such-val 9
qy gval testval

qy _help dval
qy dval testval
qy dval *
qy dval no-such-val

qy _help gats
qy gats
