#!/afs/athena/contrib/perl/perl
# Usage: afs_create locker type cell path quota user group

require "/moira/bin/afs_utils.pl";

$protodir="/moira/dotfiles";
$quota=1;

%vtypes_ATHENA_MIT_EDU =
    ("ACTIVITY", "activity",
     "APROJ", "aproj",
     "AREF", "aref",
     "CONTRIB", "contrib",
     "COURSE", "course",
     "HOMEDIR", "user",
     "PROJECT", "project",
     "REF", "ref",
     "SW", "sw",
     "SYSTEM", "sw",
     );

%proc =
    ("ATHENA.MIT.EDU", 'athena_proc' );

umask(0);

die "Usage: $0 locker type cell path user group\n" if (@ARGV != 6);
($locker,$type,$cell,$path,$user,$group) = @ARGV;

# Lookup volume type
($c = $cell) =~ s/\./_/g;
$vtype = eval "\$vtypes_$c{$type}";
die "Cannot create $type volumes in $cell\n" unless $vtype;
$vname = $vtype . "." . $locker;

# Find free space
($asrv,$apart) = &afs_find($cell,$type,$quota);
die "Unable to find space to create $vname in $cell\n" unless ($asrv&&$apart);

# Create volume
system("$vos create $asrv $apart $vname -cell $cell >/dev/null") &&
    &fatal("Unable to create $vname in $cell");
push(@clean, "$vos remove $asrv $apart $vname -cell $cell >/dev/null");

# Create mountpoint and set quota
$path =~ s:^/afs/([^.]):/afs/.\1:;
system("$fs checkb >/dev/null; $fs mkm $path $vname");
&fatal("Unable to create $path") if ($?);
push(@clean, "$fs rmm $path");

# Obtain user/group information
$uid = $gid = 0;
open(PTS, "$pts ex $user -cell $cell|");
chop($_ = <PTS>);
close(PTS);
($uid,$uid,$uid,$uid) = split(/[:,] /, $_) unless ($?);

open(PTS, "$pts ex system:$group -cell $cell|");
chop($_ = <PTS>);
close(PTS);
($gid,$gid,$gid,$gid) = split(/[:,] /, $_) unless ($?);
$gid = 0-$gid;

# Dispatch to the cell-specific creation routines
eval "&$proc{$cell}";
&fatal($@) if ($@);

# Set the filesystem quota
system("$fs sq $path $quota");
&fatal("Unable to set the quota on $path") if ($?);

# Release the parent volume
($p = $path) =~ s:/[^/]+$::;
open(FS, "$fs lv $p|") || &fatal("Can't get information about $p");
chop($_ = <FS>);
close(FS);
&fatal("Can't get information about $p") if ($?);
@tmp = (split(/ /,$_));
system("$vos release $tmp[$#tmp] -cell $cell >/dev/null") &&
    &fatal("Can't release $tmp[$#tmp] in cell $cell");

&afs_quota_adj($cell,$asrv,$apart,$quota);
exit(0);

sub fatal
{
    local($cmd);
    $_ = join(' ',@_);
    s/\n$//;

    while (@clean) {
	$cmd = pop(@clean);
	warn "$locker: Cleanup failed: $cmd\n" if (system("$cmd"));
    }
    die "$locker: $_\n";
}

# Cell specific procedures
sub athena_proc
{
    # Default acls:
    #
    # ACTIVITY  <user> all <group> all system:anyuser rl
    # APROJ     <user> all <group> all system:anyuser rl
    # AREF      <user> all <group> rl
    # CONTRIB   <user> all system:anyuser rl
    # COURSE    <user> all <group> all system:facdev all system:authuser rl
    # HOMEDIR   <user> all
    # PROJECT   <user> all <group> all
    # REF       <user> all system:anyuser rl
    # SW        <user> all system:swmaint all system:authuser rl
    # SYSTEM    system:administrators all system:anyuser rl
    #
    # Notes:
    # 1. All directories also have "system:expunge ld".

    @acl=("system:expunge ld");
    push(@acl,"system:facdev all") if ($type =~ /^(COURSE)/);
    push(@acl,"system:swmaint all") if ($type =~ /^(SW)/);
    push(@acl,"system:administrators all") if ($type =~ /^(SYSTEM)/);
    push(@acl,"$user all")
	if ($uid != 0 && $type =~ /^(ACTIVITY|APROJ|AREF|CONTRIB|COURSE|HOMEDIR|PROJECT|REF|SW)/);
    push(@acl,"system:$group all")
	if ($gid != 0 && $type =~ /^(ACTIVITY|APROJ|COURSE|PROJECT)/);
    push(@acl,"system:$group rl") if ($gid != -1 && $type =~ /^(AREF)/);
    push(@acl,"system:authuser rl")
	if ($type =~ /^(COURSE|SW)/);
    push(@acl,"system:anyuser rl")
	if ($type =~ /^(ACTIVITY|APROJ|CONTRIB|REF|SYSTEM)/);

    if ($type !~ /^(AREF|SYSTEM)/) {
	system("$fs mkm $path/OldFiles $vname.backup");
	warn "$locker: Unable to create OldFiles mountpoint\n" if ($?);
    }

    if ($type eq "HOMEDIR") {
	die "Unable to get uid for user\n" unless ($uid);

	mkdir("$path/Public",0755) && chown($uid,0,"$path/Public") &&
	    mkdir("$path/Private",0700) && mkdir("$path/Mail", 0700) &&
		chown($uid,0,"$path/Public","$path/Private","$path/Mail") ||
		    die "Unable to create subdirectories\n";
	system("$fs sa $path/Public @acl system:anyuser rl -clear") &&
	    die "Unable to set acl on Public directory";
	system("$fs sa -dir $path/Private $path/Mail -acl @acl -clear") &&
	    die "Unable to set acl on Private and/or Mail directories\n";

	opendir(DIR,$protodir) || die "Unable to open prototype directory\n";
	@files=readdir(DIR);
	closedir(DIR);

	for $i (@files) {
	    next if ($i eq "." || $i eq "..");
	    next unless -f "$protodir/$i";
	    open(IN,"<$protodir/$i") || die "Unable to open $protodir/$i\n";
	    open(OUT,">$path/$i") || die "Unable to create $i\n";
	    while ($_=<IN>) { print OUT $_; };
	    close(OUT);
	    close(IN);
	    chown($uid,0,"$path/$i");
	}
	chown($uid,0,$path) && 
	    system("$fs sa $path @acl system:anyuser l -clear") &&
		die "Unable to set acl on top-level directory\n";
	return;
    }

    system("$fs sa $path @acl -clear") &&
	die "Unable to set acl of $path\n";
}
