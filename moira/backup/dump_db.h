#define dump_nl(f) { if (putc('\n', f) < 0) wpunt(); }
#define dump_sep(f) { if (putc(':', f) < 0) wpunt(); }
			   
