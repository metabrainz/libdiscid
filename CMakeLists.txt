PROJECT(libdiscid)
SET(PROJECT_VERSION 0.1.1)

# 1. If the library source code has changed at all since the last update, then increment revision.
# 2. If any interfaces have been added, removed, or changed since the last update, increment current, and set revision to 0.
# 3. If any interfaces have been added since the last public release, then increment age.
# 4. If any interfaces have been removed since the last public release, then set age to 0. 
SET(libdiscid_SOVERSION_CURRENT  0)
SET(libdiscid_SOVERSION_REVISION 1)
SET(libdiscid_SOVERSION_AGE      0)

SET(libdiscid_VERSION ${libdiscid_SOVERSION_CURRENT}.${libdiscid_SOVERSION_AGE}.${libdiscid_SOVERSION_REVISION})
SET(libdiscid_SOVERSION ${libdiscid_SOVERSION_CURRENT})

SET(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)")
SET(EXEC_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE PATH "Installation prefix for executables and object code libraries" FORCE)
SET(BIN_INSTALL_DIR ${EXEC_INSTALL_PREFIX}/bin CACHE PATH "Installation prefix for user executables" FORCE)
SET(LIB_INSTALL_DIR ${EXEC_INSTALL_PREFIX}/lib${LIB_SUFFIX} CACHE PATH  "Installation prefix for object code libraries" FORCE)
SET(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include CACHE PATH "Installation prefix for C header files" FORCE)

# compatibility with automake
SET(PACKAGE ${PROJECT_NAME})
SET(VERSION ${PROJECT_VERSION})
SET(prefix ${CMAKE_INSTALL_PREFIX})
SET(exec_prefix ${EXEC_INSTALL_PREFIX})
SET(includedir ${INCLUDE_INSTALL_DIR})
SET(libdir ${LIB_INSTALL_DIR})

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/libdiscid.pc.in ${CMAKE_CURRENT_BINARY_DIR}/libdiscid.pc)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    SET(libdiscid_OSDEP_SRCS src/disc_linux.c)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    SET(libdiscid_OSDEP_SRCS src/disc_darwin.c)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    SET(libdiscid_OSDEP_SRCS src/disc_win32.c src/disc_win32_new.c)
    SET(libdiscid_OSDEP_LIBS winmm)
ELSE(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    MESSAGE(FATAL_ERROR	"unsupported operating system")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

ADD_LIBRARY(libdiscid SHARED ${libdiscid_OSDEP_SRCS} src/base64.c src/disc.c src/sha1.c)
TARGET_LINK_LIBRARIES(libdiscid ${libdiscid_OSDEP_LIBS})
SET_TARGET_PROPERTIES(libdiscid PROPERTIES
    OUTPUT_NAME discid
    VERSION ${libdiscid_VERSION}
    SOVERSION ${libdiscid_SOVERSION}
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

ADD_EXECUTABLE(discid examples/discid.c)
TARGET_LINK_LIBRARIES(discid libdiscid)

ADD_EXECUTABLE(test_discid test/test_discid.c)
TARGET_LINK_LIBRARIES(test_discid libdiscid)

INSTALL(TARGETS libdiscid DESTINATION ${LIB_INSTALL_DIR})
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/libdiscid.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
INSTALL(FILES include/discid/discid.h DESTINATION ${INCLUDE_INSTALL_DIR}/discid)

ADD_CUSTOM_TARGET(docs doxygen)

ADD_CUSTOM_TARGET(dist
    rm -rf "${PROJECT_NAME}-${PROJECT_VERSION}"
    COMMAND svn export http://svn.musicbrainz.org/${PROJECT_NAME}/trunk "${PROJECT_NAME}-${PROJECT_VERSION}"
    COMMAND tar -zcvf "${PROJECT_NAME}-${PROJECT_VERSION}.tar.gz" "${PROJECT_NAME}-${PROJECT_VERSION}"
)
