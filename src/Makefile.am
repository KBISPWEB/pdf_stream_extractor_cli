SUBDIRS = filext buffer zlib

AM_CFLAGS = -I$(srcdir)/filext/src -I$(srcdir)/buffer/src -I$(srcdir)/zlib/

bin_PROGRAMS = $(top_builddir)/bin/psec
__top_builddir__bin_psec_SOURCES = pse.c stream_scanner.c
# __top_builddir__bin_psec_LDFLAGS = -static-libgcc -static
__top_builddir__bin_psec_LDADD = filext/bin/libfilext.a buffer/bin/libbuffer.a zlib/bin/libz.a
