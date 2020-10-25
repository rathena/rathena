#!/usr/bin/env bash

function aborterror {
	echo $@
	exit 1
}

mysql -u $DB_ROOT -e "CREATE DATABASE $DB_NAME;" || aborterror "Unable to create database."
mysql -u $DB_ROOT $DB_NAME < sql-files/main.sql || aborterror "Unable to import main database."
mysql -u $DB_ROOT $DB_NAME < sql-files/logs.sql || aborterror "Unable to import logs database."
mysql -u $DB_ROOT $DB_NAME < sql-files/item_cash_db.sql || aborterror "Unable to import cash item table."
mysql -u $DB_ROOT $DB_NAME < sql-files/item_cash_db2.sql || aborterror "Unable to import cash item 2 table."
mysql -u $DB_ROOT $DB_NAME < sql-files/item_db.sql || aborterror "Unable to import pre-renewal item table structure."
mysql -u $DB_ROOT $DB_NAME < sql-files/item_db_usable.sql || aborterror "Unable to import pre-renewal usable item table."
mysql -u $DB_ROOT $DB_NAME < sql-files/item_db_equip.sql || aborterror "Unable to import pre-renewal equip item table."
mysql -u $DB_ROOT $DB_NAME < sql-files/item_db_etc.sql || aborterror "Unable to import pre-renewal etc item table."
mysql -u $DB_ROOT $DB_NAME < sql-files/item_db2.sql || aborterror "Unable to import pre-renewal item 2 table."
mysql -u $DB_ROOT $DB_NAME < sql-files/item_db_re.sql || aborterror "Unable to import renewal item table structure."
mysql -u $DB_ROOT $DB_NAME < sql-files/item_db_re_usable.sql || aborterror "Unable to import renewal usable item table."
mysql -u $DB_ROOT $DB_NAME < sql-files/item_db_re_equip.sql || aborterror "Unable to import renewal equip item table."
mysql -u $DB_ROOT $DB_NAME < sql-files/item_db_re_etc.sql || aborterror "Unable to import renewal etc item table."
mysql -u $DB_ROOT $DB_NAME < sql-files/item_db2_re.sql || aborterror "Unable to import renewal item 2 table."
mysql -u $DB_ROOT $DB_NAME < sql-files/mob_db.sql || aborterror "Unable to import pre-renewal monster table."
mysql -u $DB_ROOT $DB_NAME < sql-files/mob_db2.sql || aborterror "Unable to import pre-renewal monster 2 table."
mysql -u $DB_ROOT $DB_NAME < sql-files/mob_db_re.sql || aborterror "Unable to import renewal monster table."
mysql -u $DB_ROOT $DB_NAME < sql-files/mob_db2_re.sql || aborterror "Unable to import renewal monster 2 table."
mysql -u $DB_ROOT $DB_NAME < sql-files/mob_skill_db.sql || aborterror "Unable to import pre-renewal monster skill table."
mysql -u $DB_ROOT $DB_NAME < sql-files/mob_skill_db2.sql || aborterror "Unable to import pre-renewal monster skill 2 table."
mysql -u $DB_ROOT $DB_NAME < sql-files/mob_skill_db_re.sql || aborterror "Unable to import renewal monster skill table."
mysql -u $DB_ROOT $DB_NAME < sql-files/mob_skill_db2_re.sql || aborterror "Unable to import renewal monster skill 2 table."
mysql -u $DB_ROOT $DB_NAME < sql-files/roulette_default_data.sql || aborterror "Unable to import roulette table."
mysql -u $DB_ROOT -e "GRANT SELECT,INSERT,UPDATE,DELETE ON $DB_NAME.* TO '$DB_USER'@'$DB_HOST' IDENTIFIED BY '$DB_PASS';"
