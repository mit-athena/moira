package mit.moira;

import javax.servlet.*;
import javax.servlet.http.*;
import java.io.*;
import java.util.*;
import java.sql.SQLException;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.ResourceBundle;

public class MoiraServlet extends HttpServlet {
    static final String MOIRA_SERVER = "moira.mit.edu";
    Object LOCK = new Object();	// Used to synchronize Moira authentication
				// with the ticket fetching thread
    Kticket kt = new Kticket("jis5", "foobar", "ATHENA.MIT.EDU", LOCK);
    boolean ktinit = false;
    private static final int MAXDISPLAY = 10;
    Hashtable FileParts = new Hashtable();
    Hashtable FileTimes = new Hashtable();
    static final SimpleDateFormat df = new SimpleDateFormat("dd-MMM-yyyy HH:mm:ss");
    static final int KE_RD_AP_BADD = 39525414; // This is a kludge


    public synchronized void service(HttpServletRequest request, HttpServletResponse response) {
	System.err.println("MoiraSerlvet: Service");
	System.err.println("---------------------");

	if (!ktinit) {		// Once only initialization
	    ktinit = true;
	    Thread t = new Thread(kt);
	    t.start();
	}

	String msg = null;

	Hashtable qv = new Hashtable();

	String modifier = "";

	try {			// Get Parameters
	    Enumeration e = request.getParameterNames();
	    while (e.hasMoreElements()) {
		String pname = (String)e.nextElement();
		String [] pvaluearray = request.getParameterValues(pname);
		String pvalue = "";
		if (pvaluearray.length > 1) {
		    for (int i = 0; i < pvaluearray.length; i++)
			pvalue += pvaluearray[i];
		} else pvalue = pvaluearray[0];
		    
		System.err.println(pname + " = " + pvalue);
		qv.put(pname, pvalue);
	    }

	    modifier = (String)qv.get("modifier");
	    if (modifier == null) modifier = "";

	    if (modifier.equals("showform")) {
		do_showform(qv, request, response);
		return;
	    } else if (modifier.equals("showurl")) {
		do_showurl(qv, request, response);
		return;
	    } else if (modifier.equals("makesession")) {
		request.getSession(true);
		return;
	    }

	    String operation = (String) qv.get("operation");
	    if (operation == null) {
		try {
		    DataOutputStream out = new DataOutputStream(response.getOutputStream());
		    response.setContentType("text/html");
		    if (modifier.equals("getfile"))
			out.writeBytes("You need to select an operation from the list on the left!");
		    else if (modifier.equals("displayonly"))
			out.writeBytes("error");
		    else if (modifier.equals("getlistname")) {
			out.writeBytes(do_getlistinput(qv, request, response));
		    }
		    out.flush();
		} catch (Exception ee) {
		    ee.printStackTrace();
		}
		return;
	    }
	    if (operation.charAt(operation.length()-2) == '\r')	// Trim newline stuff
		operation = operation.substring(0, operation.length() - 2);
	    if (modifier.equals("getlistname")) {
		msg = do_getlistinput(qv, request, response);
		if (msg.equals("")) return;
	    } else if (modifier.equals("getfile")) {
		msg = getfile(operation);
	    } else if (operation.equals("getmembers")) {
		if (modifier.equals("displayonly")) {
		    msg = "show members";
		} else
		    msg = do_getmembers(qv, request, response);
		if (msg.equals("")) return;
	    } else if (operation.equals("addfinal")) {
		if (modifier.equals("displayonly")) {
		    msg = "added members";
		} else
		    msg = do_addfinal(qv, request, response);
		if (msg.equals("")) return;
	    } else if (operation.equals("editme")) {
		if (modifier.equals("displayonly")) {
		    msg = "add or remove yourself from a list";
		} else
		    msg = do_editme(qv, request, response);
		if (msg.equals("")) return;
	    } else if (operation.equals("removemembers")) {
		if (modifier.equals("displayonly")) {
		    msg = "edit/remove members";
		} else 
		    msg = do_removemembers(qv, request, response);
		if (msg.equals("")) return;
	    } else if (operation.equals("delmembers")) {
		if (modifier.equals("displayonly")) {
		    msg = "edit/remove members";
		} else 
		    msg = do_delmember(qv, request, response);
		if (msg.equals("")) return;
	    } else if (operation.equals("delconfirm")) {
		if (modifier.equals("displayonly")) {
		    msg = "member deleted";
		} else 
		    msg = do_delconfirm(qv, request, response);
		if (msg.equals("")) return;
	    } else if (operation.equals("addmember")) {
		if (modifier.equals("displayonly")) {
		    msg = "add member(s)";
		} else
		    msg = do_addmember(qv, request, response);
		if (msg.equals("")) return;
	    } else if (operation.equals("addme")) {
		if (modifier.equals("displayonly")) {
		    msg = "added to list";
		} else
		    msg = do_addremme(qv, request, response, true);
	    } else if (operation.equals("delme")) {
		if (modifier.equals("displayonly")) {
		    msg = "removed from list";
		} else
		    msg = do_addremme(qv, request, response, false);
	    } else if (operation.equals("displaylistinfo")) {
		if (modifier.equals("displayonly")) {
		    msg = "display list characteristics";
		} else
		    msg = do_showlistinfo(qv, request, response);
	    } else if (operation.equals("updatelistinfo")) {
		if (modifier.equals("displayonly")) {
		    msg = "update list characteristics";
		} else
		    msg = do_updatelistinfo(qv, request, response);
	    } else if (operation.equals("updatelistinfoconf")) {
		if (modifier.equals("displayonly")) {
		    msg = "update list characteristics";
		} else
		    msg = do_updatelistinfoconf(qv, request, response);
	    } else {
		sendError("Unimplemented Operation: " + (String)qv.get("operation"), response, true);
		return;
	    }

	} catch (AuthenticationError e) {
	    msg = "<b>" + e.getMessage() + "</b><p>\r\n";
	} catch (Exception e) {
	    msg += "<h1>Error during Processing</h1>\r\n";
	    msg += "Please try again later</HTML>\r\n";
	    e.printStackTrace();
	}
    
	try {
	    DataOutputStream out = new DataOutputStream(response.getOutputStream());
	    response.setContentType("text/html");
	    if (!MOIRA_SERVER.equals("moira.mit.edu") && modifier.equals(""))
		out.writeBytes("<font color=red>Moira Server: " + MOIRA_SERVER + "<br></font>\r\n");
	    out.writeBytes(msg);
	    out.flush();
	} catch (Exception e) {
	    e.printStackTrace();
	}
    
    }
    void sendError(String message, HttpServletResponse response, boolean showheader) {
	String msg;
	if (showheader) {
	    msg = "<HTML><TITLE>Error</TITLE></HEAD><BODY BGCOLOR=#FFFFFF>\r\n";
	    msg += "<H1>Error</H1>\r\n";
	    msg += message;
	    msg += "<p></body></html>\r\n";
	} else msg = "<b>" + message + "</b>";
	try {
	    DataOutputStream out = new DataOutputStream(response.getOutputStream());
	    response.setContentType("text/html");
	    out.writeBytes(msg);
	    out.flush();
	} catch (Exception e) {
	    e.printStackTrace();
	}
    }

