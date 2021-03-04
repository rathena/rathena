-- HT_SANDMAN
UPDATE `char` c, `skill` s SET `c`.skill_point = `c`.skill_point + `s`.lv WHERE (`c`.class = 4190 OR `c`.class = 4191) AND `s`.id = 119 AND `c`.char_id = `s`.char_id;
DELETE FROM `skill` USING `skill`, `char` WHERE (`char`.class = 4190 OR `char`.class = 4191) AND `skill`.id = 119 AND `char`.char_id = `skill`.char_id;

-- HT_FLASHER
UPDATE `char` c, `skill` s SET `c`.skill_point = `c`.skill_point + `s`.lv WHERE (`c`.class = 4190 OR `c`.class = 4191) AND `s`.id = 120 AND `c`.char_id = `s`.char_id;
DELETE FROM `skill` USING `skill`, `char` WHERE (`char`.class = 4190 OR `char`.class = 4191) AND `skill`.id = 120 AND `char`.char_id = `skill`.char_id;

-- HT_FREEZINGTRAP
UPDATE `char` c, `skill` s SET `c`.skill_point = `c`.skill_point + `s`.lv WHERE (`c`.class = 4190 OR `c`.class = 4191) AND `s`.id = 121 AND `c`.char_id = `s`.char_id;
DELETE FROM `skill` USING `skill`, `char` WHERE (`char`.class = 4190 OR `char`.class = 4191) AND `skill`.id = 121 AND `char`.char_id = `skill`.char_id;
