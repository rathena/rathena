--
-- Rune Tablet - database changes
--
-- Rune Tablet progress lives in character registries (`char_reg_num`), so the
-- feature itself needs no table. This script only:
--  1. adds the Rune Tablet episode race groups to existing SQL monster tables
--     (`mob_db`, `mob_db2`, `mob_db_re`, `mob_db2_re`, when use_sql_db is on);
--  2. migrates the legacy custom rune system (`runes`, `runes_book`).
--
-- Import into the main database with all servers stopped; back it up first.
-- Safe to re-run: missing columns are added once, the legacy tables are
-- migrated once and then renamed to `runes_legacy` / `runes_book_legacy`.
-- No existing Rune Tablet registry value is overwritten.
-- See doc/rune_tablet.txt.
--

DELIMITER $$

--
-- 1. Race groups RC2_RUNE_EP18..21 for SQL monster databases
-- These nullable fields do not assign any monster to a group.
--

DROP PROCEDURE IF EXISTS `rune_tablet_add_racegroups`$$
CREATE PROCEDURE `rune_tablet_add_racegroups`(IN `tbl` VARCHAR(64))
BEGIN
	-- `tbl` takes the database collation; compare it as binary so it never
	-- mixes with the information_schema collation.
	IF EXISTS (SELECT 1 FROM information_schema.`TABLES` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = CAST(`tbl` AS BINARY))
		AND NOT EXISTS (SELECT 1 FROM information_schema.`COLUMNS` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = CAST(`tbl` AS BINARY) AND `COLUMN_NAME` = 'racegroup_rune_ep18') THEN
		SET @rune_tablet_sql = CONCAT('ALTER TABLE `', `tbl`, '`',
			' ADD COLUMN `racegroup_rune_ep18` tinyint(1) unsigned DEFAULT NULL AFTER `racegroup_encroached_gephenia`,',
			' ADD COLUMN `racegroup_rune_ep19` tinyint(1) unsigned DEFAULT NULL AFTER `racegroup_rune_ep18`,',
			' ADD COLUMN `racegroup_rune_ep20` tinyint(1) unsigned DEFAULT NULL AFTER `racegroup_rune_ep19`,',
			' ADD COLUMN `racegroup_rune_ep21` tinyint(1) unsigned DEFAULT NULL AFTER `racegroup_rune_ep20`');
		PREPARE `rune_tablet_stmt` FROM @rune_tablet_sql;
		EXECUTE `rune_tablet_stmt`;
		DEALLOCATE PREPARE `rune_tablet_stmt`;
	END IF;
END$$

--
-- 2. Legacy custom rune system migration
-- runes_book (char_id, rune_id, book_id)        -> RuneTabletFragment[book_id] = 1
-- runes (char_id, rune_id, set_id, upgrade,     -> RuneTabletSet[set_id] = progress
--        failcount, reward, selected)           -> RuneTabletReward[set_id] = reward
--                                               -> RuneTabletEquipSet[set_id] = 1
-- Legacy tag IDs (rune_id) are the client categories, book IDs the fragment IDs.
-- Progress encoding: failcount 0 stores level+1; otherwise
-- 2^32 + (failcount << 8) + level + 1 (see rune_tablet_logic in src/map/rune_tablet.cpp).
-- Sets equipped beyond rune_tablet_max_equipped (or a second set of a category
-- with rune_tablet_equip_per_category) are pruned at the next login.
--

DROP PROCEDURE IF EXISTS `rune_tablet_migrate_legacy`$$
CREATE PROCEDURE `rune_tablet_migrate_legacy`()
BEGIN
	IF EXISTS (SELECT 1 FROM information_schema.`TABLES` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'runes_book') THEN
		INSERT IGNORE INTO `char_reg_num` (`char_id`, `key`, `index`, `value`)
			SELECT `char_id`, 'RuneTabletFragment', `book_id`, 1 FROM `runes_book`;
		RENAME TABLE `runes_book` TO `runes_book_legacy`;
	END IF;

	IF EXISTS (SELECT 1 FROM information_schema.`TABLES` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'runes') THEN
		INSERT IGNORE INTO `char_reg_num` (`char_id`, `key`, `index`, `value`)
			SELECT `char_id`, 'RuneTabletSet', `set_id`,
				IF(`failcount` = 0, LEAST(`upgrade`, 15) + 1, 4294967296 + (`failcount` << 8) + LEAST(`upgrade`, 15) + 1)
			FROM `runes`;
		INSERT IGNORE INTO `char_reg_num` (`char_id`, `key`, `index`, `value`)
			SELECT `char_id`, 'RuneTabletReward', `set_id`, LEAST(`reward`, 7) FROM `runes` WHERE `reward` > 0;
		INSERT IGNORE INTO `char_reg_num` (`char_id`, `key`, `index`, `value`)
			SELECT `char_id`, 'RuneTabletEquipSet', `set_id`, 1 FROM `runes` WHERE `selected` <> 0;
		RENAME TABLE `runes` TO `runes_legacy`;
	END IF;
END$$

DELIMITER ;

CALL `rune_tablet_add_racegroups`('mob_db');
CALL `rune_tablet_add_racegroups`('mob_db2');
CALL `rune_tablet_add_racegroups`('mob_db_re');
CALL `rune_tablet_add_racegroups`('mob_db2_re');
CALL `rune_tablet_migrate_legacy`();

DROP PROCEDURE `rune_tablet_add_racegroups`;
DROP PROCEDURE `rune_tablet_migrate_legacy`;

--
-- Once the migration is verified in game, the renamed legacy tables can be dropped:
--
-- DROP TABLE IF EXISTS `runes_legacy`;
-- DROP TABLE IF EXISTS `runes_book_legacy`;
--
