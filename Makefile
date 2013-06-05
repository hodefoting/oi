CC = ./oicc/oicc

include .mm/magic


EXAMPLES_PKGMODULES=gobject-2.0
LIB_PKGMODULES=
PROJECT_NAME      = oi

include .mm/lib

CFLAGS += -Wall -Wextra

oicc/oicc: oicc/*.c
	CC=gcc make -C oicc all
	oicc/oipp *.c > /dev/null # generate initial headers

#%.c:%.C /usr/local/bin/oicc /usr/local/bin/oipp
#	oipp $< > $@
#
clean: clean-too
clean-too:
	make -C oicc clean
	rm -f `echo *.h | sed s/oi-mem\.h// | sed s/oi\.h//`
install: install-too
install-too:
	make -C oicc install
