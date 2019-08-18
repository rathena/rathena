ALTER TABLE `bonus_script`
	ADD COLUMN `id` INT NOT NULL AUTO_INCREMENT FIRST,
	ADD PRIMARY KEY (`id`);
	ADD INDEX `char_id` (`char_id`);

ALTER TABLE `buyingstore_items`
	ADD PRIMARY KEY (`buyingstore_id`, `index`);

ALTER TABLE `charlog`
	ADD PRIMARY KEY (`time`, `account_id`, `char_num`);

ALTER TABLE `friends`
	ADD PRIMARY KEY (`char_id`, `friend_account`, `friend_id`);

ALTER TABLE `interlog`
	ADD COLUMN `id` INT NOT NULL AUTO_INCREMENT FIRST,
	ADD PRIMARY KEY (`id`),
	ADD INDEX `time` (`time`);

ALTER TABLE `ipbanlist`
	ADD PRIMARY KEY (`list`, `btime`);

ALTER TABLE `sc_data`
	ADD PRIMARY KEY (`account_id`, `char_id`, `type`);

ALTER TABLE `skillcooldown`
	ADD PRIMARY KEY (`account_id`, `char_id`, `skill`);

ALTER TABLE `vending_items`
	ADD PRIMARY KEY (`vending_id`, `index`);
