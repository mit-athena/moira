#!/usr/athena/bin/perl 

# Dependencies:
# 1. Schema must have "create table tablename" all on one line
# 2. Fields (columns) must be declared "NOT NULL WITH DEFAULT"
# 3. C variables corresponding to database fields must use the
#    database field name with "t_" prefixed.
# 4. Field names, table names, and C "t_" variable names are of the same case


# Step 1. Read the schema
open(SCHEMA,"../db/schema") || die "Could not open schema, stopped\n";

while(<SCHEMA>) {
    if(/create\s+table\s+/) {
	($table,$ignore) = split(' ',$');
	next;
    }
    if(/\s+(\w+)\s+(VAR)?CHAR\((\d+)\)\s+NOT NULL WITH DEFAULT/) {
	$fields{$table . "." . $1} = $3;
    }
}


#while (($key,$value) = each %fields) {
#    print $key . " = " . $value . "\n";
#}


# Step 2. Read the code to be cleaned up

# Pass through most lines
# Note the opening and closing of cursors
# Play with IIcsGetio.  Change 4th argument

while(<>) {
    if(/IIwritio\(.*"select.*from\s+(\w+)"\);/) {
	$table = $1;
	next;
    }
    if(/IIcsGetio\((.*,.*,.*,(\d+),&?t_(\w+))\);/)

       {

	   $arguments = $1;
	   $width = $2;
	   $field = $3;

	   $tablefield = $table . "." . $field;
	   $shouldbe = $fields{$tablefield};

	   if( $shouldbe && $shouldbe != $width ) {
	       print "Field " . $tablefield . " found with length " . $width . " -- " . " should be " . $shouldbe . ".\n";
	   }
       }
} continue {
#    print;
}

