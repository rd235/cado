cmake_minimum_required(VERSION 2.6)

project("cado" C)

set(PACKAGE "cado")
set(PACKAGE_NAME "cado")
set(PACKAGE_TARNAME "cado")
set(VERSION "0.9.2")
set(PACKAGE_VERSION "0.9.2")
set(PACKAGE_STRING "cado 0.9.2")
set(PACKAGE_BUGREPORT "info@v2.cs.unibo.it")
set(PACKAGE_URL "")

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

include(GNUInstallDirs)
include_directories(${PROJECT_SOURCE_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

find_package(PAM REQUIRED)
find_package(PAM_MISC REQUIRED)
find_package(EXECS REQUIRED)
find_package(MHASH REQUIRED)
find_package(LIBCAP REQUIRED)

include(CheckIncludeFile)
include(CheckIncludeFiles)
check_include_file(fcntl.h HAVE_FCNTL_H)
check_include_file(stdint.h HAVE_STDINT_H)
check_include_file(stdlib.h HAVE_STDLIB_H)
check_include_file(string.h HAVE_STRING_H)
check_include_file(unistd.h HAVE_UNISTD_H)

include(CheckTypeSize)
check_type_size(uid_t SIZEOF_UID_T)
if (SIZEOF_UID EQUAL "")
        set(uid_t int)
endif (SIZEOF_UID EQUAL "")
check_type_size(gid_t SIZEOF_GID_T)
if (SIZEOF_GID_T EQUAL "")
        set(gid_t int)
endif (SIZEOF_GID_T EQUAL "")
check_type_size(pid_t SIZEOF_PID_T)
if (SIZEOF_PID_T EQUAL "")
        set(pid_t int)
endif (SIZEOF_PID_T EQUAL "")
check_type_size(ssize_t SIZEOF_SSIZE_T)
if (SIZEOF_SSIZE_T EQUAL "")
        set(ssize_t int)
endif (SIZEOF_SSIZE_T EQUAL "")
check_type_size(uint64_t SIZEOF_UINT64_T)
if (SIZEOF_UINT64_T EQUAL "")
        message(STATUS "'uint64_t': identifier not found")
endif (SIZEOF_UINT64_T EQUAL "")

include(CheckFunctionExists)
check_function_exists(strdup HAVE_STRDUP)
check_function_exists(strtoull HAVE_STRTOULL)

if (WITHEDITOR)
        set(EDITOR "${WITHEDITOR}")
else (WITHEDITOR)
        set(EDITOR "/usr/bin/vi")
endif (WITHEDITOR)

if (NOT SPOOL_DIR)
        set(
                SPOOL_DIR "${localstatedir}/spool/cado"
                CACHE STRING "where all the user scado files reside"
        )
endif (NOT SPOOL_DIR)
if (NOT CADO_EXE_DIR)
        set(
                CADO_EXE_DIR "/tmp"
                CACHE STRING "where all the temporary executable files reside"
        )
endif (NOT CADO_EXE_DIR)

configure_file(
        "${PROJECT_SOURCE_DIR}/config.h.in"
        "${PROJECT_SOURCE_DIR}/config.h"
)

add_executable(
        cado cado.c pam_check.c get_user_groups.c capset_from_namelist.c
        read_conf.c set_ambient_cap.c compute_digest.c file_utils.c
        scado_parse.c cado_scado_check.c
)
target_link_libraries(
        cado ${PAM_LIBRARY} ${PAM_MISC_LIBRARY} ${LIBCAP_LIBRARY} ${MHASH_LIBRARY}
        ${EXECS_LIBRARY})

add_executable(cadrop cadrop.c capset_from_namelist.c set_ambient_cap.c)
target_link_libraries(cadrop ${LIBCAP_LIBRARY})

add_executable(
        scado scado.c pam_check.c file_utils.c compute_digest.c
        capset_from_namelist.c scado_parse.c
)
target_link_libraries(
        scado ${PAM_LIBRARY} ${PAM_MISC_LIBRARY} ${LIBCAP_LIBRARY} ${MHASH_LIBRARY}
        ${EXECS_LIBRARY}
)

add_executable(caprint caprint.c)
target_link_libraries(caprint ${LIBCAP_LIBRARY})

install(TARGETS cado cadrop scado caprint DESTINATION ${CMAKE_INSTALL_BINDIR})

file(GLOB MAN1_PAGES ${PROJECT_SOURCE_DIR}/*.1)
install(FILES ${MAN1_PAGES} DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

file(GLOB MAN5_PAGES ${PROJECT_SOURCE_DIR}/*.5)
install(FILES ${MAN5_PAGES} DESTINATION ${CMAKE_INSTALL_MANDIR}/man5)

install(
        CODE "execute_process(
                COMMAND ${CMAKE_COMMAND}
                -DBINDIR=${CMAKE_INSTALL_FULL_BINDIR}
                -DLIBDIR=${CMAKE_INSTALL_FULL_LIBDIR}
                -P ${PROJECT_SOURCE_DIR}/PostInstall.cmake
        )"
)

add_custom_target(
        uninstall
        "${CMAKE_COMMAND}" -P "${PROJECT_SOURCE_DIR}/Uninstall.cmake"
)
