package mit.moira;

public class MoiraConnect {
    boolean connected = false;
    boolean isauth = false;
    Object LOCK;

    String server = "";
    MoiraConnect() {
    }

    public MoiraConnect(String server, Object lock) {
	this.server = server;
	this.LOCK = lock;
    }

    public void connect() throws MoiraException {
	if (!connected) {
	    MoiraConnectInternal.connect(server);
	    connected = true;
	}
    }
	    
    public void auth() throws MoiraException {
	if (!isauth) {
	    synchronized(LOCK) {
		MoiraConnectInternal.auth();
	    }
	    isauth = true;
	}
    }

    public void proxy(String user) throws MoiraException {
	MoiraConnectInternal.proxy(user);
    }
	    
    public void disconnect() {
	if (connected) {
	    MoiraConnectInternal.disconnect();
	    connected = false;
	    isauth = false;
	}
    }

    public ListInfo get_list_info(String list) throws MoiraException {
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

    void update_list_info(String list, ListInfo info) throws MoiraException {
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

    void delete_member_from_list(String list, String type, String member) throws MoiraException {
	if (!connected) throw new MoiraException("Not Connected");
	String [] args = new String[3];
	args[0] = list;
	args[1] = type;
	args[2] = member;
	Object [] retinternal = MoiraConnectInternal.mr_query("delete_member_from_list", args);
	return;
    }

    void add_member_to_list(String list, String type, String member) throws MoiraException {
	if (!connected) throw new MoiraException("Not Connected");
	String [] args = new String[3];
	args[0] = list;
	args[1] = type;
	args[2] = member;
	Object [] retinternal = MoiraConnectInternal.mr_query("add_member_to_list", args);
	return;
    }

    public Member [] get_members_of_list(String list) throws MoiraException {
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
