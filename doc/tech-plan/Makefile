#
#
#
LIB= owl.PS plan.mak
SRC= sms.mss intro.mss structure.mss protocol.mss tools.mss deploy.mss \
	dcm.mss clientlib.mss server.mss addendum.mss db_analysis.mss

all: plan

plan: ${LIB} ${SRC}
	scribe sms.mss

xerox: sms.x9700

x9700: sms.x9700

sms.x9700: ${LIB} ${SRC}
	scribe sms.mss -device x9700

clean:
	rm -f *.x9700 *.PS *.err *~ *.aux
