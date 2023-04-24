
add_custom_target(scripts)

function(copy_to_source_dir FILENAMES)
    foreach(FILE ${FILENAMES})
        add_custom_command(
            TARGET scripts POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/${FILE}
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${FILE})
    endforeach()
endfunction()

set(FILES_TO_COPY
"serv.bat"
"logserv.bat"
"charserv.bat"
"mapserv.bat"
    "runserver.bat"

)