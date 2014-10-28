#
#  makefile for CSCE 515 project3
#  Yixing Cheng
#

include ../Make.defines

PROGS = confclient confserver

all: ${PROGS}

confclient: confclient.o
	${CC} ${CFLAGS} -o $@ confclient.o ${LIBS}

confserver: confserver.o
	${CC} ${CFLAGS} -o $@ confserver.o ${LIBS}

clean:
	rm -f ${PROGS} ${CLEANFILES}
