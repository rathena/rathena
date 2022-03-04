c4_setup_benchmarking()

# thirdparty libs that will be compared with ryml

set(_ed ${CMAKE_CURRENT_BINARY_DIR}/subprojects) # casual ryml extern dir (these projects are not part of ryml and are downloaded and compiled on the fly)

# libyaml
c4_require_subproject(libyaml REMOTE
    GIT_REPOSITORY https://github.com/yaml/libyaml
    GIT_TAG master
  OVERRIDE BUILD_TESTING OFF
  SET_FOLDER_TARGETS ext yaml)

# libfyaml - not in windows
if(NOT WIN32)
    c4_require_subproject(libfyaml REMOTE
        GIT_REPOSITORY https://github.com/pantoniou/libfyaml
        GIT_TAG fd35c3a5fa58ff2effd8207f2185852c3a837d76 #v0.7.11
        SET_FOLDER_TARGETS ext fyaml
        )
endif()

# mini-yaml
# too incomplete ATM
set(miniyaml_dir ${_ed}/miniyaml)
c4_download_remote_proj(miniyaml miniyaml_dir
    GIT_REPOSITORY https://github.com/jimmiebergmann/mini-yaml
    GIT_TAG master)
add_library(miniyaml
    ${miniyaml_dir}/yaml/Yaml.hpp
    ${miniyaml_dir}/yaml/Yaml.cpp)
target_include_directories(miniyaml PUBLIC
    $<BUILD_INTERFACE:${miniyaml_dir}>
    $<BUILD_INTERFACE:${miniyaml_dir}/yaml>
    $<INSTALL_INTERFACE:include>)

# yaml-cpp
c4_import_remote_proj(yaml-cpp ${_ed}/yaml-cpp REMOTE
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp
    # the master branch regularly fails on windows builds.
    # so use fixed pre-validated commit hashes
    GIT_TAG 587b24e2eedea1afa21d79419008ca5f7bda3bf4
  OVERRIDE
      YAML_CPP_BUILD_TESTS OFF
      YAML_CPP_BUILD_TOOLS OFF
      YAML_CPP_BUILD_CONTRIB OFF
      YAML_CPP_BUILD_INSTALL OFF
  SET_FOLDER_TARGETS ext yaml-cpp format)
set(ryml_yaml_cpp_inc ${_ed}/yaml-cpp/src/include)
if(MSVC)
    target_compile_definitions(yaml-cpp PUBLIC -D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
endif()

# jsoncpp needs to be compiled
c4_require_subproject(jsoncpp REMOTE
    GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp
    GIT_TAG 65bb1b1c1d8019dc72279c12bb74df92925dfd5e
    OVERRIDE
        JSONCPP_WITH_TESTS OFF
        JSONCPP_WITH_POST_BUILD_UNITTEST OFF
        JSONCPP_WITH_WARNING_AS_ERROR OFF
        JSONCPP_WITH_STRICT_ISO OFF
        JSONCPP_WITH_PKGCONFIG_SUPPORT OFF
        JSONCPP_WITH_CMAKE_PACKAGE OFF
	)
c4_set_folder_remote_project_targets(ext/jsoncpp
    jsoncpp
    jsoncpp_object
    jsoncpp_static
    examples
    readFromStream
    readFromString
    streamWrite
    stringWrite)

# nlohmannjson needs to be compiled
c4_require_subproject(nlohmann_json REMOTE
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG master
    OVERRIDE
        JSON_BuildTests OFF
        JSON_Install OFF
        JSON_MultipleHeaders OFF
    )

# rapidjson is header only
set(rapidjson_dir ${_ed}/rapidjson)
c4_download_remote_proj(rapidjson rapidjson_dir
    GIT_REPOSITORY https://github.com/Tencent/rapidjson
    GIT_TAG version1.1.0)
set(RYML_RAPIDJSON_INC_DIR ${rapidjson_dir}/include)

# sajson is header only
set(sajson_dir ${_ed}/sajson)
c4_download_remote_proj(sajson sajson_dir
    GIT_REPOSITORY https://github.com/chadaustin/sajson
    GIT_TAG 2dcfd350586375f9910f74821d4f07d67ae455ba)
set(RYML_SAJSON_INC_DIR ${sajson_dir}/include)


# -----------------------------------------------------------------------------

function(ryml_add_bm_exe name)
    c4_add_executable(ryml-bm-${name}
        SOURCES bm_common.hpp ${ARGN}
        LIBS ryml yaml yaml-cpp benchmark jsoncpp_static nlohmann_json c4fs
        INC_DIRS ${RYML_RAPIDJSON_INC_DIR} ${RYML_SAJSON_INC_DIR}
        FOLDER bm)
    if(RYML_DBG)
        target_compile_definitions(ryml-bm-${name} PRIVATE RYML_DBG)
    endif()
    if(NOT WIN32)
        target_compile_definitions(ryml-bm-${name} PRIVATE RYML_HAVE_LIBFYAML)
        target_link_libraries(ryml-bm-${name} PRIVATE fyaml)
    endif()
    add_custom_target(ryml-bm-${name}-all) # for aggregating all the cases
    _c4_set_target_folder(ryml-bm-${name}-all bm/run)
endfunction()

ryml_add_bm_exe(parse bm_parse.cpp)
ryml_add_bm_exe(emit bm_emit.cpp)

function(ryml_add_bm_case target name case_file)
    c4_dbg("adding benchmark case: ${case_file}")
    get_filename_component(case "${case_file}" NAME_WE) # case identifier
    get_filename_component(ext "${case_file}" EXT) # prevent json readers from reading yml data
    if(NOT ("${ext}" STREQUAL ".json"))
        set(filter_json "yml|yaml")
    endif()
    c4_add_target_benchmark(${target} ${case}
        FILTER "${filter_json}"
        ARGS ${case_file})
    add_dependencies(ryml-bm-${name}-all ryml-bm-${name}-${case})
endfunction()

set(RYML_BM_CASE_MANIFESTS "" CACHE STRING "additional benchmark case manifests")
foreach(mnf ${RYML_BM_CASE_MANIFESTS} "${CMAKE_CURRENT_LIST_DIR}/cases/bm-cases.txt")
    if(NOT EXISTS "${mnf}")
        c4_err("benchmark manifest file not found: ${mnf}")
    endif()
    get_filename_component(mnf_dir "${mnf}" DIRECTORY)
    file(STRINGS "${mnf}" bm_cases)
    foreach(case_file ${bm_cases})
        if(NOT EXISTS "${mnf_dir}/${case_file}")
            c4_err("benchmark case file not found: ${mnf_dir}/${case_file}")
        endif()
        ryml_add_bm_case(ryml-bm-parse parse "${mnf_dir}/${case_file}")
        ryml_add_bm_case(ryml-bm-emit emit "${mnf_dir}/${case_file}")
    endforeach()
endforeach()
