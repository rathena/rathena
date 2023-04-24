function(copy_import_files SRC_DIR DST_DIR FILE_LIST)

    file(MAKE_DIRECTORY ${DST_DIR})

    file(GLOB FILES_IMPORTED
        LIST_DIRECTORIES false
        RELATIVE ${DST_DIR}/ ${DST_DIR}/*)
    
    
    # message("files to import are: " "${FILE_LIST}")
    # message("Already made fs are: " "${FILES_IMPORTED}")

    list(REMOVE_ITEM FILE_LIST ${FILES_IMPORTED})

    # message("  rest of files are: " ${FILE_LIST})
    
    foreach(FILE ${FILE_LIST})
        message("Importing ${FILE}")
        file(COPY ${SRC_DIR}/${FILE} DESTINATION ${DST_DIR})
    endforeach()
endfunction()
