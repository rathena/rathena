ALTER TABLE `mob_db2` Add column `Drop9id` mediumint(9) NOT NULL default '0';
ALTER TABLE `mob_db2` Add column `Drop9per` mediumint(9) NOT NULL default '0';
ALTER TABLE `mob_db2` Add column `DropCardid` mediumint(9) NOT NULL default '0';
ALTER TABLE `mob_db2` Add column `DropCardper` mediumint(9) NOT NULL default '0';
ALTER TABLE `mob_db2` Drop column `Item1`;
ALTER TABLE `mob_db2` Drop column `Item2`;
ALTER TABLE `mob_db2` Change column `Drop7d` `Drop7id` mediumint(9) NOT NULL default '0';

ALTER TABLE `mob_db` Add column `Drop9id` mediumint(9) NOT NULL default '0';
ALTER TABLE `mob_db` Add column `Drop9per` mediumint(9) NOT NULL default '0';
ALTER TABLE `mob_db` Add column `DropCardid` mediumint(9) NOT NULL default '0';
ALTER TABLE `mob_db` Add column `DropCardper` mediumint(9) NOT NULL default '0';
