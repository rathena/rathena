#!/bin/sh -e

IMPORT_PATH="/rathena/conf/import"
for config_file in $(ls ${IMPORT_PATH}/*_conf.txt); do
    >&2 echo "Generating ${config_file}"
    config_file_name=$(basename $config_file)
    config_prefix=${config_file_name%_conf.txt}
    upper_config_prefix=$(echo $config_prefix | tr 'a-z' 'A-Z')
    env_prefix="RATHENA_${upper_config_prefix}_"
    for var in $(env | grep "^${env_prefix}"); do
        >&2 echo "Loading $var"
        clean_var=${var#$env_prefix}
        var_name=$(echo $clean_var | cut -d'=' -f1 | tr 'A-Z' 'a-z')
        var_value=$(echo $clean_var | cut -d'=' -f2)
        if $(grep -q "^${var_name}: " $config_file); then
            >&2 echo "${var_name} already in ${config_file}"
        else
            config_line="${var_name}: ${var_value}"
            >&2 echo "Adding '$config_line' to ${config_file}"
            echo $config_line >> $config_file
        fi
    done
done
