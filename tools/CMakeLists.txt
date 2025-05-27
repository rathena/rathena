
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

if(WIN32)
    set(WIN_FILES_TO_COPY
        "charserv.bat"
        "csv2yaml.bat"
        "logserv.bat"
        "mapcache.bat"
        "mapserv.bat"
        "runserver.bat"
        "serv.bat"
        "webserv.bat"
        "yaml2sql.bat"
        "yamlupgrade.bat"
        "navigenerator.bat"
    )
    copy_to_source_dir("${WIN_FILES_TO_COPY}")
else()
    set(LINUX_FILES_TO_COPY
        "athena-start.sh"
        "function.sh"
        "install.sh"
        "uninstall.sh"
    )

    copy_to_source_dir("${LINUX_FILES_TO_COPY}")
endif()
