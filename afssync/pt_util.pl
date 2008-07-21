#!/usr/athena/bin/perl

while ($_ = <STDIN>) {
    if (/^\s/) {
	$members{$who} = $members{$who} . $_;
	$mcount{$who}++;
	next;
    }
    ($f,$f,$who,$own) = split(/\s+/, $_);
    $data{$who} = $_;
}

# Do system:authuser@cell groups
for (keys %data) {
    next unless ($data{$_} =~ /^system:authuser@/);
    ($name, $attr, $id, $oid, $cid) = split(/\s+/, $data{$_});
    ($flags, $mcount) = split(/\//, $attr);
    $mcount += $mcount{$_};
    print "$name $flags/$mcount $id $oid $cid\n";
    delete $data{$_};
}

# Do users
for (keys %data) {
    next if ($_ < 0);
    print $data{$_};
    delete $data{$_};
}

# Now do all the lists
$x = 1;
while ((@keys = keys %data)+0 && $x) {
    $x = 0;
    for $key (keys %data) {
	($f,$f,$who,$own) = split(/\s+/, $data{$key});
	next if ($who!=$own && $data{$own});
	$x = 1;
	print $data{$key};
	print $members{$key} if ($members{$key});
	delete $data{$key};
	delete $members{$key} if ($members{$key});
    }
}
if (@keys+0) {
    print STDERR "Unprocessed:\n";
    for (keys %data) {
	$f = split(/\s+/, $data{$_});
	print STDERR "\t$f ($_)\n";
    }
}
