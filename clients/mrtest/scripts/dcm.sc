% DCM Stuff
qy dshi test kiwi.mit.edu
qy dsin test

qy _help gsin
qy gsin *
qy gsin nfs
qy gsin no-such-service

qy _help qgsv
qy qgsv true true true
qy qgsv false false false
qy qgsv dontcare dontcare dontcare
qy qgsv dontcare dontcare bad-type
qy qgsv dontcare bad-type dontcare
qy qgsv bad-type dontcare dontcare

qy _help asin
qy asin test 1 target script unique 1 user mar
qy asin test 1 target script unique 1 user mar
qy gsin test
qy asin testing 1 target script bad-type 1 user mar
qy asin testing 1 target script unique 1 bad-acl mar
qy asin testing 1 target script unique 1 user bad-user

qy _help usin
qy usin test 2 trgt scrpt replicated 0 list dbadmin
qy gsin test
qy usin test 60 trgt scrpt replicated 0 list dbadmin
qy gsin test*
qy usin test 60 t s bad-type 1 list dbadmin
qy usin test 60 t s unique 1 bad-acl dbadmin
qy usin test 60 t s unique 1 list bad-list

qy _help rsve
qy rsve *
qy ssif test now 0 1 msg
qy gsin test
qy rsve test
qy gsin test
qy rsve test
qy rsve no-such-service

qy _help ssif
qy ssif test now 1 1 msg
qy ssif test now 1 1 msg
qy ssif test now 0 0 ""
qy ssif test bad-date 0 0 foo
qy ssif no-such-service now 1 1 msg
qy ssif * now 1 1 msg

qy _help dsin
qy dsin nfs
qy dsin *
qy dsin test
qy dsin no-such-service
qy asin test 1 target script unique 1 user mar

qy _help gshi
qy gshi * *
qy gshi nfs *
qy gshi * kiwi.mit.edu
qy gshi nfs kiwi.mit.edu
qy gshi nfs e40-po.mit.edu
qy gshi pop kiwi.mit.edu
qy gshi no-service *
qy gshi * no-host

qy _help qgsh
qy qgsh * dontcare dontcare dontcare dontcare dontcare
qy qgsh test dontcare dontcare dontcare dontcare dontcare
qy qgsh nfs true true true true true
qy qgsh hesiod false false false false false
qy qgsh no-service dontcare dontcare dontcare dontcare dontcare
qy qgsh * dontcare dontcare dontcare dontcare bad-bool
qy qgsh * dontcare dontcare dontcare bad-bool dontcare
qy qgsh * dontcare dontcare bad-bool dontcare dontcare
qy qgsh * dontcare bad-bool dontcare dontcare dontcare
qy qgsh * bad-bool dontcare dontcare dontcare dontcare

qy _help ashi
qy ashi test kiwi.mit.edu 1 1 2 list
qy gshi test kiwi.mit.edu
qy ashi test kiwi.mit.edu 1 1 2 list
qy ashi no-service kiwi.mit.edu 1 1 2 list
qy ashi test no-such-host 1 1 2 list
qy ashi * kiwi.mit.edu 1 1 2 list
qy ashi test * 1 2 3 foo

qy _help ushi
qy ushi test kiwi.mit.edu 0 42 56 ""
qy gshi test kiwi.mit.edu
qy ushi no-service kiwi.mit.edu 0 1 2 3
qy uwhi test no-such-host 0 1 2 3

qy _help rshe
qy sshi test kiwi.mit.edu 0 0 0 1 msg now now
qy gshi test kiwi.mit.edu
qy rshe test kiwi.mit.edu
qy gshi test kiwi.mit.edu
qy rshe test kiwi.mit.edu
qy rshe no-service kiwi.mit.edu
qy rshe test no-such-host
qy rshe test *
qy rshe * kiwi.mit.edu

qy _help ssho
qy sshi test kiwi.mit.edu 0 0 0 0 "" now now
qy gshi test kiwi.mit.edu
qy ssho test kiwi.mit.edu
qy gshi test kiwi.mit.edu
qy ssho test kiwi.mit.edu
qy ssho no-service kiwi.mit.edu
qy ssho test no-such-host
qy ssho test *
qy ssho * kiwi.mit.edu

qy _help sshi
qy gshi test kiwi.mit.edu
qy sshi test kiwi.mit.edu 0 1 1 42 msg now now
qy gshi test kiwi.mit.edu
qy sshi no-service kiwi.mit.edu 0 1 1 42 msg now now
qy sshi test no-such-host 0 1 1 42 msg now now
qy sshi test kiwi.mit.edu 0 1 1 42 msg bad-date now
qy sshi test kiwi.mit.edu 0 1 1 42 msg now bad-date

qy _help dshi
qy dsin test
qy dshi test kiwi.mit.edu
qy dshi no-service kiwi.mit.edu
qy dshi test no-such-host
qy dshi test *
qy dshi * kiwi.mit.edu
qy dsin test

qy _help gslo
qy gslo *
qy gslo nfs
qy gslo no-such-service

