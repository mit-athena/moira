BEGIN				{state = 0}
$1=="#ifsql"&&$2=="INGRES"	{state = 0; printf("\n"); next}
$1=="#ifsql"			{state = 1; printf("\n"); next}
$1=="#elsesql"			{state = 1-state; printf("\n"); next}
$1=="#endsql"			{state = 0; printf("\n"); next}
state==1			{printf("\n"); next}
state==0
