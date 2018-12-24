SELECT * FROM inventory
WHERE nameid IN (2963, 2964) AND (
	(
		(option_id0 IN (16) AND option_val0 < 3 AND option_val0 > 5 ) OR
		(option_id0 IN (170) AND option_val0 < 5 AND option_val0 > 7 ) OR
		(option_id0 IN (164) AND option_val0 < 5 AND option_val0 > 8 ) OR
		(option_id0 IN (26,27,28,29,172) AND option_val0 < 3 AND option_val0 > 5 ) OR
		(option_id0 IN (35,1,2,13,14) AND option_val0 < 1 AND option_val0 > 3 )
	) OR (
		(option_id1 IN (16) AND option_val1 < 3 AND option_val1 > 5 ) OR
		(option_id1 IN (170) AND option_val1 < 5 AND option_val1 > 7 ) OR
		(option_id1 IN (164) AND option_val1 < 5 AND option_val1 > 8 ) OR
		(option_id1 IN (26,27,28,29,172) AND option_val1 < 3 AND option_val1 > 5 ) OR
		(option_id1 IN (35,1,2,13,14) AND option_val1 < 1 AND option_val1 > 3 )
	)
) OR nameid IN (15073, 15074) AND (
	(
		(option_id0 IN (3,4,5,6,7,8) AND option_val0 < 1 AND option_val0 > 4 ) OR
		(option_id0 IN (1,2) AND option_val0 < 3 AND option_val0 > 5 )
	) OR (
		(option_id1 IN (11,12) AND option_val1 < 50 AND option_val1 > 100 ) OR
		(option_id1 IN (18,22) AND option_val1 < 15 AND option_val1 > 30 ) OR
		(option_id1 IN (20) AND option_val1 < 20 AND option_val1 > 60 ) OR
		(option_id1 IN (21,168,169,170) AND option_val1 < 5 AND option_val1 > 10 ) OR
		(option_id1 IN (186,176,177,178,179) AND option_val1 < 1 AND option_val1 > 1 )
	)
)
