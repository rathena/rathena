ALTER TABLE guild_position 
ADD CONSTRAINT `guild_position_ibfk_1` FOREIGN KEY (`guild_id`) 
REFERENCES `guild` (`guild_id`) ON DELETE CASCADE
