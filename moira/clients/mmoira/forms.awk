# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/forms.awk,v 1.4 1992-12-10 10:41:39 mar Exp $

BEGIN		{ numfields = 0; numforms = 0; orfield = 0;
		  printf("/* This file is machine generated, do not edit. */");
		  printf("\n#include <Xm/Text.h>\n");
		  printf("#include \"mmoira.h\"\n\n");
		}

/^;/		{ next; }
NF == 0		{ next; }
NF == 1 && $1 == "or" { orfield = 1; next; }

$1 == "{"	{ formname = sprintf("form%d", numforms);
		  formrealname = $2;
		  numforms++;
		  instructions = $3;
		  for (i = 4; i <= NF; i++)
			instructions = sprintf("%s %s", instructions, $i);
		  next;
		}

$1 == "}"	{ printf("static UserPrompt *%s_fields[] = {\n", formname);
		  for (i = 0; i < numfields; i++)
			printf("\t&%s_fld%d,\n", formname, i);
		  printf("\t(UserPrompt *)NULL\n};\n");
		  printf("static EntryForm %s = {\n\t\"%s\",\n", \
			formname, formrealname);
		  printf("\t\"%s\",\n\t%s_fields\n};\n", \
			instructions, formname);
		  numfields = 0;
		  next
		}

$NF == "s" || $NF == "S" \
		{ printf("static UserPrompt %s_fld%d = { \"", \
			formname, numfields);
		  for (i = 1; i < NF; i++) printf("%s ", $i);
		  if (orfield) {
			printf("\", FT_STRING, True };\n");
			orfield = 0;
		  } else {
			printf("\", FT_STRING, False };\n");
		  }
		  numfields++;
		  next
		}

$NF == "b" || $NF == "B" \
		{ printf("static UserPrompt %s_fld%d = { \"", \
			formname, numfields);
		  for (i = 1; i < NF; i++) printf("%s ", $i);
		  if (orfield) {
			printf("\", FT_BOOLEAN, True };\n");
			orfield = 0;
		  } else {
			printf("\", FT_BOOLEAN, False };\n");
		  }
		  numfields++;
		  next
		}

$NF == "k" || $NF == "K" \
		{ printf("static UserPrompt %s_fld%d = { \"", \
			formname, numfields);
		  for (i = 1; i < NF; i++) printf("%s ", $i);
		  if (orfield) {
			printf("\", FT_KEYWORD, True };\n");
			orfield = 0;
		  } else {
			printf("\", FT_KEYWORD, False };\n");
		  }
		  numfields++;
		  next
		}

		{ printf("forms syntax error on line %d of file %s:\n%s\n", \
			NR, FILENAME, $0);
		  exit 1
		}

END		{
		  printf("EntryForm *MoiraForms[] = {\n");
		  for (i = 0; i < numforms; i++)
			printf("\t&form%d,\n", i);
		  printf("\t(EntryForm *)NULL,\n};\n");
		}
