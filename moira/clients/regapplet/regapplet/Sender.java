package regapplet;

import java.awt.*;
import java.net.*;
import java.io.*;
import mit.cipher.*;

public class Sender {
  Socket s;
  BufferedOutputStream o;
  public Encap encap;   // Mumble, I wish this didn't have to be
                        // public, but I need the Parser to get to
                        // it

  public Sender (Socket as) {
    s = as;
    try {
        o = new BufferedOutputStream(s.getOutputStream());
        encap = new Encap();
        o.write((byte) 0x43);
        byte [] data = encap.getBytes();
        sendata(data);
    } catch (IOException e) {
        System.err.println("Creating Sender failed!");
        e.printStackTrace();
    }
  }
  public void SendNameData(String FirstName, String MiddleName, String LastName, String MITID) {
     String s;
     s = "v1" + '\0' + "RIFO" + '\0' + FirstName + '\0' + MiddleName + '\0' 
	 + LastName + '\0' + MITID + '\0';
     sendstring(s);
  }
  protected void sendstring (String s) {
         try {
	     byte [] data = new byte[s.length()];
	     s.getBytes(0, s.length(), data, 0);
	     //             byte [] data = s.getBytes(); // JDK 1.1 way
             data = encap.encrypt(data);
             o.write((byte) 0x45);
             sendata(data);
         } catch (IOException e) {
           System.err.println("IO Exception sending data");
           e.printStackTrace();
         } catch (MITCipherException e) {
           e.printStackTrace();
         }
  }
  protected void sendata (byte [] data) throws IOException {
         o.write((byte) (data.length >> 8));
         o.write((byte) (data.length & 0xFF));
         o.write(data);
         o.flush();
  }
  public void SendWords(String Word1,
		   String Word2,
		   String Word3,
		   String Word4,
		   String Word5,
		   String Word6) {
     String s;
     s = "v1" + '\0' + "SWRD" + '\0' + Word1 + '\0' + Word2 + '\0' 
	 + Word3 + '\0' + Word4 + '\0' + Word5 + '\0' + Word6 + '\0';
     sendstring(s);
  }
  public void SendLogin(String Login) {
     String s;
     s = "v1" + '\0' + "LOGN" + '\0' + Login + '\0';
     sendstring(s);
  }
  public void SendPassword(String Password) {
     String s;
     s = "v1" + '\0' + "PSWD" + '\0' + Password + '\0';
     sendstring(s);
  }
}
