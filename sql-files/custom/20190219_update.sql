# remove special mission
DELETE FROM `player_mission` WHERE `mission_id` IN (1548750155, 1548750287, 1548750314, 1548750365, 1548750433);
DELETE FROM `mission_board` WHERE `npc_id` LIKE '|99|';

# set Golden Thread to be repeatable
UPDATE `pakkun`.`mission_board` SET `redo_delay`='0' WHERE  `id`=1548750837;

# Alter Guild Invite table.
ALTER TABLE `guild_invite_members`  ADD `request` TINYINT(1) NOT NULL DEFAULT '0'  AFTER `date_registered`,  ADD `date_requested` DATETIME NOT NULL DEFAULT '0000-00-00 00:00:00'  AFTER `request`;

# clear broken data in multi_pc table
TRUNCATE TABLE `multi_pc`;