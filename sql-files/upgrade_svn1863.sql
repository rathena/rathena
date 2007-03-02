ALTER TABLE item_db ADD `refineable` tinyint(1) default NULL AFTER `equip_level`;
ALTER TABLE item_db2 ADD `refineable` tinyint(1) default NULL AFTER `equip_level`;
UPDATE item_db SET refineable=1 where type=4 or type=5;
UPDATE item_db2 SET refineable=1 where type=4 or type=5;
UPDATE item_db SET refineable = 0 where id = 1243 OR id = 1530 OR id = 2110 OR id = 2112 OR id = 2250 OR id = 2253 OR id = 2264 OR id = 2271 OR id = 2279 OR id = 2282 OR id = 2289 OR id = 2290 OR id = 2293 OR id = 2298 OR id = 2352 OR id = 2410 OR id = 2413 OR id = 2414 OR id = 2509 OR id = 2510 OR id = 5008 OR id = 5015 OR id = 5037 OR id = 5039 OR id = 5046 OR id = 5049 OR id = 5050 OR id = 5053 OR id = 5055 OR id = 5058 OR id = 5098;
UPDATE item_db SET refineable = 0 where equip_locations = 513 OR equip_locations = 512 OR equip_locations = 136 OR equip_locations = 1;
UPDATE item_db2 SET refineable = 0 where equip_locations = 513 OR equip_locations = 512 OR equip_locations = 136 OR equip_locations = 1;
