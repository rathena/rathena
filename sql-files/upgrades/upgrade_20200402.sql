-- AB_EUCHARISTICA
UPDATE `char` c, `skill` s SET `c`.skill_point = `c`.skill_point + `s`.lv WHERE `s`.id = 2049 AND `c`.char_id = `s`.char_id;
DELETE FROM `skill` WHERE `id` = 2049;

-- GN_SLINGITEM
UPDATE `char` c, `skill` s SET `c`.skill_point = `c`.skill_point + `s`.lv WHERE `s`.id = 2493 AND `c`.char_id = `s`.char_id;
DELETE FROM `skill` WHERE `id` = 2493;

-- GN_MAKEBOMB
UPDATE `char` c, `skill` s SET `c`.skill_point = `c`.skill_point + `s`.lv WHERE `s`.id = 2496 AND `c`.char_id = `s`.char_id;
DELETE FROM `skill` WHERE `id` = 2496;

-- ONLY RUN THE BELOW QUERIES IF YOU ARE ON RENEWAL
-- CR_CULTIVATION
UPDATE `char` c, `skill` s SET `c`.skill_point = `c`.skill_point + `s`.lv WHERE `s`.id = 491 AND `c`.char_id = `s`.char_id;
DELETE FROM `skill` WHERE `id` = 491;
