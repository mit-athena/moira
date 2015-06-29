package mit.cipher;

public class PAD {
  private PAD() {};		// Static methods only

  public static byte [] doPad(byte [] in) {
    int len = in.length;
    int padlen = 8 - (len % 8);
    byte [] ret = new byte [len + padlen];
    System.arraycopy(in, 0, ret, 0, len);
    for (int i = 0; i < padlen; i++) {
      ret[len+i] = (byte) padlen;
    }
    return (ret);
  }

  public static byte [] unPad(byte [] in) {
    int len = in.length;
    int padlen = in[len-1];
    byte [] ret = new byte [len-padlen];
    System.arraycopy(in, 0, ret, 0, len-padlen);
    return (ret);
  }
}

