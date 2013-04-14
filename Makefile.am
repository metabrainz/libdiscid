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
SUBDIRS = src examples test

pcdir = $(libdir)/pkgconfig
pc_DATA = libdiscid.pc

discid_incdir = $(includedir)/discid
discid_inc_HEADERS = include/discid/discid.h
noinst_HEADERS = include/discid/discid_private.h

EXTRA_DIST = libdiscid.pc.in Doxyfile.in CMakeLists.txt config-cmake.h.in

# not deleted automatically, in contrast to the .lo
CLEANFILES = versioninfo.o

docs:
	doxygen
