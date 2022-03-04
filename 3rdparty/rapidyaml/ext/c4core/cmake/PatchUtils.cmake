# create a script that applies a patch (it's different in windows)

# to generate a patch:
# subversion: svn diff --patch-compatible > path/to/the/patch.diff


function(apply_patch patch where mark)
    if(NOT EXISTS "${mark}")
        if(NOT Patch_EXECUTABLE)
          find_package(Patch REQUIRED)
        endif()
        file(TO_NATIVE_PATH ${patch} patch_native)
        get_filename_component(patch_name "${patch}" NAME)
        message(STATUS "Applying patch: ${patch_name}")
        execute_process(
          COMMAND "${Patch_EXECUTABLE}" "-p0" "--input=${patch_native}"
          WORKING_DIRECTORY "${where}"
          RESULT_VARIABLE status)
        if(NOT status STREQUAL "0")
            message(FATAL_ERROR "could not apply patch: ${patch} ---> ${where}")
        else()
          file(TOUCH "${mark}")
        endif()
    endif()
endfunction()
