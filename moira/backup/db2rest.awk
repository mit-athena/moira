#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/db2rest.awk,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/db2rest.awk,v 1.2 1988-05-10 11:51:41 mar Exp $
#
#	This converts the file used to originally create the database
#	into a program to restore it from a backup.
#	This is not guaranteed to work for all data types; it may
#	need to be extended.

BEGIN { print "/* This file automatically generated */";
	print "/* Do not edit */";
	print "#include <stdio.h>";
	print "void parse_nl(), parse_str(), parse_sep();"
	print "/* This file automatically generated */" > "rest1.qc";
	print "/* Do not edit */" 
	print "#include <stdio.h>" >> "rest1.qc"
	print "FILE *open_file();" >> "rest1.qc"
	print "do_restores(prefix)\n\tchar *prefix;\n{" >>"rest1.qc"
}

/^create/ { printf "restore_%s(f)\nFILE *f;\n", $2;
	printf "\trestore_%s(open_file(prefix, \"%s\"));\n", $2, $2 >> "rest1.qc"
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
		vsize[count] = temp2[1]+1;
		vtype[count]="str"
	} else if ($3 ~ /date/) {
		printf "##	char	t_%s[26];\n", vname[count];
		vtype[count]="str";
		vsize[count]=26;
	} else if ($3 ~ /c[0-9]*/) {
		t = split($3, temp, ",")
		vsize[count] = substr(temp[1], 2) + 1
		printf "##	char	t_%s[%d];\n", vname[count],vsize[count]
		vtype[count]="str"
	} else printf "Unknown data type %s\n", $3;
	count++;
}

/^\($/ { print "##{" }
/^\)$/ { 
	print "\twhile(!feof(f)) {"
	for (i = 0; i < count; i++) {
		if (i != 0) print "\t\tparse_sep(f);"
		if (vtype[i] ~ /int/) {
			printf("\t\tt_%s = parse_int(f);\n", vname[i]);
		} else printf "\t\tparse_%s(f, t_%s, %d);\n", vtype[i], vname[i], vsize[i]
		if (i == 0) print "\t\tif (feof(f)) break;"
	}
	printf "\t\tparse_nl(f);\n"

	printf "##\t\tappend %s (\n", tablename
	for (i = 0; i < count; i++) {
		if (i != 0) printf ",\n";
		printf "##\t\t\t%s = t_%s", vname[i], vname[i]
	}
	printf ")\n"
	printf "\t}\n"
	printf "\t(void) fclose(f);\n"
	printf "##}\n"
}
END { print "/* All done */"
	print "}" >>"rest1.qc"
}
