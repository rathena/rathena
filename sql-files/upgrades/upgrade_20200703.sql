-- Fix rename flag and intimacy in inventories
update `inventory` `i`
inner join `pet` `p`
on
	`i`.`card0` = 256
and
	( `i`.`card1` | ( `i`.`card2` << 16 ) ) = `p`.`pet_id`
set
	`i`.`card3` = 
		( 
			CASE
				WHEN `p`.`intimate` < 100 THEN
					1 -- awkward
				WHEN `p`.`intimate` < 250 THEN
					2 -- shy
				WHEN `p`.`intimate` < 750 THEN
					3 -- neutral
				WHEN `p`.`intimate` < 910 THEN
					4 -- cordial
				WHEN `p`.`intimate` <= 1000 THEN
					5 -- loyal
				ELSE 0 -- unrecognized
			END << 1
		) | `p`.`rename_flag`
;

-- Fix rename flag and intimacy in carts
update `cart_inventory` `i`
inner join `pet` `p`
on
	`i`.`card0` = 256
and
	( `i`.`card1` | ( `i`.`card2` << 16 ) ) = `p`.`pet_id`
set
	`i`.`card3` = 
		( 
			CASE
				WHEN `p`.`intimate` < 100 THEN
					1 -- awkward
				WHEN `p`.`intimate` < 250 THEN
					2 -- shy
				WHEN `p`.`intimate` < 750 THEN
					3 -- neutral
				WHEN `p`.`intimate` < 910 THEN
					4 -- cordial
				WHEN `p`.`intimate` <= 1000 THEN
					5 -- loyal
				ELSE 0 -- unrecognized
			END << 1
		) | `p`.`rename_flag`
;

-- Fix rename flag and intimacy in storages
update `storage` `i`
inner join `pet` `p`
on
	`i`.`card0` = 256
and
	( `i`.`card1` | ( `i`.`card2` << 16 ) ) = `p`.`pet_id`
set
	`i`.`card3` = 
		( 
			CASE
				WHEN `p`.`intimate` < 100 THEN
					1 -- awkward
				WHEN `p`.`intimate` < 250 THEN
					2 -- shy
				WHEN `p`.`intimate` < 750 THEN
					3 -- neutral
				WHEN `p`.`intimate` < 910 THEN
					4 -- cordial
				WHEN `p`.`intimate` <= 1000 THEN
					5 -- loyal
				ELSE 0 -- unrecognized
			END << 1
		) | `p`.`rename_flag`
;

-- Fix rename flag and intimacy in guild storages
update `guild_storage` `i`
inner join `pet` `p`
on
	`i`.`card0` = 256
and
	( `i`.`card1` | ( `i`.`card2` << 16 ) ) = `p`.`pet_id`
set
	`i`.`card3` = 
		( 
			CASE
				WHEN `p`.`intimate` < 100 THEN
					1 -- awkward
				WHEN `p`.`intimate` < 250 THEN
					2 -- shy
				WHEN `p`.`intimate` < 750 THEN
					3 -- neutral
				WHEN `p`.`intimate` < 910 THEN
					4 -- cordial
				WHEN `p`.`intimate` <= 1000 THEN
					5 -- loyal
				ELSE 0 -- unrecognized
			END << 1
		) | `p`.`rename_flag`
;
