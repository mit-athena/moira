% lists
qy dlis testlist
qy dlis testl
qy dlis testlst

qy _help glin
% qy glin *
qy glin mar
qy glin no-such-list

qy _help exln
% qy exln *
qy exln mar
qy exln no-such-list
qy exln m*r

qy _help alis
qy alis testlist 1 0 0 0 0 "create unique GID" user mar desc
qy glin testlist
qy alis testlist 1 0 0 0 0 "create unique GID" user mar desc
qy alis testl 1 1 1 1 1 1234 list testl desc
qy glin testl
qy alis tst 0 0 0 0 0 0 bad-type mar desc
qy alis tst 0 0 0 0 0 0 user no-such-user desc
qy alis tst 0 0 0 0 0 0 list no-such-list desc

qy _help ulis
qy ulis testlist testlist 1 0 0 1 0 "create unique GID" user mar description
qy glin testlist
qy ulis testlist testlst 1 0 0 1 1 "create unique GID" user mar description
qy glin testl*
qy ulis no-such-list testlst 1 0 0 1 1 "create unique GID" user mar desc
qy ulis testlst dbadmin 1 0 0 1 1 "create unique GID" user mar description
qy ulis testlst tst 1 0 0 1 1 "create unique GID" bad-type mar description

qy _help dlis
qy dlis testlst
qy dlis testlst
qy dlis dbadmin
qy dlis *

qy _help amtl
qy amtl testl user mar
qy amtl testl list testl
qy amtl testl string "a random string (with * stars)"
qy amtl testl user mar
qy amtl testl user default
qy gmol testl
qy amtl no-such-list user mar
qy amtl testl user no-such-user
qy amtl testl list no-such-list

qy _help dmfl
qy dmfl testl user default
qy dmfl testl user default
qy dmfl testl user no-such-user
qy dmfl no-such-list user mar
qy dmfl testl list no-such-list
qy dmfl testl string foobar
qy gmol

qy _help gaus
qy gaus user mar
qy gaus ruser gamit
qy gaus user no-such-user
qy gaus list testl
qy gaus list no-such-list
qy gaus rlist test
qy gaus no-such-type mar

qy _help qgli
qy qgli false false false false false
qy qgli true true true true true
% qy qgli dontcare dontcare dontcare dontcare dontcare
qy qgli true dontcare dontcare true false
qy qgli dontcare dontcare dontcare dontcare no-such-bool
qy qgli dontcare dontcare dontcare no-such-bool dontcare
qy qgli dontcare dontcare no-such-bool dontcare dontcare
qy qgli dontcare no-such-bool dontcare dontcare dontcare
qy qgli no-such-bool dontcare dontcare dontcare dontcare

qy _help gmol
qy gmol testl
qy gmol no-such-list
qy gmol *

qy _help glom
qy glom user mar
qy glom ruser mar
qy glom list test
qy glom rlist test
qy glom user no-such-user
qy glom list no-such-list
qy glom string "a random string (with * stars)"
qy glom bad-type foo
qy glom user *

qy _help cmol
qy cmol test
qy cmol testl
qy cmol no-such-list
qy cmol default
qy cmol *
