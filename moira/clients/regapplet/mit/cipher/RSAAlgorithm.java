// Crib'd from cryptix by JIS

// $Id: RSAAlgorithm.java,v 1.1 1998-08-01 18:25:19 danw Exp $

/*
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package mit.cipher;

import java.io.PrintStream;
import mit.random.SecureRandom;

/**
 * A class that calculates the RSA algorithm. A single method is
 * used for encryption, decryption, signing and verification:
 * <ul>
 *   <li> for encryption and verification, the public exponent, <i>e</i>,
 *        should be given.
 *   <li> for decryption and signing, the private exponent, <i>d</i>,
 *        should be given.
 * </ul>
 * <p>
 * The purpose of having this as a separate class is to avoid duplication
 * between the RSA Cipher and Signature implementations.
 * <p>
 * <b>References:</b>
 * <ol>
 *   <li> Donald E. Knuth,
 *        <cite>The Art of Computer Programming</cite>,
 *        ISBN 0-201-03822-6 (v.2) pages 270-274.
 *        <p>
 *   <li> <cite>ANS X9.31, Appendix B</cite>.
 * </ol>
 * <p>
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.1 $</b>
 * @author  Raif S. Naffah
 * @author  David Hopwood
 * @since   Cryptix 2.2.2
 */
public final class RSAAlgorithm
{
    private RSAAlgorithm() {} // static methods only

// Debugging methods and vars.
//...........................................................................

    private static final boolean DEBUG = true;
    private static final PrintStream err = DEBUG ? System.err : null;
    private static void debug(String s) { err.println("RSAAlgorithm: " + s); }


// Constants
//...........................................................................

    private static final BigInt ONE = BigInt.valueOf("1");


// Own methods
//...........................................................................

    /**
     * Computes the RSA algorithm. If <i>p</i> is null, straightforward
     * modular exponentiation is used.
     * <p>
     * Otherwise, this method uses the Chinese Remainder Theorem (CRT) to
     * compute the result given the known factorisation of the public
     * modulus <i>n</i> into two relatively prime factors <i>p</i> and <i>q</i>.
     * The arithmetic behind this method is detailed in [1] and [2].
     * <p>
     * The comments that follow, which are edited from the PGP
     * <samp>mpilib.c</samp> file <em>with p and q reversed</em>, make
     * the practical algorithmic implementation clearer:
     * <p>
     * <blockquote>
     *     Y = X**d (mod n) = X**d (mod pq)
     * </blockquote>
     * <p>
     * We form this by evaluating:
     * <blockquote>
     *     p2 = plain**d (mod p) and <br>
     *     q2 = plain**d (mod q)
     * </blockquote>
     * and then combining the two by the CRT.
     * <p>
     * Two optimisations of this are possible. First, we reduce X modulo p
     * and q before starting, since:
     * <blockquote>
     *    x**a (mod b) = (x (mod b))**a (mod b)
     * </blockquote>
     * <p>
     * Second, since we know the factorisation of p and q (trivially derived
     * from the factorisation of n = pq), and input is relatively prime to
     * both p and q, we can use Euler's theorem:
     * <blockquote>
     *     X**phi(m) = 1 (mod m),
     * </blockquote>
     * to throw away multiples of phi(p) or phi(q) in d. Letting
     * <blockquote>
     *     ep = d (mod phi(p)) and <br>
     *     eq = d (mod phi(q))
     * </blockquote>
     * then combining these two speedups, we only need to evaluate:
     * <blockquote>
     *     p2 = (X mod p)**ep (mod p) and <br>
     *     q2 = (X mod q)**eq (mod q).
     * </blockquote>
     * <p>
     * Now we need to apply the CRT. Starting with:
     * <blockquote>
     *     Y = p2 (mod p) and <br>
     *     Y = q2 (mod q)
     * </blockquote>
     * we can say that:
     * <blockquote>
     *     Y = q2 + kq
     * </blockquote>
     * and if we assume that:
     * <blockquote>
     *     0 <= q2 < q, then <br>
     *     0 <= Y < pq for some 0 <= k < p
     * </blockquote>
     * <p>
     * Since we want:
     * <blockquote>
     *     Y = p2 (mod p),
     * </blockquote>
     * then
     * <blockquote>
     *     kq = (p2 - q2) (mod q)
     * <blockquote>
     * <p>
     * Since p and q are relatively prime, q has a multiplicative inverse
     * u mod p. In other words, uq = 1 (mod p).
     * <p>
     * Multiplying by u on both sides gives:
     * <blockquote>
     *     k = u * (p2 - q2) (mod p)
     * </blockquote>
     * <p>
     * Once we have k, evaluating kq + q2 is trivial, and that gives
     * us the result.
     *
     * @param  X    the BigInt to be used as input.
     * @param  n    the public modulus.
     * @param  exp  the exponent (e for encryption and verification,
     *              d for decryption and signing).
     * @param  p    the first factor of the public modulus.
     * @param  q    the second factor of the public modulus.
     * @param  u    the multiplicative inverse of q modulo p.
     * @return the result of the computation.
     */
    public static BigInt rsa(BigInt X, BigInt n, BigInt exp,
                          BigInt p, BigInt q, BigInt u) {
        if (p != null) {
            //
            // Factors are known.
            // First check if u = (1/q) mod p; if not exchange p and q
            // before using CRT. This is needed for factors not generated/set
            // by cryptix.provider.rsa classes; eg. PGP applications.
            //
            if (! u.equals(q.modInverse(p))) {
                BigInt t = q;
                q = p;
                p = t;
            }
            //
            // Factors are known and usable by our CRT code.
            //
//            // Was "X.mod(p).modPow(...)", but BigInt.modPow already
//            // calculates X.mod(p).
//            BigInt p2 = X.modPow(exp.mod(p.subtract(ONE)), p);
//            BigInt q2 = X.modPow(exp.mod(q.subtract(ONE)), q);
            BigInt p2 = X.mod(p).modPow(exp.mod(p.subtract(ONE)), p);
            BigInt q2 = X.mod(q).modPow(exp.mod(q.subtract(ONE)), q);

            // "if (p2.compareTo(q2) == 0) return q2;" removed because it
            // is redundant.
            if (p2.equals(q2)) return q2;
            BigInt k = (p2.subtract(q2).mod(p)).multiply(u).mod(p);
            return q.multiply(k).add(q2);
        } else {
            // Slower method.
	  //	    System.err.println("RSA (exp): " + exp.toString(16)); // XXX
	  //	    System.err.println("RSA (n):   " + n.toString(16));
	    X = X.modPow(exp, n);
	    // System.err.println("RSA =:     " + X.toString(16));
            return (X);
        }
    }

