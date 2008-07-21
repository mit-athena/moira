package mit.moira;

public class Moira {
    boolean connected = false;
    boolean isauth = false;
    String server = "";
    boolean isValid = true;

    protected static Object LOCK = new Object();
    protected static boolean isAvailable = true;

    public static Moira getInstance(String server) {
	synchronized (LOCK) {
	    while (!isAvailable) {
		try {
		    LOCK.wait();
		} catch (InterruptedException ie) {
		}
	    }
	    isAvailable = false;
	    return new Moira(server);
	}
    }
		
    protected static void returnInstance() {
	synchronized(LOCK) {
	    isAvailable = true;
	    LOCK.notify();
	}
    }

    public synchronized void done() {
	if (!isValid) return;	// Don't make noise...
	try {
	    disconnect();
	} catch (MoiraException m) {
	}
	isValid = false;
	returnInstance();
    }

    protected Moira() {
    }

    protected Moira(String server) {
	this.server = server;
    }

    public synchronized void connect() throws MoiraException {
	if (!isValid) throw new MoiraException("Attempt to use stale Moira Object");
	if (!connected) {
	    MoiraConnectInternal.connect(server);
	    connected = true;
	}
    }
	    
    public synchronized void auth() throws MoiraException {
	if (!isValid) throw new MoiraException("Attempt to use stale Moira Object");
	if (!isauth) {
	    synchronized(LOCK) {
		MoiraConnectInternal.auth();
	    }
	    isauth = true;
	}
    }

    public synchronized void proxy(String user) throws MoiraException {
	if (!isValid) throw new MoiraException("Attempt to use stale Moira Object");
	MoiraConnectInternal.proxy(user);
    }
	    
    public synchronized void disconnect() throws MoiraException {
	if (!isValid) throw new MoiraException("Attempt to use stale Moira Object");
	if (connected) {
	    MoiraConnectInternal.disconnect();
	    connected = false;
	    isauth = false;
	}
    }

    public synchronized ListInfo get_list_info(String list) throws MoiraException {
	if (!isValid) throw new MoiraException("Attempt to use stale Moira Object");
	if (!connected) throw new MoiraException("Not Connected");
	String [] args = new String[1];
	args[0] = list;
	Object [] retinternal = MoiraConnectInternal.mr_query("get_list_info", args);
	if (retinternal == null) return (null);
	if (retinternal.length == 0) return (null);
	if (retinternal.length != 1) throw new MoiraException("get_list_info returned more then one list!");
	String [] entry = (String []) retinternal[0];
	return (new ListInfo(entry[0], entry[1], entry[2], entry[3], entry[4], entry[5], entry[6], entry[7], entry[8], entry[9], entry[10], entry[11], entry[12]));
    }

    public synchronized void update_list_info(String list, ListInfo info) throws MoiraException {
	if (!isValid) throw new MoiraException("Attempt to use stale Moira Object");
	if (!connected) throw new MoiraException("Not Connected");
	String [] args = new String[11];
	args[0] = list;
	args[1] = info.name;
	args[2] = info.active ? "1" : "0";
	args[3] = info.bpublic ? "1" : "0";
	args[4] = info.hidden ? "1" : "0";
	args[5] = info.maillist ? "1" : "0";
	args[6] = info.grouplist ? "1" : "0";
	args[7] = info.gid_original;
	args[8] = info.ace_type;
	args[9] = info.ace_name;
	args[10] = info.description;
	Object [] retinternal = MoiraConnectInternal.mr_query("update_list", args);
	return;
    }

    public synchronized void delete_member_from_list(String list, String type, String member) throws MoiraException {
	if (!isValid) throw new MoiraException("Attempt to use stale Moira Object");
	if (!connected) throw new MoiraException("Not Connected");
	String [] args = new String[3];
	args[0] = list;
	args[1] = type;
	args[2] = member;
	Object [] retinternal = MoiraConnectInternal.mr_query("delete_member_from_list", args);
	return;
    }

    public synchronized void add_member_to_list(String list, String type, String member) throws MoiraException {
	if (!isValid) throw new MoiraException("Attempt to use stale Moira Object");
	if (!connected) throw new MoiraException("Not Connected");
	String [] args = new String[3];
	args[0] = list;
	args[1] = type;
	args[2] = member;
	Object [] retinternal = MoiraConnectInternal.mr_query("add_member_to_list", args);
	return;
    }

    public synchronized String [] get_user_by_login(String user) throws MoiraException {
	if (!isValid) throw new MoiraException("Attempt to use stale Moira Object");
	if (!connected) throw new MoiraException("Not Connected");
	String [] args = new String[1];
	args[0] = user;
	Object [] retinternal = MoiraConnectInternal.mr_query("get_user_by_login", args);
	if (retinternal == null) return (null);
	String [] entry = (String []) retinternal[0];
	return(entry);
    }

    public synchronized Member [] get_members_of_list(String list) throws MoiraException {
	if (!isValid) throw new MoiraException("Attempt to use stale Moira Object");
	if (!connected) throw new MoiraException("Not Connected");
	String [] args = new String[1];
	args[0] = list;
	Object [] retinternal = MoiraConnectInternal.mr_query("get_members_of_list", args);
	if (retinternal == null) return (null);
	Member [] retval = new Member[retinternal.length];
	for (int i = 0; i < retinternal.length; i++) {
	    String [] entry = (String []) retinternal[i];
	    retval[i] = new Member(entry[0], entry[1]);
	}
	return (retval);
    }

    public void finalize() {
	if (isValid) System.err.println("Moira object finalized while valid!");
	else System.err.println("Moira object finalized (normally)");
    }

}

class MoiraConnectInternal {
    static native void connect(String server) throws MoiraException;
    static native void auth() throws MoiraException;
    static native void proxy(String user) throws MoiraException;
    static native void disconnect();
    static native Object [] mr_query(String command, String [] args) throws MoiraException;
    static {
	System.loadLibrary("moirajava");
    }
}
