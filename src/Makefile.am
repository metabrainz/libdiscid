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
lib_LTLIBRARIES = libdiscid.la

libdiscid_la_SOURCES = base64.c sha1.c disc.c

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_builddir)/include
libdiscid_la_CFLAGS = $(WARN_CFLAGS)

# use a (well defined) version number, rather than version-info calculations
libdiscid_la_LDFLAGS = -version-number @libdiscid_VERSION_LT@ -no-undefined
libdiscid_la_LIBADD =

#if OS_BEOS
#libdiscid_la_LIBADD += "-lbe -lroot"
#endif
if OS_DARWIN
libdiscid_la_LDFLAGS += "-framework CoreFoundation -framework IOKit"
libdiscid_la_SOURCES += disc_darwin.c
endif
if OS_FREEBSD
libdiscid_la_SOURCES += disc_freebsd.c
endif
if OS_GENERIC
libdiscid_la_SOURCES += disc_generic.c
endif
if OS_LINUX
libdiscid_la_SOURCES += disc_linux.c
endif
if OS_OPENBSD
libdiscid_la_SOURCES += disc_openbsd.c
endif
#if OS_QNX
#libdiscid_la_LIBADD += "-lsocket"
#endif
if OS_SOLARIS
libdiscid_la_SOURCES += disc_solaris.c
endif
if OS_WIN32
libdiscid_la_LIBADD += -lwinmm
libdiscid_la_SOURCES += disc_win32.c disc_win32_new.c versioninfo.rc
endif

EXTRA_DIST = base64.h sha1.h

# for Windows Resource Files
RCCOMPILE = $(RC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS)
LTRCCOMPILE = $(LIBTOOL) --mode=compile --tag=RC $(RCCOMPILE)

.rc.o:
	$(RCCOMPILE) -i "$<" -o "$@"

.rc.lo:
	$(LTRCCOMPILE) -i "$<" -o "$@"
