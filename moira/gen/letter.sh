#!/moira/bin/perl
# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/letter.sh,v 1.5 1992-12-21 18:03:53 mar Exp $
# This script prints the letters confirming registration for the extra
# kerberos principal.

# The algorithm consists of first checking to see that the previous 
# day's job printed.  If so, the cached copy is deleted.  Otherwise,
# today's is appended to yesterday's and we continue working with that.

$printer = "nil";
$newfile  = "/tmp/letter.out";
$savefile = "/u1/letter.save";
$holdfile = "/u1/letter.hold";
$logfile = "/u1/letter.log";

open(LOG, ">>" . $logfile);

print LOG "\nRunning " . `date`;

$printed =`/usr/athena/bin/lpquota -l | tail -2`;
@printed = split(/\s+/, $printed);

print LOG "\nlast printed " . $printed;

$filed = `ls -l $savefile`;
@filed = split(/\s+/, $filed);

print LOG "\nfile touched " . $filed;

if ($printed[1] eq $filed[4] && $printed[2] eq $filed[5]) {
    system("mv $newfile $savefile");
    print LOG "\nall OK";
} else  {
    if ( -s $savefile) {
	if ( ! -f $holdfile) {
	    open(MAIL, "|/bin/mail dbadmin thorne");
	    print MAIL "To: dbadmin, thorne\n";
	    print MAIL "Subject: reg_extra printing error\n";
	    print MAIL "Reg_extra letters failed to print yesterday\n";
	    print MAIL $filed;
	    print MAIL $printed;
	    close(MAIL);
	    print LOG  "\nmail sent";
	}
	system("cat $newfile >> $savefile");
    } else {
	system("mv $newfile $savefile");
    }
}

unlink($holdfile);

print LOG "\nprinting...";

open(OUT, "/usr/ucb/lpr -P$printer -h $savefile|");
$msg = <OUT>;
close(OUT);

print LOG "done\n";

if ($msg eq "Printer queue is disabled.\n") {
    open(FOO, $holdfile);
    close(FOO);
    print LOG "got expected queue down error\n";
}

exit 0;
