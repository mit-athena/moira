package regapplet;

import mit.cipher.*;
import java.awt.*;
import java.io.*;
import java.net.*;

public class Parser {
  Socket s;
  InputStream is;
  Worker worker;
  public Encap encap;
  final static int LEN = 10;

  public Parser(Socket as, Worker aworker) throws IOException, RegAppletException {
    s = as;
    is = s.getInputStream();
    worker = aworker;
  }
  public void read() throws MITCipherException, RegAppletException, IOException {
     byte [] data = null;
     int type = is.read();
     if (type == 0x45 || type == 0x50) {
       int len = is.read();
       len = (len << 8) + is.read();
       int i = 0;
       int j = 0;
       data = new byte[len];
       while (i < len) {
	 j = is.read(data, i, len - i);
	 if (j < 0) {
	   throw new IOException("Short read");
	 }
	 i += j;
       }
       if (type == 0x45) data = encap.decrypt(data);
       else System.arraycopy(data, 8, data, 0, data.length-8);
     } else {
       throw new IOException("Unknown type code in data stream, or I/O Error");
     }
     String [] pdat = pparse(data);
     if (!pdat[0].equals("v1")) {
        throw new IOException("bad version number");
     }
     if (pdat[3].equals("c")) worker.applet.clear();
     else if (!pdat[3].equals("d")&& !pdat[1].equals("INIT"))
        throw new RegAppletException("Bad response from server.");
     if (pdat[1].equals("GETW")) {
        if (!pdat[2].equals("")) {
           worker.setState(Worker.DIALOG, Worker.SENDWORDS);
           worker.applet.showMessage(pdat[2], true);
        } else {
          worker.setState(Worker.SENDWORDS);
          worker.applet.showWordsDiag();
        }
     } else if (pdat[1].equals("GETL")) {
       if (!pdat[4].equals("")) {
	 worker.applet.guesslogin = pdat[4];
       } else worker.applet.guesslogin = "";
       if (!pdat[2].equals("")) {
          worker.setState(Worker.DIALOG, Worker.SENDLOGIN);
          worker.applet.showMessage(pdat[2], true);
       } else {
          worker.setState(Worker.SENDLOGIN);
	  worker.applet.showLoginDiag();
       }
     } else if (pdat[1].equals("GETP")) {
       if (!pdat[2].equals("")) {
          worker.setState(Worker.DIALOG, Worker.SENDPASSWD);
          worker.applet.showMessage(pdat[2], true);
       } else {
          worker.setState(Worker.SENDPASSWD);
          worker.applet.showPassDiag();
       }
     } else if (pdat[1].equals("GETN")) {
       if (!pdat[2].equals("")) {
          worker.setState(Worker.DIALOG, Worker.SENDNAME);
          worker.applet.showMessage(pdat[2], true);
       } else {
          worker.setState(Worker.SENDNAME);
          worker.applet.showNameDiag();
       }
     } else if (pdat[1].equals("INIT")) {
       worker.applet.clear();	// Clear sensitive data *now*
       if (!pdat[2].equals("")) {
	 worker.setState(Worker.DIALOG, Worker.INIT);
	 worker.applet.showMessage(pdat[2], true);
       } else {
	 if (worker.applet.isStandalone) {
	   worker.close();
	   System.exit(0);
	 }
	 worker.applet.showInit();
       }
     }
  }
  protected String [] pparse(byte [] data) throws IOException {
     String [] retval = new String [LEN];
     int i = 0;
     int j = 0;
     int k = 0;
     for (j = 0; j < LEN; j++) retval[j] = "";
     for (j = 0; j < data.length; j++) {
         if (data[j] == 0) {
            retval[i] = new String(data, 0, k, j - k);
            k = j+1;
            if (i++ == LEN) {
               throw new IOException("Too many tokens in string");
            }
         }
     }
     return (retval);
  }
}
