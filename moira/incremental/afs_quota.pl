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

shift(@tmp), $_ = shift(@tmp);
($asrv,$asrv,$apart) = split(/\s+/,$_);

system("$fs","sq",$path,$newq);
die "Unable to change quota on $path\n" if ($?);
&afs_quota_adj($cell,$asrv,$apart,$newq-$oldq);

#$stat_fs = $?;
#$stat_qa = &afs_quota_adj($cell,$asrv,$apart,($stat_fs ? 0 : $newq-$oldq));
#die "Unable to change quota on $path\n" if ($stat_fs && $stat_qa);
#warn "Unable to change quota on $path\n" if ($stat_fs);
