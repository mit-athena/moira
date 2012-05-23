# $HeadURL$ $Id$

use Fcntl;

$aklog="/bin/athena/aklog";
if ( ! -x $aklog ) { $aklog="/usr/bin/aklog"; }

system("$aklog");

$afsbin="/moira/bin";
$vos="$afsbin/vos";
$pts="$afsbin/pts";
$fs="$afsbin/fs";
$zwrite="/usr/athena/bin/zwrite";
if ( ! -x $zwrite ) { $zwrite="/usr/local/bin/zwrite"; }

$afs_data="/moira/afs/afs_data";
$afs_save="$afs_data.tmp";

%vtypes_ATHENA_MIT_EDU =
    ("ACTIVITY", "activity",
     "APROJ", "aproj",
     "AREF", "aref",
     "CONTRIB", "contrib",
     "COURSE", "course",
     "HOMEDIR", "user",
     "LEASE", "dept",
     "ORG", "org",
     "PROJECT", "project",
     "REF", "ref",
     "SITE", "site",
     "SW", "sw",
     "SYSTEM", "system",
     "UROP", "urop",
     );

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


$flock_t="ssllllllll";

sub afs_lock
{
    open(SRV,"+<$afs_data") || die "Unable to open $afs_data\n";
    select((select(SRV), $|=1)[$[]);
    $flkarr[0]=&F_WRLCK;
    $flkarr[1]=$flkarr[2]=$flkarr[3]=$flkarr[4]=$flkarr[5]=$flkarr[6]=0;
    $flkarr[7]=$flkarr[8]=$flkarr[9]=0;
    $flk=pack($flock_t,@flkarr);
    fcntl(SRV, &F_SETLKW, $flk) || die "Unable to lock $afs_data:$!\n";
    die "Temporary status file: $afs_save exists... aborting\n"
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
# Note: This routine does not actually adjust the quota;
# the calling routine should use afs_quota_adj();

sub afs_find
{
    local($cell,$type,$quota,@except) = @_;
    local($j,$k);
    local(@max) = ("", "", undef);

    &afs_lock;
    chop(@afs_data);

  sloop:
    for (@afs_data) {
	local ($a, $asrv, $apart, $t, $total, $used, $alloc) = split(/\s+/,$_);
	next if ($a ne $cell || !$total || $type !~ /$t/);
	for $j (@except) {
	    next sloop if ($j eq $asrv);
	}
	$alloc = $used if ($alloc < $used);
	$j = 2*$total - $used - $alloc;
	@max = ($asrv,$apart,$j) if (!$max[2] || $j > $max[2]);
    }

    &afs_unlock;
    return(@max);
}

#
# Quota adjustments
#
sub afs_quota_adj
{
    local($cell,$asrv,$apart,$adj,$dusage) = @_;
    local($found) = 0;

    &afs_lock;
    chop(@afs_data);
    truncate(SRV, 0);
    for (@afs_data) {
	local ($c, $as, $ap, $t, $total, $used, $alloc) = split(/\s+/,$_);
	if ($c eq $cell && $as eq $asrv && $ap eq $apart) {
	    $dusage = $used unless ($dusage);
	    $alloc += $adj;
	    $_ = join(' ',$c,$asrv,$apart,$t,$total,$dusage,$alloc);
	    $found = 1;
	}
	print SRV "$_\n";
    }
    &afs_unlock;
    return($found);
}
