ALTER TABLE `merchant_configs`
	DROP PRIMARY KEY,
	ADD PRIMARY KEY (`world_name`, `account_id`, `char_id`, `store_type`)
;
