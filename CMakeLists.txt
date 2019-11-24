cmake_minimum_required(VERSION 3.13)
project(cado
    VERSION 0.9.2
    DESCRIPTION "Capability Ambient DO. Provide users just the capabilities they need."
    HOMEPAGE_URL "https://github.com/rd235/cado"
    LANGUAGES C)

include(GNUInstallDirs)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic")

set(LIBS_REQUIRED cap execs mhash pam pam_misc)

foreach(THISLIB IN LISTS LIBS_REQUIRED)
  find_library(${THISLIB}_library ${THISLIB})
  if(NOT ${THISLIB}_library)
    message(FATAL_ERROR "library lib${THISLIB} not found")
  endif()
endforeach(THISLIB)

if (WITHEDITOR)
        set(EDITOR "${WITHEDITOR}")
else (WITHEDITOR)
        set(EDITOR "/usr/bin/vi")
endif (WITHEDITOR)

configure_file(
        "${PROJECT_SOURCE_DIR}/include/config.h.in"
        "${PROJECT_SOURCE_DIR}/include/config.h"
)

add_subdirectory(man)
add_subdirectory(src)

add_custom_target(uninstall
        "${CMAKE_COMMAND}" -P "${PROJECT_SOURCE_DIR}/Uninstall.cmake")
