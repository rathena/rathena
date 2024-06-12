# rAthena Tools

As with any product, please make a backup of your database directory before continuing with any of these tools listed below.

## CSV2YAML

This tool will convert older generation TXT (or CSV, comma separated values) databases into the new YAML format. Not every database has been converted to YAML, but it's a process rAthena is taking to make efforts easier for future development.

The rAthena devs have taken steps to produce this tool which can be ran on any environment that supports rAthena, without any extra software!

Note, if you have modified a database with custom fields then the CSV2YAML will need to be modified to support those extra fields before converting.

Once ran, you will be prompted for each database you'd like to convert to YAML, allow any custom features in your collection to be translated over. Please add the `#define CONVERT_ALL` macro in `src/custom/define_pre.hpp` or uncomment `CONVERT_ALL` in `src/tools/yaml.hpp` before building if you wish to remove the prompts.

## Mapcache

The mapcache tool will allow you to generate or update the map_cache.dat that is located in `db/`. Simply add the GRF or Data directories that contain the `.gat` and `.rsw` files to the `conf/grf-files.txt` before running.

## YAML2SQL

This tool will convert the Item and Monster databases from YAML to SQL. This still gives the ability for servers that wish to utilize these databases to continue down that path.

Once ran, you will be prompted for each database you'd like to convert to SQL, allow any custom features in your collection to be translated over. Please add the `#define CONVERT_ALL` macro in `src/custom/define_pre.hpp` or uncomment `CONVERT_ALL` in `src/tools/yaml.hpp` before building if you wish to remove the prompts.

## YAMLUpgrade

There are times when a YAML database may have to go through a restructure. When running your server, you will be prompted with:

> Database version # is not supported anymore. Minimum version is: #

Simply run the YAMLUpgrade tool and when prompted to upgrade said database, let the tool handle the conversion for you!
