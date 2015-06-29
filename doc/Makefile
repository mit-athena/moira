SHELL=/bin/sh

.SUFFIXES: .ps .dvi .tex

.tex.ps:
	latex $<
	@rm -f $*.log $*.aux
	dvips -o $@ $*.dvi
	@rm -f $*.dvi

all: dcm.ps manual.ps queries.ps

manual.ps: /mit/consult/lib/tex/macros/psbox.sty
	@echo latex $<
	@env TEXINPUTS=/mit/consult/lib/tex/macros: latex $<
	@env TEXINPUTS=/mit/consult/lib/tex/macros: latex $<
	@rm -f $*.log $*.aux $*.toc $*.idx
	dvips -o $@ $*.dvi
	@rm -f $*.dvi

/mit/consult/lib/tex/macros/psbox.sty:
	attach -h -n consult

clean:
	rm -f *.ps *.dvi *.aux *.idx *.log *.toc
