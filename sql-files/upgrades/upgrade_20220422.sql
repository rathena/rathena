-- Reset Nameless Island and Geoborg Family Curse quests which were using custom IDs that have been overtaken
DELETE FROM `quest` WHERE `quest_id` >= 17000 AND `quest_id` <= 17017;
DELETE FROM `quest` WHERE `quest_id` >= 18030 AND `quest_id` <= 18052;
