#!/moira/bin/perl

chdir("/u1/sms_backup/backup_1");

($sec, $min, $hour, $mday, $month) = localtime($^T);
@MONTHS = ( "Jan", "Feb", "Mar", "Apr", "May", "Jun",
	 "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" );
printf("\t\t\tMOIRA SUMMARY for %s %d\n\n", $MONTHS[$month], $mday);

open(MACHINES, "machine") || die "Cannot open machine file for input.\n";

$total = 0;

while (<MACHINES>) {
    split(/\|/);
    $TYPES{$_[2]}++;
    $total++;
}

close(MACHINES);
delete $TYPES{"NONE"};

printf("%5d Machines by type:\n", $total);
grep(push(@values, sprintf("   %5d %-8s %2d%%\n", $TYPES{$_}, $_, (100 * $TYPES{$_} + $total/2)/$total)), keys(%TYPES));
print reverse sort(@values);
print "\n";
undef %TYPES;


open(CLUSTERS, "cluster") || die "Cannot open cluster file for input.\n";

$total = 0;

while (<CLUSTERS>) {
    $total++;
}

close(CLUSTERS);
delete $TYPES{"NONE"};

printf("%5d Clusters\n", $total);


open(PRINTCAP, "printcap") || die "Cannot open printcap file for input.\n";

$total = 0;

while (<PRINTCAP>) {
    split(/\|/);
    if ($_[5]) { $auth++;  }
    $total++;
}

close(PRINTCAP);

printf("%5d Printers, %d with authentication (%d%%).\n\n", $total, $auth,
	(100 * $auth + $total/2)/$total);


if (1) {
open(USERS, "users") || die "Cannot open users file for input.\n";

$total = 0;

while (<USERS>) {
    split(/\|/);
    $total++; $STATUS{$_[7]}++;
    if ($_[7] != 3) { $classtotal++; $CLASS{$_[9]}++; }
    if ($_[7] == 1) { $CLASSA{$_[9]}++; }
    if ($_[7] == 1 || $_[7] == 6) { $pototal++; $POTYPE{$_[25]}++; }
}

close(USERS);
delete $STATUS{"NONE"};
delete $CLASS{""};

sub bytotal { substr($b, 9, 5) <=> substr($a, 9, 5); }
undef @values;
printf("%5d Non-deactivated users by class:\n", $classtotal);
printf("   class    total %%total active %%active\n");
grep(push(@values, sprintf("   %-8s %5d   %2d    %5d   %3d\n",
		$_, $CLASS{$_}, (100 * $CLASS{$_} + $classtotal/2)/$classtotal,
		$CLASSA{$_}, (100 * $CLASSA{$_} + $CLASS{$_}/2)/$CLASS{$_})),
	keys(%CLASS));
print sort bytotal @values;
print "\n";
undef %CLASS;

@STATUS = ("Registerable (0)",
	  "Active (1)",
	  "Half Registered (2)",
	  "Deleted (3)",
	  "Not registerable (4)",
	  "Enrolled/Registerable (5)",
	  "Enrolled/Not Registerable (6)",
	  "Half Enrolled (7)" );

undef @values;
printf("%5d Users by status:\n", $total);
grep(push(@values, sprintf("   %5d %-29s %2d%%\n", $STATUS{$_}, $STATUS[$_], (100 * $STATUS{$_} + $total/2)/$total)), keys(%STATUS));
print reverse sort(@values);
print "\n";
undef %STATUS;
undef @STATUS;

undef @values;
printf("%5d Active or enrolled users by pobox type:\n", $pototal);
grep(push(@values, sprintf("   %5d %-8s %2d%%\n", $POTYPE{$_}, $_, (100 * $POTYPE{$_} + $total/2)/$total)), keys(%POTYPE));
print reverse sort(@values);
print "\n";
undef %POTYPE;

}

open(LISTS, "list") || die "Cannot open list file for input.\n";

$total = 0;

while (<LISTS>) {
    split(/\|/);
    $total++;
    if ($_[2]) { $active++; }
    if ($_[3]) { $public++; }
    if ($_[4]) { $hidden++; }
    if ($_[5]) { $maillist++; }
    if ($_[6]) { $group++; }
}
close(LISTS);

printf("%5d Lists:\n", $total);
printf("   %5d %-9s %2d%%\n",$active, "active", (100 * $active + $total/2)/$total);
printf("   %5d %-9s %2d%%\n",$public, "public", (100 * $public + $total/2)/$total);
printf("   %5d %-9s %2d%%\n",$hidden, "hidden", (100 * $hidden + $total/2)/$total);
printf("   %5d %-9s %2d%%\n",$maillist, "maillist", (100 * $maillist + $total/2)/$total);
printf("   %5d %-9s %2d%%\n",$group, "group", (100 * $group + $total/2)/$total);
print "\n";


open(FILSYS, "filesys") || die "Cannot open filesys file for input.\n";

$total = 0;
while (<FILSYS>) {
    split(/\|/);
    $total++;
    $FSTYPE{$_[4]}++;
    $LTYPE{$_[13]}++;
}
close(FILSYS);
# remove dummy entry
delete $LTYPE{""};
$FSTYPE{"ERR"}--;
$total--;

undef @values;
printf("%5d Filesystems by protocol type:\n", $total);
grep(push(@values, sprintf("   %5d %-8s %2d%%\n", $FSTYPE{$_}, $_, (100 * $FSTYPE{$_} + $total/2)/$total)), keys(%FSTYPE));
print reverse sort(@values);
print "\n";
undef %FSTYPE;

undef @values;
printf("%5d Filesystems by locker type:\n", $total);
grep(push(@values, sprintf("   %5d %-8s %2d%%\n", $LTYPE{$_}, $_, (100 * $LTYPE{$_} + $total/2)/$total)), keys(%LTYPE));
print reverse sort(@values);
print "\n";
undef %LTYPE;


exit 0;


sub print_stats {
	local(@values);
	local(%data);
	print "print_stats called with ", $_[0], $_[1];
	%data = $_[1];
	grep(push(@values, sprintf($_[0], $data{$_}, $_, (100 * $data{$_} + $total/2)/$total)), keys(%data));
	print reverse sort(@values);
}
