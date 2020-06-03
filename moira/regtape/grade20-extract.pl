#!/moira/bin/perl -Tw

use DBI;


$CRED_FILE = "/moira/reg/grade20.creds";
open(CREDS, $CRED_FILE) or die "Unable to open $CRED_FILE: $!\n";
while (<CREDS>) {
    chomp($g20pwd = $_);
}
close(CREDS);

$grade20 = DBI->connect("dbi:Oracle:grade20", "moira", $g20pwd,
				     { RaiseError => 1 });
$moira = DBI->connect("dbi:Oracle:moira", "moira", "moira",
                      { RaiseError => 1});

# Get year we're running. 
$year = (localtime)[5];
$year += 1900;
$month = (localtime)[4];
# months are zero-indexed.
$month++;

# Check for expected entry in the next 9 months.
$end_month = $month + 9;
$end_year = $year;
if ($end_month > 12) {
    $end_month = $end_month % 12;
    $end_year++;
}

%mitemails = ();
%rows = ();

$admits_ref = $grade20->selectcol_arrayref("SELECT pidm, application_source from apbgn_application WHERE " .
					   " TO_DATE(month_of_expected_entry || '/' || year_of_expected_entry, 'mm/yyyy') " .
					   " >= TO_DATE('$month/$year', 'mm/yyyy') AND " .
					   " TO_DATE(month_of_expected_entry || '/' || year_of_expected_entry, 'mm/yyyy') " .
					   " <= TO_DATE('$end_month/$end_year', 'mm/yyyy') " .
					   " AND reply_code IN ('A', 'CE') AND admissions_program_type = 'G'", { Columns=>[1,2] });
%admits = @$admits_ref;

foreach $pidm (keys %admits) {
    # get MITID, first, and last from SPRIDEN table.
    ($mitid, $first, $last) = $grade20->selectrow_array("SELECT spriden_id, spriden_first_name, spriden_last_name FROM spriden " .
							"WHERE spriden_pidm = " . $grade20->quote($pidm) .
							" AND spriden_change_ind IS NULL");
    if (!defined($mitid)) {
	print STDERR "MITIDEMPTY: PIDM $pidm has empty MITID, skipping.\n";
	next;
    }


    if (length($mitid) != 9 || $mitid !~ /^9.*/) {
	print STDERR "MITIDWRONGFORMAT: PIDM $pidm has weird MITID $mitid, skipping.\n";
	next;
    }

    if (!defined($first)) {
	print STDERR "FIRSTEMPTY: PIDM $pidm has empty first name, skipping.\n";
	next;
    }

    if ($first eq "") {
	print STDERR "FIRSTEMPTY: PIDM $pidm has empty first name, skipping.\n";
	next;
    }

    if (!defined($last)) {
	print STDERR "LASTEMPTY: PIDM $pidm has empty last name, skipping.\n";
	next;
    }

    if ($last eq "") {
	print STDERR "LASTEMPTY: PIDM $pidm has empty last name, skipping.\n";
	next;
    }


    ($email) = $grade20->selectrow_array("SELECT LOWER(addr.street1) || LOWER(addr.street2) FROM sprpe_address addr, sprpe_address_meaning mean " .
					"WHERE mean.pidm = " . $grade20->quote($pidm) . " AND mean.address_type = 'EA' AND mean.thru_date IS NULL " .
					"AND mean.address_id = addr.address_id");

    if (!defined($email)) {
	# Under some circumstances, users with active accounts end up with blank emails in MITSIS.  Check for that case.
	($count) = $moira->selectrow_array("SELECT COUNT(*) FROM users WHERE clearid = " . $moira->quote($mitid));
	if (!defined($count)) {
	    print STDERR "MEGAWEIRD3: SELECT COUNT returned empry value for MIT ID $mitid.\n";
	    next;
	}

	if ($count == 0) {
	    print STDERR "EMAILEMPTY: PIDM $pidm, MIT ID $mitid has empty email address and no active Moira account, skipping.\n";
	    next;
	} elsif ($count == 1) {
	    ($login) = $moira->selectrow_array("SELECT login FROM users WHERE clearid = " . $moira->quote($mitid));
	    if (!defined($login)) {
		print STDERR "MEGAWEIRD4: Empty login for Moira account with PIDM $pidm, MIT ID $mitid, skipping.\n";
		next;
	    }

	    $email = $login . "\@mit.edu";
	} else {
	    # Multiple accounts for this MIT ID,
	    $sth = $moira->prepare("SELECT login, status FROM users WHERE clearid = " . $moira->quote($mitid));
	    $sth->execute;
	    %logins = ();
	    while (($login, $status) = $sth->fetchrow_array) {
		$logins{$status} = $login;
	    }

	    if (defined($logins{"1"})) {
		$email = $logins{"1"} . "\@mit.edu";
	    } elsif (defined($logins{"10"})) {
		$email = $logins{"10"} . "\@mit.edu";
	    } elsif (defined($logins{"3"})) {
		$email = $logins{"3"} . "\@mit.edu";
	    } else {
		print STDERR "EMAILEMPTY2: PIDM $pidm, MIT ID $mitid has empty email address and we couldn't find a Moira account for them, skipping.\n";
	    } 
	}
    }

    if ($email !~ /\@/) {
	print STDERR "EMAILFORMAT: PIDM $pidm has weird email address $email, skipping.\n";
	next;
    }
    
    if ($email =~ /\@mit.edu/) {
	$mitemails{$pidm} = "$mitid,$first,$last,$email";
	next;
    }

    $rows{$pidm} = "$mitid,$first,$last,$email";
}   

