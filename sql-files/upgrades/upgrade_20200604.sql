UPDATE `char` `c`
INNER JOIN `login` `l`
ON `l`.`account_id` = `c`.`account_id`
SET `c`.`sex` = `l`.`sex`
WHERE
	`c`.`sex` = 'U'
AND
	`l`.`sex` <> 'S'
;

ALTER TABLE `char`
	MODIFY `sex` ENUM('M','F') NOT NULL
;
