#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/db2bkup.awk,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/db2bkup.awk,v 1.3 1988-05-10 11:51:28 mar Exp $
#
#	This converts the file used to originally create the database
#	into a program to back it up.
#	This is not guaranteed to work for all data types; it may
#	need to be extended.

BEGIN { print "/* This file automatically generated */";
	print "/* Do not edit */";
	print "#include <stdio.h>";
	print "#include \"dump_db.h\"";
	print "/* This file automatically generated */" > "bkup1.qc";
	print "/* Do not edit */" >> "bkup1.qc"
	print "#include <stdio.h>" >> "bkup1.qc"
	print "FILE *open_file();" >> "bkup1.qc"
	print "do_backups(prefix)\n\tchar *prefix;\n{" >>"bkup1.qc"
}

/^create/ { printf "dump_%s(f)\nFILE *f;\n", $2;
	printf "\tdump_%s(open_file(prefix, \"%s\"));\n", $2, $2 >> "bkup1.qc"
	tablename = $2;
	rangename = substr(tablename, 1, 1);
	count = 0; }

$2 ~ /\=/ {
	vname[count] = $1; 
	printf "/* %s */\n", $0
	if ($3 ~ /i[124]/) {
		printf "##	int	t_%s;\n", vname[count]
		vtype[count]="int"
	} else if ($3 ~ /text\([0-9]*\)/) {
		t = split($3, temp, "(")
		if (t != 2) printf "Can't parse %s\n", $3;
		t = split(temp[2], temp2, ")")
		if (t != 2) printf "Can't parse %s\n", temp[2];
		printf "##	char 	t_%s[%d];\n", vname[count], temp2[1]+1;
		vtype[count]="str"
	} else if ($3 ~ /date/) {
		printf "##	char	t_%s[26];\n", vname[count]
		vtype[count]="str"
	} else if ($3 ~ /c[0-9]*/) {
		t = split($3, temp, ",")
		printf "##	char	t_%s[%d];\n", vname[count], substr(temp[1], 2) + 1
		vtype[count]="str"
	} else printf "Unknown data type %s\n", $3;
	count++;
}

/^\($/ { print "##{" }
/^\)$/ { 
	printf "##	range of %s is %s\n", rangename, tablename
	printf "##	retrieve(\n"
	for (i = 0; i < count; i++) {
		if (i != 0) printf ",\n";
		printf "##\t\tt_%s = %s.%s", vname[i], rangename, vname[i]
	}
	printf ")\n"
	printf "##	{\n"
	for (i = 0; i < count; i++) {
		if (i != 0) print "\t\tdump_sep(f);"
		printf "\t\tdump_%s(f, t_%s);\n", vtype[i], vname[i]
	}
	printf "\t\tdump_nl(f);\n"
	printf "##	}\n"
	printf "\tsafe_close(f);\n"
	printf "##}\n"
}
END { print "/* All done */"
	print "}" >>"bkup1.qc"
}
