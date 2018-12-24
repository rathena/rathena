SELECT *
FROM inventory
WHERE nameid IN (21016,1450,28706,13455,1400,2026,1600,1900,1996,13128,18121,28107,28008,13328,16041,28605,1800)
AND (
	(
		( option_id0 IN (17, 19) AND ( option_val0 < 1 OR option_val0 > 50 ) )
		OR ( option_id1 IN (17, 19) AND ( option_val1 < 1 OR option_val1 > 50 ) )
		OR ( option_id2 IN (17, 19) AND ( option_val2 < 1 OR option_val2 > 50 ) )
	) OR (
		( option_id0 IN (37,39,41,43,45,47,49,51,53,55,57,59,61,63,65,67,69,71,73,75,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,147,148,151,152,164,166,168,170,171,172) AND ( option_val0 < 1 OR option_val0 > 20 ) )
		OR ( option_id1 IN (37,39,41,43,45,47,49,51,53,55,57,59,61,63,65,67,69,71,73,75,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,147,148,151,152,164,166,168,170,171,172) AND ( option_val1 < 1 OR option_val1 > 20 ) )
		OR ( option_id2 IN (37,39,41,43,45,47,49,51,53,55,57,59,61,63,65,67,69,71,73,75,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,147,148,151,152,164,166,168,170,171,172) AND ( option_val2 < 1 OR option_val2 > 20 ) )
	) OR (
		( option_id0 IN (127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,153,154,155,156) AND ( option_val0 < 1 OR option_val0 > 30 ) )
		OR ( option_id1 IN (127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,153,154,155,156) AND ( option_val1 < 1 OR option_val1 > 30 ) )
		OR ( option_id2 IN (127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,153,154,155,156) AND ( option_val2 < 1 OR option_val2 > 30 ) )
	) OR (
		( option_id0 IN (3,4,6,7,8) AND ( option_val0 < 1 OR option_val0 > 10 ) )
		OR ( option_id1 IN (3,4,6,7,8) AND ( option_val1 < 1 OR option_val1 > 10 ) )
		OR ( option_id2 IN (3,4,6,7,8) AND ( option_val2 < 1 OR option_val2 > 10 ) )
	) OR (
		( option_id0 IN (13,14,24) AND ( option_val0 < 1 OR option_val0 > 15 ) )
		OR ( option_id1 IN (13,14,24) AND ( option_val1 < 1 OR option_val1 > 15 ) )
		OR ( option_id2 IN (13,14,24) AND ( option_val2 < 1 OR option_val2 > 15 ) )
	) OR (
		( option_id0 IN (15) AND ( option_val0 < 1 OR option_val0 > 5 ) )
		OR ( option_id1 IN (15) AND ( option_val1 < 1 OR option_val1 > 5 ) )
		OR ( option_id2 IN (15) AND ( option_val2 < 1 OR option_val2 > 5 ) )
	) OR (
		( option_id0 IN (163,176,177,178,179,181,182,185) AND ( option_val0 < 1 OR option_val0 > 1 ) )
		OR ( option_id1 IN (163,176,177,178,179,181,182,185) AND ( option_val1 < 1 OR option_val1 > 1 ) )
		OR ( option_id2 IN (163,176,177,178,179,181,182,185) AND ( option_val2 < 1 OR option_val2 > 1 ) )
	)
)