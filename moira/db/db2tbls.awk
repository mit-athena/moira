#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/db/db2tbls.awk,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/db/db2tbls.awk,v 1.2 1990-12-11 15:23:51 mar Exp $
#
#	This converts the file used to originally create the database
#	into a list of tables.

BEGIN { print "/* This file automatically generated */";
	print "/* Do not edit */";
	print "char *tables[] = {";
}

$1=="CREATE"	{ printf "	\"%s\",\n", $3; }
$1=="create" && $2=="table"	{ printf "	\"%s\",\n", $3; }
$1=="create" && $2!="table"	{ printf "	\"%s\",\n", $2; }

END   { print "	0,";
	print "};"; }
