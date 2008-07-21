#	$Id: db2rest.awk,v 1.11 2004-10-19 03:43:59 zacheiss Exp $
#
#	This converts the file used to originally create the database
#	into a program to restore it from a backup.
#	This is not guaranteed to work for all data types; it may
#	need to be extended.

BEGIN {
	print "/* This file automatically generated */";
	print "/* Do not edit */\n";
	print "EXEC SQL INCLUDE sqlca;";
	print "EXEC SQL WHENEVER SQLERROR DO dbmserr();";
	print "#include <com_err.h>";
	print "#include \"dump_db.h\"";
	print "#include \"rest.h\"";

	print "/* This file automatically generated */" > "rest1.pc";
	print "/* Do not edit */\n" >> "rest1.pc";
	print "#include \"dump_db.h\"" >> "rest1.pc";
	print "#include \"rest.h\"" >> "rest1.pc";
	print "void do_restores(char *prefix)\n{" >> "rest1.pc";

	print "/* This file automatically generated */" > "bkup.h";
	print "/* Do not edit */\n" >> "bkup.h";
}

$1=="#" { next; }

/^create/ {
	printf "void restore_%s(FILE *f)\n", $3;
	print "{\n  EXEC SQL BEGIN DECLARE SECTION;";
	printf "  restore_%s(open_file(prefix, \"%s\"));\n", $3, $3 >> "rest1.pc";
	printf "void restore_%s(FILE *f);\n", $3 >> "rest.h";

	tablename = $3;
	rangename = substr(tablename, 1, 1);
	count = 0;
	next;
}

NF >= 2 {
	vname[count] = $1;
	printf "  /* %s */\n", $0;
	if ($2 ~ /INTEGER/ || $2 ~ /SMALLINT/ || $2 ~ /INTEGER1/) {
		printf "  char\tt_%s[39];\n", vname[count];
		vtype[count]="int";
	} else if ($2 ~ /CHAR\([0-9]*\)/) {
		t = split($2, temp, "(");
		if (t != 2) printf "Can't parse %s\n", $2;
		t = split(temp[2], temp2, ")");
		if (t != 2) printf "Can't parse %s\n", temp[2];
		printf "  char\tt_%s[%d];\n", vname[count], temp2[1]+1;
		if ($1 == "signature" || $1 == "sid") {
			vtype[count]="bin";
			printf "  EXEC SQL VAR t_%s IS STRING(%d);\n", vname[count], temp2[1]+1;
		} else vtype[count]="str";
		vsize[count] = temp2[1]+1;
	} else if ($2 ~ /DATE/) {
		printf "  char\tt_%s[26];\n", vname[count];
		vtype[count]="date";
	} else printf "Unknown data type %s\n", $2;
	count++;
}

/^\);$/ {
	printf "  EXEC SQL END DECLARE SECTION;\n\n  int count = 0;\n";

	print "  while (!feof(f))\n    {";
	print "      if (!(++count % 100))\n        EXEC SQL COMMIT;\n";

	for (i = 0; i < count; i++) {
		if (i != 0) print "      parse_sep(f);";
		if (vtype[i] ~ /int/) {
			printf("      parse_str(f, t_%s, 39);\n", vname[i]);
		} else if (vtype[i] ~ /date/) {
			printf "      parse_str(f, t_%s, 26);\n", vname[i];
		} else {
			printf "      parse_str(f, t_%s, %d);\n", vname[i], vsize[i];
		}
		if (i == 0) print "      if (feof(f))\n        break;";
	}
	printf "      parse_nl(f);\n"

	printf "      EXEC SQL INSERT INTO %s (\n", tablename;
	for (i = 0; i < count; i++) {
		if (i != 0) printf ",\n";
		printf "          %s", vname[i];
	}
	printf ")\n        VALUES (\n";
	for (i = 0; i < count; i++) {
		if (i != 0) printf ",\n";
		if (vtype[i] ~ /date/) {
			printf "          TO_DATE(NVL(:t_%s,TO_CHAR(SYSDATE, 'DD_mon-YYYY HH24:MI:SS')), 'DD-mon-YYYY HH24:MI:SS')", vname[i];
		} else if(vtype[i] ~ /int/) {
		        printf "          TO_NUMBER(:t_%s)", vname[i];
		} else {
			printf "          NVL(:t_%s,CHR(0))", vname[i];
		}
	}
	printf ");\n      if (sqlca.sqlcode != 0)\n        {\n";
	printf "          sqlca.sqlerrm.sqlerrmc[sqlca.sqlerrm.sqlerrml] = 0;\n";
	printf "          printf(\"%%s\\n\", sqlca.sqlerrm.sqlerrmc);\n";
	printf "          com_err(\"restore\", 0, \"insert failed\");\n";
	printf "          exit(2);\n        }\n    }\n";
	printf "  fclose(f);\n";
	printf "  EXEC SQL COMMIT;\n";
	printf "}\n\n";
}


END {
	print "/* All done */";
	print "}" >> "rest1.pc";
}
