-	script	rewardcommand	HIDDEN_WARP_NPC,{
OnInit:
//==== CONF =====
	.sender$ = "Ragnamania Staff";
	.title$ = "Reward";
	.body$ = "Here's something for you. Have fun!";
	.zeny = 0;	// Default zent sent
	setarray .mailitem[0], 607, 617; // Default items (Max.5)
	setarray .mailamount[0], 3, 1; // Default items quantity
//===============	
	bindatcmd "reward", strnpcinfo(3)+"::OnAtcommand", 99,100;
	end;

OnAtcommand:
	if (.@atcmd_parameters$[0] == "all")
		.@mode = 1;
	if (.@atcmd_parameters$[0] == "map")
		.@mode = 2;
	if (.@mode) {
		if (getarraysize(.mailitem) != getarraysize(.mailamount)) {
			dispbottom "Error.";
			end;
		}
		if (.@mode == 1)
			addrid(0, 0);
		else
			addrid(1, 0);
		if (checkvending(strcharinfo(0)))
			detachrid;
		if (.mailitem[0])
			mail getcharid(0), .sender$, .title$, .body$, .zeny, .mailitem, .mailamount;
		else
			mail getcharid(0), .sender$, .title$, .body$, .zeny;
		detachrid;
		end;
	}
	mes "Use [@reward all] to send rodex to everyone online.";
	mes "Use [@reward map] to send rodex to everyone on your map.";
	while (true) {
		next;
		mes "RodEx contents:";
		mes "" + callfunc("F_InsertComma",.zeny) + "z.";
		if (.mailitem[0]) {
			for ( .@i = 0; .@i < getarraysize(.mailitem); ++.@i )
				mes "" + .mailamount[.@i] + "x " + getitemname(.mailitem[.@i]) + ".";
		}
		switch(select("Close.", "Change zeny.", "Change item(s).")) {
		case 1:
			close;
		case 2:
			input .zeny;
			break;
		case 3:
			do {
				next;
				mes "[Item list]";
				if (.mailitem[0]) {
					for ( .@i = 0; .@i < getarraysize(.mailitem); ++.@i )
						mes "" + .mailamount[.@i] + "x " + getitemname(.mailitem[.@i]) + ".";
				}
				else
					mes "Empty.";
				next;
				switch(select("Return", "Add item", "Remove item", "Remove all")) {
				case 1:
					break;
				case 2:
					if (getarraysize(.mailitem) > 4) {
						mes "You cannot add more items. Max 5.";
						break;
					}
					mes "Input item ID.";
					input .@newitem;
					if (getitemname(.@newitem) == "null") {
						mes "Item not found.";
						break;
					}
					mes "- " + getitemname(.@newitem);
					if (getiteminfo(.@newitem, 2) == 4 || getiteminfo(.@newitem, 2) == 5 || getiteminfo(.@newitem, 2) == 12) {
						.@newqut = 1;
					}
					else {
						mes "Input item quantity.";
						input .@newqut;
						if (.@newqut < 1 || .@newqut > 1000) {
							mes "Invalid quantity.";
							break;
						}
					}
					.mailitem[getarraysize(.mailitem)] = .@newitem;
					.mailamount[getarraysize(.mailamount)] = .@newqut;
					next;
					break;
				case 3:
					if (.mailitem[0]) {
						mes "Input index to remove.";
						for ( .@i = 0; .@i < getarraysize(.mailitem); ++.@i )
							mes "" + .@i + ": " + .mailamount[.@i] + "x " + getitemname(.mailitem[.@i]) + ".";
						input .@i;
						if (.mailitem[.@i] == 0 || .mailamount[.@i] == 0)
							mes "Invalid index.";
						else {
							deletearray .mailitem[.@i], 1;
							deletearray .mailamount[.@i], 1;
						}
					}
					else
						mes "No item to remove.";
					break;
				case 4:
					deletearray .mailitem[0], getarraysize(.mailitem);
					deletearray .mailamount[0], getarraysize(.mailamount);
					break;
				}
			} while (@menu != 1);
		}
	}
}