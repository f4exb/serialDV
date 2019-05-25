cmake_minimum_required(VERSION 3.1.0)

project(serialdv)

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
  serialdatacontroller.cpp
  dvcontroller.cpp
)

set(serialdv_HEADERS
  serialdv_export.h
  serialdatacontroller.h
  dvcontroller.h
)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(serialdv SHARED
    ${serialdv_SOURCES}
)

if(BUILD_TOOL)
add_executable(dvtest
    dvtest.cpp
)

target_include_directories(dvtest PUBLIC
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(dvtest serialdv)

install(TARGETS dvtest DESTINATION bin)
endif(BUILD_TOOL)

install(TARGETS serialdv DESTINATION lib)
install(FILES ${serialdv_HEADERS} DESTINATION include/${PROJECT_NAME})
