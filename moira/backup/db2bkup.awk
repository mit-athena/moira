#	$Id $
#
#	This converts the file used to originally create the database
#	into a program to back it up.
#	This is not guaranteed to work for all data types; it may
#	need to be extended.

BEGIN {
	print "/* This file automatically generated */";
	print "/* Do not edit */\n";
	print "EXEC SQL INCLUDE sqlca;";
	print "EXEC SQL WHENEVER SQLERROR DO dbmserr();";
	print "#include \"dump_db.h\"";
	print "#include \"bkup.h\"";
	print "#define dump_bin dump_str\n";

	print "/* This file automatically generated */" > "bkup1.pc";
	print "/* Do not edit */\n" >> "bkup1.pc";
	print "#include \"dump_db.h\"" >> "bkup1.pc";
	print "#include \"bkup.h\"" >> "bkup1.pc";
	print "void do_backups(char *prefix)\n{" >> "bkup1.pc";

	print "/* This file automatically generated */" > "bkup.h";
	print "/* Do not edit */\n" >> "bkup.h";
}

$1=="#" { next; }

/^create/ {
	printf "void dump_%s(FILE *f)\n{\n  EXEC SQL BEGIN DECLARE SECTION;\n", $3;
	printf "  dump_%s(open_file(prefix, \"%s\"));\n", $3, $3 >> "bkup1.pc";
	printf "void dump_%s(FILE *f);\n", $3 >> "bkup.h";

	tablename = $3;
	rangename = substr(tablename, 1, 1);
	count = 0;
	next;
}

NF>=2 {
	vname[count] = $1;
	printf "  /* %s */\n", $0;
	if ($2 ~ /INTEGER/ || $2 ~ /SMALLINT/ || $2 ~ /INTEGER1/) {
		printf "  int\tt_%s;\n", vname[count];
		vtype[count]="int";
	} else if ($2 ~ /CHAR\([0-9]*\)/) {
		t = split($2, temp, "(");
		if (t != 2) printf "Can't parse %s\n", $2;
		t = split(temp[2], temp2, ")");
		if (t != 2) printf "Can't parse %s\n", temp[2];
		printf "  char\tt_%s[%d];\n", vname[count], temp2[1]+1;
		if ($1 == "signature") {
			vtype[count]="bin";
			printf "  EXEC SQL VAR t_signature IS STRING(%d);\n", temp2[1]+1;
		} else {
			vtype[count]="str";
		}
		vsize[count] = temp2[1]+1;
	} else if ($2 ~ /DATE/) {
		printf "  char\tt_%s[26];\n", vname[count];
		vtype[count]="date";
	} else printf "Unknown data type %s\n", $2;
	count++;
}

/^\);$/ {
	printf "  EXEC SQL END DECLARE SECTION;\n\n";
	printf "  EXEC SQL DECLARE c_%s CURSOR FOR SELECT\n", tablename;
	for (i = 0; i < count; i++) {
		if (i != 0) {
			print ",";
		}
		if(vtype[i] ~ /date/) {
			printf "    TO_CHAR(%s, 'DD-mon-YYYY HH24:MI:SS')", vname[i];
		} else printf "    %s", vname[i];
	}
	printf " FROM %s;\n", tablename;

	printf "  EXEC SQL OPEN c_%s;\n", tablename;
	printf "  while (1)\n    {\n      EXEC SQL FETCH c_%s INTO\n", tablename;
	for (i = 0; i < count; i++) {
		if (i != 0) printf ",\n";
		printf "        :t_%s", vname[i];
	}
	printf ";\n";
	printf "      if (sqlca.sqlcode != 0)\n        break;\n";
	for (i = 0; i < count; i++) {
		if (i != 0) print "      dump_sep(f);";
		if (vtype[i] ~ /str/ || vtype[i] ~ /date/) {
			printf "      dump_str(f, strtrim(t_%s));\n", vname[i];
		} else {
			printf "      dump_%s(f, t_%s);\n", vtype[i], vname[i];
		}
	}
	printf "      dump_nl(f);\n";
	printf "    }\n";
	printf "  EXEC SQL CLOSE c_%s;\n", tablename;
	printf "  safe_close(f);\n";
	printf "}\n\n";
}

END {
	print "/* All done */";
	print "}" >> "bkup1.pc";
}
