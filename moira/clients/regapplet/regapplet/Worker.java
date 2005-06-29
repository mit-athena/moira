package regapplet;

import java.awt.*;
import java.net.*;
import java.io.*;
import java.util.Properties;
import mit.cipher.*;

public class Worker implements Runnable {
  public static final int SENDNAME = 0;
  public static final int SENDWORDS = 1;
  public static final int SENDLOGIN = 2;
  public static final int SENDPASSWD = 3;
  public static final int INIT = 4;
  public static final int DIALOG = 5;
  public static final int INIT1 = 6;
  public static final int SENDPIN = 7;
  public static final int CONFIRMLOGIN = 8;
  int oldstate = INIT;
  int state = INIT;
  int nextstate = INIT;
  Socket sock = null;
  Sender send;
  Parser parse;
  public Regapplet applet;
  public Worker(Regapplet tapplet) {
    applet = tapplet;
  }
  public void run () {
    try {
      switch (state) {
      case DIALOG:
        state = nextstate;
        switch (state) {
           case SENDWORDS:
                applet.showWordsDiag();
                break;
	   case SENDPIN:
	        applet.showPinDiag();
		break;
	   case SENDNAME:
                applet.showNameDiag();
                break;
           case SENDLOGIN:
	        applet.showLoginDiag();
		break;
	   case CONFIRMLOGIN:
	        applet.showLoginConfirmDiag();
		break;
	   case SENDPASSWD:
	        applet.showPassDiag();
		break;
           case INIT:
                applet.worker.close();
                applet.workthread = null;
                applet.worker = null;
		if (applet.isStandalone) {
		  System.exit(0);
		}
                applet.showInit();
           break;
        }
        return;
      case INIT:
	if (Regapplet.doRules) {
	  state = INIT1;
	  applet.showRules();
	  // Hmm. the lines below caused problems on the Mac
	  //	  Thread z = new Thread(new Seeder()); // Generate numbers in thread
	  //	  z.start();
	  return;
	}
	// Fall through
      case INIT1:
	applet.showMessage("Connecting to Server...");
        try {
	  String host;
	  if (applet.isStandalone) {
	    host = (System.getProperties()).getProperty("host", "");
	    if (host == null || host.equals("")) {
	      System.err.println("Cannot learn host name (application) using localhost");
	      host = "localhost";
	    }
	  } else {
	    URL cb = applet.getCodeBase();
	    String protocol = cb.getProtocol();
	    if (protocol.equals("file") || protocol.equals("FILE")) {
	      System.err.println("Applet: FILE protocol in use, connecting to localhost");
	      host = "localhost";
	    } else {
	      host = cb.getHost();
	    }
	  }
	  sock = new Socket(host, 9001);
	} catch (Exception e) {
          e.printStackTrace();
	  applet.showError1(true);
          applet.show();
	  state = DIALOG;
	  nextstate = INIT;
	  return;
	  //          try {
	  //              Thread.sleep(5000);
	  //          } catch (java.lang.InterruptedException ae) {
	  //            // Ignore exception for now
	  //          }
	  //	  applet.showInit();
	  //	  return;
        }
	send = new Sender(sock);
	parse = new Parser(sock, this);
        parse.encap = send.encap;      // Mumble, they need to share
                                       // this
	break;			       // We now expect a GETN after connect
	//        applet.showNameDiag();
	//        state = SENDNAME;
	//	return;
      case SENDNAME:
        applet.showMessage("Please wait...");
	send.SendNameData(applet.FirstName.getText(),
                          applet.MiddleName.getText(),
			  applet.LastName.getText(),
			  applet.MITID.getText());
        break;
      case SENDWORDS:
	applet.showMessage("Please wait...");
	send.SendWords(applet.Word1.getText(),
		       applet.Word2.getText(),
		       applet.Word3.getText(),
		       applet.Word4.getText(),
		       applet.Word5.getText(),
		       applet.Word6.getText());
	break;
      case SENDPIN:
	applet.showMessage("Please wait...");
	send.SendPin(applet.Pin.getText());
	break;

      case SENDLOGIN:
	applet.showMessage("Please wait...");
	send.SendLogin(applet.LoginName.getText());
	break;
      case CONFIRMLOGIN:
	applet.showMessage("Please wait...");
	send.ConfirmLogin(applet.chosenlogin);
	break;
      case SENDPASSWD:
	if (!(applet.Password1.getText().equals(applet.Password2.getText()))) {
	  applet.Password1.setText("");
	  applet.Password2.setText("");
	  applet.showMessage("Entered Passwords do not match, try again");
	  try {
	    Thread.sleep(3000);
	  } catch (java.lang.InterruptedException e) {
	  }
	  applet.showPassDiag();
	  return;
	}
	applet.showMessage("Please wait...");
	send.SendPassword(applet.Password1.getText());
	break;
      }
      parse.read();
      // Add code to parse results and switch on new state
    } catch (IOException e) {
      e.printStackTrace();
      applet.showError(true);
      state = DIALOG;
      nextstate = INIT;
      // Moby reset applet
    } catch (MITCipherException e) {
      applet.showError(true);
      state = DIALOG;
      nextstate = INIT;
      e.printStackTrace();
      // Moby reset applet here too.
    } catch (RegAppletException a) {
      a.printStackTrace();
      applet.showError(true);
      state = DIALOG;
      nextstate = INIT;
    }
  }
  public void setState(int i) {
         state = i;
  }
  public void setState(int i, int j) {
         state = i;
         nextstate = j;
  }
  public void close() {
         if (sock != null) {
            try {
                sock.close();
            } catch (Exception e) {
            }
	    sock = null;
         }
  }
  public void finalize() {
    if (sock != null) {
      System.err.println("Worker Finalized with open socket! Closing it.");
      close();
    }
  }
}
