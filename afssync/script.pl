#!/moira/bin/perl

$host = "maeander";
$ptdump = "/var/local/ptdump";
$skip_to_members = 0;

# make sure process is unlimited
`csh -fc 'limit datasize'` =~ /datasize\s*(\w*) kbytes/;
if ($1 < 10000) {
    die "Must 'unlimit' before running this script!\n";
}

&startquel;
&quelcmd("set autocommit on");

if (!$skip_to_members) {
ptdumpagain:
open(AUSERS, "rsh $host $ptdump 2>&1 |") || die "Couldn't get to $host\n";

while (<AUSERS>) {
    if (/Ubik Version number changed during execution/) {
	close AUSERS;
	undef %AUSERS;
	print "AFS database changed, trying again.\n";
	goto ptdumpagain;
    }
    if (/Name: (\w*) ID: (\d*)/ && $2<32767) {
	$AUSERS{$1} = $2;
    }
}

close(AUSERS);
print "Got the users from AFS\n";

$AUSERS{"nobody"} = 32767;
delete $AUSERS{"anonymous"};
#$AUSERS{"CLU"} = 60;
#delete $AUSERS{"clu"};
#$AUSERS{"Saltzer"} = 994;
#delete $AUSERS{"saltzer"};

@VAL = &quelcmd("select login, uid from users where status=1 or status=2");
printf("Got %d users from Moira\n", scalar(@VAL));

open(OUT, ">user.add") || die "Couldn't open output file\n";

while ($_ = shift @VAL) {
    tr/A-Z/a-z/;
    if ($_ =~ /\|(\w*)\s*\|\s*(\d*)\|/) {
	if ($AUSERS{$1} == $2) {
	    delete $AUSERS{$1};
	} else {
	    print OUT "/moira/bin/pts createuser -name $1 -id $2\n";
	}
    }
}

close OUT;
open(OUT, ">user.del") || die "Couldn't open output file\n";

foreach $login (keys %AUSERS) {
    print OUT "/moira/bin/pts delete $login\n";
}

close OUT;
undef @VAL;
undef %AUSERS;
print "Done processing users\n";

groupagain:
open(AGROUP, "rsh $host $ptdump -g 2>&1 |") || die "Couldn't get to $host\n";

while (<AGROUP>) {
    if (/Ubik Version number changed during execution/) {
	close AGROUP;
	undef %AGROUP;
	print "AFS database changed, trying again.\n";
	goto groupagain;
    }
    if (/Name: system:(\w*) ID: -(\d*)/ && $2<32767) {
	$AGROUP{$1} = $2;
    }
}

close(AGROUP);
print "Got the groups from AFS\n";

$AGROUP{"mit"} = 101;
delete $AGROUP{"anyuser"};
#$AGROUP{"CLU"} = 184;
#delete $AGROUP{"clu"};
#$AGROUP{"Saltzer"} = 185;
#delete $AGROUP{"saltzer"};

@VAL = &quelcmd("select name, gid from list where active=1 and grouplist=1");
printf("Got %d groups from Moira\n", scalar(@VAL));
open(OUT, ">group.add") || die "Couldn't open output file\n";

while ($_ = shift @VAL) {
    tr/A-Z/a-z/;
    if ($_ =~ /\|(\w*)\s*\|\s*(\d*)\|/) {
	if ($AGROUP{$1} == $2) {
	    delete $AGROUP{$1};
	} else {
	    print OUT "/moira/bin/pts creategroup -name system:$1 -owner system:administrators -id -$2\n";
	}
    }
}

close OUT;
open(OUT, ">group.del") || die "Couldn't open output file\n";

foreach $group (keys %AGROUP) {
    print OUT "/moira/bin/pts delete system:$group\n";
}
close OUT;

undef @VAL;
undef %AGROUP;
print "Done processing groups\n";
}

