-- Episode 17.2: grant "Who made the king" (130005) to characters that already watched the coronation ceremony
-- Run this while the servers are offline, otherwise an online character's next save drops the new row
UPDATE `achievement` a, `char_reg_num` r
SET a.`completed` = NOW()
WHERE a.`id` = 130005 AND a.`completed` IS NULL
AND r.`char_id` = a.`char_id` AND r.`key` = 'ep17_2_main' AND r.`index` = 0 AND r.`value` >= 35;

INSERT INTO `achievement` (`char_id`, `id`, `completed`)
SELECT r.`char_id`, 130005, NOW() FROM `char_reg_num` r
WHERE r.`key` = 'ep17_2_main' AND r.`index` = 0 AND r.`value` >= 35
AND NOT EXISTS (SELECT 1 FROM `achievement` a WHERE a.`char_id` = r.`char_id` AND a.`id` = 130005);
