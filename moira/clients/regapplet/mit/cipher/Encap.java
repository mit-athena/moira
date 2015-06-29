package mit.cipher;

import mit.random.SecureRandom;

public class Encap {
  private byte [] deskey = null;
  private SecureRandom random = new SecureRandom();
  private BigInt enckey;
  static private BigInt n = null;
  static private BigInt e = null;
  static private BigInt d = null;
  static private BigInt p = null;
  static private BigInt q = null;
  static private BigInt cr = null;
  static {
    // BELOW IS THE CURRENT PRODUCTION "N"
    n = BigInt.valueOf("C2724E68C044FC1193BED5BBA7C7CDD8A69F828ADFBC8BE3FADCA6E0B520E3D4FB7573E8946D9AEA98562D03E76E71BD35E3F07127FD8627ABD6ADE86E9D1D3C922A2E322FE4F5027E81AD2A57DC11466269D17BFF59A06EC72D256684E761019C72CB633CACAC766EEEB23589B4ADDF35E15BE5212B5470867E49A82A81E50B", 16);
    // BELOW ARE TEST VALUES
    // n = BigInt.valueOf("B831414E0B4613922BD35B4B36802BC1E1E81C95A27C958F5382003DF646154CA92FC1CE02C3BE047A45E9B02A9089B4B90278237C965192A0FCC86BB49BC82AE6FDC2DE709006B86C7676EFDF597626FAD633A4F7DC48C445D37EB55FCB3B1ABB95BAAA826D5390E15FD14ED403FA2D0CB841C650609524EC555E3BC56CA957", 16);
    // d = BigInt.valueOf("AA8FB9C85A3A2EFF4923F3C30719D2EB3B94E37B50B68B0BE8A9562E0A724560F2BE2D79E6277A3ACD3B1635B2849B6FC56E5AEF897BECD799C9DA9199F2337CAC74CC109E3A285EA8CB70852BA565707526D52FE280B62F3EB4FB087D10DDE7C58BFA61CAB12B8F0DB79634D97CB925082B8DCD1395007E0DBCD91F5C2D2E39", 16);

    // F4
    e = BigInt.valueOf("010001", 16);
  }
  public Encap() {
    //    deskey = new byte [8];
    //    random.nextBytes(deskey);
    deskey = DES.genKey(random);
    int modlen = (n.bitLength() + 7)/8;
    byte rbytes [] = new byte [modlen - 11];
    random.nextBytes(rbytes);
    for (int i = 0; i < rbytes.length; i++) {
      if (rbytes[i] == 0) rbytes[i] = (byte)128;
    }
    byte [] q = new byte[modlen];
    q[0] = 0;
    q[1] = 2;
    System.arraycopy(rbytes, 0, q, 2, rbytes.length);
    System.arraycopy(deskey, 0, q, modlen-8, deskey.length);
    //    System.out.println("PKCSblock: " + Hex.toString(q));
    enckey = RSAAlgorithm.rsa(BigInt.valueOf(Hex.toString(q), 16), n, e, null, null, null);
  }
  public Encap(byte [] encbytes) throws MITCipherException {
    enckey = BigInt.valueOf(Hex.toString(encbytes), 16);
    //    System.out.println("ENcapInput: " + Hex.toString(encbytes));
    BigInt q1  = RSAAlgorithm.rsa(enckey, n, d, p, q, cr);
    byte [] q = Hex.fromString(q1.toString(16));
    //    System.out.println(q1.toString(16)); // XXX
    if ((q[0] != 0 && q[1] != 2) && (q[0] != 2)) // Normalization may have removed the first byte of zeros
      throw new MITCipherException("Decryption of RSA block failed");
    int modlen = q.length;
    deskey = new byte [8];
    System.arraycopy(q, modlen-8, deskey, 0, 8);
  }
  public byte [] getBytes() {
    return (Hex.fromString(enckey.toString(16)));
  }
  public byte [] encrypt(byte [] in) throws MITCipherException {
    return(zcommon(in, true));
  }
  public byte [] decrypt(byte [] in) throws MITCipherException {
    return(zcommon(in, false));
  }
  private byte [] zcommon(byte [] in, boolean encrypt) throws MITCipherException {
    DES des = new DES(encrypt);
    byte [] iv = { 0, 0, 0, 0, 0, 0, 0, 0};
    //    byte [] iv = new byte[8];
    //    System.arraycopy(deskey, 0, iv, 0, 8);
    des.setiv(iv);
    des.setKey(deskey);
    if (encrypt) {
      return(des.CBCcrypt(confound(PAD.doPad(in))));
    } else {
      return(PAD.unPad(unconfound(des.CBCcrypt(in))));
    }
  }
  private byte [] confound(byte [] in) {
    byte [] confound = new byte [8];
    random.nextBytes(confound);
    byte [] retval = new byte [in.length + confound.length];
    System.arraycopy(confound, 0, retval, 0, 8);
    System.arraycopy(in, 0, retval, 8, in.length);
    return (retval);
  }
  private byte [] unconfound(byte [] in) throws MITCipherException {
    if (in.length <= 8)
      throw new MITCipherException("Unconfound: Length <= 8");
    byte [] retval = new byte [in.length - 8];
    System.arraycopy(in, 8, retval, 0, retval.length);
    return (retval);
  }

}

