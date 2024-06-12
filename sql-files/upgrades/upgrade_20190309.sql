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

INSERT INTO `inventory`( `char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3` )
SELECT
	`p`.`char_id`,						-- Character ID
	`p`.`egg_id`,						-- Egg Item ID
	'1',								-- Amount
	'0',								-- Equip
	'1',								-- Identify
	'0',								-- Refine
	'1',								-- Attribute
	'256',								-- Card0
	( `p`.`pet_id` & 0xFFFF ),			-- Card1
	( ( `p`.`pet_id` >> 16 ) & 0xFFFF ),	-- Card2
	'0'									-- Card3
FROM `pet` `p`
LEFT JOIN `inventory` `i`
ON
	`i`.`char_id` = `p`.`char_id`
AND
	`i`.`nameid` = `p`.`egg_id`
AND
	`i`.`card0` = '256'
AND
	( `i`.`card1` | ( `i`.`card2` << 16 ) ) = `p`.`pet_id`
WHERE
	`p`.`incubate` = '0'
AND
	`i`.`id` IS NULL
;
