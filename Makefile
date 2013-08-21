CC=gcc
CFLAGS=-Wall
PREFIX=/usr
SBINDIR=${PREFIX}/sbin

all: ccwm

install: ccwm
	@echo installing executable file to ${DESTDIR}${SBINDIR}/ccwm
	mkdir -p ${DESTDIR}${SBINDIR}
	install -m 755 ccwm ${DESTDIR}${SBINDIR}

uninstall:
	@echo removing executable file from ${DESTDIR}${SBINDIR}/ccwm
	rm -f ${DESTDIR}${SBINDIR}/ccwm

clean:
	@echo cleaning
	rm ccwm
