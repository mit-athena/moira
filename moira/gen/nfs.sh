#!/bin/csh -f
set path=(/bin /usr/bin /etc /usr/etc )
set TARFILE=/tmp/nfs
set TMPDIR=/tmp/nfs.dir

if ( ! -d $TMPDIR ) then
	rm -f $TMPDIR
	mkdir $TMPDIR
	chmod 755 $TMPDIR
endif

cd $TMPDIR
tar xf $TARFILE
if ($status) exit 1

set uchost=`hostname|tr a-z A-Z`.MIT.EDU
set uchostrev=`echo $uchost | rev`

foreach i ( ${uchost}* )
	set t1=`echo $i | rev`
	set dev=`basename $t1 :$uchostrev | rev | sed 's;@;/;g'`

	echo ${uchost}:$dev

	./install_fs $dev < $i
end

# build new credentials file.
rm -f /usr/etc/credentials.new
cp credentials /usr/etc/credentials.new
if ($status) exit 1

/usr/etc/mkcred /usr/etc/credentials.new
if ($status) exit 1

foreach e ( "" .dir .pag)
	mv /usr/etc/credentials.new$e /usr/etc/credentials$e
end

