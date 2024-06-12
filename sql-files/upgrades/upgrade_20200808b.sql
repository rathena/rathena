-- WL_SUMMONFB
UPDATE `char` c, `skill` s SET `c`.skill_point = `c`.skill_point + (`s`.lv - 2), `s`.lv = 2 WHERE `s`.id = 2222 AND `s`.lv > 2 AND `c`.char_id = `s`.char_id;

-- WL_SUMMONBL
UPDATE `char` c, `skill` s SET `c`.skill_point = `c`.skill_point + (`s`.lv - 2), `s`.lv = 2 WHERE `s`.id = 2223 AND `s`.lv > 2 AND `c`.char_id = `s`.char_id;

-- WL_SUMMONWB
UPDATE `char` c, `skill` s SET `c`.skill_point = `c`.skill_point + (`s`.lv - 2), `s`.lv = 2 WHERE `s`.id = 2224 AND `s`.lv > 2 AND `c`.char_id = `s`.char_id;

-- WL_SUMMONSTONE
UPDATE `char` c, `skill` s SET `c`.skill_point = `c`.skill_point + (`s`.lv - 2), `s`.lv = 2 WHERE `s`.id = 2229 AND `s`.lv > 2 AND `c`.char_id = `s`.char_id;
