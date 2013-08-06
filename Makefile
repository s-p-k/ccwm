CC=gcc
CFLAGS=-Wall
PREFIX=/usr
SBINDIR=$(PREFIX)/sbin

all: ccwm

install: ccwm
	@echo installing executable file to $(SBINDIR)/ccwm
	mkdir -p $(SBINDIR)
	install -m 755 ccwm $(SBINDIR)

uninstall:
	@echo removing executable file from $(SBINDIR)/ccwm
	rm -f $(SBINDIR)/ccwm

clean:
	@echo cleaning
	rm ccwm
