cmake_minimum_required(VERSION 2.6)
set(CMAKE_LEGACY_CYGWIN_WIN32 0)

PROJECT(libdiscid C)

# VERSION NUMBERING:
# see http://semver.org, we use that schmea starting with 0.x.x already though
# short:
# MAJOR is increased when backwards incompatible changes are made to the API
# MINOR is increased when features are added to the API
# PATCH is increase for all other releases
SET(libdiscid_MAJOR 0)
SET(libdiscid_MINOR 6)
SET(libdiscid_PATCH 2)


SET(libdiscid_VERSION ${libdiscid_MAJOR}.${libdiscid_MINOR}.${libdiscid_PATCH})
SET(libdiscid_SOVERSION ${libdiscid_MAJOR})
# version for windows resource file
SET(libdiscid_VERSION_RC ${libdiscid_MAJOR},${libdiscid_MINOR},${libdiscid_PATCH},0)
IF(libdiscid_PATCH LESS 10)
    SET(libdiscid_VERSION_NUM ${libdiscid_MINOR}0${libdiscid_PATCH})
ELSE()
    SET(libdiscid_VERSION_NUM ${libdiscid_MINOR}${libdiscid_PATCH})
ENDIF()

# currently the sole purpose of the project is the library,
# so we use that version
SET(PROJECT_VERSION ${libdiscid_VERSION})


SET(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)")
SET(EXEC_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE PATH "Installation prefix for executables and object code libraries" FORCE)
SET(BIN_INSTALL_DIR ${EXEC_INSTALL_PREFIX}/bin CACHE PATH "Installation prefix for user executables" FORCE)
SET(LIB_INSTALL_DIR ${EXEC_INSTALL_PREFIX}/lib${LIB_SUFFIX} CACHE PATH  "Installation prefix for object code libraries" FORCE)
SET(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include CACHE PATH "Installation prefix for C header files" FORCE)

# compatibility with automake
SET(PACKAGE ${PROJECT_NAME})
SET(VERSION ${PROJECT_VERSION})
SET(PACKAGE_STRING "${PACKAGE} ${VERSION}")
SET(prefix ${CMAKE_INSTALL_PREFIX})
SET(exec_prefix ${EXEC_INSTALL_PREFIX})
SET(includedir ${INCLUDE_INSTALL_DIR})
SET(libdir ${LIB_INSTALL_DIR})

CONFIGURE_FILE(libdiscid.pc.in libdiscid.pc)
CONFIGURE_FILE(versioninfo.rc.in versioninfo.rc)
CONFIGURE_FILE(Doxyfile.in Doxyfile)
CONFIGURE_FILE(include/discid/discid.h.in include/discid/discid.h)

# normalizing operating systems
IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    SET(libdiscid_OS "linux")
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    SET(libdiscid_OS "darwin")
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    SET(libdiscid_OS "bsd")
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "NetBSD")
    SET(libdiscid_OS "bsd")
    SET(libdiscid_OSDEP_LIBS util) # for getrawpartition
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
    SET(libdiscid_OS "bsd")
    SET(libdiscid_OSDEP_LIBS util) # for getrawpartition
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "SunOS")
    SET(libdiscid_OS "solaris")
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Haiku")
    SET(libdiscid_OS "haiku")
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    SET(libdiscid_OS "win32")
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "CYGWIN")
    SET(libdiscid_OS "win32")
ELSE()
    SET(libdiscid_OS "generic")
    MESSAGE(STATUS "WARNING: unknown operating system, no disc reading available!")
ENDIF()

# choose platform dependent source files
IF(libdiscid_OS STREQUAL "win32")
    SET(libdiscid_OSDEP_SRCS src/toc.c src/disc_win32.c)
    SET(libdiscid_RCS ${CMAKE_CURRENT_BINARY_DIR}/versioninfo.rc)
ELSEIF(libdiscid_OS STREQUAL "generic")
    SET(libdiscid_OSDEP_SRCS src/disc_${libdiscid_OS}.c)
ELSE()
    # unix platforms are the standard/default case
    SET(libdiscid_OSDEP_SRCS src/toc.c src/unix.c src/disc_${libdiscid_OS}.c)
    IF(libdiscid_OS STREQUAL "darwin") # Extra libraries needed
        FIND_LIBRARY(COREFOUNDATION_LIBRARY CoreFoundation)
        FIND_LIBRARY(IOKIT_LIBRARY IOKit)
        SET(libdiscid_OSDEP_LIBS ${COREFOUNDATION_LIBRARY} ${IOKIT_LIBRARY})
    ENDIF()
