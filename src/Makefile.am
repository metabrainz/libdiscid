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
EXTRA_libdiscid_la_SOURCES = \
	disc_linux.c disc_win32.c disc_win32_new.c \
	disc_darwin.c disc_freebsd.c disc_solaris.c disc_openbsd.c \
	disc_generic.c

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_builddir)/include
libdiscid_la_CFLAGS = $(WARN_CFLAGS)
libdiscid_la_LIBADD = @DISC_OS_OBJ@
libdiscid_la_DEPENDENCIES = @DISC_OS_OBJ@

# use a (well defined) version number, rather than version-info calculations
libdiscid_la_LDFLAGS = -version-number @libdiscid_VERSION_LT@ -no-undefined

EXTRA_DIST = base64.h sha1.h

# for Windows Resource Files
RCCOMPILE = $(RC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS)
LTRCCOMPILE = $(LIBTOOL) --mode=compile --tag=RC $(RCCOMPILE)

.rc.o:
	$(RCCOMPILE) -i "$<" -o "$@"

.rc.lo:
	$(LTRCCOMPILE) -i "$<" -o "$@"
