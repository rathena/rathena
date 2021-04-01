# SQL Imports

## Explanation of SQL import files

The files in this directory are basic SQL table building scripts. The contained scripts are needed for initial installs as well as subsequent updates.

### New Install
---
For a new install, the following needs to be imported into the 'ragnarok' schema:
Note: The schema name is defined in `conf/inter_athena.conf::map_server_db`.

* main.sql - Contains tables for normal server usage.
* roulette_default_data.sql - Contains data for the client's roulette game.

For a new install, the following needs to be imported into the 'log' schema:
Note: The schema name is defined in `conf/inter_athena.conf::log_db_db`.

* logs.sql - Contains tables for logging of server events.

If your server is setup to read SQL database data, import the following:
Note: If `conf/inter_athena.conf::use_sql_db` is set to yes continue with these imports else these can be skipped. Not all files have to be imported, only the ones that apply to the same mode as the server being ran.

* item_cash_db.sql - Used for client's cash shop.
* item_cash_db2.sql - Used for client's cash shop (import).
* item_db.sql - Contains __pre-renewal__ item data table structure.
* item_db_equip.sql - Contains __pre-renewal__ equipment item data.
* item_db_etc.sql - Contains __pre-renewal__ etcetera item data.
* item_db2.sql - Contains __pre-renewal__ item data (import).
* item_db_re.sql - Contains __renewal__ item data table structure.
* item_db_re_equip.sql - Contains __renewal__ equipment item data.
* item_db_re_etc.sql - Contains __renewal__ etcetera item data.
* item_db_re_usable.sql - Contains __renewal__ usable item data.
* item_db_usable.sql - Contains __pre-renewal__ usable item data.
* item_db2_re.sql - Contains __renewal__ item data (import).
* mob_db.sql - Contains __pre-renewal__ mob data.
* mob_db2.sql - Contains __pre-renewal__ mob data (import).
* mob_db_re.sql - Contains __renewal__ mob data.
* mob_db2_re.sql - Contains __renewal__ mob data (import).
* mob_skill_db.sql - Contains __pre-renewal__ mob skill data.
* mob_skill_db2.sql - Contains __pre-renewal__ mob skill data (import).
* mob_skill_db_re.sql - Contains __renewal__ mob skill data.
* mob_skill_db2_re.sql - Contains __renewal__ mob skill data (import).

### Updates
---
Over the course of time new features and optimizations will take place. This may require SQL changes to happen. In the `upgrades` folder will be upgrade files with an attached date.
These imports only have to executed if an update has occurred after the initial installation. Many times a SQL error will be displayed on the server console stating the format differs from what is required.

### Compatibility
---
The `compatibility` folder contains SQL views which are used with helping control panels or websites grab prevalent data for a table that may have changed structure.

These are optional imports but website data such as item databases will not work properly when using the new YAML format without these views:

* item_db_compat.sql - Creates a view for the item_db.
* item_db2_compat.sql - Creates a view for the item_db2 (import).
* item_db_re_compat.sql - Creates a view for the item_db_re.
* item_db2_re_compat.sql - Creates a view for the item_db2_re (import).

### Tools
---
The `tools` folder contains some simple adjustments if needed for an administrator's personal preferences.

* convert_engine_innodb.sql - Converts the SQL table engine setting to InnoDB.
* convert_engine_myiasm.sql - Converts the SQL table engine setting to MyISAM.
* convert_passwords.sql - Converts the login table's password value to MD5.

Useful tools for converting custom SQL items to TXT and then YAML. Please adjust the `INTO OUTFILE` in the query to a desired location.
To run these queries the user requires the [FILE](https://dev.mysql.com/doc/refman/8.0/en/privileges-provided.html#priv_file) permission. It's also required to either [set or disable](https://computingforgeeks.com/how-to-solve-mysql-server-is-running-with-the-secure-file-priv-error/) the `secure-file-priv`. 

* item_db_re_to_txt.sql - Dumps the __renewal__ item data table to TXT format.
* item_db_to_txt.sql - Dumps the __pre-renewal__ item data table to TXT format.
* item_db2_re_to_txt.sql - Dumps the __renewal__ item data table (import) to TXT format.
* item_db2_to_txt.sql - Dumps the __pre-renewal__ item data table (import) to TXT format.
