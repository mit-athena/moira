#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/db2bkup.awk,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/db2bkup.awk,v 1.10 1997-07-22 03:20:17 danw Exp $
#
#	This converts the file used to originally create the database
#	into a program to back it up.
#	This is not guaranteed to work for all data types; it may
#	need to be extended.

BEGIN { 
	print "/* This file automatically generated */";
	print "/* Do not edit */\n";
	print "#include <stdio.h>";
	print "EXEC SQL INCLUDE sqlca;";
	print "EXEC SQL WHENEVER SQLERROR DO dbmserr();";
	print "#include \"dump_db.h\"";
	print "#define dump_bin dump_str\n";

	print "/* This file automatically generated */" > "bkup1.pc";
	print "/* Do not edit */\n" >> "bkup1.pc";
	print "#include <stdio.h>" >> "bkup1.pc";
	print "FILE *open_file();" >> "bkup1.pc";
	print "do_backups(prefix)\n\tchar *prefix;\n{" >> "bkup1.pc";
}

$1=="#" { next; }

/^create/ { 
	printf "dump_%s(f)\nFILE *f;\n{\n\tEXEC SQL BEGIN DECLARE SECTION;\n", $3;
	printf "\tdump_%s(open_file(prefix, \"%s\"));\n", $3, $3 >> "bkup1.pc";

	tablename = $3;
	rangename = substr(tablename, 1, 1);
	count = 0;
	next;
}

NF>=2 {
	vname[count] = $1; 
	printf "/* %s */\n", $0;
	if ($2 ~ /INTEGER/ || $2 ~ /SMALLINT/ || $2 ~ /INTEGER1/) {
		printf "\tint\tt_%s;\n", vname[count];
		vtype[count]="int";
	} else if ($2 ~ /CHAR\([0-9]*\)/) {
		t = split($2, temp, "(");
		if (t != 2) printf "Can't parse %s\n", $2;
		t = split(temp[2], temp2, ")");
		if (t != 2) printf "Can't parse %s\n", temp[2];
		printf "\tchar\tt_%s[%d];\n", vname[count], temp2[1]+1;
		if ($1 == "signature") {
			vtype[count]="bin";
			printf "\tEXEC SQL VAR t_signature IS STRING(%d);\n", temp2[1]+1;
		} else {
			vtype[count]="str";
		}
		vsize[count] = temp2[1]+1;
	} else if ($2 ~ /DATE/) {
		printf "\tchar\tt_%s[26];\n", vname[count];
		vtype[count]="date";
	} else printf "Unknown data type %s\n", $2;
	count++;
}

/^\);$/ { 
	printf "\tEXEC SQL END DECLARE SECTION;\n";
	printf "\tEXEC SQL DECLARE c_%s CURSOR FOR\n", tablename;
	printf "\t\tSELECT\n";
	for (i = 0; i < count; i++) {
		if (i != 0) {
			print ",";
		}
		if(vtype[i] ~ /date/) {
			printf "\t\t\tTO_CHAR(%s, 'DD-mon-YYYY HH24:MI:SS')", vname[i];
		} else printf "\t\t\t%s", vname[i];
	}
	printf " FROM %s;\n", tablename;

	printf "\tEXEC SQL OPEN c_%s;\n", tablename;
	printf "\twhile(1) {\n\t\tEXEC SQL FETCH c_%s INTO\n", tablename;
	for (i = 0; i < count; i++) {
		if (i != 0) printf ",\n";
		printf "\t\t\t:t_%s", vname[i];
	}
	printf ";\n";
	printf "\t\tif(sqlca.sqlcode != 0) break;\n";
	for (i = 0; i < count; i++) {
		if (i != 0) print "\t\tdump_sep(f);";
		if (vtype[i] ~ /str/ || vtype[i] ~ /date/) {
			printf "\t\tdump_str(f, strtrim(t_%s));\n", vname[i];
		} else {
			printf "\t\tdump_%s(f, t_%s);\n", vtype[i], vname[i];
		}
	}
	printf "\t\tdump_nl(f);\n";
	printf "\t}\n";
	printf "\tEXEC SQL CLOSE c_%s;\n", tablename;
	printf "\tsafe_close(f);\n";
	printf "}\n\n";
}

END {
	print "/* All done */";
	print "}" >> "bkup1.pc";
}
