UPDATE `picklog` SET `card0` = 256 WHERE `card0` = -256;

UPDATE `picklog` SET `card1` = (65536 + `card1`) WHERE `card1` < 0 AND `card0` IN(254,255);
UPDATE `picklog` SET `card2` = (65536 + `card2`) WHERE `card2` < 0 AND `card0` IN(254,255);
UPDATE `picklog` SET `card3` = (65536 + `card3`) WHERE `card3` < 0 AND `card0` IN(254,255);

ALTER TABLE `picklog` MODIFY `nameid` smallint(5) unsigned NOT NULL default '0';
ALTER TABLE `picklog` MODIFY `card0` smallint(5) unsigned NOT NULL default '0';
ALTER TABLE `picklog` MODIFY `card1` smallint(5) unsigned NOT NULL default '0';
ALTER TABLE `picklog` MODIFY `card2` smallint(5) unsigned NOT NULL default '0';
ALTER TABLE `picklog` MODIFY `card3` smallint(5) unsigned NOT NULL default '0';
ALTER TABLE `mvplog` MODIFY `prize` smallint(5) unsigned NOT NULL default '0';
