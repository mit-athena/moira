package mit.moira;

import java.util.Date;
import java.text.SimpleDateFormat;

public class ListInfo {
    String name;
    boolean active;
    boolean bpublic;
    boolean hidden;
    boolean maillist;
    boolean grouplist;
    String gid_original;
    int gid;
    String ace_type;
    String ace_name;
    String description;
    Date modtime;
    String moduser;
    String modwith;
    static final SimpleDateFormat df = new SimpleDateFormat("dd-MMM-yyyy HH:mm:ss");

    public ListInfo(String name, String active, String spublic, String hidden, String maillist, String grouplist, String gid, String ace_type, String ace_name, String description, String modtime, String moduser, String modwith) throws MoiraException {
	this.name = name;
	this.active = (active.equals("1")) ? true : false;
	this.bpublic = (spublic.equals("1")) ? true : false;
	this.hidden = (hidden.equals("1")) ? true : false;
	this.maillist = (maillist.equals("1")) ? true : false;
	this.grouplist = (grouplist.equals("1")) ? true : false;
	this.gid_original = gid;
	try {
	    this.gid = Integer.parseInt(gid);
	} catch (NumberFormatException e) {
	    this.gid = 0;
	}
	this.ace_type = ace_type;
	this.ace_name = ace_name;
	this.description = description;
	try {
	    this.modtime = df.parse(modtime);
	} catch (java.text.ParseException p) {
	    throw new MoiraException(p.getMessage());
	}
	this.moduser = moduser;
	this.modwith = modwith;
    }
}

