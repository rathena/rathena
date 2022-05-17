ALTER TABLE `bonus_script`
    DROP PRIMARY KEY,
    ADD KEY `char_id` (`char_id`);
