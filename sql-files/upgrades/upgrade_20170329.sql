UPDATE `charlog` SET `time` = '1000-01-01 00:00:00' WHERE `time` = '0000-00-00 00:00:00';
UPDATE `interlog` SET `time` = '1000-01-01 00:00:00' WHERE `time` = '0000-00-00 00:00:00';
UPDATE `ipbanlist` SET `btime` = '1000-01-01 00:00:00' WHERE `btime` = '0000-00-00 00:00:00';
UPDATE `ipbanlist` SET `rtime` = '1000-01-01 00:00:00' WHERE `rtime` = '0000-00-00 00:00:00';
UPDATE `login` SET `lastlogin` = '1000-01-01 00:00:00' WHERE `lastlogin` = '0000-00-00 00:00:00';
UPDATE `login` SET `birthdate` = '1000-01-01' WHERE `lastlogin` = '0000-00-00';