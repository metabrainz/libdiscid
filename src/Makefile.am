#
#  MusicBrainz -- The Internet music metadatabase
#
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
#    $Id$
#
lib_LTLIBRARIES = libdiscid.la

libdiscid_la_SOURCES = base64.c sha1.c disc.c
EXTRA_libdiscid_la_SOURCES = \
	disc_linux.c disc_win32.c disc_win32_new.c \
	disc_darwin.c disc_freebsd.c disc_solaris.c disc_openbsd.c

AM_CPPFLAGS = -I$(top_srcdir)/include
libdiscid_la_LIBADD = @DISC_OS_OBJ@
libdiscid_la_DEPENDENCIES = @DISC_OS_OBJ@

# Here are a set of rules to help you update your library version information:
#  Scheme: current:revsion:age
#  1. Start with version information of `0:0:0' for each libtool library.
#  2. Update the version information only immediately before a public release 
#     of your software. More frequent updates are unnecessary, and only 
#     guarantee that the current interface number gets larger faster.
#  3. If the library source code has changed at all since the last update, 
#     then increment revision (`c:r:a' becomes `c:r+1:a').
#  4. If any interfaces have been added, removed, or changed since the last 
#     update, increment current, and set revision to 0.
#  5. If any interfaces have been added since the last public release, then 
#     increment age.
#  6. If any interfaces have been removed since the last public release, then 
#     set age to 0. 
libdiscid_la_LDFLAGS = -version-info 3:2:3 -no-undefined

EXTRA_DIST = base64.h sha1.h

# for Windows Resource Files
RCCOMPILE = $(RC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS)
LTRCCOMPILE = $(LIBTOOL) --mode=compile --tag=RC $(RCCOMPILE)

.rc.o:
	$(RCCOMPILE) -i "$<" -o "$@"

.rc.lo:
	$(LTRCCOMPILE) -i "$<" -o "$@"
