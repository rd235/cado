# Try to find the LIBCAP library
# Once done this will define
#
#  LIBCAP_FOUND - system has libcap
#  LIBCAP_INCLUDE_DIR - the capability include directory
#  LIBCAP_LIBRARIES - libcap libraries

if (LIBCAP_LIBRARY)
	set(LIBCAP_FIND_QUIETLY TRUE)
endif (LIBCAP_LIBRARY)

find_path(LIBCAP_INCLUDE_DIR sys/capability.h)
find_library(LIBCAP_LIBRARY cap)

if (LIBCAP_INCLUDE_DIR AND LIBCAP_LIBRARY)
        set(LIBCAP_FOUND TRUE)
	set(LIBCAP_LIBRARIES ${LIBCAP_LIBRARY})
        set(HAVE_LIBCAP 1)

	if (EXISTS ${LIBCAP_INCLUDE_DIR})
		set(HAVE_SYS_CAPABILITY_H 1)
        else (EXISTS ${LIBCAP_INCLUDE_DIR})
                message(FATAL_ERROR "Missing libcap header.")
	endif (EXISTS ${LIBCAP_INCLUDE_DIR})
endif (LIBCAP_INCLUDE_DIR AND LIBCAP_LIBRARY)

if (LIBCAP_FOUND)
	if (NOT LIBCAP_FIND_QUIETLY)
		message(STATUS "Found LIBCAP: ${LIBCAP_LIBRARY}")
	endif (NOT LIBCAP_FIND_QUIETLY)
else (LIBCAP_FOUND)
	if (LIBCAP_FIND_REQUIRED)
                message(FATAL_ERROR "LIBCAP was not found")
	endif(LIBCAP_FIND_REQUIRED)
endif (LIBCAP_FOUND)

mark_as_advanced(LIBCAP_LIBRARY DL_LIBRARY)
