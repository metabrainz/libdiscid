#
#  MusicBrainz -- The Internet music metadatabase
#
#  Copyright (C) 2013 Johannes Dewender
#  Copyright (C) 2006 Matthias Friedrich
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2.1 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#

EXTRA_DIST = libdiscid.pc.in Doxyfile.in CMakeLists.txt config-cmake.h.in

# not deleted automatically, in contrast to the .lo
CLEANFILES = versioninfo.o

AM_CFLAGS = $(WARN_CFLAGS)
AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_builddir)/include


pcdir = $(libdir)/pkgconfig
pc_DATA = libdiscid.pc

discid_incdir = $(includedir)/discid
discid_inc_HEADERS = include/discid/discid.h
noinst_HEADERS = include/discid/discid_private.h src/base64.h src/sha1.h
noinst_HEADERS += test/test.h src/unix.h src/ntddcdrm.h


if RUN_TESTS
TESTS = test_core test_put test_read test_read_full
endif

# put tests that don't work here (so it shows up as expected failure)
XFAIL =

check_PROGRAMS = test_core test_put test_read test_read_full
noinst_PROGRAMS = discid discisrc

# Tests
test_core_SOURCES = test/test.c test/test_core.c
test_core_LDADD = $(top_builddir)/libdiscid.la
test_put_SOURCES = test/test.c test/test_put.c
test_put_LDADD = $(top_builddir)/libdiscid.la
test_read_SOURCES = test/test.c test/test_read.c
test_read_LDADD = $(top_builddir)/libdiscid.la
test_read_full_SOURCES = test/test.c test/test_read_full.c
test_read_full_LDADD = $(top_builddir)/libdiscid.la

# Examples
discid_SOURCES = examples/discid.c
discid_LDADD = $(top_builddir)/libdiscid.la
discisrc_SOURCES = examples/discisrc.c
discisrc_LDADD = $(top_builddir)/libdiscid.la
if HAVE_MUSICBRAINZ5
noinst_PROGRAMS += disc_metadata
disc_metadata_SOURCES = examples/disc_metadata.c
disc_metadata_LDADD = $(top_builddir)/libdiscid.la -lmusicbrainz5
endif


lib_LTLIBRARIES = libdiscid.la

libdiscid_la_SOURCES = src/base64.c src/sha1.c src/disc.c

# use a (well defined) version number, rather than version-info calculations
libdiscid_la_LDFLAGS = -version-number @libdiscid_VERSION_LT@ -no-undefined
libdiscid_la_LIBADD =

#if OS_BEOS
#libdiscid_la_LIBADD += -lbe -lroot
#endif
if OS_DARWIN
libdiscid_la_LDFLAGS += -framework CoreFoundation -framework IOKit
libdiscid_la_SOURCES += src/toc.c src/unix.c src/disc_darwin.c
endif
if OS_NETBSD
libdiscid_la_SOURCES += src/toc.c src/unix.c src/disc_bsd.c
libdiscid_la_LIBADD += -lutil
endif
if OS_FREEBSD
libdiscid_la_SOURCES += src/toc.c src/unix.c src/disc_bsd.c
endif
if OS_GENERIC
libdiscid_la_SOURCES += src/disc_generic.c
endif
if OS_LINUX
libdiscid_la_SOURCES += src/toc.c src/unix.c src/disc_linux.c
endif
#if OS_QNX
#libdiscid_la_LIBADD += -lsocket
#endif
if OS_SOLARIS
libdiscid_la_SOURCES += src/toc.c src/unix.c src/disc_solaris.c
endif
if OS_WIN32
libdiscid_la_SOURCES += src/toc.c src/disc_win32.c versioninfo.rc
endif


# for Windows Resource Files
RCCOMPILE = $(RC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS)
LTRCCOMPILE = $(LIBTOOL) --mode=compile --tag=RC $(RCCOMPILE)

.rc.o:
	$(AM_V_GEN)$(RCCOMPILE) -i "$<" -o "$@"

.rc.lo:
	$(AM_V_GEN)$(LTRCCOMPILE) -i "$<" -o "$@"


docs:
	doxygen

git-clean: maintainer-clean
	rm -rf configure config.h.in Makefile.in
	rm -rf aclocal.m4 build-aux/
