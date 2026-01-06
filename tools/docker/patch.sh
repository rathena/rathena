#!/bin/sh
set -e

# ----------- Copy import-tmpl folders without overwriting existing files ----------
copy_if_missing() {
  src="$1"
  dst="$2"

  mkdir -p "$dst"

  for file in "$src"/*; do
    name="$(basename "$file")"
    if [ ! -e "$dst/$name" ]; then
      cp -a "$file" "$dst/"
    fi
  done
}

copy_if_missing conf/import-tmpl conf/import
copy_if_missing db/import-tmpl db/import
copy_if_missing conf/msg_conf/import-tmpl conf/msg_conf/import

# ----------- Patch conf files in place ----------
patch_conf() {
  file="$1"
  shift
  while [ "$#" -gt 0 ]; do
    key="$1"
    value="$2"
    if grep -q "^$key:" "$file"; then
      sed -i "s|^$key:.*|$key: $value|" "$file"
    else
      echo "$key: $value" >> "$file"
    fi
    shift 2
  done
}

patch_conf conf/import/char_conf.txt login_ip login char_ip 127.0.0.1
patch_conf conf/import/inter_conf.txt login_server_ip db ipban_db_ip db char_server_ip db map_server_ip db log_db_ip db
patch_conf conf/import/map_conf.txt char_ip char map_ip 127.0.0.1
