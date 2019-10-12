cmake_minimum_required(VERSION 3.1.0)

project(serialdv)

set(MAJOR_VERSION 1)
set(MINOR_VERSION 1)
set(PATCH_VERSION 2)
set(VERSION_STRING ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION})
set(VERSION ${VERSION_STRING})

include(GNUInstallDirs)
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}") # "lib" or "lib64"

option(BUILD_TOOL "Build dvtest tool" ON)

# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH  FALSE)

# use c++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

#list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

# Compiler flags.
if(WIN32)
  add_definitions(-D__WINDOWS__)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall ${EXTRA_FLAGS}")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2 -ffast-math -ftree-vectorize ${EXTRA_FLAGS}")
endif()

set(serialdv_SOURCES
  datacontroller.cpp
  dummydatacontroller.cpp
  dvcontroller.cpp
)

set(serialdv_HEADERS
  serialdv_export.h
  datacontroller.h
  dummydatacontroller.h
  dvcontroller.h
)

if (NOT APPLE)
    set(serialdv_SOURCES
        ${serialdv_SOURCES}
        serialdatacontroller.cpp
        udpdatacontroller.cpp
    )
    set(serialdv_HEADERS
        ${serialdv_SOURCES}
        serialdatacontroller.h
        udpdatacontroller.h
    )
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(serialdv SHARED
    ${serialdv_SOURCES}
)

if(BUILD_TOOL AND NOT WIN32)
add_executable(dvtest
    dvtest.cpp
)

target_include_directories(dvtest PUBLIC
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(serialdv PROPERTIES VERSION ${VERSION} SOVERSION ${MAJOR_VERSION})

target_link_libraries(dvtest serialdv)

install(TARGETS dvtest DESTINATION bin)
endif(BUILD_TOOL AND NOT WIN32)

install(TARGETS serialdv LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${serialdv_HEADERS} DESTINATION include/${PROJECT_NAME})
