#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/db2rest.awk,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/db2rest.awk,v 1.4 1997-01-20 18:14:09 danw Exp $
#
#	This converts the file used to originally create the database
#	into a program to restore it from a backup.
#	This is not guaranteed to work for all data types; it may
#	need to be extended.

BEGIN {
	print "/* This file automatically generated */";
	print "/* Do not edit */\n";
	print "#include <stdio.h>";
	print "EXEC SQL INCLUDE sqlca;";
	print "void parse_nl(), parse_str(FILE *, char *, int), parse_sep();\n";

	print "/* This file automatically generated */" > "rest1.dc";
	print "/* Do not edit */\n" >> "rest1.dc";
	print "#include <stdio.h>" >> "rest1.dc";
	print "FILE *open_file();" >> "rest1.dc";
	print "do_restores(prefix)\n\tchar *prefix;\n{" >> "rest1.dc";
}

$1=="#" { next; }

/^create/ {
	printf "restore_%s(f)\nFILE *f;\n", $3;
	print "{\n\tEXEC SQL BEGIN DECLARE SECTION;";
	printf "\trestore_%s(open_file(prefix, \"%s\"));\n", $3, $3 >> "rest1.dc";

	tablename = $3;
	rangename = substr(tablename, 1, 1);
	count = 0;
	next;
}

NF >= 2 {
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
		} else vtype[count]="str";
		vsize[count] = temp2[1]+1;
	} else if ($2 ~ /DATE/) {
		printf "\tchar\tt_%s[26];\n", vname[count];
		vtype[count]="date";
	} else printf "Unknown data type %s\n", $2;
	count++;
}

/^\);$/ { 
	printf "\tEXEC SQL END DECLARE SECTION;\n\tint count=0;\n";

	print "\twhile(!feof(f)) {";
	print "\t\tif(!(++count%100)) {\n\t\t\tEXEC SQL COMMIT;\n\t\t}\n";

	for (i = 0; i < count; i++) {
		if (i != 0) print "\t\tparse_sep(f);";
		if (vtype[i] ~ /int/) {
			printf("\t\tt_%s = parse_int(f);\n", vname[i]);
		} else if (vtype[i] ~ /date/) {
			printf "\t\tparse_str(f, t_%s, 26);\n", vname[i];
		} else {
			printf "\t\tparse_str(f, t_%s, %d);\n", vname[i], vsize[i];
		}
		if (i == 0) print "\t\tif (feof(f)) break;";
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
		if (vtype[i] ~ /date/) {
			printf "\t\t\tTO_DATE(NVL(:t_%s,TO_CHAR(SYSDATE, 'DD_mon-YYYY HH24:MI:SS')), 'DD-mon-YYYY HH24:MI:SS')", vname[i];
		} else if(vtype[i] ~ /int/) {
			printf "\t\t\t:t_%s", vname[i];
		} else {
			printf "\t\t\tNVL(:t_%s,CHR(0))", vname[i];
		}
	}
	printf ");\n\t\tif (sqlca.sqlcode != 0) {\n";
	printf "\t\t\tsqlca.sqlerrm.sqlerrmc[sqlca.sqlerrm.sqlerrml]=0;\n";
	printf "\t\t\tprintf(\"%%s\\n\", sqlca.sqlerrm.sqlerrmc);\n";
	printf "\t\t\tcom_err(\"restore\", 0, \"insert failed\");\n";
	printf "\t\t\texit(2);\n\t\t}\n\t}\n";
	printf "\t(void) fclose(f);\n";
	printf "\tEXEC SQL COMMIT;\n";
	printf "}\n\n";
}


END {
	print "/* All done */";
	print "}" >> "rest1.dc";
}