    /**
     * Authenticate the user.
     *
     * @param request The HttpServlet request object
     * @return String value of the Kerberos username who owns this request
     * @exception AuthenticationError if authentication cannot be performed
     */
    String do_authentication(HttpServletRequest request) throws AuthenticationError {

	// Attempt to obtain authenticated username from the session

	HttpSession session = request.getSession(true);
	String kname = (String) session.getValue("kname");
	if (kname != null) return (kname);

	String client_email = (String)request.getAttribute("org.apache.jserv.SSL_CLIENT_EMAIL");
	if (client_email == null)
	    throw new AuthenticationError("You must use a Certificate to access this service.");
	
	// Need to remove the @MIT.EDU portion
	int i = client_email.indexOf('@');
	if (i == -1)
	    throw new AuthenticationError("Malformed or non-MIT Certificate, shouldn't happen!");
	if (!client_email.substring(i+1).equals("MIT.EDU"))
	    throw new AuthenticationError("Certificate Email Address must end in MIT.EDU");
	kname = client_email.substring(0, i);
	session.putValue("kname", kname);
	return (kname);
    }

    String do_getmembers(Hashtable qv, HttpServletRequest request, HttpServletResponse response) throws AuthenticationError {
	String kname = do_authentication(request);

	String msg = null;
	String arg = (String) qv.get("list");
	if (arg == null || arg.equals("")) {
	    msg = "<p>Argument is required</p>";
	    return (msg);
	}
	
	// Obtain the list of members stashed in the session object
	// If this is the first call to do_getmembers for this list,
	// we won't have one in which case we will obtain it from Moira
	// below

	String list = "";
	Member [] members = null;
	HttpSession session = request.getSession(true);
	if (session != null) {
	    list = (String)session.getValue("list");
	    if (list == null) list = "";
	    members = (Member []) session.getValue("members");
	}
	if (!list.equals(arg)) // Different list from argument, new session
	    members = null;	// Force obtaining members from Moira

	// Determine offset of list to view
	int offset = 0;
	String tmp = (String)qv.get("offset");
	if (tmp != null)
	    offset = Integer.parseInt(tmp);

	boolean showall = false;
	if (offset < 0) showall = true;

	if (members == null) {
	    MoiraConnect mc = null;
	    try {
		mc = connect();
		mc.proxy(kname);
		members = mc.get_members_of_list(arg);
		mc.disconnect();
		mc = null;
		if (members != null)
		    session.putValue("members", members);
		session.putValue("list", arg);
	    } catch (MoiraException m) {
		try {
		    msg = "<P><b>\r\n";
		    msg += m.getMessage();
		    msg += "</b></P>\r\n";
		    msg += "<!\r\n";
		    CharArrayWriter err = new CharArrayWriter();
		    PrintWriter perr = new PrintWriter(err);
		    m.printStackTrace(perr);
		    perr.flush();
		    msg += err.toString();
		    msg += "!>\r\n";
		    return (msg);
		} catch (Exception e) {
		    e.printStackTrace();
		}
	    } finally {
		if (mc != null) mc.disconnect();
	    }
	}

	// Do the actual display of the members
	if (members ==  null)
	    return("<b>No members for list " + arg + "</b>");
	    
	msg = "<table border=0>\r\n";
	msg += "<tr><td>&nbsp;</td><td><table border=1 cellpadding=2>\r\n";
	msg += "<tr><td colspan=2><b>Members of list: " + arg + "</b></td></tr>\r\n";
	int len = offset + MAXDISPLAY;
	if (len > members.length) len = members.length;
	if (showall) {
	    offset = 0;
	    len = members.length;
	}
	for (int i = offset; i < len; i++) {
	    msg += "<tr><td>" + members[i].getMemberType() + "</td>";
	    msg += "<td>" + members[i].getMemberId() + "</td></tr>\r\n";
	}
	msg += "</table></td><td>&nbsp;</td></tr><tr>\r\n";
	if ((offset > 0) && !showall) {
	    msg += "<td width=50><form method=POST action=\"" +
		response.encodeUrl("showresult.jhtml") + "\">\r\n";
	    msg += "<input type=hidden name=operation value=getmembers>";
	    msg += "<input type=hidden name=offset value=\"" + (offset - MAXDISPLAY) + "\">\r\n";
	    msg += "<input type=hidden name=list value=\"" + arg + "\">";
	    msg += "<input type=submit value=\"Previous\"></form></td>\r\n";
	} else
	    msg += "<td width=50>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>";
	if (!showall && ((offset > 0) || (members.length > len))) {
	    msg += "<td width=50><form method=POST action=\"" +
		response.encodeUrl("showresult.jhtml") + "\">\r\n";
	    msg += "<input type=hidden name=operation value=getmembers>";
	    msg += "<input type=hidden name=offset value=\"-1\">";
	    msg += "<input type=hidden name=list value=\"" + arg + "\">";
	    msg += "<input type=submit value=\"Show All\"></form></td>\r\n";
	} else
	    msg += "<td width=50>&nbsp;</td>";
	if (members.length > len && !showall) {
	    msg += "<td width=50><form method=POST action=\"" +
		response.encodeUrl("showresult.jhtml") + "\">\r\n";
	    msg += "<input type=hidden name=operation value=getmembers>";
	    msg += "<input type=hidden name=offset value=\"" + (offset + MAXDISPLAY) + "\">\r\n";
	    msg += "<input type=hidden name=list value=\"" + arg + "\">";
	    msg += "<input type=submit value=\"Next\"></form></td>\r\n";
	} else
	    msg += "<td width=50>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>";
	msg += "</tr>\r\n";
	msg += "</table>\r\n";
	return (msg);
    }

