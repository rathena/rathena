ALTER TABLE `party` ADD COLUMN `leader_char` int(11) NOT NULL DEFAULT '0' AFTER `leader_id`;
UPDATE `party`,`char` SET `leader_char` = `char`.char_id WHERE (`char`.party_id = `party`.party_id AND `party`.leader_id = `char`.account_id);
