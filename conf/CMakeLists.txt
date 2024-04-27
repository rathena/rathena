include(CopyImportFiles)

set(CONF_FILES_TO_IMPORT
    "atcommands.yml"
    "battle_conf.txt"
    "char_conf.txt"
    "groups.yml"
    "inter_conf.txt"
    "inter_server.yml"
    "log_conf.txt"
    "login_conf.txt"
    "map_conf.txt"
    "packet_conf.txt"
    "script_conf.txt"
    "web_conf.txt")

copy_import_files(${CMAKE_CURRENT_SOURCE_DIR}/import-tmpl/
    ${CMAKE_CURRENT_SOURCE_DIR}/import
    "${CONF_FILES_TO_IMPORT}")

set(MSG_FILES_TO_IMPORT
    "map_msg_chn_conf.txt"
    "map_msg_eng_conf.txt"
    "map_msg_frn_conf.txt"
    "map_msg_grm_conf.txt"
    "map_msg_idn_conf.txt"
    "map_msg_mal_conf.txt"
    "map_msg_por_conf.txt"
    "map_msg_rus_conf.txt"
    "map_msg_spn_conf.txt"
    "map_msg_tha_conf.txt")

copy_import_files(${CMAKE_CURRENT_SOURCE_DIR}/msg_conf/import-tmpl/
    ${CMAKE_CURRENT_SOURCE_DIR}/msg_conf/import
    "${MSG_FILES_TO_IMPORT}")
