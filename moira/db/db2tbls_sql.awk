#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/db/db2tbls_sql.awk,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/db/db2tbls_sql.awk,v 1.1 1990-09-26 09:19:27 kostas Exp $
#
#	This converts the file used to originally create the database
#	into a list of tables.

BEGIN { print "/* This file automatically generated */";
	print "/* Do not edit */";
	print "char *tables[] = {";
}

/^create/ { printf "	\"%s\",\n", $2; }

END   { print "	0,";
	print "};"; }
