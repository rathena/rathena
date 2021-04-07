sala_premmy,140,66,3	script	Legendary Smith	813,{
	function Cm;
	function Refine_Item;
	function Equip_Menu; 
	if(!vip_status(1) && getgmlevel()<10) end;
	mes .n$;
	mes "Hello, friend...";
	mes "How can I help you today?";
	next;
	switch(select("~ Normal Refine:~ Enriched Refine:~ Purify Ore")) {
		case 1:
			mes .n$;
			mes "What do you want to refine?";
			next;
			Equip_Menu(1);
			setarray .@Materials[0],985,1010,1011,984,984;
			setarray .@Safe[0],4,7,6,5,4;
			set .@WLv, getequipweaponlv(@i);
			set .@SafeCount, .@Safe[.@WLv]-getequiprefinerycnt(@i);
			mes .n$;
			mes "Item: +"+getequiprefinerycnt(@i)+" "+getequipname(@i);
			next;
			switch(select(" ~ Refine once (^0055FF"+getequippercentrefinery(@i)+"^000000% success): ~ Refine multiple times (up to "+(10-getequiprefinerycnt(@i))+"): ~ Refine to safe limit ("+((.@SafeCount>0)?.@SafeCount+" refines":"^777777disabled^000000")+"): ~ ^777777Cancel^000000")) {
				case 1:
					Refine_Item(1, .@Materials[.@WLv], .RP[.@WLv], .@Safe[.@WLv]);
					close;
				case 2:
					input .@Refines;
					if (.@Refines+getequiprefinerycnt(@i) > 10 || .@Refines < 1) {
						mes .n$;
						mes "Invalid number ("+.@Refines+").";
					} else
						Refine_Item(.@Refines, .@Materials[.@WLv], .RP[.@WLv], .@Safe[.@WLv]);
					close;
				case 3:
					if (.@SafeCount < 1) {
						mes .n$;
						mes "The item has already reached the safe limit.";
					} else
						Refine_Item(.@SafeCount, .@Materials[.@WLv], .RP[.@WLv], .@Safe[.@WLv]);
					close;
				case 4:
					close;
			}
		case 2:
			mes .n$;
			mes "What do you want to refine?";
			next;
			Equip_Menu(1);
			setarray .@Materials[0],7619,7620,7620,7620,7620;
			setarray .@Safe[0],4,7,6,5,4;
			set .@WLv, getequipweaponlv(@i);
			mes .n$;
			mes "Item: +"+getequiprefinerycnt(@i)+" "+getequipname(@i);
			next;
			if(select(" ~ Refine item: ~ ^777777Cancel^000000") == 2)
				close;
			Refine_Item(1, .@Materials[.@WLv], .RP[.@WLv], .@Safe[.@WLv], 1);
			close;
			break;
		case 3:
			callfunc "massPurify","Legendary Smith";
			end;
			break;
	}

OnInit:
	.n$ = "^990099[ Legendary Smith ]^000000";
	setarray .RP[0],2000,50,200,5000,20000;	// Zeny for armor, lv1, lv2, lv3, lv4 refines
	end;
	

	function Equip_Menu {
		setarray .@pos$[1],"Left Accessory","Footgear","Garment","Lower Headgear","Mid Headgear","Upper Headgear","Armor","Left hand","Right hand","Right Accessory";
		set .@menu$,"^0000AACancel^000000:";
		for (set .@i,2; .@i < 10; set .@i,.@i+1) {
			if (getequipisequiped(.@i))
				set .@menu$, .@menu$+.@pos$[.@i]+" [^0055FF"+getequipname(.@i)+"^000000]";
			set .@menu$, .@menu$+":";
		}
		set @i, select(.@menu$);
		if(.@i == 1) 
			close;
		if (!getequipisequiped(@i)) {
			mes .n$;
			mes "Nothing is equipped there!";
			close;
		}
		if (getarg(0)) {
			if (!getequipisenableref(@i)) {
				mes .n$;
				mes getequipname(@i)+" cannot be refined.";
				close;
			}
			if (getequiprefinerycnt(@i) >= 10 ) {
				mes .n$;
				mes "+"+getequiprefinerycnt(@i)+" "+getequipname(@i)+" cannot be refined any further.";
				close;
			}
		}
		return;
	}


	function Cm {
		set .@str$, getarg(0);
		for(set .@i,getstrlen(.@str$)-3; .@i>0; set .@i,.@i-3)
			set .@str$, insertchar(.@str$,",",.@i);
		return .@str$;
	}

	function Refine_Item {
		mes .n$;
		set .@price, getarg(0)*getarg(2);
		mes "I'll need "+getarg(0)+"x "+getitemname(getarg(1))+" and "+Cm(.@price)+" Zeny.";
		if (countitem(getarg(1)) < getarg(0) || Zeny < .@price) {
			mes "Come back when you have the required materials.";
			close;
		}
		if (getequiprefinerycnt(@i)+getarg(0) > getarg(3))
			mes "The item will be refined above the safe limit. It may break.";
		next;
		if(select(" ~ ^0055FFContinue^000000: ~ ^777777Cancel^000000") == 2)
			close;
		mes .n$;
		set .@j, getarg(0);
		while(.@j) {
			if(rand(100) > 20) //20% chance of not consuming the item
				delitem getarg(1),1;
			set Zeny, Zeny-getarg(2);
			if (getarg(4,0)) {
				if (getequippercentrefinery(@i) <= rand(100) && getequippercentrefinery(@i) <= rand(100)) {
					mes getequipname(@i)+" broke while refining.";
					failedrefitem @i;
					close;
				}
			} else {
				if (getequippercentrefinery(@i) <= rand(100)) {
					mes getequipname(@i)+" broke while refining.";
					failedrefitem @i;
					close;
				}
			}
			successrefitem @i;
			set .@j, .@j-1;
		}
		mes "All done!";
		close;
	}
}


