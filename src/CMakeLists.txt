cmake_minimum_required(VERSION 3.13)

add_executable(
        cado cado.c pam_check.c get_user_groups.c capset_from_namelist.c
        read_conf.c set_ambient_cap.c compute_digest.c file_utils.c
        scado_parse.c cado_scado_check.c
)
target_include_directories(cado PRIVATE ${PROJECT_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include)
target_link_libraries(
        cado ${pam_library} ${pam_misc_library} ${cap_library} ${mhash_library}
        ${execs_library})

add_executable(cadrop cadrop.c capset_from_namelist.c set_ambient_cap.c)
target_include_directories(cadrop PRIVATE ${PROJECT_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include)
target_link_libraries(cadrop ${cap_library})

add_executable(
        scado scado.c pam_check.c file_utils.c compute_digest.c
        capset_from_namelist.c scado_parse.c
)
target_include_directories(scado PRIVATE ${PROJECT_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include)
target_link_libraries(
        scado ${pam_library} ${pam_misc_library} ${cap_library} ${mhash_library}
        ${execs_library}
)

add_executable(caprint caprint.c)
target_include_directories(caprint PRIVATE ${PROJECT_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include)
target_link_libraries(caprint ${cap_library})

install(
        TARGETS cado cadrop scado caprint
        RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(NOT DEBUILD)
        install(
                CODE "execute_process(
                        COMMAND ${CMAKE_COMMAND}
                        -DBINDIR=${CMAKE_INSTALL_FULL_BINDIR}
                        -DLIBDIR=${CMAKE_INSTALL_FULL_LIBDIR}
                        -P ${PROJECT_SOURCE_DIR}/PostInstall.cmake
                )"
        )
endif()
