# Upgrade login and character server tables for eAthena 1.0 RC 1 to 1.0 RC 2

ALTER TABLE `login` MODIFY `account_id` INT(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2000000;