    void do_showform(Hashtable qv, HttpServletRequest request, HttpServletResponse response) {
	String server = request.getServerName();
	if (server.indexOf(".") == -1)
	    server += ".mit.edu"; // MIT Specific kludge!!! XXX
	String msg = "<form method=post action=\"" + response.encodeUrl("https://" + server + ":445/moira/showresult.jhtml") + "\">";
	try {
	    DataOutputStream out = new DataOutputStream(response.getOutputStream());
	    response.setContentType("text/html");
	    out.writeBytes(msg);
	    out.flush();
	} catch (Exception e) {
	    e.printStackTrace(); // Nothing else I can do here
	}
    }
	
    void do_showurl(Hashtable qv, HttpServletRequest request, HttpServletResponse response) {
	HttpSession session = request.getSession(true);	// Sigh, have to create the session here if it doesn't already exist.
	String msg = "<a href=\"" + response.encodeUrl((String)qv.get("url"))
	    + "\">";
	try {
	    DataOutputStream out = new DataOutputStream(response.getOutputStream());
	    response.setContentType("text/html");
	    out.writeBytes(msg);
	    out.flush();
	} catch (Exception e) {
	    e.printStackTrace(); // Nothing else I can do here
	}
    }

    String do_getlistinput(Hashtable qv, HttpServletRequest request, HttpServletResponse response) {
	HttpSession session = request.getSession(false);
	String list = null;
	list = (String) qv.get("list");
	if ((list == null) && (session != null)) list = (String) session.getValue("list");
	if (list == null) list = "";
	return("<input name=list value=\"" + list + "\" length=15 size=12 maxlength=40>\r\n");
    }

    String do_addmember(Hashtable qv, HttpServletRequest request, HttpServletResponse response) throws AuthenticationError {
	String msg = "";
	String list = (String) qv.get("list");
	if (list == null || list.equals("")) {
	    msg = "<p>Argument is required</p>";
	    return (msg);
	}
	
	String kname = do_authentication(request);

	HttpSession session = request.getSession(true);

	MoiraConnect mc = null;
	ListInfo li = null;
	try {
	    mc = connect();
	    mc.proxy(kname);
	    li = mc.get_list_info(list);
	} catch (MoiraException m) {
	    msg += "Error getting list info: " + m.getMessage();
	    	    msg += "<!\r\n";
	    CharArrayWriter err = new CharArrayWriter();
	    PrintWriter perr = new PrintWriter(err);
	    m.printStackTrace(perr);
	    perr.flush();
	    msg += err.toString();
	    msg += "!>\r\n";
	    return (msg);
	} finally {
	    try {
		if (mc != null) mc.disconnect();
		mc = null;
	    } catch (Exception e) {
	    }
	}

	String list_description = "";
	if (li != null) list_description = descript(li.description);

	msg += "<form method=post action=\"" +
	    response.encodeUrl("showresult.jhtml") + "\">\r\n";
	msg += "        <table border=1 cellpadding=2>\r\n          <tr> \r\n            <td colspan=2> \r\n              <p><b>Add member(s) to list " + list + "<br>\r\n</b>Description: " + list_description + "<br>\r\n                (you may enter more than one member of the same type by listing \r\n                each member on its own line)</p>\r\n            </td>\r\n          </tr>\r\n          <tr> \r\n            <td> \r\n              <select name=type>\r\n                <option value=\"USER\" selected>user</option>\r\n                <option value=\"STRING\">string</option>\r\n                <option value=\"LIST\">list</option>\r\n              </select>\r\n            </td>\r\n            <td> \r\n              <textarea name=\"member\" cols=\"20\" rows=\"4\"></textarea>\r\n            </td>\r\n          </tr>\r\n        </table>\r\n";
	msg += "<input type=hidden name=list value=\"" + list + "\">\r\n";
	msg += "<input type=hidden name=operation value=addfinal>\r\n";
	msg += "<input type=submit name=submit value=\"Add Member(s)\">\r\n";
	session.putValue("list", list);
	session.removeValue("members");	// In case we had them from old list
	return (msg);
    }

    String do_editme(Hashtable qv, HttpServletRequest request, HttpServletResponse response) throws AuthenticationError {
	String msg = null;
	String arg = (String) qv.get("list");
	if (arg == null || arg.equals("")) {
	    msg = "<p>Argument is required</p>";
	    return (msg);
	}
	
	String kname = do_authentication(request);

	HttpSession session = request.getSession(true);

	MoiraConnect mc = null;
	boolean found = false;
	boolean sublists = false;
	Member [] members = null;
	try {
	    mc = connect();
	    mc.proxy(kname);
	    members = mc.get_members_of_list(arg);
	    if (members != null) {
		for (int i = 0; i < members.length; i++) {
		    if (members[i].getMemberId().equals(kname)) {
			found = true;
			if (sublists) break;
		    }
		    if (members[i].getMemberType().equals("LIST")) {
			sublists = true;
			if (found) break;
		    }
		}
	    }
	    mc.disconnect();
	    mc = null;
	} catch (MoiraException m) {
	    try {
		msg = "<P><b>\r\n";
		msg += m.getMessage();
		msg += "</b></P>\r\n";
		msg += "<!\r\n";
		CharArrayWriter err = new CharArrayWriter();
		PrintWriter perr = new PrintWriter(err);
		m.printStackTrace(perr);
		perr.flush();
		msg += err.toString();
		msg += "!>\r\n";
		return (msg);
	    } catch (Exception e) {
		e.printStackTrace();
	    }
	} finally {
	    if (mc != null) mc.disconnect();
	}
	msg = "You are " + (found ? "" : "not ") + "a member of the list <b>" + arg + "</b>.<br>\r\n";
	if (!found && sublists)
	    msg += "Note: You may be a member of a sublist of <b>" + arg + "</b>. You may wish to check the sublists by using the <b>show list members</b> function.<br>\r\n";
	msg += "<form method=POST action=\"" + response.encodeUrl("showresult.jhtml") + "\">\r\n";
	msg += "<input type=hidden name=list value=\"" + arg + "\">\r\n";
	if (found)
	    msg += "<input type=hidden name=operation value=delme>\r\n";
	else
	    msg += "<input type=hidden name=operation value=addme>\r\n";
	msg += "<input name=submit type=submit value=\"" + (found ? "Remove Me" : "Add Me") + "\">\r\n";
	msg += "</form>";
	if (members != null) {
	    session.putValue("list", arg);
	    session.putValue("members", members);
	}
	return (msg);
    }

