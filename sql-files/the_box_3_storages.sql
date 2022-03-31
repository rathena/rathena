/*
 Navicat Premium Data Transfer

 Source Server         : localhost_3306
 Source Server Type    : MySQL
 Source Server Version : 80021
 Source Host           : localhost:3306
 Source Schema         : tb3

 Target Server Type    : MySQL
 Target Server Version : 80021
 File Encoding         : 65001

 Date: 16/03/2022 11:55:07
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for storage2
-- ----------------------------
DROP TABLE IF EXISTS `storage2`;
CREATE TABLE `storage2`  (
  `id` int UNSIGNED NOT NULL AUTO_INCREMENT,
  `account_id` int UNSIGNED NOT NULL DEFAULT 0,
  `nameid` int UNSIGNED NOT NULL DEFAULT 0,
  `amount` smallint UNSIGNED NOT NULL DEFAULT 0,
  `equip` int UNSIGNED NOT NULL DEFAULT 0,
  `identify` smallint UNSIGNED NOT NULL DEFAULT 0,
  `refine` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `attribute` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `card0` int UNSIGNED NOT NULL DEFAULT 0,
  `card1` int UNSIGNED NOT NULL DEFAULT 0,
  `card2` int UNSIGNED NOT NULL DEFAULT 0,
  `card3` int UNSIGNED NOT NULL DEFAULT 0,
  `option_id0` smallint NOT NULL DEFAULT 0,
  `option_val0` smallint NOT NULL DEFAULT 0,
  `option_parm0` tinyint NOT NULL DEFAULT 0,
  `option_id1` smallint NOT NULL DEFAULT 0,
  `option_val1` smallint NOT NULL DEFAULT 0,
  `option_parm1` tinyint NOT NULL DEFAULT 0,
  `option_id2` smallint NOT NULL DEFAULT 0,
  `option_val2` smallint NOT NULL DEFAULT 0,
  `option_parm2` tinyint NOT NULL DEFAULT 0,
  `option_id3` smallint NOT NULL DEFAULT 0,
  `option_val3` smallint NOT NULL DEFAULT 0,
  `option_parm3` tinyint NOT NULL DEFAULT 0,
  `option_id4` smallint NOT NULL DEFAULT 0,
  `option_val4` smallint NOT NULL DEFAULT 0,
  `option_parm4` tinyint NOT NULL DEFAULT 0,
  `expire_time` int UNSIGNED NOT NULL DEFAULT 0,
  `bound` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `unique_id` bigint UNSIGNED NOT NULL DEFAULT 0,
  `enchantgrade` tinyint UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`) USING BTREE,
  INDEX `account_id`(`account_id`) USING BTREE
) ENGINE = MyISAM AUTO_INCREMENT = 148832 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = FIXED;

-- ----------------------------
-- Table structure for storage3
-- ----------------------------
DROP TABLE IF EXISTS `storage3`;
CREATE TABLE `storage3`  (
  `id` int UNSIGNED NOT NULL AUTO_INCREMENT,
  `account_id` int UNSIGNED NOT NULL DEFAULT 0,
  `nameid` int UNSIGNED NOT NULL DEFAULT 0,
  `amount` smallint UNSIGNED NOT NULL DEFAULT 0,
  `equip` int UNSIGNED NOT NULL DEFAULT 0,
  `identify` smallint UNSIGNED NOT NULL DEFAULT 0,
  `refine` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `attribute` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `card0` int UNSIGNED NOT NULL DEFAULT 0,
  `card1` int UNSIGNED NOT NULL DEFAULT 0,
  `card2` int UNSIGNED NOT NULL DEFAULT 0,
  `card3` int UNSIGNED NOT NULL DEFAULT 0,
  `option_id0` smallint NOT NULL DEFAULT 0,
  `option_val0` smallint NOT NULL DEFAULT 0,
  `option_parm0` tinyint NOT NULL DEFAULT 0,
  `option_id1` smallint NOT NULL DEFAULT 0,
  `option_val1` smallint NOT NULL DEFAULT 0,
  `option_parm1` tinyint NOT NULL DEFAULT 0,
  `option_id2` smallint NOT NULL DEFAULT 0,
  `option_val2` smallint NOT NULL DEFAULT 0,
  `option_parm2` tinyint NOT NULL DEFAULT 0,
  `option_id3` smallint NOT NULL DEFAULT 0,
  `option_val3` smallint NOT NULL DEFAULT 0,
  `option_parm3` tinyint NOT NULL DEFAULT 0,
  `option_id4` smallint NOT NULL DEFAULT 0,
  `option_val4` smallint NOT NULL DEFAULT 0,
  `option_parm4` tinyint NOT NULL DEFAULT 0,
  `expire_time` int UNSIGNED NOT NULL DEFAULT 0,
  `bound` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `unique_id` bigint UNSIGNED NOT NULL DEFAULT 0,
  `enchantgrade` tinyint UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`) USING BTREE,
  INDEX `account_id`(`account_id`) USING BTREE
) ENGINE = MyISAM AUTO_INCREMENT = 148832 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = FIXED;

-- ----------------------------
-- Table structure for storage4
-- ----------------------------
DROP TABLE IF EXISTS `storage4`;
CREATE TABLE `storage4`  (
  `id` int UNSIGNED NOT NULL AUTO_INCREMENT,
  `account_id` int UNSIGNED NOT NULL DEFAULT 0,
  `nameid` int UNSIGNED NOT NULL DEFAULT 0,
  `amount` smallint UNSIGNED NOT NULL DEFAULT 0,
  `equip` int UNSIGNED NOT NULL DEFAULT 0,
  `identify` smallint UNSIGNED NOT NULL DEFAULT 0,
  `refine` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `attribute` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `card0` int UNSIGNED NOT NULL DEFAULT 0,
  `card1` int UNSIGNED NOT NULL DEFAULT 0,
  `card2` int UNSIGNED NOT NULL DEFAULT 0,
  `card3` int UNSIGNED NOT NULL DEFAULT 0,
  `option_id0` smallint NOT NULL DEFAULT 0,
  `option_val0` smallint NOT NULL DEFAULT 0,
  `option_parm0` tinyint NOT NULL DEFAULT 0,
  `option_id1` smallint NOT NULL DEFAULT 0,
  `option_val1` smallint NOT NULL DEFAULT 0,
  `option_parm1` tinyint NOT NULL DEFAULT 0,
  `option_id2` smallint NOT NULL DEFAULT 0,
  `option_val2` smallint NOT NULL DEFAULT 0,
  `option_parm2` tinyint NOT NULL DEFAULT 0,
  `option_id3` smallint NOT NULL DEFAULT 0,
  `option_val3` smallint NOT NULL DEFAULT 0,
  `option_parm3` tinyint NOT NULL DEFAULT 0,
  `option_id4` smallint NOT NULL DEFAULT 0,
  `option_val4` smallint NOT NULL DEFAULT 0,
  `option_parm4` tinyint NOT NULL DEFAULT 0,
  `expire_time` int UNSIGNED NOT NULL DEFAULT 0,
  `bound` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `unique_id` bigint UNSIGNED NOT NULL DEFAULT 0,
  `enchantgrade` tinyint UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`) USING BTREE,
  INDEX `account_id`(`account_id`) USING BTREE
) ENGINE = MyISAM AUTO_INCREMENT = 148832 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = FIXED;

-- ----------------------------
-- Table structure for storage5
-- ----------------------------
DROP TABLE IF EXISTS `storage5`;
CREATE TABLE `storage5`  (
  `id` int UNSIGNED NOT NULL AUTO_INCREMENT,
  `account_id` int UNSIGNED NOT NULL DEFAULT 0,
  `nameid` int UNSIGNED NOT NULL DEFAULT 0,
  `amount` smallint UNSIGNED NOT NULL DEFAULT 0,
  `equip` int UNSIGNED NOT NULL DEFAULT 0,
  `identify` smallint UNSIGNED NOT NULL DEFAULT 0,
  `refine` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `attribute` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `card0` int UNSIGNED NOT NULL DEFAULT 0,
  `card1` int UNSIGNED NOT NULL DEFAULT 0,
  `card2` int UNSIGNED NOT NULL DEFAULT 0,
  `card3` int UNSIGNED NOT NULL DEFAULT 0,
  `option_id0` smallint NOT NULL DEFAULT 0,
  `option_val0` smallint NOT NULL DEFAULT 0,
  `option_parm0` tinyint NOT NULL DEFAULT 0,
  `option_id1` smallint NOT NULL DEFAULT 0,
  `option_val1` smallint NOT NULL DEFAULT 0,
  `option_parm1` tinyint NOT NULL DEFAULT 0,
  `option_id2` smallint NOT NULL DEFAULT 0,
  `option_val2` smallint NOT NULL DEFAULT 0,
  `option_parm2` tinyint NOT NULL DEFAULT 0,
  `option_id3` smallint NOT NULL DEFAULT 0,
  `option_val3` smallint NOT NULL DEFAULT 0,
  `option_parm3` tinyint NOT NULL DEFAULT 0,
  `option_id4` smallint NOT NULL DEFAULT 0,
  `option_val4` smallint NOT NULL DEFAULT 0,
  `option_parm4` tinyint NOT NULL DEFAULT 0,
  `expire_time` int UNSIGNED NOT NULL DEFAULT 0,
  `bound` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `unique_id` bigint UNSIGNED NOT NULL DEFAULT 0,
  `enchantgrade` tinyint UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`) USING BTREE,
  INDEX `account_id`(`account_id`) USING BTREE
) ENGINE = MyISAM AUTO_INCREMENT = 148832 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = FIXED;

-- ----------------------------
-- Table structure for storage6
-- ----------------------------
DROP TABLE IF EXISTS `storage6`;
CREATE TABLE `storage6`  (
  `id` int UNSIGNED NOT NULL AUTO_INCREMENT,
  `account_id` int UNSIGNED NOT NULL DEFAULT 0,
  `nameid` int UNSIGNED NOT NULL DEFAULT 0,
  `amount` smallint UNSIGNED NOT NULL DEFAULT 0,
  `equip` int UNSIGNED NOT NULL DEFAULT 0,
  `identify` smallint UNSIGNED NOT NULL DEFAULT 0,
  `refine` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `attribute` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `card0` int UNSIGNED NOT NULL DEFAULT 0,
  `card1` int UNSIGNED NOT NULL DEFAULT 0,
  `card2` int UNSIGNED NOT NULL DEFAULT 0,
  `card3` int UNSIGNED NOT NULL DEFAULT 0,
  `option_id0` smallint NOT NULL DEFAULT 0,
  `option_val0` smallint NOT NULL DEFAULT 0,
  `option_parm0` tinyint NOT NULL DEFAULT 0,
  `option_id1` smallint NOT NULL DEFAULT 0,
  `option_val1` smallint NOT NULL DEFAULT 0,
  `option_parm1` tinyint NOT NULL DEFAULT 0,
  `option_id2` smallint NOT NULL DEFAULT 0,
  `option_val2` smallint NOT NULL DEFAULT 0,
  `option_parm2` tinyint NOT NULL DEFAULT 0,
  `option_id3` smallint NOT NULL DEFAULT 0,
  `option_val3` smallint NOT NULL DEFAULT 0,
  `option_parm3` tinyint NOT NULL DEFAULT 0,
  `option_id4` smallint NOT NULL DEFAULT 0,
  `option_val4` smallint NOT NULL DEFAULT 0,
  `option_parm4` tinyint NOT NULL DEFAULT 0,
  `expire_time` int UNSIGNED NOT NULL DEFAULT 0,
  `bound` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `unique_id` bigint UNSIGNED NOT NULL DEFAULT 0,
  `enchantgrade` tinyint UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`) USING BTREE,
  INDEX `account_id`(`account_id`) USING BTREE
) ENGINE = MyISAM AUTO_INCREMENT = 148832 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = FIXED;

-- ----------------------------
-- Table structure for storage7
-- ----------------------------
DROP TABLE IF EXISTS `storage7`;
CREATE TABLE `storage7`  (
  `id` int UNSIGNED NOT NULL AUTO_INCREMENT,
  `account_id` int UNSIGNED NOT NULL DEFAULT 0,
  `nameid` int UNSIGNED NOT NULL DEFAULT 0,
  `amount` smallint UNSIGNED NOT NULL DEFAULT 0,
  `equip` int UNSIGNED NOT NULL DEFAULT 0,
  `identify` smallint UNSIGNED NOT NULL DEFAULT 0,
  `refine` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `attribute` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `card0` int UNSIGNED NOT NULL DEFAULT 0,
  `card1` int UNSIGNED NOT NULL DEFAULT 0,
  `card2` int UNSIGNED NOT NULL DEFAULT 0,
  `card3` int UNSIGNED NOT NULL DEFAULT 0,
  `option_id0` smallint NOT NULL DEFAULT 0,
  `option_val0` smallint NOT NULL DEFAULT 0,
  `option_parm0` tinyint NOT NULL DEFAULT 0,
  `option_id1` smallint NOT NULL DEFAULT 0,
  `option_val1` smallint NOT NULL DEFAULT 0,
  `option_parm1` tinyint NOT NULL DEFAULT 0,
  `option_id2` smallint NOT NULL DEFAULT 0,
  `option_val2` smallint NOT NULL DEFAULT 0,
  `option_parm2` tinyint NOT NULL DEFAULT 0,
  `option_id3` smallint NOT NULL DEFAULT 0,
  `option_val3` smallint NOT NULL DEFAULT 0,
  `option_parm3` tinyint NOT NULL DEFAULT 0,
  `option_id4` smallint NOT NULL DEFAULT 0,
  `option_val4` smallint NOT NULL DEFAULT 0,
  `option_parm4` tinyint NOT NULL DEFAULT 0,
  `expire_time` int UNSIGNED NOT NULL DEFAULT 0,
  `bound` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `unique_id` bigint UNSIGNED NOT NULL DEFAULT 0,
  `enchantgrade` tinyint UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`) USING BTREE,
  INDEX `account_id`(`account_id`) USING BTREE
) ENGINE = MyISAM AUTO_INCREMENT = 148832 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = FIXED;

-- ----------------------------
-- Table structure for storage8
-- ----------------------------
DROP TABLE IF EXISTS `storage8`;
CREATE TABLE `storage8`  (
  `id` int UNSIGNED NOT NULL AUTO_INCREMENT,
  `account_id` int UNSIGNED NOT NULL DEFAULT 0,
  `nameid` int UNSIGNED NOT NULL DEFAULT 0,
  `amount` smallint UNSIGNED NOT NULL DEFAULT 0,
  `equip` int UNSIGNED NOT NULL DEFAULT 0,
  `identify` smallint UNSIGNED NOT NULL DEFAULT 0,
  `refine` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `attribute` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `card0` int UNSIGNED NOT NULL DEFAULT 0,
  `card1` int UNSIGNED NOT NULL DEFAULT 0,
  `card2` int UNSIGNED NOT NULL DEFAULT 0,
  `card3` int UNSIGNED NOT NULL DEFAULT 0,
  `option_id0` smallint NOT NULL DEFAULT 0,
  `option_val0` smallint NOT NULL DEFAULT 0,
  `option_parm0` tinyint NOT NULL DEFAULT 0,
  `option_id1` smallint NOT NULL DEFAULT 0,
  `option_val1` smallint NOT NULL DEFAULT 0,
  `option_parm1` tinyint NOT NULL DEFAULT 0,
  `option_id2` smallint NOT NULL DEFAULT 0,
  `option_val2` smallint NOT NULL DEFAULT 0,
  `option_parm2` tinyint NOT NULL DEFAULT 0,
  `option_id3` smallint NOT NULL DEFAULT 0,
  `option_val3` smallint NOT NULL DEFAULT 0,
  `option_parm3` tinyint NOT NULL DEFAULT 0,
  `option_id4` smallint NOT NULL DEFAULT 0,
  `option_val4` smallint NOT NULL DEFAULT 0,
  `option_parm4` tinyint NOT NULL DEFAULT 0,
  `expire_time` int UNSIGNED NOT NULL DEFAULT 0,
  `bound` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `unique_id` bigint UNSIGNED NOT NULL DEFAULT 0,
  `enchantgrade` tinyint UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`) USING BTREE,
  INDEX `account_id`(`account_id`) USING BTREE
) ENGINE = MyISAM AUTO_INCREMENT = 148832 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = FIXED;

-- ----------------------------
-- Table structure for storage9
-- ----------------------------
DROP TABLE IF EXISTS `storage9`;
CREATE TABLE `storage9`  (
  `id` int UNSIGNED NOT NULL AUTO_INCREMENT,
  `account_id` int UNSIGNED NOT NULL DEFAULT 0,
  `nameid` int UNSIGNED NOT NULL DEFAULT 0,
  `amount` smallint UNSIGNED NOT NULL DEFAULT 0,
  `equip` int UNSIGNED NOT NULL DEFAULT 0,
  `identify` smallint UNSIGNED NOT NULL DEFAULT 0,
  `refine` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `attribute` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `card0` int UNSIGNED NOT NULL DEFAULT 0,
  `card1` int UNSIGNED NOT NULL DEFAULT 0,
  `card2` int UNSIGNED NOT NULL DEFAULT 0,
  `card3` int UNSIGNED NOT NULL DEFAULT 0,
  `option_id0` smallint NOT NULL DEFAULT 0,
  `option_val0` smallint NOT NULL DEFAULT 0,
  `option_parm0` tinyint NOT NULL DEFAULT 0,
  `option_id1` smallint NOT NULL DEFAULT 0,
  `option_val1` smallint NOT NULL DEFAULT 0,
  `option_parm1` tinyint NOT NULL DEFAULT 0,
  `option_id2` smallint NOT NULL DEFAULT 0,
  `option_val2` smallint NOT NULL DEFAULT 0,
  `option_parm2` tinyint NOT NULL DEFAULT 0,
  `option_id3` smallint NOT NULL DEFAULT 0,
  `option_val3` smallint NOT NULL DEFAULT 0,
  `option_parm3` tinyint NOT NULL DEFAULT 0,
  `option_id4` smallint NOT NULL DEFAULT 0,
  `option_val4` smallint NOT NULL DEFAULT 0,
  `option_parm4` tinyint NOT NULL DEFAULT 0,
  `expire_time` int UNSIGNED NOT NULL DEFAULT 0,
  `bound` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `unique_id` bigint UNSIGNED NOT NULL DEFAULT 0,
  `enchantgrade` tinyint UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`) USING BTREE,
  INDEX `account_id`(`account_id`) USING BTREE
) ENGINE = MyISAM AUTO_INCREMENT = 148832 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = FIXED;

-- ----------------------------
-- Table structure for storage10
-- ----------------------------
DROP TABLE IF EXISTS `storage10`;
CREATE TABLE `storage10`  (
  `id` int UNSIGNED NOT NULL AUTO_INCREMENT,
  `account_id` int UNSIGNED NOT NULL DEFAULT 0,
  `nameid` int UNSIGNED NOT NULL DEFAULT 0,
  `amount` smallint UNSIGNED NOT NULL DEFAULT 0,
  `equip` int UNSIGNED NOT NULL DEFAULT 0,
  `identify` smallint UNSIGNED NOT NULL DEFAULT 0,
  `refine` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `attribute` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `card0` int UNSIGNED NOT NULL DEFAULT 0,
  `card1` int UNSIGNED NOT NULL DEFAULT 0,
  `card2` int UNSIGNED NOT NULL DEFAULT 0,
  `card3` int UNSIGNED NOT NULL DEFAULT 0,
  `option_id0` smallint NOT NULL DEFAULT 0,
  `option_val0` smallint NOT NULL DEFAULT 0,
  `option_parm0` tinyint NOT NULL DEFAULT 0,
  `option_id1` smallint NOT NULL DEFAULT 0,
  `option_val1` smallint NOT NULL DEFAULT 0,
  `option_parm1` tinyint NOT NULL DEFAULT 0,
  `option_id2` smallint NOT NULL DEFAULT 0,
  `option_val2` smallint NOT NULL DEFAULT 0,
  `option_parm2` tinyint NOT NULL DEFAULT 0,
  `option_id3` smallint NOT NULL DEFAULT 0,
  `option_val3` smallint NOT NULL DEFAULT 0,
  `option_parm3` tinyint NOT NULL DEFAULT 0,
  `option_id4` smallint NOT NULL DEFAULT 0,
  `option_val4` smallint NOT NULL DEFAULT 0,
  `option_parm4` tinyint NOT NULL DEFAULT 0,
  `expire_time` int UNSIGNED NOT NULL DEFAULT 0,
  `bound` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `unique_id` bigint UNSIGNED NOT NULL DEFAULT 0,
  `enchantgrade` tinyint UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`) USING BTREE,
  INDEX `account_id`(`account_id`) USING BTREE
) ENGINE = MyISAM AUTO_INCREMENT = 148832 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = FIXED;

SET FOREIGN_KEY_CHECKS = 1;
