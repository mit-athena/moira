package mit.moira;

import java.util.Vector;
import java.util.Enumeration;

/**
 * Class with static functions for base64 encoding and decoding as well
 * as other utilities.
 */

public class Coder {
  static final char encv[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };
  static char x2c (char [] what, int off) {
    int digit;
    digit = (what[off] >= 'A' ? ((what[off] & 0xdf) - 'A')+10 : (what[off] - '0'));
    digit *= 16;
    digit += (what[off+1] >= 'A' ? ((what[off+1] & 0xdf) - 'A')+10 : (what[off+1] - '0'));
    return((char)digit);
  }
  
  private Coder() {		// No Constructors...
  }

  /**
   * Remove URL escape sequences
   *
   * @param aurl A String with potential embedded escapes.
   * @return the String with the escapes removed.
   */
  public static String unescape_url(String aurl) {
    int x, y, i;
    char [] url = new char [aurl.length()];
    aurl.getChars(0, aurl.length(), url, 0);
    StringBuffer ret = new StringBuffer();
    for (i = 0; i < url.length; i++) {
      if (url[i] == '%') {
	ret.append(x2c(url, i+1));
	i += 2;
      } else if (url[i] == '+') {
	ret.append(' ');
      } else ret.append(url[i]);
    }
    return (new String(ret));
  }

  /**
   * Encodes a byte array in base64 characters and returns the result
   * as a String.
   *
   * @param data the Byte array
   * @return The encoded string
   */

  public static String encode(byte [] data) {
    StringBuffer ret = new StringBuffer();
    int len = data.length;
    int c;
    int p = 0;
    int i = 0;
    int nib1, nib2, nib3, nib4;
    while (i < len) {
      if ((i+3) <= len) {
	c = (data[i++] & 0xFF);
	c = (c << 8) | (data[i++] & 0xFF);
	c = (c << 8) | (data[i++] & 0xFF);
	nib1 = (c & 0x0FC0000) >>> 18;
	nib2 = (c & 0x003F000) >>> 12;
	nib3 = (c & 0x0000FC0) >>> 6;
	nib4 = (c & 0x000003F);
	ret.append(encv[nib1]);
	ret.append(encv[nib2]);
	ret.append(encv[nib3]);
	ret.append(encv[nib4]);
      } else {
	p = len - i;
	c = data[i++];
	c = c << 8;
	if (p > 1) c |= (data[i++] & 0xFF);
	c = c << 8;
	if (p > 2) c |= (data[i++] & 0xFF); // Should never happen
	nib1 = (c & 0x0FC0000) >>> 18;
	nib2 = (c & 0x003F000) >>> 12;
	nib3 = (c & 0x0000FC0) >>> 6;
	nib4 = (c & 0x000003F);
	ret.append(encv[nib1]);
	ret.append(encv[nib2]);
	if (p == 1) ret.append('=');
	else ret.append(encv[nib3]);
	ret.append('=');
      }
    }
    return (new String(ret));
  }

  /**
   * Decodes a base64 encoded string and returns the decoded value
   * in a byte array.
   *
   * @param s The base64 encoded string
   * @return the byte array decoded
   */
  public static byte [] decode (String s) {
    Vector v = new Vector();
    int p = 0;
    int i = 0;
    int k = 0;
    int z = 0;
    int reg = 0;
    for (i = 0; i < s.length(); i++) {
      char c = s.charAt(i);
      if (c == '/') z = 63;
      else if (c == '+') z = 62;
      else if ((c >= 'a') && (c <= 'z')) z = (c - 'a') + 26;
      else if ((c >= 'A') && (c <= 'Z')) z = (c - 'A');
      else if ((c >= '0') && (c <= '9')) z = (c - '0') + 52;
      else if (c == '=') {
	p++;
	z = 0;
      } else continue;
      reg <<= 6;
      reg += z;
      k++;
      if (k == 4) {
	for (z = 0; z < 3; z++) {
	  v.addElement(new Integer((reg & 0xff0000) >>> 16));
	  reg = (reg << 8);
	}
	k = 0;
      }
    }
    Enumeration e = v.elements();
    byte [] retval = new byte[v.size() - p];
    for(i = 0; i < v.size() - p; i++) {
      retval[i] = ((Integer) e.nextElement()).byteValue();
    }
    return (retval);
  }
}
      
      
    
