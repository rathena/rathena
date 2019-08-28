ALTER TABLE `charlog` ALTER `time` DROP DEFAULT;
ALTER TABLE `interlog` ALTER `time` DROP DEFAULT;
ALTER TABLE `ipbanlist` ALTER `btime` DROP DEFAULT;
ALTER TABLE `ipbanlist` ALTER `rtime` DROP DEFAULT;
ALTER TABLE `login` ALTER `lastlogin` DROP DEFAULT;
ALTER TABLE `login` ALTER `birthdate` DROP DEFAULT;

ALTER TABLE `login` MODIFY `lastlogin` datetime;
ALTER TABLE `login` MODIFY `birthdate` date;

UPDATE `login` SET `lastlogin` = NULL WHERE `lastlogin` = '0000-00-00 00:00:00';
UPDATE `login` SET `birthdate` = NULL WHERE `birthdate` = '0000-00-00';

-- Optional: delete useless entries
-- DELETE FROM `charlog` WHERE `time` = '0000-00-00 00:00:00';
-- DELETE FROM `interlog` WHERE `time` = '0000-00-00 00:00:00';
-- DELETE FROM `ipbanlist` WHERE `btime` = '0000-00-00 00:00:00';
-- DELETE FROM `ipbanlist` WHERE `rtime` = '0000-00-00 00:00:00';
