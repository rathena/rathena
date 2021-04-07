function	script	F_HouseInstancing	{
	.@instance$ = "Home Sweet Home";
	if(instance_id(IM_CHAR) > 0) {
		instance_destroy instance_id(IM_CHAR);
		sleep2 500;
	}

    .@create = instance_create(.@instance$, IM_CHAR);
    if (.@create < 0) {
        mes "[Home Sweet Home]";
        switch (.@create) {
            case -1: mes "ERROR: Invalid type."; break;
            case -2: mes "ERROR: Party not found."; break;
            case -3: mes "ERROR: Instance already exists."; break;
            case -4: mes "ERROR: No free instances."; break;
        }
        mes " ";
        mes "Instance creation ^FF0000failed^000000.";
        emotion ET_HUK;
        close;
    }

	.@enter = instance_enter(.@instance$,-1,-1,getcharid(0),instance_id(IM_CHAR));
	if (.@enter != 0) {
		mes "[Home Sweet Home]";
		switch (.@enter) {
			case 1: mes "ERROR: Party not found."; break;
			case 2: mes "ERROR: Party does not have an instance."; break;
			case 3: mes "ERROR: Unknown error."; break;
		}
		mes " ";
		mes "Instance entry ^FF0000failed^000000.";
		emotion ET_HUK;
		close;
	}
	set getvariableofinstance('HouseNumber,instance_id(IM_CHAR)),atoi(#HOUSING$[0]); //'
	callfunc "F_CheckServices";
	callfunc "F_SpawnCrops";
	end;
}


function	script	F_CheckServices	{
	// #HOUSING$[0], rand(1,23);				// House Id
	// #HOUSING$[1], gettimetick(2) + .Period;	// Rent amount
	
	// #HOUSING$[4], 0; 						// Claning Lady
	// #HOUSING$[8], 0; 						// Wardrobe/Mirror
	
	// #HOUSING$[2], 0; // Gardener
	if(atoi(#HOUSING$[2])) donpcevent instance_npcname("Maia",instance_id(IM_CHAR))+"::OnEnable";
	// #HOUSING$[3], 0; // Housekeeper
	if(atoi(#HOUSING$[3])) donpcevent instance_npcname("Alfonso",instance_id(IM_CHAR))+"::OnEnable";
	// #HOUSING$[6], 0; // Storage
	if (HOUSE_STORAGE) donpcevent instance_npcname("#Storage",instance_id(IM_CHAR))+"::OnEnable";
	// #HOUSING$[7], 0; // Beds
	if (HOUSE_BEDS) donpcevent instance_npcname("#Beds",instance_id(IM_CHAR))+"::OnEnable";
	// #HOUSING$[5], 0; // Oven
	if (HOUSE_OVEN) donpcevent instance_npcname("#Oven",instance_id(IM_CHAR))+"::OnEnable";

	return;	
}

function	script	F_SpawnCrops	{
	for(.@i=0;.@i<getarraysize(#HOUSING_CROPS$);.@i++)
		if(atoi(#HOUSING_CROPS$[.@i]) > 0) { //Plantado mas nao pronto pra colher TODO
			.@item 	= getelementofarray(getvariableofnpc(.Crops,instance_npcname("Maia",instance_id(IM_CHAR))),.@i);
			.@y		= getelementofarray(getvariableofnpc(.CropsY,instance_npcname("Maia",instance_id(IM_CHAR))),.@i);
			.@mobid	= 1078+.@i;
			if(.@i>4) .@mobid -= (.@i-5);
			.@j=29;
			for(.@c=0;.@c<atoi(#HOUSING_CROPS_QTY$[.@i]);.@c++) {
				if(atoi(#HOUSING_CROPS$[.@i])-10800 > gettimetick(2)) // falta mais de 3 horas pra acabar 
					monster instance_mapname("rentb1",instance_id(IM_CHAR)),.@j,.@y,getitemname(.@item)+" Sprout",.@mobid,1,instance_npcname("Maia",instance_id(IM_CHAR))+"::On_"+.@item,1,AI_NONE;
				else if(atoi(#HOUSING_CROPS$[.@i]) > gettimetick(2)) // Quite there...
					monster instance_mapname("rentb1",instance_id(IM_CHAR)),.@j,.@y,getitemname(.@item)+" Sprout",.@mobid,1,instance_npcname("Maia",instance_id(IM_CHAR))+"::On_"+.@item,0,AI_NONE;
				else // Ready to harvest!
					monster instance_mapname("rentb1",instance_id(IM_CHAR)),.@j,.@y,getitemname(.@item)+" Sprout",.@mobid,1,instance_npcname("Maia",instance_id(IM_CHAR))+"::On_"+.@item,2,AI_NONE;
				.@j+=2;
			}
		}
	return;
}

function	script	F_AdjustTipsPot	{
	.@index		= getarg(0);
	.@amount 	= getarg(1);
	set .@tips, atoi(#HOUSING_TIPS$[.@index]);
	set .@tips, .@tips - .@amount;
	if(.@tips < 0) set .@tips, 0;
	set #HOUSING_TIPS$[0], .@tips;

	return;
}	

function	script	F_CalcCosts	{
	.@total =  getvariableofnpc(.Rent,"mngr");
	.@total += (atoi(#HOUSING$[2])>gettimetick(2)?getvariableofnpc(.Gardener,"mngr"):0);
	.@total += (atoi(#HOUSING$[3])>gettimetick(2)?getvariableofnpc(.Housekeeper,"mngr"):0);

	return .@total;
}