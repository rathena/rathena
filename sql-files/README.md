# SQL Imports

## Explanation of SQL import files

The files in this directory are basic SQL table building scripts. The contained scripts are needed for initial installs as well as subsequent updates.

### New Install
---
For a new install, the following needs to be imported into the main schema:
Note: The schema name is defined in `conf/inter_athena.conf::map_server_db`.

* main.sql - Contains tables for normal server usage.
* web.sql - Contains tables for the web service
* roulette_default_data.sql - Contains data for the client's roulette game.

For a new install, the following can be imported into the main schema but is highly suggested to be imported into a separate schema for logs:
Note: The schema name is defined in `conf/inter_athena.conf::log_db_db`.

* logs.sql - Contains tables for logging of server events.

Follow the next steps only if `conf/inter_athena.conf::use_sql_db` is set to yes, otherwise these can be skipped.

1. Make sure to compile the yaml2sql tool in the desired mode pre-renewal or renewal.
2. Then run the yaml2sql tool and convert all required YML files to SQL.<br/>
   The files will be converted into the /sql-files/ directory.<br/>
   Note: The files are not shipped by default anymore and have to be converted initially.
3. Convert the mob skill database with /tools/convert_sql.pl
4. After the conversion has finished the following tables have to be imported into your main schema.<br/>
   Note: Not all files have to be imported, only the ones that apply to the same mode as the server being ran.

* __Pre-Renewal:__
  * item_db.sql - Contains item data table structure.
  * item_db_equip.sql - Contains equipment item data.
  * item_db_etc.sql - Contains etcetera item data.
  * item_db2.sql - Contains item data (import).
  * item_db_usable.sql - Contains usable item data.
  * mob_db.sql - Contains mob data.
  * mob_db2.sql - Contains mob data (import).
  * mob_skill_db.sql - Contains mob skill data.
  * mob_skill_db2.sql - Contains mob skill data (import).

* __Renewal:__
  * item_db_re.sql - Contains item data table structure.
  * item_db_re_equip.sql - Contains equipment item data.
  * item_db_re_etc.sql - Contains etcetera item data.
  * item_db_re_usable.sql - Contains usable item data.
  * item_db2_re.sql - Contains item data (import).
  * mob_db_re.sql - Contains mob data.
  * mob_db2_re.sql - Contains mob data (import).
  * mob_skill_db_re.sql - Contains mob skill data.
  * mob_skill_db2_re.sql - Contains mob skill data (import).

### Updates
---
Over the course of time new features and optimizations will take place. This may require SQL changes to happen. In the `upgrades` folder will be SQL files with an attached date.
These only have to executed one time if an update has occurred after the initial installation. It's possible to see when an update may be required when a SQL error will be displayed on the server console stating the format differs from what is required.

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

### Notes
---
The `web-server` must be able to read the `login` and `guild` tables from the `login-server` and `char-server`, respectively.
