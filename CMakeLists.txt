# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

#list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

project(serialdv)

set(serialdv_SOURCES
  serialdatacontroller.cpp
  dv3000serial.cpp
  dvcontroller.cpp
)

set(serialdv_HEADERS
  serialdatacontroller.h
  dv3000serial.h
  dvcontroller.h
)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(serialdv SHARED
    ${serialdv_SOURCES}
)

install(TARGETS serialdv DESTINATION lib)
install(FILES ${serialdv_HEADERS} DESTINATION include/${PROJECT_NAME})