// Ori/Elu Functions
//============================================================
function	script	massPurify	{
	disable_items;
	.@npc_name$ = "^990099[ "+getarg(0)+" ]^000000";
	mes .@npc_name$;
	mes "I can purify your";
	mes "Rough Oridecons or";
	mes "Rough Eluniums. I'll need";
	mes "5 Rough Stones to make";
	mes "1 pure one for you.";
	next;
	mes .@npc_name$;
	mes "I run this in batches";
	mes "and will proccess all";
	mes "the chosen minerals at once.";
	next;
	switch(select("Make Oridecon:Make Elunium:Ask about Enchanted Stones")) {
	case 1:
		.@rough = 756;
		.@pure = 984;
		break;
	case 2:
		.@rough = 757;
		.@pure = 985;
		break;
	case 3:
		mes .@npc_name$;
		mes "Enchanted Stones...?";
		mes "I've been a stonesmith for 20 years, so I've heard a lot about them. Supposedly, there are";
		mes "four different kinds.";
		next;
		mes .@npc_name$;
		mes "Each Enchanted Stone possesses one of the following elemental properties: Earth, Wind, Water and Fire.";
		next;
		mes .@npc_name$;
		mes "If someone combines a Enchanted Stone with a weapon while smithing, that weapon will possess the same property as the Stone.";
		next;
		mes .@npc_name$;
		mes "Needless to say, you need to have some smithing skill to produce this kind of elemental weapon.";
		close;
	}

	.@qt_mp = countitem(.@rough);
	.@qt_proc = .@qt_mp/5;
	.@cost = .@qt_proc * 80;

	mes .@npc_name$;
	if(!.@qt_proc) {
		mes "I am sorry but you don't seem to have enough ^0000FF" + getitemname(.@rough ) +"^000000 in order for us to continue.";
		close;
	}

	mes "You have ^0000ff"+ .@qt_mp+ "x "+getitemname(.@rough )+" ^000000in your inventory... We could make ^0000FF" + .@qt_proc + "x "+getitemname(.@pure)+"^000000 out of them and it would cost you only ^ff0000"+callfunc("F_InsertComma",.@cost) +"z^000000. Do you want to continue?";
	next;
	if(select("Yes, please:No, thank you.") == 2)
		close;

	if (checkweight(.@pure,.@qt_proc) == 0) {
		mes .@npc_name$;
		mes "I am afraid you are carrying too much weight in order for us to continue.";
		close;
	}
	if (Zeny < .@cost) {
		mes .@npc_name$;
		mes "I am afraid you don't have enough Zeny to continue.";
		close;
	}
	delitem .@rough,.@qt_mp;  //rough stones
	getitem .@pure,.@qt_proc; // purified stone

	mes .@npc_name$;
	mes "We've created ^0000ff" + .@qt_proc +" "+ getitemname(.@pure) + "^000000 it has costed you ^ff0000" + callfunc("F_InsertComma",.@cost) +"z.^000000";
	close;
}