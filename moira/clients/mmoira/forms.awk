# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/forms.awk,v 1.2 1991-05-22 18:24:46 mar Exp $

BEGIN		{ numfields = 0; numforms = 0;
		  printf("/* This file is machine generated, do not edit. */");
		  printf("\n#include <Xm/Text.h>\n");
		  printf("#include \"data.h\"\n\n");
		}

/^;/		{ next }
NF == 0		{ next }
NF == 1 && $1 == "or" { next }

$1 == "{"	{ formname = sprintf("form%d", numforms);
		  numforms++;
		  instructions = $3;
		  for (i = 4; i <= NF; i++)
			instructions = sprintf("%s %s", instructions, $i);
		  next
		}

$1 == "}"	{ printf("static UserPrompt *%s_fields[] = {\n", formname);
		  for (i = 0; i < numfields; i++)
			printf("\t&%s_fld%d,\n", formname, i);
		  printf("\t(UserPrompt *)NULL\n};\n");
		  printf("static EntryForm %s = {\n\t\"%s\",\n", \
			formname, formname);
		  printf("\t\"%s\",\n\t%s_fields\n};\n", \
			instructions, formname);
		  numfields = 0;
		  next
		}

$NF == "s" || $NF == "S" \
		{ printf("static UserPrompt %s_fld%d = { \"", \
			formname, numfields);
		  for (i = 1; i < NF; i++) printf("%s ", $i);
		  printf("\", FT_STRING };\n");
		  numfields++;
		  next
		}

$NF == "b" || $NF == "B" \
		{ printf("static UserPrompt %s_fld%d = { \"", \
			formname, numfields);
		  for (i = 1; i < NF; i++) printf("%s ", $i);
		  printf("\", FT_BOOLEAN };\n");
		  numfields++;
		  next
		}

$NF == "k" || $NF == "K" \
		{ printf("static UserPrompt %s_fld%d = { \"", \
			formname, numfields);
		  for (i = 1; i < NF; i++) printf("%s ", $i);
		  printf("\", FT_KEYWORD };\n");
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
		  printf("\t(EntryForm *)NULL,\n}\n");
		}
