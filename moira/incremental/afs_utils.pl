$afsbin="/moira/bin";
$vos="$afsbin/vos";
$pts="$afsbin/pts";
$fs="$afsbin/fs";

$afs_data="/moira/afs_data";
$afs_save="$afs_data.tmp";

$LOCK_EX=2;
$LOCK_UN=8;

# File format:
#    cell server partition total used alloc

# Locking/re-write algorithm:
# 1. Open the data file.
# 2. Obtain a lock on the data file.
# 3. Check for the existence of a temporary data file - die if it exists.
# 4. Save current contents into temporary data file.
# 5. Re-write output (with line-buffering).
# 6. Unlink temporary file.
# 7. Unlock data file.
# 8. Close the data file.


sub afs_lock
{
    open(SRV,"+<$afs_data") || die "Unable to open $afs_data\n";
    select((select(SRV), $|=1)[$[]);
    flock(SRV, $LOCK_EX) || die "Unable to lock $afs_data\n";
    die "Temporary status file: $afs_save exists... abortin\n"
	if (-f $afs_save);
    open(SRV2, ">$afs_save");
    @afs_data = <SRV>;
    print SRV2 @afs_data;
    close(SRV2);
    seek(SRV, 0, 0);
}

sub afs_unlock
{
    unlink($afs_save);
    close(SRV);
}

# Find server/partition for allocation.
#
# Best fit algorithm used:
#    max[ (2*free space) - (unused quota) ]
#    = max(2*total - usage - alloc)
#
# Note: This routine does not actually adjust the quota; the caller
# should use afs_quota_adj();

sub afs_find
{
    local($cell,$type,$quota) = @_;
    local($j);
    local(@max);

    &afs_lock;
    chop(@afs_data);

    for (@afs_data) {
	local ($a, $asrv, $apart, $t, $total, $used, $alloc) = split(/\s+/,$_);
	next if ($a ne $cell || !$total || $type !~ /$t/);
	$j = 2*$total - $used - $alloc;
	@max = ($asrv,$apart,$j) if (! @max || $j > $max[2]);
    }

#    truncate(SRV, 0);
#    for (@afs_data) {
#	($a, $asrv, $apart, $t, $total, $used, $alloc) = split(/\s+/,$_);
#	if ($a eq $cell && $asrv eq $max[0] && $apart eq $max[1]) {
#	    $alloc += $quota;
#	    $_ = join(' ',$a,$asrv,$apart,$t, $total,$used,$alloc);
#	}
#	print SRV "$_\n";
#    }

    &afs_unlock;
    return(@max);
}

#
# Quota adjustments
#
sub afs_quota_adj
{
    local($cell,$asrv,$apart,$adj) = @_;

    &afs_lock;
    chop(@afs_data);
    truncate(SRV, 0);
    for (@afs_data) {
	local ($c, $as, $ap, $t, $total, $used, $alloc) = split(/\s+/,$_);
	if ($c eq $cell && $as eq $asrv && $ap eq $apart) {
	    $alloc += $adj;
	    $_ = join(' ',$c,$asrv,$apart,$t,$total,$used,$alloc);
	}
	print SRV "$_\n";
    }
    &afs_unlock;
}


#
# Nightly reaper - Recompute all usages/allocations.
#
sub afs_nightly
{
    local ($key, @vos, %used,%alloc,%total, %a);
    local ($c,$as,$ap,$total,$used,$alloc);

    &afs_lock;
    truncate(SRV, 0);
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
}
