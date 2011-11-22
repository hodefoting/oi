CFILES  = $(wildcard *.c)
OBJS    = $(CFILES:.c=.o)
PROJNAME = oi
LIBNAME = lib$(PROJNAME)
PREFIX = /usr/local
LIBEXT  = so
TARGETS = $(LIBNAME).a $(LIBNAME).$(LIBEXT)

CFLAGS = -I.. -I. -fpic -ldl
LIBS = -ldl -lpthread #`pkg-config glib-2.0 gio-2.0 --libs`
CFLAGS += -O3 -g
CFLAGS += -msse2 -msse -mssse3
CFLAGS += -Wall
#CFLAGS += -Wall -Wno-unused-value -Wextra
CFLAGS += -funroll-loops -fforce-addr -ftracer -fpeel-loops -fmerge-all-constants
#CFLAGS += -fomit-frame-pointer

all: $(TARGETS)

$(LIBNAME).a: $(OBJS)
	@echo "  AR" $@;ar rcs $@ $(OBJS)
$(LIBNAME).$(LIBEXT): $(OBJS)
	@echo "  LD" $@;cc -shared $(OBJS) -o $@ $(FLAGS) $(LIBS)
%.o: %.c *.h
	@echo "  CC" $<;$(CC) -c $(CFLAGS) $< -o $@

EXAMPLES_CFILES = $(wildcard examples/*.c)
EXAMPLES_BINS   = $(EXAMPLES_CFILES:.c=)

examples/%: examples/%.c $(LIBNAME).a
	@echo "CCLD" $@; $(CC) -I .. $(CFLAGS) $(LIBS) $< $(LIBNAME).a `pkg-config --cflags --libs gobject-2.0` -o $@

all: $(EXAMPLES_BINS)

clean:
	rm -f $(OBJS) $(TARGETS) $(BIN) $(EXAMPLES_BINS)
	make -C tests clean

check: all
	make -C tests check

install: $(LIBNAME).$(LIBEXT)
	sudo install -t $(PREFIX)/lib $(LIBNAME).$(LIBEXT)
	sudo install -t $(PREFIX)/lib/pkgconfig $(PROJNAME).pc
	sudo install -d $(PREFIX)/include/$(PROJNAME)
	sudo install -t $(PREFIX)/include/$(PROJNAME) *.h
