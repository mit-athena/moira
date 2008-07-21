% Zephyr queries
qy dzcl test

qy _help gzcl
#qy gzcl *
qy gzcl login
qy gzcl no-such-class

qy _help azcl
qy azcl test user mar list mar none none list test
qy azcl test user mar list mar none none list test
qy azcl testing user bad-user list mar none none list test
qy azcl testing user mar list bad-list none none list test
qy azcl testing user mar list mar none random-none list test
qy azcl testing user mar list mar none none bad-type test
qy azcl name*star user mar list mar none none list test

qy _help uzcl
qy gzcl test*
qy uzcl test test list test list dbadmin list poto list kit
qy gzcl test*
qy uzcl test testing user mar user qjb user kit user poto
qy gzcl test*
qy uzcl testing login user mar user qjb user kit none poto
qy uzcl testing tested user no-such-user user qjb user kit user poto
qy uzcl testing tested user mar list no-such-list user kit user poto
qy uzcl testing tested user mar user qjb none none bad-type poto

qy _help dzcl
qy dzcl testing
qy dzcl test
qy dzcl *

