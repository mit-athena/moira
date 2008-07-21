package mit.moira;

import java.util.Date;
import java.io.IOException;

public class Kticket implements Runnable {
    String name;
    String instance;
    String realm;
    boolean dostop = false;
    Runtime r;
    Date renewTime;

    Kticket(String name, String instance, String realm) {
	this.name = name;
	this.instance = instance;
	this.realm = realm;
	r = Runtime.getRuntime();
	renewTime = new Date();
    }

    public void run() {
	for(;;) {
	    if (dostop) return;
	    Date now = new Date();
	    if (now.after(renewTime)) {
		renew();
		renewTime = new Date(System.currentTimeMillis() + 6*3600*1000L); // 6 hours
	    }
	    try {
		Thread.sleep(30*1000L); // Sleep for 30 seconds
	    } catch (InterruptedException i) {
		// Nothing to be done about it
	    }
	}
    }

    public void renew() {
	Moira m = null;
	try {
	    m = Moira.getInstance("MOIRA.MIT.EDU"); // Host doesn't matter, we won't be making a connection!
				// We are getting a Moira instance so that no
				// other thread will have one and we can safely
				// modify the ticket file
	    Process p = r.exec("/usr/athena/bin/kinit -k -t /mit/jis/javahacking/moira/KEY " + name + "/" + instance + "@" + realm);
	    p.waitFor();
	} catch (IOException e) {
	    e.printStackTrace();
	} catch (InterruptedException i) {
	} finally {
	    m.done();		// Release Moira object
	}
    }

    public void destroy() {
	dostop = true;
	try {
	    Process p = r.exec("/usr/athena/bin/kdestroy");
	    p.waitFor();
	} catch (IOException e) {
	    e.printStackTrace();
	} catch (InterruptedException i) {
	}
    }
}
