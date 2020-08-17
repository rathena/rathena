DELETE FROM `skill` s WHERE EXISTS (SELECT * FROM `char` c WHERE `c`.char_id == `s`.char_id AND (`c`.class == 4218 OR `c`.class == 4220) AND `c`.job_level > 60);
UPDATE `char` c SET `c`.job_level = 60, `c`.skill_point = 59 WHERE (`c`.class == 4218 OR `c`.class == 4220) AND `c`.job_level > 60;
