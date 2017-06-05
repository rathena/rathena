-- Resetting all `lv` of skills that has `flag` >= 3 (the skill that its `learned_lv` has been changed by script or special case by `learned_lv` + SKILL_FLAG_REPLACED_LV_0)
-- If there's ALL_INCCARRY and ALL_BUYING_STORE, set the `flag` to SKILL_FLAG_PERM_GRANTED (new value is 3), those are exclusive skills given in our official scripts!

UPDATE `skill` SET `lv` = `flag` - 3 WHERE `flag` >= 3;
UPDATE `skill` SET `flag` = 0 WHERE `flag` >= 3;
UPDATE `skill` SET `flag` = 3 WHERE `id` = 681 OR `id` = 2535;
