#!/moira/bin/perl
# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/letter.pl,v 1.1 1992-08-06 17:22:51 mar Exp $
# This script prints the letters confirming registration for the extra
# kerberos principal.

# The algorithm consists of first checking to see that the previous 
# day's job printed.  If so, the cached copy is deleted.  Otherwise,
# today's is appended to yesterday's and we continue working with that.

$printer = "linus";
$newfile  = "/tmp/letter.out";
$savefile = "/u1/letter.save";
$holdfile = "/u1/letter.hold";

$printed =`lpquota -l | tail -2`;
@printed = split(/\s+/, $printed);

$filed = `ls -l $savefile`;
@filed = split(/\s+/, $filed);

if ($printed[1] eq $filed[4] && $printed[2] eq $filed[5]) {
    system("mv $newfile $savefile");
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
	}
	system("cat $newfile >> $savefile");
    } else {
	system("mv $newfile $savefile");
    }
}

unlink($holdfile);

open(OUT, "lpr -P$printer -h $savefile|");
$msg = <OUT>;
close(OUT);

if ($msg eq "Printer queue is disabled.") {
    open(FOO, $holdfile);
    close(FOO);
}

exit 0;
