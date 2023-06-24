cmake_minimum_required(VERSION 3.13)
project(cado
	VERSION 0.9.6
	DESCRIPTION "Capability Ambient DO. Provide users just the capabilities they need."
	HOMEPAGE_URL "https://github.com/rd235/cado"
	LANGUAGES C)

include(GNUInstallDirs)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FORTIFY_SOURCE=2 -O2 -Wall -pedantic")

set(LIBS_REQUIRED cap execs mhash pam pam_misc)

option(CADO_RUN_POST_INSTALL "Run PostInstall.cmake" ON)

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

# Spool dir for scado files
set(CADO_SPOOL_DIR
	"${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/spool/${CMAKE_PROJECT_NAME}")

configure_file(
	"include/config.h.in"
	"include/config.h"
)

add_subdirectory(man)
add_subdirectory(src)

add_custom_target(uninstall
	"${CMAKE_COMMAND}"
		-DCADO_SPOOL_DIR=${CADO_SPOOL_DIR}
		-P "${PROJECT_SOURCE_DIR}/Uninstall.cmake"
	)
