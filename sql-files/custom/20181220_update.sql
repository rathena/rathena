# rename GMT coin
REPLACE INTO `item_db2_re` (`id`, `name_english`, `name_japanese`, `type`, `price_buy`, `price_sell`, `weight`, `atk:matk`, `defence`, `range`, `slots`, `equip_jobs`, `equip_upper`, `equip_genders`, `equip_locations`, `weapon_level`, `equip_level`, `refineable`, `view`, `script`, `equip_script`, `unequip_script`) VALUES
(6671, 'Geffen_Magic_Tournament_Coin', 'Geffen Magic Tournament Coin', 3, 10, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

# food box
REPLACE INTO `item_db2_re` (`id`, `name_english`, `name_japanese`, `type`, `price_buy`, `price_sell`, `weight`, `atk:matk`, `defence`, `range`, `slots`, `equip_jobs`, `equip_upper`, `equip_genders`, `equip_locations`, `weapon_level`, `equip_level`, `refineable`, `view`, `script`, `equip_script`, `unequip_script`) VALUES
(8503, '10_Food_Box', '+10 Food Box', 18, 20, NULL, 0, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getrandgroupitem(IG_Cash_Food);', NULL, NULL);

# Blacksmith Blessing boxes
REPLACE INTO `item_db2_re` (`id`, `name_english`, `name_japanese`, `type`, `price_buy`, `price_sell`, `weight`, `atk:matk`, `defence`, `range`, `slots`, `equip_jobs`, `equip_upper`, `equip_genders`, `equip_locations`, `weapon_level`, `equip_level`, `refineable`, `view`, `script`, `equip_script`, `unequip_script`) VALUES
(8504, 'Blacksmith_Blessing_20Box', 'Blacksmith Blessing x 20 Box', 2, 20, NULL, 10, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getitem 6226,10;', NULL, NULL),
(8505, 'Blacksmith_Blessing_40Box', 'Blacksmith Blessing x 40 Box', 2, 20, NULL, 10, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getitem 6226,10;', NULL, NULL),
(8506, 'Blacksmith_Blessing_1000Box', 'Blacksmith Blessing x 100 Box', 2, 20, NULL, 10, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getitem 6226,10;', NULL, NULL);