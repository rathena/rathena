prt_in,44,113,3	script	BG Items Trader	56,{

Main:
	mes "^009999[BG Items Trader]^000000";
	mes "Welcome, mighty warrior.";
	mes "Are you looking for supplies?";
	mes "I have a vast range of ^FF0000" + .Target$ + "^000000 items in exchange for your ^AA00AA"+getitemname(7828)+"^000000.";
	next;

	set .@Menu$, "";
	for( .@i=0; .@i < getarraysize(.Items); .@i++ )
		set .@Menu$, .@Menu$ + .Amount[.@i] + " " + getitemname(.Items[.@i]) + " (" + .Value[.@i] + " badges):";
	set .@j, select(.@Menu$) - 1;

	mes "^009999[BG Items Trader]^000000";
	mes "Item: ^0000FF " + .Amount[.@j] + "x " + getitemname(.Items[.@j]) + "^000000";
	mes "Cost: ^0000FF " + .Value[.@j] + " " + getitemname(7828) + "^000000";
	mes " ";
	mes "--------------------------------";
	mes "^FF0000" + .Target$ + " Item^000000";
	mes "--------------------------------";

	while(!.@repeat) {
		if( countitem(7828) >= .Value[.@j] ) {
			if(select("^FF0000Pay ["+ .Value[.@j] + " / " + countitem(7828) + "] " + getitemname(7828)+"^000000:Cancel") == 2) {
				next;
				goto Main;
			} else {
				if( checkweight(.Items[.j],.Amount[.j]) == 0 ) {
					next;
					mes "^009999[BG Items Trader]^000000";
					mes "I am afraid you can't carry more items. Please free up some space in your inventory.";
					close;
				} else {
					delitem 7828,.Value[.@j];
					getitem2 .Items[.@j],.Amount[.@j],1,0,0,254,0,.cID&0xffff,(.cID>>16)&0xffff;
					logmes "Trocou " + .Value[.@j] + "x " + getitemname(7828) + " por " + .Amount[.@j] + " " + getitemname(.Item[.@j]);
					if(select("Continue:Leave") == 2) {
						next;
						goto Main;
					}
				}
			}
		} else {
			next;
			mes "^009999[BG Items Trader]^000000";
			mes "You do not have enough badges. Sorry...";
			mes "Participate the ^FF0000"+.Target$+"^000000 to earn more.";
			close;
		}
	}
	end;

OnInit:
	setarray .Items[0],   547,  504,  505,  678, 7135, 7136, 7137, 7138, 7139,  715,  716,  717,12016, 1025, 7321,  662,12072,12077,12082,12087,12092,12097,12215,12216;
	setarray .Amount[0],  150,  100,  100,   10,   40,   40,   30,   30,   25,   50,  100,  200,   10,   40,   25,   10,    3,    3,    3,    3,    3,    3,	10,	10;
	//setarray .Value[0],     6,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    1,    1,    1,    1,    1,    1;
	setarray .Value[0],     1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,	1,	1;
	set .cID, getbattleflag("bg_reserved_char_id");
	set .Target$, "Battleground";
	end;
}