ENDIF()

STRING(REPLACE ";" " " libdiscid_OSDEP_STR "${libdiscid_OSDEP_SRCS}")
MESSAGE(STATUS "Using discid implementation ${libdiscid_OSDEP_STR}")

# This is only a dependency for a libmusicbrainz example
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
FIND_PACKAGE(Musicbrainz5 QUIET)
IF(NOT MUSICBRAINZ5_FOUND)
    SET(MUSICBRAINZ5_INCLUDE_DIRS "")
ENDIF()

ADD_LIBRARY(libdiscid SHARED ${libdiscid_OSDEP_SRCS} ${libdiscid_RCS} src/base64.c src/disc.c src/sha1.c)
TARGET_LINK_LIBRARIES(libdiscid ${libdiscid_OSDEP_LIBS})
SET_TARGET_PROPERTIES(libdiscid PROPERTIES
    OUTPUT_NAME discid
    VERSION ${libdiscid_VERSION}
    SOVERSION ${libdiscid_SOVERSION}
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}/include ${MUSICBRAINZ5_INCLUDE_DIRS})

ADD_EXECUTABLE(discid examples/discid.c)
TARGET_LINK_LIBRARIES(discid libdiscid)
ADD_EXECUTABLE(discisrc examples/discisrc.c)
TARGET_LINK_LIBRARIES(discisrc libdiscid)
IF(MUSICBRAINZ5_FOUND)
    ADD_EXECUTABLE(disc_metadata examples/disc_metadata.c)
    TARGET_LINK_LIBRARIES(disc_metadata libdiscid
                          ${MUSICBRAINZ5_LIBRARIES} stdc++)
ENDIF()

ADD_EXECUTABLE(test_core EXCLUDE_FROM_ALL test/test.c test/test_core.c)
TARGET_LINK_LIBRARIES(test_core libdiscid)
ADD_EXECUTABLE(test_put EXCLUDE_FROM_ALL test/test.c test/test_put.c)
TARGET_LINK_LIBRARIES(test_put libdiscid)
ADD_EXECUTABLE(test_read EXCLUDE_FROM_ALL test/test.c test/test_read.c)
TARGET_LINK_LIBRARIES(test_read libdiscid)
ADD_EXECUTABLE(test_read_full EXCLUDE_FROM_ALL test/test.c test/test_read_full.c)
TARGET_LINK_LIBRARIES(test_read_full libdiscid)

INSTALL(TARGETS libdiscid DESTINATION ${LIB_INSTALL_DIR})
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/libdiscid.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/discid/discid.h DESTINATION ${INCLUDE_INSTALL_DIR}/discid)


ADD_CUSTOM_TARGET(docs doxygen)

ADD_CUSTOM_TARGET(check
	COMMAND echo test_core:
	COMMAND echo ----------
	COMMAND ./test_core
	COMMAND echo && echo
	COMMAND echo test_put:
	COMMAND echo ---------
	COMMAND ./test_put
	COMMAND echo && echo
	COMMAND echo test_read:
	COMMAND echo ----------
	COMMAND ./test_read || test $$? -eq 77
	COMMAND echo && echo
	COMMAND echo test_read_full:
	COMMAND echo ---------------
	COMMAND ./test_read_full || test $$? -eq 77
	DEPENDS test_core test_put test_read test_read_full)

ADD_CUSTOM_TARGET(memcheck
	COMMAND valgrind --quiet --error-exitcode=1 --leak-check=full
		./test_core > /dev/null
	COMMAND valgrind --quiet --error-exitcode=1 --leak-check=full
		./test_put > /dev/null
	COMMAND valgrind --quiet --error-exitcode=1 --leak-check=full
		./test_read > /dev/null || test $$? -eq 77
	COMMAND valgrind --quiet --error-exitcode=1 --leak-check=full
		./test_read_full > /dev/null || test $$? -eq 77
	COMMAND valgrind --quiet --error-exitcode=66 --leak-check=full
		./discid > /dev/null || test $$? -ne 66
	COMMAND valgrind --quiet --error-exitcode=66 --leak-check=full
		./discisrc > /dev/null || test $$? -ne 66
	DEPENDS test_core test_put test_read test_read_full)

SET(libdiscid_DISTDIR "${PROJECT_NAME}-${PROJECT_VERSION}")

