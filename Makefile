CC = ./oicc/oicc

include ../.mm/magic

EXAMPLES_PKGMODULES=gobject-2.0
LIB_PKGMODULES=
PROJECT_NAME      = oi

include ../.mm/lib

oicc/oicc: oicc/*.c
	CC=gcc make -C oicc all

#%.c:%.C /usr/local/bin/oicc /usr/local/bin/oipp
#	oipp $< > $@
#
clean: clean-too
clean-too:
	make -C oicc clean
install: install-too
install-too:
	make -C oicc install
