set(MANIFEST "${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt")

if(NOT EXISTS ${MANIFEST})
    message(FATAL_ERROR "Cannot find install manifest: '${MANIFEST}'")
endif()

file(STRINGS ${MANIFEST} files)
foreach(file ${files})
    if(EXISTS ${file})
        message(STATUS "Removing file: '${file}'")

        execute_process(
            COMMAND bash "-c" "rm ${file}"
            OUTPUT_VARIABLE remove_file
        )

        if(${remove_file})
            message(FATAL_ERROR "Failed to remove file: '${file}'.")
        endif()
    else()
        MESSAGE(STATUS "File '${file}' does not exist.")
    endif()
endforeach(file)

message(STATUS "Removing user: '_cado'")
execute_process(
    COMMAND bash "-c" "userdel _cado"
    OUTPUT_VARIABLE remove_user
)
if(${remove_user})
    message(FATAL_ERROR "Failed to remove user: '_cado'.")
endif()

message(STATUS "Removing folder: '/usr/local/var/spool/cado'")
execute_process(
    COMMAND bash "-c" "rm -r /usr/local/var/spool/cado"
    OUTPUT_VARIABLE remove_spool_folder
)
if("${remove_spool_folder}")
    message(FATAL_ERROR "Failed to remove folder: '/usr/local/var/spool/cado'.")
endif()
