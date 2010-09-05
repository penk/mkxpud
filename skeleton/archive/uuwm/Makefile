# uuwm - micro window manager
# See LICENSE file for copyright and license details.

include config.mk

WM = uuwm
WMV = ${WM}-${VERSION}
SRC = ${WM}.c
OBJ = ${SRC:.c=.o}

all: options ${WM}

options:
	@echo ${WM} build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} -DOLD_XCB_AUX $<

${OBJ}: config.mk

${WM}: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f ${WM} ${OBJ} ${WMV}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p ${WMV}
	@cp -R LICENSE Makefile README config.mk ${SRC} ${WMV}
	@tar -czf ${WMV}.tar ${WMV}
	@rm -rf ${WMV}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f ${WM} ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/${WM}

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/${WM}

.PHONY: all options clean dist install uninstall
