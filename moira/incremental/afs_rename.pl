#!/usr/athena/bin/perl
# Usage: afs_change \
# 		oldname oldcell oldtype oldltype oldpath \
# 		newname newcell newtype newltype newpath

require "/moira/bin/afs_utils.pl";

die "$0: Incorrect number of arguments\n" if (@ARGV != 10);

($oldname, $oldcell, $oldtype, $oldltype, $oldpath,
 $newname, $newcell, $newtype, $newltype, $newpath) =
    @ARGV;

# Modify the paths, as only the read-write paths are used.
$oldpath =~ s:^/afs/([^.]):/afs/.\1:;	# use read-write path
$newpath =~ s:^/afs/([^.]):/afs/.\1:;	# use read-write path

die "Cannot change cells\n" if ($oldcell ne $newcell);
die "Can only handle AFS and ERR lockers\n"
    if (($oldtype !~ /^(AFS|ERR)$/) ||
	($newtype !~ /^(AFS|ERR)$/));

# Lookup volume type
($c = $newcell) =~ s/\./_/g;
$vtype = eval "\$vtypes_${c}{$newltype}";
die "Cannot handle $newltype volumes\n" unless $vtype;
$newvname = $vtype . "." . $newname;

if ($oldtype eq "ERR") {
    # If volume was never unmounted, we don't need to do anything.
    system("$fs lsm $oldpath > /dev/null");
    exit(0) if ($? == 0);

    # Lookup volume type for old locker
    ($c = $oldcell) =~ s/\./_/g;
    $vtype = eval "\$vtypes_${c}{$oldltype}";
    die "Cannot handle $oldltype volumes\n" unless $vtype;
    $oldvname = $vtype . "." . $oldname;
    $oldvname =~ s/[^-A-Za-z0-9_.]//g;	# strip out illegal characters

    if (&check("X" . $oldvname) && &check("Xn." . $oldvname)) {
	print STDERR "Cannot locate deactivated locker $oldname\n";
	exit(1) if ($newtype eq "AFS");
	exit(0);
    }
    $newvname = "n." . $newvname if ($oldvname =~ /^Xn\./);
} else {
    $prefix = "";

    open(FS, "$fs lsm $oldpath|");
    chop($_ = <FS>);
    close(FS);
    die "Unable to locate locker $oldname\n" if ($?);
    ($oldvname = $_) =~ s/^.* volume '.(.*)'$/\1/;
    die "Unusual mountpoint encountered: $oldpath\n" if ($oldvname =~ /[ :]/);
    $newvname = "n." . $newvname if ($oldvname =~ /^n\./);
}

$newvname = "X" . $newvname if ($newtype eq "ERR");
$newvname =~ s/[^-A-Za-z0-9_.]//g;	# strip out illegal characters

if ($oldbackup && $newvname =~ /^n\./) {
    system("$vos remove $oldbackup $oldvname.backup -cell $oldcell");
}
if ($oldvname ne $newvname) {
    &run("$vos rename $oldvname $newvname -cell $newcell");
    push(@clean, "$vos rename $newvname $oldvname -cell $newcell");
}

if ($oldtype eq "AFS") {
    &run("$fs rmm $oldpath");
    push(@clean, "$fs mkm $oldpath $oldvname");
    &release_parent($oldpath)
	if ($newtype ne "AFS" || $oldpath ne $newpath);
}
if ($newtype eq "AFS") {
    &run("$fs mkm $newpath $newvname");
    push(@clean, "$fs rmm $newpath");
    &release_parent($newpath);
}

&do_releases;
exit;


sub run
{
    local(@cmd) = @_;

    system("@cmd >/dev/null");
    &fatal("@cmd: FAILED") if ($?);
    return 0;
}


sub fatal
{
    local($cmd);
    $_ = join(' ',@_);
    s/\n$//;

    while (@clean) {
	$cmd = shift(@clean);
	warn "$newname: Cleanup failed: $cmd\n" if (system("$cmd"));
    }
    die "$newname: $_\n";
}


sub check
{
    local($vname) = @_;
    local(@vos, @a);

    open (VOS, "$vos listvldb -name $vname -cell $oldcell 2>/dev/null|");
    chop(@vos = <VOS>);
    close(VOS);
    return 1 if ($?);

    $oldvname = $vname;
    @a = split(/\s+/, $vos[$[ + 4]);
    if (($_ = pop @a) eq "valid") {
	splice(@vos, 0, 6);
	for (@vos) {
	    ($oldbackup=$_) =~ s/^.*server (.*) partition (.*) RW .*$/\1 \2/
		if (/RW Site/);
	}
    }
    return 0;
}

sub release_parent
{
    local($p) = @_;

    $p =~ s:/[^/]+$::;
    open(FS, "$fs lv $p|") || &fatal("Can't get information about $p");
    chop($_ = <FS>);
    close(FS);
    return if ($?);

    local(@tmp) = (split(/ /,$_));
    push(@vrelease, "$tmp[$#tmp] -cell $newcell");
}


sub do_releases
{
    local($lastv) = "";
    local(@volumes) = sort @vrelease;
    while (@volumes) {
	$_ = shift(@volumes);
	next if ($_ eq $lastv);
	system("$vos release $_");
    }
}