    String do_addremme(Hashtable qv, HttpServletRequest request, HttpServletResponse response, boolean add) throws AuthenticationError {
	HttpSession session = request.getSession(false); // Better be one
	if (session == null) {
	    return("<p>Could not proceed, has it been 30 minutes since you last interaction. If so, back up and try again.</p>");
	}

	String kname = do_authentication(request);

	String msg = null;
	String listname = (String) session.getValue("list");
	if (listname == null) {
	    return("<p>Could not find list name (shouldn't happen).</p>");
	}
	MoiraConnect mc = null;
	try {
	    mc = connect();
	    mc.proxy(kname);
	    if (!add) 
		mc.delete_member_from_list(listname, "USER", kname);
	    else
		mc.add_member_to_list(listname, "USER", kname);
	} catch (MoiraException m) {
	    msg = "<p><b>Unable to " + (add? "add" : "remove") + " you " + (add? "to" : "from") +  " the " + listname + " list.<br>\r\n";
	    msg += "The error from Moira was: " + m.getMessage() + "</b></p>";
	    msg += "<!\r\n";
	    CharArrayWriter err = new CharArrayWriter();
	    PrintWriter perr = new PrintWriter(err);
	    m.printStackTrace(perr);
	    perr.flush();
	    msg += err.toString();
	    msg += "!>\r\n";
	    return (msg);
	} finally {
	    try {
		if (mc != null) mc.disconnect();
	    } catch (Exception e) {
	    }
	}
	if (add) msg = "You have been added to the <b>" + listname + "</b> list.";
	else msg = "You have been removed from the <b>" + listname + "</b> list.";
	return (msg);
    }

    String do_removemembers(Hashtable qv, HttpServletRequest request, HttpServletResponse response) throws AuthenticationError {
	Delmember [] del = null;
	String msg = null;
	String arg = (String) qv.get("list");
	if (arg == null || arg.equals("")) {
	    msg = "<p>Argument is required</p>";
	    return (msg);
	}
	
	String kname = do_authentication(request);

	HttpSession session = request.getSession(true);

	MoiraConnect mc = null;
	try {
	    mc = connect();
	    mc.proxy(kname);
	    Member [] members = mc.get_members_of_list(arg);
	    if (members == null) {
		return("<P><b>No such list or empty list</b></p>.");
	    }
	    del = new Delmember[members.length];
	    for (int i = 0; i < members.length; i++) {
		del[i] = new Delmember(members[i]);
	    }
	    mc.disconnect();
	    mc = null;
	    session.putValue("list", arg);
	    session.removeValue("members"); // In case left over from previous call
	    session.putValue("delmembers", del);
	} catch (MoiraException m) {
	    try {
		msg = "<P><b>\r\n";
		msg += m.getMessage();
		msg += "</b></P>\r\n";
		msg += "<!\r\n";
		CharArrayWriter err = new CharArrayWriter();
		PrintWriter perr = new PrintWriter(err);
		m.printStackTrace(perr);
		perr.flush();
		msg += err.toString();
		msg += "!>\r\n";
	    } catch (Exception e) {
		e.printStackTrace();
	    }
	} finally {
	    if (mc != null) mc.disconnect();
	}
	return (do_remdisplay(qv, request, response, del, 0));
    }

    String do_remdisplay(Hashtable qv, HttpServletRequest request, HttpServletResponse response, Delmember [] del, int offset) {
	// Offset = -1 means show everybody
	boolean showall = false;
	if (offset < 0) {
	    showall = true;
	    offset = 0;
	}
	String msg = "<h2>Select Members to Delete</h2>\r\n";
	if (((offset > 0) || del.length > MAXDISPLAY) && !showall)
	    msg += "<p><font color=red>Note: Selections are remembered when you select the \"Previous\" and \"Next Buttons\".</font></p>\r\n";
	msg += "<form method=POST action=\"" +
	    response.encodeUrl("showresult.jhtml") + "\">\r\n";
	msg += "<table border=0><tr><td></td><td>\r\n";
	msg += "<table border=1 cellpadding=2>\r\n";
	int len;
	if (showall) len = del.length;
	else len = offset + MAXDISPLAY;
	if (del.length < len)
	    len = del.length;
	for (int i = offset; i < len; i++) {
	    msg += "<tr><td><input type=checkbox name=selected value=\" " + i + "\"";
	    if (del[i].marked) msg += " checked";
	    msg += "></td><td>" + del[i].member.getMemberType() + "</td>";
	    msg += "<td>" + del[i].member.getMemberId() + "</td></tr>\r\n";
	}
	msg += "</table></td>\r\n";
	msg += "<td><input type=hidden name=operation value=delmembers></td></tr>\r\n";
	if (offset > 0)
	    msg += "<td><input type=submit name=dodel value=\"Previous\"></td>";
	else
	    msg += "<td width=50>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>";
	if (!showall && (del.length > MAXDISPLAY))
	    msg += "<td><input type=submit name=dodel value=\"Show All\"></td>";
	else msg += "<td width=50>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>";
	if (((offset + MAXDISPLAY) < del.length) && !showall)
	    msg += "<td><input type=submit name=dodel value=\"Next\"></td></tr>\r\n";
	else msg += "<td width=50>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td></tr>\r\n";
	msg += "<tr><td></td><td><input type=submit name=dodel value=\"Delete Selected\"></td><td></td></tr></table></form>\r\n";
	HttpSession session = request.getSession(false);
	session.putValue("offset", new Integer(offset));
	return (msg);
    }