memberagain:
open(AMEM, "rsh $host $ptdump -g -c 2>&1 |") || die "Couldn't get to $host\n";

while (<AMEM>) {
    if (/Ubik Version number changed during execution/) {
	close AMEM;
	undef @AMEM;
	print "AFS database changed, trying again.\n";
	goto memberagain;
    }
    if (/Group: system:([^\s]*)/) {
	$list = $1;
    } elsif (/Group: /) {
	$list = "";
    } elsif (/Member:  ([^\s.]*)$/) {
	if ($list) {
	    push(@AMEM, $list . " " . $1);
	}
    } elsif (/Member\(co\):  ([^\s.]*)$/) {
	if ($list) {
	    push(@AMEM, $list . " " . $1);
	}
    }
}

close(AMEM);
printf("Got %d members from AFS\n", scalar(@AMEM));

@VAL = &quelcmd("select l.name, u.login from list l, users u, imembers m where l.list_id=m.list_id and u.users_id=m.member_id and m.member_type='USER' and l.grouplist=1 and l.active=1 and (u.status=1 or u.status=2)");
printf("Got %d members from Moira\n", scalar(@VAL));

while ($_ = shift @VAL) {
    tr/A-Z/a-z/;
    if ($_ =~ /\|([^\s\|]*)\s*\|([^\s\|]*)\s*\|/) {
	push(@MMEM, $1 . " " . $2);
    }
}

#  throw away column headings
pop @MMEM;
undef @VAL;
@AMEM1 = sort @AMEM;
@MMEM1 = sort @MMEM;

open(OUT, ">member.del") || die "Couldn't open output file\n";
open(OUT1, ">member.add") || die "Couldn't open output file\n";

while ($#AMEM1 + $#MMEM1 > 0) {
    if ($AMEM1[0] eq $MMEM1[0]) {
	shift @AMEM1;
	shift @MMEM1;
    } elsif ($AMEM1[0] lt $MMEM1[0] && $#AMEM1 > 0) {
	($list, $user) = split(/ /, shift @AMEM1);
	print OUT "/moira/bin/pts removeuser -user $user -group system:$list\n";
    } elsif ($MMEM1[0] lt $AMEM1[0] && $#MMEM1 > 0) {
	($list, $user) = split(/ /, shift @MMEM1);
	if (($user eq "login" && $list eq "name") || $user eq "root") {
	    next;
	}
	print OUT1 "/moira/bin/pts adduser -user $user -group system:$list\n";
    } else {
	last;
    }
}

close OUT;
close OUT1;

print "Done processing members.\n";

system("cat member.del group.del user.del user.add group.add member.add > fixit");

&stopquel;
exit 0;


sub startquel {
	$SIG{'INT'} = $SIG{'TERM'} = $SIG{'HUP'} = 'handler';
	$ENV{"TERM"} = "glass";
	pipe(CREAD, QUELIN);
	pipe(QUELOUT, COUT);
	select(QUELIN), $|=1;
	select(STDOUT);
	$quelpid = fork;
	if (! $quelpid) {
	    select(COUT), $|=1;
	    open(STDOUT, ">&COUT");
	    open(STDERR, ">/dev/null");
	    open(STDIN, "<&CREAD");
	    exec "sql moira";
	    exit(1);
	}
	close(CREAD);
	close(COUT);
}

sub quelcmd {
	local (@RETVAL);
	print QUELIN $_[0], "\\g\n";
	while (($_ = <QUELOUT>) !~ /Executing \. \. \./) { next; }
	while (($_ = <QUELOUT>) !~ /^continue$/) { push(@RETVAL, $_); }
	return @RETVAL;
}

sub stopquel {
	print QUELIN "\\q\n";
	close QUELIN;
	close QUELOUT;
}

sub  handler {
	local($sig) = @_;
	&stopquel;
	kill "HUP", $quelpid;
	print("Caught a signal $sig\n");
	exit(1);
}
