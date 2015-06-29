# $Id: doschema.awk 3956 2010-01-05 20:56:56Z zacheiss $
#
# Build moira_schema.c and .h from the schema.sql file
# It knows which one to build cased on whether or not its input is
# in all caps.

BEGIN {
	print "/* This file automatically generated */";
	print "/* Do not edit */\n";

	ntables = 0;
	maxsize = 0;
}

$1 == "#" { next; }

/^create/ {
  cfile = 1;
  tablename[ntables] = $3;
  count = 0;
  next;
}

/^CREATE/ {
  hfile = 1;
  tablename[ntables] = $3;
  count = 0;
  next;
}

NF >= 2 {
  if ($2 ~ /CHAR\([0-9]*\)/) {
    t = split($2, temp, "(");
    if (t != 2) printf "Can't parse %s\n", $2;
    t = split(temp[2], temp2, ")");
    if (t != 2) printf "Can't parse %s\n", temp[2];
    width[count] = temp2[1];
  } else if ($2 ~ /DATE/)
    width[count] = 21;
  else if ($2 ~ /INT/)
    width[count] = 10;
  else
    width[count] = 0;

  if (hfile && width[count]) {
    printf "#define %s_%s_SIZE %d\n", tablename[ntables], $1, width[count] + 1;
    if (width[count] > maxsize)
      maxsize = width[count];
  }

  count++;
}

/^\);$/ {
  if (cfile) {
    printf "int %s_widths[] = { ", tablename[ntables];
    for (i = 0; i < count; i++) {
      printf "%d", width[i];
      if (i < count-1)
	printf ", ";
    }
    printf "};\n";
  }
  ntables++;
}

END {
  if (hfile) {
    printf "\n#define MAX_FIELD_WIDTH %d\n", maxsize + 1;
    printf "\nenum tables { NO_TABLE,";
    len = 22;
  } else {
    printf "\nint num_tables = %d;\n", ntables;
    printf "\nchar *table_name[] = { \"none\",";
    len = 29;
  }

  for (i = 0; i < ntables; i++) {
    if (hfile)
      str = sprintf(" %s_TABLE", tablename[i]);
    else
      str = sprintf(" \"%s\"", tablename[i]);
    if (len + length(str) > 75) {
      printf "\n ";
      len = 1;
    }
    printf str;
    if (i != ntables - 1)
      printf ",";
    len += length(str);
  }
  print "\n};\n";
}
