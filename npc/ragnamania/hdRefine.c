function	script	hdRefine	{
	disable_items;
	.@npc_name$ = getarg(0);
	mes .@npc_name$;
	mes "Unlike others, I am a blacksmith who refines a very limited number of items.";
	mes "I refine only items that are ^CC0000+7 to +9^000000.";
	next;
	mes .@npc_name$;
	mes "My specialty is that even if my refining fails, the refine level decreases by 1 without losing the gear. Isn't it great?";
	next;
	mes .@npc_name$;
	mes "So lets kick this into overdrive, what d' ya say? What item do you want to refine?";
	next;
	setarray .@indices[1], EQI_HEAD_TOP, EQI_ARMOR, EQI_HAND_L, EQI_HAND_R, EQI_GARMENT, EQI_SHOES, EQI_ACC_L, EQI_ACC_R, EQI_HEAD_MID, EQI_HEAD_LOW;
	for(set .@i,1; .@i<=10; set .@i,.@i+1) 
		set .@menu$, .@menu$ + ( getequipisequiped(.@indices[.@i]) ? getequipname(.@indices[.@i]) : F_getpositionname(.@indices[.@i]) +"-[Not equipped]" ) +":";
	set .@part, .@indices[ select(.@menu$) ];
	if (!getequipisequiped(.@part)) {
		mes .@npc_name$;
		switch(.@part) {
		case 1:
			mes "I'm a blacksmith, not a hairstylist.";
			break;
		case 2:
			mes "With my hammer, I will make you a star of the sky.";
			break;
		case 3:
		case 4:
			mes "Making artificial hands is not my specialty.";
			break;
		case 5:
			mes "Bring out the item so I can refine it!";
			break;
		case 6:
			mes "Where is this foot odor coming from?";
			break;
		case 7:
		case 8:
			mes "Where is the accessory?";
			break;
		case 9:
			mes "What do you want me to refine?";
			break;
		case 10:
			mes "Huh? What do you want me to do?";
			break;
		}
		close;
	}
	if (!getequipisenableref(.@part)) {
		mes .@npc_name$;
		mes "This item can't be refined.";
		close;
	}
	if (getequiprefinerycnt(.@part) < 7 || getequiprefinerycnt(.@part) > 9) {
		mes .@npc_name$;
		mes "I only handle items with refine levels from +7 to +9.";
		close;
	}
	
	.@refineitemid = getequipid(.@part); // save id of the item
	.@refinerycnt = getequiprefinerycnt(.@part); //save refinery count
	setarray .@card[0], getequipcardid(.@part,0), getequipcardid(.@part,1), getequipcardid(.@part,2), getequipcardid(.@part,3);
	.@price = getequiprefinecost(.@part, REFINE_COST_HD, REFINE_ZENY_COST);
	.@material = getequiprefinecost(.@part, REFINE_COST_HD, REFINE_MATERIAL_ID);
	
	mes .@npc_name$;
	mes "In order to refine the gear you selected you need ^ff9999"+getitemname(.@material)+"^000000 and 20,000 zeny as a fee.";
	mes "Do you have them ready?";
	next;
	if(select("Yes:No") == 2) {
		mes .@npc_name$;
		mes "I will wait until you are ready.";
		close;
	}
	if (getequippercentrefinery(.@part) < 100) {
		mes .@npc_name$;
		mes "It looks like this item will likely fail to be refined.";
		mes "Well, even if it fails, it only decreases by 1 refine level.";
		mes "Would you like to continue refining?";
		next;
		if(select("Yes:No") == 2) {
			mes .@npc_name$;
			mes "Only those who overcome fear of failure will obtain a masterpiece.";
			close;
		}
	}
	if (countitem(.@material) == 0 || Zeny < .@price) {
		mes .@npc_name$;
		mes "Didn't you just say you had everything ready?";
		close;
	}
	delitem .@material,1;
	set Zeny, Zeny-.@price;

	// anti-hack
	if (callfunc("F_IsEquipIDHack", .@part, .@refineitemid) || callfunc("F_IsEquipCardHack", .@part, .@card[0], .@card[1], .@card[2], .@card[3]) ||
		callfunc("F_IsEquipRefineHack", .@part, .@refinerycnt)) {
		mes .@npc_name$;
		emotion ET_FRET;
		mes "Wait a second...";
		mes "Do you think I'm stupid?!";
		mes "You switched the item while I wasn't looking! Get out of here!";
		close;
	}

	mes .@npc_name$;
	mes "Tac! Tac! Tac!";
	if (getequippercentrefinery(.@part, true) > rand(100)) {
		successrefitem .@part;
		next;
		emotion ET_BEST;
		mes .@npc_name$;
		mes "The sound refreshes my mind everytime I hear it.";
		mes "Here, have it. Refine succeeded flawlessly!";
		close;
	}
	downrefitem .@part;
	next;
	emotion ET_HUK;
	mes .@npc_name$;
	mes "Oops!!";
	next;
	mes .@npc_name$;
	mes "I am sure a person like you would never blame me for a decrease in refine level by 1. Hmm.";
	close;
}