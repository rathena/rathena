prt_in,46,97,0	script	PvP Items Trader	730,{

Main:
	mes "^009999[PvP Items Trader]^000000";
	mes "Welcome, mighty warrior.";
	mes "Are you looking for supplies?";
	mes "I have a vast range of ^FF0000" + .Target$ + "^000000 items in exchange for your ^AA00AA"+getitemname(7829)+"^000000.";
	next;

	set .@Menu$, "";
	for( .@i=0; .@i < getarraysize(.Items); .@i++ )
		set .@Menu$, .@Menu$ + .Amount[.@i] + " " + getitemname(.Items[.@i]) + " (" + .Value[.@i] + " badges):";
	set .@j, select(.@Menu$) - 1;

	mes "^009999[PvP Items Trader]^000000";
	mes "Item: ^0000FF " + .Amount[.@j] + "x " + getitemname(.Items[.@j]) + "^000000";
	mes "Cost: ^0000FF " + .Value[.@j] + " " + getitemname(7829) + "^000000";
	mes " ";
	mes "--------------------------------";
	mes "^FF0000" + .Target$ + " Item^000000";
	mes "--------------------------------";

	while(!.@repeat) {
		if( countitem(7829) >= .Value[.@j] ) {
			if(select("^FF0000Pay ["+ .Value[.@j] + " / " + countitem(7829) + "] " + getitemname(7829)+"^000000:Cancel") == 2) {
				next;
				goto Main;
			} else {
				if( checkweight(.Items[.j],.Amount[.j]) == 0 ) {
					next;
					mes "^009999[PvP Items Trader]^000000";
					mes "I am afraid you can't carry more items. Please free up some space in your inventory.";
					close;
				} else {
					delitem 7829,.Value[.@j];
					getitem2 .Items[.@j],.Amount[.@j],1,0,0,254,0,.cID&0xffff,(.cID>>16)&0xffff;
					logmes "Trocou " + .Value[.@j] + "x " + getitemname(7829) + " por " + .Amount[.@j] + " " + getitemname(.Item[.@j]);
					if(select("Continue:Leave") == 2) {
						next;
						goto Main;
					}
				}
			}
		} else {
			next;
			mes "^009999[PvP Items Trader]^000000";
			mes "You do not have enough badges. Sorry...";
			mes "Participate the ^FF0000"+.Target$+"^000000 to earn more.";
			close;
		}
	}
	end;

OnInit:
	setarray .Items[0],   547,  504,  505,  678, 7135, 7136, 7137, 7138, 7139,  715,  716,  717,  662,12072,12077,12082,12087,12092,12097;
	setarray .Amount[0],   30,   50,   10,    3,   10,   10,   15,   15,    5,   20,   20,   20,    5,    5,    5,    5,    5,    5,    5;
	setarray .Value[0],     1,    1,    2,    1,    2,    2,    2,    2,    2,    1,    1,    1,    1,    3,    3,    3,    3,    3,    3;
	set .cID, getbattleflag("pvp_reserved_char_id");
	set .Target$, "PvP";
	end;
}

  
// 547, // White Slim Potion
// 504,   // White potion
// 505,   // Blue Potion
// 678,   // Poison Bottle
// 7135,  // Granade Bottle
// 7136,  // Acid Bottle
// 7137,  // Plant Bottle
// 7138,  // Marine Sphere Bottle
// 7139,  // Glistening Coar Bottle
// 715,   // Yellow Gemstone
// 716,   // Red Gemstone
// 717,   // Blue Gemstone
// 662,   // Authoritative Badge
// 12072, // STR +7
// 12077, // INT +7
// 12082, // VIT +7
// 12087, // AGI +7
// 12092, // DEX +7
// 12097; // LUK +7