    String do_delmember(Hashtable qv, HttpServletRequest request, HttpServletResponse response) throws AuthenticationError {
	String kname = do_authentication(request);

	HttpSession session = request.getSession(false); // Better be one
	if (session == null) {
	    return("<p>Could not proceed, has it been 30 minutes since you last interaction. If so, back up and try again.</p>");
	}

	Delmember [] del = (Delmember []) session.getValue("delmembers");
	if (del == null) 
	    return("<p>Could not proceed, membership list doesn't exist, this should not happen!</p>");
	Integer OffsetO = (Integer) session.getValue("offset");
	if (OffsetO == null) {	// Hmmm....
	    OffsetO = new Integer(0);
	}
	
	int offset = OffsetO.intValue();

	String dodel = (String)qv.get("dodel");	// This is the submit button
	if (dodel == null)
	    return("<p>Could not proceed, submit button incorrect, shouldn't happen.</p>");

	// Process the selections on this form

	int last = offset + MAXDISPLAY;
	if (last > del.length) last = del.length;
	for (int i = offset; i < last; i++) // Clear marked bits for displayed
	    del[i].marked = false; 	    // entries

	String selected = (String)qv.get("selected");
	if (selected == null) selected = ""; // None selected

	StreamTokenizer tk = new StreamTokenizer(new StringReader(selected));
	try {
	    tk.parseNumbers();
	    while (tk.nextToken() != StreamTokenizer.TT_EOF) {
		if (tk.ttype != StreamTokenizer.TT_NUMBER) continue;
		int v = (int) tk.nval;
		del[v].marked = true;
	    }
	} catch (IOException e) {
	    e.printStackTrace();
	    return("<p>Exception while processing...</p>");
	}
	
	if (dodel.equals("Next"))
	    return(do_remdisplay(qv, request, response, del, offset + MAXDISPLAY));
	if (dodel.equals("Previous"))
	    return(do_remdisplay(qv, request, response, del, offset - MAXDISPLAY));
	if (dodel.equals("Show All"))
	    return(do_remdisplay(qv, request, response, del, -1));
	if (!dodel.equals("Delete Selected"))
	    return("<p>Cannot proceed, bad submit value, should not happen</p>");
	// At this point we are going to display a list of who will be
	// deleted and offer a confirmation.

	String listname = (String) session.getValue("list");
	if (listname == null) {
	    return("<p>Could not find list name (shouldn't happen).</p>");
	}

	// Check to see if anyone will be removed

	boolean havesome = false;
	for (int i = 0; i < del.length; i++) {
	    if (del[i].marked) {
		havesome = true;
		break;
	    }
	}
	if (!havesome)
	    return("No members selected to be removed");

	// Some will be, so throw up confirmation dialog

	String msg = "<h2>Please Confirm Deletion of:</h2>\r\n";
	msg += "<table border=1 cellpadding=2>\r\n";
	for (int i = 0; i < del.length; i++) {
	    if (del[i].marked) {
		msg += "<tr><td>" + del[i].member.getMemberType() + "</td>";
		msg += "<td>" + del[i].member.getMemberId() + "</td></tr>\r\n";
	    }
	}
	msg += "</table>\r\n";
	msg += "<form method=POST action=\"" +
	    response.encodeUrl("showresult.jhtml") + "\">\r\n";
	msg += "<input type=hidden name=operation value=delconfirm>\r\n";
	msg += "<input type=submit value=\"Confirm Deletion\">\r\n";
	msg += "</form>\r\n";
	return (msg);
    }

    String do_delconfirm(Hashtable qv, HttpServletRequest request, HttpServletResponse response) throws AuthenticationError {

	String kname = do_authentication(request);

	String msg = "";

	HttpSession session = request.getSession(false); // Better be one
	if (session == null) {
	    return("<p>Could not proceed, has it been 30 minutes since you last interaction. If so, back up and try again.</p>");
	}

	String listname = (String)session.getValue("list");

	Delmember [] del = (Delmember []) session.getValue("delmembers");
	if (del == null)
	    return("<p>Cannot proceed, nothing to delete, shouldn't happen!</p>");
	
	Vector problems = new Vector();
	Vector success = new Vector();

	MoiraConnect mc = null;
	try {
	    mc = connect();
	    mc.proxy(kname);
	    for (int i = 0; i < del.length; i++) {
		try {
		    if (del[i].marked) {
			mc.delete_member_from_list(listname, del[i].member.getMemberType(), del[i].member.getMemberId());
			success.addElement(del[i].member);
		    }
		} catch (MoiraException m) {
		    problems.addElement("<tr><td>" + del[i].member.getMemberId() + "</td><td>" + m.getMessage() + "</td></tr>\r\n");
		}
	    }
	} catch (MoiraException m) {
	    msg = "<p><b>" + m.getMessage() + "</b></p>";
	    msg += "<!\r\n";
	    CharArrayWriter err = new CharArrayWriter();
	    PrintWriter perr = new PrintWriter(err);
	    m.printStackTrace(perr);
	    perr.flush();
	    msg += err.toString();
	    msg += "!>\r\n";
	    return (msg);
	} finally {
	    try {
		if (mc != null) mc.disconnect();
	    } catch (Exception e) {
	    }
	}

	msg = "";
	if (success.size() > 0) {
	    msg += "<h2>Deleted the following:</h2>\r\n";
	    msg += "<table border=1 cellpadding=2>\r\n";
	    for (int i = 0; i < success.size(); i++) {
		msg += "<tr><td>" + ((Member)success.elementAt(i)).getMemberId() + "</td></tr>\r\n";
	    }
	    msg += "</table>\r\n";
	}
	if (problems.size() > 0) {
	    msg += "<p><h2>There were difficulties removing:</h2>\r\n";
	    msg += "<table border=1 cellpadding=2>\r\n";
	    for (int i = 0; i < problems.size(); i++) {
		msg += (String)problems.elementAt(i);
	    }
	    msg += "</table>\r\n";
	}

	return (msg);
    }

