#!/afs/athena/contrib/perl/perl

require "/moira/bin/afs_utils.pl";

#
# Nightly reaper - Recompute all usages/allocations.
#

local ($key, @vos, %used,%alloc,%total, %a);
local ($c,$as,$ap,$total,$used,$alloc);

if (open(TMP,"/afs/athena.mit.edu/service/afs_data")) {
    @new_data = <TMP>;
    close(TMP);
}

chop(@new_data);
for (@new_data) {
    $as = $ap = 0;
    if ($_ !~ /^\#/) {
	($c,$as,$ap,$type) = split(/\s+/,$_);
	($as) = gethostbyname($as);
	$c =~ tr/a-z/A-Z/;
	$as =~ tr/a-z/A-Z/;
	$ap =~ s:^([^/]):/vicep\1:;
    }
    next unless ($as && $ap);
    
    &afs_lock;
    truncate(SRV, 0);
    for (@afs_data) {
	($c2,$as2,$ap2) = split(/\s+/,$_);
	print SRV $_ unless ($c eq $c2 && $as eq $as2 && $ap eq $ap2);
    }
    &afs_unlock;
    
    open(VOS,"$vos partinfo $as $ap -cell $c -noauth|");
    chop(@vos = <VOS>);
    close(VOS);
    next if ($?);
    @vos = split(/\s+/,$vos[0]);
    $total = pop(@vos);
    $used = $total-$vos[5];
    $alloc = 0;
    
    open(VOS,"$vos listvol $as $ap -cell $c -long -noauth|");
    @vos = <VOS>;
    close(VOS);
    next if ($?);
    
    while ($_ = shift(@vos)) {
	next unless /On-line/;
	local($vn,$id) = split(/\s+/, $_);
	shift(@vos);
	local($p,$p,$p) = split(/\s+/, shift(@vos));
	local($q,$q,$q) = split(/\s+/, shift(@vos));
	if ($id==$p || !$a{$p}) {
	    $a{$p} = $q;
	}
    }
    for $p (keys %a) { $alloc += $a{$p}; delete $a{$p}; }

    &afs_lock;
    seek(SRV, 0, 2);
    print SRV "$c $as $ap $type $total $used $alloc\n";
    &afs_unlock;
}
