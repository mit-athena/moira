/* Decompiled by Mocha from SecureRandom.class */
/* Originally compiled from SecureRandom.java */

package mit.random;

import java.util.Random;

public class SecureRandom extends Random
{
    private byte state[];
    private SHA1 digest = null;
    private static SecureRandom generatorGenerator;
    private byte randomBytes[];
    private int randomBytesUsed;
    private long counter;

    public SecureRandom()
    {
        byte [] ab = nextSeed();
	digest = new SHA1();
	setSeed(ab);
    }

    private static synchronized byte[] nextSeed()
    {
        if (generatorGenerator == null)
            generatorGenerator = new SecureRandom(getSeed(20));
        byte ab[] = new byte[20];
        generatorGenerator.nextBytes(ab);
        return ab;
    }

    public SecureRandom(byte ab[])
    {
      digest = new SHA1();
      setSeed(ab);
    }

    public synchronized void setSeed(byte ab[])
    {
        if (digest == null) return;
        if (state != null)
            digest.Update(state);
        state = digest.Digest(ab);
    }

    public void setSeed(long i)
    {
        if (i != 0L)
            setSeed(longToByteArray(i));
    }

    public synchronized void nextBytes(byte ab[])
    {
        int i = ab.length;
        for (int j = 0; j < i; )
        {
            if (randomBytes == null || randomBytesUsed == randomBytes.length)
            {
                digest.Update(state);
                randomBytes = digest.Digest(longToByteArray(counter++));
                randomBytesUsed = 0;
            }
            ab[j++] = randomBytes[randomBytesUsed++];
        }
    }

    protected final int next(int i1)
    {
        int j = (i1 + 7) / 8;
        byte ab[] = new byte[j];
        int k = 0;
        nextBytes(ab);
        for (int i2 = 0; i2 < j; i2++)
            k = (k << 8) + (ab[i2] & 255);
        return k >>> j * 8 - i1;
    }

    public static byte[] getSeed(int i)
    {
        byte ab[] = new byte[i];
        for (int j = 0; j < i; j++)
            ab[j] = (byte)SeedGenerator.genSeed();
        return ab;
    }

    private static byte[] longToByteArray(long i)
    {
        byte ab[] = new byte[8];
        for (int j = 0; j < 8; j++)
        {
            ab[j] = (byte)i;
            i >>= 8;
        }
        return ab;
    }
}
