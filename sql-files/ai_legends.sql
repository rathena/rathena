--
-- Table structure for AI Legends system
--

--
-- Table structure for AI Legend characters
--

CREATE TABLE IF NOT EXISTS `ai_legends` (
  `legend_id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(64) NOT NULL,
  `class_id` int(11) NOT NULL,
  `base_level` int(11) NOT NULL DEFAULT '200',
  `job_level` int(11) NOT NULL DEFAULT '70',
  `mbti` varchar(4) NOT NULL,
  `enabled` tinyint(1) NOT NULL DEFAULT '1',
  `last_appearance` datetime DEFAULT NULL,
  `last_map` varchar(32) DEFAULT NULL,
  `last_x` int(11) DEFAULT NULL,
  `last_y` int(11) DEFAULT NULL,
  `current_party_id` int(11) DEFAULT NULL,
  `current_mvp_id` int(11) DEFAULT NULL,
  `pvp_challenges_today` int(11) NOT NULL DEFAULT '0',
  `last_pvp_reset` date DEFAULT NULL,
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`legend_id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for AI Legend equipment
--

CREATE TABLE IF NOT EXISTS `ai_legend_equipment` (
  `equip_id` int(11) NOT NULL AUTO_INCREMENT,
  `legend_id` int(11) NOT NULL,
  `item_id` int(11) NOT NULL,
  `refine` tinyint(4) NOT NULL DEFAULT '0',
  `attribute` tinyint(4) NOT NULL DEFAULT '0',
  `card0` int(11) NOT NULL DEFAULT '0',
  `card1` int(11) NOT NULL DEFAULT '0',
  `card2` int(11) NOT NULL DEFAULT '0',
  `card3` int(11) NOT NULL DEFAULT '0',
  `option_id0` smallint(5) NOT NULL DEFAULT '0',
  `option_val0` smallint(5) NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) NOT NULL DEFAULT '0',
  `option_id1` smallint(5) NOT NULL DEFAULT '0',
  `option_val1` smallint(5) NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) NOT NULL DEFAULT '0',
  `option_id2` smallint(5) NOT NULL DEFAULT '0',
  `option_val2` smallint(5) NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) NOT NULL DEFAULT '0',
  `option_id3` smallint(5) NOT NULL DEFAULT '0',
  `option_val3` smallint(5) NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) NOT NULL DEFAULT '0',
  `option_id4` smallint(5) NOT NULL DEFAULT '0',
  `option_val4` smallint(5) NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) NOT NULL DEFAULT '0',
  `expire_time` int(11) NOT NULL DEFAULT '0',
  `bound` tinyint(3) NOT NULL DEFAULT '0',
  `unique_id` bigint(20) NOT NULL DEFAULT '0',
  PRIMARY KEY (`equip_id`),
  KEY `legend_id` (`legend_id`),
  CONSTRAINT `ai_legend_equipment_ibfk_1` FOREIGN KEY (`legend_id`) REFERENCES `ai_legends` (`legend_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for AI Legend skills
--

CREATE TABLE IF NOT EXISTS `ai_legend_skills` (
  `skill_id` int(11) NOT NULL AUTO_INCREMENT,
  `legend_id` int(11) NOT NULL,
  `skill_id` int(11) NOT NULL,
  `skill_lv` tinyint(4) NOT NULL DEFAULT '0',
  PRIMARY KEY (`skill_id`),
  KEY `legend_id` (`legend_id`),
  CONSTRAINT `ai_legend_skills_ibfk_1` FOREIGN KEY (`legend_id`) REFERENCES `ai_legends` (`legend_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for AI Legend backstory pieces
--

CREATE TABLE IF NOT EXISTS `ai_legend_backstory` (
  `backstory_id` int(11) NOT NULL AUTO_INCREMENT,
  `legend_id` int(11) NOT NULL,
  `piece_number` int(11) NOT NULL,
  `content` text NOT NULL,
  `hint` varchar(255) NOT NULL,
  PRIMARY KEY (`backstory_id`),
  UNIQUE KEY `legend_piece` (`legend_id`,`piece_number`),
  CONSTRAINT `ai_legend_backstory_ibfk_1` FOREIGN KEY (`legend_id`) REFERENCES `ai_legends` (`legend_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for AI Legend secret skills
--

CREATE TABLE IF NOT EXISTS `ai_legend_secret_skills` (
  `secret_skill_id` int(11) NOT NULL AUTO_INCREMENT,
  `legend_id` int(11) NOT NULL,
  `skill_id` int(11) NOT NULL,
  `skill_name` varchar(64) NOT NULL,
  `skill_description` text NOT NULL,
  `skill_condition` text NOT NULL,
  `skill_effect` text NOT NULL,
  PRIMARY KEY (`secret_skill_id`),
  UNIQUE KEY `legend_id` (`legend_id`),
  CONSTRAINT `ai_legend_secret_skills_ibfk_1` FOREIGN KEY (`legend_id`) REFERENCES `ai_legends` (`legend_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for AI Legend player interactions
--

CREATE TABLE IF NOT EXISTS `ai_legend_interactions` (
  `interaction_id` int(11) NOT NULL AUTO_INCREMENT,
  `legend_id` int(11) NOT NULL,
  `char_id` int(11) NOT NULL,
  `interaction_type` enum('CONVERSATION','PVP','PARTY','GIFT','QUEST','SKILL_TEACH') NOT NULL,
  `interaction_data` text,
  `interaction_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`interaction_id`),
  KEY `legend_id` (`legend_id`),
  KEY `char_id` (`char_id`),
  CONSTRAINT `ai_legend_interactions_ibfk_1` FOREIGN KEY (`legend_id`) REFERENCES `ai_legends` (`legend_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for AI Legend player relationships
--

CREATE TABLE IF NOT EXISTS `ai_legend_relationships` (
  `relationship_id` int(11) NOT NULL AUTO_INCREMENT,
  `legend_id` int(11) NOT NULL,
  `char_id` int(11) NOT NULL,
  `relationship_value` int(11) NOT NULL DEFAULT '0',
  `relationship_type` varchar(32) NOT NULL DEFAULT 'Neutral',
  `backstory_pieces_revealed` text,
  `last_interaction` datetime DEFAULT NULL,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`relationship_id`),
  UNIQUE KEY `legend_char` (`legend_id`,`char_id`),
  KEY `legend_id` (`legend_id`),
  KEY `char_id` (`char_id`),
  CONSTRAINT `ai_legend_relationships_ibfk_1` FOREIGN KEY (`legend_id`) REFERENCES `ai_legends` (`legend_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for AI Legend memory
--

CREATE TABLE IF NOT EXISTS `ai_legend_memory` (
  `memory_id` int(11) NOT NULL AUTO_INCREMENT,
  `legend_id` int(11) NOT NULL,
  `char_id` int(11) NOT NULL,
  `memory_type` varchar(32) NOT NULL,
  `content` text NOT NULL,
  `embedding` text,
  `importance` int(11) NOT NULL DEFAULT '1',
  `is_long_term` tinyint(1) NOT NULL DEFAULT '0',
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `last_accessed` datetime DEFAULT NULL,
  `expiry_time` datetime DEFAULT NULL,
  PRIMARY KEY (`memory_id`),
  KEY `legend_id` (`legend_id`),
  KEY `char_id` (`char_id`),
  KEY `memory_type` (`memory_type`),
  CONSTRAINT `ai_legend_memory_ibfk_1` FOREIGN KEY (`legend_id`) REFERENCES `ai_legends` (`legend_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for AI Legend conversations
--

CREATE TABLE IF NOT EXISTS `ai_legend_conversations` (
  `conversation_id` int(11) NOT NULL AUTO_INCREMENT,
  `legend_id` int(11) NOT NULL,
  `char_id` int(11) NOT NULL,
  `message` text NOT NULL,
  `is_player` tinyint(1) NOT NULL DEFAULT '1',
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`conversation_id`),
  KEY `legend_id` (`legend_id`),
  KEY `char_id` (`char_id`),
  CONSTRAINT `ai_legend_conversations_ibfk_1` FOREIGN KEY (`legend_id`) REFERENCES `ai_legends` (`legend_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for AI Legend PvP history
--

CREATE TABLE IF NOT EXISTS `ai_legend_pvp_history` (
  `pvp_id` int(11) NOT NULL AUTO_INCREMENT,
  `legend_id` int(11) NOT NULL,
  `char_id` int(11) NOT NULL,
  `winner` enum('LEGEND','PLAYER','DRAW') NOT NULL,
  `pvp_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `pvp_map` varchar(32) NOT NULL,
  `pvp_x` int(11) NOT NULL,
  `pvp_y` int(11) NOT NULL,
  PRIMARY KEY (`pvp_id`),
  KEY `legend_id` (`legend_id`),
  KEY `char_id` (`char_id`),
  CONSTRAINT `ai_legend_pvp_history_ibfk_1` FOREIGN KEY (`legend_id`) REFERENCES `ai_legends` (`legend_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for AI Legend party history
--

CREATE TABLE IF NOT EXISTS `ai_legend_party_history` (
  `party_id` int(11) NOT NULL AUTO_INCREMENT,
  `legend_id` int(11) NOT NULL,
  `party_id` int(11) NOT NULL,
  `join_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `leave_time` datetime DEFAULT NULL,
  `mvp_id` int(11) DEFAULT NULL,
  `mvp_name` varchar(64) DEFAULT NULL,
  `success` tinyint(1) DEFAULT NULL,
  PRIMARY KEY (`party_id`),
  KEY `legend_id` (`legend_id`),
  CONSTRAINT `ai_legend_party_history_ibfk_1` FOREIGN KEY (`legend_id`) REFERENCES `ai_legends` (`legend_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for AI Legend gifts
--

CREATE TABLE IF NOT EXISTS `ai_legend_gifts` (
  `gift_id` int(11) NOT NULL AUTO_INCREMENT,
  `legend_id` int(11) NOT NULL,
  `char_id` int(11) NOT NULL,
  `item_id` int(11) NOT NULL,
  `amount` int(11) NOT NULL DEFAULT '1',
  `refine` tinyint(4) NOT NULL DEFAULT '0',
  `attribute` tinyint(4) NOT NULL DEFAULT '0',
  `card0` int(11) NOT NULL DEFAULT '0',
  `card1` int(11) NOT NULL DEFAULT '0',
  `card2` int(11) NOT NULL DEFAULT '0',
  `card3` int(11) NOT NULL DEFAULT '0',
  `gift_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `gift_reason` text,
  PRIMARY KEY (`gift_id`),
  KEY `legend_id` (`legend_id`),
  KEY `char_id` (`char_id`),
  CONSTRAINT `ai_legend_gifts_ibfk_1` FOREIGN KEY (`legend_id`) REFERENCES `ai_legends` (`legend_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for player secret skills
--

CREATE TABLE IF NOT EXISTS `player_secret_skills` (
  `player_skill_id` int(11) NOT NULL AUTO_INCREMENT,
  `char_id` int(11) NOT NULL,
  `skill_id` int(11) NOT NULL,
  `skill_lv` tinyint(4) NOT NULL DEFAULT '1',
  `legend_id` int(11) NOT NULL,
  `acquisition_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`player_skill_id`),
  UNIQUE KEY `char_skill` (`char_id`,`skill_id`),
  KEY `legend_id` (`legend_id`),
  CONSTRAINT `player_secret_skills_ibfk_1` FOREIGN KEY (`legend_id`) REFERENCES `ai_legends` (`legend_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for secret skill quests
--

CREATE TABLE IF NOT EXISTS `secret_skill_quests` (
  `quest_id` int(11) NOT NULL AUTO_INCREMENT,
  `char_id` int(11) NOT NULL,
  `legend_id` int(11) NOT NULL,
  `skill_id` int(11) NOT NULL,
  `quest_step` int(11) NOT NULL DEFAULT '1',
  `total_steps` int(11) NOT NULL DEFAULT '5',
  `current_progress` int(11) NOT NULL DEFAULT '0',
  `required_progress` int(11) NOT NULL DEFAULT '1',
  `start_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `last_update` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `completed` tinyint(1) NOT NULL DEFAULT '0',
  `completion_time` datetime DEFAULT NULL,
  PRIMARY KEY (`quest_id`),
  UNIQUE KEY `char_legend_skill` (`char_id`,`legend_id`,`skill_id`),
  KEY `legend_id` (`legend_id`),
  CONSTRAINT `secret_skill_quests_ibfk_1` FOREIGN KEY (`legend_id`) REFERENCES `ai_legends` (`legend_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for AI Legend logs
--

CREATE TABLE IF NOT EXISTS `ai_legend_logs` (
  `log_id` int(11) NOT NULL AUTO_INCREMENT,
  `legend_id` int(11) DEFAULT NULL,
  `char_id` int(11) DEFAULT NULL,
  `log_type` varchar(32) NOT NULL,
  `message` text NOT NULL,
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`log_id`),
  KEY `legend_id` (`legend_id`),
  KEY `char_id` (`char_id`),
  KEY `log_type` (`log_type`),
  KEY `created_at` (`created_at`),
  CONSTRAINT `ai_legend_logs_ibfk_1` FOREIGN KEY (`legend_id`) REFERENCES `ai_legends` (`legend_id`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for AI Legend statistics
--

CREATE TABLE IF NOT EXISTS `ai_legend_statistics` (
  `stat_id` int(11) NOT NULL AUTO_INCREMENT,
  `legend_id` int(11) NOT NULL,
  `appearances` int(11) NOT NULL DEFAULT '0',
  `conversations` int(11) NOT NULL DEFAULT '0',
  `pvp_challenges` int(11) NOT NULL DEFAULT '0',
  `pvp_wins` int(11) NOT NULL DEFAULT '0',
  `pvp_losses` int(11) NOT NULL DEFAULT '0',
  `pvp_draws` int(11) NOT NULL DEFAULT '0',
  `parties_joined` int(11) NOT NULL DEFAULT '0',
  `mvp_assists` int(11) NOT NULL DEFAULT '0',
  `gifts_given` int(11) NOT NULL DEFAULT '0',
  `skills_taught` int(11) NOT NULL DEFAULT '0',
  `backstory_pieces_revealed` int(11) NOT NULL DEFAULT '0',
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`stat_id`),
  UNIQUE KEY `legend_id` (`legend_id`),
  CONSTRAINT `ai_legend_statistics_ibfk_1` FOREIGN KEY (`legend_id`) REFERENCES `ai_legends` (`legend_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Initial data for AI Legends
--

INSERT IGNORE INTO `ai_legends` (`name`, `class_id`, `base_level`, `job_level`, `mbti`) VALUES
('Siegfried Dragonheart', 4060, 200, 70, 'INTJ'),
('Elara Stormweaver', 4061, 200, 70, 'INTP'),
('Talos Elementis', 4062, 200, 70, 'ENFJ'),
('Artemis Hawkgaze', 4063, 200, 70, 'ISTP'),
('Lyric Songweaver', 4064, 200, 70, 'ESFP'),
('Aria Melodica', 4065, 200, 70, 'ENFP'),
('Nexus Gearhart', 4066, 200, 70, 'ISTJ'),
('Malachite Vialborn', 4067, 200, 70, 'ENTP'),
('Nyx Shadowstep', 4068, 200, 70, 'INFJ'),
('Mirage Duskwalker', 4069, 200, 70, 'ISFP'),
('Seraphina Lightbringer', 4070, 200, 70, 'ESFJ'),
('Thorne Judgement', 4071, 200, 70, 'ENTJ'),
('Flint Sixkiller', 4072, 200, 70, 'ESTP'),
('Kage Nightshade', 4073, 200, 70, 'INFP'),
('Asteria Starforge', 4074, 200, 70, 'INTJ'),
('Omni Allskill', 4075, 200, 70, 'ESTJ'),
('Wraith Soulbinder', 4076, 200, 70, 'ISFJ'),
('Fauna Wildshaper', 4077, 200, 70, 'ENFP'),
('Aegis Shieldwall', 4078, 200, 70, 'ISFJ'),
('Maverick Gunsmoke', 4079, 200, 70, 'ISTP'),
('Forge Mastercraft', 4080, 200, 70, 'ISTJ'),
('Vigil Duskgaze', 4081, 200, 70, 'INTJ'),
('Sanctus Divinus', 4082, 200, 70, 'ENFJ'),
('Tempus Clockwork', 4083, 200, 70, 'INTP'),
('Zephyr Skyrider', 4084, 200, 70, 'ESTP'),
('Kuro Shadowflame', 4085, 200, 70, 'INFP'),
('Mortis Soulharvest', 4086, 200, 70, 'INTJ'),
('Prism Elementbound', 4087, 200, 70, 'ENFP'),
('Luna Moonshadow', 4088, 200, 70, 'ISFP'),
('Oni Spiritwalker', 4089, 200, 70, 'ENTP'),
('Mei Charmweaver', 4090, 200, 70, 'ESFJ'),
('Nexus Soulbond', 4091, 200, 70, 'INFJ'),
('Rookie Masterofall', 4092, 200, 70, 'ENFP'),
('Whisper Spiritcaller', 4093, 200, 70, 'INFP'),
('Feral Beastmaster', 4094, 200, 70, 'ESTP'),
('Gaia Naturebound', 4095, 200, 70, 'INFJ');

--
-- Initial data for AI Legend secret skills
--

INSERT IGNORE INTO `ai_legend_secret_skills` (`legend_id`, `skill_id`, `skill_name`, `skill_description`, `skill_condition`, `skill_effect`) VALUES
(1, 8001, 'Dragon\'s Breath', 'Channel the power of dragons to unleash a devastating breath attack.', 'Defeat 10 dragon-type monsters without using any potions', 'Deals fire damage to all enemies in a cone in front of the caster. Damage scales with base level and INT.'),
(2, 8002, 'Arcane Convergence', 'Focus arcane energy into a single devastating spell.', 'Cast 100 different spells in a single day without dying', 'Deals magic damage to a single target based on the number of different spells cast in the last hour.'),
(3, 8003, 'Elemental Harmony', 'Harmonize with the elements to enhance elemental magic.', 'Summon all four elemental spirits simultaneously and maintain them for 10 minutes', 'Increases damage of elemental spells by 50% and reduces their cast time by 30%.'),
(4, 8004, 'Piercing Sight', 'See through any disguise or invisibility.', 'Hit 50 moving targets from over 10 cells away without missing', 'Reveals hidden enemies and allows attacks to ignore a portion of enemy defense.'),
(5, 8005, 'Symphony of Souls', 'Play a melody that resonates with the souls of allies.', 'Play music for 30 different players and receive positive feedback from each', 'Enhances all party member stats and grants immunity to silence and sleep.'),
(6, 8006, 'Harmonic Resonance', 'Create a harmonic field that enhances allies and disrupts enemies.', 'Perform a song in every major city without using any transportation methods', 'Creates an area of effect that boosts ally stats and reduces enemy stats.'),
(7, 8007, 'Mechanical Overload', 'Push mechanical systems beyond their normal limits.', 'Disassemble 100 mechanical monsters and collect their parts', 'Temporarily enhances all mechanical equipment and skills, increasing damage and reducing cooldowns.'),
(8, 8008, 'Catalytic Conversion', 'Convert one substance into another through advanced alchemy.', 'Create 50 different potions and use them on other players', 'Transforms items into more valuable forms and enhances potion effects.'),
(9, 8009, 'Void Embrace', 'Embrace the void to become one with the shadows.', 'Assassinate 30 targets without being detected or entering combat', 'Grants temporary invisibility that persists during attacks and increases critical rate.'),
(10, 8010, 'Phantom Mirage', 'Create a perfect illusion that can act independently.', 'Create 20 successful illusions that fool other players', 'Creates a clone that can use skills and take damage, confusing enemies.'),
(11, 8011, 'Divine Intervention', 'Call upon divine power to save allies from certain death.', 'Resurrect 50 fallen players without receiving any compensation', 'Automatically resurrects fallen party members with full HP and SP once every 30 minutes.'),
(12, 8012, 'Righteous Fury', 'Channel divine wrath into devastating attacks.', 'Defeat 30 demon-type monsters using only holy damage', 'Converts a portion of physical damage to holy damage and increases damage against demon and undead enemies.'),
(13, 8013, 'Bullet Time', 'Slow down perception of time to make impossible shots.', 'Hit 10 different targets with a single bullet using ricochet techniques', 'Temporarily slows down time, increasing attack speed and evasion dramatically.'),
(14, 8014, 'Shadow Merge', 'Merge with shadows to move undetected.', 'Remain undetected in a crowded area for 1 hour while completing objectives', 'Allows movement through shadows, bypassing obstacles and remaining undetected.'),
(15, 8015, 'Cosmic Alignment', 'Align with cosmic forces to enhance abilities.', 'Meditate under the stars for 7 consecutive nights in different locations', 'Enhances all stats based on the current phase of the moon and time of day.'),
(16, 8016, 'Omnipotence', 'Draw upon knowledge of all classes to adapt to any situation.', 'Master at least one skill from every job class in the game', 'Allows temporary use of skills from other classes based on the situation.'),
(17, 8017, 'Spirit Legion', 'Summon a legion of spirits to fight alongside you.', 'Commune with spirits in 20 different haunted locations', 'Summons multiple spirits that attack enemies and provide various buffs.'),
(18, 8018, 'Wild Transformation', 'Transform into various animal forms.', 'Befriend 30 different animal species without using any items', 'Allows transformation into different animal forms with unique abilities.'),
(19, 8019, 'Impenetrable Bulwark', 'Create an impenetrable shield that protects allies.', 'Protect 20 different players from fatal damage using defensive skills', 'Creates a barrier that absorbs damage for all party members.'),
(20, 8020, 'Revolutionary Barrage', 'Fire a barrage of bullets that can hit multiple targets.', 'Defeat 50 enemies using only explosive ammunition', 'Fires multiple bullets that can hit multiple targets and ignore a portion of defense.'),
(21, 8021, 'Masterwork Creation', 'Create perfect items with enhanced properties.', 'Craft 30 perfect items without any failures', 'Enhances crafting success rate and adds bonus properties to crafted items.'),
(22, 8022, 'Shadow Surveillance', 'Use shadows to spy on enemies and gather information.', 'Detect and expose 20 hidden enemies without being detected yourself', 'Reveals hidden enemies and provides information about their stats and weaknesses.'),
(23, 8023, 'Divine Judgment', 'Call down divine judgment on enemies.', 'Purify 30 cursed areas using only holy magic', 'Deals holy damage to all enemies in an area and has a chance to instantly defeat undead enemies.'),
(24, 8024, 'Temporal Distortion', 'Manipulate time to gain advantages in battle.', 'Complete 10 time-sensitive tasks with perfect timing', 'Temporarily slows down time for enemies while speeding it up for allies.'),
(25, 8025, 'Cyclone Strike', 'Harness the power of wind to strike from above.', 'Perform 50 successful aerial maneuvers during combat', 'Deals wind damage to all enemies in an area and has a chance to knock them back.'),
(26, 8026, 'Shadow Conflagration', 'Combine shadow and fire to create devastating attacks.', 'Master both shadow and fire techniques by defeating 30 enemies using only these elements', 'Deals shadow and fire damage to all enemies in an area and has a chance to cause burning and blind status.'),
(27, 8027, 'Soul Harvest', 'Harvest the souls of fallen enemies to enhance abilities.', 'Collect the souls of 50 different monster types', 'Absorbs a portion of defeated enemy\'s stats temporarily.'),
(28, 8028, 'Elemental Convergence', 'Combine all elements into a single devastating attack.', 'Master all four elements by defeating 10 monsters of each elemental type', 'Deals damage of all elements to a single target, bypassing elemental resistances.'),
(29, 8029, 'Lunar Transformation', 'Transform based on the phases of the moon.', 'Experience all phases of the moon while completing specific tasks at each phase', 'Grants different abilities based on the current phase of the moon.'),
(30, 8030, 'Spirit Transformation', 'Transform into different spirit forms.', 'Master the art of transformation by assuming 10 different forms', 'Allows transformation into different spirit forms with unique abilities.'),
(31, 8031, 'Mystic Charm', 'Charm enemies and allies with mystical eastern magic.', 'Create 20 successful illusions that charm other players or monsters', 'Charms enemies to fight for you temporarily and enhances party member abilities.'),
(32, 8032, 'Soul Convergence', 'Link souls with allies to share abilities and power.', 'Link souls with 30 different players and help them achieve their goals', 'Links with party members to share stats and abilities.'),
(33, 8033, 'Beginner\'s Luck', 'Channel the unpredictable power of a novice.', 'Defeat 10 MVP monsters while wearing only novice equipment', 'Has a chance to perform miraculous feats that would normally be impossible.'),
(34, 8034, 'Spirit Communion', 'Commune with spirits to gain their power.', 'Communicate with 20 different types of spirits in their natural habitats', 'Summons spirits that provide various buffs and can attack enemies.'),
(35, 8035, 'Wild Communion', 'Commune with wild beasts to gain their power.', 'Tame 30 different wild beasts without using any items', 'Summons wild beasts that fight alongside you and provide various buffs.'),
(36, 8036, 'Nature\'s Blessing', 'Channel the power of nature to heal and protect.', 'Commune with nature spirits in 20 different natural locations', 'Enhances healing abilities and provides protection from elemental damage.');

--
-- Initial data for AI Legend backstory pieces
--

INSERT IGNORE INTO `ai_legend_backstory` (`legend_id`, `piece_number`, `content`, `hint`) VALUES
(1, 1, 'Siegfried was born in the shadow of Dragon Mountain, where ancient dragons were said to slumber.', 'Ask about birthplace'),
(1, 2, 'As a child, Siegfried survived a dragon attack that destroyed his village, leaving him with a scar and an unusual connection to dragons.', 'Notice the scar on his face'),
(1, 3, 'He trained under the legendary Dragon Knight Bahamut, learning the ancient arts of dragon communion.', 'Ask about his mentor'),
(1, 4, 'Siegfried once saved a young dragon from poachers, earning the gratitude of its mother, who gifted him with a scale that he forged into his armor.', 'Comment on his unusual armor'),
(1, 5, 'He discovered an ancient ritual that allowed him to temporarily take on draconic aspects, but each transformation takes a toll on his humanity.', 'Witness his transformation'),
(1, 6, 'Siegfried seeks the legendary Dragon\'s Heart, an artifact said to grant perfect communion with dragons.', 'Ask about his quest'),
(1, 7, 'He fears that one day he will fully transform into a dragon, losing his human consciousness forever.', 'Share a campfire with him at night');

-- Add more backstory pieces for other legends as needed