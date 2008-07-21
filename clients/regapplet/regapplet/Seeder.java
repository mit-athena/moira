// Class used to seed SecureRandom() in a separate thread.

package regapplet;
import mit.random.SecureRandom;

public class Seeder implements Runnable
{
  Seeder() {
  }

  public void run() {
    new SecureRandom();
  }
}
