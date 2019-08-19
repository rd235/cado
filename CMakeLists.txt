# Set the minimum required version of cmake for a project.
cmake_minimum_required(VERSION 2.6)

# Sets the name of the project.
project("cado")

set(PACKAGE "cado")
set(PACKAGE_NAME "cado")
set(PACKAGE_TARNAME "cado")
set(VERSION "0.9.2")
set(PACKAGE_VERSION "0.9.2")
set(PACKAGE_STRING "cado 0.9.2")
set(PACKAGE_BUGREPORT "info@v2.cs.unibo.it")
set(PACKAGE_URL "")

include(GNUInstallDirs)
include_directories(${PROJECT_SOURCE_DIR})

# Compilation flags to be used when compiling C files.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

# Checks for libraries.
INCLUDE (CheckLibraryExists)

CHECK_LIBRARY_EXISTS(execs s2argv "" HAVE_LIBEXECS)
if(NOT HAVE_LIBEXECS)
    message(SEND_ERROR "Error: Could  not find execs library. (https://github.com/rd235/s2argv-execs)")
endif()

CHECK_LIBRARY_EXISTS(mhash mhash_init "" HAVE_LIBMHASH)
if(NOT HAVE_LIBMHASH)
    message(SEND_ERROR "Error: Could  not find mhash library.")
endif()

# Checks for header files.
INCLUDE (CheckIncludeFile)
INCLUDE (CheckIncludeFiles)

CHECK_INCLUDE_FILE(execs.h HAVE_EXECS_H)
CHECK_INCLUDE_FILE(mhash.h HAVE_MHASH_H)
CHECK_INCLUDE_FILE(fcntl.h HAVE_FCNTL_H)
CHECK_INCLUDE_FILE(memory.h HAVE_MEMORY_H)
check_include_files(unistd.h HAVE_UNISTD_H)

CHECK_INCLUDE_FILE(sys/capability.h HAVE_SYS_CAPABILITY_H)
if(NOT HAVE_SYS_CAPABILITY_H)
    message(SEND_ERROR "Error: Missing libcap header.")
endif()
CHECK_INCLUDE_FILE(sys/stat.h HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE(sys/types.h HAVE_SYS_TYPES_H)

CHECK_INCLUDE_FILE(security/pam_appl.h HAVE_SECURITY_PAM_APPL_H)
CHECK_INCLUDE_FILE(security/pam_misc.h HAVE_SECURITY_PAM_MISC_H)
if(NOT(HAVE_SECURITY_PAM_APPL_H OR HAVE_SECURITY_PAM_MISC_H))
    message(SEND_ERROR "Error: Missing PAM headers.")
endif()

CHECK_INCLUDE_FILE(execs.h HAVE_EXECS_H)
if(NOT HAVE_EXECS_H)
    message(SEND_ERROR "Error: Missing execs header.")
endif()

CHECK_INCLUDE_FILE(mhash.h HAVE_MHASH_H)
if(NOT HAVE_MHASH_H)
    message(SEND_ERROR "Error: Missing mhash header.")
endif()

INCLUDE(CheckFunctionExists)

# Checking whether system has ANSI C header files
CHECK_INCLUDE_FILES("dlfcn.h;stdint.h;stddef.h;inttypes.h;stdlib.h;stdarg.h;strings.h;string.h;float.h" STDC_HEADERS)
if(STDC_HEADERS)
    CHECK_FUNCTION_EXISTS(memchr HAVE_MEMCHR)
    if(HAVE_MEMCHR)
        CHECK_FUNCTION_EXISTS(free HAVE_FREE)
        if(HAVE_FREE)
            # ANSI C header files found
            set(HAVE_DLFCN_H 1)
            set(HAVE_STDINT_H 1)
            set(HAVE_STDDEF_H 1)
            set(HAVE_INTTYPES_H 1)
            set(HAVE_STDLIB_H 1)
            set(HAVE_STDARG_H 1)
            set(HAVE_STRINGS_H 1)
            set(HAVE_STRING_H 1)
            set(HAVE_FLOAT_H 1)
        endif(HAVE_FREE)
    endif(HAVE_MEMCHR)
endif(STDC_HEADERS)

# Checks for library functions.
CHECK_FUNCTION_EXISTS(strdup HAVE_STRDUP)
CHECK_FUNCTION_EXISTS(strtoull HAVE_STRTOULL)

if(WITHEDITOR)
    set(EDITOR "${WITHEDITOR}")
else()
    set(EDITOR "/usr/bin/vi")
endif()

# Set the paths.
if(NOT SPOOL_DIR)
    set(SPOOL_DIR "${localstatedir}/spool/cado" CACHE STRING "the directory where all the user scado files reside")
endif()
if(NOT CADO_EXE_DIR)
    set(CADO_EXE_DIR "/tmp" CACHE STRING "the directory where all the temporary executable files reside")
endif()

configure_file ("${PROJECT_SOURCE_DIR}/config.h.in" "${PROJECT_SOURCE_DIR}/config.h")

add_executable(cado cado.c pam_check.c get_user_groups.c capset_from_namelist.c read_conf.c set_ambient_cap.c compute_digest.c file_utils.c scado_parse.c cado_scado_check.c)
target_link_libraries(cado -lpam)
target_link_libraries(cado -lpam_misc)
target_link_libraries(cado -lcap)
target_link_libraries(cado -lmhash)
target_link_libraries(cado -lexecs)

add_executable(cadrop cadrop.c capset_from_namelist.c set_ambient_cap.c)
target_link_libraries(cadrop -lcap)

add_executable(scado scado.c pam_check.c file_utils.c compute_digest.c capset_from_namelist.c scado_parse.c)
target_link_libraries(scado -lpam)
target_link_libraries(scado -lpam_misc)
target_link_libraries(scado -lcap)
target_link_libraries(scado -lmhash)
target_link_libraries(scado -lexecs)

add_executable(caprint caprint.c)
target_link_libraries(caprint -lcap)

install(TARGETS cado cadrop scado caprint DESTINATION ${CMAKE_INSTALL_BINDIR})
install(SCRIPT "${PROJECT_SOURCE_DIR}/PostInstall.cmake")

file(GLOB MAN1_PAGES ${PROJECT_SOURCE_DIR}/*.1)
install(FILES ${MAN1_PAGES} DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

file(GLOB MAN5_PAGES ${PROJECT_SOURCE_DIR}/*.5)
install(FILES ${MAN5_PAGES} DESTINATION ${CMAKE_INSTALL_MANDIR}/man5)

add_custom_target(uninstall
    "${CMAKE_COMMAND}" -P "${PROJECT_SOURCE_DIR}/Uninstall.cmake"
)
