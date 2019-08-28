ALTER TABLE `atcommandlog` ALTER `atcommand_date` DROP DEFAULT;
ALTER TABLE `branchlog` ALTER `branch_date` DROP DEFAULT;
ALTER TABLE `cashlog` ALTER `time` DROP DEFAULT;
ALTER TABLE `chatlog` ALTER `time` DROP DEFAULT;
ALTER TABLE `feedinglog` ALTER `time` DROP DEFAULT;
ALTER TABLE `loginlog` ALTER `time` DROP DEFAULT;
ALTER TABLE `mvplog` ALTER `mvp_date` DROP DEFAULT;
ALTER TABLE `npclog` ALTER `npc_date` DROP DEFAULT;
ALTER TABLE `picklog` ALTER `time` DROP DEFAULT;
ALTER TABLE `zenylog` ALTER `time` DROP DEFAULT;

-- Optional: delete useless entries
-- DELETE FROM `atcommandlog` WHERE `atcommand_date` = '0000-00-00 00:00:00';
-- DELETE FROM `branchlog` WHERE `branch_date` = '0000-00-00 00:00:00';
-- DELETE FROM `cashlog` WHERE `time` = '0000-00-00 00:00:00';
-- DELETE FROM `chatlog` WHERE `time` = '0000-00-00 00:00:00';
-- DELETE FROM `feedinglog` WHERE `time` = '0000-00-00 00:00:00';
-- DELETE FROM `loginlog` WHERE `time` = '0000-00-00 00:00:00';
-- DELETE FROM `mvplog` WHERE `mvp_date` = '0000-00-00 00:00:00';
-- DELETE FROM `npclog` WHERE `npc_date` = '0000-00-00 00:00:00';
-- DELETE FROM `picklog` WHERE `time` = '0000-00-00 00:00:00';
-- DELETE FROM `zenylog` WHERE `time` = '0000-00-00 00:00:00';
