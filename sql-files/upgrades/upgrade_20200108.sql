ALTER TABLE `charlog`
    DROP PRIMARY KEY, -- comment if primary key has not been created yet
    ADD COLUMN `id` bigint(20) unsigned NOT NULL auto_increment first,
    ADD PRIMARY KEY (`id`),
    ADD KEY `account_id` (`account_id`);