    String do_showlistinfo(Hashtable qv, HttpServletRequest request, HttpServletResponse response) throws AuthenticationError {

	String kname = do_authentication(request);

	String list = (String)qv.get("list");

	if (list == null || list.equals("")) {
	    return("Hmmm, no list specified");
	}

	HttpSession session = request.getSession(true);

	String msg = "";
	MoiraConnect mc = null;
	ListInfo li = null;
	try {
	    mc = connect();
	    mc.proxy(kname);
	    li = mc.get_list_info(list);
	} catch (MoiraException m) {
	    msg += "Error getting list info: " + m.getMessage();
	    	    msg += "<!\r\n";
	    CharArrayWriter err = new CharArrayWriter();
	    PrintWriter perr = new PrintWriter(err);
	    m.printStackTrace(perr);
	    perr.flush();
	    msg += err.toString();
	    msg += "!>\r\n";
	    return (msg);
	} finally {
	    try {
		if (mc != null) mc.disconnect();
	    } catch (Exception e) {
	    }
	}
	if (li == null) {
	    return("Did not find list info.");
	}
	msg += "<b>List: " + list + "</b><br>\r\n";
	msg += "Description: " + descript(li.description) + "<br>\r\n";
	if (li.maillist)
	    msg += "This list is a mailing list.<br>\r\n";
	if (li.grouplist)
	    msg += "This list is a Group and its ID number is " + li.gid + ".<br>\r\n";
	msg += "The Administrator of this list is the " + li.ace_type + ": " + li.ace_name + ".<br>\r\n";
	msg += "This list is: " + mkflags(li) + ".<br>\r\n";
	msg += "Last modification by " + li.moduser + " at " + df.format(li.modtime) + " with " + li.modwith + ".<br>\r\n";
	return (msg);
    }

    String do_updatelistinfo(Hashtable qv, HttpServletRequest request, HttpServletResponse response) throws AuthenticationError {

	String kname = do_authentication(request);

	String list = (String)qv.get("list");

	if (list == null || list.equals("")) {
	    return("Hmmm, no list specified");
	}

	HttpSession session = request.getSession(true);

	String msg = "";
	MoiraConnect mc = null;
	ListInfo li = null;
	try {
	    mc = connect();
	    mc.proxy(kname);
	    li = mc.get_list_info(list);
	} catch (MoiraException m) {
	    msg += "Error getting list info: " + m.getMessage();
	    	    msg += "<!\r\n";
	    CharArrayWriter err = new CharArrayWriter();
	    PrintWriter perr = new PrintWriter(err);
	    m.printStackTrace(perr);
	    perr.flush();
	    msg += err.toString();
	    msg += "!>\r\n";
	    return (msg);
	} finally {
	    try {
		if (mc != null) mc.disconnect();
	    } catch (Exception e) {
	    }
	}
	if (li == null) {
	    return("Did not find list info.");
	}

	session.putValue("listinfo", li); // Save for the confirmation call
	session.putValue("list", list);	// Save for side bar update
	session.removeValue("members");	// In case left over...

	msg += "<form method=POST action=\"" +
	    response.encodeUrl("showresult.jhtml") + "\">\r\n";
	msg += "<input type=hidden name=operation value=updatelistinfoconf>\r\n";
	msg += "<b>Update characteristics of list " + list + "</b><p>\r\n";
	msg += "Is this list a maillist? <input type=radio name=maillist value=1 "
	    + (li.maillist ? "checked" : "") + ">Yes <input type=radio name=maillist value=0 " + (li.maillist ? "" : "checked") + "> No<br>\r\n";
	// Note: We don't update group information here

	msg += "Is this list a public list? <input type=radio name=public value=1 "
	    + (li.bpublic ? "checked" : "") + ">Yes <input type=radio name=public value=0 " + (li.bpublic ? "" : "checked") + "> No<br>\r\n";
	msg += "Is this list a hidden list? <input type=radio name=hidden value=1 "
	    + (li.hidden ? "checked" : "") + ">Yes <input type=radio name=hidden value=0 " + (li.hidden ? "" : "checked") + "> No<p>\r\n";
	
	msg += "The Administrator for this list is<br>\r\n";
	msg += "<select name=ace_type><option value=\"USER\"" +
	    (li.ace_type.equals("USER") ? " selected" : "") + ">user</option>" +
	    "<option value=\"LIST\"" + (li.ace_type.equals("LIST") ? " selected" : "") + ">list</option>" +
	    "<option value=\"KERBEROS\"" + (li.ace_type.equals("KERBEROS") ? " selected" : "") + ">kerberos</option></select>";
	msg += " <input name=ace_name value=\"" + li.ace_name + "\"><br>\r\n";
	msg += "Description:<br>\r\n";
	msg += "<textarea name=description cols=60 rows=3>" + li.description + "</textarea><br>\r\n";
	msg += "<input type=submit value=\"Make Update\"></form>\r\n";
	return (msg);
    }

