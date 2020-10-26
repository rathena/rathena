@echo off

rem MySQL database setup

rem Use the `MYSQL_PWD` environment variable to avoid insecure warning.
set MYSQL_PWD=%DB_ROOTPW%

%MYSQL% -u %DB_ROOT% -e "CREATE DATABASE %DB_NAME%;"
%MYSQL% -u %DB_ROOT% %DB_NAME% -e "source sql-files\main.sql"
%MYSQL% -u %DB_ROOT% %DB_NAME% -e "source sql-files\logs.sql"
%MYSQL% -u %DB_ROOT% %DB_NAME% -e "source sql-files\item_cash_db.sql"
%MYSQL% -u %DB_ROOT% %DB_NAME% -e "source sql-files\item_cash_db2.sql"
%MYSQL% -u %DB_ROOT% %DB_NAME% -e "source sql-files\item_db.sql"
%MYSQL% -u %DB_ROOT% %DB_NAME% -e "source sql-files\item_db_usable.sql"
%MYSQL% -u %DB_ROOT% %DB_NAME% -e "source sql-files\item_db_equip.sql"
%MYSQL% -u %DB_ROOT% %DB_NAME% -e "source sql-files\item_db_etc.sql"
%MYSQL% -u %DB_ROOT% %DB_NAME% -e "source sql-files\item_db2.sql"
%MYSQL% -u %DB_ROOT% %DB_NAME% -e "source sql-files\item_db_re.sql"
%MYSQL% -u %DB_ROOT% %DB_NAME% -e "source sql-files\item_db_re_usable.sql"
%MYSQL% -u %DB_ROOT% %DB_NAME% -e "source sql-files\item_db_re_equip.sql"
%MYSQL% -u %DB_ROOT% %DB_NAME% -e "source sql-files\item_db_re_etc.sql"
%MYSQL% -u %DB_ROOT% %DB_NAME% -e "source sql-files\item_db2_re.sql"
%MYSQL% -u %DB_ROOT% %DB_NAME% -e "source sql-files\mob_db.sql"
%MYSQL% -u %DB_ROOT% %DB_NAME% -e "source sql-files\mob_db2.sql"
%MYSQL% -u %DB_ROOT% %DB_NAME% -e "source sql-files\mob_db_re.sql"
%MYSQL% -u %DB_ROOT% %DB_NAME% -e "source sql-files\mob_db2_re.sql"
%MYSQL% -u %DB_ROOT% %DB_NAME% -e "source sql-files\mob_skill_db.sql"
%MYSQL% -u %DB_ROOT% %DB_NAME% -e "source sql-files\mob_skill_db2.sql"
%MYSQL% -u %DB_ROOT% %DB_NAME% -e "source sql-files\mob_skill_db_re.sql"
%MYSQL% -u %DB_ROOT% %DB_NAME% -e "source sql-files\mob_skill_db2_re.sql"
%MYSQL% -u %DB_ROOT% %DB_NAME% -e "source sql-files\roulette_default_data.sql"
%MYSQL% -u %DB_ROOT% -e "GRANT SELECT,INSERT,UPDATE,DELETE ON %DB_NAME%.* TO '%DB_USER%'@'%DB_HOST%' IDENTIFIED BY '%DB_USERPW%';"
