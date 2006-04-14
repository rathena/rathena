#Delete 2^13 (Peco Knight)
update item_db set equip_jobs = equip_jobs&~0x2000 where equip_jobs&0x2000;
#Move 2^20 -> 2^19 (Dancer -> Bard)
update item_db set equip_jobs = (equip_jobs|0x80000)&~0x100000 where equip_jobs&0x100000;
#Remove 2^21 (Peco Crusader)
update item_db set equip_jobs = equip_jobs&~0x200000 where equip_jobs&0x200000;
#Remove 2^22 (Wedding)
update item_db set equip_jobs = equip_jobs&~0x400000 where equip_jobs&0x400000;
#Remove 2^23 (S. Novice)
update item_db set equip_jobs = equip_jobs&~0x800000 where equip_jobs&0x800000;
#Move 2^24 -> 2^21 (TK)
update item_db set equip_jobs = (equip_jobs|0x200000)&~0x1000000 where equip_jobs&0x1000000;
#Move 2^25 -> 2^22 (SG)
update item_db set equip_jobs = (equip_jobs|0x400000)&~0x2000000 where equip_jobs&0x2000000;
#Move 2^26 -> 2^23 (SL)
update item_db set equip_jobs = (equip_jobs|0x800000)&~0x8000000 where equip_jobs&0x8000000;
#Move 2^28 -> 2^24 (GS)
update item_db set equip_jobs = (equip_jobs|0x1000000)&~0x4000000 where equip_jobs&0x4000000;
#Move 2^27 -> 2^25 (NJ)
update item_db set equip_jobs = (equip_jobs|0x2000000)&~0x10000000 where equip_jobs&0x10000000;
#Make items usable by everyone into 0xFFFFFFFF
update item_db set equip_jobs = 0xFFFFFFFF where equip_jobs = 0x3EFDFFF;
#Make items usable by everyone but novice into 0xFFFFFFFE
update item_db set equip_jobs = 0xFFFFFFFE where equip_jobs = 0x0EFDFFE;
#Update items usable by everyone except acolyte/priest/monk/gunslinger
update item_db set equip_jobs = 0xFDFF7EEF where equip_jobs = 0x28F5EEF;
