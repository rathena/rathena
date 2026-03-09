UPDATE `char`
SET `body` = `class`
WHERE `body` = '0'
;

UPDATE `char`
SET `body` = '4332' -- RUNE_KNIGHT_2ND
WHERE `body` = '1'
AND `class` IN ( 
	'4054', -- RUNE_KNIGHT
	'4060' -- RUNE_KNIGHT_T
)
;

UPDATE `char`
SET `body` = '4335' -- WARLOCK_2ND
WHERE `body` = '1'
AND `class` IN ( 
	'4055', -- WARLOCK
	'4061' -- WARLOCK_T
)
;

UPDATE `char`
SET `body` = '4337' -- RANGER_2ND
WHERE `body` = '1'
AND `class` IN ( 
	'4056', -- RANGER
	'4062' -- RANGER_T
)
;

UPDATE `char`
SET `body` = '4336' -- ARCHBISHOP_2ND
WHERE `body` = '1'
AND `class` IN ( 
	'4057', -- ARCH_BISHOP
	'4063' -- ARCH_BISHOP_T
)
;

UPDATE `char`
SET `body` = '4333' -- MECHANIC_2ND
WHERE `body` = '1'
AND `class` IN ( 
	'4058', -- MECHANIC
	'4064' -- MECHANIC_T
)
;

UPDATE `char`
SET `body` = '4334' -- GUILLOTINE_CROSS_2ND
WHERE `body` = '1'
AND `class` IN ( 
	'4059', -- GUILLOTINE_CROSS
	'4065' -- GUILLOTINE_CROSS_T
)
;

UPDATE `char`
SET `body` = '4338' -- ROYAL_GUARD_2ND
WHERE `body` = '1'
AND `class` IN ( 
	'4066', -- ROYAL_GUARD
	'4073' -- ROYAL_GUARD_T
)
;

UPDATE `char`
SET `body` = '4341' -- SORCERER_2ND
WHERE `body` = '1'
AND `class` IN ( 
	'4067', -- SORCERER
	'4074' -- SORCERER_T
)
;

UPDATE `char`
SET `body` = '4343' -- MINSTREL_2ND
WHERE `body` = '1'
AND `class` IN ( 
	'4068', -- MINSTREL
	'4075' -- MINSTREL_T
)
;

UPDATE `char`
SET `body` = '4344' -- WANDERER_2ND
WHERE `body` = '1'
AND `class` IN ( 
	'4069', -- WANDERER
	'4076' -- WANDERER_T
)
;

UPDATE `char`
SET `body` = '4342' -- SURA_2ND
WHERE `body` = '1'
AND `class` IN ( 
	'4070', -- SURA
	'4077' -- SURA_T
)
;

UPDATE `char`
SET `body` = '4339' -- GENETIC_2ND
WHERE `body` = '1'
AND `class` IN ( 
	'4071', -- GENETIC
	'4078' -- GENETIC_T
)
;

UPDATE `char`
SET `body` = '4340' -- SHADOW_CHASER_2ND
WHERE `body` = '1'
AND `class` IN ( 
	'4072', -- SHADOW_CHASER
	'4079' -- SHADOW_CHASER_T
)
;
