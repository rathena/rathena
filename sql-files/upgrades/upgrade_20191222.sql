ALTER TABLE `bonus_script`
    ADD PRIMARY KEY (`char_id`, `type`);

ALTER TABLE `buyingstore_items`
    ADD PRIMARY KEY (`buyingstore_id`, `index`);

ALTER TABLE `friends`
    DROP INDEX `char_id`,
    ADD PRIMARY KEY (`char_id`, `friend_id`);

ALTER TABLE `interlog`
    ADD COLUMN `id` INT NOT NULL AUTO_INCREMENT FIRST,
    ADD PRIMARY KEY (`id`),
    ADD INDEX `time` (`time`);

ALTER TABLE `ipbanlist`
    DROP INDEX `list`,
    ADD PRIMARY KEY (`list`, `btime`);

ALTER TABLE `sc_data`
    DROP INDEX `account_id`,
    DROP INDEX `char_id`,
    ADD PRIMARY KEY (`char_id`, `type`);

ALTER TABLE `skillcooldown`
    DROP INDEX `account_id`,
    DROP INDEX `char_id`,
    ADD PRIMARY KEY (`char_id`, `skill`);

ALTER TABLE `vending_items`
    ADD PRIMARY KEY (`vending_id`, `index`);

DROP TABLE `sstatus`;