    String do_updatelistinfoconf(Hashtable qv, HttpServletRequest request, HttpServletResponse response) throws AuthenticationError {

	String kname = do_authentication(request);
	HttpSession session = request.getSession(false); // Better be one
	if (session == null) {
	    return("<p>Could not proceed, has it been 30 minutes since you last interaction. If so, back up and try again.</p>");
	}
	
	ListInfo li = (ListInfo)session.getValue("listinfo");
	if (li == null) {
	    return("<p>Could not proceed, could not find orignial list info, should not happen!</p>");
	}
	
	String tmp = (String)qv.get("description");
	if (tmp != null) li.description = tmp;
	tmp = (String)qv.get("maillist");
	if (tmp != null) {
	    if (tmp.equals("1")) li.maillist = true;
	    else li.maillist = false;
	}
	tmp = (String)qv.get("public");
	if (tmp != null) {
	    if (tmp.equals("1")) li.bpublic = true;
	    else li.bpublic = false;
	}
	tmp = (String)qv.get("hidden");
	if (tmp != null) {
	    if (tmp.equals("1")) li.hidden = true;
	    else li.hidden = false;
	}
	tmp = (String)qv.get("ace_type");
	if (tmp != null)
	    li.ace_type = tmp;

	tmp = (String)qv.get("ace_name");
	if (tmp != null)
	    li.ace_name = tmp;

	MoiraConnect mc = null;
	try {
	    mc = connect();
	    mc.proxy(kname);
	    mc.update_list_info(li.name, li);
	    mc.disconnect();
	    mc = null;
	} catch (MoiraException e) {
	    String msg;
	    if (e.getMessage().startsWith("No such list")) {
		msg = "<p><b>Error updating " + li.name + ".<br>\r\n";
		msg += "The list you specified as administrator does not exist.</p>\r\n";
	    } else if(e.getMessage().startsWith("No such user")) {
		msg = "<p><b>Error updating " + li.name + ".<br>\r\n";
		msg += "The user you specified as administrator is not known to Moira.</p>\r\n";		
	    } else 
		msg = "<p><b>Error during update of " + li.name + ": " + e.getMessage() + "</b></p>";
	    msg += "<!\r\n";
	    CharArrayWriter err = new CharArrayWriter();
	    PrintWriter perr = new PrintWriter(err);
	    e.printStackTrace(perr);
	    perr.flush();
	    msg += err.toString();
	    msg += "!>\r\n";
	    return (msg);
	} finally {
	    try {
		if (mc != null) mc.disconnect();
	    } catch (Exception e) {
	    }
	}
	String msg = "<b>Update of " + li.name + " succeeded</b><p>\r\n";
	qv.put("list", li.name);
	msg += do_showlistinfo(qv, request, response);
	return (msg);
    }

    String do_addfinal(Hashtable qv, HttpServletRequest request, HttpServletResponse response) throws AuthenticationError {
	HttpSession session = request.getSession(false); // Better be one
	if (session == null) {
	    return("<p>Could not proceed, has it been 30 minutes since you last interaction. If so, back up and try again.</p>");
	}

	String kname = do_authentication(request);

	String listname = (String) session.getValue("list");
	if (listname == null) {
	    return("<p>Unable to find list name! (shouldn't happen).</p>");
	}
	String msg = "";
	String member = (String)qv.get("member");
	String type = (String)qv.get("type");
	if (member == null || type == null || member.equals("")) {
	    return("<p>No names selected to be added!</p>");
	}
	MoiraConnect mc = null;
	StreamTokenizer tk = null;
	msg += "<table border=1 cellpadding=2>\r\n";
	boolean addheader = false;
	Vector warnings = new Vector();
	Vector problems = new Vector();
	try {
	    mc = connect();
	    mc.proxy(kname);
	    tk = new StreamTokenizer(new StringReader(member));
	    tk.wordChars('@', '@');
	    tk.wordChars('0', '9');
	    tk.wordChars('_', '_');
	    while (tk.nextToken() != StreamTokenizer.TT_EOF) {
		if (tk.ttype != StreamTokenizer.TT_WORD) continue;
		try {

		    // Pre-process user input
		    String [] user = canonicalize(tk.sval, type);

		    mc.add_member_to_list(listname, user[1], user[0]);
		    if (!addheader) {
			msg += "<tr><td>Added to list</td></tr>\r\n";
			addheader = true;
		    }
		    msg += "<tr><td>" + user[1] + "</td><td>" + user[0] + "</td></tr>\r\n";
		    if (user.length > 2)
			warnings.addElement(user[2]);
		} catch (MoiraException e) {
		    String err = "<tr><td>" + tk.sval + "</td><td>" + e.getMessage() + "</td></tr>\r\n";
		    problems.addElement(err);
		}
	    }
	    if (!addheader) 	// We didn't seem to be able to add anyone
		msg += "<tr><td>No one added!</td></tr>\r\n";
	    msg += "</table>\r\n";
	    mc.disconnect();
	    mc = null;
	} catch (MoiraException e) {
	    if (tk != null)
		msg = "<p><b>Error adding " + tk.sval + ": " + e.getMessage() + "</b></p>";
	    else
		msg = "<p><b>Error during add: " + e.getMessage() + "</b></p>";
	    msg += "<!\r\n";
	    CharArrayWriter err = new CharArrayWriter();
	    PrintWriter perr = new PrintWriter(err);
	    e.printStackTrace(perr);
	    perr.flush();
	    msg += err.toString();
	    msg += "!>\r\n";
	    return (msg);
	} catch (IOException e) {
	    e.printStackTrace(); // Shouldn't happen
	} finally {
	    try {
		if (mc != null) mc.disconnect();
	    } catch (Exception e) {
	    }
	}
	if (problems.size() != 0) {
	    msg += "<p>There were difficulties adding the following users:<br>\r\n";
	    msg += "<table border=1 cellpadding=2>\r\n";
	    for (int i = 0; i < problems.size(); i++)
		msg += problems.elementAt(i);
	    msg += "</table>\r\n";
	}
	if (warnings.size() != 0) {
	    msg += "<p>The following warnings were generated:<br>\r\n";
	    msg += "<table border=1 cellpadding=2>\r\n";
	    for (int i = 0; i < warnings.size(); i++)
		msg += "<tr><td>" + warnings.elementAt(i) + "</td></tr>\r\n";
	    msg += "</table>\r\n";
	}
	return (msg);
    }