    /**
     * Computes the RSA algorithm, without using the Chinese Remainder
     * Theorem.
     *
     * @param  X    the BigInt to be used as input.
     * @param  n    the public modulus.
     * @param  exp  the exponent (e for encryption and verification,
     *              d for decryption and signing).
     * @return the result of the computation.
     */
    public static BigInt rsa(BigInt X, BigInt n, BigInt exp) {
        return BigInt.modExp(X, exp, n);
	//        return X.modPow(exp, n);
    }

    /**
     * Used to assert that the generated <i>p</i> and <i>q</i> factors
     * are primes with a probability that will exceed <i>1 - (1/2)**
     * CONFIDENCE</i>.
     * <p>
     * FIPS-186 (May 1994), APPENDIX 2. GENERATION OF PRIMES FOR THE DSA,
     * section 2.1. A PROBABILISTIC PRIMALITY TEST, describes the algorithm
     * used in BigInt's <i>isProbablePrime()</i> method and suggests a
     * value not less than 50. The litterature [1] recommends a value of 80
     * which we use.
     */
    private static final int CONFIDENCE = 80;

  //    public static void generateKeyPair() {
  //      generateKeyPair(512);
    //        return (generateKeyPair(512));
  //    }

    private static SecureRandom source = new SecureRandom();

  /*    public static void generateKeyPair(int strength) {
        int k1 = strength / 2;
        int k2 = strength - k1;
        BigInt p, q, n, phi, d;
	BigInt e = BigInt.valueOf(0x100001L);
        long t1 = 0;

	if (DEBUG) t1 = System.currentTimeMillis();

        while (true) {
            try {
                while (true) {
//                    p = new BigInt(k1, CONFIDENCE, source).setBit(k1-1).setBit(k1-2);
//                    q = new BigInt(k2, CONFIDENCE, source).setBit(k2-1).setBit(k2-2);
                    p = new BigInt(k1, CONFIDENCE, source);
                    q = new BigInt(k2, CONFIDENCE, source);
                    n = p.multiply(q);
                    if (p.compareTo(q) != 0 && n.bitLength() == strength)
                        break;
                }
                phi = p.subtract(ONE).multiply(q.subtract(ONE));
                d = e.modInverse(phi);
                break;
            }
            catch (ArithmeticException ae) {}  // gcd(e * phi) != 1. Try again
        }

	if (DEBUG) {
	  t1 = System.currentTimeMillis() - t1;
	  debug(" ...generateKeyPair() completed in "+t1+" ms.");
	}

	if (DEBUG) {
	  try {
	    // rsa-encrypt twice
	    err.print("RSA parameters self test #1/2... ");
	    BigInt x = new BigInt(k1, source);
	    BigInt y = RSAAlgorithm.rsa(x, n, e);
	    BigInt z = RSAAlgorithm.rsa(y, n, d);
	    boolean yes = z.compareTo(x) == 0;
	    err.println(yes ? "OK" : "Failed");
	    if (!yes) throw new RuntimeException();
	    // rsa-encrypt then -decrypt
	    err.print("RSA parameters self test #2/2... ");
	    BigInt u = q.modInverse(p);
	    z = RSAAlgorithm.rsa(y, n, d, p, q, u);
	    yes = z.compareTo(x) == 0;
	    err.println(yes ? "OK" : "Failed");
	    if (!yes) throw new RuntimeException();
	    err.println();
	  }
	  catch (Exception ex) {
	    err.println("RSA parameters:");
	    err.println("         n: " + BI.dumpString(n));
	    err.println("         e: " + BI.dumpString(e));
	    err.println("         d: " + BI.dumpString(d));
	    err.println("         p: " + BI.dumpString(p));
	    err.println("         q: " + BI.dumpString(q));
	    err.println("q^-1 mod p: " + BI.dumpString(q.modInverse(p)));
	    throw new RuntimeException(e.toString());
	  }
	  err.println("RSA parameters:");
	  err.println("         n: " + BI.dumpString(n));
	  err.println("         e: " + BI.dumpString(e));
	  err.println("         d: " + BI.dumpString(d));
	  err.println("         p: " + BI.dumpString(p));
	  err.println("         q: " + BI.dumpString(q));
	  err.println("q^-1 mod p: " + BI.dumpString(q.modInverse(p)));
	}
	//       return makeKeyPair(n, e, d, p, q);
    }

  // For testing
  public static void main(String [] Args) {
    generateKeyPair();
  }
  */
}
