#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/db2rest.awk,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/db2rest.awk,v 1.3 1993-07-14 10:40:26 mar Exp $
#
#	This converts the file used to originally create the database
#	into a program to restore it from a backup.
#	This is not guaranteed to work for all data types; it may
#	need to be extended.

BEGIN { print "/* This file automatically generated */";
	print "/* Do not edit */";
	print "#include <stdio.h>";
	print "EXEC SQL INCLUDE sqlca;";
	print "void parse_nl(), parse_str(), parse_sep();"
	print "/* This file automatically generated */" > "rest1.dc";
	print "/* Do not edit */" >> "rest1.dc";
	print "#include <stdio.h>" >> "rest1.dc";
	print "FILE *open_file();" >> "rest1.dc";
	print "do_restores(prefix)\n\tchar *prefix;\n{" >>"rest1.dc";
}

$1=="#" { next; }

/^create/ { printf "restore_%s(f)\nFILE *f;\n", $3;
	print "{\n\tEXEC SQL BEGIN DECLARE SECTION;";
	printf "\trestore_%s(open_file(prefix, \"%s\"));\n", $3, $3 >> "rest1.dc";
	tablename = $3;
	rangename = substr(tablename, 1, 1);
	count = 0;
	next; }

NF >= 2 {
	vname[count] = $1; 
	printf "/* %s */\n", $0
	if ($2 ~ /INTEGER/ || $2 ~ /SMALLINT/ || $2 ~ /INTEGER1/) {
		printf "\tint\tt_%s;\n", vname[count]
		vtype[count]="int"
	} else if ($2 ~ /CHAR\([0-9]*\)/) {
		t = split($2, temp, "(")
		if (t != 2) printf "Can't parse %s\n", $2;
		t = split(temp[2], temp2, ")")
		if (t != 2) printf "Can't parse %s\n", temp[2];
		printf "\tchar\tt_%s[%d];\n", vname[count], temp2[1]+1;
		vsize[count] = temp2[1]+1;
		vtype[count]="str"
	} else if ($2 ~ /DATE/) {
		printf "\tchar\tt_%s[26];\n", vname[count];
		vtype[count]="str";
		vsize[count]=26;
	} else printf "Unknown data type %s\n", $2;
	count++;
}

/^\);$/ { 
	print "\tEXEC SQL END DECLARE SECTION;\n";
	print "\twhile(!feof(f)) {"
	for (i = 0; i < count; i++) {
		if (i != 0) print "\t\tparse_sep(f);"
		if (vtype[i] ~ /int/) {
			printf("\t\tt_%s = parse_int(f);\n", vname[i]);
		} else printf "\t\tparse_%s(f, t_%s, %d);\n", vtype[i], vname[i], vsize[i]
		if (i == 0) print "\t\tif (feof(f)) break;"
	}
	printf "\t\tparse_nl(f);\n"

	printf "\t\tEXEC SQL INSERT INTO %s (\n", tablename;
	for (i = 0; i < count; i++) {
		if (i != 0) printf ",\n";
		printf "\t\t\t%s", vname[i];
	}
	printf ")\n\t\tVALUES (\n";
	for (i = 0; i < count; i++) {
		if (i != 0) printf ",\n";
		printf "\t\t\t:t_%s", vname[i];
	}
	printf ");\n\t\tif (sqlca.sqlcode != 0) {\n"
	printf "\t\t\tcom_err(\"restore\", 0, \"insert failed\");\n"
	printf "\t\t\texit(2);\n\t\t}\n\t}\n"
	printf "\t(void) fclose(f);\n"
	printf "}\n"
}
END { print "/* All done */"
	print "}" >>"rest1.dc";
}
