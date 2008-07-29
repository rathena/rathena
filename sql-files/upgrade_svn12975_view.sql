-- The statements below will create a 'view' (virtual table) that mimics
-- the previous `login` table layout. You can use this hack to make your db
-- compatible with older eathena servers or control panels.
-- Note: also adjust the account.sql.account_db setting in login_athena.conf.
-- Note: if your CP does not have a config setting for the `login` table name
--       you'll have to either modify its code or do some table renaming.

START TRANSACTION;

-- create dummy columns, needed to make the view insertable
ALTER TABLE `login` ADD `error_message` SMALLINT UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `login` ADD `memo` SMALLINT UNSIGNED NOT NULL DEFAULT '0';

-- create the view
CREATE VIEW `login_view` ( `account_id`, `userid`, `user_pass`, `lastlogin`, `sex`, `logincount`, `email`, `level`, `error_message`, `connect_until`, `last_ip`, `memo`, `ban_until`, `state` ) AS SELECT `account_id`, `userid`, `user_pass`, `lastlogin`, `sex`, `logincount`, `email`, `level`, `error_message`, `expiration_time`, `last_ip`, `memo`, `unban_time`, `state` FROM `login`;

COMMIT;
