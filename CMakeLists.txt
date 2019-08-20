cmake_minimum_required(VERSION 2.6)

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
include(CheckLibraryExists)

check_library_exists(execs s2argv "" HAVE_LIBEXECS)
if(NOT HAVE_LIBEXECS)
    message(SEND_ERROR "Error: Could  not find execs library. (https://github.com/rd235/s2argv-execs)")
endif()

check_library_exists(mhash mhash_init "" HAVE_LIBMHASH)
if(NOT HAVE_LIBMHASH)
    message(SEND_ERROR "Error: Could  not find mhash library.")
endif()

# Checks for header files.
include(CheckIncludeFile)

check_include_file(execs.h HAVE_EXECS_H)
check_include_file(mhash.h HAVE_MHASH_H)
check_include_file(fcntl.h HAVE_FCNTL_H)
check_include_file(memory.h HAVE_MEMORY_H)
check_include_file(unistd.h HAVE_UNISTD_H)

check_include_file(sys/capability.h HAVE_SYS_CAPABILITY_H)
if(NOT HAVE_SYS_CAPABILITY_H)
    message(SEND_ERROR "Error: Missing libcap header.")
endif()

check_include_file(sys/stat.h HAVE_SYS_STAT_H)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)

# Checks for typedefs, structures, and compiler characteristics.
include(CheckTypeSize)

check_type_size(uid_t SIZEOF_UID_T)
if(SIZEOF_UID EQUAL "")
    set(uid_t int)
endif()

check_type_size(gid_t SIZEOF_GID_T)
if(SIZEOF_GID_T EQUAL "")
    set(gid_t int)
endif()

check_type_size(pid_t SIZEOF_PID_T)
if(SIZEOF_PID_T EQUAL "")
    set(pid_t int)
endif()

check_type_size(ssize_t SIZEOF_SSIZE_T)
if(SIZEOF_SSIZE_T EQUAL "")
    set(ssize_t int)
endif()

check_type_size(uint64_t SIZEOF_UINT64_T)
if(SIZEOF_UINT64_T EQUAL "")
    message(STATUS "'uint64_t': identifier not found")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "(Solaris|SunOS)" )
    set(_UINT64_T 1)
endif()

check_include_file(security/pam_appl.h HAVE_SECURITY_PAM_APPL_H)
check_include_file(security/pam_misc.h HAVE_SECURITY_PAM_MISC_H)
if(NOT(HAVE_SECURITY_PAM_APPL_H OR HAVE_SECURITY_PAM_MISC_H))
    message(SEND_ERROR "Error: Missing PAM headers.")
endif()

check_include_file(execs.h HAVE_EXECS_H)
if(NOT HAVE_EXECS_H)
    message(SEND_ERROR "Error: Missing execs header.")
endif()

check_include_file(mhash.h HAVE_MHASH_H)
if(NOT HAVE_MHASH_H)
    message(SEND_ERROR "Error: Missing mhash header.")
endif()

# Checking whether system has ANSI C header files
include(CheckIncludeFiles)
include(CheckFunctionExists)

check_include_files("dlfcn.h;stdint.h;stddef.h;inttypes.h;stdlib.h;stdarg.h;strings.h;string.h;float.h" STDC_HEADERS)
if(STDC_HEADERS)
    check_function_exists(memchr HAVE_MEMCHR)
    if(HAVE_MEMCHR)
        check_function_exists(free HAVE_FREE)
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
check_function_exists(strdup HAVE_STRDUP)
check_function_exists(strtoull HAVE_STRTOULL)

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

configure_file("${PROJECT_SOURCE_DIR}/config.h.in" "${PROJECT_SOURCE_DIR}/config.h")

add_executable(cado cado.c pam_check.c get_user_groups.c capset_from_namelist.c read_conf.c set_ambient_cap.c compute_digest.c file_utils.c scado_parse.c cado_scado_check.c)
target_link_libraries(cado -lpam -lpam_misc -lcap -lmhash -lexecs)

add_executable(cadrop cadrop.c capset_from_namelist.c set_ambient_cap.c)
target_link_libraries(cadrop -lcap)

add_executable(scado scado.c pam_check.c file_utils.c compute_digest.c capset_from_namelist.c scado_parse.c)
target_link_libraries(scado -lpam -lpam_misc -lcap -lmhash -lexecs)

add_executable(caprint caprint.c)
target_link_libraries(caprint -lcap)

install(TARGETS cado cadrop scado caprint DESTINATION ${CMAKE_INSTALL_BINDIR})

file(GLOB MAN1_PAGES ${PROJECT_SOURCE_DIR}/*.1)
install(FILES ${MAN1_PAGES} DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

file(GLOB MAN5_PAGES ${PROJECT_SOURCE_DIR}/*.5)
install(FILES ${MAN5_PAGES} DESTINATION ${CMAKE_INSTALL_MANDIR}/man5)

install(CODE "execute_process(
    COMMAND ${CMAKE_COMMAND}
    -DBINDIR=${CMAKE_INSTALL_FULL_BINDIR}
    -DLIBDIR=${CMAKE_INSTALL_FULL_LIBDIR}
    -P ${PROJECT_SOURCE_DIR}/PostInstall.cmake
)")

add_custom_target(post_install
    "${CMAKE_COMMAND}" -P "${PROJECT_SOURCE_DIR}/PostInstall.cmake"
)

add_custom_target(uninstall
    "${CMAKE_COMMAND}" -P "${PROJECT_SOURCE_DIR}/Uninstall.cmake"
)