ADD_CUSTOM_TARGET(dist
   COMMAND git clone "${CMAKE_CURRENT_SOURCE_DIR}" "${libdiscid_DISTDIR}"
   COMMAND cd "${libdiscid_DISTDIR}" && ./autogen.sh
   COMMAND cd "${libdiscid_DISTDIR}" && ./configure && make distcheck
   COMMAND cp "${libdiscid_DISTDIR}/${libdiscid_DISTDIR}.tar.gz" .
   COMMAND rm -rf "${libdiscid_DISTDIR}"
)
ADD_DEPENDENCIES(dist check)

# create binary in MinGW/MSYS
IF(libdiscid_OS MATCHES "win32")
    ADD_CUSTOM_TARGET(distwin32
	COMMAND mkdir "${libdiscid_DISTDIR}-win32"
	COMMAND tar -xf "${libdiscid_DISTDIR}.tar.gz"
	COMMAND cp -a "${libdiscid_DISTDIR}" "${libdiscid_DISTDIR}-win32/"
	COMMAND rm -rf "${libdiscid_DISTDIR}"
	COMMAND cp "libdiscid.dll" "${libdiscid_DISTDIR}-win32/discid.dll"
	COMMAND zip -r "${libdiscid_DISTDIR}-win32.zip" "${libdiscid_DISTDIR}-win32/"
	COMMAND rm -rf "${libdiscid_DISTDIR}-win32"
    )
    ADD_DEPENDENCIES(distwin32 dist)
ENDIF()

# create universal binary on Mac OS X
IF(libdiscid_OS MATCHES "darwin")
    SET(darwin_build "${libdiscid_DISTDIR}/_build_darwin")
    ADD_CUSTOM_TARGET(distmac
        COMMAND mkdir "${libdiscid_DISTDIR}-mac"
        COMMAND tar -xf "${libdiscid_DISTDIR}.tar.gz"
        COMMAND cp -a "${libdiscid_DISTDIR}" "${libdiscid_DISTDIR}-mac/"
        COMMAND cd "${libdiscid_DISTDIR}-mac"
                && mkdir intel32 intel64 ppc universal
        COMMAND mkdir "${darwin_build}_32"
        COMMAND cd "${darwin_build}_32" && CMAKE_OSX_ARCHITECTURES=i386 cmake ..
        COMMAND cd "${darwin_build}_32" && make check
        COMMAND cp ${darwin_build}_32/libdiscid.${PROJECT_VERSION}.dylib
                "${libdiscid_DISTDIR}-mac/intel32/libdiscid.0.dylib"
        COMMAND mkdir "${darwin_build}_64"
        COMMAND cd "${darwin_build}_64"
                && CMAKE_OSX_ARCHITECTURES=x86_64 cmake ..
        COMMAND cd "${darwin_build}_64" && make check
        COMMAND cp ${darwin_build}_64/libdiscid.${PROJECT_VERSION}.dylib
                "${libdiscid_DISTDIR}-mac/intel64/libdiscid.0.dylib"
        COMMAND mkdir "${darwin_build}_ppc"
        COMMAND cd "${darwin_build}_ppc" && CMAKE_OSX_ARCHITECTURES=ppc cmake ..
        COMMAND cd "${darwin_build}_ppc" && make check
        COMMAND cp ${darwin_build}_ppc/libdiscid.${PROJECT_VERSION}.dylib
                "${libdiscid_DISTDIR}-mac/ppc/libdiscid.0.dylib"
        COMMAND lipo -create
                ${darwin_build}_*/libdiscid.${PROJECT_VERSION}.dylib
                -output "${libdiscid_DISTDIR}-mac/universal/libdiscid.0.dylib"
        COMMAND rm -rf "${darwin_build}_i386" "${darwin_build}_x86_64"
                "${darwin_build}_ppc"
        COMMAND rm -rf "${libdiscid_DISTDIR}"
        COMMAND zip -r "${libdiscid_DISTDIR}-mac.zip"
                "${libdiscid_DISTDIR}-mac/"
        COMMAND rm -rf "${libdiscid_DISTDIR}-mac"
    )
    ADD_DEPENDENCIES(distmac dist)
ENDIF()


# Tests needed for sha1.h
INCLUDE(TestBigEndian)
TEST_BIG_ENDIAN(WORDS_BIGENDIAN)
INCLUDE(CheckTypeSize)
CHECK_TYPE_SIZE(long SIZEOF_LONG)

CONFIGURE_FILE(config-cmake.h.in ${CMAKE_BINARY_DIR}/config.h)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
ADD_DEFINITIONS(-DHAVE_CONFIG_H)

IF(CMAKE_COMPILER_IS_GNUCC)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2")
ENDIF()
