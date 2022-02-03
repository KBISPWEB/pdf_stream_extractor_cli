SUBDIRS = filext buffer

AM_CFLAGS = -I$(srcdir)/filext/src -I$(srcdir)/buffer/src

bin_PROGRAMS = $(top_builddir)/bin/psec
__top_builddir__bin_psec_SOURCES = pse.c
# __top_builddir__bin_psec_LDFLAGS = -static-libgcc -static
__top_builddir__bin_psec_LDADD = filext/bin/libfilext.a buffer/bin/libbuffer.a -lz
