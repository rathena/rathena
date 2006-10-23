ALTER TABLE `picklog` ADD INDEX (`type`);
ALTER TABLE `zenylog` ADD INDEX (`type`);
ALTER TABLE `branchlog` ADD INDEX (`account_id`), ADD INDEX (`char_id`);
ALTER TABLE `atcommandlog` ADD INDEX (`account_id`), ADD INDEX (`char_id`);
ALTER TABLE `npclog` ADD INDEX (`account_id`), ADD INDEX (`char_id`);
ALTER TABLE `chatlog` ADD INDEX (`src_accountid`), ADD INDEX (`src_charid`);
