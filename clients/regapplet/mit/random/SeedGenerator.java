/* Decompiled by Mocha from SeedGenerator.class */
/* Originally compiled from SeedGenerator.java */

package mit.random;

import java.io.PrintStream;


class SeedGenerator
{
    private static int sleepTime;
    private static final int TARGET_SPIN_COUNT = 55000;
    private static final int MIN_SPIN_COUNT = 33000;
    private static final int MAX_SPIN_COUNT = 110000;

    private static final boolean DEBUG = true;
    private static final PrintStream err = DEBUG ? System.err : null;
    private static void debug(String s) { if(DEBUG) { err.println("SeedGen: " + s); }}



    private static void setSleepTime()
    {
        sleepTime = 55000000 / genSeed(1000);
        debug(sleepTime + " ms.");
    }

    public static synchronized int genSeed()
    {
        int i;
        for (i = genSeed(sleepTime); i < 33000; i = genSeed(sleepTime))
        {
            debug(i + " ms.");
            setSleepTime();
        }
        if (i > 110000)
        {
            debug(i + " ms.");
            setSleepTime();
        }
        return i;
    }

    private static int genSeed(int i)
    {
        int j = 0;
	int sj;
        Sleeper sleeper = new Sleeper(i);
	try {
	  sleeper.setPriority(Thread.MAX_PRIORITY);
	  Thread.currentThread().setPriority(Thread.MIN_PRIORITY);
	} catch (SecurityException s) {
	}
        sleeper.start();
	sj = j;
        while (sleeper.isAlive())
        {
            j++;
            Thread.yield();
	    if ((j - sj) > 100000) {
	      // Hmm. yield may not be working
	      debug("s - sj > 100000 sleeping");
	      sj = j;
	      try {
		Thread.sleep(100);
	      } catch (InterruptedException e) {
	      }
	    }
        }
	Thread.currentThread().setPriority(Thread.NORM_PRIORITY);	
        return j;
    }

    SeedGenerator()
    {
    }

    static 
    {
        setSleepTime();
    }
}
