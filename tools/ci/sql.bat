@echo off

rem MySQL database setup

%MYSQL% -u %DB_ROOT% -p%DB_ROOTPW% -e "CREATE DATABASE %DB_NAME%;"

%MYSQL% -u %DB_ROOT% -p%DB_ROOTPW% %DB_NAME% -e "source sql-files\main.sql"

%MYSQL% -u %DB_ROOT% -p%DB_ROOTPW% %DB_NAME% -e "source sql-files\logs.sql"

%MYSQL% -u %DB_ROOT% -p%DB_ROOTPW% %DB_NAME% -e "source sql-files\item_cash_db.sql"

%MYSQL% -u %DB_ROOT% -p%DB_ROOTPW% %DB_NAME% -e "source sql-files\item_cash_db2.sql"

%MYSQL% -u %DB_ROOT% -p%DB_ROOTPW% %DB_NAME% -e "source sql-files\item_db.sql"

%MYSQL% -u %DB_ROOT% -p%DB_ROOTPW% %DB_NAME% -e "source sql-files\item_db2.sql"

%MYSQL% -u %DB_ROOT% -p%DB_ROOTPW% %DB_NAME% -e "source sql-files\item_db_re.sql"

%MYSQL% -u %DB_ROOT% -p%DB_ROOTPW% %DB_NAME% -e "source sql-files\item_db2_re.sql"

%MYSQL% -u %DB_ROOT% -p%DB_ROOTPW% %DB_NAME% -e "source sql-files\mob_db.sql"

%MYSQL% -u %DB_ROOT% -p%DB_ROOTPW% %DB_NAME% -e "source sql-files\mob_db2.sql"

%MYSQL% -u %DB_ROOT% -p%DB_ROOTPW% %DB_NAME% -e "source sql-files\mob_db_re.sql"

%MYSQL% -u %DB_ROOT% -p%DB_ROOTPW% %DB_NAME% -e "source sql-files\mob_db2_re.sql"

%MYSQL% -u %DB_ROOT% -p%DB_ROOTPW% %DB_NAME% -e "source sql-files\mob_skill_db.sql"

%MYSQL% -u %DB_ROOT% -p%DB_ROOTPW% %DB_NAME% -e "source sql-files\mob_skill_db2.sql"

%MYSQL% -u %DB_ROOT% -p%DB_ROOTPW% %DB_NAME% -e "source sql-files\mob_skill_db_re.sql"

%MYSQL% -u %DB_ROOT% -p%DB_ROOTPW% %DB_NAME% -e "source sql-files\mob_skill_db2_re.sql"

%MYSQL% -u %DB_ROOT% -p%DB_ROOTPW% %DB_NAME% -e "source sql-files\roulette_default_data.sql"

%MYSQL% -u %DB_ROOT% -p%DB_ROOTPW% -e "GRANT SELECT,INSERT,UPDATE,DELETE ON %DB_NAME%.* TO '%DB_USER%'@'%DB_HOST%' IDENTIFIED BY '%DB_USERPW%';"
