#!/afs/athena/contrib/perl/perl
# Usage: afs_quota path quota

require "/moira/bin/afs_utils.pl";

die "Usage: $0 path quota\n" unless (@ARGV==2);

($path,$newq) = @ARGV;
$path =~ s:^/afs/([^.]):/afs/.\1:;

system("$fs checkb >/dev/null");

open(PROG,"$fs lv $path|");
chop(@fs=<PROG>);
close(PROG);
die "Unable to get information about $path\n" if ($?);

@tmp = split(/\s+/, shift(@fs));
$vname = pop(@tmp);
@tmp = split(/\s+/, shift(@fs));
$oldq = pop(@tmp);

open(PROG,"$fs whichcell $path|");
chop($_=<PROG>);
close(PROG);
die "Unable to get information about $path\n" if ($?);

@tmp=split(/\s+/,$_);
($cell=pop(@tmp)) =~ s/^'(.*)'$/\1/;
$cell =~ tr/a-z/A-Z/;

open(PROG,"$vos ex $vname -cell $cell|");
chop(@tmp=<PROG>);
close(PROG);
die "Unable to get volume information about $vname ($path)\n" if ($?);

$_ = shift(@tmp);
($vusage, $vusage, $vusage, $vusage) = split(/\s+/,$_);
$_ = shift(@tmp);
($asrv,$asrv,$apart) = split(/\s+/,$_);

open(PROG,"$vos partinfo $asrv $apart -cell $cell|");
chop($_ = <PROG>);
close(PROG);
die "Unable to get partition information about $asrv $apart\n" if ($?);

@tmp = split(/\s+/,$_);
$dtotal = $tmp[11];
$dusage = $dtotal - $tmp[5];

system("$fs","sq",$path,$newq);
die "Unable to change quota on $path\n" if ($?);

&afs_quota_adj($cell,$asrv,$apart,$newq-$oldq,$dusage);

if ($dusage > (0.9 * $dtotal)) {
    system("$zwrite","-q","-n","-c","afsadm","-i","moira","-m",
	   "WARNING: Disk usage on $asrv:$apart is greater than 90%
Used $dusage K out of $dtotal K");
}
elsif (($dusage + ($newq-$vusage)) > (0.9 * $dtotal)) {
    system("$zwrite","-q","-n","-c","afsadm","-i","moira","-m",
	   "Quota change on volume $vname, if used completely will cause
$asrv:$apart to be over 90% used.

Current volume usage: $vusage, old quota $oldq, new quota $newq");
}
elsif ($newq-$oldq > 999999) {
    system("$zwrite","-q","-n","-c","afsadm","-i","moira","-m",
	   "Increased quota on volume $vname ($asrv:$apart) from $oldq to $newq");
}
