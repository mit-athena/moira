/*
 * Run the salvager after doing this!! 
 */

main()
{

##	ingres sms
##	range of u is users
##	range of f is filesys
##	range of p is pobox
##	range of l is list
##	range of nq is nfsquota

##	replace u (status = 0, login="#1003") where u.login = "tau"
	
##	delete f where f.label = "tau"
##	delete p where p.box = "tau"
## 	delete l where l.name = "tau"
##	delete nq where nq.users_id = 14651

}
