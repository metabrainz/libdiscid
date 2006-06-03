lib_LTLIBRARIES = libdiscid.la

libdiscid_la_SOURCES = base64.c sha1.c disc.c
EXTRA_libdiscid_la_SOURCES = disc_linux.c disc_win32.c

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
libdiscid_la_LDFLAGS = -version-info 0:0:0 -no-undefined

EXTRA_DIST = base64.h sha1.h
