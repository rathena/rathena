-- Convert the engine from MyISAM to InnoDB
ALTER TABLE `login` ENGINE = InnoDB;
ALTER TABLE `global_acc_reg_num` ENGINE = InnoDB;
ALTER TABLE `global_acc_reg_str` ENGINE = InnoDB;
ALTER TABLE `ipbanlist` ENGINE = InnoDB;

-- Clean up old undeleted global account variables, where the account is lost
DELETE FROM `global_acc_reg_num`
WHERE
	`account_id` NOT IN (
		SELECT `account_id` FROM `login`
	)
;

DELETE FROM `global_acc_reg_str`
WHERE
	`account_id` NOT IN (
		SELECT `account_id` FROM `login`
	)
;

-- Add a foreign key between account and global account variables
ALTER TABLE `global_acc_reg_num`
ADD
	FOREIGN KEY (`account_id`)
		REFERENCES `login`(`account_id`)
		ON DELETE CASCADE
		ON UPDATE CASCADE
;

ALTER TABLE `global_acc_reg_str`
ADD
	FOREIGN KEY (`account_id`)
		REFERENCES `login`(`account_id`)
		ON DELETE CASCADE
		ON UPDATE CASCADE
;
