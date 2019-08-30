# Try to find the EXECS library
# Once done this will define
#
#  EXECS_FOUND - system has execs
#  EXECS_INCLUDE_DIR - the execs include directory
#  EXECS_LIBRARIES - libexecs libraries

if (EXECS_LIBRARY)
	set(EXECS_FIND_QUIETLY TRUE)
endif (EXECS_LIBRARY)

find_path(EXECS_INCLUDE_DIR execs.h)
find_library(EXECS_LIBRARY execs)

if (EXECS_INCLUDE_DIR AND EXECS_LIBRARY)
        set(EXECS_FOUND TRUE)
	set(EXECS_LIBRARIES ${EXECS_LIBRARY})
        set(HAVE_LIBEXECS 1)

	if (EXISTS ${EXECS_INCLUDE_DIR})
		set(HAVE_EXECS_H 1)
	endif (EXISTS ${EXECS_INCLUDE_DIR})
endif (EXECS_INCLUDE_DIR AND EXECS_LIBRARY)

if (EXECS_FOUND)
	if (NOT EXECS_FIND_QUIETLY)
		message(STATUS "Found execs: ${EXECS_LIBRARY}")
	endif (NOT EXECS_FIND_QUIETLY)
else (EXECS_FOUND)
	if (EXECS_FIND_REQUIRED)
                message(FATAL_ERROR "Could not find execs library")
	endif(EXECS_FIND_REQUIRED)
endif (EXECS_FOUND)

mark_as_advanced(EXECS_LIBRARY DL_LIBRARY)
