%{
#undef yywrap
int num_lines = 0;
%}

PC	[^\"]
AN	[A-Z_a-z0-9]
%%
\n		++num_lines;

error_table	return ERROR_TABLE;
et		return ERROR_TABLE;
error_code	return ERROR_CODE_ENTRY;
ec		return ERROR_CODE_ENTRY;
end		return END;

[\t\n ]		;

\"{PC}*\"	{ register char *p; yylval.dynstr = ds(yytext+1);
		  p=strrchr(yylval.dynstr, '"');
		  if (p) *p='\0';
		  return QUOTED_STRING;
		}

{AN}*	{ yylval.dynstr = ds(yytext); return STRING; }

#.*\n		;

.		{ return (*yytext); }
%%
static const char l_rcsid[] = "$Id: et_lex.lex.l,v 1.2 1998-02-05 22:13:09 danw Exp $";

int yywrap()
{
	return 1;
}
