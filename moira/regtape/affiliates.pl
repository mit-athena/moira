#!/moira/bin/perl -Tw
# $Id$

use DBI;

$moira = DBI->connect("dbi:Oracle:moira", "moira", "moira",
                      { RaiseError => 1});

$tnow = localtime;
print "Beginning at $tnow\n";

($count) = $moira->selectrow_array("SELECT count(*) FROM users WHERE xmodtime < SYSDATE - 2");

$sth = $moira->prepare("UPDATE users SET affiliation_basic = 'affiliate', affiliation_detailed = 'MIT Affiliate' " .
		       "WHERE xmodtime < SYSDATE - 2");
$sth->execute;

print "Updated $count records.\n";

$tnow = localtime;
print "Ending at $tnow\n";

$moira->disconnect;
exit(0);