    String getfile(String operation) {
	ResourceBundle bd = null;
	try {
	    bd = ResourceBundle.getBundle("mit.moira.FileParts");
	} catch (java.util.MissingResourceException e) {
	    return("Cannot find FileParts.properties");
	}
	if (bd == null) return("Cannot find FileParts.properties!");
	String filename = null;
	try {
	    filename = bd.getString(operation);
	} catch (java.util.MissingResourceException e) {
	    return("Cannot find file for operation: " + operation);
	}
	if (filename == null) {
	    return("Cannot find file for operation: " + operation);
	}
	File file = new File(filename);
	if (!file.isFile()) return("Cannot file file: " + filename);
	long filemodtime = file.lastModified();
	Long cachemodtime = (Long) FileTimes.get(filename);
	if (cachemodtime == null || filemodtime > cachemodtime.longValue()) {
	    // Need to fetch the file into the cache
	    byte [] buffer = null;
	    try {
		BufferedInputStream input = new BufferedInputStream(new FileInputStream(filename));
		buffer = new byte[input.available()];
		input.read(buffer);
		input.close();
	    } catch (FileNotFoundException f) {
				// Should never happen given check above
	    } catch (IOException e) {
		return("IO Error Reading: " + filename);
	    }
	    String data = new String(buffer);
	    data += "\r\n<!-- FileName: " + filename + "-->\r\n";
	    FileTimes.put(filename, new Long(filemodtime));
	    FileParts.put(filename, data);
	    return (data);
	} else {
	    return ((String)FileParts.get(filename));
	}
    }

    String mkflags(ListInfo li) {
	String retval = "";
	int state = 0;
	if (!li.hidden) {
	    retval = "visible";
	    state = 1;
	}
	if (!li.bpublic) {
	    switch (state) {
	    case 0:
		retval = "private";
		state = 1;
		break;
	    case 1:
		retval = "private and " + retval;
		state = 2;
		break;
	    case 2:
		retval = "private, " + retval;
	    }
	}
	if (li.active) {
	    switch (state) {
	    case 0:
		retval = "active";
		state = 1;
		break;
	    case 1:
		retval = "active and " + retval;
		state = 2;
		break;
	    case 2:
		retval = "active, " + retval;
	    }
	}
	return (retval);
    }

    /**
     * pre-process user input. Mostly this catches cases where the
     * incorrect type is used to add someone to a list.
     *
     * @param user String or Userid being proposed for an add
     * @param type Either LIST,STRING or USER
     * @return an array of elements. The first is the user portion followed by the type and an optional warning message.
     */

    private String [] canonicalize(String user, String type) throws MoiraException {
	int i;
	String [] retval = null;
	if (type.equals("STRING")) {
	    i = user.indexOf('@');
	    if (i != -1) {
		String host = user.substring(i + 1);
		if (host.equalsIgnoreCase("mit.edu")) {
		    retval = new String[3];
		    retval[0] = user.substring(0, i);
		    retval[1] = "USER";
		    retval[2] = "Converted " + user + " to userid " + retval[0];
		    return (retval);
		}
		// Don't convert
		retval = new String[2];
		retval[0] = user;
		retval[1] = "STRING";
		return (retval);
	    } else { // No @ sign
		throw new MoiraException("STRING (mailing list entries) must have an \'@\' in them!");
	    }
	} else if (type.equals("USER")) {
	    i = user.indexOf('@');
	    if (i != -1) {
		String host = user.substring(i + 1);
		if (host.equalsIgnoreCase("mit.edu")) {
		    retval = new String[3];
		    retval[0] = user.substring(0, i);
		    retval[1] = "USER";
		    retval[2] = "Converted " + user + " to userid " + retval[0];
		    return (retval);
		}
		throw new MoiraException("USER types must not have \'@\'s in them!");
	    }
	    retval = new String[2];
	    retval[0] = user;
	    retval[1] = type;
	    return (retval);
	} else if (type.equals("LIST")) {
	    i = user.indexOf('@');
	    if (i == -1) {	// No '@' sign, just process normally
		retval = new String[2];
		retval[0] = user;
		retval[1] = type;
		return (retval);
	    } else {
		String host = user.substring(i + 1);
		if (host.equalsIgnoreCase("mit.edu")) {	// trim mit.edu
		    retval = new String[3];
		    retval[0] = user.substring(0, i);
		    retval[1] = "LIST";
		    retval[2] = "Converted " + user + " to list named " + retval[0];
		    return (retval);
		} else
		    throw new MoiraException("LIST types may not contain \'@\'s in them!");
	    } 
	} else { // Just pass through everything else for now
	    retval = new String[2];
	    retval[0] = user;
	    retval[1] = type;
	    return (retval);
	}
    }

    /**
     * Remove &lt;SCRIPT&gt; tags from input String
     *
     * @param input String to check out
     * @return String with &lt;SCRIPT&gt; tag removed
     */
    private String descript(String input) {
	String lc = input.toLowerCase();
	int i = lc.indexOf("<scri");
	int j = lc.lastIndexOf("</scri");
	int len = input.length();
	if (i == -1) return (input); // Nothing to do.
	else if (j == -1) {	// No closing script tag
	    return (input.substring(0, i));
	} else {
	    return (input.substring(0, i) + input.substring(j));
	}
    }

    /**
     * Shutdown the servlet
     *
     */
    public void destroy() {
	super.destroy();
	kt.destroy();
    }


    /**
     * Connect to the Moira Server. Special case the error KE_RD_AP_BADD.
     * This error occurs if the tickets we have contain the wrong IP address.
     * This can happen if we are on a multi-homed system. If this happens,
     * get new tickets and try again. If we exceed the loop count, throw
     * an exception (sigh).
     *
     * @return A Moira Connection Object
     * @exception MoiraException on any error
     */
    protected MoiraConnect connect() throws MoiraException {
	MoiraConnect retval = null;
	int count = 0;
	while (count++ < 10) {	// Got to stop at some point!
	    retval = new MoiraConnect(MOIRA_SERVER, LOCK);
	    retval.connect();
	    try {
		retval.auth();
		return (retval); // No exception, return. yeah!
	    } catch (MoiraException m) {
		if ((m.getCode() != KE_RD_AP_BADD) || (count > 8)) {
		    if (retval != null) {
			try {
			    retval.disconnect();
			} catch (Exception e) {
			}
		    }
		    throw m;	// Re-throw if not the error we expect or we are looping
		}
	    }
	    if (retval != null) {
		try {
		    retval.disconnect();
		} catch (Exception e) {
		}
		retval = null;
	    }
	    System.err.println("MoiraServlet: Forced Renewal...");
	    kt.renew();		// Renew tickets
	}
	return (null);
    }

}

class Delmember {
    boolean marked = false;
    Member member = null;

    Delmember(Member m) {
	member = m;
    }
}
