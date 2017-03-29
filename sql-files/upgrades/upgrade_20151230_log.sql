--
-- Table structure for table `feedinglog`
--

CREATE TABLE IF NOT EXISTS `feedinglog` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `time` DATETIME NOT NULL DEFAULT '0000-00-00 00:00:00',
  `char_id` INT(11) NOT NULL,
  `target_id` INT(11) NOT NULL,
  `target_class` SMALLINT(11) NOT NULL,
  `type` ENUM('P','H','O') NOT NULL, -- P: Pet, H: Homunculus, O: Other
  `intimacy` INT(11) UNSIGNED NOT NULL,
  `item_id` SMALLINT(5) UNSIGNED NOT NULL,
  `map` VARCHAR(11) NOT NULL,
  `x` SMALLINT(5) UNSIGNED NOT NULL,
  `y` SMALLINT(5) UNSIGNED NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE = MyISAM AUTO_INCREMENT = 1;

