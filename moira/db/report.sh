#! /bin/sh

PATH=/etc:/usr/ucb:/bin:/usr/bin:.:/etc/athena:/bin/athena

DATADIR=$1
TMPDIR=/tmp/report.tmp
mkdir ${TMPDIR}
cd ${TMPDIR}

awk -F\| '$8 != 3 {print $10} \
	($8==1 || $8==6) && $25=="POP" {pop += 1} \
	($8==1 || $8==6) && $25=="SMTP" {smtp += 1} \
	($8==1 || $8==6) && $25=="NONE" {nopob += 1} \
	END {	nopob -= 1; total = pop + smtp + nopob; \
		printf("%5d Active users by pobox:\n", total) > "pobox.summary"; \
		printf("\t%5d POP boxes\t\t%2d%%\n", pop, (100 * pop + total/2)/total) > "pobox.summary"; \
		printf("\t%5d foreign boxes\t%2d%%\n", smtp, (100 * smtp + total/2)/total) > "pobox.summary"; \
		printf("\t%5d without boxes\t%2d%%\n", nopob, (100 * nopob + total/2)/total) > "pobox.summary";} \
' < ${DATADIR}/users | sort | uniq -c | sort -r -n > ./users.all
awk -F\| '$8==1 {print $10}' < ${DATADIR}/users | sort | uniq -c | sort -r -n > ./users.active

cat ./users.active ./users.all | awk \
'	BEGIN {all=0; lastx=100000;}
	all==0 && lastx<$1 {all += 1} \
	all==0 && NF==2 {active[$2]=$1; lastx=$1} \
	all==1 && NF==2 {printf("\t%-8s %5d  %5d  %3d\n", $2, $1, active[$2], (100 * active[$2] + $1/2)/$1)} \
' | grep -v "%" | sort +2rn | (echo "	class    total active    %"; cat - -) > ./users.summary

awk '{total+=$2;active+=$3}\
END {printf("\t%-8s %5d  %5d\n", "Totals", total, active)}' < ./users.summary > ./users.summary1
cat users.summary1 >> users.summary

awk -F\| ' \
	NR != 1 {total += 1; active += $3; public += $4; \
		hidden += $5; maillist += $6; group += $7;} \
	END {	printf("%d Lists:\n", total); \
		printf("\t%5d active\t%2d%%\n", active, (100 * active)/total); \
		printf("\t%5d public\t%2d%%\n", public, (100 * public)/total); \
		printf("\t%5d hidden\t%2d%%\n", hidden, (100 * hidden)/total); \
		printf("\t%5d maillists\t%2d%%\n", maillist, (100 * maillist)/total); \
		printf("\t%5d groups\t%2d%%\n", group, (100 * group)/total);} \
' < ${DATADIR}/list > ./list.summary

awk '{total+=1}\
END {printf("%5d Clusters\n", total)}' < ${DATADIR}/cluster > ./cluster.summary


awk -F\| ' \
	NR != 1 {total += 1; t[$5] += 1; l[$14] += 1;} \
	$2!=0	{printf("%s\n",$2) > "filesys.phys" } \
	END{	printf("%d Filesystems by protocol type:\n", total); \
		printf("\t%5d NFS\t%2d%%\n", t["NFS"], (100 * t["NFS"] + total/2)/total); \
		printf("\t%5d AFS\t%2d%%\n", t["AFS"], (100 * t["AFS"] + total/2)/total); \
		printf("\t%5d RVD\t%2d%%\n", t["RVD"], (100 * t["RVD"] + total/2)/total); \
		printf("\t%5d FSGROUP\t%2d%%\n", t["FSGROUP"], (100 * t["FSGROUP"] + total/2)/total); \
		printf("\t%5d ERROR\t%2d%%\n\n", t["ERR"], (100 * t["ERR"] + total/2)/total); \
		printf("%d Filesystems by locker type:\n", total); \
		printf("\t%5d HOMEDIR\t%2d%%\n", l["HOMEDIR"], (100 * l["HOMEDIR"] + total/2)/total); \
		printf("\t%5d SYSTEM\t%2d%%\n", l["SYSTEM"], (100 * l["SYSTEM"])/total); \
		printf("\t%5d PROJECT\t%2d%%\n", l["PROJECT"], (100 * l["PROJECT"] + total/2)/total); \
		printf("\t%5d COURSE\t%2d%%\n", l["COURSE"], (100 * l["COURSE"])/total); \
		printf("\t%5d PERSONAL\t%2d%%\n", l["PERSONAL"], (100 * l["PERSONAL"] + total/2)/total); \
		printf("\t%5d ACTIVITY\t%2d%%\n", l["ACTIVITY"], (100 * l["ACTIVITY"] + total/2)/total); \
		printf("\t%5d EXTERN\t%2d%%\n", l["EXTERN"], (100 * l["EXTERN"] + total/2)/total); \
		printf("\t%5d OTHER\t%2d%%\n", l["OTHER"], (100 * l["OTHER"])/total); \
 } \
' < ${DATADIR}/filesys > ./filesys.summary

awk -F\| 'NR!=1 {total += 1; print $3} \
	END {printf("%5d Machines by type (workstations and servers):\n", total)>"machine.summary"} \
'< ${DATADIR}/machine | sort | uniq -c | sort -r -n > machine.types
awk '{printf("\t%-8s %4d\n", $2, $1)}' < machine.types >> machine.summary

sort ./filesys.phys | uniq -c | awk '{printf("%s %s\n", $2, $1)}' > phys.num
awk -F\| '{printf("%s %s\n", $2, $1)}' < ${DATADIR}/machine | sort > machine.num
sed 's/\|/ /g' < ${DATADIR}/nfsphys | sort | join - phys.num | awk '{printf("%s %s %s %s\n", $2, $4, $6, $12)}' | sort | join - machine.num | awk '$3>0&&$4>0{printf("\t%8d    %4d    %s:%s\n", $3, $4, $5, $2)}' | sort -n -r > phys.usage

awk -F\| ' \
	NR != 1 {total += 1; auth += $6} \
	END {	printf("%5d Printers, %d with authentication (%d%%)\n", \
			total, auth, (100 * auth)/total) } \
' < ${DATADIR}/printcap > ./printcap.summary

echo "			MOIRA SUMMARY" for `ls -ls ${DATADIR}/values | awk '{printf("%s %s", $6, $7)}'` > ./report.out
echo "" >> report.out
cat machine.summary >> report.out
echo "" >> report.out
cat cluster.summary >> report.out
echo "" >> report.out
cat printcap.summary >> report.out
echo "" >> report.out
awk '{printf("%5d Users by class (total in database, registered for use, %% registered):\n", $2)}' < users.summary1 >> report.out
cat users.summary >> report.out
echo "" >> report.out
cat pobox.summary >> report.out
echo "" >> report.out
cat list.summary >> report.out
echo "" >> report.out
cat filesys.summary >> report.out
echo "" >> report.out
awk '{total+=1}\
END {printf("%5d NFS Physical partitions in use (only those with quota shown):\n", total)}' < ${DATADIR}/nfsphys >> report.out
echo '	allocated| no. of | Server and partition' >> report.out
echo '	  quota  |lockers |' >> report.out
cat phys.usage >> report.out

cp report.out ${DATADIR}/report

cd /
rm -rf ${TMPDIR}

exit 0
