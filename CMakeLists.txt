cmake_minimum_required (VERSION 2.8.7)

project (xinput_pointermanager)

include_directories (src)

include (${xinput_pointermanager_SOURCE_DIR}/bld/debian-version.cmake)
include (${xinput_pointermanager_SOURCE_DIR}/bld/gengetopt.cmake)
include (${xinput_pointermanager_SOURCE_DIR}/bld/help2man.cmake)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frounding-math -std=c++0x")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${CMAKE_CURRENT_BINARY_DIR}")

include (GNUInstallDirs)

add_custom_command (
  OUTPUT xinput-pointermanager-options.c xinput-pointermanager-options.h
  COMMAND ${GENGETOPT_EXECUTABLE} ARGS --set-version ${DEBFULLVERSION} -F xinput-pointermanager-options --input=${CMAKE_CURRENT_SOURCE_DIR}/src/xinput-pointermanager.ggo
  DEPENDS src/xinput-pointermanager.ggo
  )
set_source_files_properties (src/xinput-pointermanager.cpp PROPERTIES OBJECT_DEPENDS xinput-pointermanager-options.h)
add_executable (xinput-pointermanager
  src/PointerManager.h
  src/PointerManager.cpp
  src/xinput-pointermanager.cpp
  xinput-pointermanager-options.c
  xinput-pointermanager-options.h
  xinput-pointermanager.1
)
manpage (xinput-pointermanager "manage synthetic XInput mouse pointers")
target_link_libraries (xinput-pointermanager Xi Xrandr X11)
install (TARGETS xinput-pointermanager RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

set (CPACK_PACKAGE_NAME "xinput-pointermanager")
set (CPACK_PACKAGE_CONTACT "Klee Dienes <klee@mit.edu>")
include (CPack)
