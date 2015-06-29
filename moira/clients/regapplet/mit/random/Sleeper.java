/* Decompiled by Mocha from Sleeper.class */
/* Originally compiled from SeedGenerator.java */

package mit.random;
import java.lang.Thread;
import java.io.PrintStream;

class Sleeper extends Thread
{
    private int sleepTime;
    private static final boolean DEBUG = true;
    private static final PrintStream err = DEBUG ? System.err : null;
    private static void debug(String s) {if (DEBUG) { err.println("Sleeper: " + s); }}

    Sleeper(int i)
    {
        sleepTime = i;
    }

    public synchronized final void run()
    {
        try
        {
            Thread.sleep((long)sleepTime);
        }
        catch (InterruptedException e)
        {
        }
        return;
    }
}
