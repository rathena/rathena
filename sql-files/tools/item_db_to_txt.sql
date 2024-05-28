#
# Exports the `item_db` to TXT format
#

SELECT
	`id`, `name_english`, `name_japanese`, `type`, IFNULL(`price_buy`, ''), IFNULL(`price_sell`, ''), IFNULL(`weight`, ''), IFNULL(`attack`, ''), IFNULL(`defence`, ''), IFNULL(`range`, ''), IFNULL(`slots`, ''), IFNULL(`equip_jobs`, ''), IFNULL(`equip_upper`, ''), IFNULL(`equip_genders`, ''), IFNULL(`equip_locations`, ''), IFNULL(`weapon_level`, ''), IFNULL(`equip_level`, ''), IFNULL(`refineable`, ''), IFNULL(`view`, ''),
	CASE
		WHEN ISNULL(`script`) THEN "{}"
		ELSE CONCAT("{ ", `script`, " }")
	END,
	CASE
		WHEN ISNULL(`equip_script`) THEN "{}"
		ELSE CONCAT("{ ", `equip_script`, " }")
	END,
	CASE
		WHEN ISNULL(`unequip_script`) THEN "{}"
		ELSE CONCAT("{ ", `unequip_script`, " }")
	END
FROM `item_db`
INTO OUTFILE 'C:/item_db.txt'
FIELDS OPTIONALLY ENCLOSED BY ''
TERMINATED BY ','
ESCAPED BY ''
LINES TERMINATED BY '\r\n';
