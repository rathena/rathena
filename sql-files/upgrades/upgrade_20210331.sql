UPDATE `inventory` SET `card2` = `card2` & 65535 WHERE `card0` = 254 OR `card0` = 255;
UPDATE `cart_inventory` SET `card2` = `card2` & 65535 WHERE `card0` = 254 OR `card0` = 255;
UPDATE `storage` SET `card2` = `card2` & 65535 WHERE `card0` = 254 OR `card0` = 255;
UPDATE `guild_storage` SET `card2` = `card2` & 65535 WHERE `card0` = 254 OR `card0` = 255;
