cmake_minimum_required(VERSION 2.6)
set(CMAKE_LEGACY_CYGWIN_WIN32 0)

PROJECT(libdiscid)

# VERSION NUMBERING:
# see http://semver.org, we use that schmea starting with 0.x.x already though
# short:
# MAJOR is increased when backwards incompatible changes are made to the API
# MINOR is increased when features are added to the API
# PATCH is increase for all other releases
SET(libdiscid_MAJOR 0)
SET(libdiscid_MINOR 5)
SET(libdiscid_PATCH 0)


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
    SET(libdiscid_OS "freebsd")
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
    SET(libdiscid_OS "openbsd")
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "NetBSD")
    SET(libdiscid_OS "openbsd")
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "SunOS")
    SET(libdiscid_OS "solaris")
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    SET(libdiscid_OS "win32")
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "CYGWIN")
    SET(libdiscid_OS "win32")
ELSE()
    SET(libdiscid_OS "generic")
    MESSAGE(STATUS "WARNING: unknown operating system, no disc reading available!")
ENDIF()

# choose platform dependent source files
IF(libdiscid_OS MATCHES "win32")
    SET(libdiscid_OSDEP_SRCS src/disc_win32.c)
    SET(libdiscid_RCS ${CMAKE_CURRENT_BINARY_DIR}/versioninfo.rc)
ELSEIF(libdiscid_OS MATCHES "darwin")
    SET(libdiscid_OSDEP_SRCS src/disc_darwin.c)
    FIND_LIBRARY(COREFOUNDATION_LIBRARY CoreFoundation)
    SET(libdiscid_OSDEP_LIBS ${COREFOUNDATION_LIBRARY} IOKit)
ELSE()
    SET(libdiscid_OSDEP_SRCS src/disc_${libdiscid_OS}.c)
ENDIF()

STRING(REPLACE ";" " " libdiscid_OSDEP_STR "${libdiscid_OSDEP_SRCS}")
MESSAGE(STATUS "Using discid implementation ${libdiscid_OSDEP_STR}")

ADD_LIBRARY(libdiscid SHARED ${libdiscid_OSDEP_SRCS} ${libdiscid_RCS} src/base64.c src/disc.c src/sha1.c)
TARGET_LINK_LIBRARIES(libdiscid ${libdiscid_OSDEP_LIBS})
SET_TARGET_PROPERTIES(libdiscid PROPERTIES
    OUTPUT_NAME discid
    VERSION ${libdiscid_VERSION}
    SOVERSION ${libdiscid_SOVERSION}
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/include)

ADD_EXECUTABLE(discid examples/discid.c)
TARGET_LINK_LIBRARIES(discid libdiscid)

ADD_EXECUTABLE(discisrc examples/discisrc.c)
TARGET_LINK_LIBRARIES(discisrc libdiscid)

ADD_EXECUTABLE(test_discid test/test_discid.c)
TARGET_LINK_LIBRARIES(test_discid libdiscid)

INSTALL(TARGETS libdiscid DESTINATION ${LIB_INSTALL_DIR})
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/libdiscid.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/discid/discid.h DESTINATION ${INCLUDE_INSTALL_DIR}/discid)


ADD_CUSTOM_TARGET(docs doxygen)

ADD_CUSTOM_TARGET(check ./test_discid DEPENDS test_discid)

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
