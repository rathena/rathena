ALTER TABLE `charlog`
    DROP PRIMARY KEY, -- comment if primary key has not been created yet
    ADD COLUMN `id` bigint unsigned NOT NULL AUTO_INCREMENT first,
    ADD PRIMARY KEY (`id`),
    ADD KEY `account_id` (`account_id`);
