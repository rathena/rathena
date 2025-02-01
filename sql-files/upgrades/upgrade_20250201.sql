ALTER TABLE `skillcooldown_homunculus`
	DROP PRIMARY KEY,
	ADD PRIMARY KEY(`homun_id`, `skill`)
;

ALTER TABLE `skillcooldown_mercenary`
	DROP PRIMARY KEY,
	ADD PRIMARY KEY(`mer_id`, `skill`)
;
