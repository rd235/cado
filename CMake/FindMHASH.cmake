# Try to find the MHASH library
# Once done this will define
#
#  MHASH_FOUND - system has mhash
#  MHASH_INCLUDE_DIR - the mhash include directory
#  MHASH_LIBRARIES - libmhash libraries

if (MHASH_LIBRARY)
	set(MHASH_FIND_QUIETLY TRUE)
endif (MHASH_LIBRARY)

find_path(MHASH_INCLUDE_DIR mhash.h)
find_library(MHASH_LIBRARY mhash)

if (MHASH_INCLUDE_DIR AND MHASH_LIBRARY)
        set(MHASH_FOUND TRUE)
	set(MHASH_LIBRARIES ${MHASH_LIBRARY})
        set(HAVE_LIBMHASH 1)

	if (EXISTS ${MHASH_INCLUDE_DIR})
		set(HAVE_MHASH_H 1)
	endif (EXISTS ${MHASH_INCLUDE_DIR})
endif (MHASH_INCLUDE_DIR AND MHASH_LIBRARY)

if (MHASH_FOUND)
	if (NOT MHASH_FIND_QUIETLY)
		message(STATUS "Found MHASH: ${MHASH_LIBRARY}")
	endif (NOT MHASH_FIND_QUIETLY)
else (MHASH_FOUND)
	if (MHASH_FIND_REQUIRED)
                message(FATAL_ERROR "MHASH was not found")
	endif(MHASH_FIND_REQUIRED)
endif (MHASH_FOUND)

mark_as_advanced(MHASH_LIBRARY DL_LIBRARY)
