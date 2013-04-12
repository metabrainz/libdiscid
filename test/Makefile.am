if RUN_TESTS
TESTS = test_discid
endif

check_PROGRAMS = test_discid

AM_CFLAGS = $(WARN_CFLAGS)
AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_builddir)/include

test_discid_SOURCES = test_discid.c
test_discid_LDADD = $(top_builddir)/src/libdiscid.la

