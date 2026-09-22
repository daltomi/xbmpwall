dist_noinst_DATA = data

bin_PROGRAMS = xbmpwall

xbmpwall_SOURCES = 	src/xbmpwall.c \
					src/xbmpwall.h \
					src/hexcolors.h

xbmpwall_CFLAGS = -std=c11 -pedantic

dist_doc_DATA = COPYING README.md

distclean-local:
	-@rm -rf autom4te.cache \
	src/.deps \
	aclocal.m4 \
	*~ \
	compile \
	configure \
	config.guess \
	config.sub \
	depcomp \
	install-sh \
	Makefile.in \
	missing \
	config.h.in	\
	src/config.h.in	\
	src/*~ \
	*.tar.gz
