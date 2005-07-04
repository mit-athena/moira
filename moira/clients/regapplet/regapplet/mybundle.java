package regapplet;

import java.util.Hashtable;

// This module is a kludge which has a similar interface as the ResourceBundle
// Object, but it hardcodes most things. Its here because Netscape 3.0 doesn't
// implement Resource Bundles and I depended on them, so thus this kludge.

public class mybundle {
  private Hashtable ht;
  public mybundle() {
    ht = new Hashtable();
    ht.put("welcome", "Welcome to the Athena Computing Environment");
    ht.put("getw", "You should have received an \"Athena Account Coupon\" containing six secret Key Words to be used to verify your identity. Please enter those six words below.\n(If you are a freshmen entering in the fall term, you should have received this Coupon in your orientation packet. If you are a transfer student or graduate student, you should have picked up your Coupon at the Student Services Center.)\nIf you do not have your Athena Account Coupon, you will not be able to register for an account at this time. You can bring your MIT ID to the Student Services Center in Building 11 to pick up a new Coupon.");
    ht.put("geti", "Please enter the 8 digit PIN code that you have been using to access your MIT/Sloan MBA Online Application.\nIf you do no have or do not remember your personal PIN code please contact admit@sloan.mit.edu for assistance.");
    ht.put("getl1", "Now you must pick an Athena username. Your Athena username will also be used for your MIT.EDU email address. Friends, colleagues, and others will use this username to communicate with you. No two people may have the same Athena username.\nA username must be at least 3 characters, and not more than 8. It must start with a lowercase letter, and may contain only lowercase letters, numbers, and the underscore character (_).\nMost people choose a username which is similar to their real name. A common choice is to use the first initial (or first and middle initials) followed by the last name (for example, \"jdoe\" for John Doe).");
    ht.put("getl2", "Some example usernames based on your name are:");
    ht.put("getl3", "You may choose whatever you want for your username, but the username you choose now will stay with you for your entire career at MIT. Once it is registered for you, it CANNOT be changed.");
    ht.put("getp", "You need to invent a secret password to keep your work on Athena secure and private. Make it easy to remember, but something that others won't easily guess--don't use your name, your dog's name, your birthday, or a word from the dictionary.\nYou'll be able to change your password whenever you like. Passwords should be at least 6 characters long, and may contain UPPER- and lower-case letters, numbers, punctuation, and spaces. A good password might be:\n-- some initials, like \"GykoR-66\" for \"Get your kicks on Route 66.\"\n-- an easily pronounced nonsense word, like \"slaRooBey\" or \"krang-its\"\n-- a mis-spelled phrase, like \"2HotPeetzas\" or \"ItzAGurl\"\nTo help keep your new password secret, you won't see it while you're typing, so be sure to type carefully.\nNote: Your password (and everything else you type into this program) is encrypted when sent over the network.");
    ht.put("err","The registration server unexpectedly stopped communicating with us. This may indicate a temporary problem with the registration system, or with the network.\nPlease try again later. If this problem persists, contact the Athena User Accounts Office in N42-105A, x3-1325.");
    ht.put("err1","We were unable to contact the Athena Registration Server.\nThis may be because you are behind a firewall which is blocking TCP port 9001. If your company, network provider, or personal computer is set up to block network traffic over this port, you will not be able to connect to the MIT registration server.\nThis error may also indicate a temporary problem with the registration system, or with the network. If you are sure you're not using a firewall, please try again later. If this problem persists, contact the Athena User Accounts Office at 617-253-1325 or accounts@mit.edu, and describe the error.");
    ht.put("bale","By hitting CANCEL you have requested that we terminate your registration session. However if you have already gone past the screen where you choose your login name, your choice has been set and cannot be unset.\nIf you really want to quit, then click on the \"Yes\" button below. Otherwise choose \"No\" and we will go back to where you were.");

    ht.put("rules", "MITnet Rules of Use\n\nAll users of Athena and MITnet (MIT's campus network) agree to abide by\nthe Athena/MITnet rules of use. These rules are summarized here. For the\ncomplete text, see the document \"Welcome to Athena\" or\nhttp://web.mit.edu/olh/Rules/.\n\n     Comply with Intended Use of the System\n\n     1. Don't violate the intended use of MITnet.\n\n        * Don't try to interfere with or alter the integrity of the\n          system at large.\n        * Don't try to restrict or deny access to the system by legitimate\n          users.\n        * Don't use MITnet for private financial gain.\n\n     Assure Ethical Use of the System\n\n     2. Don't let anyone know your password(s).\n\n     3. Don't violate the privacy of other users.\n\n     4. Don't copy or misuse copyrighted material (including software).\n\n     5. Don't use MITnet to harass anyone in any way.\n\n     Assure Proper Use of System Resources\n\n     6. Don't overload the communication servers; in particular, don't\nabuse your electronic mail (email) or Zephyr privileges.\n\nAdditional Rules for Athena Facilities\n\n     Comply with Intended Use of the System\n\n     A1. Don't violate the intended use of the Athena system.\n\n     Protect Athena Equipment\n\n     A2. Don't eat, drink, or bring food or liquids into the Athena clusters.\n\n     A3. Don't turn the power off on Athena equipment.\n\n     A4. Don't reconfigure the cluster, either hardware or software.\n\n     Assure Fair Access to Workstations in Athena Clusters\n\n     A5. Don't violate the official priorities for the use of\nworkstations; in particular, don't play games or engage in other\nnon-academic activity if the cluster is busy, and don't log on to more\nthan one workstation at a time.\n\n     A6. Don't leave your workstation unattended for more than 20 minutes.\n\n     A7. Don't make a lot of noise in the Athena clusters.\n\n     Assure Fair Access to Athena Printers\n\n     A8. Don't violate the official priorities for the use of\nprinters; in particular, don't be a printer hog or use the Athena\nprinters as copy machines.\n\n\n\n\n\n\n\n\n");
  }
  public String getString(String key) {
    return ((String) ht.get(key));
  }
}
