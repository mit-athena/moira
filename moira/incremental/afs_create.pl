#!/usr/athena/bin/perl
# Usage: afs_create locker type cell path quota user group

require "/moira/bin/afs_utils.pl";

$protodir="/moira/dotfiles";
$quota=1;

%proc =
    ("ATHENA.MIT.EDU", 'athena_proc' );

umask(0);

die "Usage: $0 locker type cell path user group\n" if (@ARGV != 6);
($locker,$type,$cell,$path,$user,$group) = @ARGV;

# Lookup volume type
($c = $cell) =~ s/\./_/g;
$vtype = eval "\$vtypes_${c}{$type}";
die "Cannot create $type volumes in $cell\n" unless $vtype;
$vname = $vtype . "." . $locker;
$vname =~ s/[^-A-Za-z0-9_.]//g;		# strip out illegal characters

# Find free space/Create volume
$tries = 0; $code = 1;
while ($tries<3 && $code) {
    ($asrv,$apart) = &afs_find($cell,$type,$quota,@except);
    die "Unable to find space to create $vname in $cell\n" unless ($asrv&&$apart);
    $code = system("$vos create $asrv $apart $vname -cell $cell >/dev/null");
    push(@except, $asrv);
    $tries++;
}
&fatal("Unable to create $vname in $cell") if ($code); # Too many create errors
push(@clean, "$vos remove $asrv $apart $vname -cell $cell >/dev/null");

# Create mountpoint and set quota
$path =~ s:^/afs/([^.]):/afs/.\1:;
system("$fs checkv >/dev/null; $fs mkm $path $vname");
&fatal("Unable to create $path") if ($?);
push(@clean, "$fs rmm $path");

# Obtain user/group information (uid >= 0, gid <= 0)
$uid = $gid = 0;
open(PTS, "$pts ex $user -cell $cell|");
chop($_ = <PTS>);
close(PTS);
($uid,$uid,$uid,$uid) = split(/[:,] /, $_) unless ($?);

open(PTS, "$pts ex system:$group -cell $cell|");
chop($_ = <PTS>);
close(PTS);
($gid,$gid,$gid,$gid) = split(/[:,] /, $_) unless ($?);

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
if ($tmp[$#tmp] !~ /user\../) {
    system("$vos release $tmp[$#tmp] -cell $cell >/dev/null") &&
	&fatal("Can't release $tmp[$#tmp] in cell $cell");
}

# Update the quota records.
&afs_quota_adj($cell,$asrv,$apart,$quota,0);
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
    # LEASE	<user> all
    # ORG	<user> all <group> all system:cwisfac all system:anyuser rl
    # PROJECT   <user> all <group> all
    # REF       <user> all system:anyuser rl
    # SW        <user> all system:swmaint all system:authuser rl
    # SYSTEM    system:administrators all system:anyuser rl
    # UROP	<user> all <group> all system:facdev all system:authuser rl
    #
    # Notes:
    # 1. All directories also have "system:expunge ld".

    @acl=("system:expunge ld");
    push(@acl,"system:facdev all") if ($type =~ /^(COURSE|UROP)/);
    push(@acl,"system:swmaint all") if ($type =~ /^(SW)/);
    push(@acl,"system:cwisfac all") if ($type =~ /^(ORG)/);
    push(@acl,"system:administrators all") if ($type =~ /^(SYSTEM)/);
    push(@acl,"$user all")
	if ($uid != 0 && $type =~ /^(ACTIVITY|APROJ|AREF|CONTRIB|COURSE|HOMEDIR|LEASE|ORG|PROJECT|REF|SW|UROP)/);
    push(@acl,"system:$group all")
	if ($gid != 0 && $type =~ /^(ACTIVITY|APROJ|COURSE|ORG|PROJECT|UROP)/);
    push(@acl,"system:$group rl") if ($gid != 0 && $type =~ /^(AREF)/);
    push(@acl,"system:authuser rl")
	if ($type =~ /^(COURSE|SW|UROP)/);
    push(@acl,"system:anyuser rl")
	if ($type =~ /^(ACTIVITY|APROJ|CONTRIB|ORG|REF|SYSTEM)/);

    if ($type !~ /^(AREF|ORG|SYSTEM)/) {
	system("$fs mkm $path/OldFiles $vname.backup");
	warn "$locker: Unable to create OldFiles mountpoint\n" if ($?);
    }

    if ($type =~ /ACTIVITY|APROJ|PROJECT/) {
	system("/moira/bin/uchown $path $gid");
	die "Unable to set volume ownership\n" if ($?);
    } elsif ($type =~ /HOMEDIR|UROP/) {
	chown($uid,0,$path) ||
	    die "Unable to set volume ownership\n";
    }

    if ($type eq "COURSE") {
	mkdir("$path/www",0755) &&
	    chown(0,0,"$path/www") ||
		die "Unable to create subdirectories\n";
	system("$fs sa $path/www @acl system:anyuser rl -clear") &&
	    die "Unable to set acl on www directory\n";

	system("$fs sa $path @acl system:anyuser l -clear") &&
	    die "Unable to set acl on top-level directory\n";
	return;
    }

    if ($type eq "HOMEDIR") {
	die "Unable to get uid for user\n" unless ($uid);

	chmod(0755, $path);
	mkdir("$path/Public",0755) && mkdir("$path/www",0755) &&
	    mkdir("$path/Private",0700) && mkdir("$path/Mail", 0700) &&
		chown($uid,0,"$path/Public","$path/www",
		      "$path/Private","$path/Mail") ||
			  die "Unable to create subdirectories\n";
	system("$fs sa -dir $path/Public $path/www -acl @acl system:anyuser rl -clear") &&
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
	system("$fs sa $path @acl system:anyuser l -clear") &&
	    die "Unable to set acl on top-level directory\n";
	return;
    }

    system("$fs sa $path @acl -clear") &&
	die "Unable to set acl of $path\n";
}
