#!/moira/bin/perl -Tw
# $Id$

$db = "";
$mrtest = "mrtest";
$logfile = "/moira/stellar.log";
$datafile = "/moira/stellar/stellar-groups";
$ENV{'PATH'} = "/moira/bin";

$sendmail = '/usr/lib/sendmail';
if ( ! -x $sendmail) { $sendmail = '/usr/sbin/sendmail'; }

use DBI;

$moira = DBI->connect("dbi:Oracle:moira", "moira", "moira",
                      { RaiseError => 1});

($root_id) = $moira->selectrow_array("SELECT users_id FROM users ".
				     "WHERE login = 'root'");

# Get names of current Stellar lists
($admin_id) = $moira->selectrow_array("SELECT list_id FROM list ".
				      "WHERE name = 'stellar-group-admin'");
$sth = $moira->prepare("SELECT name FROM list WHERE acl_type = 'LIST' ".
		       "AND acl_id = $admin_id");
$sth->execute;
while (($name) = $sth->fetchrow_array) {
    $lists{$name} = $name;
}

open(MRTEST, "|$mrtest >/dev/null 2>&1");
print MRTEST "connect $db\n";
print MRTEST "auth\n";
open(LOG, ">>$logfile");

open(DATA, "$datafile") or die "Unable to open $datafile: $!\n";
while (<DATA>) {
    $changed = 0;
    ($stellargroup, $junk, $membership) = split(/:/);
    chomp($stellargroup);
    chomp($junk);
    chomp($membership);
    $stellargroup = lc($stellargroup);
    @membership = split(/[,]+/, $membership);

    # Skip lists with leading dashes.  They scare us.
    if ($stellargroup =~ /^-/) {
	print LOG "Skipping $stellargroup due to leading dash.\n";
	push(@mailout, "Skipping $stellargroup due to leading dash.\n");
	next;
    }

    # Skip lists with names that are too long.
    if (length($stellargroup) > 56) {
	print LOG "Skipping $stellargroup due to name longer than 56 characters.\n";
	push(@mailout, "Skipping $stellargroup due to name longer than 56 characters.\n");
	next;
    }

    ($conflict_exists) = $moira->selectrow_array("SELECT count(*) FROM list ".
						 "WHERE name = " . $moira->quote($stellargroup) .
						 "AND ((acl_type != 'LIST') OR " .
						 "(acl_type = 'LIST' AND acl_id != $admin_id))");
    if ($conflict_exists > 0) {
	print LOG "$stellargroup already exists with an owner other than stellar-group-admin.\n";
	push(@mailout, "$stellargroup already exists with an owner other than stellar-group-admin.\n");
	next;
    }

    # Create list if it doesn't exist.
    &check_list($stellargroup, "stellar-group-admin", 1, "Automatically imported from Stellar");

    # Now fill in lists.
    %mrmembers = ();
    $sth = $moira->prepare("SELECT u.login FROM users u, imembers i, list l ".
                           "WHERE l.list_id = i.list_id AND i.member_id = ".
                           "u.users_id AND i.direct = 1 AND i.member_type = ".
			   "'USER' AND l.name = " . $moira->quote($stellargroup));
    $sth->execute;
    while (($login) = $sth->fetchrow_array) {
        $mrmembers{$login} = $login;
    }

    $sth = $moira->prepare("SELECT s.string FROM strings s, imembers i, list l ".
			   "WHERE l.list_id = i.list_id AND i.member_id = ".
			   "s.string_id AND i.direct = 1 AND i.member_type = ".
			   "'STRING' AND l.name = " . $moira->quote($stellargroup));
    $sth->execute;
    while (($string) = $sth->fetchrow_array) {
	$mrmembers{$string} = $string;
    }

    foreach $member (@membership) {
	if ($member =~ /\@mit.edu/) {
	    $member =~ s/\@mit.edu//;
	    if (!$mrmembers{$member}) {
		print LOG "Adding USER $member to $stellargroup\n";
		&add_member($member, USER, $stellargroup);
		$changed = 1;
	    } else {
		delete $mrmembers{$member};
	    }
	} else {
	    if (!$mrmembers{$member}) {
		print LOG "Adding STRING $member to $stellargroup\n";
		&add_member($member, STRING, $stellargroup);
		$changed = 1;
	    } else {
		delete $mrmembers{$member};
	    }
	}
    }

    # Everyone in membership will have been removed from mrmembers
    # now, so delete the remaining users since they don't belong.
    foreach $member (keys(%mrmembers)) {
	if ($member =~ /\@/) {
	    print LOG "Deleting STRING $member from $stellargroup\n";
	    &del_member($member, STRING, $stellargroup);
	    $changed = 1;
	} else {
	    print LOG "Deleting USER $member from $stellargroup\n";
	    &del_member($member, USER, $stellargroup);
	    $changed = 1;
	}
    }

    if ($changed) {
        $moira->do("UPDATE list SET modtime = SYSDATE, modby = $root_id, ".
                   "modwith = 'stellar' WHERE name = " . 
                   $moira->quote($stellargroup));
    }
}

close(DATA);
$moira->disconnect;

if (scalar(@mailout) > 0) {
    print LOG "Found problems.  Sending mail.\n";
    open(MAIL, "|$sendmail -t -f errors\@mit.edu" ) || die "Failed to run $sendmail";
    print MAIL "From: errors\@mit.edu\nTo: moira-admin@mit.edu\n";
    print MAIL "Subject: Moira Stellar group data load problems\n";
    print MAIL "The following problems were found during the Moira import of Stellar group data:\n\n";
    foreach $line (@mailout) {
        print MAIL $line;
    }
    close(MAIL);
} else {
    print LOG "No problems found.\n";
}

exit 0;

sub check_list {
    my ( $name, $owner, $export, $desc ) = @_;
    if (!$lists{$name}) {
        print LOG "Creating $name\n";
        print MRTEST "qy alis $name 1 0 1 1 $export \"create unique GID\" 0 0 [NONE] LIST $owner NONE NONE \"$desc\"\n";
    }
}

sub add_member {
    my ( $user, $type, $list ) = @_;
    print MRTEST "qy amtl $list $type $user\n";
}

sub del_member {
    my ( $user, $type, $list ) = @_;
    print MRTEST "qy dmfl $list $type $user\n";
}
