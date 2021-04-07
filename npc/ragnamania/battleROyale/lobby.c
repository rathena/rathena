// Ragnamania battleROyale
// by Biali
// v1.0



new_3-1,145,150,4	script	Manias Shop	553,{
	function BuyCostumes; function BuyManias;

	//DEBUG
	#MANIAS = #MANIAS + 2000;


	mes blue("[ Manias Shop ]");
	mes "Meow! Nice to see you " + blue(strcharinfo(0)) + "!";
	mes "How can I help you today?";
	mes " ";
	mes grey("Manias: ") + #MANIAS;
	next;
	switch(select("Buy Manias:Buy Costumes:Cancel")){
		case 3:
			close;
			break;
		case 2:
			BuyCostumes;
			break;
		case 1:
			BuyManias;
			break;
	}
	close;

	function BuyCostumes {
		for(.@i=0;.@i<getarraysize($@COSTUMES);.@i++) {
			.@menu$ = .@menu$ + strmobinfo(1,$@COSTUMES[.@i]) + grey(" ............ ( m$ " + $@COSTUMES_COST[.@i] + " ):"); 
		}
		.@menu$ = .@menu$ + red("Cancel");
		.@m = select(.@menu$);
		debugmes "menu : " + .@m;

		mes blue("[ Manias Shop ]");
		
		if(getarraysize(#COSTUMES) > 0 && inarray(#COSTUMES[0],$@COSTUMES[(.@m-1)])) {
			mes "Meow! Sorry " + strcharinfo(0) + ", but you already have that one!";
			next;
			return;
		}

		mes "Are you sure you want to buy the " + green(strmobinfo(1,$@COSTUMES[(.@m-1)])) + " costume for " + green($@COSTUMES_COST[(.@m-1)]) + " manias?";
		next;
		if(select("Yes, please:No, Cancel") == 2)
			return;
		else {
			if (#MANIAS >= $@COSTUMES_COST[(.@m-1)]) {
				#MANAIS = #MANIAS - $@COSTUMES_COST[(.@m-1)];
				setarray #COSTUMES[getarraysize(#COSTUMES)], $@COSTUMES[(.@m-1)];
				mes blue("[ Manias Shop ]");
				mes "Congratulations! Yo've got a brand new costume you can use in the Arena!";
				mes "Do you want to set that as your default costume now?";
				next;
				if(select("Yes, please:No, not now") == 2) {
					mes blue("[ Manias Shop ]");
					mes "No problem! See you around!";
					next;
					return;
				} else {
					ACTIVE_COSTUME = $@COSTUMES[(.@m-1)];
					mes blue("[ Manias Shop ]");
					mes "Perfect! I've done that for you!";
					mes "You will get into your costume automatically next time you get your ass in the Arena.";
					mes "I bet it is gonna be a huge success!";
					next;
					return;
				}
			} else {
				mes blue("[ Manias Shop ]");
				mes "Oh dear, "+ strcharinfo(0) +"! I am afraid you don't have enough Manias to buy that!";
				next;
				return;
			}
		}

		return;
	}

	function	BuyManias {

		return;
	}

}

// MAPFLAGS //
//*************************************************
new_3-1	mapflag	loadevent
new_3-1	mapflag	pvp	off
new_3-1	mapflag	noskill
new_3-1	mapflag	partylock
new_3-1	mapflag	guildlock