foreach $key (keys %mitemails) {
    ($mitid, $first, $last, $email) = split(/,/, $mitemails{$key});
    ($user, $domain) = split(/\@/, $email);
    if ($domain ne "mit.edu") {
	print STDERR "MEGAWEIRD: MIT.EDU email address has domain $domain, not mit.edu.\n";
	next;
    }

    ($status, $potype, $pobox) = $moira->selectrow_array("SELECT u.status, u.potype, s.string FROM users u, strings s " .
							 "WHERE login = " . $moira->quote($user) . 
							 " AND s.string_id = u.box_id");

    if (!defined($status)) {
	print STDERR "NOSUCHUSER: PIDM $key has MIT email address $email but no corresponding Moira account.\n";
	next;
    }

    if ($status eq "1") {
	print STDERR "ACTIVEMITEDU: PIDM $key, MIT ID $mitid has active MIT account $user.\n";
	next;
    } elsif ($status eq "10") {
	# Forwarded their email somewhere?
	if ($potype eq "SMTP" || $potype eq "SPLIT") {
	    $rows{$key} = "$mitid,$first,$last,$pobox";
	    next;
	} else {
	    print STDERR "SUSPENDEDMITEDU: PIDM $key, MIT ID $mitid has suspended MIT account $user and no external forwarding.\n";
	    next;
	}
    } elsif ($status eq "3") {
	# Forwarded their email somewhere?
	if ($potype eq "SMTP" || $potype eq "SPLIT") {
	    $rows{$key} = "$mitid,$first,$last,$pobox";
	    next;
	} else {
	    print STDERR "DELETEDMITEDU: PIDM $key, MIT ID $mitid has deleted MIT account $user and no external forwarding.\n";
	    next;
	}
    } else {
	print STDERR "OTHERMITEDU: PIDM $key, MIT ID $mitid has status $status MIT account $user.\n";
	next;
    }
}

$addfile = "g20.add";
open(ADD, ">$addfile") or die "Unable to open $addfile: $!\n";

$registerable = "g20.reg";
open(REG, ">$registerable") or die "Unable to open $registerable: $!\n";

$active = "g20.active";
open(ACTIVE, ">$active") or die "Unable to open $active: $!\n";

$suspended = "g20.suspended";
open(SUSPENDED, ">$suspended") or die "Unable to open $suspended: $!\n";

$deleted = "g20.deleted";
open(DELETED, ">$deleted") or die "Unable to open $deleted: $!\n";

$multiple = "g20.multiple";
open(MULTIPLE, ">$multiple") or die "Unable to open $multiple: $!\n";

$other = "g20.other";
open(OTHER, ">$other") or die "Unable to open $other: $!\n";

foreach $key (keys %rows) {
    ($mitid, $first, $last, $email) = split(/,/, $rows{$key});

    ($count) = $moira->selectrow_array("SELECT COUNT(*) FROM users WHERE clearid = " . $moira->quote($mitid));

    if (!defined($count)) {
	print STDERR "MEGAWEIRD2: SELECT COUNT returned empty value for MIT ID $mitid.\n";
	next;
    }

    # No record for this MIT ID.
    if ($count == 0) {
	print ADD "$rows{$key}" . ",0" . "\n";
    } elsif ($count > 1) {
	# This intentionally generates a file that won't be parsed by reg-token.py, as manual cleanup is required.
	print MULTIPLE "$rows{$key}\n";
    } else {
	($status) = $moira->selectrow_array("SELECT status FROM users WHERE clearid = " . $moira->quote($mitid));

	if ($status eq "0") {
	    print REG "$rows{$key}" . ",$status" . "\n";
	} elsif ($status eq "1") {
	    print ACTIVE "$rows{$key}" . ",$status" . "\n";
	} elsif ($status eq "3") {
	    print DELETED "$rows{$key}" . ",$status" . "\n";
	} elsif ($status eq "10") {
	    print SUSPENDED "$rows{$key}" . ",$status" . "\n";
	} else {
	    print OTHER "$rows{$key}" . ",$status" . "\n";
	}
    }
}

close(ADD);
close(MULTIPLE);
close(REG);
close(ACTIVE);
close(DELETED);
close(SUSPENDED);
close(OTHER);

$grade20->disconnect;
$moira->disconnect;
exit(0);
