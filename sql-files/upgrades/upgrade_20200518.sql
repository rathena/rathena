-- WM_DOMINION_IMPULSE
UPDATE `char` c, `skill` s SET `c`.skill_point = `c`.skill_point + `s`.lv WHERE `s`.id = 2417 AND `c`.char_id = `s`.char_id;
DELETE FROM `skill` WHERE `id` = 2417;
