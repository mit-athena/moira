#define WhoIsThis(call,a,id) \
	((*(id) = ((call)?(int)(call):SYSADMINID)), 0)
#define WhoIsThisWithName(call,a,id,name) \
	((*(id) = ((call)?(int)(call):SYSADMINID)), 0)

#include <afs/ptserver.h>
