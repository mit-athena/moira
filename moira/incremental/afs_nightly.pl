#!/afs/athena/contrib/perl/perl

require "/moira/bin/afs_utils.pl";

#
# Nightly reaper - Recompute all usages/allocations.
#

local ($key, @vos, %used,%alloc,%total, %a);
local ($c,$as,$ap,$total,$used,$alloc);

&afs_lock;
truncate(SRV, 0);

if (open(TMP,"/afs/athena/service/afs_data")) {
    @afs_data = <TMP>;
    close(TMP);
}

chop(@afs_data);
for (@afs_data) {
    $as = $ap = 0;
    if ($_ !~ /^\#/) {
	($c,$as,$ap,$t,$total,$used,$alloc) = split(/\s+/,$_);
	($as) = gethostbyname($as);
	$c =~ tr/a-z/A-Z/;
	$as =~ tr/a-z/A-Z/;
	$ap =~ s:^([^/]):/vicep\1:;
    }
    if (!$as || !$ap) {
	print SRV "$_\n"; next;
    }
    
    $key = "$c $as $ap";
    $total{$key} = $used{$key} = $alloc{$key} = 0;
    $type{$key} = $t;
    
    open(VOS,"$vos partinfo $as $ap -cell $c -noauth|");
    chop(@vos = <VOS>);
    close(VOS);
    next if ($?);
    @vos = split(/\s+/,$vos[0]);
    $total{$key} = pop(@vos);
    $used{$key} = $total{$key}-$vos[5];
    
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
    for $p (keys %a) { $alloc{$key} += $a{$p}; delete $a{$p}; }
}

for (sort keys %total) {
    print SRV "$_ $type{$_} $total{$_} $used{$_} $alloc{$_}\n";
    delete $type{$_};
    delete $total{$_};
    delete $used{$_};
    delete $alloc{$_};
}

&afs_unlock;
