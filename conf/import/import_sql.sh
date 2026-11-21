#!/bin/bash

shopt -s nullglob
files=(./sql-files/*.sql)

IFS=$'\n' files=($(printf '%s\n' "${files[@]}" | sort))
unset IFS

if [ ${#files[@]} -eq 0 ]; then
    echo "No SQL files found."
    exit 1
fi

read -sp "MariaDB password: " DB_PASSWORD
echo

for file in "${files[@]}"; do
    echo "Importing: $file"

    mariadb -u rathena -p"$DB_PASSWORD" rathena < "$file"

    if [ $? -ne 0 ]; then
        echo "Import failed: $file"
        unset DB_PASSWORD
        exit 1
    fi
done

unset DB_PASSWORD

echo "All SQL files imported successfully."