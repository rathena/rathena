ALTER TABLE `pet`
	ADD COLUMN `autofeed` tinyint(2) NOT NULL default '0' AFTER `incubate`;

UPDATE `inventory` `i`
INNER JOIN `char` `c`
ON `i`.`char_id` = `c`.`char_id` AND `c`.`pet_id` <> '0'
SET `i`.`attribute` = '1'
WHERE
	`i`.`card0` = '256'
AND
	( `i`.`card1` | ( `i`.`card2` << 16 ) ) = `c`.`pet_id`
;
