# Try to find the PAM_MISC library
# Once done this will define
#
#  PAM_MISC_FOUND - system has pam_misc
#  PAM_MISC_INCLUDE_DIR - the pam_misc include directory
#  PAM_MISC_LIBRARIES - libpam_misc libraries

if (PAM_MISC_LIBRARY)
	set(PAM_MISC_FIND_QUIETLY TRUE)
endif (PAM_MISC_LIBRARY)

find_path(PAM_MISC_INCLUDE_DIR NAMES security/pam_misc.h pam/pam_misc.h)
find_library(PAM_MISC_LIBRARY pam_misc)

if (PAM_MISC_INCLUDE_DIR AND PAM_MISC_LIBRARY)
        set(PAM_MISC_FOUND TRUE)
	set(PAM_MISC_LIBRARIES ${PAM_MISC_LIBRARY})

	if (EXISTS ${PAM_MISC_INCLUDE_DIR})
		set(HAVE_SECURITY_PAM_MISC_H 1)
	endif (EXISTS ${PAM_MISC_INCLUDE_DIR})
endif (PAM_MISC_INCLUDE_DIR AND PAM_MISC_LIBRARY)

if (PAM_MISC_FOUND)
	if (NOT PAM_MISC_FIND_QUIETLY)
		message(STATUS "Found PAM misc: ${PAM_MISC_LIBRARIES}")
	endif (NOT PAM_MISC_FIND_QUIETLY)
else (PAM_MISC_FOUND)
	if (PAM_MISC_FIND_REQUIRED)
		message(FATAL_ERROR "Could not find PAM misc library")
	endif(PAM_MISC_FIND_REQUIRED)
endif (PAM_MISC_FOUND)

mark_as_advanced(PAM_MISC_LIBRARY DL_LIBRARY)
