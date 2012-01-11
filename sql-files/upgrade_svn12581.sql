ALTER TABLE `quest_objective` ADD COLUMN `char_id` INTEGER UNSIGNED NOT NULL AFTER `num`,
  DROP PRIMARY KEY,
  ADD PRIMARY KEY  USING BTREE(`quest_id`, `num`, `char_id`);
